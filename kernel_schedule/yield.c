#include <linux/module.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/kthread.h>

MODULE_LICENSE("GPL");

int thread_func(void *argc)
{
	printk("in the kernel thread function!\n");
	printk("the current pid is:%d\n", current->pid);
	printk("out the kernel thread function\n");

	return 0;
}

static int __init yield_init(void)
{
	struct task_struct *result;
	int wake;
	char namefrm[] = "yield.c";
	printk("into yield init.\n");
	result = kthread_create_on_node(thread_func, NULL, -1, namefrm);
	wake = wake_up_process(result);
	printk("wake up ret:%d\n",wake);
	yield();
	printk("the pid of new thread is:%d\n",result->pid);
	printk("the current pid is:%d\n",current->pid);
	printk("out yield_init.\n");

	return 0;
}

static void __exit yield_exit(void)
{
	printk("<0>Goodbye yield\n");
}

module_init(yield_init);
module_exit(yield_exit);
