/*
 *  linux/drivers/ide/ide-disk.c	Version 1.10	June 9, 2000
 *
 *  Copyright (C) 1994-1998  Linus Torvalds & authors (see below)
 */

/*
 *  Mostly written by Mark Lord <mlord@pobox.com>
 *                and Gadi Oxman <gadio@netvision.net.il>
 *                and Andre Hedrick <andre@linux-ide.org>
 *
 * This is the IDE/ATA disk driver, as evolved from hd.c and ide.c.
 *
 * Version 1.00		move disk only code from ide.c to ide-disk.c
 *			support optional byte-swapping of all data
 * Version 1.01		fix previous byte-swapping code
 * Version 1.02		remove ", LBA" from drive identification msgs
 * Version 1.03		fix display of id->buf_size for big-endian
 * Version 1.04		add /proc configurable settings and S.M.A.R.T support
 * Version 1.05		add capacity support for ATA3 >= 8GB
 * Version 1.06		get boot-up messages to show full cyl count
 * Version 1.07		disable door-locking if it fails
 * Version 1.08		fixed CHS/LBA translations for ATA4 > 8GB,
 *			process of adding new ATA4 compliance.
 *			fixed problems in allowing fdisk to see
 *			the entire disk.
 * Version 1.09		added increment of rq->sector in ide_multwrite
 *			added UDMA 3/4 reporting
 * Version 1.10		request queue changes, Ultra DMA 100
 */

#define IDEDISK_VERSION	"1.10"

#undef REALLY_SLOW_IO		/* most systems can safely undef this */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/major.h>
#include <linux/errno.h>
#include <linux/genhd.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/ide.h>

#include <asm/byteorder.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#ifdef CONFIG_BLK_DEV_PDC4030
#define IS_PDC4030_DRIVE (HWIF(drive)->chipset == ide_pdc4030)
#else
#define IS_PDC4030_DRIVE (0)	/* auto-NULLs out pdc4030 code */
#endif

static void idedisk_bswap_data (void *buffer, int wcount)
{
	u16 *p = buffer;

	while (wcount--) {
		*p = *p << 8 | *p >> 8; p++;
		*p = *p << 8 | *p >> 8; p++;
	}
}

static inline void idedisk_input_data (ide_drive_t *drive, void *buffer, unsigned int wcount)
{
	ide_input_data(drive, buffer, wcount);
	if (drive->bswap)
		idedisk_bswap_data(buffer, wcount);
}

static inline void idedisk_output_data (ide_drive_t *drive, void *buffer, unsigned int wcount)
{
	if (drive->bswap) {
		idedisk_bswap_data(buffer, wcount);
		ide_output_data(drive, buffer, wcount);
		idedisk_bswap_data(buffer, wcount);
	} else
		ide_output_data(drive, buffer, wcount);
}

/*
 * lba_capacity_is_ok() performs a sanity check on the claimed "lba_capacity"
 * value for this drive (from its reported identification information).
 *
 * Returns:	1 if lba_capacity looks sensible
 *		0 otherwise
 *
 * It is called only once for each drive.
 */
static int lba_capacity_is_ok (struct hd_driveid *id)
{
	unsigned long lba_sects, chs_sects, head, tail;

	/*
	 * The ATA spec tells large drives to return
	 * C/H/S = 16383/16/63 independent of their size.
	 * Some drives can be jumpered to use 15 heads instead of 16.
	 * Some drives can be jumpered to use 4092 cyls instead of 16383.
	 */
	if ((id->cyls == 16383
	     || (id->cyls == 4092 && id->cur_cyls == 16383)) &&
	    id->sectors == 63 &&
	    (id->heads == 15 || id->heads == 16) &&
	    id->lba_capacity >= 16383*63*id->heads)
		return 1;

	lba_sects   = id->lba_capacity;
	chs_sects   = id->cyls * id->heads * id->sectors;

	/* perform a rough sanity check on lba_sects:  within 10% is OK */
	if ((lba_sects - chs_sects) < chs_sects/10)
		return 1;

	/* some drives have the word order reversed */
	head = ((lba_sects >> 16) & 0xffff);
	tail = (lba_sects & 0xffff);
	lba_sects = (head | (tail << 16));
	if ((lba_sects - chs_sects) < chs_sects/10) {
		id->lba_capacity = lba_sects;
		return 1;	/* lba_capacity is (now) good */
	}

	return 0;	/* lba_capacity value may be bad */
}

