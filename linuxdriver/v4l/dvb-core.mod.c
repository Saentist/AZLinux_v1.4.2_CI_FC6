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
	{ 0x94ebe896, "cdev_del" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0x52438774, "__mod_timer" },
	{ 0x1b8b046b, "cdev_init" },
	{ 0xf9a482f9, "msleep" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0x89b301d4, "param_get_int" },
	{ 0x645f004, "del_timer" },
	{ 0xd8e484f0, "register_chrdev_region" },
	{ 0xe1b7029c, "print_tainted" },
	{ 0xc8b57c27, "autoremove_wake_function" },
	{ 0x9c4a0495, "class_device_destroy" },
	{ 0xac795081, "class_device_create" },
	{ 0x947430b8, "malloc_sizes" },
	{ 0xc88c7ada, "_spin_lock" },
	{ 0xf6a5a6c8, "schedule_work" },
	{ 0xba154599, "alloc_netdev" },
	{ 0x8f1cc996, "mutex_unlock" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0x2fd1d81c, "vfree" },
	{ 0x98bd6f46, "param_set_int" },
	{ 0x9d75ed43, "_spin_lock_irqsave" },
	{ 0x7d11c268, "jiffies" },
	{ 0x343a1a8, "__list_add" },
	{ 0xd533bec7, "__might_sleep" },
	{ 0xc3fe6db4, "__spin_lock_init" },
	{ 0xda4008e6, "cond_resched" },
	{ 0x4edaae97, "netif_rx" },
	{ 0x12f237eb, "__kzalloc" },
	{ 0x5ef2e6ff, "mutex_lock_interruptible" },
	{ 0xa9d30d69, "__mutex_init" },
	{ 0x1b7d4074, "printk" },
	{ 0xffd293ad, "_spin_lock_irq" },
	{ 0x5152e605, "memcmp" },
	{ 0x5568be43, "lock_kernel" },
	{ 0x4a814c5f, "free_netdev" },
	{ 0x2da418b5, "copy_to_user" },
	{ 0x602a308b, "register_netdev" },
	{ 0x840813de, "mutex_lock" },
	{ 0x521445b, "list_del" },
	{ 0x625acc81, "__down_failed_interruptible" },
	{ 0xa00f0fa6, "class_create" },
	{ 0xe49e22e1, "_spin_unlock_irqrestore" },
	{ 0x69900829, "_spin_unlock" },
	{ 0x796deb, "cdev_add" },
	{ 0x5dfe8f1a, "unlock_kernel" },
	{ 0xf4d17791, "skb_over_panic" },
	{ 0x7dceceac, "capable" },
	{ 0xb6dcbd1c, "kmem_cache_alloc" },
	{ 0x9d1bc11c, "__alloc_skb" },
	{ 0x4292364c, "schedule" },
	{ 0x17d59d01, "schedule_timeout" },
	{ 0x35c2ba9e, "refrigerator" },
	{ 0x7ea2c004, "kfree_skb" },
	{ 0x6b2dc060, "dump_stack" },
	{ 0x158e436c, "skb_under_panic" },
	{ 0xba9c0d03, "wake_up_process" },
	{ 0x80462479, "ether_setup" },
	{ 0x6b185be2, "init_waitqueue_head" },
	{ 0xc80dc866, "init_timer" },
	{ 0x6804a02c, "__wake_up" },
	{ 0x9801ed3d, "kmem_cache_zalloc" },
	{ 0x72270e35, "do_gettimeofday" },
	{ 0x618c442a, "_spin_unlock_bh" },
	{ 0x37a0cba, "kfree" },
	{ 0x932da67e, "kill_proc" },
	{ 0x801678, "flush_scheduled_work" },
	{ 0xb7b61546, "crc32_be" },
	{ 0x3da7af2e, "prepare_to_wait" },
	{ 0x46d33fe1, "list_add" },
	{ 0xdb4fb7fa, "class_destroy" },
	{ 0x95a59f20, "finish_wait" },
	{ 0x7e9ebb05, "kernel_thread" },
	{ 0x60a4461c, "__up_wakeup" },
	{ 0x2cb6e470, "unregister_netdev" },
	{ 0x25da070, "snprintf" },
	{ 0xdf06ce39, "_spin_lock_bh" },
	{ 0xf2a644fb, "copy_from_user" },
	{ 0x9e7d6bd0, "__udelay" },
	{ 0xdc43a9c8, "daemonize" },
	{ 0x7bd258f9, "_spin_unlock_irq" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "DE2A8E11504052AE76823EF");
