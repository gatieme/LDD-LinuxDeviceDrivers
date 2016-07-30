/*
 * Definitions for the virtual LDD bus.
 *
 * $Id: lddbus.h,v 1.4 2004/08/20 18:49:44 corbet Exp $
 */

//extern struct device ldd_bus;
extern struct bus_type ldd_bus_type;


/*
 * The LDD driver type.
 */

struct ldd_driver {
	char *version;
	struct module *module;
	struct device_driver driver;
	struct driver_attribute version_attr;
};

#define to_ldd_driver(drv) container_of(drv, struct ldd_driver, driver);

/*
 * A device type for things "plugged" into the LDD bus.
 */

struct ldd_device {
	char *name;
	struct ldd_driver *driver;
	struct device dev;
};

#define to_ldd_device(dev) container_of(dev, struct ldd_device, dev);

extern int register_ldd_device(struct ldd_device *);
extern void unregister_ldd_device(struct ldd_device *);
extern int register_ldd_driver(struct ldd_driver *);
extern void unregister_ldd_driver(struct ldd_driver *);
