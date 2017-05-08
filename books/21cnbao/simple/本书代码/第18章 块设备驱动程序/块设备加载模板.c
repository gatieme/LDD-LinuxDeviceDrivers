static int __init Virtual_blkdev_init(void)
{
	int ret;
	static struct gendisk *xxx_disk; /*通用磁盘结构*/
	static struct request_queue *xxx_queue; /*请求队列*/
	xxx_disk = alloc_disk(1); /*分配通用磁盘*/
	if (!xxx_disk) {/*分配通用磁盘失败，退出模块*/
		ret = -ENOMEM;
		goto err_alloc_disk;
	}
	
	if(register_blkdev(xxx_MAJOR,"xxx"))/*注册设备*/
	{
		ret=-EIO;
		goto err_alloc_disk;
	}
	
	xxx_queue = blk_init_queue(xx_request, NULL);/*请求队列初始化*/
	if (!xxx_queue) {/*请求队列初始化失败*/
		ret = -ENOMEM;
		goto err_init_queue;
	}

	strcpy(xxx_disk->disk_name, XXX_DISKNAME);/*设定设备名*/
	xxx_disk->major =xxx_MAJOR;/*设置主设备号*/
	xxx_disk->first_minor = 0;/*设置次设备号*/
	xxx_disk->fops = &xxx_fops;/*块设备操作函数*/
	xxx_disk->queue = xxx_queue;/*设置请求队列*/
	set_capacity(xxx_disk, xxx_BYTES>>9);/*设置设备容量*/
	add_disk(xxx_disk);/*激活磁盘设备*/
	return 0;
	err_init_queue:/*队列初始化失败*/
		unregister_blkdev(xxx_MAJOR,"xxx");
	err_alloc_disk:/*分配磁盘失败*/
		put_disk(xxx_disk);
		return ret;
}