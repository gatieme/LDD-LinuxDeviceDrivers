/*
 *  fork_test.c
 *  version 1
 *  Created on: 2010-5-29
 *      Author: wangth
 */
#include <unistd.h>
#include <stdio.h>
int main ()
{
	pid_t fpid; //fpid表示fork函数返回的值
	int count=0;

	while( 1 ) {
		fpid=fork();

		if (fpid < 0)
			printf("error in fork!");
		else if (fpid == 0) {
			printf("Child pid = %d\n", getpid( ));
			exit(0);
		}
		else {
			if (waitpid(fpid, NULL, 0) != fpid)		/* wait for first child */
				perror("waitpid error");
			printf("Parent PID = %d\n", getpid( ));
			sleep(20);
		}
	}

	return 0;
}
