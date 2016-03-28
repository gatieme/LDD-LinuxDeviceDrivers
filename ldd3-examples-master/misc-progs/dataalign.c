/*
 * dataalign.c -- show alignment needs
 *
 * Copyright (C) 2001 Alessandro Rubini and Jonathan Corbet
 * Copyright (C) 2001 O'Reilly & Associates
 *
 * The source code in this file can be freely used, adapted,
 * and redistributed in source or binary form, so long as an
 * acknowledgment appears in derived source files.  The citation
 * should list that the code comes from the book "Linux Device
 * Drivers" by Alessandro Rubini and Jonathan Corbet, published
 * by O'Reilly & Associates.   No warranty is attached;
 * we cannot take responsibility for errors or fitness for use.
 *
 * This runs with any Linux kernel (not any Unix, because of <linux/types.h>)
 */
#include <stdio.h>
#include <sys/utsname.h>
#include <linux/types.h>

/*
 * Define several data structures, all of them start with a lone char
 * in order to present an unaligned offset for the next field
 */
struct c   {char c;  char      t;} c;
struct s   {char c;  short     t;} s;
struct i   {char c;  int       t;} i;
struct l   {char c;  long      t;} l;
struct ll  {char c;  long long t;} ll;
struct p   {char c;  void *    t;} p;
struct u1b {char c;  __u8      t;} u1b;
struct u2b {char c;  __u16     t;} u2b;
struct u4b {char c;  __u32     t;} u4b;
struct u8b {char c;  __u64     t;} u8b;

int main(int argc, char **argv)
{
    struct utsname name;

    uname(&name); /* never fails :) */
    printf("arch  Align:  char  short  int  long   ptr long-long "
	   " u8 u16 u32 u64\n");
    printf(       "%-12s  %3i   %3i   %3i   %3i   %3i   %3i      "
	   "%3i %3i %3i %3i\n",
	   name.machine,
	   /* note that gcc can subtract void * values, but it's not ansi */
	   (int)((void *)(&c.t)   - (void *)&c),
	   (int)((void *)(&s.t)   - (void *)&s),
	   (int)((void *)(&i.t)   - (void *)&i),
	   (int)((void *)(&l.t)   - (void *)&l),
	   (int)((void *)(&p.t)   - (void *)&p),
	   (int)((void *)(&ll.t)  - (void *)&ll),
	   (int)((void *)(&u1b.t) - (void *)&u1b),
	   (int)((void *)(&u2b.t) - (void *)&u2b),
	   (int)((void *)(&u4b.t) - (void *)&u4b),
	   (int)((void *)(&u8b.t) - (void *)&u8b));
    return 0;
}
