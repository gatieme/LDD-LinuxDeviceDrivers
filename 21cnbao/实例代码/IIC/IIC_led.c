
#define MODULE	
#include <linux/module.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/miscdevice.h>
#include <linux/init.h>
#include <linux/compiler.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/irq.h>

#include "IIC.h"

#define ZLG7290_SLA_ADD	0x70	//the slave address of ZLG7290:ic which drivers leds.
#define LED_SLA_ADD	0x70	//another name of the slave address
#define LED_MAJOR	100	//the major device number of led


#define TIME_TURN 	0	//turn to display time
#define DATE_TURN 	1	//turn to display date

devfs_handle_t  dev_handle;     /* register handle to store device fs */



//buffers for 8 leds:

//..[0]: the zlg790's slave address
//..[1]: the commad buffer's address of zlg7290
//..[2]: the first command
//..[3]: the second command
//..[4]: the end symbol
signed char sec_buf_l[5]={LED_SLA_ADD,0x07,0x63,-1,-1};
signed char sec_buf_h[5]={LED_SLA_ADD,0x07,0x62,-1,-1};
signed char blank_buf_l[5]={LED_SLA_ADD,0X07,0X61,0x1f,-1};
signed char min_buf_l[5]={LED_SLA_ADD,0x07,0x60,-1,-1};
signed char min_buf_h[5]={LED_SLA_ADD,0x07,0x67,-1,-1};
signed char blank_buf_h[5]={LED_SLA_ADD,0X07,0X66,0x1f,-1};
signed char hour_buf_l[5]={LED_SLA_ADD,0x07,0x65,-1,-1};
signed char hour_buf_h[5]={LED_SLA_ADD,0x07,0x64,-1,-1};
//buffers for 8 leds:
signed char date_buf_l[5]={LED_SLA_ADD,0x07,0x63,-1,-1};
signed char date_buf_h[5]={LED_SLA_ADD,0x07,0x62,-1,-1};
signed char mon_buf_l[5]={LED_SLA_ADD,0x07,0x61,-1,-1};
signed char mon_buf_h[5]={LED_SLA_ADD,0x07,0x60,-1,-1};
signed char year_buf_0[5]={LED_SLA_ADD,0x07,0x64,2,-1};
signed char year_buf_1[5]={LED_SLA_ADD,0x07,0x65,0,-1};
signed char year_buf_2[5]={LED_SLA_ADD,0x07,0x66,-1,-1};
signed char year_buf_3[5]={LED_SLA_ADD,0x07,0x67,-1,-1};


int date_time_turn = TIME_TURN;	//turn to display time first
signed int date_s = 0;		//the seconds how long date has been displayed
signed int time_s = 0;		//the seconds how long time has been displayed


//time structure
static struct my_time{
	char second;
	char minute;
	char hour;
	char date;
	char month;
	char year;
}led_time;




/*
 * write the cpu's rtc!
 * you can change the time and date
 */
static void led_set_time(void)
{
	RTCCON = 0X01;	//get the right to write rtc
	BCDYEAR = 0x04;	//2004.9.26 11:49:30
	BCDMON = 0x09;
	BCDDATE = 0x26;
	BCDHOUR = 0x11;
	BCDMIN = 0x49;
	BCDSEC = 0x30;
//	printk("HAHAHAHA!!%d\n",RTCCON);
	RTCCON = 0X00;	//free the right to write rtc
}



/*
 * get the date and time from rtc
 */
static void led_get_time(struct my_time *tempt)
{
	tempt->second = BCDSEC;
	tempt->minute = BCDMIN;
	tempt->hour   = BCDHOUR;
	tempt->date   = BCDDATE;
	tempt->month  = BCDMON;
	tempt->year   = BCDYEAR; 
}



/*
 * get the correct format of the time and date
 * see the cpu 2410's doc for more details of the RTC registers
 */
static void led_time_format(struct my_time *tempt)
{
	sec_buf_l[3] = (tempt->second & 0x0f);
	sec_buf_h[3] = (tempt->second & 0x70) >> 4;
	min_buf_l[3] = (tempt->minute & 0x0f) | 0x80;
	min_buf_h[3] = (tempt->minute & 0x70) >> 4;
	hour_buf_l[3] = (tempt->hour & 0x0f) |0x80;
	hour_buf_h[3] = (tempt->hour & 0x30) >> 4;

   	date_buf_l[3] = (tempt->date & 0x0f);
	date_buf_h[3] = (tempt->date & 0x30) >> 4;
	mon_buf_l[3] = (tempt->month & 0x0f) | 0x80;
	mon_buf_h[3] = (tempt->month & 0x10) >> 4;
	year_buf_2[3] = (tempt->year) / 10;
	year_buf_3[3]= (tempt->year) % 10 | 0x80;
}





/*
 * send command to zlg7290
 */
