#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x146be775, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x15625c2c, __VMLINUX_SYMBOL_STR(misc_deregister) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x4bc5f446, __VMLINUX_SYMBOL_STR(misc_register) },
	{ 0x92c17db0, __VMLINUX_SYMBOL_STR(dma_release_from_coherent) },
	{ 0xf19146ad, __VMLINUX_SYMBOL_STR(dev_err) },
	{ 0xd6b7c3a5, __VMLINUX_SYMBOL_STR(dma_ops) },
	{ 0x9134613b, __VMLINUX_SYMBOL_STR(_dev_info) },
	{ 0xc31e44bc, __VMLINUX_SYMBOL_STR(dma_alloc_from_coherent) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

