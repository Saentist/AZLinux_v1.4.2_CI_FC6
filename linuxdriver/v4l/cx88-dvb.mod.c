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
	{ 0x75febb1, "cx8802_resume_common" },
	{ 0xe160b085, "cx8802_suspend_common" },
	{ 0x89b301d4, "param_get_int" },
	{ 0x98bd6f46, "param_set_int" },
	{ 0x922b8b5, "__pci_register_driver" },
	{ 0x4ace7662, "dvb_pll_lg_tdvs_h06xf" },
	{ 0x5e6ead16, "i2c_transfer" },
	{ 0xa97b832a, "dvb_pll_configure" },
	{ 0x27ca467, "videobuf_dvb_register" },
	{ 0xb9577bdd, "cx88_call_i2c_clients" },
	{ 0x9da7a65d, "mb86a16_attach" },
	{ 0x8e515cb3, "isl6421_attach" },
	{ 0xf43f1610, "cx24123_attach" },
	{ 0xfe31062a, "dvb_pll_tuv1236d" },
	{ 0xa31e68a3, "nxt200x_attach" },
	{ 0x8648e90, "lgdt330x_attach" },
	{ 0xfc7476fb, "dvb_pll_microtune_4042" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x6b2dc060, "dump_stack" },
	{ 0xe1b7029c, "print_tainted" },
	{ 0xfc5ecebb, "dvb_pll_thomson_dtt761x" },
	{ 0xa7f2f49c, "or51132_attach" },
	{ 0x55908f33, "dvb_pll_thomson_fe6600" },
	{ 0xa960d62f, "dvb_pll_unknown_1" },
	{ 0x865e1b56, "dvb_pll_lg_z201" },
	{ 0x7546c590, "zl10353_attach" },
	{ 0x64318459, "mt352_attach" },
	{ 0xc0eb63d0, "dvb_pll_fmd1216me" },
	{ 0xc93002dc, "dvb_pll_attach" },
	{ 0x30463a3, "dvb_pll_thomson_dtt7579" },
	{ 0xe0f830ed, "dvb_pll_thomson_dtt759x" },
	{ 0xde7d0f42, "cx22702_attach" },
	{ 0x99e9e305, "videobuf_queue_init" },
	{ 0x1b7d4074, "printk" },
	{ 0x1182453, "cx8802_init_common" },
	{ 0x9801ed3d, "kmem_cache_zalloc" },
	{ 0x947430b8, "malloc_sizes" },
	{ 0x4e6954bf, "cx88_boards" },
	{ 0xb43ca858, "cx88_core_get" },
	{ 0xd2b19f66, "cx8802_buf_prepare" },
	{ 0x930dae30, "cx8802_buf_queue" },
	{ 0x333c960a, "cx88_free_buffer" },
	{ 0x37a0cba, "kfree" },
	{ 0x742ef39b, "cx88_core_put" },
	{ 0x5298a2d7, "cx8802_fini_common" },
	{ 0xadcb1869, "videobuf_dvb_unregister" },
	{ 0x9c5a304a, "pci_unregister_driver" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=cx8802,dvb-pll,i2c-core,video-buf-dvb,cx88xx,mb86a16,isl6421,cx24123,nxt200x,lgdt330x,or51132,zl10353,mt352,cx22702,video-buf";

MODULE_ALIAS("pci:v000014F1d00008802sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "898855347B5E1155F577633");
