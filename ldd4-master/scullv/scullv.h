/* -*- C -*-
 * scullv.h -- definitions for the scullv char module
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

/*
 * Macros to help debugging
 */

#undef PDEBUG             /* undef it, just in case */
#ifdef SCULLV_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "scullv: " fmt, ## args)
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...) /* nothing: it's a placeholder */

#define SCULLV_MAJOR 0   /* dynamic major by default */

#define SCULLV_DEVS 4    /* scullv0 through scullv3 */

/*
 * The bare device is a variable-length region of memory.
 * Use a linked list of indirect blocks.
 *
 * "scullv_dev->data" points to an array of pointers, each
 * pointer refers to a memory page.
 *
 * The array (quantum-set) is SCULLV_QSET long.
 */
#define SCULLV_ORDER    4 /* 16 pages at a time */
#define SCULLV_QSET     500

struct scullv_dev {
	void **data;
	struct scullv_dev *next;  /* next listitem */
	int vmas;                 /* active mappings */
	int order;                /* the current allocation order */
	int qset;                 /* the current array size */
	size_t size;              /* 32-bit will suffice */
	struct mutex mutex;       /* Mutual exclusion */
	struct cdev cdev;
};

extern struct scullv_dev *scullv_devices;

extern struct file_operations scullv_fops;

/*
 * The different configurable parameters
 */
extern int scullv_major;     /* main.c */
extern int scullv_devs;
extern int scullv_order;
extern int scullv_qset;

/*
 * Prototypes for shared functions
 */
int scullv_trim(struct scullv_dev *dev);
struct scullv_dev *scullv_follow(struct scullv_dev *dev, int n);


#ifdef SCULLV_DEBUG
#  define SCULLV_USE_PROC
#endif

/*
 * Ioctl definitions
 */

/* Use 'K' as magic number */
#define SCULLV_IOC_MAGIC  'K'

#define SCULLV_IOCRESET    _IO(SCULLV_IOC_MAGIC, 0)

/*
 * S means "Set" through a ptr,
 * T means "Tell" directly
 * G means "Get" (to a pointed var)
 * Q means "Query", response is on the return value
 * X means "eXchange": G and S atomically
 * H means "sHift": T and Q atomically
 */
#define SCULLV_IOCSORDER   _IOW(SCULLV_IOC_MAGIC,  1, int)
#define SCULLV_IOCTORDER   _IO(SCULLV_IOC_MAGIC,   2)
#define SCULLV_IOCGORDER   _IOR(SCULLV_IOC_MAGIC,  3, int)
#define SCULLV_IOCQORDER   _IO(SCULLV_IOC_MAGIC,   4)
#define SCULLV_IOCXORDER   _IOWR(SCULLV_IOC_MAGIC, 5, int)
#define SCULLV_IOCHORDER   _IO(SCULLV_IOC_MAGIC,   6)
#define SCULLV_IOCSQSET    _IOW(SCULLV_IOC_MAGIC,  7, int)
#define SCULLV_IOCTQSET    _IO(SCULLV_IOC_MAGIC,   8)
#define SCULLV_IOCGQSET    _IOR(SCULLV_IOC_MAGIC,  9, int)
#define SCULLV_IOCQQSET    _IO(SCULLV_IOC_MAGIC,  10)
#define SCULLV_IOCXQSET    _IOWR(SCULLV_IOC_MAGIC,11, int)
#define SCULLV_IOCHQSET    _IO(SCULLV_IOC_MAGIC,  12)

#define SCULLV_IOC_MAXNR 12



