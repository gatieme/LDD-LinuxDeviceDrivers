/*
*  Author: HIT CS HDMC team.
*  Create: 2010-3-12 8:50
*  Last modified: 2010-6-13 14:06:20
*  Description:
*  	Memory fault injection engine running as a kernel module.
*		This module will create "/proc/memoryEngine/" directory and 9 proc nodes.
*   Write rwramenters and request to these proc nodes and read the output from related proc node.
*/

#include "main.h"

unsigned long   rw_data;

/*
*
*/
int proc_write_rw(struct file *file, const char *buffer, unsigned long count, void * data)
{
	int iRet;
	char srw[MAX_LINE];

	if(count <= 0) { return FAIL; }
	memset(srw, '\0', sizeof(srw));

    iRet = copy_from_user(srw, buffer, count);

    if(iRet) { return FAIL; }

    iRet = sscanf(srw,"%lx",&rw_data);

    if(iRet != 1) { return FAIL; }

    printk(KERN_INFO "Rcv rw : 0x%lx\n", rw_data);
	return count;
}

#ifndef  PROC_SEQ_FILE_OPERATIONS
/*
*
*/
int proc_read_rw(char * rwge,char **start, off_t off, int count, int * eof,void * data)
{
	int iLen;
    printk("0x%lx\n", rw_data);
	iLen = sprintf(rwge, "%lx", rw_data);
	return iLen;
}

const struct file_operations proc_rw_fops =
{
    .owner = THIS_MODULE,
    .read  = proc_read_rw,                         // read
	.write = proc_write_rw,                        // write
};

#else

// seq_operations -> show
static int seq_show_rw(struct seq_file *m, void *v)
{
    /*
	char buf[MAX_LINE];
	int ret = 0;
    printk("0x%lx\n", rw_data);
	ret = sprintf(buf, "%lx", rw_data);
    */

    printk("0x%lx\n", rw_data);
	seq_printf(m, "%lx", rw_data);

	return 0; //!! must be 0, or will show nothing T.T
}


// seq_operations -> open
static int proc_open_rw(struct inode *inode, struct file *file)
{
	return single_open(file, seq_show_rw, NULL);
}

const struct file_operations proc_rw_fops =
{
	.owner		= THIS_MODULE,
	.open		= proc_open_rw,
	.read		= seq_read,
	.write 		= proc_write_rw,
	.llseek		= seq_lseek,
	.release	= single_release,
};

#endif
