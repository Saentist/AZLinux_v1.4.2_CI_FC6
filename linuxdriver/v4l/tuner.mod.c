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
	{ 0xf9a482f9, "msleep" },
	{ 0x89b301d4, "param_get_int" },
	{ 0x8193dee7, "i2c_detach_client" },
	{ 0x1e42dfb, "i2c_del_driver" },
	{ 0xe1b7029c, "print_tainted" },
	{ 0x947430b8, "malloc_sizes" },
	{ 0x5e6ead16, "i2c_transfer" },
	{ 0x806d5133, "param_array_get" },
	{ 0x6483655c, "param_get_short" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x98bd6f46, "param_set_int" },
	{ 0x89cef6fb, "param_array_set" },
	{ 0x7d11c268, "jiffies" },
	{ 0x4f2322a9, "i2c_clients_command" },
	{ 0x1b7d4074, "printk" },
	{ 0x73e20c1c, "strlcpy" },
	{ 0x5ebefe4b, "v4l_printk_ioctl" },
	{ 0x766576d5, "i2c_register_driver" },
	{ 0x6b2dc060, "dump_stack" },
	{ 0x683a3221, "param_set_copystring" },
	{ 0xcc5005fe, "msleep_interruptible" },
	{ 0x92ffa9f5, "i2c_master_recv" },
	{ 0x9801ed3d, "kmem_cache_zalloc" },
	{ 0x37a0cba, "kfree" },
	{ 0x2e60bace, "memcpy" },
	{ 0x57a9f882, "i2c_probe" },
	{ 0x40046949, "param_set_short" },
	{ 0x93304684, "param_get_string" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=i2c-core,v4l2-common";


MODULE_INFO(srcversion, "339887ED128B76E8DDB65F2");
