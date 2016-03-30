
#include <linux/config.h>
#include <linux/utsname.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/string.h>
#include <linux/fcntl.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/errno.h>

#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/mm.h>

#include <asm/uaccess.h>
#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/proc/pgtable.h>
#include <asm/io.h>
#include <linux/poll.h>
#include <asm/arch/memory.h>
#include <asm/arch/io.h>
#include <asm/arch/smdk.h>
#include "def.h"

#define        MVAL	         (13)
#define        MVAL_USED 	 (0)
#define        M5D(n)            ((n) & 0x1fffff)	// To get lower 21bits

#define        LCD_XSIZE_CSTN 	 (320)
#define        LCD_YSIZE_CSTN 	 (240)


#define        SCR_XSIZE_CSTN 	 (LCD_XSIZE_CSTN)          //*2for virtual screen  
#define        SCR_YSIZE_CSTN 	 (LCD_YSIZE_CSTN)          //*2

#define        HOZVAL_CSTN	 (LCD_XSIZE_CSTN*3/8-1)	   // Valid VD data line number is 8.
#define        LINEVAL_CSTN	 (LCD_YSIZE_CSTN-1)

#define        WLH_CSTN	         (0)
#define        WDLY_CSTN	 (0)
#define        LINEBLANK_CSTN	 (16 &0xff)

#define        CLKVAL_CSTN	 (6) 	
#define        LCDFRAMEBUFFER    LCD_BASE                 //_NONCACHE_STARTADDRESS 
 set in the smdk.c

#define        BYTESPERLINE 	 (LCD_XSIZE_CSTN)
typedef unsigned long 	tOff;
#define        XY2OFF(x,y) 	 (tOff)((tOff)y*(tOff)BYTESPERLINE + (x))

#define        READ_MEM(Off, Data)    Data = LCD_READ_MEM(Off)
#define        WRITE_MEM(Off, Data)   LCD_WRITE_MEM(Off, Data)

#define        GETPIXEL(x, y)         GetPixelIndex(x, y)
#define        SETPIXEL(x, y, c)      SetPixel(x, y, c)

unsigned lcd_count;

void Lcd_CstnOnOff(int onoff);

U8 LCD_COLOR = 0x00;
U16 LCD_BKCOLOR;

#define LCD_MAJOR 	60 

/*
*************************************************************************************************************
- 函数名称 : void CloseLCD(struct inode * inode, struct file * file)
- 函数说明 : LCD关闭
- 输入参数 : struct inode * inode, struct file * file
- 输出参数 : 0
*************************************************************************************************************
*/
static void CloseLCD(struct inode * inode, struct file * file)
{
	printk("LCD is closed\n");
	return ;
}

/*
*************************************************************************************************************
- 函数名称 : static int OpenLCD(struct inode * inode, struct file * file)
- 函数说明 : LCD打开
- 输入参数 : struct inode * inode, struct file * file
- 输出参数 : 0
*************************************************************************************************************
*/
static int OpenLCD(struct inode * inode, struct file * file)
{
	printk("LCD is open\n");
	return 0;
}

