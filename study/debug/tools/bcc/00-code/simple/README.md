    # learn-bpf
    
    This program has been tested with 4.12.5-200.fc25.x86_64.
    
    memcpy_kprobe inserts kprobe at the entry of kernel memcpy() and prints bpf
    kernel messages in trace buffer. memcpy_stat prepares a table in kernel
    space itself for the count of memcpy() called with different sizes.
    
    # make
    # ./memcpy_kprobe
       memcpy_kprobe-24908 [005] d... 151374.866218: : memcpy size 2
       memcpy_kprobe-24908 [005] d... 151374.866221: : memcpy size 13
       memcpy_kprobe-24908 [005] d... 151374.866224: : memcpy size 1
       memcpy_kprobe-24908 [005] d... 151374.866226: : memcpy size 2
       memcpy_kprobe-24908 [005] d... 151374.866229: : memcpy size 2
       memcpy_kprobe-24908 [005] d... 151374.866232: : memcpy size 1
       memcpy_kprobe-24908 [005] d... 151374.866234: : memcpy size 1
       memcpy_kprobe-24908 [005] d... 151374.866237: : memcpy size 2
    
    # ./memcpy_stat
            Size            Count
       0                    134
       1 -   64             10444
      65 -  128             23
     129 -  192             16
     193 -  256             240
     257 -  320             0
     321 -  384             0
     385 -  448             7
     449 -  512             7
     513 -  576             0
     577 -  640             0
     641 -  704             0
     705 -  768             0
     769 -  832             0
     833 -  896             0
     897 -  960             0
     961 - 1024*            107
    * Size > 1024 have been counted in this interval
  
eBPF history and program description
====================================

Writing a simple eBPF application for Kernel Tracing
=====================================================



eBPF an introduction
======================

* BPF machine

In the year 1992, Steven McCanne and Van Jacobson from Lawrence Berkeley
Laboratory [1] proposed a solution to BSD Unix systems for minimising
unwanted network packet copy to user space by implementing an in-kernel
packet filter. This filter is known as Berkeley Packet Filter(BPF). It was
latter introduced in Linux Kernel version 2.1.75 in 1997. 

This filter was aimed to filter all the unwanted packets as early as
possible, so the filtering mechanism had to be shifted from user space
utilities like tcpdump to the in-kernel virtual machine. A group of
assembly like instructions for filtering necessary packet are sent from
user space to kernel by  a system call bpf(). Kernel statically analyzes
the programs before loading them and makes sure that they cannot hang or
harm a running system.

The BPF machine abstraction consists of [1] an accumulator, an index
register ( x ),  a scratch memory store, and an implicit program counter.
It has a small set of arithmetic, logical, and jump instructions.  The
accumulator is used for arithmetic operations, while the index register
provides offsets into the packet or into the scratch memory areas. Lets see
the example of a  small BPF program written in BPF bytecode:

    ldh    [12]
    jeq    #ETHERTYPE_IP, l1, l2
    l1:    ret    #TRUE
    l2:    ret    #0

ldh instruction loads a half word (16 bit) value in accumulator from offset
12 in ethernet packet which is ethernet type field. If it is not an IP
packet then 0 will be returned and so the packet would be rejected.

* BPF JIT compiler

A just in time (JIT) compiler was introduced into kernel [2] in 2011 to
speed up BPF bytecode execution. This compiler translates BPF bytecode into
host system’s assembly code. Such compiler exists for x86_64, SPARC,
PowerPC, ARM, ARM64, MIPS and s390 and can be enabled through
CONFIG_BPF_JIT.

* eBPF machine

Extended BPF (eBPF) is an enhancement over BPF (which is now called as cBPF
i.e. classical BPF) having more resources like 10 registers and 1-8 byte
load/store instructions etc [3]. While BPF was having only forward jump,
eBPF has both backward as well as forward jump, and so we can have a loop.
Of Course kernel takes care that loop still terminates properly. It also
includes global data store which is called maps, and this maps state
persist between events, therefore eBPF can be also used for aggregating
statistics of events. Further, an eBPF program can be written in ‘C’ like
functions, which can be compiled using GCC/LLVM compiler. eBPF has been
designed to be JITed with one to one mapping,  so a very optimized code
having performance as fast as natively compiled code can be generated.

eBPF and tracing review
==========================

* Upstream kernel development

Traditional built-in tracers in Linux are used in post-process manner,
where they would dump fixed event details and then user space tools like
perf or trace-cmd can post process to get required information e.g `perf
stat`.  However, eBPF has ability to prepare user information in kernel
context, and only transfer needed information to user space. So far support
of kprobes, tracepoints and perf_events filtering using eBPF have been
implemented in upstream kernel. They have been supported with arch x86-64,
aarch64, s390x, powerpc64 and sparc64.

One can look into following Linux kernel files to get an insight of it.

- kernel/bpf/
- kernel/trace/bpf_trace.c
- kernel/events/core.c

* User space development

There have been user space tools development for in-kernel tree as well as
out of the kernel tree. Following files/directories are good to look into
upstream kernel for ebpf usage.

