#include <linux/filter.h>
#include <linux/ptrace.h>
#include <linux/version.h>
#include <uapi/linux/bpf.h>
#include "bpf_helpers.h"

SEC("kprobe/memcpy")

int bpf_prog1(struct pt_regs *ctx)
{
	unsigned long long size;
	char fmt[] = "memcpy size %d\n";

	bpf_probe_read(&size, sizeof(size), (void *)&PT_REGS_PARM3(ctx));

	bpf_trace_printk(fmt, sizeof(fmt), size);

	return 0;
}

char _license[] SEC("license") = "GPL";
u32 _version SEC("version") = LINUX_VERSION_CODE;
