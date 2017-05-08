
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/sched.h>
#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/timer.h>
#include <linux/types.h>	/* size_t */
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/hdreg.h>	/* HDIO_GETGEO */
#include <linux/kdev_t.h>
#include <linux/vmalloc.h>
#include <linux/genhd.h>
//#include <linux/elevator.h>
#include <linux/blkdev.h>
#include <linux/buffer_head.h>	/* invalidate_bdev */
#include <linux/bio.h>


#define VIRTUAL_BLKDEV_DEVICEMAJOR COMPAQ_SMART2_MAJOR
#define VIRTUAL_BLKDEV_DISKNAME "Virtual_blkdev"


#define VIRTUAL_BLKDEV_DEVICEMAJOR COMPAQ_SMART2_MAJOR /*主设备号*/
#define VIRTUAL_BLKDEV_DISKNAME "Virtual_blkdev"/*设备名*/
#define VIRTUAL_BLKDEV_BYTES (8*1024*1024)/*设备的大小为8M*/
static struct request_queue *Virtual_blkdev_queue;/*请求队列指针*/
static struct gendisk *Virtual_blkdev_disk;/*通用磁盘*/
unsigned char Virtual_blkdev_data[VIRTUAL_BLKDEV_BYTES];/*8M的线性静态内存空间*/

static void Virtual_blkdev_do_request(struct request_queue *q)
{
	struct request *req;
	while ((req = elv_next_request(q)) != NULL) 
	{
		if ((req->sector + req->current_nr_sectors) << 9> VIRTUAL_BLKDEV_BYTES) 
		{
		printk(KERN_ERR VIRTUAL_BLKDEV_DISKNAME": bad request: block=%llu, count=%u\n",
			(unsigned long long)req->sector,
			req->current_nr_sectors);
		end_request(req, 0);
		continue;
	    }/*endif*/
		switch (rq_data_dir(req)) 
		{
			case READ:
				memcpy(req->buffer,Virtual_blkdev_data + (req->sector << 9),
					req->current_nr_sectors << 9);
				end_request(req, 1);
				break;
			case WRITE:
				memcpy(Virtual_blkdev_data + (req->sector << 9),
					req->buffer, req->current_nr_sectors << 9);
				end_request(req, 1);
				break;
			default:
				/* No default because rq_data_dir(req) is 1 bit */
				break;
		}
	}/*endwhile*/
}

struct block_device_operations Virtual_blkdev_fops =
{
	.owner = THIS_MODULE,
};

static int __init Virtual_blkdev_init(void)
{
	int ret;
	struct elevator_queue * old_elevator;

	Virtual_blkdev_disk = alloc_disk(1);
	if (!Virtual_blkdev_disk) 
	{
		ret = -ENOMEM;
		goto err_alloc_disk;
	}
	Virtual_blkdev_queue = blk_init_queue(Virtual_blkdev_do_request, NULL);
	if (!Virtual_blkdev_queue) 
	{
		ret = -ENOMEM;
		goto err_init_queue;
	}
	
	old_elevator=Virtual_blkdev_queue->elevator;
	if(IS_ERR_VALUE(elevator_init(Virtual_blkdev_queue,"noop")))
	{
		printk(KERN_WARNING"elevator is error");
	}
	else
	{
		elevator_exit(old_elevator);
	}
	strcpy(Virtual_blkdev_disk->disk_name, VIRTUAL_BLKDEV_DISKNAME);
	Virtual_blkdev_disk->major = VIRTUAL_BLKDEV_DEVICEMAJOR;
	Virtual_blkdev_disk->first_minor = 0;
	Virtual_blkdev_disk->fops = &Virtual_blkdev_fops;
	Virtual_blkdev_disk->queue = Virtual_blkdev_queue;
	set_capacity(Virtual_blkdev_disk, VIRTUAL_BLKDEV_BYTES>>9);
	add_disk(Virtual_blkdev_disk);
	return 0;
	err_init_queue:
		put_disk(Virtual_blkdev_disk);
	err_alloc_disk:
		return ret;
}
static void __exit Virtual_blkdev_exit(void)
{
	del_gendisk(Virtual_blkdev_disk);
	put_disk(Virtual_blkdev_disk);
	blk_cleanup_queue(Virtual_blkdev_queue);
}
module_init(Virtual_blkdev_init);
module_exit(Virtual_blkdev_exit);
