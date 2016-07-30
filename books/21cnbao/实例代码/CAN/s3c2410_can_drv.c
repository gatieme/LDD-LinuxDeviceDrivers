/

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/interrupt.h>    /* for in_interrupt */
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/delay.h>        /* for udelay */
#include <linux/modversions.h>
#include <linux/version.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/hardware.h>

#include <asm/uaccess.h>

#include "can_bus.h"


#define IRQ_BUTTON_CTL 		IRQ_EINT19


#define canbus_MAJOR    98
#define DEVICE_NAME "can_bus"


#define IRQ_CAN_BUS IRQ_EINT2
#define GPIO_CAN_BUS_EINT2           (GPIO_MODE_ALT0 | GPIO_PULLUP_EN | GPIO_F2) //For ringing detect


#define DATA_GONE       1
#define DATA_NOT_GONE   0

static int READ_DATA_STATUS = DATA_NOT_GONE;

wait_queue_head_t       CAN_wq;	//define the wait queue

devfs_handle_t  dev_handle;     /* register handle to store device fs */

static void CAN_BUS_interrupt(int nr, void *devid, struct pt_regs *regs);
static int CAN_BUS_temp_count=0;


//define for the cpld
static int CPLD_CTL_ADDR;
static int XGPIO_OUT_ADDR;
static int ADDR_SJA1000_ADDR;
static int ADDR_SJA1000_DATA;

ssize_t canbus_read (struct file * file ,char * buf, size_t count, loff_t * f_ops)
{	
	int ret,i;
	char CAN_BUS_BUF[11];
        DECLARE_WAITQUEUE(wait, current);	// declare the wait queue
	#ifdef BUTTON_DEBUG
        printk ("canbus_read\n");
        #endif

	enable_irq(IRQ_CAN_BUS);
	READ_DATA_STATUS = DATA_GONE;	//If you don't do this ,the interrupt may come before you begain to read,and
					//it may die here.
	sleep_on(&CAN_wq);
	disable_irq(IRQ_CAN_BUS);
	#ifdef BUTTON_DEBUG
        printk("read interrupt\n");
        #endif
	
	// Read the data
        CAN_BUS_BUF[1] = read_can_reg(RxBuffer1);
        CAN_BUS_BUF[2] = read_can_reg(RxBuffer2);
        CAN_BUS_BUF[3] = read_can_reg(RxBuffer3);
        CAN_BUS_BUF[4] = read_can_reg(RxBuffer4);
        CAN_BUS_BUF[5] = read_can_reg(RxBuffer5);
        CAN_BUS_BUF[6] = read_can_reg(RxBuffer6);
        CAN_BUS_BUF[7] = read_can_reg(RxBuffer7);
        CAN_BUS_BUF[8] = read_can_reg(RxBuffer8);
        CAN_BUS_BUF[9] = read_can_reg(RxBuffer9);
        CAN_BUS_BUF[10] = read_can_reg(RxBuffer10);

        write_can_reg(0x04, CommandReg);


//******************************************************************
        copy_to_user(buf,CAN_BUS_BUF,11);

        return 0 ;
}


ssize_t canbus_write (struct file * file ,const char * buf, size_t count, loff_t * f_ops)
{
	#ifdef LIDUO_CAN_DEBUG
                printk ("canbus_write [ --kernel--]\n");
        #endif


        write_can_reg(buf[1], TxBuffer1);
        write_can_reg(buf[2], TxBuffer2);
        write_can_reg(buf[3], TxBuffer3);
        write_can_reg(buf[4] , TxBuffer4);
        write_can_reg(buf[5] , TxBuffer5);
        write_can_reg(buf[6] , TxBuffer6);
        write_can_reg(buf[7] , TxBuffer7);
        write_can_reg(buf[8] , TxBuffer8);
        write_can_reg(buf[9] , TxBuffer9);
        write_can_reg(buf[10] , TxBuffer10);

        write_can_reg(0x01, CommandReg);

        return 0;

}

ssize_t canbus_ioctl (struct inode * inode ,struct file * file, unsigned int cmd, long data)
{
     
        return 0;
}

