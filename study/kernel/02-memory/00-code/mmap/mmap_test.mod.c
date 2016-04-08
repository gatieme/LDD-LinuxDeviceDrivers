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
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0xa87cf413, __VMLINUX_SYMBOL_STR(clear_bit) },
	{ 0x15625c2c, __VMLINUX_SYMBOL_STR(misc_deregister) },
	{ 0xdc435006, __VMLINUX_SYMBOL_STR(device_create_file) },
	{ 0xae8c4d0c, __VMLINUX_SYMBOL_STR(set_bit) },
	{ 0x71603d38, __VMLINUX_SYMBOL_STR(kmem_cache_alloc) },
	{ 0x442f54a5, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x4bc5f446, __VMLINUX_SYMBOL_STR(misc_register) },
	{ 0x4813cae7, __VMLINUX_SYMBOL_STR(remap_pfn_range) },
	{ 0x41182cc0, __VMLINUX_SYMBOL_STR(pgprot_default) },
	{ 0xf8e398fc, __VMLINUX_SYMBOL_STR(memstart_addr) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

