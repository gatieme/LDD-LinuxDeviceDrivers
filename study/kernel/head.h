//  http://www.cnblogs.com/chen-farsight/p/6146168.html

Linux驱动开发常用头文件

头文件目录中总共有32个.h头文件。其中主目录下有13个，asm子目录中有4个，linux子目录中有10个，sys子目录中有5个。这些头文件各自的功能如下：

1、主目录

<a.out.h>：a.out头文件，定义了a.out执行文件格式和一些宏。
<const.h>：常数符号头文件，目前仅定义了i节点中i_mode字段的各标志位。
<ctype.h>：字符类型头文件，定义了一些有关字符类型判断和转换的宏。
<errno.h>：错误号头文件，包含系统中各种出错号。(Linus从minix中引进的)。
<fcntl.h>：文件控制头文件，用于文件及其描述符的操作控制常数符号的定义。
<signal.h>：信号头文件，定义信号符号常量，信号结构以及信号操作函数原型。
<stdarg.h>：标准参数头文件，以宏的形式定义变量参数列表。主要说明了一个类型（va_list）和3个宏（va_start, va_arg和va_end），用于vsprintf、vprintf、vfprintf函数。
<stddef.h>：标准定义头文件，定义了NULL, offsetof(TYPE, MEMBER)。
<string.h>：字符串头文件，主要定义了一些有关字符串操作的嵌入函数。
<termios.h>：终端输入输出函数头文件，主要定义控制异步通信口的终端接口。
<time.h>：时间类型头文件，主要定义了tm结构和一些有关时间的函数原形。
<unistd.h>：Linux标准头文件，定义了各种符号常数和类型，并声明了各种函数。如，定义了__LIBRARY__，则还包括系统调用号和内嵌汇编_syscall0()等。
<utime.h>：用户时间头文件，定义了访问和修改时间结构以及utime()原型。

2、体系结构相关头文件子目录include/asm

这些头文件主要定义了一些与CPU体系结构密切相关的数据结构、宏函数和变量。共4个文件。
<asm/io.h>：I/O头文件，以宏的嵌入汇编程序形式定义对I/O端口操作的函数。
<asm/memory.h>：内存拷贝头文件，含有memcpy()嵌入式汇编宏函数。
<asm/segment.h>：段操作头文件，定义了有关段寄存器操作的嵌入式汇编函数。
<asm/system.h>：系统头文件，定义了设置或修改描述符/中断门等的嵌入式汇编宏。

3、Linux内核专用头文件子目录include/linux

<linux/config.h>：内核配置头文件，定义键盘语言和硬盘类型（HD_TYPE）可选项。
<linux/fdreg.h>：软驱头文件，含有软盘控制器参数的一些定义。
<linux/fs.h>：文件系统头文件，定义文件表结构（file,buffer_head,m_inode等）。
<linux/hdreg.h>：硬盘参数头文件，定义访问硬盘寄存器端口、状态码和分区表等信息。
<linux/head.h>：head头文件，定义了段描述符的简单结构，和几个选择符常量。
<linux/kernel.h>：内核头文件，含有一些内核常用函数的原形定义。
<linux/mm.h>：内存管理头文件，含有页面大小定义和一些页面释放函数原型。
<linux/sched.h>： 调度程序头文件，定义了任务结构task_struct、初始任务0的数据，以及一些有关描述符参数设置和获取的嵌入式汇编函数宏语句。
<linux/sys.h>：系统调用头文件，含有72个系统调用C函数处理程序,以"sys_"开头。
<linux/tty.h>：tty头文件，定义了有关tty_io，串行通信方面的参数、常数。

4、系统专用数据结构子目录include/sys

<sys/stat.h>： 文件状态头文件，含有文件或文件系统状态结构stat{}和常量。
<sys/times.h>：定义了进程中运行时间结构tms以及times()函数原型。
<sys/types.h>：类型头文件，定义了基本的系统数据类型。
<sys/utsname.h>：系统名称结构头文件。
<sys/wait.h>：等待调用头文件，定义系统调用wait()和waitpid()及相关常数符号。



#include <Linux/***.h>                     //是在linux-2.6.29/include/linux下面寻找源文件。
#include <asm/***.h>                     //是在linux-2.6.29/arch/arm/include/asm下面寻找源文件。
#include <mach/***.h>                   //是在linux-2.6.29/arch/arm/mach-s3c2410/include/mach下面寻找源文件。
#include <plat/regs-adc.h>           //在linux-2.6.31_TX2440A20100510\linux-2.6.31_TX2440A\arch\arm\plat-s3c\include\plat
#include <linux/module.h>            //最基本的文件，支持动态添加和卸载模块。Hello World驱动要这一个文件就可以了
#include <linux/fs.h>                       //包含了文件操作相关struct的定义，例如大名鼎鼎的struct file_operations，包含了struct inode 的定义，MINOR、MAJOR的头文件。
#include <linux/errno.h>                 //包含了对返回值的宏定义，这样用户程序可以用perror输出错误信息。
#include <linux/types.h>                 //对一些特殊类型的定义，例如dev_t, off_t, pid_t.其实这些类型大部分都是unsigned int型通过一连串的typedef变过来的，只是为了方便阅读。
#include <linux/cdev.h>                  //对字符设备结构cdev以及一系列的操作函数的定义。//包含了cdev 结构及相关函数的定义。
#include <linux/wait.h>                   //等代队列相关头文件//内核等待队列，它包含了自旋锁的头文件
#include <linux/init.h>                     //初始化头文件
#include <linux/kernel.h>               //驱动要写入内核，与内核相关的头文件
#include <linux/slab.h>                   //包含了kcalloc、kzalloc内存分配函数的定义。
#include <linux/uaccess.h>           //包含了copy_to_user、copy_from_user等内核访问用户进程内存地址的函数定义。
#include <linux/device.h>               //包含了device、class 等结构的定义
#include <linux/io.h>                       //包含了ioremap、iowrite等内核访问IO内存等函数的定义。
#include <linux/miscdevice.h>      //包含了miscdevice结构的定义及相关的操作函数。
#include <linux/interrupt.h>            //使用中断必须的头文件
#include <mach/irqs.h>                  //使用中断必须的头文件
#include <asm/bitops.h>                //包含set_bit等位操作函数，实现Input子系统时可用。
#include <linux/semaphore.h>      //使用信号量必须的头文件
#include <linux/spinlock.h>            //自旋锁
#include <linux/sched.h>                //内核等待队列中要使用的TASK_NORMAL、TASK_INTERRUPTIBLE包含在这个头文件
#include <linux/kfifo.h>                    //fifo环形队列
#include <linux/timer.h>                  //内核定时器
#include<linux/input.h>                   //中断处理
#include <linux/delay.h>                  //延时头文件
#include <asm/irq.h>                       //与处理器相关的中断
#include<linux/interrupt.h>              //操作系统中断
#include <asm/uaccess.h>            //与处理器相关的入口
#include<asm/arch/regs-gpio.h>  //与处理器相关的IO口操作
#include<mach/regs-gpio.h>         //同上
#include<asm/hardware.h>           //与处理器相关的硬件
#include<mach/hardware.h>          //同上
#include <linux/poll.h>                      //轮询文件
#include <linux/gpio.h>                     //操作系统相关的IO口文件
#include <stdio.h>                             //标准输入输出
#include <stdlib.h>                            //标准库
#include <unistd.h>                           //Unix标准
#include <sys/ioctl.h>                       //IO控制

分类: Linux设备驱动开发
