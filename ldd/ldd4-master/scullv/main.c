/* -*- C -*-
 * main.c -- the bare scullv char module
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
 * $Id: _main.c.in,v 1.21 2004/10/14 20:11:39 corbet Exp $
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/aio.h>
#include <asm/uaccess.h>
#include <linux/vmalloc.h>
#include "scullv.h"		/* local definitions */


int scullv_major =   SCULLV_MAJOR;
int scullv_devs =    SCULLV_DEVS;	/* number of bare scullv devices */
int scullv_qset =    SCULLV_QSET;
int scullv_order =   SCULLV_ORDER;

module_param(scullv_major, int, 0);
module_param(scullv_devs, int, 0);
module_param(scullv_qset, int, 0);
module_param(scullv_order, int, 0);
MODULE_AUTHOR("Alessandro Rubini");
MODULE_LICENSE("Dual BSD/GPL");

struct scullv_dev *scullv_devices; /* allocated in scullv_init */

int scullv_trim(struct scullv_dev *dev);
void scullv_cleanup(void);






#ifdef SCULLV_USE_PROC /* don't waste space if unused */
/*
 * The proc filesystem: function to read and entry
 */

void scullv_proc_offset(char *buf, char **start, off_t *offset, int *len)
{
	if (*offset == 0)
		return;
	if (*offset >= *len) {
		/* Not there yet */
		*offset -= *len;
		*len = 0;
	} else {
		/* We're into the interesting stuff now */
		*start = buf + *offset;
		*offset = 0;
	}
}

/* FIXME: Do we need this here??  It be ugly  */
int scullv_read_procmem(char *buf, char **start, off_t offset,
                   int count, int *eof, void *data)
{
	int i, j, order, qset, len = 0;
	int limit = count - 80; /* Don't print more than this */
	struct scullv_dev *d;

	*start = buf;
	for(i = 0; i < scullv_devs; i++) {
		d = &scullv_devices[i];
		if (mutex_lock_interruptible(&d->mutex))
			return -ERESTARTSYS;
		qset = d->qset;  /* retrieve the features of each device */
		order = d->order;
		len += sprintf(buf+len,"\nDevice %i: qset %i, order %i, sz %li\n",
				i, qset, order, (long)(d->size));
		for (; d; d = d->next) { /* scan the list */
			len += sprintf(buf+len,"  item at %p, qset at %p\n",d,d->data);
			scullv_proc_offset (buf, start, &offset, &len);
			if (len > limit)
				goto out;
			if (d->data && !d->next) /* dump only the last item - save space */
				for (j = 0; j < qset; j++) {
					if (d->data[j])
						len += sprintf(buf+len,"    % 4i:%8p\n",j,d->data[j]);
					scullv_proc_offset (buf, start, &offset, &len);
					if (len > limit)
						goto out;
				}
		}
	  out:
		mutex_unlock(&scullv_devices[i].mutex);
		if (len > limit)
			break;
	}
	*eof = 1;
	return len;
}

#endif /* SCULLV_USE_PROC */

/*
 * Open and close
 */

int scullv_open (struct inode *inode, struct file *filp)
{
	struct scullv_dev *dev; /* device information */

	/*  Find the device */
	dev = container_of(inode->i_cdev, struct scullv_dev, cdev);

    	/* now trim to 0 the length of the device if open was write-only */
	if ( (filp->f_flags & O_ACCMODE) == O_WRONLY) {
		if (mutex_lock_interruptible(&dev->mutex))
			return -ERESTARTSYS;
		scullv_trim(dev); /* ignore errors */
		mutex_unlock(&dev->mutex);
	}

	/* and use filp->private_data to point to the device data */
	filp->private_data = dev;

	return 0;          /* success */
}

int scullv_release (struct inode *inode, struct file *filp)
{
	return 0;
}

/*
 * Follow the list 
 */
struct scullv_dev *scullv_follow(struct scullv_dev *dev, int n)
{
	while (n--) {
		if (!dev->next) {
			dev->next = kmalloc(sizeof(struct scullv_dev), GFP_KERNEL);
			memset(dev->next, 0, sizeof(struct scullv_dev));
		}
		dev = dev->next;
		continue;
	}
	return dev;
}

/*
 * Data management: read and write
 */

