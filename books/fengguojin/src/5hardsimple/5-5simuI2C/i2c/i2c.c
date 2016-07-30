
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/configfs.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/delay.h>

#include <asm/uaccess.h>
#include <asm/io.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/signal.h>
#include <asm/hardware.h>
#include <asm/uaccess.h>
#include <asm/arch/regs-gpio.h>

#define DEBUG_SHOW 1
#define BUFFERSIZE 4
#define DEVICE_NAME  "i2c"

int dev_MAJOR=235;

#define DEVICE_ADD 0x50
#define READ_DEV_ADDR ((DEVICE_ADD<<1)+1)
#define WRITE_DEV_ADDR (DEVICE_ADD<<1)

#define I2C_READ_DATA  1
#define I2C_WRITE_DATA 2


struct I2C_MSGbuffer
{
	unsigned short len;
	unsigned char addr;
	char buffer[64];
};

void SetSDAOut()
{
	__raw_writel((__raw_readl(S3C2410_GPECON)&(~(1<<31)))|(1<<30),S3C2410_GPECON);
	__raw_writel((readl(S3C2410_GPEUP)|(1<<15)),S3C2410_GPEUP);
}

void SetSDAInput()
{
	__raw_writel((__raw_readl(S3C2410_GPECON)&(~(3<<30))),S3C2410_GPECON);
}

void SetSCLOut()
{
	__raw_writel((__raw_readl(S3C2410_GPECON)&(~(1<<29)))|(1<<28),S3C2410_GPECON);
}

void SetSCLInput()
{
	__raw_writel((__raw_readl(S3C2410_GPECON)&(~(3<<28))),S3C2410_GPECON);
}

int GetSDAValue()
{
	int ret=__raw_readl(S3C2410_GPEDAT)&(1<<15);
	if(ret==0)
		return 0;
	else
	    return 1;
}

void SetSDAHigh()
{
	__raw_writel((__raw_readl(S3C2410_GPEDAT)|(1<<15)),S3C2410_GPEDAT);
}

void SetSDALow()
{
	__raw_writel((__raw_readl(S3C2410_GPEDAT)&(~(1<<15))),S3C2410_GPEDAT);
}

void SetSCLHigh()
{
	__raw_writel((__raw_readl(S3C2410_GPEDAT)|(1<<14)),S3C2410_GPEDAT);
}

void SetSCLLow()
{
	__raw_writel((__raw_readl(S3C2410_GPEDAT)&(~(1<<14))),S3C2410_GPEDAT);
}

void I2CStart(void)
{
	SetSDAHigh();
	SetSCLHigh();
    udelay(40);
	SetSDALow();
	udelay(100);
	SetSCLLow();
	udelay(80);
}


void I2CStop(void)
{
	SetSDALow();
	udelay(60);
	SetSCLHigh();
	udelay(80);
	SetSDAHigh();
}

int WaitAck(void)
{
	int isum=2000;
	int ret=0;
	udelay(1);
    SetSDAInput();
	SetSCLHigh();
	udelay(10);
	while(isum>0)
	{
		ret=GetSDAValue();
		if(ret==0)
		{
			break;
		}
		udelay(2);
		isum--;
	}
	SetSCLLow();
	SetSDAOut();
	udelay(2);
	return ret;
}

void SendAck(void)
{
	SetSDALow();
	SetSCLHigh();
	udelay(8);
	SetSCLLow();
	SetSDAHigh();
}


 void SendNotAck(void)
 {
	SetSDAHigh();
	SetSCLHigh();
	udelay(8);
	SetSCLLow();
 }


void I2CSendByte(unsigned char ch)
{
    unsigned char i=8;
      while (i--)
      {
		    if(ch&0x80)
		    {
				SetSDAHigh();
		    }
			else
			{
				SetSDALow();
			}
			udelay(10);
			SetSCLHigh();
			udelay(100);
			SetSCLLow();
			udelay(80);
			ch=ch<<1;
      }
	  SetSDAHigh();
}

unsigned char I2CReceiveByte(void)
{
	  int ret=0;
      unsigned char i=8;
	  unsigned char data=0;
      SetSDAHigh();
      SetSDAInput();
	  udelay(20);
      while (i--)
      {
    	    data<<=1;
			SetSCLHigh();
			udelay(60);
			ret=GetSDAValue();
			data|=ret;
			udelay(20);
			SetSCLLow();
			udelay(60);
		}
	  SetSDAOut();
      return data;
}

