/*
*  Author: HIT CS HDMC team.
*  Last modified by zhaozhilong: 2010-1-14
*/
#ifndef __COMMON_H_INCLUDE__
#define __COMMON_H_INCLUDE__

#ifdef __KERNEL__   /*  in kernel modules  */


#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/kprobes.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/mount.h>
#include <linux/dcache.h>
#include <linux/slab.h>
#include <linux/highmem.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/kallsyms.h>
#include <linux/kprobes.h>
#include <asm/pgtable.h>
#include <asm/page.h>

#else

#ifdef __cplusplus

    #include <cstdio>
    #include <cstdlib>
    #include <cerrno>

#else   #ifndef __cplusplus
    //C header files
    #include <stdio.h>
    #include <stdlib.h>
    #include <errno.h>

#endif  //  #endif ifdef __cplusplus

#endif


#include "config.h"

/// 当前字符是数字
#define IsDigit(c)         (c >= '0' && c <= '9')

/// 当前字符是8进制数据
#define IsOctDigit(c)      (c >= '0' && c <= '7')

/// 当前字符是16进制数据
#define IsHexDigit(c)      (IsDigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))

/// 当前字符是字母或者_
#define IsLetter(c)        ((c >= 'a' && c <= 'z') || (c == '_') || (c >= 'A' && c <= 'Z'))

/// 当前自负是否满足C的变量命名规则
#define IsLetterOrDigit(c) (IsLetter(c) || IsDigit(c))
#define IsIdentifier    IsLetterOrDigit

/// 当前字符是空白字符
#define IsSpace(c)  ((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\r')



/// 取出当前信息的高4位
#define HIGH_4BIT(v)       ((v) >> (8 * sizeof(int) - 4) & 0x0f)
/// 取出当前信息的高3位
#define HIGH_3BIT(v)       ((v) >> (8 * sizeof(int) - 3) & 0x07)
/// 取出当前信息的高1位
#define HIGH_1BIT(v)       ((v) >> (8 * sizeof(int) - 1) & 0x01)

#ifndef ALIGN
#define ALIGN(size, align) ((size + align - 1) & (~(align - 1)))
#endif

#define BitGet(number, pos) ((number) >> (pos) & 1)     /// 用宏得到某数的某位
#define BitSet(number, pos) ((number) |= 1 << (pos))    /// 把某位置1
#define BitClr(number, pos) ((number) &= ~(1 << (pos))) /// 把某位清0
#define BitCpl(number, pos) ((number) ^= 1 << (pos))    /// 把number的POS位取反


///03: 得到指定地址上的一个字节或字
#define  MEM_B(x) (*((byte *)(x)))
#define  MEM_W(x) (*((word *)(x)))

///04: 求最大值和最小值
#define  MAX(x,y) (((x)>(y)) ? (x) : (y))
#define  MIN(x,y) (((x) < (y)) ? (x) : (y))

///05: 得到一个field在结构体(struct)中的偏移量
#define FPOS(type,field) ((dword)&((type *)0)->field)

///06: 得到一个结构体中field所占用的字节数
#define FSIZ(type,field) sizeof(((type *)0)->field)

///07: 按照LSB格式把两个字节转化为一个Word
#define FLIPW(ray) ((((word)(ray)[0]) * 256) + (ray)[1])

///08: 按照LSB格式把一个Word转化为两个字节
#define FLOPW(ray,val) (ray)[0] = ((val)/256); (ray)[1] = ((val) & 0xFF)

///09: 得到一个变量的地址（word宽度）
#define B_PTR(var)  ((byte *) (void *) &(var))
#define W_PTR(var)  ((word *) (void *) &(var))

///10: 得到一个字的高位和低位字节
#define WORD_LO(xxx)  ((byte) ((word)(xxx) & 255))
#define WORD_HI(xxx)  ((byte) ((word)(xxx) >> 8))

///11: 返回一个比X大的最接近的8的倍数
#define RND8(x) ((((x) + 7)/8) * 8)

///12: 将一个字母转换为大写
#define UPCASE(c) (((c)>='a' && (c) <= 'z') ? ((c) - 0x20) : (c))
#define ToUpper(c)		   (c & ~0x20)