ssize_t scullv_read (struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
	struct scullv_dev *dev = filp->private_data; /* the first listitem */
	struct scullv_dev *dptr;
	int quantum = PAGE_SIZE << dev->order;
	int qset = dev->qset;
	int itemsize = quantum * qset; /* how many bytes in the listitem */
	int item, s_pos, q_pos, rest;
	ssize_t retval = 0;

	if (mutex_lock_interruptible(&dev->mutex))
		return -ERESTARTSYS;
	if (*f_pos > dev->size) 
		goto nothing;
	if (*f_pos + count > dev->size)
		count = dev->size - *f_pos;
	/* find listitem, qset index, and offset in the quantum */
	item = ((long) *f_pos) / itemsize;
	rest = ((long) *f_pos) % itemsize;
	s_pos = rest / quantum; q_pos = rest % quantum;

    	/* follow the list up to the right position (defined elsewhere) */
	dptr = scullv_follow(dev, item);

	if (!dptr->data)
		goto nothing; /* don't fill holes */
	if (!dptr->data[s_pos])
		goto nothing;
	if (count > quantum - q_pos)
		count = quantum - q_pos; /* read only up to the end of this quantum */

	if (copy_to_user (buf, dptr->data[s_pos]+q_pos, count)) {
		retval = -EFAULT;
		goto nothing;
	}
	mutex_unlock(&dev->mutex);

	*f_pos += count;
	return count;

  nothing:
	mutex_unlock(&dev->mutex);
	return retval;
}



ssize_t scullv_write (struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
	struct scullv_dev *dev = filp->private_data;
	struct scullv_dev *dptr;
	int quantum = PAGE_SIZE << dev->order;
	int qset = dev->qset;
	int itemsize = quantum * qset;
	int item, s_pos, q_pos, rest;
	ssize_t retval = -ENOMEM; /* our most likely error */

	if (mutex_lock_interruptible(&dev->mutex))
		return -ERESTARTSYS;

	/* find listitem, qset index and offset in the quantum */
	item = ((long) *f_pos) / itemsize;
	rest = ((long) *f_pos) % itemsize;
	s_pos = rest / quantum; q_pos = rest % quantum;

	/* follow the list up to the right position */
	dptr = scullv_follow(dev, item);
	if (!dptr->data) {
		dptr->data = kmalloc(qset * sizeof(void *), GFP_KERNEL);
		if (!dptr->data)
			goto nomem;
		memset(dptr->data, 0, qset * sizeof(char *));
	}
	/* Allocate a quantum using virtual addresses */
	if (!dptr->data[s_pos]) {
		dptr->data[s_pos] = (void *)vmalloc(PAGE_SIZE << dptr->order);
		if (!dptr->data[s_pos])
			goto nomem;
		memset(dptr->data[s_pos], 0, PAGE_SIZE << dptr->order);
	}
	if (count > quantum - q_pos)
		count = quantum - q_pos; /* write only up to the end of this quantum */
	if (copy_from_user (dptr->data[s_pos]+q_pos, buf, count)) {
		retval = -EFAULT;
		goto nomem;
	}
	*f_pos += count;
 
    	/* update the size */
	if (dev->size < *f_pos)
		dev->size = *f_pos;
	mutex_unlock(&dev->mutex);
	return count;

  nomem:
	mutex_unlock(&dev->mutex);
	return retval;
}

/*
 * The ioctl() implementation
 */

