static void test_device_release(struct device * dev)
{
	printk(KERN_DEBUG"test_device release()");
}

struct device test_device=
{
	.bus_id="test_device",
	.release=test_device_release
};

int ret;
ret=device_register(&test_device);
if(ret)
	printk(KERN_DEBUG "register is error");
