# SPDX-License-Identifier: GPL-2.0

srctree := /home/gatieme/Work/Kernel/linux/linux
objtree := /home/gatieme/Work/Kernel/linux/build/x86_64

# kbuild trick to avoid linker error. Can be omitted if a module is built.
obj- := dummy.o

# List of programs to build
hostprogs-y += memcpy_kprobe
hostprogs-y += memcpy_stat

# Libbpf dependencies
# use the libbpf and libbpf_load in current direntory
#LIBBPF := ./bpf/bpf.o
#LIBBPF_LOAD := ./bpf_load.o
# use the kernel libbpf and libbpf_load libraty 
LIBBPF := ../../../../../../../../Kernel/linux/linux/tools/lib/bpf/bpf.o
LIBBPF_LOAD := ../../../../../../../../Kernel/linux/linux/samples/bpf/bpf_load.o


memcpy_kprobe-objs := $(LIBBPF) $(LIBBPF_LOAD) memcpy_kprobe_user.o
memcpy_stat-objs := $(LIBBPF) $(LIBBPF_LOAD) memcpy_stat_user.o


# Tell kbuild to always build the programs
always := $(hostprogs-y)
always += memcpy_stat_kern.o
always += memcpy_kprobe_kern.o

HOSTCFLAGS += -I$(objtree)/usr/include
#HOSTCFLAGS += -I$(srctree)/include
#HOSTCFLAGS += -I$(srctree)/include/uapi
HOSTCFLAGS += -I$(srctree)/tools/lib/
HOSTCFLAGS += -I$(srctree)/tools/testing/selftests/bpf/
HOSTCFLAGS += -I$(srctree)/tools/lib/
HOSTCFLAGS += -I$(srctree)/tools/include
HOSTCFLAGS += -I$(srctree)/tools/include/uapi
HOSTCFLAGS += -I$(srctree)/tools/perf

HOSTCFLAGS_bpf_load.o += -I$(objtree)/usr/include -Wno-unused-variable
HOSTCFLAGS_bpf.o += $(HOSTCFLAGS) -Wno-unused-variable

HOSTLOADLIBES_memcpy_kprobe += -lelf
HOSTLOADLIBES_memcpy_stat += -lelf
HOSTLOADLIBES_bpf_load += -lelf

# Allows pointing LLC/CLANG to a LLVM backend with bpf support, redefine on cmdline:
#  make samples/bpf/ LLC=~/git/llvm/build/bin/llc CLANG=~/git/llvm/build/bin/clang
LLC ?= llc
CLANG ?= clang

# Trick to allow make to be run from this directory
all:
	$(MAKE) -C $(objtree) M=$(CURDIR)

clean:
	$(MAKE) -C $(objtree) M=$(CURDIR) clean
	@rm -f *~

$(obj)/syscall_nrs.s:	$(src)/syscall_nrs.c
	$(call if_changed_dep,cc_s_c)

$(obj)/syscall_nrs.h:	$(obj)/syscall_nrs.s FORCE
	$(call filechk,offsets,__SYSCALL_NRS_H__)

clean-files += syscall_nrs.h

FORCE:


# Verify LLVM compiler tools are available and bpf target is supported by llc
.PHONY: verify_cmds verify_target_bpf $(CLANG) $(LLC)

verify_cmds: $(CLANG) $(LLC)
	@for TOOL in $^ ; do \
		if ! (which -- "$${TOOL}" > /dev/null 2>&1); then \
			echo "*** ERROR: Cannot find LLVM tool $${TOOL}" ;\
			exit 1; \
		else true; fi; \
	done

verify_target_bpf: verify_cmds
	@if ! (${LLC} -march=bpf -mattr=help > /dev/null 2>&1); then \
		echo "*** ERROR: LLVM (${LLC}) does not support 'bpf' target" ;\
		echo "   NOTICE: LLVM version >= 3.7.1 required" ;\
		exit 2; \
	else true; fi

$(src)/*.c: verify_target_bpf

$(obj)/tracex5_kern.o: $(obj)/syscall_nrs.h

# asm/sysreg.h - inline assembly used by it is incompatible with llvm.
# But, there is no easy way to fix it, so just exclude it since it is
# useless for BPF samples.
$(obj)/%.o: $(src)/%.c
	$(CLANG) $(NOSTDINC_FLAGS) $(LINUXINCLUDE) $(EXTRA_CFLAGS) -I$(obj) \
		-I$(srctree)/tools/testing/selftests/bpf/ \
		-D__KERNEL__ -D__ASM_SYSREG_H -Wno-unused-value -Wno-pointer-sign \
		-Wno-compare-distinct-pointer-types \
		-Wno-gnu-variable-sized-type-not-at-end \
		-Wno-address-of-packed-member -Wno-tautological-compare \
		-Wno-unknown-warning-option \
		-O2 -emit-llvm -c $< -o -| $(LLC) -march=bpf -filetype=obj -o $@
