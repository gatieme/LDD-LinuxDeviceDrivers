/*
 * setlevel.c -- choose a console_loglevel for the kernel
 *
 * Copyright (C) 1998,2000,2001 Alessandro Rubini
 * 
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* A script to change the console log level. It takes as a command
   line argument an integer between 1 and 8 inclusive. Higher numbers
   mean more gets printed to the console. */

int main(int argc, char **argv)
{
    int level;

    if (argc == 2) {
	level = atoi(argv[1]);
    } else {
        fprintf(stderr, "%s: need a single arg\n", argv[0]);
	exit(1);
    }
    /* 8 says we are changing the log level. See the syslog(2) man page for the
       full list of actions. */
    if (klogctl(8, NULL, level) < 0) {  
        fprintf(stderr,"%s: syslog(setlevel): %s\n", argv[0], strerror(errno));
        exit(1);
    }
    exit(0);
}
