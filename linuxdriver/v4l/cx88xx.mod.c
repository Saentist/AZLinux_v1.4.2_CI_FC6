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
	{ 0x98e0cf13, "i2c_master_send" },
	{ 0x4c3af445, "__request_region" },
	{ 0xb6fa3d07, "pci_bus_read_config_byte" },
	{ 0xf9a482f9, "msleep" },
	{ 0x6d6511e7, "ir_dump_samples" },
	{ 0x2456e513, "ir_decode_pulsedistance" },
	{ 0x89b301d4, "param_get_int" },
	{ 0x9c1eed03, "ir_input_keydown" },
	{ 0x645f004, "del_timer" },
	{ 0x89cc1189, "ir_codes_winfast" },
	{ 0xdc3eaf70, "iomem_resource" },
	{ 0x933d0bb3, "ir_codes_msi_tvanywhere" },
	{ 0xc8b57c27, "autoremove_wake_function" },
	{ 0xfd8086a4, "video_device_release" },
	{ 0x6f334029, "tveeprom_hauppauge_analog" },
	{ 0x947430b8, "malloc_sizes" },
	{ 0x6aefdbea, "ir_codes_npgtech" },
	{ 0x5e6ead16, "i2c_transfer" },
	{ 0xf6a5a6c8, "schedule_work" },
	{ 0x806d5133, "param_array_get" },
	{ 0x902a3cd2, "ir_codes_hauppauge_new" },
	{ 0x8f1cc996, "mutex_unlock" },
	{ 0x98bd6f46, "param_set_int" },
	{ 0x1d26aa98, "sprintf" },
	{ 0x89cef6fb, "param_array_set" },
	{ 0x7d11c268, "jiffies" },
	{ 0x343a1a8, "__list_add" },
	{ 0xd0d444e8, "i2c_add_adapter" },
	{ 0xd92ec7c1, "ir_input_nokey" },
	{ 0x4f2322a9, "i2c_clients_command" },
	{ 0xdfcf23df, "ir_codes_norwood" },
	{ 0x2a4852cc, "ir_codes_dntv_live_dvb_t" },
	{ 0x8f566355, "video_device_alloc" },
	{ 0xa9d30d69, "__mutex_init" },
	{ 0x1b7d4074, "printk" },
	{ 0x85d37490, "ir_codes_dntv_live_dvbt_pro" },
	{ 0xef1f47c1, "ir_input_init" },
	{ 0x43912bb8, "i2c_bit_del_bus" },
	{ 0x73e20c1c, "strlcpy" },
	{ 0x840813de, "mutex_lock" },
	{ 0x521445b, "list_del" },
	{ 0x43c89ef4, "ir_decode_biphase" },
	{ 0x4743fb9e, "mod_timer" },
	{ 0x26dd9286, "videobuf_dma_unmap" },
	{ 0x200649b7, "btcx_riscmem_free" },
	{ 0xea6299e7, "tveeprom_read" },
	{ 0x9eac042a, "__ioremap" },
	{ 0xf96c0c08, "btcx_riscmem_alloc" },
	{ 0x1cb148f5, "ir_extract_bits" },
	{ 0xdaa041ad, "ir_codes_cinergy_1400" },
	{ 0x83661f9, "ir_codes_avertv_303" },
	{ 0x3449b8a6, "input_register_device" },
	{ 0x17d59d01, "schedule_timeout" },
	{ 0xfa177653, "ir_codes_pixelview" },
	{ 0x6e9c8e50, "input_free_device" },
	{ 0x8bb33e7d, "__release_region" },
	{ 0xcc5005fe, "msleep_interruptible" },
	{ 0x92ffa9f5, "i2c_master_recv" },
	{ 0xc80dc866, "init_timer" },
	{ 0x6b87c69d, "ir_codes_iodata_bctv7e" },
	{ 0x6804a02c, "__wake_up" },
	{ 0xd2965f6f, "kthread_should_stop" },
	{ 0x9801ed3d, "kmem_cache_zalloc" },
	{ 0xdc14eda7, "pci_pci_problems" },
	{ 0x72270e35, "do_gettimeofday" },
	{ 0x6e02bbff, "pci_bus_write_config_byte" },
	{ 0xd594bca1, "videobuf_waiton" },
	{ 0x37a0cba, "kfree" },
	{ 0x2e60bace, "memcpy" },
	{ 0xedc91d5d, "input_unregister_device" },
	{ 0x801678, "flush_scheduled_work" },
	{ 0x3da7af2e, "prepare_to_wait" },
	{ 0xedc03953, "iounmap" },
	{ 0x95a59f20, "finish_wait" },
	{ 0x87479210, "videobuf_dma_free" },
	{ 0x25da070, "snprintf" },
	{ 0x6e2a1870, "ir_codes_adstech_dvb_t_pci" },
	{ 0x71fddbef, "input_allocate_device" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=i2c-core,ir-common,videodev,tveeprom,ir-common,i2c-algo-bit,video-buf,btcx-risc";


MODULE_INFO(srcversion, "D08FD347D0448DB73462B98");