///13: 判断字符是不是10进值的数字
#define  DECCHK(c) ((c)>='0' && (c)<='9')

///14: 判断字符是不是16进值的数字
#define HEXCHK(c) (((c) >= '0' && (c)<='9') ((c)>='A' && (c)<= 'F') \
((c)>='a' && (c)<='f'))

///15: 防止溢出的一个方法
#define INC_SAT(val) (val=((val)+1>(val)) ? (val)+1 : (val))

///16: 返回数组元素的个数
#define ARR_SIZE(a)  (sizeof((a))/sizeof((a[0])))

///17: 返回一个无符号数n尾的值MOD_BY_POWER_OF_TWO(X,n)=X%(2^n)
#define MOD_BY_POWER_OF_TWO( val, mod_by ) ((dword)(val) & (dword)((mod_by)-1))

///18: 对于IO空间映射在存储空间的结构,输入输出处理
#define inp(port) (*((volatile byte *)(port)))
#define inpw(port) (*((volatile word *)(port)))
#define inpdw(port) (*((volatile dword *)(port)))
#define outp(port,val) (*((volatile byte *)(port))=((byte)(val)))
#define outpw(port, val) (*((volatile word *)(port))=((word)(val)))
#define outpdw(port, val) (*((volatile dword *)(port))=((dword)(val)))






//#define DEBUG
//#define MMAP_INVALID_ARGUMENT /*    invalid argument    when mmap   */


/**
 * Macros to help with debugging. Set SCULL_DEBUG to 1 enable
 * debugging (which you can do from the Makefile); these macros work
 * in both kernelspace and userspace.
 */

/* undef it, just in case someone else defined it. */

#ifdef dbgprint
#undef dbgprint
#endif // dbgprint

#ifdef dprint
#undef dprint
#endif // dprint

#ifdef dout
#undef dount
#endif // dout


