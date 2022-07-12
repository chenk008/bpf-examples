#include <linux/module.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/kthread.h>

typedef struct kthread_arg{
    struct mutex my_mutex1; /* shared between the threads */
    struct mutex my_mutex2; /* shared between the threads */
} kthread_arg;


static struct task_struct *myThread1 = NULL;
static struct task_struct *myThread2 = NULL;


static int MyPrintk1(void *data)
{
    struct kthread_arg *test = (struct kthread_arg*)data;
    mutex_lock(&test->my_mutex1);
    msleep(1000);
    mutex_lock(&test->my_mutex2);
    return 0;
}

static int MyPrintk2(void *data)
{
    struct kthread_arg *test = (struct kthread_arg*)data;
    mutex_lock(&test->my_mutex2);
    msleep(1000);
    mutex_lock(&test->my_mutex1);
    return 0;
}

static int __init d_init(void)
{
    struct kthread_arg *test = (struct kthread_arg*)kmalloc(sizeof(struct kthread_arg),GFP_KERNEL);
    mutex_init(&test->my_mutex1); /* called only ONCE */
    mutex_init(&test->my_mutex2); /* called only ONCE */

    myThread1 = kthread_run(MyPrintk1, test, "mythread1");
    myThread2 = kthread_run(MyPrintk2, test, "mythread2");

    return 0;
}

static void __exit d_exit(void)
{

}

module_init(d_init);
module_exit(d_exit);
MODULE_LICENSE("GPL");