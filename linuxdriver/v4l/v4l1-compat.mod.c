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
	{ 0x89b301d4, "param_get_int" },
	{ 0x947430b8, "malloc_sizes" },
	{ 0x98bd6f46, "param_set_int" },
	{ 0x1b7d4074, "printk" },
	{ 0x8b52068c, "poll_freewait" },
	{ 0x63977b75, "poll_initwait" },
	{ 0x4292364c, "schedule" },
	{ 0x9801ed3d, "kmem_cache_zalloc" },
	{ 0x37a0cba, "kfree" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "0D03C82F977C63406D7310C");
