
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>


#define LCDHEIGHT	240
#define LCDWIDTH	320
typedef unsigned int  U32;
typedef unsigned char U8;

U32 LCDBuffer[LCDHEIGHT][LCDWIDTH];
/*
*************************************************************************************************************
- 函数名称 : void LCD_Refresh(int *fbp)
- 函数说明 : 更新区域
- 输入参数 : int *fbp
- 输出参数 : 无
*************************************************************************************************************
*/
void LCD_Refresh(int *fbp)
{
    int i,j;
    U32 lcddata;
    U32 pixcolor;
    U8* pbuf = (U8*)LCDBuffer[0]; 

    for(i=0;i<LCDWIDTH*LCDHEIGHT/4;i++)
    {
	lcddata = 0;

	for(j=24;j>=0;j-=8)
        {
	    pixcolor = (pbuf[0]&0xe0)|((pbuf[1]>>3)&0x1c)|(pbuf[2]>>6);
	    lcddata |= pixcolor<<j;
	    pbuf    += 4;
	}
	*(fbp+i) = lcddata;
    }
}

/*
*************************************************************************************************************
- 函数名称 : Exep_S3cint_Init(void)
- 函数说明 : 异常及中断控制器的初始化
- 输入参数 : 无
- 输出参数 : 无
*************************************************************************************************************
*/
void Test_Cstn256(U32 *fbp)
{
    int i,j,k,jcolor=0x00;

    for(i=0;i<9;i++) 
    {
	switch(i) 
        {
	    case 0: 
	        jcolor = 0x00000000;
	        break;
	    case 1: 
	        jcolor = 0x000000e0;
	        break;
	    case 2: 
	      jcolor = 0x000070e0;
		break;
	    case 3: 
	      jcolor = 0x0000e0e0;
		break;
	    case 4: 
	      jcolor = 0x0000e000;
		break;
	    case 5: 
	      jcolor = 0x00e0e000;
		break;
	    case 6:
	      jcolor = 0x00e00000;
		break;
	    case 7: 
	      jcolor = 0x00e000e0;
		break;
	    case 8: 
	      jcolor = 0x00e0e0e0;
		break;
	}

	for(k=0;k<240;k++)
		for(j=i*32; j<i*32+32; j++)
		   LCDBuffer[k][j]=jcolor;
                 
     }
	
    	jcolor=0x000000ff;

    	for(i=0;i<240;i++)
        {
    	    if(i==80||i==160)
    		jcolor<<=8;
    	    for(j=288;j<320;j++)
    		LCDBuffer[i][j]=jcolor;
    	}
    	for(i=0;i<240;i++)
    	    for(j=0;j<320;j++)
	
    	LCD_Refresh(fbp);
}

/*
*************************************************************************************************************
- 函数名称 : Exep_S3cint_Init(void)
- 函数说明 : 异常及中断控制器的初始化
- 输入参数 : 无
- 输出参数 : 无
*************************************************************************************************************
*/
int main(void)
{
       int fb;
       int cmd;
       unsigned char*  fb_mem;
	
	if ((fb = open("/dev/fb0", O_RDWR)) < 0) 
	{
		printf("cannot open /dev/fb0\n");
		exit(0);
	};
	
	fb_mem =(unsigned char *) mmap(NULL,320*240,PROT_READ|PROT_WRITE,MAP_SHARED,fb,0);
        memset(fb_mem,0,320*240);

        Test_Cstn256(fb_mem);
	
       	cmd = getchar();
        munmap(fb, 320*240);
	close(fb);
        return 0;
}
