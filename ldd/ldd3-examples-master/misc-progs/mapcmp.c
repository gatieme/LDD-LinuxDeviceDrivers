/*
 * Simple program to compare two mmap'd areas.
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
 * $Id: mapcmp.c,v 1.2 2004/03/05 17:35:41 corbet Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/errno.h>
#include <fcntl.h>

static char *mapdev (const char *, unsigned long, unsigned long);
#define PAGE_SIZE 4096

/*
 * memcmp dev1 dev2 offset pages
 */
int main (int argc, char **argv)
{
	unsigned long offset, size, i;
	char *addr1, *addr2;
/*
 * Sanity check.
 */
	if (argc != 5)
	{
		fprintf (stderr, "Usage: mapcmp dev1 dev2 offset pages\n");
		exit (1);
	}
/*
 * Map the two devices.
 */
	offset = strtoul (argv[3], NULL, 16);
	size = atoi (argv[4])*PAGE_SIZE;
	printf ("Offset is 0x%lx\n", offset);
	addr1 = mapdev (argv[1], offset, size);
	addr2 = mapdev (argv[2], offset, size);
/*
 * Do the comparison.
 */
	printf ("Comparing...");
	fflush (stdout);
	for (i = 0; i < size; i++)
		if (*addr1++ != *addr2++)
		{
			printf ("areas differ at byte %ld\n", i);
			exit (0);
		}
	printf ("areas are identical.\n");
	exit (0);
}



static char *mapdev (const char *dev, unsigned long offset,
		unsigned long size)
{
	char *addr;
	int fd = open (dev, O_RDONLY);

	if (fd < 0)
	{
		perror (dev);
		exit (1);
	}
	addr = mmap (0, size, PROT_READ, MAP_PRIVATE, fd, offset);
	if (addr == MAP_FAILED)
	{
		perror (dev);
		exit (1);
	}
	printf ("Mapped %s (%lu @ %lx) at %p\n", dev, size, offset, addr);
	return (addr);
}
