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
	{ 0x9b8bb7ed, "cx88_risc_databuffer" },
	{ 0xb6fa3d07, "pci_bus_read_config_byte" },
	{ 0x89b301d4, "param_get_int" },
	{ 0x645f004, "del_timer" },
	{ 0xd0539fe6, "pci_disable_device" },
	{ 0xc88c7ada, "_spin_lock" },
	{ 0x4e6954bf, "cx88_boards" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x333c960a, "cx88_free_buffer" },
	{ 0x17f856f7, "videobuf_iolock" },
	{ 0x98bd6f46, "param_set_int" },
	{ 0x9b35a4ad, "cx88_sram_channel_setup" },
	{ 0x9d75ed43, "_spin_lock_irqsave" },
	{ 0x7d11c268, "jiffies" },
	{ 0x343a1a8, "__list_add" },
	{ 0x3b9a2aca, "cx88_sram_channel_dump" },
	{ 0xc3fe6db4, "__spin_lock_init" },
	{ 0xbb5cc064, "cx88_ir_init" },
	{ 0x9b140fff, "cx88_sram_channels" },
	{ 0xabe31fbd, "pci_set_master" },
	{ 0xf0d93747, "del_timer_sync" },
	{ 0x358af268, "pci_restore_state" },
	{ 0xabd0efaa, "cx88_card_setup" },
	{ 0x1b7d4074, "printk" },
	{ 0x521445b, "list_del" },
	{ 0xe49e22e1, "_spin_unlock_irqrestore" },
	{ 0x4743fb9e, "mod_timer" },
	{ 0x200649b7, "btcx_riscmem_free" },
	{ 0x69900829, "_spin_unlock" },
	{ 0x26e96637, "request_irq" },
	{ 0xc4339359, "cx88_core_irq" },
	{ 0x6b185be2, "init_waitqueue_head" },
	{ 0xc80dc866, "init_timer" },
	{ 0x6804a02c, "__wake_up" },
	{ 0xfc838e67, "pci_set_power_state" },
	{ 0x31f3e975, "cx88_i2c_init" },
	{ 0xbdf48e5a, "cx88_shutdown" },
	{ 0x777b6d7f, "cx88_print_irqbits" },
	{ 0x1a521407, "pci_choose_state" },
	{ 0xb41837f1, "cx88_risc_stopper" },
	{ 0x810897a4, "pci_enable_device" },
	{ 0x4ba2381f, "cx88_wakeup" },
	{ 0xb9577bdd, "cx88_call_i2c_clients" },
	{ 0x68f34377, "cx88_reset" },
	{ 0xf20dabd8, "free_irq" },
	{ 0x6c86ca0f, "pci_save_state" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=cx88xx,video-buf,cx88xx,btcx-risc";


MODULE_INFO(srcversion, "7E3E4FA1F67757C00344929");
