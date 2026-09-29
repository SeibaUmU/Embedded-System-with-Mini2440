#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/version.h>
#include<linux/fs.h>

struct char_driver{
    dev_t major_num;
}c_drv;

static int __init third_init(void)
{
    int ret;
    if((ret = alloc_chrdev_region(&c_drv.major_num, 0, 1, "CharSample")) < 0){
        printk(KERN_ALERT "Failed to register number dynamically\n");
        return -1;
    }
    printk(KERN_INFO "Allocated device number (%d,%d)\n", MAJOR(c_drv.major_num), MINOR(c_drv.major_num));
    return 0;
}

static void __exit third_exit(void)
{
    unregister_chrdev_region(c_drv.major_num, 1);
}
module_init(third_init);
module_exit(third_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Seiba");
MODULE_DESCRIPTION("Allocate device number dynamically");