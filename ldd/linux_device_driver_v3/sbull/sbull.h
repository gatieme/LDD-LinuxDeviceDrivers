
/*
 * sbull.h -- definitions for the char module
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


#include <linux/ioctl.h>

/* Multiqueue only works on 2.4 */
#ifdef SBULL_MULTIQUEUE
#    warning "Multiqueue only works on 2.4 kernels"
#endif

/*
 * Macros to help debugging
 */

#undef PDEBUG             /* undef it, just in case */
#ifdef SBULL_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "sbull: " fmt, ## args)
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...) /* nothing: it's a placeholder */


#define SBULL_MAJOR 0       /* dynamic major by default */
#define SBULL_DEVS 2        /* two disks */
#define SBULL_RAHEAD 2      /* two sectors */
#define SBULL_SIZE 2048     /* two megs each */
#define SBULL_BLKSIZE 1024  /* 1k blocks */
#define SBULL_HARDSECT 512  /* 2.2 and 2.4 can used different values */

#define SBULLR_MAJOR 0      /* Dynamic major for raw device */
/*
 * The sbull device is removable: if it is left closed for more than
 * half a minute, it is removed. Thus use a usage count and a
 * kernel timer
 */

typedef struct Sbull_Dev {
   int size;
   int usage;
   struct timer_list timer;
   spinlock_t lock;
   u8 *data;
#ifdef SBULL_MULTIQUEUE
   request_queue_t *queue;
   int busy;
#endif
}              Sbull_Dev;