ssize_t canbus_open (struct inode * inode ,struct file * file)
{
        int ret;

        #ifdef BUTTON_DEBUG
        printk ("canbus_open\n");
        #endif

        MOD_INC_USE_COUNT;

        
        init_waitqueue_head(&CAN_wq);	//initialize the queue

        return 0;
}

ssize_t canbus_release (struct inode  * inode ,struct file * file)
{
       

        MOD_DEC_USE_COUNT;

        return 0;
}



// -------------------------------------------------
struct file_operations CAN_ctl_ops ={


        open:           canbus_open,

        read:           canbus_read,

        write:          canbus_write,

        ioctl:          canbus_ioctl,

        release:        canbus_release,

};

static void CAN_BUS_interrupt(int nr, void *devid, struct pt_regs *regs)
{

      
        #ifdef BUTTON_DEBUG
        printk ("canbus_interrupt\n");
        printk(".....StatusReg is %x\n", read_can_reg(StatusReg));
        printk(".....InterruptReg is %x\n", read_can_reg(InterruptReg));
        printk(".....ControlReg is %x\n", read_can_reg(ControlReg));
	#endif

	read_can_reg(StatusReg);
	read_can_reg(InterruptReg);
	read_can_reg(ControlReg);
	

        if (READ_DATA_STATUS == DATA_GONE)
        {
                READ_DATA_STATUS = DATA_NOT_GONE;
	#ifdef  DEBUG
                printk("INT call read!\n");
	#endif
	        wake_up(&CAN_wq);

        }

}

int init_sja1000(void)
{

        int i;

        printk("ControlReg is %x\n", read_can_reg(ControlReg));
        write_can_reg(0x01, ControlReg);
        printk("ControlReg is %x\n", read_can_reg(ControlReg));

        printk("StatusReg is %x\n", read_can_reg(StatusReg));

        udelay(10);


        printk("ClockDivideReg is %x\n", read_can_reg(ClockDivideReg));
        write_can_reg(0x48, ClockDivideReg);
        printk("ClockDivideReg is %x\n", read_can_reg(ClockDivideReg));

        printk("StatusReg is %x\n", read_can_reg(StatusReg));

        udelay(10);


        printk("AcceptCodeReg is %x\n", read_can_reg(AcceptCodeReg));
        write_can_reg(0, AcceptCodeReg);
        printk("AcceptCodeReg is %x\n", read_can_reg(AcceptCodeReg));

        printk("AcceptMaskReg is %x\n", read_can_reg(AcceptMaskReg));
        write_can_reg(0xff, AcceptMaskReg);
        printk("AcceptMaskReg is %x\n", read_can_reg(AcceptMaskReg));

        printk("BusTiming0Reg is %x\n", read_can_reg(BusTiming0Reg));
        write_can_reg(0x85, BusTiming0Reg);
        printk("BusTiming0Reg is %x\n", read_can_reg(BusTiming0Reg));

        printk("BusTiming1Reg is %x\n", read_can_reg(BusTiming1Reg));
        write_can_reg(0xb4, BusTiming1Reg);
        printk("BusTiming1Reg is %x\n", read_can_reg(BusTiming1Reg));

        printk("OutControlReg is %x\n", read_can_reg(OutControlReg));
        write_can_reg(0x1a, OutControlReg);
        printk("OutControlReg is %x\n", read_can_reg(OutControlReg));

        printk("ControlReg is %x\n", read_can_reg(ControlReg));
        write_can_reg(0x1e, ControlReg);
        printk("ControlReg is %x\n", read_can_reg(ControlReg));

	udelay(10);

        printk("StatusReg is %x\n", read_can_reg(StatusReg));

        udelay(10);


        return 0;

}

     