/*
*************************************************************************************************************
- 函数名称 : static int LCDIoctl(struct inode *inode,struct file * file,unsigned int cmd,unsigned long arg)
- 函数说明 : LCD控制输出
- 输入参数 : 
- 输出参数 : 0
*************************************************************************************************************
*/
static int LCDIoctl(struct inode *inode,struct file * file,unsigned long cmd,unsigned long arg)
{
//char color;
	struct para 
	{
	    unsigned long a;
	    unsigned long b;
	    unsigned long c;
	    unsigned long d;
	}*p_arg;


	
         switch(cmd) 
         {
		case 0:
                       	printk("set color\n");
	
			Set_Color(arg); 
	                printk("LCD_COLOR =%x\n",LCD_COLOR);
			return 1;

		case 1:
			printk("draw h_line\n");
			p_arg =(struct para *)arg;
			LCD_DrawHLine(p_arg->a,p_arg->b,p_arg->c);		// draw h_line
 	                LCD_DrawHLine(p_arg->a,p_arg->b+15,p_arg->c);		// draw h_line
	                LCD_DrawHLine(p_arg->a,p_arg->b+30,p_arg->c);		// draw h_line
			return 1;
			
	       case 2:
		        printk("draw v_line\n");	
			p_arg =(struct para *)arg;
    		        LCD_DrawVLine(p_arg->a,p_arg->b,p_arg->c); 		// draw v_line
		        LCD_DrawVLine(p_arg->a+15,p_arg->b,p_arg->c); 		// draw v_line
		        LCD_DrawVLine(p_arg->a+30,p_arg->b,p_arg->c); 		// draw v_line
			return 1;
			
	 	case 3 :
	        	printk("drwa circle\n");
			p_arg =(struct para *)arg;
	                LCD_DrawCircle(p_arg->a,p_arg->b,p_arg->c);		// draw circle
	                return 1;
	       
		case 4:
			printk("draw rect\n");
			p_arg =(struct para *)arg;
			LCD_FillRect(p_arg->a,p_arg->b,p_arg->c,p_arg->d);      // draw rect
			return 1;
			
	         case 5:
     		        printk("draw fillcircle\n");
			p_arg =(struct para *)arg;
			LCD_FillCircle(p_arg->a, p_arg->b, p_arg->c);// draw fillcircle
			return 1;
			
	 	case 6 :
	      	        printk("LCD is clear\n");
 			LCD_Clear(0,0,319,239);     // clear screen
	                return 1;
	
 	        case 7:
			printk("draw rect\n");
			p_arg =(struct para *)arg;
			LCD_FillRect(p_arg->a,p_arg->b,p_arg->c,p_arg->d);      // draw rect
			return 1;
                default:
         	        return -EINVAL;
             }
             return 1;
}

/*
*************************************************************************************************************
- 函数名称 : struct file_operations LCD_fops
- 函数说明 : 文件结构
- 输入参数 : 无
- 输出参数 : 无
*************************************************************************************************************
*/
static struct file_operations LCD_fops = 
{
	ioctl:	   LCDIoctl,           	/* ioctl */
	open:	   OpenLCD,    		/* just a selector for the real open */
	release:   CloseLCD,            /* release */
};

/*
*************************************************************************************************************
- 函数名称 : U16  LCD_Init(U8 Lcd_Bpp)  
- 函数说明 : LCD硬件初始化函数
- 输入参数 : 
- 输出参数 : 无
*************************************************************************************************************
*/
U16  Setup_LCDInit(void) 
{
     long i; unsigned char * base;
     GPCUP = 0xffffffff; // Disable Pull-up register
     GPCCON= 0xaaaaaaaa; //Initialize VD[7:0],LCDVF[2:0],VM,VFRAME,VLINE,VCLK,LEND 
     GPDUP = 0xffffffff; // Disable Pull-up register
     GPDCON= 0xaaaaaaaa; //Initialize VD[23:8]
          
     // Packed Type : The L.C.M of 12 and 32 is 96.
     LCDCON1 = (CLKVAL_CSTN<<8)|(MVAL_USED<<7)|(2<<5)|(3<<1)|0;
    // 8-bit single scan,8bpp CSTN,ENVID=off
     LCDCON2 = (0<<24)|(LINEVAL_CSTN<<14)|(0<<6)|0;
     LCDCON3 = (WDLY_CSTN<<19)|(HOZVAL_CSTN<<8)|(LINEBLANK_CSTN<<0);
     LCDCON4 = (MVAL<<8)|(WLH_CSTN<<0);
 
     LCDCON5 = 2;
  

     LCDADDR1 = ((U32)0x33800000>>22)<<21;
  //use th physical address
     LCDADDR2 = M5D((((SCR_XSIZE_CSTN )*LCD_YSIZE_CSTN))>>1);
 
     LCDADDR3 = (((SCR_XSIZE_CSTN - LCD_XSIZE_CSTN)/2)<<11)|(LCD_XSIZE_CSTN / 2);

     DITHMODE = 0;
     REDLUT   = 0xfdb96420;
     GREENLUT = 0xfdb96420;
     BLUELUT  = 0xfb40;

     base =(unsigned char*)LCD_BASE;
     Lcd_CstnOnOff(1);
	
    
     for(i=0;i<320*240;i++)
     {
          *base++ = 0xff;
     }
     return 0;    
}

