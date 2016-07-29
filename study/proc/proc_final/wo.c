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



char    ro_buffer[MAX_LINE] =
/*
*
*/
int proc_write_ctl( struct file *file,
                    const char *buffer,
                    unsigned long count,
                    void * data)
{
	int iRet;
	char sCtl[MAX_LINE];

	if(count <= 0)
    {
        return FAIL;
    }

    memset(sCtl, '\0', sizeof(sCtl));

    iRet = copy_from_user(sCtl, buffer, count);
	if(iRet)
    {
        return FAIL;
    }

    iRet = sscanf(sCtl,"%d",&ctl);
	if(iRet != 1)
    {
        return FAIL;
    }
    dbginfo("Rcv ctl : %d\n", ctl);
    do_request( );

    return count;
}

const struct file_operations proc_ctl_fops =
{
    .owner = THIS_MODULE,
    //.read  = proc_read_ctl,                       // can read
    .write = proc_write_ctl,                        // write only
};
