/*
*  Author: HIT CS HDMC team.
*  Create: 2010-3-12 8:20:01
*  Last modified: 2010-6-13 14:13:47
*/

#ifndef __MAIN_H_INCLUDE__
#define __MAIN_H_INCLUDE__

/*
*  common include header files
*/

#include "common.h"


#define MAX_LINE		        256						/// max length of a line
#define DELIMITER		        '\n'					/// the char is used to split the taskinfo
#define PERMISSION	            0644					/// proc node permission
/*
*	return values
*/
#define OK					     0
#define FAIL					-1

/*
 * proc files
 */
#define     PROC_ROOT               "proc"
#define     PROC_DIR                "proc_test"
#define     PROC_READ_ONLY_ENTRY    "read_only"
#define     PROC_WRITE_ONLY_ENTRY   "write_only"
#define     PROC_READ_WRITE_ENTRY   "read_write"

#define     PROC_READ_ONLY_FILE      PROC_ROOT "/" PROC_DIR "/" PROC_READ_ONLY_ENTRY
#define     PROC_WRITE_ONLY_FILE     PROC_ROOT "/" PROC_DIR "/" PROC_WRITE_ONLY_ENTRY
#define     PROC_READ_WRITE_FILE     PROC_ROOT "/" PROC_DIR "/" PROC_READ_WRITE_ENTRY



#endif                      //  #define __MAIN_H_INCLUDE__
