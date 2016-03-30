

#ifndef __LCDLIB_H__
	#define __LCDLIB_H__

	#include <string.h>                                                                   
	#include <stddef.h>                                                           
    #include "GLib.h"
	                                                

#define MVAL		(13)
#define MVAL_USED 	(0)
#define M5D(n) ((n) & 0x1fffff)	// To get lower 21bits

//Color STN LCD Panel(320*240)
#define MODE_CSTN_8BIT   (0x2001)
#define MODE_CSTN_12BIT (0x2002)

//Color STN
#define LCD_XSIZE_CSTN 	(320)
#define LCD_YSIZE_CSTN 	(240)

//Color STN
#define SCR_XSIZE_CSTN 	(LCD_XSIZE_CSTN)   //*2for virtual screen  
#define SCR_YSIZE_CSTN 	(LCD_YSIZE_CSTN)//*2

//Color STN
#define HOZVAL_CSTN		(LCD_XSIZE_CSTN*3/8-1)	// Valid VD data line number is 8.
#define LINEVAL_CSTN		(LCD_YSIZE_CSTN-1)

//CSTN timing parameter for LCBHBT161M(NANYA)
#define WLH_CSTN	        (0)
#define WDLY_CSTN			(0)
#define LINEBLANK_CSTN		(16 &0xff)

//Timing parameter for LTS350Q1(SAMSUNG) 
#define VBPD_240320		((2-1)&0xff)
#define VFPD_240320		((3-1)&0xff)
#define VSPW_240320		((2-1) &0x3f)
#define HBPD_240320		((7-1)&0x7f)
#define HFPD_240320		((3-1)&0xff)
#define HSPW_240320		((4-1)&0xff)


#define CLKVAL_CSTN		(6) 	
    // 130hz @50Mhz,WLH=16hclk,WDLY=16hclk,LINEBLANK=16*8hclk,VD=8  

#define LCDFRAMEBUFFER 0x33800000 //_NONCACHE_STARTADDRESS 
    // 1. The LCD frame buffer should be write-through or non-cachable.
    // 2. The total frame memory should be inside 4MB.
    // 3. To meet above 2 conditions, the frame buffer should be 
    // inside the following regions.
    // 0x31000000~0x313ffffff,
    // 0x31400000~0x317ffffff,
    // 0x31800000~0x31bffffff,
    //        .....    
    // 0x33800000~0x33bffffff                

void Lcd_CstnOnOff(int onoff);

void LCD_Init(int type);
void Lcd_Port_Init(void);

#endif /*__LCDLIB_H__*/