static int led_send_cmd(signed char *data)
{
	//*************** hr add ***************
//	char  a[]={'f','f','f','f'};
//	signed char *temp = a;
	//*************************************
	signed char *temp = data;

	if (*temp == -1)	//nothing to send!
		return 0;

	IICDS = *temp;	//store the slave device address!
	IICSTAT = 0xF0;	//START SEND
	temp++;


	while (*temp != -1) 
	{
		while (!(IICCON & IICCON_PEND_INT_STATU));	//WAIT UNTIL TRANSFERS ENDED!
		IICDS = *temp;		//put new command into it to be sended!
		IICCON &= ~IICCON_PEND_INT_CLEAR; //A NEW TRANSFERS
		temp++;		
	}		

	while (!(IICCON & IICCON_PEND_INT_STATU));
        IICSTAT = 0xD0;//STOP IIC
        IICCON &= ~IICCON_PEND_INT_CLEAR;//CLEAR PENDING INT
	mdelay(5);	//delay needed before another transfers
	return 1;
}




/*
 * init IIC registers and zlg7290 and led:led like this:88_88_88
 */
static void led_reg_init(void)
{
	int i = 0;

	printk("led_reg_init!\n");

	IICCON = IICCON_DISABLE_ACKNOWLEDGE | IICCON_FPCLK_512 | IICCON_ENABLE_INT | IICCON_TXCLK_24K;
	//enable int,I2CCLK = 24k
	IICSTAT = 0xD0; //MASTER TX MODE AND ENABLE OUTPUT

	for (i=0 ;i <8 ;i++)	//all the lights display 8.
	{
		
		IICDS = ZLG7290_SLA_ADD;	//ZLG7920 SLAVE ADDRESS
		IICSTAT = 0xF0;	//START TRANSLATE	
	

	        while (!(IICCON & IICCON_PEND_INT_STATU));//WAIT UNTIL TRANSFERS ENDED!
	        IICDS = 0x10+i; //led i  buffer
	        IICCON &= ~IICCON_PEND_INT_CLEAR; //A NEW TRANLATE


		while (!(IICCON & IICCON_PEND_INT_STATU));//WAIT UNTIL TRANSFERS ENDED!
		if(i == 1 || i == 6)
			IICDS = 0x00;	//this two led turn off.
		else
			IICDS = 0xfc; //operate led 8.
		IICCON &= ~IICCON_PEND_INT_CLEAR; //A NEW TRANLATE


		while (!(IICCON & IICCON_PEND_INT_STATU));
		IICSTAT = 0xD0;//STOP IIC
		IICCON &= ~IICCON_PEND_INT_CLEAR;//CLEAR PENDING INT

		mdelay(5);
//		printk("Stop IIC!\n");
	}	
}





/*
 * interface of open operation
 */
static int led_open(void)
{
	led_reg_init();		//init IIC AND LED
//	led_set_time();		//set a new time when necessary!
	return 0;	
}





/*
 * interface of write operation
 */
static ssize_t led_write(struct file *file, signed char *buf, size_t count, loff_t *ppos)
{
	led_get_time(&led_time);
	printk("led_get_time!\n");

	led_time_format(&led_time);
	printk("led_time_format!\n");

	//turn to display time
	if (date_time_turn == TIME_TURN)
	{
	led_send_cmd(sec_buf_l);
	led_send_cmd(sec_buf_h);
	led_send_cmd(blank_buf_l);
	led_send_cmd(min_buf_l);
	led_send_cmd(min_buf_h);
	led_send_cmd(blank_buf_h);
	led_send_cmd(hour_buf_l);
	led_send_cmd(hour_buf_h);
	time_s++;
	if(time_s >= 10)
	  {
		time_s = 0;
		date_time_turn = DATE_TURN;
	  }
	}
	
	
	//turn to display date
	else if (date_time_turn == DATE_TURN)
	{
	        led_send_cmd(date_buf_l);
	        led_send_cmd(date_buf_h);
	        led_send_cmd(mon_buf_l);
	        led_send_cmd(mon_buf_h);
	        led_send_cmd(year_buf_0);
	        led_send_cmd(year_buf_1);
	        led_send_cmd(year_buf_2);
	        led_send_cmd(year_buf_3);
	        date_s++;
	        if(date_s >= 3)
        	  {
                	date_s = 0;
	                date_time_turn = TIME_TURN;
        	  }
        }
	
	
	return count;
}




/*
 * device file
 */
static struct file_operations led_fops = {
          owner:          THIS_MODULE,
//        read:           led_read,
	  write:	  led_write,
//        poll:           led_poll,
//        ioctl:          led_ioctl,
          open:           led_open,
//        release:        led_release,
//        fasync:         led_fasync,
};






/*
 * Initialization and exit routines
 */
int __init
led_init(void)
{
        int retval;

        retval = devfs_register_chrdev(LED_MAJOR,"led",&led_fops);
        //registered as a char device
	if (retval)
	{
		printk(KERN_ERR"led: can't register on minor=%d\n",LED_MAJOR);
		return retval;
	}

	dev_handle = devfs_register( NULL, "led", DEVFS_FL_DEFAULT, LED_MAJOR, 0, S_IFCHR, &led_fops, NULL);


        return retval;
}



void __exit
led_exit(void)
{
        unregister_chrdev(LED_MAJOR,"led");
}



module_init(led_init);
module_exit(led_exit);



















