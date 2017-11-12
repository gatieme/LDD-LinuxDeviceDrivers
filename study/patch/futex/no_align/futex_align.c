// futex_align.c
#include <stdio.h>
#include <linux/futex.h>
#include <syscall.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
	char *p = malloc(128);

	struct robust_list_head *ro1;
	struct robust_list *entry;
	struct robust_list *pending;

	int ret = 0;

	pid_t pid = getpid();

	printf("size = %d, p %p pid [%d] \n",
	sizeof(struct robust_list_head), p, pid);

	ro1 = p;
	entry = p + 20;
	pending = p + 40;

	ro1->list.next = entry;
	ro1->list_op_pending = pending;

	entry->next = &(ro1->list);

	ro1->futex_offset = 41;

	*((int *)((char *)entry + 41)) = pid;

	printf(" entry + offert [%p] [%d] \n",
		(int *)((char *)entry + 41),
		*((int *)((char *)entry + 41)));

	ret = syscall(SYS_set_robust_list, ro1,
		sizeof(struct robust_list_head));

	printf("ret = [%d]\n", ret);

	return 0;
}