/*
 * read_intr() is the handler for disk read/multread interrupts
 */
static ide_startstop_t read_intr (ide_drive_t *drive)
{
	byte stat;
	int i;
	unsigned int msect, nsect;
	struct request *rq;

	/* new way for dealing with premature shared PCI interrupts */
	if (!OK_STAT(stat=GET_STAT(),DATA_READY,BAD_R_STAT)) {
		if (stat & (ERR_STAT|DRQ_STAT)) {
			return ide_error(drive, "read_intr", stat);
		}
		/* no data yet, so wait for another interrupt */
		ide_set_handler(drive, &read_intr, WAIT_CMD, NULL);
		return ide_started;
	}
	msect = drive->mult_count;
	
read_next:
	rq = HWGROUP(drive)->rq;
	if (msect) {
		if ((nsect = rq->current_nr_sectors) > msect)
			nsect = msect;
		msect -= nsect;
	} else
		nsect = 1;
	idedisk_input_data(drive, rq->buffer, nsect * SECTOR_WORDS);
#ifdef DEBUG
	printk("%s:  read: sectors(%ld-%ld), buffer=0x%08lx, remaining=%ld\n",
		drive->name, rq->sector, rq->sector+nsect-1,
		(unsigned long) rq->buffer+(nsect<<9), rq->nr_sectors-nsect);
#endif
	rq->sector += nsect;
	rq->buffer += nsect<<9;
	rq->errors = 0;
	i = (rq->nr_sectors -= nsect);
	if (((long)(rq->current_nr_sectors -= nsect)) <= 0)
		ide_end_request(1, HWGROUP(drive));
	if (i > 0) {
		if (msect)
			goto read_next;
		ide_set_handler (drive, &read_intr, WAIT_CMD, NULL);
                return ide_started;
	}
        return ide_stopped;
}

/*
 * write_intr() is the handler for disk write interrupts
 */
static ide_startstop_t write_intr (ide_drive_t *drive)
{
	byte stat;
	int i;
	ide_hwgroup_t *hwgroup = HWGROUP(drive);
	struct request *rq = hwgroup->rq;

	if (!OK_STAT(stat=GET_STAT(),DRIVE_READY,drive->bad_wstat)) {
		printk("%s: write_intr error1: nr_sectors=%ld, stat=0x%02x\n", drive->name, rq->nr_sectors, stat);
        } else {
#ifdef DEBUG
		printk("%s: write: sector %ld, buffer=0x%08lx, remaining=%ld\n",
			drive->name, rq->sector, (unsigned long) rq->buffer,
			rq->nr_sectors-1);
#endif
		if ((rq->nr_sectors == 1) ^ ((stat & DRQ_STAT) != 0)) {
			rq->sector++;
			rq->buffer += 512;
			rq->errors = 0;
			i = --rq->nr_sectors;
			--rq->current_nr_sectors;
			if (((long)rq->current_nr_sectors) <= 0)
				ide_end_request(1, hwgroup);
			if (i > 0) {
				idedisk_output_data (drive, rq->buffer, SECTOR_WORDS);
				ide_set_handler (drive, &write_intr, WAIT_CMD, NULL);
                                return ide_started;
			}
                        return ide_stopped;
		}
		return ide_stopped;	/* the original code did this here (?) */
	}
	return ide_error(drive, "write_intr", stat);
}

/*
 * ide_multwrite() transfers a block of up to mcount sectors of data
 * to a drive as part of a disk multiple-sector write operation.
 *
 * Returns 0 on success.
 *
 * Note that we may be called from two contexts - the do_rw_disk context
 * and IRQ context. The IRQ can happen any time after we've output the
 * full "mcount" number of sectors, so we must make sure we update the
 * state _before_ we output the final part of the data!
 */
int ide_multwrite (ide_drive_t *drive, unsigned int mcount)
{
 	ide_hwgroup_t	*hwgroup= HWGROUP(drive);
 	struct request	*rq = &hwgroup->wrq;
 
  	do {
  		char *buffer;
  		int nsect = rq->current_nr_sectors;
 
		if (nsect > mcount)
			nsect = mcount;
		mcount -= nsect;
		buffer = rq->buffer;

		rq->sector += nsect;
		rq->buffer += nsect << 9;
		rq->nr_sectors -= nsect;
		rq->current_nr_sectors -= nsect;

		/* Do we move to the next bh after this? */
		if (!rq->current_nr_sectors) {
			struct buffer_head *bh = rq->bh->b_reqnext;

			/* end early early we ran out of requests */
			if (!bh) {
				mcount = 0;
			} else {
				rq->bh = bh;
				rq->current_nr_sectors = bh->b_size >> 9;
				rq->buffer             = bh->b_data;
			}
		}

		/*
		 * Ok, we're all setup for the interrupt
		 * re-entering us on the last transfer.
		 */
		idedisk_output_data(drive, buffer, nsect<<7);
	} while (mcount);

        return 0;
}

