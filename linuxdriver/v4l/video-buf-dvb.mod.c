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
	{ 0xb2c5ba57, "dvb_dmx_init" },
	{ 0xdcc4e157, "dvb_unregister_adapter" },
	{ 0xa258115e, "dvb_register_frontend" },
	{ 0x27df3d20, "dvb_unregister_frontend" },
	{ 0x8f1cc996, "mutex_unlock" },
	{ 0x98bd6f46, "param_set_int" },
	{ 0x9d75ed43, "_spin_lock_irqsave" },
	{ 0x66d5d710, "dvb_net_release" },
	{ 0x343a1a8, "__list_add" },
	{ 0xcfc367fd, "dvb_frontend_detach" },
	{ 0x68b85bae, "dvb_dmxdev_release" },
	{ 0xa9d30d69, "__mutex_init" },
	{ 0x1b7d4074, "printk" },
	{ 0xea80542d, "kthread_stop" },
	{ 0x97b390cb, "dvb_dmx_swfilter" },
	{ 0xf6ad507f, "dvb_net_init" },
	{ 0x840813de, "mutex_lock" },
	{ 0x1814192e, "videobuf_read_start" },
	{ 0x521445b, "list_del" },
	{ 0xe49e22e1, "_spin_unlock_irqrestore" },
	{ 0xf8c7eaf8, "dvb_dmx_release" },
	{ 0x4292364c, "schedule" },
	{ 0x35c2ba9e, "refrigerator" },
	{ 0xba9c0d03, "wake_up_process" },
	{ 0x2f446844, "dvb_register_adapter" },
	{ 0xd2965f6f, "kthread_should_stop" },
	{ 0xd594bca1, "videobuf_waiton" },
	{ 0x27eef6d7, "kthread_create" },
	{ 0xf19dfa76, "videobuf_read_stop" },
	{ 0xa7def023, "dvb_dmxdev_init" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=dvb-core,dvb-core,video-buf";


MODULE_INFO(srcversion, "9F5FBE7F70FF4F8DE226891");
