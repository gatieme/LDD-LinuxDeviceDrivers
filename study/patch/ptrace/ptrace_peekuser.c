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

int main()
{
	pid_t child;
	long orig_eax;
	child = fork();
	if(child == 0) {
		ptrace(PTRACE_TRACEME, 0, NULL, NULL);
		execl("/bin/ls", "ls", NULL);
	}
	else {
		wait(NULL);
		orig_eax = ptrace(PTRACE_PEEKUSER, child, 8*15, NULL);
		printf("The child made a system call %ld\n", orig_eax);
		ptrace(PTRACE_CONT, child, NULL, NULL);
	}
	return EXIT_SUCCESS;
}