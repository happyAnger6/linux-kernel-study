#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <asm/current.h>

static dev_t wait_dev;
static int wait_major;
static int wait_minor;
static int wait_num;
static int flag;

module_param(wait_minor,int,0644);
module_param(wait_num,int,0644);

struct waitqueue_dev{
	spinlock_t lock;
	wait_queue_head_t my_wait;
	struct cdev cdev;
};

static struct waitqueue_dev *pdev;

ssize_t waitqueue_read(struct file *filp, char __user *buf,size_t count,loff_t *pos)
{
	int res;
	unsigned long flags;
	wait_queue_t wait;
	long timeout;

	spin_lock_irqsave(&pdev->lock, flags);

	flag = 0;
	res = 0;
	if(flag == 0)
	{
		init_waitqueue_entry(&wait, current);
		wait.flags |= WQ_FLAG_EXCLUSIVE;
		__add_wait_queue(&pdev->my_wait, &wait);
	
		for (;;) {
			
			set_current_state(TASK_INTERRUPTIBLE);

			if (signal_pending(current)) {
				res = -EINTR;
				break;
			}

			if(flag != 0)
				break;
			
			spin_unlock_irqrestore(&pdev->lock, flags);
			timeout = schedule_timeout(MAX_SCHEDULE_TIMEOUT);
			spin_lock_irqsave(&pdev->lock, flags);
		}
	}
	
	__remove_wait_queue(&pdev->my_wait, &wait);

	set_current_state(TASK_RUNNING);

	spin_unlock_irqrestore(&pdev->lock, flags);

	printk(KERN_NOTICE "process %i (%s) is awaking..\r\n",current->pid,current->comm);
	return res;
}


ssize_t waitqueue_write(struct file *filp,const char __user *buf,size_t count,loff_t *pos)
{
	unsigned long flags;
	
	printk(KERN_NOTICE "process %i (%s) awakening the readers...\r\n",
		current->pid,current->comm);

	spin_lock_irqsave(&pdev->lock, flags);

	flag = 1;
	
	__wake_up(&pdev->my_wait,TASK_NORMAL,1,NULL);

	spin_unlock_irqrestore(&pdev->lock, flags);
	return count;
}

int waitqueue_open(struct inode *inode,struct file *filp)
{
	struct waitqueue_dev *dev;
	dev = container_of(inode->i_cdev,struct waitqueue_dev,cdev);
	filp->private_data = dev;

	return 0;
}

int waitqueue_release(struct inode *inode,struct file *filp)
{
	filp->private_data = NULL;
	return 0;
}

struct file_operations waitqueue_fops = {
	.owner = THIS_MODULE,
	.open = waitqueue_open,
	.write = waitqueue_write,
	.read = waitqueue_read,
	.release = waitqueue_release,
};



static void waitqueue_setup_cdev(struct waitqueue_dev *dev,int index)
{
	int err,devno = MKDEV(wait_major,wait_minor+index);

	cdev_init(&dev->cdev,&waitqueue_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &waitqueue_fops;
	err = cdev_add(&dev->cdev,devno,1);

	if(err)
	{
		printk(KERN_NOTICE "Err %d adding waitqueue %d",err,index);
	}

	printk(KERN_NOTICE "add dev ok major %d minor %d\r\n",MAJOR(devno),MINOR(devno));
}

static int __init waitqueue_test_init(void)
{
	int result;
		
	pdev = kmalloc(sizeof(struct waitqueue_dev),GFP_KERNEL);
	if(pdev == NULL)
	{
		printk(KERN_WARNING "kmalloc pdev err\r\n");
		return -1;
	}

	spin_lock_init(&pdev->lock);
	init_waitqueue_head(&pdev->my_wait);
	
	result = alloc_chrdev_region(&wait_dev,wait_minor,wait_num,"waitqueue_test");
	wait_major = MAJOR(wait_dev);

	if(result < 0)
	{
		printk(KERN_NOTICE "waitqueue can't get a major %d\r\n",wait_major);
		return result;
	}

	waitqueue_setup_cdev(pdev,0);
	printk(KERN_NOTICE "waitqueue alloc a major %d\r\n",wait_major);

	return result;
}

static void __exit waitqueue_test_exit(void)
{
	unregister_chrdev_region(wait_dev,wait_num);
	cdev_del(&pdev->cdev);
	printk(KERN_NOTICE "waitqueue exit %d\r\n",wait_num);
}



module_init(waitqueue_test_init);
module_exit(waitqueue_test_exit);
MODULE_LICENSE("GPL");

