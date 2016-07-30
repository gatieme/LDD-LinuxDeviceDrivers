/* -*- C -*-
 * sculld.h -- definitions for the sculld char module
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
 */

#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include "../include/lddbus.h"

/*
 * Macros to help debugging
 */

#undef PDEBUG             /* undef it, just in case */
#ifdef SCULLD_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "sculld: " fmt, ## args)
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...) /* nothing: it's a placeholder */

#define SCULLD_MAJOR 0   /* dynamic major by default */

#define SCULLD_DEVS 4    /* sculld0 through sculld3 */

/*
 * The bare device is a variable-length region of memory.
 * Use a linked list of indirect blocks.
 *
 * "sculld_dev->data" points to an array of pointers, each
 * pointer refers to a memory page.
 *
 * The array (quantum-set) is SCULLD_QSET long.
 */
#define SCULLD_ORDER    0 /* one page at a time */
#define SCULLD_QSET     500

struct sculld_dev {
	void **data;
	struct sculld_dev *next;  /* next listitem */
	int vmas;                 /* active mappings */
	int order;                /* the current allocation order */
	int qset;                 /* the current array size */
	size_t size;              /* 32-bit will suffice */
	struct semaphore sem;     /* Mutual exclusion */
	struct cdev cdev;
	char devname[20];
	struct ldd_device ldev;
};

extern struct sculld_dev *sculld_devices;

extern struct file_operations sculld_fops;

/*
 * The different configurable parameters
 */
extern int sculld_major;     /* main.c */
extern int sculld_devs;
extern int sculld_order;
extern int sculld_qset;

/*
 * Prototypes for shared functions
 */
int sculld_trim(struct sculld_dev *dev);
struct sculld_dev *sculld_follow(struct sculld_dev *dev, int n);


#ifdef SCULLD_DEBUG
#  define SCULLD_USE_PROC
#endif

/*
 * Ioctl definitions
 */

/* Use 'K' as magic number */
#define SCULLD_IOC_MAGIC  'K'

#define SCULLD_IOCRESET    _IO(SCULLD_IOC_MAGIC, 0)

/*
 * S means "Set" through a ptr,
 * T means "Tell" directly
 * G means "Get" (to a pointed var)
 * Q means "Query", response is on the return value
 * X means "eXchange": G and S atomically
 * H means "sHift": T and Q atomically
 */
#define SCULLD_IOCSORDER   _IOW(SCULLD_IOC_MAGIC,  1, int)
#define SCULLD_IOCTORDER   _IO(SCULLD_IOC_MAGIC,   2)
#define SCULLD_IOCGORDER   _IOR(SCULLD_IOC_MAGIC,  3, int)
#define SCULLD_IOCQORDER   _IO(SCULLD_IOC_MAGIC,   4)
#define SCULLD_IOCXORDER   _IOWR(SCULLD_IOC_MAGIC, 5, int)
#define SCULLD_IOCHORDER   _IO(SCULLD_IOC_MAGIC,   6)
#define SCULLD_IOCSQSET    _IOW(SCULLD_IOC_MAGIC,  7, int)
#define SCULLD_IOCTQSET    _IO(SCULLD_IOC_MAGIC,   8)
#define SCULLD_IOCGQSET    _IOR(SCULLD_IOC_MAGIC,  9, int)
#define SCULLD_IOCQQSET    _IO(SCULLD_IOC_MAGIC,  10)
#define SCULLD_IOCXQSET    _IOWR(SCULLD_IOC_MAGIC,11, int)
#define SCULLD_IOCHQSET    _IO(SCULLD_IOC_MAGIC,  12)

#define SCULLD_IOC_MAXNR 12



