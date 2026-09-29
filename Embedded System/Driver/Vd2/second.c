#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/version.h>
#include<linux/fs.h>

struct char_driver{
    dev_t major_num;
}c_drv;

static int __init second_init(void)
{   
    int ret;
    c_drv.major_num = MKDEV(123,0);
    ret=register_chrdev_region(c_drv.major_num,1,"CharSample");
    if(ret<0){
        printk(KERN_ERR "Failed to register device number\n");
        return -1;
    }
    printk(KERN_INFO "Initialize char driver successfully\n");
    return 0;
}
static void __exit second_exit(void)
{
    unregister_chrdev_region(c_drv.major_num,1);
}
module_init(second_init);
module_exit(second_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Allocate device number statically");
MODULE_AUTHOR("Seiba");