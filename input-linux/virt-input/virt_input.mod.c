#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x2c635209, "module_layout" },
	{ 0x8c03d20c, "destroy_workqueue" },
	{ 0x9fa7184a, "cancel_delayed_work_sync" },
	{ 0xc6f46339, "init_timer_key" },
	{ 0xffeedf6a, "delayed_work_timer_fn" },
	{ 0x6bd13537, "input_unregister_device" },
	{ 0x49cd25ed, "alloc_workqueue" },
	{ 0x88fcdf1a, "input_free_device" },
	{ 0xb7539a1d, "input_register_device" },
	{ 0x92997ed8, "_printk" },
	{ 0x842da7ac, "input_allocate_device" },
	{ 0x54b1fac6, "__ubsan_handle_load_invalid_value" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0xb2fcb56d, "queue_delayed_work_on" },
	{ 0x5677d93c, "input_event" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "DD716FD1917C0D17C3CEC0D");
