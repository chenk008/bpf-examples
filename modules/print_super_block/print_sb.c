#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/kdev_t.h>
#include <linux/kallsyms.h> //方法二


// 方法二，使用kallsyms_lookup_name()查找函数或变量的虚拟地址
// 使用时，需要注释其它方法的代码，取消此处及下面方法二的注释
spinlock_t * sb_lock_address;
struct list_head * super_blocks_address;


/*
 *方法三，内核模块中直接使用内核函数的虚拟地址
 *使用时，需要注释其他方法的代码，取消此处及下面方法三的注释
  #define SUPER_BLOCKS_ADDRESS 0xffffffff91d2efe0
  #define SB_LOCK_ADDRESS 0xffffffff922f35d4
 */

static int __init my_init(void)
{
    struct super_block *sb;
    struct list_head *pos;
    struct list_head *linode;
    struct inode *pinode;
    unsigned long long count = 0;

    printk("\nPrint some fields of super_blocks:\n");

    
    //方法二
    sb_lock_address = (spinlock_t *)kallsyms_lookup_name("sb_lock");
    super_blocks_address = (struct list_head *)kallsyms_lookup_name("super_blocks");
    spin_lock(sb_lock_address);
    list_for_each(pos, super_blocks_address) {
    

    /*
     *方法三
     spin_lock((spinlock_t *)SB_LOCK_ADDRESS);
     list_for_each(pos, (struct list_head *)SUPER_BLOCKS_ADDRESS) {
     */

    // 没有被 EXPORT_SYMBOL 相关的宏导出的变量或函数是不能直接使用的
    
    /*此处使用了未导出变量，若使用方法二或方法三时需要注释以下两行*/
    // spin_lock(&sb_lock); 
    // list_for_each(pos, &super_blocks) {

        // 获得当前超级块的地址
        sb = list_entry(pos, struct super_block, s_list);
        printk("dev_t:%d:%d", MAJOR(sb->s_dev), MINOR(sb->s_dev));
        //打印文件系统所在设备的主设备号和次设备号
        printk("file_type name:%s\n", sb->s_type->name);
        //打印文件系统名

        list_for_each(linode, &sb->s_inodes)
        {
            pinode = list_entry(linode, struct inode, i_sb_list);
            count++;
            printk("%lu\t", pinode->i_ino); //打印索引节点号
        }
    }
    spin_unlock(sb_lock_address);  //方法二
    // spin_unlock(SB_LOCK_ADDRESS);	 //方法三
    // spin_unlock(&sb_lock); //解锁，此处使用了未导出的变量
    printk("The number of inodes:%llu\n", sizeof(struct inode) * count);
    return 0;
}

static void __exit my_exit(void)
{
    printk("unloading…\n");
}
module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");