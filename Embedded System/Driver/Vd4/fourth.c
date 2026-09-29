/* fourth.c - fourth device driver code */
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>

struct char_driver {
    dev_t major_num;
    struct class *class_num;
    struct device *dev_num;
} c_drv;

static int __init fourth_init(void) /* Constructor */
{
    int ret;
    
    /* 1. Cap phat dong device number */
    if ((ret = alloc_chrdev_region(&c_drv.major_num, 0, 1, "CharSample")) < 0)
    {
        printk("Failed to register device number dynamically \n");
        return -1;
    }
    printk("Allocated device number (%d,%d)\n", MAJOR(c_drv.major_num), MINOR(c_drv.major_num));

    /* 2. Tao lop thiet bi (device class) */
    if (IS_ERR(c_drv.class_num = class_create("CharClass")))
    {
        printk("Failed to create device class\n");
        unregister_chrdev_region(c_drv.major_num, 1);
        return -1;
    }

    /* 3. Tao file thiet bi trong /dev (device file) */
    if (IS_ERR(c_drv.dev_num = device_create(c_drv.class_num, NULL, c_drv.major_num, NULL, "CharDevice")))
    {
        printk("Failed to create a device \n");
        class_destroy(c_drv.class_num);
        unregister_chrdev_region(c_drv.major_num, 1);
        return -1;
    }

    printk("Initialize char driver successfully\n");
    return 0;
}

static void __exit fourth_exit(void) /* Destructor */
{
    device_destroy(c_drv.class_num, c_drv.major_num);
    class_destroy(c_drv.class_num);
    unregister_chrdev_region(c_drv.major_num, 1);
}

module_init(fourth_init);
module_exit(fourth_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Seiba");
MODULE_DESCRIPTION("Create a device file");