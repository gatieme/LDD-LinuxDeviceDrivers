/*
 * load50.c -- a simple busy-looping tool. 
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
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	int i, load = 50;

	if (argc == 2) {
		load = atoi(argv[1]);
	}
	printf("Increasing load by %i\n", load);
  
	for (i=0; i<load; i++)
		if (fork() == 0)
			break;

	while(1)
		;
	return 0;
}
