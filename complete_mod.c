#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/completion.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <asm/current.h>

static dev_t complete_dev;
static int complete_major;
static int complete_minor;
static int complete_num;

module_param(complete_minor,int,0644);
module_param(complete_num,int,0644);

struct complete_dev{
	struct completion my_completion;
	struct cdev cdev;
};

static struct complete_dev *pdev;

ssize_t complete_read(struct file *filp, char __user *buf,size_t count,loff_t *pos)
{
	printk(KERN_NOTICE "process %i (%s) goint to sleep \r\n",
		current->pid,current->comm);

	wait_for_completion(&pdev->my_completion);
	printk(KERN_NOTICE "awoken %i (%s)\r\n",current->pid,current->comm);
	return 0;
}


ssize_t complete_write(struct file *filp,const char __user *buf,size_t count,loff_t *pos)
{
	printk(KERN_NOTICE "process %i (%s) awakening the readers...\r\n",
		current->pid,current->comm);
	complete(&pdev->my_completion);
	return count;
}

int complete_open(struct inode *inode,struct file *filp)
{
	struct complete_dev *dev;
	dev = container_of(inode->i_cdev,struct complete_dev,cdev);
	filp->private_data = dev;

	return 0;
}

int complete_release(struct inode *inode,struct file *filp)
{
	filp->private_data = NULL;
	return 0;
}

struct file_operations complete_fops = {
	.owner = THIS_MODULE,
	.open = complete_open,
	.write = complete_write,
	.read = complete_read,
	.release = complete_release,
};



static void complete_setup_cdev(struct complete_dev *dev,int index)
{
	int err,devno = MKDEV(complete_major,complete_minor+index);

	cdev_init(&dev->cdev,&complete_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &complete_fops;
	err = cdev_add(&dev->cdev,devno,1);

	if(err)
	{
		printk(KERN_NOTICE "Err %d adding complete %d",err,index);
	}

	printk(KERN_NOTICE "add dev ok major %d minor %d\r\n",MAJOR(devno),MINOR(devno));
}

static int __init complete_test_init(void)
{
	int result;
		
	pdev = kmalloc(sizeof(struct complete_dev),GFP_KERNEL);
	if(pdev == NULL)
	{
		printk(KERN_WARNING "kmalloc pdev err\r\n");
		return -1;
	}

	init_completion(&pdev->my_completion);
	
	result = alloc_chrdev_region(&complete_dev,complete_minor,complete_num,"complete_test");
	complete_major = MAJOR(complete_dev);

	if(result < 0)
	{
		printk(KERN_NOTICE "complete can't get a major %d\r\n",complete_major);
		return result;
	}

	complete_setup_cdev(pdev,0);
	printk(KERN_NOTICE "complete alloc a major %d\r\n",complete_major);

	return result;
}

static void __exit complete_test_exit(void)
{
	unregister_chrdev_region(complete_dev,complete_num);
	cdev_del(&pdev->cdev);
	printk(KERN_NOTICE "complete exit %d\r\n",complete_num);
}



module_init(complete_test_init);
module_exit(complete_test_exit);
MODULE_LICENSE("GPL");
