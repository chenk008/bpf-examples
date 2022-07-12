#include <linux/module.h>
#include <linux/sched.h>
#include <linux/wait.h>

int condition = 1234;
module_param(condition, uint, 0644);

wait_queue_head_t waitq;

static int __init Ds1_init(void)
{
	long magic = 0x22334455667788;

	condition = 0x1234;
	printk("condition:%lu  magic:%lu   %p\n", condition, magic, &waitq);
	init_waitqueue_head (&waitq);
	// 此处没有任何人会将condition设置为123，因此insmod会一直等待，进而D住
	wait_event(waitq, condition == 123);

	return 0;
}

static void __exit Ds1_exit(void)
{
}

module_init(Ds1_init);
module_exit(Ds1_exit);
MODULE_LICENSE("GPL");