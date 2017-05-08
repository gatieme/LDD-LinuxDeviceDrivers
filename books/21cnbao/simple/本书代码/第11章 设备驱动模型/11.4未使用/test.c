
#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>




/*
 * The LDD bus device.
 */
static void test_dev_release(struct device *dev)
{
	printk(KERN_DEBUG "test_dev release\n");
}
	
struct device test_dev = {
	.bus_id   = "test",
	.release  = test_dev_release
};


/*
 * LDD devices.
 */

/*
 * For now, no references to LDDbus devices go out which are not
 * tracked via the module reference count, so we use a no-op
 * release function.
 */
static void ldd_dev_release(struct device *dev)
{ }

int register_ldd_device(struct ldd_device *ldddev)
{
	ldddev->dev.bus = &ldd_bus_type;
	ldddev->dev.parent = &test_dev;
	ldddev->dev.release = ldd_dev_release;
	strncpy(ldddev->dev.bus_id, ldddev->name, BUS_ID_SIZE);
	return device_register(&ldddev->dev);
}
EXPORT_SYMBOL(register_ldd_device);

void unregister_ldd_device(struct ldd_device *ldddev)
{
	device_unregister(&ldddev->dev);
}
EXPORT_SYMBOL(unregister_ldd_device);

/*
 * Crude driver interface.
 */


static ssize_t show_version(struct device_driver *driver, char *buf)
{
	struct ldd_driver *ldriver = to_ldd_driver(driver);

	sprintf(buf, "%s\n", ldriver->version);
	return strlen(buf);
}
		

int register_ldd_driver(struct ldd_driver *driver)
{
	int ret;
	
	driver->driver.bus = &ldd_bus_type;
	ret = driver_register(&driver->driver);
	if (ret)
		return ret;
	driver->version_attr.attr.name = "version";
	driver->version_attr.attr.owner = driver->module;
	driver->version_attr.attr.mode = S_IRUGO;
	driver->version_attr.show = show_version;
	driver->version_attr.store = NULL;
	return driver_create_file(&driver->driver, &driver->version_attr);
}

void unregister_ldd_driver(struct ldd_driver *driver)
{
	driver_unregister(&driver->driver);
}
EXPORT_SYMBOL(register_ldd_driver);
EXPORT_SYMBOL(unregister_ldd_driver);



static int __init test_dev_init(void)
{
	int ret;
	ret = device_register(&test_dev);
	if (ret)
		printk(KERN_NOTICE "Unable to register test_dev\n");
	return ret;
}

static void test_dev_exit(void)
{
	device_unregister(&test_dev);
}

module_init(test_dev_init);
module_exit(test_dev_exit);
