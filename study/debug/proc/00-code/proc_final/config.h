/*
*  Author: HIT CS HDMC team.
*  Last modified by zhaozhilong: 2010-1-14
*/
#ifndef _CONFIG_H_
#define _CONFIG_H_

#define DEBUG


/*
 * 3.10以后内核的proc文件
 * 废弃了原来的create_proc_entry接口
 * 3.9      http://lxr.free-electrons.com/ident?v=3.9&i=create_proc_entry
 * 2.10     http://lxr.free-electrons.com/ident?v=3.10;i=create_proc_entry
 *
 * 而引入了
 * 1.   proc_create创建proc文件,
 * 2.   seq_file(Sequence file：序列文件)接口
 * 3.   signal_file接口
 *
 * **proc_create创建proc文件**
 * 使用proc_create需要关联file_operations
 * 首先要调用创建proc文件的函数，需要绑定flie_operations
 *
 * ***seq_file接口操作proc文件*
 * http://blog.chinaunix.net/uid-28253945-id-3382865.html
 * 由于procfs的默认操作函数只使用一页的缓存，
 * 在处理较大的proc文件时就有点麻烦，
 * 并且在输出一系列结构体中的数据时也比较不灵活，
 * 需要自己在read_proc函数中实现迭代，容易出现Bug。
 * 所以内核黑客们对一些/proc代码做了研究，抽象出共性，
 * 最终形成了seq_file（Sequence file：序列文件）接口。
 * 这个接口提供了一套简单的函数来解决以上proc接口编程时存在的问题，
 * 使得编程更加容易，降低了Bug出现的机会。
 * 在需要创建一个由一系列数据顺序组合而成的虚拟文件或一个较大的虚拟文件时，
 * 推荐使用seq_file接口。
 *
 * 一般地，内核通过在procfs文件系统下建立文件来向用户空间提供输出信息，
 * 用户空间可以通过任何文本阅读应用查看该文件信息，
 * 但是procfs 有一个缺陷，如果输出内容大于1个内存页，需要多次读，
 * 因此处理起来很难，另外，如果输出太大，速度比较慢，
 * 有时会出现一些意想不到的情况，
 * Alexander Viro实现了一套新的功能，使得内核输出大文件信息更容易，
 * 该功能出现在2.4.15（包括2.4.15）以后的所有2.4内核以及2.6内核中，
 * 尤其 是在2.6内核中，已经大量地使用了该功能。
 * 要想使用seq_file功能，开发者需要包含头文件linux/seq_file.h，
 * 并定义与设置一个seq_operations结构（类似于file_operations结构）:
 *
 * 但是我个人认为，并不是只有procfs才可以使用这个seq_file接口，
 * 因为其实seq_file是实现的是一个操作函数集，这个函数集并不是与proc绑定的，
 * 同样可以用在其他的地方。
 *
 **/
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)

    #define     CREATE_PROC_ENTRY
//    #warning    "use create_proc_entry in " #LINUX_VERSION_CODE

#else
    #define     PROC_CREATE
//    #warning    "use proc_create in " #LINUX_VERSION_CODE
    #define     PROC_SEQ_FILE_OPERATIONS
    #include <linux/proc_fs.h>
    #include <linux/seq_file.h>
#endif

#if defined(CREATE_PROC_ENTRY) || defined(PROC_CREATE)
    #warning "defined CREATE_PROC_ENTRY || PROC_CREATE"
#else
    #error "you must define one of CREATE_PROC_ENTRY and PROC_CREATE"
#endif



#endif	/* _CONFIG_H_ */
