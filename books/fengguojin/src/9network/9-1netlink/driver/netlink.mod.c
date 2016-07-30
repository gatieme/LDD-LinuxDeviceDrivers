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
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__attribute_used__
__attribute__((section("__versions"))) = {
	{ 0xedf88b1, "struct_module" },
	{ 0x5890678, "sock_release" },
	{ 0xfa26f6b6, "init_net" },
	{ 0xa637dc71, "netlink_kernel_create" },
	{ 0xdce1060d, "netlink_unicast" },
	{ 0x97255bdf, "strlen" },
	{ 0xd875b90c, "skb_over_panic" },
	{ 0xb3eaedbf, "__alloc_skb" },
	{ 0x55502372, "kfree_skb" },
	{ 0xdd132261, "printk" },
	{ 0x9d669763, "memcpy" },
	{ 0xfa2a45e, "__memzero" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "4F69F608D1CE8D06BC4CF52");
