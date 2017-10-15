#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <unistd.h>

#ifdef DEBUG
#define dprintf printf
#else
#define dprintf 0 && printf
#endif

#define FILENAME()

void get_filename(char *filename, char*filepath)
{
	int i = strlen(filepath) - 1;
	for( ; filepath[i] != '\\' && filepath[i] != '/' && i >= 0; i--);
	//printf("i = %d, %s\n", i, filepath + i + 1);
	strcpy(filename, filepath + i + 1);
	filename[strlen(filepath) - i] = '\0';
	//printf("%s\n", filename);
}


int main(int argc, char *argv[])
{
	long long counter = 0;
	int wait_val;
	int pid;
	char execpath[81], execname[81];

	if (argc > 2) {
		printf("Usage :\n");
		printf("\t%s execname\n", argv[0]);
		exit(0);
	}
	else if (argc == 2) {
		strcpy(execpath, argv[1]);
		get_filename(execname, execpath);
	}
	else {
		strcpy(execpath, "./a.out");
		get_filename(execname, execpath);
	}
	printf("%s : %s\n", execpath, execname);
	//getchar( );
	puts("Please wait");

	switch (pid = fork()) {
	case -1:
		perror("fork");
		break;

	case 0:
		ptrace(PTRACE_TRACEME, 0, 0, 0);
		//execl("/bin/ls", "ls", NULL);
		//execl("./a.out", "a.out", NULL);
		execl(execpath, execname, NULL);
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