long scullv_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{

	int err = 0, ret = 0, tmp;

	/* don't even decode wrong cmds: better returning  ENOTTY than EFAULT */
	if (_IOC_TYPE(cmd) != SCULLV_IOC_MAGIC) return -ENOTTY;
	if (_IOC_NR(cmd) > SCULLV_IOC_MAXNR) return -ENOTTY;

	/*
	 * the type is a bitmask, and VERIFY_WRITE catches R/W
	 * transfers. Note that the type is user-oriented, while
	 * verify_area is kernel-oriented, so the concept of "read" and
	 * "write" is reversed
	 */
	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	if (err)
		return -EFAULT;

	switch(cmd) {

	case SCULLV_IOCRESET:
		scullv_qset = SCULLV_QSET;
		scullv_order = SCULLV_ORDER;
		break;

	case SCULLV_IOCSORDER: /* Set: arg points to the value */
		ret = __get_user(scullv_order, (int __user *) arg);
		break;

	case SCULLV_IOCTORDER: /* Tell: arg is the value */
		scullv_order = arg;
		break;

	case SCULLV_IOCGORDER: /* Get: arg is pointer to result */
		ret = __put_user (scullv_order, (int __user *) arg);
		break;

	case SCULLV_IOCQORDER: /* Query: return it (it's positive) */
		return scullv_order;

	case SCULLV_IOCXORDER: /* eXchange: use arg as pointer */
		tmp = scullv_order;
		ret = __get_user(scullv_order, (int __user *) arg);
		if (ret == 0)
			ret = __put_user(tmp, (int __user *) arg);
		break;

	case SCULLV_IOCHORDER: /* sHift: like Tell + Query */
		tmp = scullv_order;
		scullv_order = arg;
		return tmp;

	case SCULLV_IOCSQSET:
		ret = __get_user(scullv_qset, (int __user *) arg);
		break;

	case SCULLV_IOCTQSET:
		scullv_qset = arg;
		break;

	case SCULLV_IOCGQSET:
		ret = __put_user(scullv_qset, (int __user *)arg);
		break;

	case SCULLV_IOCQQSET:
		return scullv_qset;

	case SCULLV_IOCXQSET:
		tmp = scullv_qset;
		ret = __get_user(scullv_qset, (int __user *)arg);
		if (ret == 0)
			ret = __put_user(tmp, (int __user *)arg);
		break;

	case SCULLV_IOCHQSET:
		tmp = scullv_qset;
		scullv_qset = arg;
		return tmp;

	default:  /* redundant, as cmd was checked against MAXNR */
		return -ENOTTY;
	}

	return ret;
}

/*
 * The "extended" operations
 */

loff_t scullv_llseek (struct file *filp, loff_t off, int whence)
{
	struct scullv_dev *dev = filp->private_data;
	long newpos;

	switch(whence) {
	case 0: /* SEEK_SET */
		newpos = off;
		break;

	case 1: /* SEEK_CUR */
		newpos = filp->f_pos + off;
		break;

	case 2: /* SEEK_END */
		newpos = dev->size + off;
		break;

	default: /* can't happen */
		return -EINVAL;
	}
	if (newpos<0) return -EINVAL;
	filp->f_pos = newpos;
	return newpos;
}


/*
 * A simple asynchronous I/O implementation.
 */

struct async_work {
	struct kiocb *iocb;
	int result;
	struct delayed_work work;
};

/*
 * "Complete" an asynchronous operation.
 */
static void scullv_do_deferred_op(struct work_struct * work)
{
	struct async_work *stuff = container_of(work, struct async_work, work.work);
	aio_complete(stuff->iocb, stuff->result, 0);
	kfree(stuff);
}


static int scullv_defer_op(int write, struct kiocb *iocb,
		const struct iovec *iovec, unsigned long nr_segs, loff_t pos)
{
	struct async_work *stuff;
	unsigned long seg;
	size_t count;
	size_t result = 0;

	/* Copy now while we can access the buffer */
	for (seg = 0; seg < nr_segs; seg++) {
		if (write)
			count = scullv_write(iocb->ki_filp, iovec[seg].iov_base,
					iovec[seg].iov_len, &pos);
		else
			count = scullv_read(iocb->ki_filp, iovec[seg].iov_base,
					iovec[seg].iov_len, &pos);

		if (count < 0) {
			return count;
		}
		result += count;
	}

	/* If this is a synchronous IOCB, we return our status now. */
	if (is_sync_kiocb(iocb))
		return result;

	/* Otherwise defer the completion for a few milliseconds. */
	stuff = kmalloc (sizeof (*stuff), GFP_KERNEL);
	if (stuff == NULL)
		return result; /* No memory, just complete now */
	stuff->iocb = iocb;
	stuff->result = result;
	INIT_DELAYED_WORK(&stuff->work, scullv_do_deferred_op);
	schedule_delayed_work(&stuff->work, HZ/100);
	return -EIOCBQUEUED;
}