/*
*************************************************************************************************************
- 函数名称 : U16 LCD_READ_MEM(U32 off)
- 函数说明 : 针对硬件的读点函数
- 输入参数 : x,y,c
- 输出参数 : 无
*************************************************************************************************************
*/

U16 LCD_READ_MEM(U32 off)
{
     return (*((U8*)LCDFRAMEBUFFER + (off)));
}

/*
*************************************************************************************************************
- 函数名称 : void LCD_WRITE_MEM( U32 off,U8 Data)
- 函数说明 : 针对硬件的画点函数
- 输入参数 : x,y,c
- 输出参数 : 无
*************************************************************************************************************
*/	
void LCD_WRITE_MEM( U32 off,U8 Data)
{
       (* ((U8*)LCDFRAMEBUFFER + (off)) ) = Data;
}
/*
*************************************************************************************************************
- 函数名称 : static void SetPixel(U16 x,U16 y,int c)
- 函数说明 : 针对硬件的画点函数
- 输入参数 : x,y,c
- 输出参数 : 无
*************************************************************************************************************
*/
static void SetPixel(U16 x, U16 y, U32 c) 
{
    tOff Off = XY2OFF(x,y);
    WRITE_MEM(Off, c);
}

/*
*************************************************************************************************************
- 函数名称 : void LCD_DrawPixel  (U16 x, U16 y) 
- 函数说明 : 画点函数
- 输入参数 : x,y
- 输出参数 : 无
*************************************************************************************************************
*/
void LCD_DrawPixel  (U16 x, U16 y) 
{
   	SETPIXEL(x, y, LCD_COLOR);
}



/*
*************************************************************************************************************
- 函数名称 : U32 GetPixelIndex(U16 x, U16 y)
- 函数说明 : 针对硬件的得到点的颜色的函数
- 输入参数 : x,y
- 输出参数 : color
*************************************************************************************************************
*/
U32 GetPixelIndex(U16 x, U16 y) 
{
  	U32 col;
  	U8 Data;
  	tOff Off = XY2OFF(x,y);
  	READ_MEM(Off,Data);
        col = Data;
  	return col;
}

	
/*
*************************************************************************************************************
- 函数名称 : U32 LCD_GetPixel(U16 x, U16 y) 
- 函数说明 : 得到点值的函数
- 输入参数 : x,y
- 输出参数 : colof
*************************************************************************************************************
*/	
U32 LCD_GetPixel(U16 x, U16 y) 
{
	return GETPIXEL(x,y);
}

/*
*************************************************************************************************************
- 函数名称 : void LCD_DrawHLine  (U16 x0, U16 y,  U16 x1) 
- 函数说明 : 画水平线函数
- 输入参数 : x,y,x1
- 输出参数 : 无
*************************************************************************************************************
*/
void LCD_DrawHLine  (U16 x0, U16 y0,  U16 x1) 
{
    while (x0 <= x1) 
    {
     	SETPIXEL(x0, y0, LCD_COLOR);
     	x0++;
    }
}

/*
*************************************************************************************************************
- 函数名称 : void LCD_DrawVLine  (U16 x, U16 y0,  U16 y1) 
- 函数说明 : 画竖直线函数
- 输入参数 : x,y,x1
- 输出参数 : 无
*************************************************************************************************************
*/
void LCD_DrawVLine(U16 x0, U16 y0,  U16 y1) 
{
	while (y0 <= y1) 
 	{
		SETPIXEL(x0, y0, LCD_COLOR);
 		y0++;
 	}
}

/*
*************************************************************************************************************
- 函数名称 : void LCD_FillRect(U16 x0, U16 y0, U16 x1, U16 y1)
- 函数说明 : 填充矩形函数
- 输入参数 : x0,y0,x1,y1
- 输出参数 : 无
*************************************************************************************************************
*/
void LCD_FillRect(U16 x0, U16 y0, U16 x1, U16 y1) 
{
	for (; y0 <= y1; y0++) 
	{
		 LCD_DrawHLine(x0,y0, x1);
	}
}

