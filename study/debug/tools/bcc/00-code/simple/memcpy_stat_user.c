#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/bpf.h>
#include "libbpf.h"
#include "bpf_load.h"

int main(int argc, char **argv)
{
	char filename[256];
	size_t size;
	unsigned int size_cnt = 0;

	snprintf(filename, sizeof(filename), "%s_kern.o", argv[0]);

	if (load_bpf_file(filename)) {
		printf("%s", bpf_log_buf);
		return 1;
	}

	while (1) {
		printf("\tSize\t\tCount\n");
		for (size = 0; size <=1024; size = size + 64) {
			if(bpf_map_lookup_elem(map_fd[0], &size, &size_cnt))
				size_cnt = 0;
			if (size == 1024)
				printf("%4ld - %4ld*\t\t%d\n", size - 63, size,
						size_cnt);
			else if (size)
				printf("%4ld - %4ld\t\t%d\n", size - 63, size,
						size_cnt);
			else
				printf("   0\t\t\t%d\n", size_cnt);
		}
		printf ("* Size > 1024 have been counted in this interval\n");
		sleep(2);
	}

	return 0;
}
