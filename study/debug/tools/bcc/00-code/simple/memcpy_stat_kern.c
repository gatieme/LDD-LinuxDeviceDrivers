#include <linux/filter.h>
#include <linux/ptrace.h>
#include <linux/version.h>
#include <uapi/linux/bpf.h>
#include "bpf_helpers.h"

struct bpf_map_def SEC("maps") my_map = {
	.type = BPF_MAP_TYPE_HASH,
	.key_size = sizeof(size_t),
	.value_size = sizeof(u32),
	.max_entries = 17,
};

SEC("kprobe/memcpy")

int bpf_prog1(struct pt_regs *ctx)
{
	size_t size;
	u32 *val, count_start = 0;

	bpf_probe_read(&size, sizeof(size), (void *)&PT_REGS_PARM3(ctx));

	if (size % 64)
		size += (64 - size % 64);

	if (size > 1024)
		size = 1024;

	val = bpf_map_lookup_elem(&my_map, &size);
	if (val && *val < UINT_MAX)
		*val = *val + 1;
	else
		bpf_map_update_elem(&my_map, &size, &count_start, BPF_NOEXIST);

	return 0;
}

char _license[] SEC("license") = "GPL";
u32 _version SEC("version") = LINUX_VERSION_CODE;
