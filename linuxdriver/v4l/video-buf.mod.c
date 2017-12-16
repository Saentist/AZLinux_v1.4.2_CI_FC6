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
	{ 0x12da5bb2, "__kmalloc" },
	{ 0x1a1037fb, "up_read" },
	{ 0xf3f513b2, "mem_map" },
	{ 0x89b301d4, "param_get_int" },
	{ 0x4705f3ec, "page_address" },
	{ 0xe1b7029c, "print_tainted" },
	{ 0x947430b8, "malloc_sizes" },
	{ 0xb6825920, "remove_wait_queue" },
	{ 0x8f1cc996, "mutex_unlock" },
	{ 0x2fd1d81c, "vfree" },
	{ 0x98bd6f46, "param_set_int" },
	{ 0x9d75ed43, "_spin_lock_irqsave" },
	{ 0x343a1a8, "__list_add" },
	{ 0x6bec90b0, "down_read" },
	{ 0x12f237eb, "__kzalloc" },
	{ 0xffd5a395, "default_wake_function" },
	{ 0xd618655a, "__alloc_pages" },
	{ 0xa9d30d69, "__mutex_init" },
	{ 0x1b7d4074, "printk" },
	{ 0x2da418b5, "copy_to_user" },
	{ 0x840813de, "mutex_lock" },
	{ 0x521445b, "list_del" },
	{ 0xe49e22e1, "_spin_unlock_irqrestore" },
	{ 0x450c4ff0, "contig_page_data" },
	{ 0x87173220, "zone_table" },
	{ 0xb6dcbd1c, "kmem_cache_alloc" },
	{ 0x8a95eeb4, "get_user_pages" },
	{ 0x4292364c, "schedule" },
	{ 0x6b2dc060, "dump_stack" },
	{ 0xa0b04675, "vmalloc_32" },
	{ 0x6b185be2, "init_waitqueue_head" },
	{ 0xd58c141c, "add_wait_queue" },
	{ 0x37a0cba, "kfree" },
	{ 0x2717ba87, "put_page" },
	{ 0x157a7470, "vmalloc_to_page" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "995C1257B997265AE772EFC");
