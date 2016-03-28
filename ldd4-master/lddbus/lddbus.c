/*
 * A virtual bus for LDD sample code devices to plug into.  This
 * code is heavily borrowed from drivers/base/sys.c
 *
 * Copyright (C) 2001 Alessandro Rubini and Jonathan Corbet
 * Copyright (C) 2001 O'Reilly & Associates
 *
 * The source code in this file can be freely used, adapted,
 * and redistributed in source or binary form, so long as an
 * acknowledgment appears in derived source files.  The citation
 * should list that the code comes from the book "Linux Device
 * Drivers" by Alessandro Rubini and Jonathan Corbet, published
 * by O'Reilly & Associates.   No warranty is attached;
 * we cannot take responsibility for errors or fitness for use.
 *
 */
/* $Id: lddbus.c,v 1.9 2004/09/26 08:12:27 gregkh Exp $ */

#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include "lddbus.h"

MODULE_AUTHOR("Jonathan Corbet");
MODULE_LICENSE("Dual BSD/GPL");
static char *Version = "$Revision: 1.9 $";

/*
 * Respond to hotplug events.
 */
static int ldd_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	return add_uevent_var(env, "LDDBUS_VERSION=%s", Version);
}

/*
 * Match LDD devices to drivers.  Just do a simple name test.
 */
static int ldd_match(struct device *dev, struct device_driver *driver)
{
	return !strncmp(dev_name(dev), driver->name, strlen(driver->name));
}


/*
 * The LDD bus device.
 */
static void ldd_bus_release(struct device *dev)
{
	printk(KERN_DEBUG "lddbus release\n");
}
	
struct device ldd_bus = {
	.init_name = "ldd0",
	.release  = ldd_bus_release
};


/*
 * And the bus type.
 */
struct bus_type ldd_bus_type = {
	.name = "ldd",
	.match = ldd_match,
	.uevent  = ldd_uevent,
};

/*
 * Export a simple attribute.
 */
static ssize_t show_bus_version(struct bus_type *bus, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", Version);
}

static BUS_ATTR(version, S_IRUGO, show_bus_version, NULL);



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
	ldddev->dev.parent = &ldd_bus;
	ldddev->dev.release = ldd_dev_release;
	dev_set_name(&ldddev->dev, "%s", ldddev->name);
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



static int __init ldd_bus_init(void)
{
	int ret;

	ret = bus_register(&ldd_bus_type);
	if (ret)
		return ret;
	if (bus_create_file(&ldd_bus_type, &bus_attr_version))
		printk(KERN_NOTICE "Unable to create version attribute\n");
	ret = device_register(&ldd_bus);
	if (ret)
		printk(KERN_NOTICE "Unable to register ldd0\n");
	return ret;
}

static void ldd_bus_exit(void)
{
	device_unregister(&ldd_bus);
	bus_unregister(&ldd_bus_type);
}

module_init(ldd_bus_init);
module_exit(ldd_bus_exit);
