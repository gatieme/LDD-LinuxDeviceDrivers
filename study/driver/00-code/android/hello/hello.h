#ifndef _HELLO_ANDROID_H_
#define _HELLO_ANDROID_H_

#include <linux/cdev.h>
#include <linux/semaphore.h>

/*  config  */
#define HELLO_DEVFS_BUILD


#define HELLO_PROC_BUILD
#define PROC_SEQ_FILE_OPERATIONS


#define HELLO_DEVICE_NODE_NAME  "hello"
#define HELLO_DEVICE_FILE_NAME  "hello"

#ifdef HELLO_PROC_BUILD
    #define HELLO_DEVICE_PROC_NAME  "hello"
#endif

#ifdef HELLO_DEVFS_BUILD
    #define HELLO_DEVICE_CLASS_NAME "hello"
#endif

struct hello_android_dev
{
    int val;
    struct semaphore sem;
    struct cdev dev;
};

#endif
