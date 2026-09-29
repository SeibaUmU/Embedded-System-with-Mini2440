#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
static int __init first_init(void)
{
    printk(KERN_INFO "Hello, World!\n");
    return 0;
}
static void __exit first_exit(void)
{
    printk(KERN_INFO "Goodbye, World!\n");
}
module_init(first_init);
module_exit(first_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A simple Hello World kernel module");
MODULE_AUTHOR("Seiba");