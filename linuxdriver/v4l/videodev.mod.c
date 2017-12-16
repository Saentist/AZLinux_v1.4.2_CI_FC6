#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
};

static const struct modversion_info ____versions[]
__attribute_used__
__attribute__((section("__versions"))) = {
	{ 0x30c5d25f, "struct_module" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0x55899364, "class_register" },
	{ 0x947430b8, "malloc_sizes" },
	{ 0x14453852, "no_llseek" },
	{ 0x32e0a26a, "class_device_unregister" },
	{ 0x8f1cc996, "mutex_unlock" },
	{ 0x1d26aa98, "sprintf" },
	{ 0xa9d30d69, "__mutex_init" },
	{ 0x1b7d4074, "printk" },
	{ 0x288b6948, "class_unregister" },
	{ 0x1075bf0, "panic" },
	{ 0x2da418b5, "copy_to_user" },
	{ 0xc7cf206a, "class_device_create_file" },
	{ 0x840813de, "mutex_lock" },
	{ 0x5ebefe4b, "v4l_printk_ioctl" },
	{ 0x33b3b664, "v4l2_video_std_construct" },
	{ 0x43d5b2dd, "register_chrdev" },
	{ 0x6e6de2b8, "class_device_register" },
	{ 0xba9c0d03, "wake_up_process" },
	{ 0x9801ed3d, "kmem_cache_zalloc" },
	{ 0x37a0cba, "kfree" },
	{ 0x7f83e063, "v4l_compat_translate_ioctl" },
	{ 0x98adfde2, "request_module" },
	{ 0xc192d491, "unregister_chrdev" },
	{ 0xf2a644fb, "copy_from_user" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=v4l2-common,v4l1-compat";


MODULE_INFO(srcversion, "E5BD8730777AF2D339E8E4E");
