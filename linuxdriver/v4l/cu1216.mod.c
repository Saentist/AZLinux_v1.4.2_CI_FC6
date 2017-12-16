#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
};

static const struct modversion_info ____versions[]
__attribute_used__
__attribute__((section("__versions"))) = {
	{ 0x30c5d25f, "struct_module" },
	{ 0xf9a482f9, "msleep" },
	{ 0x89b301d4, "param_get_int" },
	{ 0x947430b8, "malloc_sizes" },
	{ 0x5e6ead16, "i2c_transfer" },
	{ 0x98bd6f46, "param_set_int" },
	{ 0x1b7d4074, "printk" },
	{ 0xb6dcbd1c, "kmem_cache_alloc" },
	{ 0x17d59d01, "schedule_timeout" },
	{ 0x37a0cba, "kfree" },
	{ 0x2e60bace, "memcpy" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=i2c-core";


MODULE_INFO(srcversion, "7D45BC61976717A163AA10E");
