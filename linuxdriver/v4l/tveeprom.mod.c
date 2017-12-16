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
	{ 0x806d5133, "param_array_get" },
	{ 0x6483655c, "param_get_short" },
	{ 0x98bd6f46, "param_set_int" },
	{ 0x89cef6fb, "param_array_set" },
	{ 0x1b7d4074, "printk" },
	{ 0x766576d5, "i2c_register_driver" },
	{ 0x92ffa9f5, "i2c_master_recv" },
	{ 0x9801ed3d, "kmem_cache_zalloc" },
	{ 0x37a0cba, "kfree" },
	{ 0x2e60bace, "memcpy" },
	{ 0x57a9f882, "i2c_probe" },
	{ 0x40046949, "param_set_short" },
	{ 0x25da070, "snprintf" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=i2c-core";


MODULE_INFO(srcversion, "63FBA2D027252E82BAD98C8");