/*
 * multwrite_intr() is the handler for disk multwrite interrupts
 */
static ide_startstop_t multwrite_intr (ide_drive_t *drive)
{
	byte stat;
	int i;
	ide_hwgroup_t *hwgroup = HWGROUP(drive);
	struct request *rq = &hwgroup->wrq;

	if (OK_STAT(stat=GET_STAT(),DRIVE_READY,drive->bad_wstat)) {
		if (stat & DRQ_STAT) {
			/*
			 *	The drive wants data. Remember rq is the copy
			 *	of the request
			 */
			if (rq->nr_sectors) {
				if (ide_multwrite(drive, drive->mult_count))
					return ide_stopped;
				ide_set_handler (drive, &multwrite_intr, WAIT_CMD, NULL);
				return ide_started;
			}
		} else {
			/*
			 *	If the copy has all the blocks completed then
			 *	we can end the original request.
			 */
			if (!rq->nr_sectors) {	/* all done? */
				rq = hwgroup->rq;
				for (i = rq->nr_sectors; i > 0;){
					i -= rq->current_nr_sectors;
					ide_end_request(1, hwgroup);
				}
				return ide_stopped;
			}
		}
		return ide_stopped;	/* the original code did this here (?) */
	}
	return ide_error(drive, "multwrite_intr", stat);
}

/*
 * set_multmode_intr() is invoked on completion of a WIN_SETMULT cmd.
 */
static ide_startstop_t set_multmode_intr (ide_drive_t *drive)
{
	byte stat;

	if (OK_STAT(stat=GET_STAT(),READY_STAT,BAD_STAT)) {
		drive->mult_count = drive->mult_req;
	} else {
		drive->mult_req = drive->mult_count = 0;
		drive->special.b.recalibrate = 1;
		(void) ide_dump_status(drive, "set_multmode", stat);
	}
	return ide_stopped;
}

/*
 * set_geometry_intr() is invoked on completion of a WIN_SPECIFY cmd.
 */
static ide_startstop_t set_geometry_intr (ide_drive_t *drive)
{
	byte stat;

	if (OK_STAT(stat=GET_STAT(),READY_STAT,BAD_STAT))
		return ide_stopped;

	if (stat & (ERR_STAT|DRQ_STAT))
		return ide_error(drive, "set_geometry_intr", stat);

	ide_set_handler(drive, &set_geometry_intr, WAIT_CMD, NULL);
	return ide_started;	
}

/*
 * recal_intr() is invoked on completion of a WIN_RESTORE (recalibrate) cmd.
 */
static ide_startstop_t recal_intr (ide_drive_t *drive)
{
	byte stat = GET_STAT();

	if (!OK_STAT(stat,READY_STAT,BAD_STAT))
		return ide_error(drive, "recal_intr", stat);
	return ide_stopped;
}

/*
 * do_rw_disk() issues READ and WRITE commands to a disk,
 * using LBA if supported, or CHS otherwise, to address sectors.
 * It also takes care of issuing special DRIVE_CMDs.
 */
