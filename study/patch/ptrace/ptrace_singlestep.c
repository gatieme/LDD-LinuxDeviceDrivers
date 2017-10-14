#include <stdio.h>
#include <stdlib.h>

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <unistd.h>

#ifdef DEBUG
#define dprintf printf
#else
#define dprintf 0 && printf
#endif

int main(void)
{
	long long counter = 0;
	int wait_val;
	int pid;

	puts("Please wait");

	switch (pid = fork()) {
	case -1:
		perror("fork");
		break;

	case 0:
		ptrace(PTRACE_TRACEME, 0, 0, 0);
		//execl("/bin/ls", "ls", NULL);
		execl("./a.out", "a.out", NULL);
		break;

	default:
		wait(&wait_val);
 		while (wait_val == 1407 ) {
			counter++;
			if (ptrace(PTRACE_SINGLESTEP, pid, 0, 0) != 0)
				perror("ptrace");
			else
				dprintf("counter = %lld\n", counter);
			wait(&wait_val);
		}
	}

	printf("Number of machine instructions : %lld\n", counter);
	return 0;
}
