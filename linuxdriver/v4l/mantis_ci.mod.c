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
	{ 0x52438774, "__mod_timer" },
	{ 0xf9a482f9, "msleep" },
	{ 0xc8b57c27, "autoremove_wake_function" },
	{ 0x947430b8, "malloc_sizes" },
	{ 0x2b26fe7f, "dvb_register_device" },
	{ 0xc88c7ada, "_spin_lock" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x1d26aa98, "sprintf" },
	{ 0x7d11c268, "jiffies" },
	{ 0xc3fe6db4, "__spin_lock_init" },
	{ 0xf0d93747, "del_timer_sync" },
	{ 0x1b7d4074, "printk" },
	{ 0x5152e605, "memcmp" },
	{ 0x2da418b5, "copy_to_user" },
	{ 0x685647ba, "dvb_unregister_device" },
	{ 0x69900829, "_spin_unlock" },
	{ 0xb6dcbd1c, "kmem_cache_alloc" },
	{ 0x4292364c, "schedule" },
	{ 0x17d59d01, "schedule_timeout" },
	{ 0xba9c0d03, "wake_up_process" },
	{ 0x6b185be2, "init_waitqueue_head" },
	{ 0xc80dc866, "init_timer" },
	{ 0x6804a02c, "__wake_up" },
	{ 0xd2965f6f, "kthread_should_stop" },
	{ 0x37a0cba, "kfree" },
	{ 0x27eef6d7, "kthread_create" },
	{ 0x2e60bace, "memcpy" },
	{ 0x3da7af2e, "prepare_to_wait" },
	{ 0x95a59f20, "finish_wait" },
	{ 0xf2a644fb, "copy_from_user" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=dvb-core";


MODULE_INFO(srcversion, "3FCC928757B1E9C1F4533D4");