static ide_startstop_t do_rw_disk (ide_drive_t *drive, struct request *rq, unsigned long block)
{
	if (IDE_CONTROL_REG)
		OUT_BYTE(drive->ctl,IDE_CONTROL_REG);
	OUT_BYTE(0x00, IDE_FEATURE_REG);
	OUT_BYTE(rq->nr_sectors,IDE_NSECTOR_REG);
#ifdef CONFIG_BLK_DEV_PDC4030
	if (drive->select.b.lba || IS_PDC4030_DRIVE) {
#else /* !CONFIG_BLK_DEV_PDC4030 */
	if (drive->select.b.lba) {
#endif /* CONFIG_BLK_DEV_PDC4030 */
#ifdef DEBUG
		printk("%s: %sing: LBAsect=%ld, sectors=%ld, buffer=0x%08lx\n",
			drive->name, (rq->cmd==READ)?"read":"writ",
			block, rq->nr_sectors, (unsigned long) rq->buffer);
#endif
		OUT_BYTE(block,IDE_SECTOR_REG);
		OUT_BYTE(block>>=8,IDE_LCYL_REG);
		OUT_BYTE(block>>=8,IDE_HCYL_REG);
		OUT_BYTE(((block>>8)&0x0f)|drive->select.all,IDE_SELECT_REG);
	} else {
		unsigned int sect,head,cyl,track;
		track = block / drive->sect;
		sect  = block % drive->sect + 1;
		OUT_BYTE(sect,IDE_SECTOR_REG);
		head  = track % drive->head;
		cyl   = track / drive->head;
		OUT_BYTE(cyl,IDE_LCYL_REG);
		OUT_BYTE(cyl>>8,IDE_HCYL_REG);
		OUT_BYTE(head|drive->select.all,IDE_SELECT_REG);
#ifdef DEBUG
		printk("%s: %sing: CHS=%d/%d/%d, sectors=%ld, buffer=0x%08lx\n",
			drive->name, (rq->cmd==READ)?"read":"writ", cyl,
			head, sect, rq->nr_sectors, (unsigned long) rq->buffer);
#endif
	}
#ifdef CONFIG_BLK_DEV_PDC4030
	if (IS_PDC4030_DRIVE) {
		extern ide_startstop_t do_pdc4030_io(ide_drive_t *, struct request *);
		return do_pdc4030_io (drive, rq);
	}
#endif /* CONFIG_BLK_DEV_PDC4030 */
	if (rq->cmd == READ) {
#ifdef CONFIG_BLK_DEV_IDEDMA
		if (drive->using_dma && !(HWIF(drive)->dmaproc(ide_dma_read, drive)))
			return ide_started;
#endif /* CONFIG_BLK_DEV_IDEDMA */
		ide_set_handler(drive, &read_intr, WAIT_CMD, NULL);
		OUT_BYTE(drive->mult_count ? WIN_MULTREAD : WIN_READ, IDE_COMMAND_REG);
		return ide_started;
	}
	if (rq->cmd == WRITE) {
		ide_startstop_t startstop;
#ifdef CONFIG_BLK_DEV_IDEDMA
		if (drive->using_dma && !(HWIF(drive)->dmaproc(ide_dma_write, drive)))
			return ide_started;
#endif /* CONFIG_BLK_DEV_IDEDMA */
		OUT_BYTE(drive->mult_count ? WIN_MULTWRITE : WIN_WRITE, IDE_COMMAND_REG);
		if (ide_wait_stat(&startstop, drive, DATA_READY, drive->bad_wstat, WAIT_DRQ)) {
			printk(KERN_ERR "%s: no DRQ after issuing %s\n", drive->name,
				drive->mult_count ? "MULTWRITE" : "WRITE");
			return startstop;
		}
		if (!drive->unmask)
			__cli();	/* local CPU only */
		if (drive->mult_count) {
			ide_hwgroup_t *hwgroup = HWGROUP(drive);
			/*
			 * Ugh.. this part looks ugly because we MUST set up
			 * the interrupt handler before outputting the first block
			 * of data to be written.  If we hit an error (corrupted buffer list)
			 * in ide_multwrite(), then we need to remove the handler/timer
			 * before returning.  Fortunately, this NEVER happens (right?).
			 *
			 * Except when you get an error it seems...
			 */
			hwgroup->wrq = *rq; /* scratchpad */
			ide_set_handler (drive, &multwrite_intr, WAIT_CMD, NULL);
			if (ide_multwrite(drive, drive->mult_count)) {
				unsigned long flags;
				spin_lock_irqsave(&io_request_lock, flags);
				hwgroup->handler = NULL;
				del_timer(&hwgroup->timer);
				spin_unlock_irqrestore(&io_request_lock, flags);
				return ide_stopped;
			}
		} else {
			ide_set_handler (drive, &write_intr, WAIT_CMD, NULL);
			idedisk_output_data(drive, rq->buffer, SECTOR_WORDS);
		}
		return ide_started;
	}
	printk(KERN_ERR "%s: bad command: %d\n", drive->name, rq->cmd);
	ide_end_request(0, HWGROUP(drive));
	return ide_stopped;
}

static int idedisk_open (struct inode *inode, struct file *filp, ide_drive_t *drive)
{
	MOD_INC_USE_COUNT;
	if (drive->removable && drive->usage == 1) {
		check_disk_change(inode->i_rdev);
		/*
		 * Ignore the return code from door_lock,
		 * since the open() has already succeeded,
		 * and the door_lock is irrelevant at this point.
		 */
		if (drive->doorlocking && ide_wait_cmd(drive, WIN_DOORLOCK, 0, 0, 0, NULL))
			drive->doorlocking = 0;
	}
	return 0;
}

static void idedisk_release (struct inode *inode, struct file *filp, ide_drive_t *drive)
{
	if (drive->removable && !drive->usage) {
		invalidate_bdev(inode->i_bdev, 0);
		if (drive->doorlocking && ide_wait_cmd(drive, WIN_DOORUNLOCK, 0, 0, 0, NULL))
			drive->doorlocking = 0;
	}
	MOD_DEC_USE_COUNT;
}

static int idedisk_media_change (ide_drive_t *drive)
{
	return drive->removable;	/* if removable, always assume it was changed */
}

static void idedisk_revalidate (ide_drive_t *drive)
{
	grok_partitions(HWIF(drive)->gd, drive->select.b.unit,
			1<<PARTN_BITS,
			current_capacity(drive));
}

/*
 * Compute drive->capacity, the full capacity of the drive
 * Called with drive->id != NULL.
 */
static void init_idedisk_capacity (ide_drive_t  *drive)
{
	struct hd_driveid *id = drive->id;
	unsigned long capacity = drive->cyl * drive->head * drive->sect;

	drive->select.b.lba = 0;

	/* Determine capacity, and use LBA if the drive properly supports it */
	if ((id->capability & 2) && lba_capacity_is_ok(id)) {
		capacity = id->lba_capacity;
		drive->cyl = capacity / (drive->head * drive->sect);
		drive->select.b.lba = 1;
	}
	drive->capacity = capacity;
}

static unsigned long idedisk_capacity (ide_drive_t  *drive)
{
	return (drive->capacity - drive->sect0);
}

static ide_startstop_t idedisk_special (ide_drive_t *drive)
{
	special_t *s = &drive->special;

	if (s->b.set_geometry) {
		s->b.set_geometry = 0;
		OUT_BYTE(drive->sect,IDE_SECTOR_REG);
		OUT_BYTE(drive->cyl,IDE_LCYL_REG);
		OUT_BYTE(drive->cyl>>8,IDE_HCYL_REG);
		OUT_BYTE(((drive->head-1)|drive->select.all)&0xBF,IDE_SELECT_REG);
		if (!IS_PDC4030_DRIVE)
			ide_cmd(drive, WIN_SPECIFY, drive->sect, &set_geometry_intr);
	} else if (s->b.recalibrate) {
		s->b.recalibrate = 0;
		if (!IS_PDC4030_DRIVE)
			ide_cmd(drive, WIN_RESTORE, drive->sect, &recal_intr);
	} else if (s->b.set_multmode) {
		s->b.set_multmode = 0;
		if (drive->id && drive->mult_req > drive->id->max_multsect)
			drive->mult_req = drive->id->max_multsect;
		if (!IS_PDC4030_DRIVE)
			ide_cmd(drive, WIN_SETMULT, drive->mult_req, &set_multmode_intr);
	} else if (s->all) {
		int special = s->all;
		s->all = 0;
		printk(KERN_ERR "%s: bad special flag: 0x%02x\n", drive->name, special);
		return ide_stopped;
	}
	return IS_PDC4030_DRIVE ? ide_stopped : ide_started;
}

static void idedisk_pre_reset (ide_drive_t *drive)
{
	drive->special.all = 0;
	drive->special.b.set_geometry = 1;
	drive->special.b.recalibrate  = 1;
	if (OK_TO_RESET_CONTROLLER)
		drive->mult_count = 0;
	if (!drive->keep_settings && !drive->using_dma)
		drive->mult_req = 0;
	if (drive->mult_req != drive->mult_count)
		drive->special.b.set_multmode = 1;
}

#ifdef CONFIG_PROC_FS

static int smart_enable(ide_drive_t *drive)
{
	return ide_wait_cmd(drive, WIN_SMART, 0, SMART_ENABLE, 0, NULL);
}

static int get_smart_values(ide_drive_t *drive, byte *buf)
{
	(void) smart_enable(drive);
	return ide_wait_cmd(drive, WIN_SMART, 0, SMART_READ_VALUES, 1, buf);
}

static int get_smart_thresholds(ide_drive_t *drive, byte *buf)
{
	(void) smart_enable(drive);
	return ide_wait_cmd(drive, WIN_SMART, 0, SMART_READ_THRESHOLDS, 1, buf);
}

static int proc_idedisk_read_cache
	(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	ide_drive_t	*drive = (ide_drive_t *) data;
	char		*out = page;
	int		len;

	if (drive->id)
		len = sprintf(out,"%i\n", drive->id->buf_size / 2);
	else
		len = sprintf(out,"(none)\n");
	PROC_IDE_READ_RETURN(page,start,off,count,eof,len);
}

static int proc_idedisk_read_smart_thresholds
	(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	ide_drive_t	*drive = (ide_drive_t *)data;
	int		len = 0, i = 0;

	if (!get_smart_thresholds(drive, page)) {
		unsigned short *val = ((unsigned short *)page) + 2;
		char *out = ((char *)val) + (SECTOR_WORDS * 4);
		page = out;
		do {
			out += sprintf(out, "%04x%c", le16_to_cpu(*val), (++i & 7) ? ' ' : '\n');
			val += 1;
		} while (i < (SECTOR_WORDS * 2));
		len = out - page;
	}
	PROC_IDE_READ_RETURN(page,start,off,count,eof,len);
}

static int proc_idedisk_read_smart_values
	(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	ide_drive_t	*drive = (ide_drive_t *)data;
	int		len = 0, i = 0;

	if (!get_smart_values(drive, page)) {
		unsigned short *val = ((unsigned short *)page) + 2;
		char *out = ((char *)val) + (SECTOR_WORDS * 4);
		page = out;
		do {
			out += sprintf(out, "%04x%c", le16_to_cpu(*val), (++i & 7) ? ' ' : '\n');
			val += 1;
		} while (i < (SECTOR_WORDS * 2));
		len = out - page;
	}
	PROC_IDE_READ_RETURN(page,start,off,count,eof,len);
}

static ide_proc_entry_t idedisk_proc[] = {
	{ "cache",		S_IFREG|S_IRUGO,	proc_idedisk_read_cache,		NULL },
	{ "geometry",		S_IFREG|S_IRUGO,	proc_ide_read_geometry,			NULL },
	{ "smart_values",	S_IFREG|S_IRUSR,	proc_idedisk_read_smart_values,		NULL },
	{ "smart_thresholds",	S_IFREG|S_IRUSR,	proc_idedisk_read_smart_thresholds,	NULL },
	{ NULL, 0, NULL, NULL }
};

#else

#define	idedisk_proc	NULL

#endif	/* CONFIG_PROC_FS */

static int set_multcount(ide_drive_t *drive, int arg)
{
	struct request rq;

	if (drive->special.b.set_multmode)
		return -EBUSY;
	ide_init_drive_cmd (&rq);
	drive->mult_req = arg;
	drive->special.b.set_multmode = 1;
	(void) ide_do_drive_cmd (drive, &rq, ide_wait);
	return (drive->mult_count == arg) ? 0 : -EIO;
}

static int set_nowerr(ide_drive_t *drive, int arg)
{
	if (ide_spin_wait_hwgroup(drive))
		return -EBUSY;
	drive->nowerr = arg;
	drive->bad_wstat = arg ? BAD_R_STAT : BAD_W_STAT;
	spin_unlock_irq(&io_request_lock);
	return 0;
}

static void idedisk_add_settings(ide_drive_t *drive)
{
	struct hd_driveid *id = drive->id;
	int major = HWIF(drive)->major;
	int minor = drive->select.b.unit << PARTN_BITS;

	ide_add_setting(drive,	"bios_cyl",		SETTING_RW,					-1,			-1,			TYPE_INT,	0,	65535,				1,	1,	&drive->bios_cyl,		NULL);
	ide_add_setting(drive,	"bios_head",		SETTING_RW,					-1,			-1,			TYPE_BYTE,	0,	255,				1,	1,	&drive->bios_head,		NULL);
	ide_add_setting(drive,	"bios_sect",		SETTING_RW,					-1,			-1,			TYPE_BYTE,	0,	63,				1,	1,	&drive->bios_sect,		NULL);
	ide_add_setting(drive,	"bswap",		SETTING_READ,					-1,			-1,			TYPE_BYTE,	0,	1,				1,	1,	&drive->bswap,			NULL);
	ide_add_setting(drive,	"multcount",		id ? SETTING_RW : SETTING_READ,			HDIO_GET_MULTCOUNT,	HDIO_SET_MULTCOUNT,	TYPE_BYTE,	0,	id ? id->max_multsect : 0,	1,	2,	&drive->mult_count,		set_multcount);
	ide_add_setting(drive,	"nowerr",		SETTING_RW,					HDIO_GET_NOWERR,	HDIO_SET_NOWERR,	TYPE_BYTE,	0,	1,				1,	1,	&drive->nowerr,			set_nowerr);
	ide_add_setting(drive,	"breada_readahead",	SETTING_RW,					BLKRAGET,		BLKRASET,		TYPE_INT,	0,	255,				1,	2,	&read_ahead[major],		NULL);
	ide_add_setting(drive,	"file_readahead",	SETTING_RW,					BLKFRAGET,		BLKFRASET,		TYPE_INTA,	0,	4096,			PAGE_SIZE,	1024,	&max_readahead[major][minor],	NULL);
	ide_add_setting(drive,	"max_kb_per_request",	SETTING_RW,					BLKSECTGET,		BLKSECTSET,		TYPE_INTA,	1,	255,				1,	2,	&max_sectors[major][minor],	NULL);
	ide_add_setting(drive,	"lun",			SETTING_RW,					-1,			-1,			TYPE_INT,	0,	7,				1,	1,	&drive->lun,			NULL);
	ide_add_setting(drive,	"failures",		SETTING_RW,					-1,			-1,			TYPE_INT,	0,	65535,				1,	1,	&drive->failures,		NULL);
	ide_add_setting(drive,	"max_failures",		SETTING_RW,					-1,			-1,			TYPE_INT,	0,	65535,				1,	1,	&drive->max_failures,		NULL);
}

static void idedisk_setup (ide_drive_t *drive)
{
	int i;
	
	struct hd_driveid *id = drive->id;
	unsigned long capacity;
	
	idedisk_add_settings(drive);

	if (id == NULL)
		return;

	/*
	 * CompactFlash cards and their brethern look just like hard drives
	 * to us, but they are removable and don't have a doorlock mechanism.
	 */
	if (drive->removable && !drive_is_flashcard(drive)) {
		/*
		 * Removable disks (eg. SYQUEST); ignore 'WD' drives 
		 */
		if (id->model[0] != 'W' || id->model[1] != 'D') {
			drive->doorlocking = 1;
		}
	}
	for (i = 0; i < MAX_DRIVES; ++i) {
		ide_hwif_t *hwif = HWIF(drive);

		if (drive != &hwif->drives[i]) continue;
		hwif->gd->de_arr[i] = drive->de;
		if (drive->removable)
			hwif->gd->flags[i] |= GENHD_FL_REMOVABLE;
		break;
	}

	/* Extract geometry if we did not already have one for the drive */
	if (!drive->cyl || !drive->head || !drive->sect) {
		drive->cyl     = drive->bios_cyl  = id->cyls;
		drive->head    = drive->bios_head = id->heads;
		drive->sect    = drive->bios_sect = id->sectors;
	}

	/* Handle logical geometry translation by the drive */
	if ((id->field_valid & 1) && id->cur_cyls &&
	    id->cur_heads && (id->cur_heads <= 16) && id->cur_sectors) {
		drive->cyl  = id->cur_cyls;
		drive->head = id->cur_heads;
		drive->sect = id->cur_sectors;
	}

	/* Use physical geometry if what we have still makes no sense */
	if (drive->head > 16 && id->heads && id->heads <= 16) {
		drive->cyl  = id->cyls;
		drive->head = id->heads;
		drive->sect = id->sectors;
	}

	/* calculate drive capacity, and select LBA if possible */
	init_idedisk_capacity (drive);

	/*
	 * if possible, give fdisk access to more of the drive,
	 * by correcting bios_cyls:
	 */
	capacity = idedisk_capacity (drive);
	if ((capacity >= (drive->bios_cyl * drive->bios_sect * drive->bios_head)) &&
	    (!drive->forced_geom) && drive->bios_sect && drive->bios_head)
		drive->bios_cyl = (capacity / drive->bios_sect) / drive->bios_head;

	printk (KERN_INFO "%s: %ld sectors", drive->name, capacity);

	/* Give size in megabytes (MB), not mebibytes (MiB). */
	/* We compute the exact rounded value, avoiding overflow. */
	printk (" (%ld MB)", (capacity - capacity/625 + 974)/1950);

	/* Only print cache size when it was specified */
	if (id->buf_size)
		printk (" w/%dKiB Cache", id->buf_size/2);

	printk(", CHS=%d/%d/%d", 
	       drive->bios_cyl, drive->bios_head, drive->bios_sect);
#ifdef CONFIG_BLK_DEV_IDEDMA
	if (drive->using_dma)
		(void) HWIF(drive)->dmaproc(ide_dma_verbose, drive);
#endif /* CONFIG_BLK_DEV_IDEDMA */
	printk("\n");

	drive->mult_count = 0;
	if (id->max_multsect) {
#ifdef CONFIG_IDEDISK_MULTI_MODE
		id->multsect = ((id->max_multsect/2) > 1) ? id->max_multsect : 0;
		id->multsect_valid = id->multsect ? 1 : 0;
		drive->mult_req = id->multsect_valid ? id->max_multsect : INITIAL_MULT_COUNT;
		drive->special.b.set_multmode = drive->mult_req ? 1 : 0;
#else	/* original, pre IDE-NFG, per request of AC */
		drive->mult_req = INITIAL_MULT_COUNT;
		if (drive->mult_req > id->max_multsect)
			drive->mult_req = id->max_multsect;
		if (drive->mult_req || ((id->multsect_valid & 1) && id->multsect))
			drive->special.b.set_multmode = 1;
#endif
	}
	drive->no_io_32bit = id->dword_io ? 1 : 0;
}

static int idedisk_reinit (ide_drive_t *drive)
{
	return 0;
}

static int idedisk_cleanup (ide_drive_t *drive)
{
	return ide_unregister_subdriver(drive);
}

/*
 *      IDE subdriver functions, registered with ide.c
 */
static ide_driver_t idedisk_driver = {
	name:			"ide-disk",
	version:		IDEDISK_VERSION,
	media:			ide_disk,
	busy:			0,
	supports_dma:		1,
	supports_dsc_overlap:	0,
	cleanup:		idedisk_cleanup,
	do_request:		do_rw_disk,
	end_request:		NULL,
	ioctl:			NULL,
	open:			idedisk_open,
	release:		idedisk_release,
	media_change:		idedisk_media_change,
	revalidate:		idedisk_revalidate,
	pre_reset:		idedisk_pre_reset,
	capacity:		idedisk_capacity,
	special:		idedisk_special,
	proc:			idedisk_proc,
	driver_reinit:		idedisk_reinit,
};

int idedisk_init (void);
static ide_module_t idedisk_module = {
	IDE_DRIVER_MODULE,
	idedisk_init,
	&idedisk_driver,
	NULL
};

MODULE_DESCRIPTION("ATA DISK Driver");

static void __exit idedisk_exit (void)
{
	ide_drive_t *drive;
	int failed = 0;

	while ((drive = ide_scan_devices (ide_disk, idedisk_driver.name, &idedisk_driver, failed)) != NULL) {
		if (idedisk_cleanup (drive)) {
			printk (KERN_ERR "%s: cleanup_module() called while still busy\n", drive->name);
			failed++;
		}
		/* We must remove proc entries defined in this module.
		   Otherwise we oops while accessing these entries */
#ifdef CONFIG_PROC_FS
		if (drive->proc)
			ide_remove_proc_entries(drive->proc, idedisk_proc);
#endif
	}
	ide_unregister_module(&idedisk_module);
}

int idedisk_init (void)
{
	ide_drive_t *drive;
	int failed = 0;
	
	MOD_INC_USE_COUNT;
	while ((drive = ide_scan_devices (ide_disk, idedisk_driver.name, NULL, failed++)) != NULL) {
		if (ide_register_subdriver (drive, &idedisk_driver, IDE_SUBDRIVER_VERSION)) {
			printk (KERN_ERR "ide-disk: %s: Failed to register the driver with ide.c\n", drive->name);
			continue;
		}
		idedisk_setup(drive);
		if ((!drive->head || drive->head > 16) && !drive->select.b.lba) {
			printk(KERN_ERR "%s: INVALID GEOMETRY: %d PHYSICAL HEADS?\n", drive->name, drive->head);
			(void) idedisk_cleanup(drive);
			continue;
		}
		failed--;
	}
	ide_register_module(&idedisk_module);
	MOD_DEC_USE_COUNT;
	return 0;
}

module_init(idedisk_init);
module_exit(idedisk_exit);
MODULE_LICENSE("GPL");
