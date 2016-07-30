#define WINDOW_ADDR 0x01000000      /* NOR FLASH物理地址 */
#define WINDOW_SIZE 0x800000         /* NOR FLASH大小 */
#define BUSWIDTH    2
/* 探测的接口类型，可以是"cfi_probe", "jedec_probe", "map_rom", NULL }; */
#define PROBETYPES { "cfi_probe", NULL }

#define MSG_PREFIX "S3C2410-NOR:"   /* prefix for our printk()'s */
#define MTDID      "s3c2410-nor"    /* for mtdparts= partitioning */

static struct mtd_info *mymtd;

struct map_info s3c2410nor_map =  // map_info
{
  .name = "NOR flash on S3C2410",
  .size = WINDOW_SIZE,
  .bankwidth = BUSWIDTH,
  .phys = WINDOW_ADDR,
};

#ifdef CONFIG_MTD_PARTITIONS
  /* MTD分区信息  */
  static struct mtd_partition static_partitions[] =
  {
    {
      .name = "BootLoader", .size = 0x040000, .offset = 0x0  //bootloader存放的区域
    } ,
    {
      .name = "Kernel", .size = 0x0100000, .offset = 0x40000 //内核映像存放的区域
    }
    ,
    {
      .name = "RamDisk", .size = 0x400000, .offset = 0x140000 //ramdisk存放的区域
    }
    ,
    {
      .name = "cramfs(2MB)", .size = 0x200000, .offset = 0x540000 //只读的cramfs区域
    }
    ,
    {
      .name = "jffs2(0.75MB)", .size = 0xc0000, .offset = 0x740000 //可读写的jffs2区域
    }
    ,
  };
#endif

static int mtd_parts_nb = 0;
static struct mtd_partition *mtd_parts = 0;

int __init init_s3c2410nor(void)
{
  static const char *rom_probe_types[] = PROBETYPES;
  const char **type;
  const char *part_type = 0;

  printk(KERN_NOTICE MSG_PREFIX "0x%08x at 0x%08x\n", WINDOW_SIZE, WINDOW_ADDR);
  s3c2410nor_map.virt = ioremap(WINDOW_ADDR, WINDOW_SIZE);//物理->虚拟地址

  if (!s3c2410nor_map.virt)
  {
    printk(MSG_PREFIX "failed to ioremap\n");
    return  - EIO;
  }

  simple_map_init(&s3c2410nor_map);

  mymtd = 0;
  type = rom_probe_types;
  for (; !mymtd &&  *type; type++)
  {
    mymtd = do_map_probe(*type, &s3c2410nor_map);//探测NOR FLASH
  }
  if (mymtd)
  {
    mymtd->owner = THIS_MODULE;

    #ifdef CONFIG_MTD_PARTITIONS
      mtd_parts_nb = parse_mtd_partitions(mymtd, NULL, &mtd_parts, MTDID);//探测分区信息
      if (mtd_parts_nb > 0)
        part_type = "detected";

      if (mtd_parts_nb == 0) //未探测到，使用数组定义的分区信息
      {
        mtd_parts = static_partitions;
        mtd_parts_nb = ARRAY_SIZE(static_partitions);
        part_type = "static";
      }
    #endif
    add_mtd_device(mymtd);
    if (mtd_parts_nb == 0)
      printk(KERN_NOTICE MSG_PREFIX "no partition info available\n");
    else
    {
      printk(KERN_NOTICE MSG_PREFIX "using %s partition definition\n",
        part_type);
      add_mtd_partitions(mymtd, mtd_parts, mtd_parts_nb);//添加分区信息
    }
    return 0;
  }

  iounmap((void*)s3c2410nor_map.virt);
  return  - ENXIO;
}

static void __exit cleanup_s3c2410nor(void)
{
  if (mymtd)
  {
    del_mtd_partitions(mymtd);  //删除分区
    del_mtd_device(mymtd);   //删除设备
    map_destroy(mymtd);     
  }
  if (s3c2410nor_map.virt)
  {
    iounmap((void*)s3c2410nor_map.virt);
    s3c2410nor_map.virt = 0;
  }
}
