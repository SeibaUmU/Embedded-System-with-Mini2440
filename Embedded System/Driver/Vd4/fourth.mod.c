#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x122c3a7e, "_printk" },
	{ 0x67126a19, "class_create" },
	{ 0xacb3e0b8, "device_create" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x521be16f, "class_destroy" },
	{ 0xa11827eb, "device_destroy" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x5ec55c4a, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "34CD9AF54C399392CFC5FD9");
