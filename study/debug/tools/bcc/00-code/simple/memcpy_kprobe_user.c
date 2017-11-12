#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/bpf.h>
#include "libbpf.h"
#include "bpf_load.h"

int main(int argc, char **argv)
{
	char filename[256];

	snprintf(filename, sizeof(filename), "%s_kern.o", argv[0]);

	if (load_bpf_file(filename)) {
		printf("%s", bpf_log_buf);
		return 1;
	}

	read_trace_pipe();

	return 0;
}