int writeOnedate(unsigned char addr,unsigned char ch)
{
	int ret=0;
	I2CStart();
	I2CSendByte(WRITE_DEV_ADDR);
	ret=WaitAck();
#if DEBUG_SHOW
	printk("writedate dev addr ACK:%d\n",ret);
#endif
	if(ret==0)
	{
		I2CSendByte(addr);
		ret=WaitAck();
#if DEBUG_SHOW
		printk("writedate data addr 0x%.2x ACK:%d\n",addr,ret);
#endif
		if(ret==0)
		{
			I2CSendByte(ch);
			ret=WaitAck();
#if DEBUG_SHOW
			printk("writedate data 0x%.2x ACK:%d\n",ch,ret);
#endif
		}
	}
	I2CStop();
	mdelay(10);
	return ret;
}

int readOnedate(unsigned char addr,unsigned char*ch)
{
	int ret=0;
	I2CStart();
	I2CSendByte(WRITE_DEV_ADDR);
	ret=WaitAck();
#if DEBUG_SHOW
	printk("readdate dev addr ACK:%d\n",ret);
#endif
	if(ret==0)
	{
		I2CSendByte(addr);
		ret=WaitAck();
#if DEBUG_SHOW
		printk("readdate data addr 0x%.2x ACK:%d\n",addr,ret);
#endif
		if(ret==0)
		{
			I2CStop();
			I2CStart();
			I2CSendByte(READ_DEV_ADDR);
			ret=WaitAck();
#if DEBUG_SHOW
			printk("readdate dev addr ACK:%d\n",ret);
#endif
			if(ret==0)
			{
				*ch=I2CReceiveByte();
#if DEBUG_SHOW
				printk("readdate data:0x%.2x\n",*ch);
#endif
				SendNotAck();
			}
		}
	}
	I2CStop();
	return ret;
}

int readThreeTimes(unsigned char addr,unsigned char*ch)
{
	int i=3;
	while(i>0)
	{
		if(readOnedate(addr,ch)==0)
		{
			break;
		}
		i--;
	}
	if(i==0)return 0;
	return 1;
}

int writeThreeTimes(unsigned char addr,unsigned char ch)
{
	int i=3;
	while(i>0)
	{
		if(writeOnedate(addr,ch)==0)
		{
			break;
		}
		i--;
	}
	if(i==0)return 0;
	return 1;
}


static int I2C_open(struct inode *inode, struct file *filp)
{
	int ret=0;

	SetSDAOut();
	SetSCLOut();

	return ret;
}

static int I2C_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	switch (cmd)
	{
	case I2C_READ_DATA:
		{
			int i=0;
			struct I2C_MSGbuffer imsg;
			if (copy_from_user(&imsg,(char*)arg, sizeof(struct I2C_MSGbuffer)))
			{
				return -EFAULT;
			}
			for(i=0;i<imsg.len;i++)
			{
				if(readThreeTimes(imsg.addr+i,&imsg.buffer[i])==0)
				{
					break;
				}
			}
			if (copy_to_user((char*)arg,&imsg, sizeof(struct I2C_MSGbuffer)))
			{
				return -EFAULT;
			}
			return i;
		}
		break;
	case I2C_WRITE_DATA:
		{
			int i=0;
			struct I2C_MSGbuffer imsg;
			if (copy_from_user(&imsg,(char*)arg, sizeof(struct I2C_MSGbuffer)))
			{
				return -EFAULT;
			}
			for(i=0;i<imsg.len;i++)
			{
				if(writeThreeTimes(imsg.addr+i,imsg.buffer[i])==0)
				{
					break;
				}
			}
			return i;
		}
		break;
	default:
		break;
	}
	return 0;
}

static int I2C_release(struct inode *inode,struct file *filp)
{
	return 0;
}


static struct file_operations I2C_fops={
	.owner = THIS_MODULE,
	.open = I2C_open,
	.ioctl = I2C_ioctl,
	.release = I2C_release,
};

static int __init I2C_init(void)
{
	int ret;
	ret = register_chrdev(dev_MAJOR,DEVICE_NAME,&I2C_fops);
	if(ret<0)
	{	
		printk("i2c can't get the major number...\n");
		return ret;
	}

	printk("i2c module init...\n");
	return 0;
}
 
static void __exit I2C_exit(void)
{
	unregister_chrdev(dev_MAJOR,DEVICE_NAME);
	printk("i2c module exit...\n");
}	  

module_init(I2C_init);
module_exit(I2C_exit);

MODULE_AUTHOR("fgjnew <fgjnew@163.com>");
MODULE_DESCRIPTION("S3C2410 I2C Test Driver");
MODULE_LICENSE("GPL");