/* 
 * inp.c -- read all the ports specified in hex on the command line.
 *     The program acts as inb/inw/inl according to its own name
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
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/io.h>
#include <sys/perm.h>

char *prgname;

static int read_and_print_one(unsigned int port, int size)
{
    static int iopldone = 0;

    if (port > 1024) {
	if (!iopldone && iopl(3)) {
	    fprintf(stderr, "%s: iopl(): %s\n", prgname, strerror(errno));
	    return 1;
	}
	iopldone++;
    } else if (ioperm(port, size, 1)) {
	fprintf(stderr, "%s: ioperm(%x): %s\n", prgname,
		port, strerror(errno));
	return 1;
    }

    if (size == 4)
	printf("%04x: %08x\n", port, inl(port));
    else if (size == 2)
	printf("%04x: %04x\n", port, inw(port));
    else
	printf("%04x: %02x\n", port, inb(port));
    return 0;
}

int main(int argc, char **argv)
{
    unsigned int i, n, port, size, error = 0;
    
    prgname = argv[0];
    /* Find the data size based on the program name. */
    switch (prgname[strlen(prgname) - 1]) {
        case 'w': size = 2; break;
        case 'l': size = 4; break;
        case 'b': case 'p': default:
	    size = 1;
    }

    setuid(0); /* If we're setuid, force it on. */
    for (i = 1; i < argc; i++) {
        if (sscanf(argv[i], "%x%n", &port, &n) < 1
	      || n != strlen(argv[i]) ) {
	    fprintf(stderr, "%s: argument \"%s\" is not a hex number\n",
		    argv[0], argv[i]);
	    error++; continue;
	}
	if (port & (size - 1)) {
	    fprintf(stderr, "%s: argument \"%s\" is not properly aligned\n",
		    argv[0], argv[i]);
	    error++; continue;
	}
	error += read_and_print_one(port, size);
    }
    exit(error ? 1 : 0);
}

