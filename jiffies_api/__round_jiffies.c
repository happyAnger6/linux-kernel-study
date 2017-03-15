#include <linux/module.h>
#include <linux/timer.h>

MODULE_LICENSE("GPL");

int __init __round_jiffies_init(void)
{
	printk("the _round_jiffies test begin\n");

	unsigned long j = jiffies;
	unsigned long _result1 = __round_jiffies(j,0);
	unsigned long _result2 = __round_jiffies(j,1);
	unsigned long _result3 = __round_jiffies(j,2);
	unsigned long _result4 = __round_jiffies(j,3);

	printk("the jiffies is :%ld\n", j);

	
	printk("the _result1 is :%ld\n", _result1);
	printk("the _result2 is :%ld\n", _result2);
	printk("the _result3 is :%ld\n", _result3);
	printk("the _result4 is :%ld\n", _result4);

	return 0;
}

void __exit __round_jiffies_exit(void)
{
	printk("goodbye __round_jiffies\n");
}

module_init(__round_jiffies_init);
module_exit(__round_jiffies_exit);
