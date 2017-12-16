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
	{ 0x7d676e5f, "videobuf_mmap_free" },
	{ 0xb6fa3d07, "pci_bus_read_config_byte" },
	{ 0xfe867ce8, "cx88_newstation" },
	{ 0xf9a482f9, "msleep" },
	{ 0x89b301d4, "param_get_int" },
	{ 0x645f004, "del_timer" },
	{ 0x3c730a35, "videobuf_mmap_mapper" },
	{ 0xfd8086a4, "video_device_release" },
	{ 0x63248060, "video_usercopy" },
	{ 0x947430b8, "malloc_sizes" },
	{ 0xd0539fe6, "pci_disable_device" },
	{ 0x14453852, "no_llseek" },
	{ 0xc88c7ada, "_spin_lock" },
	{ 0x806d5133, "param_array_get" },
	{ 0x32dc26d, "videobuf_queue_cancel" },
	{ 0xba25020a, "videobuf_streamon" },
	{ 0x4e6954bf, "cx88_boards" },
	{ 0x333c960a, "cx88_free_buffer" },
	{ 0x17f856f7, "videobuf_iolock" },
	{ 0x8f1cc996, "mutex_unlock" },
	{ 0x98bd6f46, "param_set_int" },
	{ 0x9b35a4ad, "cx88_sram_channel_setup" },
	{ 0x9d75ed43, "_spin_lock_irqsave" },
	{ 0x1d26aa98, "sprintf" },
	{ 0x89cef6fb, "param_array_set" },
	{ 0x7d11c268, "jiffies" },
	{ 0x343a1a8, "__list_add" },
	{ 0x3b9a2aca, "cx88_sram_channel_dump" },
	{ 0xc3fe6db4, "__spin_lock_init" },
	{ 0xcbec4348, "cx88_set_tvaudio" },
	{ 0xb11307b1, "video_register_device" },
	{ 0xbb5cc064, "cx88_ir_init" },
	{ 0x9b140fff, "cx88_sram_channels" },
	{ 0xabe31fbd, "pci_set_master" },
	{ 0xc579d952, "videobuf_read_one" },
	{ 0x358af268, "pci_restore_state" },
	{ 0xabd0efaa, "cx88_card_setup" },
	{ 0x1b7d4074, "printk" },
	{ 0xea80542d, "kthread_stop" },
	{ 0xa27be917, "videobuf_qbuf" },
	{ 0x637f996b, "video_unregister_device" },
	{ 0xfcc9399c, "cx88_set_stereo" },
	{ 0x95f4dfd3, "videobuf_querybuf" },
	{ 0x73e20c1c, "strlcpy" },
	{ 0x840813de, "mutex_lock" },
	{ 0x521445b, "list_del" },
	{ 0xe49e22e1, "_spin_unlock_irqrestore" },
	{ 0x30ee56ff, "videobuf_read_stream" },
	{ 0x4743fb9e, "mod_timer" },
	{ 0x200649b7, "btcx_riscmem_free" },
	{ 0x749ea3af, "cx88_risc_buffer" },
	{ 0x69900829, "_spin_unlock" },
	{ 0x5ebefe4b, "v4l_printk_ioctl" },
	{ 0x42c8e001, "v4l2_ctrl_next" },
	{ 0x57b02a20, "v4l2_type_names" },
	{ 0x7dbe2dd0, "i2c_del_adapter" },
	{ 0x1bd4aa6b, "cx88_set_tvnorm" },
	{ 0x99e9e305, "videobuf_queue_init" },
	{ 0x26e96637, "request_irq" },
	{ 0x33b3b664, "v4l2_video_std_construct" },
	{ 0xe3ee3174, "cx88_get_stereo" },
	{ 0xb43ca858, "cx88_core_get" },
	{ 0xba9c0d03, "wake_up_process" },
	{ 0x9c5a304a, "pci_unregister_driver" },
	{ 0xc4339359, "cx88_core_irq" },
	{ 0x448f8ee6, "videobuf_dqbuf" },
	{ 0x904b8696, "cx88_audio_thread" },
	{ 0x6b185be2, "init_waitqueue_head" },
	{ 0xc80dc866, "init_timer" },
	{ 0x6804a02c, "__wake_up" },
	{ 0xfc838e67, "pci_set_power_state" },
	{ 0x9801ed3d, "kmem_cache_zalloc" },
	{ 0x742ef39b, "cx88_core_put" },
	{ 0x31f3e975, "cx88_i2c_init" },
	{ 0x37a0cba, "kfree" },
	{ 0x27eef6d7, "kthread_create" },
	{ 0x7f83e063, "v4l_compat_translate_ioctl" },
	{ 0x2e60bace, "memcpy" },
	{ 0x98adfde2, "request_module" },
	{ 0xa12b5946, "cx88_vdev_init" },
	{ 0x6d3b48b7, "videobuf_reqbufs" },
	{ 0xbdf48e5a, "cx88_shutdown" },
	{ 0x922b8b5, "__pci_register_driver" },
	{ 0x39ae261e, "cx88_set_scale" },
	{ 0x777b6d7f, "cx88_print_irqbits" },
	{ 0x1a521407, "pci_choose_state" },
	{ 0xd8b01c49, "v4l_compat_ioctl32" },
	{ 0xb41837f1, "cx88_risc_stopper" },
	{ 0xe81fe794, "v4l_printk_ioctl_arg" },
	{ 0x810897a4, "pci_enable_device" },
	{ 0xf19dfa76, "videobuf_read_stop" },
	{ 0x4ba2381f, "cx88_wakeup" },
	{ 0xb9577bdd, "cx88_call_i2c_clients" },
	{ 0x9261bdbf, "videobuf_poll_stream" },
	{ 0x68f34377, "cx88_reset" },
	{ 0xf20dabd8, "free_irq" },
	{ 0x6c86ca0f, "pci_save_state" },
	{ 0x26116338, "videobuf_streamoff" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=video-buf,cx88xx,videodev,cx88xx,btcx-risc,v4l2-common,i2c-core,v4l1-compat,compat_ioctl32";

MODULE_ALIAS("pci:v000014F1d00008800sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "3A96870958D611B7906EC6B");