- tools/lib/bpf
- tools/perf/util/bpf-loader.c
- samples/bpf/

bcc( https://github.com/iovisor/bcc.git) is another out of kernel tree tool
which has very efficient kernel tracing programs for specific usage(like
`funccount` which counts functions matching a pattern).

Perf has also a bpf interface which can be used to load eBPF object
into kernel.

eBPF tracing: User space to kernel space flow
===============================================

Lets first understand couple of useful entity to interact with ebpf kernel:

* BPF system call

user  can interact using bpf() system call whose prototype is int bpf(int
cmd, union bpf_attr *attr, unsigned int size);
            
One can see `man bpf` for detail about the different possible arguments.
Here, I am providing summary of those arguments.

	*cmd* can be any of the defined enum bpf_cmd, which tells kernel
	mainly about management of map area (like it’s creation, updating,
	deleting or finding an element within it etc), attaching or detaching
	a program etc.

	*attr* can be one of the user defined structure which can be used
	by respective command.

	*size* will be the size of attr.

* BPF Maps:

eBPF tracing calculates the stats in kernel domain itself. We will need
some memory/data structure within the kernel to create such stats.  Maps
are a generic data structure for storage of different types of data in the
form of key-value pair.  They allow sharing of data between eBPF kernel
programs, and also between kernel and user-space applications.

Couple of important attributes for maps:

- Type (map_type)
- maximum number of elements (max_entries)
- key size in bytes (key_size)
- value size in bytes (value_size)

A map can be of different types like Hash, Array, Program array etc. We
need to choose appropriate type as per our needs. For example if key is a
string or not from an integer series then hash map can be used for faster
look up, however if key is like an index then array map will provide the
fastest look up method.

We can not have a key bigger than key_size and can not store a value bigger
than value_size. max_entries is the maximum number of key-value pair which
can be stored within map.

* Some important command:

- BPF_PROG_LOAD: Couple of important attributes for this program.

	prog_type : some of the program type useful for tracing are

		BPF_PROG_TYPE_KPROBE

		BPF_PROG_TYPE_TRACEPOINT,

		BPF_PROG_TYPE_PERF_EVENT,

	insns: 	is pointer to “struct bpf_insn” which has bpf instruction to
		be executed by in-kernel bpf VM.

	insn_cnt: total number of instructions present at insns.

	license:string, which must be GPL compatible to call helper functions
		marked gpl_only.

	kern_version: version of kernel tree

- BPF_MAP_CREATE:  It accepts attributes as discussed in BPF Maps section,
creates a new map and then returns a new file descriptor that refers to the
map. Returned map_fd can be used for lookup or update map elements with
commands like BPF_MAP_LOOKUP_ELEM, BPF_MAP_UPDATE_ELEM, BPF_MAP_DELETE_ELEM
or BPF_MAP_GET_NEXT_KEY. These map manipulation command accepts an
attribute with map_fd, key and value.

Now lets lookup some code which can explain it’s working. See the exmaple
code here: https://github.com/pratyushanand/learn-bpf

Above code is a standalone ebpf demo code which does not need any other
ebpf library code. It has a small library to load different sections of bpf
kernel code (bpf_load.c) and then some wrapper function on top of bpf()
system call (bpf.c) to manipulate map and load kernel bpf code. When we
compile this code we get two executable, memcpy_kprobe and memcpy_stat.
Lets first see what memcpy_kprobe* files do.

For each application we have one *_kern file and another *_user file.
*_kern file has a function “int bpf_prog1(struct pt_regs *ctx)”.  This
function is executed in kernel, so it can access kernel variable and
functions. memcpy_kprobe_kern.c  has three section mappings for program,
license and version respectively. Data from these sections are made part of
attributes of system call bpf(BPF_PROG_LOAD,...) and then kernel executes
loaded bpf instructions as per prog_type attribute. So, bpf code in
memcpy_kprobe_kern.c will be executed when a kprobe instrumented at the
entry of kernel memcpy() is hit. When this bpf code is executed, it will
read 3rd argument of memcpy() ie size of copy and then will print one
statement for “memcpy size” in trace buffer. memcpy_kprobe_user.c loads
kernel program and keeps on reading trace buffer to show what kernel ebpf
program is writing into it.

We have another demo memcpy_stat which prepares stats of memcpy() copy size
in kernel itself. memcpy_stat_kern.c has one more section as maps.
bpf_prog1() reads memcpy() sizes and updates map table.  Corresponding user
space program memcpy_stat_user.c reads map table at every 2 second and
prints stats on console.

Above two simple example can help one to understand, how a user can write
kernel ebpf code for kernel tracing and statistics preparation.

[1] http://www.tcpdump.org/papers/bpf-usenix93.pdf 
[2] https://lwn.net/Articles/437884/
[3] https://www.kernel.org/doc/Documentation/networking/filter.txt
[4] http://events.linuxfoundation.org/sites/events/files/slides/Performance%20Monitoring%20and%20Analysis%20Using%20perf%20and%20BPF_1.pdf


