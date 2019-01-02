#include <stdio.h>
#include <stdlib.h>

// gcc -g stack.c -o stack
//
unsigned long sp(void){ asm("mov %rsp, %rax");}
int main(int argc, char **argv)
{
	unsigned long esp = sp();
	printf("Stack pointer (ESP : 0x%lx)\n",esp);
	return 0;
}
