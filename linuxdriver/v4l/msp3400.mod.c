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
	{ 0x98e0cf13, "i2c_master_send" },
	{ 0xde5c6f50, "i2c_attach_client" },
	{ 0x89b301d4, "param_get_int" },
	{ 0x8193dee7, "i2c_detach_client" },
	{ 0x1e42dfb, "i2c_del_driver" },
	{ 0x947430b8, "malloc_sizes" },
	{ 0xb6825920, "remove_wait_queue" },
	{ 0x5e6ead16, "i2c_transfer" },
	{ 0x806d5133, "param_array_get" },
	{ 0x6483655c, "param_get_short" },
	{ 0x98bd6f46, "param_set_int" },
	{ 0x89cef6fb, "param_array_set" },
	{ 0xffd5a395, "default_wake_function" },
	{ 0x1b7d4074, "printk" },
	{ 0xea80542d, "kthread_stop" },
	{ 0xdc76cbcb, "param_set_bool" },
	{ 0x5ebefe4b, "v4l_printk_ioctl" },
	{ 0x766576d5, "i2c_register_driver" },
	{ 0xb6dcbd1c, "kmem_cache_alloc" },
	{ 0x17d59d01, "schedule_timeout" },
	{ 0x4292364c, "schedule" },
	{ 0x35c2ba9e, "refrigerator" },
	{ 0xba9c0d03, "wake_up_process" },
	{ 0x6b185be2, "init_waitqueue_head" },
	{ 0x6804a02c, "__wake_up" },
	{ 0xd2965f6f, "kthread_should_stop" },
	{ 0xd58c141c, "add_wait_queue" },
	{ 0x9c55cec, "schedule_timeout_interruptible" },
	{ 0x37a0cba, "kfree" },
	{ 0x27eef6d7, "kthread_create" },
	{ 0x57a9f882, "i2c_probe" },
	{ 0x40046949, "param_set_short" },
	{ 0x5611e4ec, "param_get_bool" },
	{ 0x25da070, "snprintf" },
	{ 0x5fef1b4a, "v4l2_ctrl_query_fill_std" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=i2c-core,v4l2-common";


MODULE_INFO(srcversion, "1261DA371AFC2A48D40FC72");
