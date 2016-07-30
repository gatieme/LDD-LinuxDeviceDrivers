/*
*  Author: HIT CS HDMC team.
*  Create: 2010-3-12 8:50
*  Last modified: 2010-6-13 14:06:20
*  Description:
*  	Memory fault injection engine running as a kernel module.
*		This module will create "/proc/memoryEngine/" directory and 9 proc nodes.
*   Write paramenters and request to these proc nodes and read the output from related proc node.
*/

#include "main.h"

/*
*	proc entries
*/



char    wo_buffer[MAX_LINE] = "WRITE ONLY!!!";
//unsigned long wo_data;
/*
*
*/
int proc_write_wo( struct file *file,
                    const char *buffer,
                    unsigned long count,
                    void * data)
{
	int iRet;
	char temp[MAX_LINE];

	if(count <= 0)
    {
        return FAIL;
    }

    memset(temp, '\0', sizeof(temp));

    iRet = copy_from_user(temp, buffer, count);
	if(iRet)
    {
        return FAIL;
    }

    iRet = sscanf(temp,"%s",wo_buffer);
	if(iRet != 1)
    {
        return FAIL;
    }
    printk(KERN_INFO "Rcv wo : %s\n", wo_buffer);
    /////////////////
    //  do something
    /////////////////

    return count;
}

const struct file_operations proc_wo_fops =
{
    .owner = THIS_MODULE,
    //.read  = proc_read_wo,                       // can read
    .write = proc_write_wo,                        // write only
};