/*
*************************************************************************************************************
- 函数名称 : void LCD_Clear(U16 x0, U16 y0, U16 x1, U16 y1)
- 函数说明 :  清屏函数
- 输入参数 : x0,y0,x1,y1
- 输出参数 : 无
*************************************************************************************************************
*/
void LCD_Clear(U16 x0, U16 y0, U16 x1, U16 y1) 
{
	LCD_COLOR = 0x00;
	for (; y0 <= y1; y0++) 
	{
		 LCD_DrawHLine(x0,y0, x1);
	}
}
/*
*************************************************************************************************************
- 函数名称 : static  void DrawPoint(U16 x0,U16 y0, U16 xoff, U16 yoff)
- 函数说明 : 画辅助点函数
- 输入参数 : x,y
- 输出参数 : 无
*************************************************************************************************************
*/
static  void _DrawPoint(U32 x0,U32 y0, U32 xoff, U32 yoff) 
{
  	LCD_DrawPixel(x0+xoff,y0+yoff);
  	LCD_DrawPixel(x0-xoff,y0+yoff);
  	LCD_DrawPixel(x0+yoff,y0+xoff);
  	LCD_DrawPixel(x0+yoff,y0-xoff);
  	
  	if (yoff) 
  	{
    	LCD_DrawPixel(x0+xoff,y0-yoff);
    	LCD_DrawPixel(x0-xoff,y0-yoff);
    	LCD_DrawPixel(x0-yoff,y0+xoff);
    	LCD_DrawPixel(x0-yoff,y0-xoff);
  	}
}

/*
*************************************************************************************************************
- 函数名称 : void LCD_DrawCircle(U16 x0, U16 y0, U16 r)
- 函数说明 : 画圆函数
- 输入参数 : x,y
- 输出参数 : 无
*************************************************************************************************************
*/
void LCD_DrawCircle(U32 x0, U32 y0, U32 r) 
{
  	U32 i;
  	U32 imax = ((int)((int)r*707))/1000 + 1;
  	U32 sqmax = (int)r*(int)r + (int)r/2;
  	U16 y = r;
  	
  	_DrawPoint(x0,y0,r,0);
  	
  	for (i=1; i<= imax; i++) 
  	{
    	if ((i*i+y*y) > sqmax) 
    	{
      		_DrawPoint(x0,y0,i,y);
      		y--;
    	}
    	_DrawPoint(x0,y0,i,y);
  	}
}
/*
*************************************************************************************************************
- 函数名称 : void LCD_FillCircle(U16 x0, U16 y0, U16 r)
- 函数说明 : 填充圆函数
- 输入参数 : x,y
- 输出参数 : 无
*************************************************************************************************************
*/
void LCD_FillCircle       (U16 x0, U16 y0, U16 r) 
{
  	U32 i;
  	U32 imax = ((int)((int)r*707))/1000+1;
  	U32 sqmax = (int)r*(int)r+(int)r/2;
  	U16 x = r;
  
  	LCD_DrawHLine(x0-r,y0,x0+r);
  
  	for (i=1; i<= imax; i++) 
  	{
    	if ((i*i+x*x) >sqmax) 
    	{
      		if (x>imax) 
      		{
        		LCD_DrawHLine (x0-i+1,y0+x, x0+i-1);
        		LCD_DrawHLine (x0-i+1,y0-x, x0+i-1);
      		}
      		x--;
    	}
       	LCD_DrawHLine(x0-x,y0+i, x0+x);
    	LCD_DrawHLine(x0-x,y0-i, x0+x);
  	}
}
/*
*************************************************************************************************************
- 函数名称 : Log2Phy(int Color)
- 函数说明 : 逻辑颜色转实际颜色函数
- 输入参数 : color
- 输出参数 : 无
*************************************************************************************************************
*/

U16 Log2Phy(U32 Color) 
{
  	U32 r,g,b;
  	b = Color & 255;
  	g = (Color >> 8 ) & 255;
  	r = Color >> 16;
  	b = (b + 42) / 85;
  	g = (g * 7 + 127) / 255;
  	r = (r * 7 + 127) / 255;
  	return b + (g << 2) + (r << 5);
}
/*
*************************************************************************************************************
- 函数名称 : LCD_Log2Phy(int Color)
- 函数说明 : 逻辑颜色转实际颜色上层函数
- 输入参数 : color
- 输出参数 : 无
*************************************************************************************************************
*/

