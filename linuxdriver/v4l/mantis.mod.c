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
	{ 0x4c3af445, "__request_region" },
	{ 0xb6fa3d07, "pci_bus_read_config_byte" },
	{ 0xf9a482f9, "msleep" },
	{ 0x89b301d4, "param_get_int" },
	{ 0xb2c5ba57, "dvb_dmx_init" },
	{ 0xdcc4e157, "dvb_unregister_adapter" },
	{ 0xdc3eaf70, "iomem_resource" },
	{ 0xc8b57c27, "autoremove_wake_function" },
	{ 0xd4ab9ce4, "stv0299_attach" },
	{ 0x947430b8, "malloc_sizes" },
	{ 0xa258115e, "dvb_register_frontend" },
	{ 0xd0539fe6, "pci_disable_device" },
	{ 0x5e6ead16, "i2c_transfer" },
	{ 0xc997038f, "stb0899_attach" },
	{ 0x27df3d20, "dvb_unregister_frontend" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x1db2fe53, "pci_release_regions" },
	{ 0x7546c590, "zl10353_attach" },
	{ 0xcb6beb40, "hweight32" },
	{ 0x98bd6f46, "param_set_int" },
	{ 0x66d5d710, "dvb_net_release" },
	{ 0xd0d444e8, "i2c_add_adapter" },
	{ 0xabe31fbd, "pci_set_master" },
	{ 0x68b85bae, "dvb_dmxdev_release" },
	{ 0x1b7d4074, "printk" },
	{ 0x97b390cb, "dvb_dmx_swfilter" },
	{ 0xf6ad507f, "dvb_net_init" },
	{ 0x50c55f94, "mantis_ca_attach" },
	{ 0xed5c73bf, "__tasklet_schedule" },
	{ 0x571a5921, "dma_free_coherent" },
	{ 0x81ffcefb, "dvb_pll_env57h1xd5" },
	{ 0xa5808bbf, "tasklet_init" },
	{ 0x9eac042a, "__ioremap" },
	{ 0xf8c7eaf8, "dvb_dmx_release" },
	{ 0x9ea1725c, "dma_alloc_coherent" },
	{ 0x348ec090, "dvb_dmx_swfilter_204" },
	{ 0x79ad224b, "tasklet_kill" },
	{ 0x7dbe2dd0, "i2c_del_adapter" },
	{ 0xb6dcbd1c, "kmem_cache_alloc" },
	{ 0x26e96637, "request_irq" },
	{ 0x17d59d01, "schedule_timeout" },
	{ 0x8bb33e7d, "__release_region" },
	{ 0x2f446844, "dvb_register_adapter" },
	{ 0x9c5a304a, "pci_unregister_driver" },
	{ 0xcc5005fe, "msleep_interruptible" },
	{ 0x6b185be2, "init_waitqueue_head" },
	{ 0xc93002dc, "dvb_pll_attach" },
	{ 0x6804a02c, "__wake_up" },
	{ 0x2a4bd382, "cu1216_attach" },
	{ 0x6e02bbff, "pci_bus_write_config_byte" },
	{ 0x37a0cba, "kfree" },
	{ 0x3da7af2e, "prepare_to_wait" },
	{ 0xedc03953, "iounmap" },
	{ 0x922b8b5, "__pci_register_driver" },
	{ 0x95a59f20, "finish_wait" },
	{ 0x6b5ad44f, "mantis_ca_detach" },
	{ 0x9da7a65d, "mb86a16_attach" },
	{ 0x810897a4, "pci_enable_device" },
	{ 0xf20dabd8, "free_irq" },
	{ 0xa7def023, "dvb_dmxdev_init" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=dvb-core,stv0299,i2c-core,stb0899,zl10353,mantis_ci,dvb-pll,cu1216,mb86a16";

MODULE_ALIAS("pci:v00001822d00004E35sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "FC8384D3F1A95B3C4CB6B18");
