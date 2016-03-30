#ifndef _IIC_H_
#define _IIC_H_













                                                                                                
                                                                                                                                               
#define IICCON          __REG(0x54000000)
#define IICSTAT         __REG(0x54000004)
#define IICADD          __REG(0x54000008)
#define IICDS           __REG(0x5400000C)
                                                                                                                                               
#define IICCON_DISABLE_ACKNOWLEDGE      (0<<7)
#define IICCON_FPCLK_512        (1<<6)
                                                                                                                                               
#define IICCON_ENABLE_INT       (1<<5)
#define IICCON_PEND_INT_STATU           (1<<4)
#define IICCON_PEND_INT_CLEAR           (1<<4)
                                                                                                                                               
#define IICCON_TXCLK_24K                0x03
                                                                                                                                               
                                                                                                                                               

                                                                                                                                               



#endif






























