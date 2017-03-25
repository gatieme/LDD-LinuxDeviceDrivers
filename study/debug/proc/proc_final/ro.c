/*
*  Author: HIT CS HDMC team.
*  Create: 2010-3-12 8:50
*  Last modified: 2010-6-13 14:06:20
*  Description:
*  	Memory fault injection engine running as a kernel module.
*		This module will create "/proc/memoryEngine/" directory and 9 proc nodes.
*   Write roramenters and request to these proc nodes and read the output from related proc node.
*/

#include "main.h"






char ro_buffer[MAX_LINE] = "READ ONLY!!!";


#ifndef  PROC_SEQ_FILE_OPERATIONS

/*
*
*/
int proc_read_ro(char * page,char **start, off_t off, int count, int * eof,void * data)
{
	int iLen;
	iLen = sprintf(page, "%s", ro_buffer);
	return iLen;
}

const struct file_operations ro_fops =
{
    .owner = THIS_MODULE,
    .read  = proc_read_ro,                    // read only
    //.write = proc_write_ro,                        // write only
};

#else

// seq_operations -> show
static int seq_show_ro(struct seq_file *m, void *v)
{
	/*
    char buf[MAX_LINE];
	int ret = 0;

    ret = sprintf(buf, "%ls", ack_ro);
    */

	seq_printf(m, "%s", ro_buffer);

	return 0; //!! must be 0, or will show nothing T.T
}


// seq_operations -> open
static int proc_open_ro(struct inode *inode, struct file *file)
{
	return single_open(file, seq_show_ro, NULL);
}

const struct file_operations proc_ro_fops =
{
	.owner		= THIS_MODULE,
	.open		= proc_open_ro,
	.read		= seq_read,
	//.write 		= proc_write_ro,
	.llseek		= seq_lseek,
	.release	= single_release,

};

#endif