U16 LCD_Log2Phy(U32 Color) 
{
    U16 PhyColor;
  
  	PhyColor = Log2Phy(Color);
  
  	return PhyColor;
}
/*
*************************************************************************************************************
- 函数名称 : void Set_Color(int color)
- 函数说明 : 设定颜色的上层函数
- 输入参数 : color
- 输出参数 : 无
*************************************************************************************************************
*/

void Set_Color(U32 color) 
{

    LCD_SetColor(LCD_Log2Phy(color));

}
/*
*************************************************************************************************************
- 函数名称 : void Set_Color(int color)
- 函数说明 : 设定颜色函数
- 输入参数 : color
- 输出参数 : 无
*************************************************************************************************************
*/
void LCD_SetColor(U16 PhyColor)   
{ 
	LCD_COLOR = PhyColor; 
}
/*
*************************************************************************************************************
- 函数名称 : void Set_Color(int color)
- 函数说明 : 设定颜色的上层函数
- 输入参数 : color
- 输出参数 : 无
*************************************************************************************************************
*/

void Set_BkColor(U32 color) 
{

    LCD_SetBkColor(LCD_Log2Phy(color));

}
/*
*************************************************************************************************************
- 函数名称 : void Set_Color(int color)
- 函数说明 : 设定颜色函数
- 输入参数 : color
- 输出参数 : 无
*************************************************************************************************************
*/
void LCD_SetBkColor(U16 PhyColor)   
{ 
	LCD_BKCOLOR = PhyColor; 
}

/*
*************************************************************************************************************
- 函数名称 : int LCDInit(void)
- 函数说明 : 注册LCD设备
- 输入参数 : 无
- 输出参数 : 0,或-EBUSY
*************************************************************************************************************
*/
int __init  LCD_Init(void)
{
    int     result;

    Setup_LCDInit();
    printk("Registering S3C2410LCD Device\t--- >\t");
    result = register_chrdev(LCD_MAJOR, "S3C2410LCD", &LCD_fops);//注册设备

    if (result<0)
	{
		printk(KERN_INFO"[FALLED: Cannot register S3C2410LCD_driver!]\n");
		return -EBUSY;
	}
	else
		printk("[OK]\n");
		
    printk("Initializing S3C2410LCD Device\t--- >\t");
    printk("[OK]\n");

    printk("S3C2410LCD Driver Installed.\n");
    return 0;
}

/*
*************************************************************************************************************
- 函数名称 : LCD_CstnOnOff
- 函数说明 : 打开和关闭lcd设备
- 输入参数 : 无
- 输出参数 : 无
*************************************************************************************************************
*/
void Lcd_CstnOnOff(int onoff)
{
    // 1:CSTN Panel on  0:CSTN Panel off //	
    
    if(onoff==1)
    {	LCDCON1 |= 1; // ENVID=ON
	printk("\nLCDCON1 is already enable.\n");
    }
    else
	    LCDCON1 = LCDCON1 & 0x3fffe; // ENVID Off

    GPBUP  = GPBUP|(1<<5);                // Pull-up disable
    GPBDAT =( GPBDAT & (~(1<<5))) |(onoff<<5); // GPB5=On or Off
    GPBCON =( GPBCON & (~(3<<10)))|(1<<10);   //GPD9=output
}
/*
*************************************************************************************************************
- 函数名称 : LCD_Exit
- 函数说明 : 卸载lcd设备
- 输入参数 : 无
- 输出参数 : 无
*************************************************************************************************************
*/
void __exit LCDdriver_Exit(void)
{
        Lcd_CstnOnOff(0);
        unregister_chrdev(LCD_MAJOR, "S3C2410LCD");
        printk("You have uninstall The LCD Driver succesfully,\n if you want to install again,please use the insmod command \n");
}

module_init(LCD_Init);
module_exit(LCDdriver_Exit);
/*
*****************************************************************************************************************
**                                                  结束文件                                                   **
*****************************************************************************************************************
*/