static ssize_t scullv_aio_read(struct kiocb *iocb, const struct iovec *iovec,
		unsigned long nr_segs, loff_t pos)
{
	return scullv_defer_op(0, iocb, iovec, nr_segs, pos);
}

static ssize_t scullv_aio_write(struct kiocb *iocb, const struct iovec *iovec,
		unsigned long nr_segs, loff_t pos)
{
	return scullv_defer_op(1, iocb, iovec, nr_segs, pos);
}

 
/*
 * Mmap *is* available, but confined in a different file
 */
extern int scullv_mmap(struct file *filp, struct vm_area_struct *vma);


/*
 * The fops
 */

struct file_operations scullv_fops = {
	.owner =     THIS_MODULE,
	.llseek =    scullv_llseek,
	.read =	     scullv_read,
	.write =     scullv_write,
	.unlocked_ioctl = scullv_ioctl,
	.mmap =	     scullv_mmap,
	.open =	     scullv_open,
	.release =   scullv_release,
	.aio_read =  scullv_aio_read,
	.aio_write = scullv_aio_write,
};

int scullv_trim(struct scullv_dev *dev)
{
	struct scullv_dev *next, *dptr;
	int qset = dev->qset;   /* "dev" is not-null */
	int i;

	if (dev->vmas) /* don't trim: there are active mappings */
		return -EBUSY;

	for (dptr = dev; dptr; dptr = next) { /* all the list items */
		if (dptr->data) {
			/* Release the quantum-set */
			for (i = 0; i < qset; i++)
				if (dptr->data[i])
					vfree(dptr->data[i]);

			kfree(dptr->data);
			dptr->data=NULL;
		}
		next=dptr->next;
		if (dptr != dev) kfree(dptr); /* all of them but the first */
	}
	dev->size = 0;
	dev->qset = scullv_qset;
	dev->order = scullv_order;
	dev->next = NULL;
	return 0;
}


static void scullv_setup_cdev(struct scullv_dev *dev, int index)
{
	int err, devno = MKDEV(scullv_major, index);
    
	cdev_init(&dev->cdev, &scullv_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &scullv_fops;
	err = cdev_add (&dev->cdev, devno, 1);
	/* Fail gracefully if need be */
	if (err)
		printk(KERN_NOTICE "Error %d adding scull%d", err, index);
}



/*
 * Finally, the module stuff
 */

int scullv_init(void)
{
	int result, i;
	dev_t dev = MKDEV(scullv_major, 0);
	
	/*
	 * Register your major, and accept a dynamic number.
	 */
	if (scullv_major)
		result = register_chrdev_region(dev, scullv_devs, "scullv");
	else {
		result = alloc_chrdev_region(&dev, 0, scullv_devs, "scullv");
		scullv_major = MAJOR(dev);
	}
	if (result < 0)
		return result;

	
	/* 
	 * allocate the devices -- we can't have them static, as the number
	 * can be specified at load time
	 */
	scullv_devices = kmalloc(scullv_devs*sizeof (struct scullv_dev), GFP_KERNEL);
	if (!scullv_devices) {
		result = -ENOMEM;
		goto fail_malloc;
	}
	memset(scullv_devices, 0, scullv_devs*sizeof (struct scullv_dev));
	for (i = 0; i < scullv_devs; i++) {
		scullv_devices[i].order = scullv_order;
		scullv_devices[i].qset = scullv_qset;
		mutex_init(&scullv_devices[i].mutex);
		scullv_setup_cdev(scullv_devices + i, i);
	}


#ifdef SCULLV_USE_PROC /* only when available */
	create_proc_read_entry("scullvmem", 0, NULL, scullv_read_procmem, NULL);
#endif
	return 0; /* succeed */

  fail_malloc:
	unregister_chrdev_region(dev, scullv_devs);
	return result;
}



void scullv_cleanup(void)
{
	int i;

#ifdef SCULLV_USE_PROC
	remove_proc_entry("scullvmem", NULL);
#endif

	for (i = 0; i < scullv_devs; i++) {
		cdev_del(&scullv_devices[i].cdev);
		scullv_trim(scullv_devices + i);
	}
	kfree(scullv_devices);
	unregister_chrdev_region(MKDEV (scullv_major, 0), scullv_devs);
}


module_init(scullv_init);
module_exit(scullv_cleanup);
