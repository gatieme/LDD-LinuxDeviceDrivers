#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>

#define MAP_SIZE 1024*1024

int main(int argc, char **argv)
{
	unsigned char *m;
	int i;
	pid_t pid;

	m = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

	for (i = 0; i < MAP_SIZE; i++)
		m[i] = 10;

	pid = fork();
	if (pid == -1) {
		exit(1);
	} else if (pid == 0) {
		sleep(10);
	} else {
		sleep (5);

		/* Cow */
		for (i = 0; i < MAP_SIZE; i++)
			m[i] = 20;
		printf("cow is done in parent pscess\n");
		sleep(10);
	}

	return EXIT_SUCCESS;
}
