/*
	2013.1.31byTX
*/
#include <linux/kernel.h>#include <asm/io.h>
#include "slaveSerial.h"


/*
assert cclk when value>1
*/
void set_cclk(void *io_base,int value)
{
	iowrite16((value>0)?1:0,io_base+REG_CCLK_OFFSET);
}
/*
assert din when value>0
*/
void set_din(void *io_base,int value)
{
	iowrite16((value>0)?1:0,io_base+REG_DIN_OFFSET);
}
/*
assert prog_b when value>0
*/
void set_prog_b(void *io_base,int value)
{
	iowrite16((value>0)?1:0,io_base+REG_PROG_B_OFFSET);
}
/*

*/
unsigned short int get_stat(void *io_base)
{
	return ioread16(io_base+REG_STAT_OFFSET);
}
//返回pin状态
int get_done_bit(void *io_base)
{
	return ((get_stat(io_base)& STAT_DONE_BIT)>0)?1:0;
}
//返回pin状态
int get_init_bit(void *io_base)
{
	return ((get_stat(io_base)& STAT_INIT_BIT)>0)?1:0;
}
int check_done(void *io_base)
{
	int i;
	int done=0;
	int init=1;
	//check init first!!
	if(0==get_init_bit(io_base))
	{
		printk("init=0,so failed\n");
		return INFOR_DONE_FAILED;
	}
	while((done==0) && (init==1))
	{
		//Apply additional CCLK pulse until DONE=1
		set_cclk(io_base,0);
		set_cclk(io_base,1);
		done=get_done_bit(io_base);
		init=get_init_bit(io_base);
	}
	if(done>0)
	{
		//printk("done=1, succeed!\n");
		for(i=0;i<8;i++)
		{
			set_cclk(io_base,0);
			set_cclk(io_base,1);
		}
		return INFOR_DONE_SUCCEED;
	}
	else
	{
		printk("failed!done=%d,init=%d\n",done, init);
		return INFOR_DONE_FAILED;
	}
}
void shiftWordOut(unsigned short int data, void *io_base)
{
	int i;
	//low byte
	for(i=0;i<8;i=i+1)	{
		set_cclk(io_base,0);//set cclk low
		set_din(io_base,(data&(0x0080>>i))?1:0);
		set_cclk(io_base,1);//set cclk high
	}
	//high byte
	for(i=0;i<8;i=i+1)
	{
		set_cclk(io_base,0);//set cclk low
		set_din(io_base,(data&(0x8000>>i))?1:0);
		set_cclk(io_base,1);//set cclk high
	}
}
void shiftByteOut(char data, void *io_base)
{
	int i;
	for(i=0;i<8;i=i+1)	{
		set_cclk(io_base,0);//set cclk low
		set_din(io_base,(data&(0x80>>i))?1:0);
		set_cclk(io_base,1);//set cclk high
	}
}



