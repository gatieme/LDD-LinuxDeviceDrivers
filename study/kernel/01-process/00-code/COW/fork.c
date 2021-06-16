#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <unistd.h>



int data = 10;

int main(int argc, char* argv[])
{
	int pid;

	pid = fork();
	if (pid == 0) {
		printf("Child process %d, data %d\n", getpid(), data);
		data = 20;
		printf("Child process %d, data %d\n", getpid(), data);
	} else {
		sleep(1);
		printf("Parent process %d, data %d\n", getpid(), data);
	}

	return EXIT_SUCCESS;
}
