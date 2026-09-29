/* second.c – second device driver code */
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/fs.h>

struct char_driver{
   dev_t major_num;
}c_drv;

static int __init second_init(void) /* Constructor */
{
    int ret;
    if ((ret = alloc_chrdev_region(&c_drv.major_num, 0, 1, "CharSample")) < 0)
    {
	printk("Failed to register device number dynamically \n");
        return -1;
    }
    printk("Allocated device number (%d,%d)\n",MAJOR(c_drv.major_num),MINOR(c_drv.major_num));   
    return 0;
}

static void __exit second_exit(void) /* Destructor */
{
    unregister_chrdev_region(c_drv.major_num, 1);
}

module_init(second_init);
module_exit(second_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Phan Tuan Anh");/* tac gia cua module */
MODULE_DESCRIPTION("The simplest character device driver");/* mo ta chuc nang cua module */