/**when you define DEBUG macro
    dbgprint to use printk with line and funcitonname in kernel
    dprint to use print without line and funcitonname in kernel


    dbgprint to use printf with line and funcitonname in userspace
    dprint to use printf without line and funcitonname in userspace
    it's the same to dbgcout and dcout in userspace

*/
#ifdef DEBUG
    #if defined(dbgprint) || defined(dprint) || defined(dprintk)
        #error "dbgprint, dprint or dprintk has been defined already..."
    #endif


    #ifdef __KERNEL__   /*  in kernel modules  */

        #if  defined(__FILE__) && defined(__LINE__)
            //printk with line and function name
            #define dbgprint(format, args...) \
            printk(KERN_INFO "[%20s, %4d] : "format, __FILE__, __LINE__, ##args)
        #endif  // ifdef defined(__FILE__) && defined(__LINE__)

        //printk without line and function name
        #define dprint(format,args...) printk(KERN_INFO format, ##args)
        #define dprintk(format,args...) printk(KERN_INFO format, ##args)


    #else   //  userspace

        #if defined(dbgprint) || defined(dprint) || defined(dprintf)
            #error "dbgprint, dprint or dprintk has been defined already..."
        #endif
        //////////
        /// printf for C language
        ///
        /// dbgprint    PRINT "[file, line], information
        ///
        /// dprint      PRINT "information"
        /// dprintf     PRINT "information"
        //////////

        /* Debugging is on and we are in userspace. */
        #define dprint(format, args...) printf(format, ## args)
        #define dprintf(format, args...) printf(format, ## args)

        #if defined(__FILE__) && defined(__LINE__)
            //printf with line and function name
            #define dbgprint(format, args...) \
            printf("[%20s,%4d] : "format, __FILE__, __LINE__, ##args)
        #endif  // ifdef defined(__FILE__) && defined(__LINE__)


        //////////
        /// cout for C++ language
        //
        //  dbgout      PRINT "[file, line], information
        //
        //  dcout       PRINT "information"
        //  debug       PRINT "information"
        //////////
        #ifdef __cplusplus

            #if defined(dbgout) || defined(cout) || defined(debug)
                #error "dbgprint, dprint or dprintk has been defined already..."
            #endif

            #if  defined(__FILE__) && defined(__LINE__)
            #define dbgcout std::cout <<"[" <<std::setw(20)<<__FILE__ <<"," <<std::setw(4)<<__LINE__ <<"] : "
            #endif

            #define dcout cout
            #define debug cout

        #endif  //  __cplusplus

        #ifdef __tmain
        #error "__tmain has been defined already"
        #endif
        #define __tmain main
    #endif // __KERNEL__ kernel and userspace

#else           /* Not debugging: do nothing. */

    #define dbgprint(format,args...)

    #define dprint(format, args...)

    #define dprintk(format,args...)
    #define dprintf(format, args...)

    #ifdef  __cplusplus
        #define dcout   0 && cout
        #define dbgcout 0 && cout
        #define debug   0 && cout
    #endif

#endif

/* PDEBUGG is a placeholder that makes it easy to "comment out" the debugging
   statements without deleting them. */
#undef undprint
#define undprint(format, args...)

#undef undcout
#define undcout 0 && count

#undef undbgprint
#define undprint(format, args...)

#undef undbgcout
#define undbgcout 0 && cout



//////////////////////////////////////////////////////////////////////////////////////////////
#define SHOW_FILE_NAME(fileName)     do                                                     \
                                    {   int i = strlen(fileName) - 1;                       \
                                        for( ; fileName[i] != '\\'                          \
                                        && fileName[i] != '/' && i >= 0; i--);              \
                                        char *fname = malloc(strlen(fileName) - i);         \
                                        strcpy(fname, fileName + i + 1);                    \
                                        printf("File: %s  ", fname);                        \
                                        free(fname);                                        \
                                    }while( 0 );
#
#
#define __file__ __FILE__
#define FILENAME()                  do                                                      \
                                    {   int i = strlen(__FILE__) - 1;                       \
                                        for( ;                                              \
                                             __FILE__[i] != '\\' && __FILE__[i] != '/';     \
                                            i--);                                           \
                                        char *fname = malloc(strlen(__FILE__) - i);         \
                                        strcpy(fname, __FILE__ + i + 1);                    \
                                        printf("FileName: %s", fname);                      \
                                        free(fname);                                        \
                                    }while( 0 );
#
#
#define __line__ __LINE__           ///
#define LINE( )                     do                                                      \
                                    {                                                       \
                                        printf("Line: %d", __LINE__);                       \
                                    }while( 0 );
#
#
#define __FUNC__ __func__           ///
#define FUNC( )                     do                                                      \
                                    {                                                       \
                                        printf("Function: %s", __FUNC__);                   \
                                    }while( 0 );
#
#
#define FILE_FUNC_LINE( )           do                                                      \
                                    {                                                       \
                                        FILENAME( );                                        \
                                        putchar(' ');                                       \
                                        FUNC( );                                            \
                                        putchar(' ');                                       \
                                        LINE( );                                            \
                                        putchar('\n');                                      \
                                    }while(0);



#if !defined(offset) && !defined(container_of)

#define offsetof(type, member) ((size_t) &((type *)0)->member)

/*
 *  get the struct(type) from the pointer pstr(one of his member)
 *  ptr     : the pointer point to the member whose type is member you know
 *  type    : the type of the struct you want get
 *  member  : the member's name in type struct
 */
#define container_of(ptr, type, member)                             \
({                                                                  \
    const typeof( ((type *)0)->member) *m_ptr = (ptr);              \
    (type *)( (char *)m_ptr - offsetof(type, member) );             \
})

#endif

/*
*	safe sprintf
* It will not be out of range.
*/
/*
#define safe_sprintf(start, n, p, format, args...);                     \
    {                                                                   \
        if( (p - start) < n )                                           \
        {                                                               \
            snprintf( (char *)p, (n - (p - start)), format, ##args );   \
        }                                                               \
    }
*/
#define safe_sprintf(start, n, p, format, args...)                      \
	(((p - start) < n )  && snprintf( (char *)p, (n - (p - start)), format, ##args ))




#endif	/* _COMMON_H_ */