static int __init HW_CAN_CTL_init(void)
{
    int ret = -ENODEV;
    int delay ;


        unsigned int cpld_ctl_data = 0;

        
        char can_data;
        int i;
        char temp_rw_addr;
       
        CAN_BUS_temp_count=0;

        // set bus width
        BWSCON = (BWSCON & ~(BWSCON_ST5 | BWSCON_WS5 | BWSCON_DW5)) | (BWSCON_ST5 | BWSCON_WS5 | BWSCON_DW(5, BWSCON_DW_8));
        //BANKCON5= BANKCON_Tacs0 | BANKCON_Tcos4 | BANKCON_Tacc14 | BANKCON_Toch1 | BANKCON_Tcah4 | BANKCON_Tacp6 | BANKCON_PMC1;


        // ---------------------------------------------------------------------------

	// ----------------- add for can bus ctl , cpld init
        CPLD_CTL_ADDR   = ioremap(0x21000000,0x0f);
        XGPIO_OUT_ADDR  = ioremap(0x21400000,0x0f);
        //printk("XGPIO_OUT_ADDR = %x\n",XGPIO_OUT_ADDR);

        // set cpld, connect can bus interrupt pin to cpu
        cpld_ctl_data = inw(CPLD_CTL_ADDR);
       
        cpld_ctl_data |= CAN_BUS_INT_ENABLE;

        

        outw(cpld_ctl_data,CPLD_CTL_ADDR);

        cpld_ctl_data = inw(CPLD_CTL_ADDR);

        outw(cpld_ctl_data|0x20, CPLD_CTL_ADDR);

        cpld_ctl_data = inw(CPLD_CTL_ADDR);

        


        // set cpld, let xgpio22 output = 1, set the sja1000 work in Intel mode
        cpld_ctl_data = inw(XGPIO_OUT_ADDR);
       

        cpld_ctl_data |= CAN_BUS_MODE;

        

        outw(cpld_ctl_data,XGPIO_OUT_ADDR);
        // -----------------

        ADDR_SJA1000_ADDR = ioremap(0x28000002,0x01);
        ADDR_SJA1000_DATA = ioremap(0x28000003,0x01);

        printk("ADDR_SJA1000_ADDR = %x\n",ADDR_SJA1000_ADDR);
        printk("ADDR_SJA1000_DATA = %x\n",ADDR_SJA1000_DATA);

//*************************************************************************************************************
		

        set_external_irq(IRQ_CAN_BUS, EXT_FALLING_EDGE, GPIO_PULLUP_EN);
	set_gpio_ctrl(GPIO_CAN_BUS_EINT2);
        ret = devfs_register_chrdev(canbus_MAJOR, DEVICE_NAME, &CAN_ctl_ops);

        if( ret < 0 )
        {
                printk (" s3c2410: init_module failed with %d\n", ret);
                return ret;
        }
        else
        {
                printk(" S3c2410 canbus register success!!!\n");
        }

        dev_handle = devfs_register( NULL, DEVICE_NAME, DEVFS_FL_DEFAULT, canbus_MAJOR, 0, S_IFCHR, &CAN_ctl_ops, NULL);
        
	ret = request_irq(IRQ_CAN_BUS, CAN_BUS_interrupt, SA_INTERRUPT, "CAN_BUS", NULL);
        
	if (ret)
        {
                printk(KERN_INFO "request CANBUS IRQ failed (%d)\n", IRQ_CAN_BUS);
                return ret;
        }

        disable_irq(IRQ_BUTTON_CTL);

        #ifdef BUTTON_DEBUG
        printk("ret=%d\n",ret);
        #endif
	
	init_sja1000();

        return ret;
}




static int __init s3c2410_CAN_CTL_init(void)
{
    int  ret = -ENODEV;

    ret = HW_CAN_CTL_init();
    if (ret)
      return ret;
    return 0;

}



static void __exit cleanup_can_ctl(void)
{
	#ifdef BUTTON_DEBUG
        printk ("cleanup_canbus\n");
        #endif

        free_irq(IRQ_CAN_BUS, NULL);
	devfs_unregister_chrdev (canbus_MAJOR, DEVICE_NAME );
        devfs_unregister (dev_handle);

}


MODULE_DESCRIPTION("canbus control module");
MODULE_LICENSE("GPL");

module_init(s3c2410_CAN_CTL_init);
module_exit(cleanup_can_ctl);

