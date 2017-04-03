/*
 * Kernel GDB tracepoint module.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Copyright(C) KGTP team (http://teawater.github.io/kgtp/), 2010-2014
 *
 */

/* If "* 10" means that this is not a release version.  */
#define GTP_VERSION			(20140510 * 10)

#include <linux/version.h>
#ifndef RHEL_RELEASE_VERSION
#define RHEL_RELEASE_VERSION(a,b)	(((a) << 8) + (b))
#define RHEL_RELEASE_CODE		0
#endif

/* Sepcial config ------------------------------------------------ */
#define GTP_RB

#ifdef GTP_FRAME_SIMPLE
/* This is a debug option.
   This define is for simple frame alloc record, then we can get how many
   memory are weste by FRAME_ALIGN. */
/* #define FRAME_ALLOC_RECORD */
#undef GTP_RB
#endif

#ifdef GTP_FTRACE_RING_BUFFER
#undef GTP_RB
#endif

/* If define USE_PROC, KGTP will use ProcFS instead DebugFS.  */
#ifndef GTP_NO_AUTO_BUILD
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11))
#define USE_PROC
#endif
#endif
#ifndef USE_PROC
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11))
#ifndef NO_WARNING
#warning If got some build error about debugfs, you can use "USE_PROC=1" handle it.
#endif
#endif
#endif

#ifdef GTP_FTRACE_RING_BUFFER
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30))
#ifndef NO_WARNING
#warning If got some build error about ring buffer, you can use "FRAME_SIMPLE=1" handle it.
#endif
#endif
#endif

/* If define GTP_CLOCK_CYCLE, $clock will return rdtscll.  */
#ifndef GTP_NO_AUTO_BUILD
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24))
#define GTP_CLOCK_CYCLE
#endif
#endif
#ifndef GTP_CLOCK_CYCLE
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24))
#ifndef NO_WARNING
#warning If got some build error about cpu_clock or local_clock, you can use "CLOCK_CYCLE=1" handle it.
#endif
#endif
#endif

#ifdef CONFIG_LOCKDEP
#ifndef NO_WARNING
#warning Current kernel open the runtime locking correctness validator (CONFIG_LOCKDEP) that will make KGTP trace functions about locks get deadlock.  Please DO NOT trace function about locks.
#endif
#endif

#ifdef GTP_FTRACE_RING_BUFFER
#ifndef CONFIG_RING_BUFFER
#define CONFIG_RING_BUFFER
#include "ring_buffer.h"
#include "ring_buffer.c"
#define GTP_SELF_RING_BUFFER
#ifndef NO_WARNING
#warning Use the ring buffer inside KGTP.
#endif
#endif
#endif
/* Sepcial config ------------------------------------------------ */

#include <linux/kernel.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
#undef MODULE
#include <linux/preempt.h>
#define MODULE
#endif
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/poll.h>
#include <linux/kprobes.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/debugfs.h>
#include <linux/highmem.h>
#include <linux/pagemap.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/ctype.h>
#include <asm/atomic.h>
#ifdef CONFIG_X86
#include <asm/debugreg.h>
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22))
#include <linux/kdebug.h>
#else
#include <asm/kdebug.h>
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33))
#include <linux/hw_breakpoint.h>
#endif
#include "gtp.h"

#ifdef GTP_FTRACE_RING_BUFFER
#ifndef GTP_SELF_RING_BUFFER
#include <linux/ring_buffer.h>
#endif
#endif
#ifdef CONFIG_PERF_EVENTS
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)) \
    && (RHEL_RELEASE_CODE < RHEL_RELEASE_VERSION(6,1))
#ifndef NO_WARNING
#warning "Current Kernel is too old.  Function of performance counters is not available."
#endif
#else
#include <linux/perf_event.h>
#define GTP_PERF_EVENTS
#endif
#else
#ifndef NO_WARNING
#warning "Current Kernel doesn't open CONFIG_PERF_EVENTS.  Function of performance counters is not available."
#endif
#endif

/* Handle KGTP_API_VERSION for the special kernel that include KGTP_API.  */
#ifdef KGTP_API_VERSION
#define KGTP_API_VERSION_LOCAL	KGTP_API_VERSION
#else
#define KGTP_API_VERSION_LOCAL	0
#endif

#ifndef __percpu
#define __percpu
#endif

#ifndef this_cpu_ptr
#define this_cpu_ptr(v)	per_cpu_ptr(v, smp_processor_id())
#endif

#define KERN_NULL

/* check ---------------------------------------------------------- */
#ifndef CONFIG_KPROBES
#ifndef NO_WARNING
#warning "Cannot trace Linux kernel because Linux Kernel config doesn't open KPROBES.  Please open it in 'General setup->Kprobes' if you need it."
#endif
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0))
#ifndef CONFIG_UPROBES
#ifndef NO_WARNING
#warning "Cannot trace user program because Linux Kernel config doesn't open UPROBES.  Please open it in 'Kernel hacking->Tracers->Enable uprobes-based dynamic events' if you need it."
#endif
#endif
#else
#ifndef NO_WARNING
#warning "Cannot trace user program because the Linux Kernel that older than 3.9 doesn't support UPROBES."
#endif
#endif

#ifdef USE_PROC
#ifndef CONFIG_PROC_FS
#ifndef NO_WARNING
#error "Linux Kernel doesn't support procfs."
#endif
#endif
#else
#ifndef CONFIG_DEBUG_FS
#error "Linux Kernel doesn't support debugfs."
#endif
#endif

#if !defined CONFIG_X86 && !defined CONFIG_MIPS && !defined CONFIG_ARM
#error "KGTP support X86_32, X86_64, MIPS and ARM."
#endif
/* ---------------------------------------------------------------- */

/* Following part is to support old Linux kernel ------------------ */
#ifndef DEFINE_SEMAPHORE
#define DEFINE_SEMAPHORE(name)	DECLARE_MUTEX(name)
#endif

#ifndef list_first_entry
#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)
#endif

/* ---------------------------------------------------------------- */

#ifdef GTPDEBUG
#define GTP_DEBUG		KERN_WARNING
#endif

/* #define GTP_DEBUG_V */

#define GTP_RW_MAX		16384
#define GTP_RW_BUFP_MAX		(GTP_RW_MAX - 4 - gtp_rw_size)

#define FID_TYPE		unsigned int
#define FID_SIZE		sizeof(FID_TYPE)
#define FID(x)			(*((FID_TYPE *)x))
enum {
	FID_HEAD = 0,
	FID_REG,
	FID_MEM,
	FID_VAR,
	FID_END,
	FID_PAGE_BEGIN,
	FID_PAGE_END,
};

/* GTP_FRAME_SIZE must align with FRAME_ALIGN_SIZE if use GTP_FRAME_SIMPLE.  */
#define GTP_FRAME_SIZE		5242880
#if defined(GTP_FRAME_SIMPLE) || defined(GTP_RB)
#define FRAME_ALIGN_SIZE	sizeof(unsigned int)
#define FRAME_ALIGN(x)		((x + FRAME_ALIGN_SIZE - 1) \
				 & (~(FRAME_ALIGN_SIZE - 1)))
#endif
#ifdef GTP_FRAME_SIMPLE
#define GTP_FRAME_HEAD_SIZE	(FID_SIZE + sizeof(char *) + sizeof(ULONGEST))
#define GTP_FRAME_REG_SIZE	(FID_SIZE + sizeof(char *) \
				 + sizeof(struct pt_regs))
#define GTP_FRAME_MEM_SIZE	(FID_SIZE + sizeof(char *) \
				 + sizeof(struct gtp_frame_mem))
#define GTP_FRAME_VAR_SIZE	(FID_SIZE + sizeof(char *) \
				 + sizeof(struct gtp_frame_var))
#endif
#ifdef GTP_RB
/* The frame head size: FID_HEAD + count id + frame number + pointer to prev frem */
#define GTP_FRAME_HEAD_SIZE	(FID_SIZE + sizeof(u64) + sizeof(ULONGEST) + sizeof(void *))
/* The frame head size: FID_PAGE_BEGIN + count id */
#define GTP_FRAME_PAGE_BEGIN_SIZE	(FID_SIZE + sizeof(u64))
#endif
#ifdef GTP_FTRACE_RING_BUFFER
#define GTP_FRAME_HEAD_SIZE	(FID_SIZE + sizeof(ULONGEST))
#endif
#if defined(GTP_FTRACE_RING_BUFFER) || defined(GTP_RB)
#define GTP_FRAME_REG_SIZE	(FID_SIZE + sizeof(struct pt_regs))
#define GTP_FRAME_MEM_SIZE	(FID_SIZE + sizeof(struct gtp_frame_mem))
#define GTP_FRAME_VAR_SIZE	(FID_SIZE + sizeof(struct gtp_frame_var))
#endif

#define INT2CHAR(h)		((h) > 9 ? (h) + 'a' - 10 : (h) + '0')

enum {
	op_check_add = 0xe0,
	op_check_sub,
	op_check_mul,
	op_check_div_signed,
	op_check_div_unsigned,
	op_check_rem_signed,
	op_check_rem_unsigned,
	op_check_lsh,
	op_check_rsh_signed,
	op_check_rsh_unsigned,
	op_check_trace,
	op_check_bit_and,
	op_check_bit_or,
	op_check_bit_xor,
	op_check_equal,
	op_check_less_signed,
	op_check_less_unsigned,
	op_check_pop,
	op_check_swap,
	op_check_if_goto,
	op_check_printf,	/* XXX: still not used.  */

	op_trace_printk = 0xfd,
	op_trace_quick_printk,
	op_tracev_printk,
};

struct action_agent_exp {
	unsigned int	size;
	uint8_t		*buf;
	int		need_var_lock;
};

struct action_m {
	int		regnum;
	CORE_ADDR	offset;
	size_t		size;
};

struct action {
	struct list_head	node;
	unsigned char		type;
	union {
		ULONGEST		reg_mask;
		struct action_agent_exp	exp;
		struct action_m		m;
	} u;
};

struct gtpsrc {
	struct gtpsrc	*next;
	char		*src;
};

enum gtp_stop_type {
	gtp_stop_normal = 0,
	gtp_stop_frame_full,
	gtp_stop_efault,
	gtp_stop_access_wrong_reg,
	gtp_stop_agent_expr_code_error,
	gtp_stop_agent_expr_stack_overflow,
};

/* See $current.  */
#define GTP_ENTRY_FLAGS_CURRENT_TASK	1
/* This gtp entry is registered inside the system.  */
#define GTP_ENTRY_FLAGS_REG		2
/* See $no_self_trace.  */
#define GTP_ENTRY_FLAGS_SELF_TRACE	4
/* This gtp entry has passcount.  */
#define GTP_ENTRY_FLAGS_HAVE_PASS	8
/* See $printk_level.  */
#define GTP_ENTRY_FLAGS_HAVE_PRINTK	16
/* See $kret.  */
#define GTP_ENTRY_FLAGS_IS_KRETPROBE	32

enum gtp_entry_type {
	/* Kernel tracepoint that use kprobes.
	   When the gtp_entry is alloced by function gtp_list_add, all
	   of gtp_entry will set to this type.  */
	gtp_entry_kprobe = 0,

	/* User space program tracepoint that use uprobes.
	   When KGTP try to register the gtp_entry, when gtp_entry->pid
	   is not same with gtpd_task->pid (the pid of Kernel), this
	   gtp_entry will set to this type.  */
	gtp_entry_uprobe,

	/* Watch tracepoint.
	   When tracepoint action is checked by function gtp_check_setv.
	   gtp_entry will be set to there types.  */
	gtp_entry_watch_static,
	gtp_entry_watch_dynamic,
};

#define GTP_IS_WATCH(tpe)	((tpe)->type == gtp_entry_watch_static \
				 || (tpe)->type == gtp_entry_watch_dynamic)

enum gtp_watch_type {
	gtp_watch_exec		= 0,
	gtp_watch_write		= 1,
	gtp_watch_read_write	= 2,
};

struct gtp_entry {
	union gtp_entry_u {
#ifdef CONFIG_KPROBES
		/* For gtp_entry_kprobe.  */
		struct gtp_kp {
			struct kretprobe	kpret;
			struct tasklet_struct	stop_tasklet;
			struct work_struct	stop_work;
		} kp;
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0)) && defined CONFIG_UPROBES
		/* For gtp_entry_uprobe.  */
		struct gtp_up {
			struct inode		*inode;
			loff_t			offset;
			struct uprobe_consumer	uc;
		} up;
#endif

		/* For gtp_entry_watch_static and gtp_entry_watch_dynamic.  */
		struct {
			int type;
			int size;
		} watch;
	} u;
	unsigned long		flags;
	enum gtp_entry_type	type;
	ULONGEST		num;
	ULONGEST		addr;

	/* The actions that set $current help tracepoint get right regs.  */
	struct action		*get_regs;
	/* The actions for the condition.  */
	struct action		*cond;
	struct list_head	action_list;
	int			step;
	struct list_head	step_action_list;

	atomic_t		current_pass;
	struct gtpsrc		*printk_str;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
	/* This is to enable and disable an tracepoint.  */
	struct tasklet_struct	enable_tasklet;
	struct work_struct	enable_work;
	struct tasklet_struct	disable_tasklet;
	struct work_struct	disable_work;
#endif
	enum gtp_stop_type	reason;

	struct gtp_entry	*next;

	/* Pid for this tracepoint.
	   KGTP will use this pid to select Kprobe or Uprobe.
	   And use this pid to get the inode for Uprobe if need.
	   In default, this pid will bet set to gtp_current_pid.  */
	pid_t			pid;

	int			disable;
	ULONGEST		pass;
	struct gtpsrc		*src;
	/* Sometime, it will not same with action
	   because action will be deleted.  */
	struct gtpsrc		*action_cmd;
};

struct gtp_frame_mem {
	CORE_ADDR	addr;
	size_t		size;
};

struct gtp_frame_var {
	unsigned int	num;
	int64_t		val;
};

struct gtpro_entry {
	struct gtpro_entry	*next;
	CORE_ADDR		start;
	CORE_ADDR		end;
};

static pid_t			gtp_gtp_pid;
static unsigned int		gtp_gtp_pid_count;
static pid_t			gtp_gtpframe_pid;
static unsigned int		gtp_gtpframe_pid_count;
#if defined(GTP_FTRACE_RING_BUFFER) || defined(GTP_RB)
static pid_t			gtp_gtpframe_pipe_pid;
#endif

static struct gtp_entry		*gtp_list;
static struct gtp_entry		*current_gtp;
static struct gtpsrc		*current_gtp_action_cmd;
static struct gtpsrc		*current_gtp_src;

static struct workqueue_struct	*gtp_wq;

static int			gtp_read_ack;
static char			*gtp_rw_buf;
static char			*gtp_rw_bufp;
static size_t			gtp_rw_size;

static int			gtp_start;

static int			gtp_disconnected_tracing;
static int			gtp_circular;
#if defined(GTP_FTRACE_RING_BUFFER)			\
    && (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,39))	\
    && !defined(GTP_SELF_RING_BUFFER)
static int			gtp_circular_is_changed;
#endif

static int			gtp_cpu_number;

/* Current number in the frame.  */
static int			gtp_frame_current_num;
/* Current tracepoint id.  */
static ULONGEST			gtp_frame_current_tpe;
static atomic_t			gtp_frame_create;
static char			*gtp_frame_file;
static size_t			gtp_frame_file_size;
static DECLARE_WAIT_QUEUE_HEAD(gtpframe_wq);
#ifdef GTP_FRAME_SIMPLE
static DEFINE_SPINLOCK(gtp_frame_lock);
static char			*gtp_frame;
static char			*gtp_frame_r_start;
static char			*gtp_frame_w_start;
static char			*gtp_frame_end;
static int			gtp_frame_is_circular;
static char			*gtp_frame_current;
#endif
#ifdef GTP_FTRACE_RING_BUFFER
static struct ring_buffer	*gtp_frame;
static struct ring_buffer_iter	*gtp_frame_iter[NR_CPUS];
static int			gtp_frame_current_cpu;
static u64			gtp_frame_current_clock;
#endif

#if defined(GTP_FTRACE_RING_BUFFER) || defined(GTP_RB)
static DECLARE_WAIT_QUEUE_HEAD(gtpframe_pipe_wq);
static atomic_t			gtpframe_pipe_wq_v;
static struct tasklet_struct	gtpframe_pipe_wq_tasklet;
#endif

static struct gtpro_entry	*gtpro_list;

#define GTP_PRINTF_MAX		256
static DEFINE_PER_CPU(char[GTP_PRINTF_MAX], gtp_printf);

#ifdef CONFIG_X86
static DEFINE_PER_CPU(u64, rdtsc_current);
static DEFINE_PER_CPU(u64, rdtsc_offset);
#endif
static DEFINE_PER_CPU(u64, local_clock_current);
static DEFINE_PER_CPU(u64, local_clock_offset);

static uint64_t			gtp_start_last_errno;
static int			gtp_start_ignore_error;

static int			gtp_pipe_trace;

static int			gtp_bt_size;

static int			gtp_noack_mode;

static pid_t			gtp_current_pid;

/* gtpd_task->pid is used as the pid of Linux kernel.  */
static struct task_struct	*gtpd_task;

#ifdef CONFIG_X86
/* Following part is for while-stepping.  */
struct gtp_step_s {
	spinlock_t		lock;
	int			step;
	int			irq_need_open;
	struct gtp_entry	*tpe;
};
static DEFINE_PER_CPU(struct gtp_step_s, gtp_step);
#endif

#ifdef CONFIG_X86
static int	gtp_have_watch_tracepoint;
static int	gtp_have_step;
#endif

#ifdef CONFIG_X86
/* Following part is for watch tracepoint.  */
/* This part is X86 special.  */
#define HWB_NUM			4

static unsigned long		gtp_hwb_drx[HWB_NUM];
static unsigned long		gtp_hwb_dr7;

#define GTP_HWB_DR7_DEF		(0x400UL)
#define GTP_HWB_DR6_MASK	(0xe00fUL)

/* This part is for all the arch.  */
struct gtp_hwb_s {
	struct list_head	node;

	/* This is the number of this hardware breakpoint.  */
	int			num;

	/* This is the address, size and type of this
	   hardware breakpoint.  */
	CORE_ADDR		addr;
	int			size;
	int			type;

	/* The previous of the address that this watch tracepoint
	   watch on.  */
	int64_t			prev_val;

	/* This is the num and address that setup this hardware
	   breakpoints.
	   For the static watch, this is the num and address of this
	   tracepoint.
	   For the dynamic watch, this is the num and address of
	   the tracepoint that call $watch_start.  */
	ULONGEST		trace_num;
	ULONGEST		trace_addr;

	unsigned int		count;

	/* Point to the watchpoint struct.
	   If NULL, this hardware breakpoint is not used.  */
	struct gtp_entry	*watch;
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33))
static struct hw_breakpoint {
	int			num;
	struct perf_event	* __percpu *pev;
} breakinfo[HWB_NUM];
#endif

static struct gtp_hwb_s	gtp_hwb[HWB_NUM];

static LIST_HEAD(gtp_hwb_used_list);
static LIST_HEAD(gtp_hwb_unused_list);

static DEFINE_RWLOCK(gtp_hwb_lock);

static unsigned int	gtp_hwb_sync_count;
static DEFINE_PER_CPU(unsigned int, gtp_hwb_sync_count_local);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
static DEFINE_PER_CPU(struct cpumask, gtp_hwb_sync_cpu_mask);
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,25))
#define gtp_get_debugreg(val, reg)	get_debugreg(val, reg)
#define gtp_set_debugreg(val, reg)	set_debugreg(val, reg)
#else
#define gtp_get_debugreg(val, reg)		\
	do {					\
		switch(reg) {			\
		case 0:				\
			get_debugreg(val, 0);	\
			break;			\
		case 1:				\
			get_debugreg(val, 1);	\
			break;			\
		case 2:				\
			get_debugreg(val, 2);	\
			break;			\
		case 3:				\
			get_debugreg(val, 3);	\
			break;			\
		}				\
	} while (0)
static void
gtp_set_debugreg(unsigned long val, int reg)
{
	switch(reg) {
	case 0:
		gtp_set_debugreg(val, 0);
		break;
	case 1:
		gtp_set_debugreg(val, 1);
		break;
	case 2:
		gtp_set_debugreg(val, 2);
		break;
	case 3:
		gtp_set_debugreg(val, 3);
		break;
	}
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,25)) && !defined(__get_cpu_var)
/*  see include/asm-generic/percpu.h, linux 3.13  */
#ifdef CONFIG_SMP
#define __get_cpu_var(var) (*this_cpu_ptr(&(var)))
#else
#define __get_cpu_var(var)      (*VERIFY_PERCPU_PTR(&(var)))
#endif
#endif

static void
gtp_hwb_stop(void *data)
{
	read_lock(&gtp_hwb_lock);
	__get_cpu_var(gtp_hwb_sync_count_local) = gtp_hwb_sync_count;
	gtp_set_debugreg(0UL, 0);
	gtp_set_debugreg(0UL, 1);
	gtp_set_debugreg(0UL, 2);
	gtp_set_debugreg(0UL, 3);
	gtp_set_debugreg(GTP_HWB_DR7_DEF, 7);
	read_unlock(&gtp_hwb_lock);
}

static void
gtp_hwb_sync_local(void)
{
	__get_cpu_var(gtp_hwb_sync_count_local) = gtp_hwb_sync_count;
	gtp_set_debugreg(gtp_hwb_drx[0], 0);
	gtp_set_debugreg(gtp_hwb_drx[1], 1);
	gtp_set_debugreg(gtp_hwb_drx[2], 2);
	gtp_set_debugreg(gtp_hwb_drx[3], 3);
	gtp_set_debugreg(gtp_hwb_dr7, 7);
}

static void
gtp_hwb_sync(void *data)
{
	gtp_hwb_sync_local();
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
static int
gtp_ipi_handler(struct kprobe *p, struct pt_regs *regs)
{
	read_lock(&gtp_hwb_lock);

	if (__get_cpu_var(gtp_hwb_sync_count_local) != gtp_hwb_sync_count)
		gtp_hwb_sync_local();

	read_unlock(&gtp_hwb_lock);

	return 0;
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
static struct kprobe gtp_ipi_kp;
#endif
#endif

static char *
string2hex(char *pkg, char *out)
{
	char	*ret = out;

	while (pkg[0]) {
		sprintf(out, "%02x", pkg[0]);
		pkg++;
		out += 2;
	}
	out[0] = '\0';

	return ret;
}

/* Strdup begin.  If end is not NULL, it point to the end of this dup.  */

static char *
gtp_strdup(char *begin, char *end)
{
	int	len;
	char	*ret;

	if (end)
		len = end - begin;
	else
		len = strlen(begin);

	ret = kmalloc(len + 1, GFP_KERNEL);
	if (ret == NULL)
		return NULL;

	strncpy(ret, begin, len);
	ret[len] = '\0';

	return ret;
}

/* Following part is for GTP_LOCAL_CLOCK.  */

#define GTP_LOCAL_CLOCK	gtp_local_clock()
#ifdef GTP_CLOCK_CYCLE
static unsigned long long
gtp_local_clock(void)
{
#ifdef CONFIG_X86
	unsigned long long a;
	rdtscll(a);
	return a;
#else
#error "This ARCH cannot get cycle."
#endif
}
#else
static unsigned long long
gtp_local_clock(void)
{
#ifdef CONFIG_HAVE_UNSTABLE_SCHED_CLOCK
	unsigned long flags;
	unsigned int cpu;

	local_irq_save(flags);
	cpu = smp_processor_id();
	local_irq_restore(flags);

	return cpu_clock(cpu);
#else
	return cpu_clock(0);
#endif	/* CONFIG_HAVE_UNSTABLE_SCHED_CLOCK */
}
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26))
static long
probe_kernel_read(void *dst, void *src, size_t size)
{
	long ret;
	mm_segment_t old_fs = get_fs();

	set_fs(KERNEL_DS);

	/* pagefault_disable();*/
	inc_preempt_count();
	barrier();

	ret = __copy_from_user_inatomic(dst,
			(__force const void __user *)src, size);

	/* pagefault_enable(); */
	barrier();
	dec_preempt_count();
	barrier();
	preempt_check_resched();

	set_fs(old_fs);

	return ret ? -EFAULT : 0;
}
#endif

#ifdef GTP_RB
#include "gtp_rb.c"
#endif

/* Following part is for TSV.  */

/* getgtprsp.pl need the ID of TSV.  */

enum {
	GTP_VAR_VERSION_ID			= 1,
	GTP_VAR_CPU_ID				= 2,
	GTP_VAR_CURRENT_TASK_ID			= 3,
	GTP_VAR_CURRENT_THREAD_INFO_ID		= 4,
	GTP_VAR_CLOCK_ID			= 5,
	GTP_VAR_COOKED_CLOCK_ID			= 6,
#ifdef CONFIG_X86
	GTP_VAR_RDTSC_ID			= 7,
	GTP_VAR_COOKED_RDTSC_ID			= 8,
#endif
#ifdef GTP_RB
	GTP_VAR_GTP_RB_DISCARD_PAGE_NUMBER	= 9,
#endif
	GTP_VAR_PRINTK_TMP_ID			= 10,
	GTP_VAR_PRINTK_LEVEL_ID			= 11,
	GTP_VAR_PRINTK_FORMAT_ID		= 12,
	GTP_VAR_DUMP_STACK_ID			= 13,
	GTP_VAR_SELF_TRACE_ID			= 14,
	GTP_VAR_CPU_NUMBER_ID			= 15,
	GTP_VAR_PC_PE_EN_ID			= 16,
	GTP_VAR_KRET_ID				= 17,
	GTP_VAR_XTIME_SEC_ID			= 18,
	GTP_VAR_XTIME_NSEC_ID			= 19,
	GTP_VAR_IGNORE_ERROR_ID			= 20,
	GTP_VAR_LAST_ERRNO_ID			= 21,
	GTP_VAR_HARDIRQ_COUNT_ID		= 22,
	GTP_VAR_SOFTIRQ_COUNT_ID		= 23,
	GTP_VAR_IRQ_COUNT_ID			= 24,
	GTP_VAR_PIPE_TRACE_ID			= 25,
	GTP_VAR_CURRENT_TASK_PID_ID		= 26,
	GTP_VAR_CURRENT_TASK_USER_ID		= 27,
	GTP_VAR_CURRENT_ID			= 28,
	GTP_VAR_BT_ID				= 29,

	GTP_VAR_ENABLE_ID			= 30,
	GTP_VAR_DISABLE_ID			= 31,

	GTP_WATCH_STATIC_ID			= 32,
	GTP_WATCH_TYPE_ID			= 33,
	GTP_WATCH_SIZE_ID			= 34,
	GTP_WATCH_SET_ID_ID			= 35,
	GTP_WATCH_SET_ADDR_ID			= 36,
	GTP_WATCH_START_ID			= 37,
	GTP_WATCH_STOP_ID			= 38,
	GTP_WATCH_TRACE_NUM_ID			= 39,
	GTP_WATCH_TRACE_ADDR_ID			= 40,
	GTP_WATCH_ADDR_ID			= 41,
	GTP_WATCH_VAL_ID			= 42,
	GTP_WATCH_PREV_VAL_ID			= 46,
	GTP_WATCH_COUNT_ID			= 43,

	GTP_STEP_COUNT_ID			= 44,
	GTP_STEP_ID_ID				= 45,

	GTP_INFERIOR_PID_ID			= 47,

	GTP_TASK_PT_REGS_ID			= 48,

	GTP_VAR_SPECIAL_MIN			= GTP_VAR_VERSION_ID,
	GTP_VAR_SPECIAL_MAX			= GTP_TASK_PT_REGS_ID,
};

enum pe_tv_id {
	pe_tv_unknown = 0,
	pe_tv_cpu,
	pe_tv_type,
	pe_tv_config,
	pe_tv_en,
	pe_tv_val,
	pe_tv_enabled,
	pe_tv_running,
};

enum {
	gtp_var_normal = 0,
#ifdef GTP_PERF_EVENTS
	gtp_var_perf_event,
	gtp_var_perf_event_per_cpu,
#endif
	gtp_var_per_cpu,
	gtp_var_special,
};

struct gtp_var;

#ifdef GTP_PERF_EVENTS
struct gtp_var_perf_event	{
	struct gtp_var_perf_event	*pc_next;
	int				en;
	struct perf_event		*event;
	int				cpu;
	u64				val;
	u64				enabled;	/* The perf inside timer */
	u64				running;	/* The perf inside timer */
	char				*name;
	struct perf_event_attr		attr;
};

struct gtp_var_pe	{
	enum pe_tv_id			ptid;
	struct gtp_var_perf_event	*pe;
};
#endif

struct gtp_var_per_cpu {
	union {
		int64_t			val;
#ifdef GTP_PERF_EVENTS
		struct gtp_var_pe	pe;
#endif
	} u;
};

struct gtp_var_pc {
	int				cpu;
	struct gtp_var_per_cpu __percpu	*pc;
};

struct gtp_var {
	struct list_head	node;
	unsigned int		type;
	unsigned int		num;
	int64_t			initial_val;
	char			*src;
	union {
		int64_t			val;
		struct gtp_var_pc	pc;
#ifdef GTP_PERF_EVENTS
		struct gtp_var_pe	pe;
#endif
		struct gtp_var_hooks	*hooks;
	} u;
};

#define gtp_var_get_pc(var)	((struct gtp_var_per_cpu *)((var)->u.pc.cpu < 0 ?  \
							    this_cpu_ptr(var->u.pc.pc)  \
							    : per_cpu_ptr((var)->u.pc.pc, (var)->u.pc.cpu)))
#ifdef GTP_PERF_EVENTS
#define gtp_var_get_pc_pe(var)	(&(gtp_var_get_pc(var)->u.pe))
#define gtp_var_get_pe(var)	((var)->type == gtp_var_perf_event_per_cpu  \
				 ? gtp_var_get_pc_pe(var) : &((var)->u.pe))
#endif

static DEFINE_SPINLOCK(gtp_var_lock);
static LIST_HEAD(gtp_var_list);
static unsigned int	gtp_var_num;
static struct gtp_var	*current_gtp_var;
static struct gtp_var	**gtp_var_array;

static struct gtp_var *
gtp_var_find_num(unsigned int num)
{
	struct gtp_var		*var;
	struct list_head	*cur;

	list_for_each(cur, &gtp_var_list) {
		var = list_entry(cur, struct gtp_var, node);
		if (var->num == num)
			return var;
	}

	return NULL;
}

static struct gtp_var *
gtp_var_find_src(char *src)
{
	struct gtp_var		*var;
	struct list_head	*cur;

	list_for_each(cur, &gtp_var_list) {
		var = list_entry(cur, struct gtp_var, node);
		if (strcmp (var->src + 2, src + 2) == 0)
			return var;
	}

	return NULL;
}

static int
gtp_var_array_find_num(struct gtp_var *var)
{
	int	i;

	for (i = 0; i < gtp_var_num; i++) {
		if (gtp_var_array[i] == var)
			return i;
	}

	return -1;
}

static struct gtp_var *
gtp_var_alloc(int cpu_id, unsigned int num, int num_not_set,
	      int64_t initial_val, char *src)
{
	struct gtp_var	*var;

	if (!num_not_set && gtp_var_find_num(num)) {
		printk(KERN_WARNING "KGTP: TSV number %d already exist.\n",
		       num);
		return ERR_PTR(-EINVAL);
	}
	if (strlen(src) < 4) {
		printk(KERN_WARNING "KGTP: TSV %d's src %s is too short.\n",
		       num, src);
		return ERR_PTR(-EINVAL);
	}
	if (gtp_var_find_src(src)) {
		printk(KERN_WARNING "KGTP: TSV src %s already exist.\n",
		       src);
		return ERR_PTR(-EINVAL);
	}

	if (cpu_id < 0)
		var = kzalloc(sizeof(struct gtp_var), GFP_KERNEL);
	else
		var = kmalloc_node(sizeof(struct gtp_var),
				   GFP_KERNEL | __GFP_ZERO,
				   cpu_to_node(cpu_id));
	if (var == NULL)
		return ERR_PTR(-ENOMEM);
	var->src = gtp_strdup(src, NULL);
	if (var->src == NULL) {
		kfree(var);
		return ERR_PTR(-ENOMEM);
	}

	var->initial_val = initial_val;
	if (num_not_set) {
		num = GTP_VAR_SPECIAL_MAX + 1;
		while (1) {
			struct gtp_var		*var;
			struct list_head	*cur;

			list_for_each(cur, &gtp_var_list) {
				var = list_entry(cur, struct gtp_var, node);
				if (var->num == num)
					break;
			}
			if (cur == &gtp_var_list)
				break;
			num++;
		}
	}
	var->num = num;

	return var;
}

static struct gtp_var *
gtp_var_special_add(unsigned int num, int num_not_set,
		    int64_t initial_val, char *name,
		    struct gtp_var_hooks *hooks)
{
	int		name_len = strlen(name);
	char		src[3 + name_len * 2];
	struct gtp_var	*var;

	if (name_len == 0) {
		printk(KERN_WARNING "KGTP: TSV name %s len cannot be zero.\n",
		       name);
		return ERR_PTR(-EINVAL);
	}

	strcpy(src, "1:");
	string2hex (name, src + 2);

	var = gtp_var_alloc(-1, num, num_not_set, initial_val, src);
	if (IS_ERR(var))
		return var;

	var->type = gtp_var_special;
	var->u.hooks = hooks;

	list_add(&var->node, &gtp_var_list);
	gtp_var_num++;

	return var;
}

static void
gtp_var_release(int include_special)
{
	struct gtp_var		*var;
	struct list_head	*cur, *tmp;

#ifdef GTP_PERF_EVENTS
	/* Remove all data of pe.  */
	while (1) {
		struct gtp_var_perf_event	*pe = NULL;

		list_for_each(cur, &gtp_var_list) {
			var = list_entry(cur, struct gtp_var, node);
			if ((var->type == gtp_var_perf_event
			     || var->type == gtp_var_perf_event_per_cpu)
			    && gtp_var_get_pe(var)->pe) {
				pe = gtp_var_get_pe(var)->pe;
				break;
			}
		}
		if (pe == NULL)
			break;
		if (pe->event)
			perf_event_release_kernel(pe->event);
		kfree(pe->name);
		kfree(pe);
		list_for_each(cur, &gtp_var_list) {
			var = list_entry(cur, struct gtp_var, node);
			if ((var->type == gtp_var_perf_event
			     || var->type == gtp_var_perf_event_per_cpu)
			    && gtp_var_get_pe(var)->pe == pe) {
				gtp_var_get_pe(var)->pe = NULL;
				if (var->type == gtp_var_perf_event_per_cpu)
					var->type = gtp_var_per_cpu;
			}
		}
	}
#endif

	/* Remove all data of pc.  */
	while (1) {
		struct gtp_var_per_cpu	*pc = NULL;

		list_for_each(cur, &gtp_var_list) {
			var = list_entry(cur, struct gtp_var, node);
			if (var->type == gtp_var_per_cpu && var->u.pc.pc) {
				pc = var->u.pc.pc;
				break;
			}
		}
		if (pc == NULL)
			break;
		free_percpu(pc);
		list_for_each(cur, &gtp_var_list) {
			var = list_entry(cur, struct gtp_var, node);
			if ((var->type == gtp_var_per_cpu)
			    && var->u.pc.pc == pc) {
				var->u.pc.pc = NULL;
			}
		}
	}

	list_for_each_safe(cur, tmp, &gtp_var_list) {
		var = list_entry(cur, struct gtp_var, node);
		if (!include_special && var->type == gtp_var_special)
			continue;

		list_del(&var->node);
		gtp_var_num--;
		kfree(var->src);
		kfree(var);
	}
}

static int
gtp_version_hooks_get_val(struct gtp_trace_s *unused1, struct gtp_var *unused2,
			  int64_t *val)
{
	*val = GTP_VERSION;
	return 0;
}

static struct gtp_var_hooks	gtp_version_hooks = {
	.gdb_get_val = gtp_version_hooks_get_val,
	.agent_get_val = gtp_version_hooks_get_val,
};

static int
gtp_cpu_id_hooks_get_val(struct gtp_trace_s *unused1, struct gtp_var *unused2,
			  int64_t *val)
{
	*val = smp_processor_id();
	return 0;
}

static struct gtp_var_hooks	gtp_cpu_id_hooks = {
	.gdb_get_val = gtp_cpu_id_hooks_get_val,
	.agent_get_val = gtp_cpu_id_hooks_get_val,
};

static int
gtp_current_task_hooks_get_val(struct gtp_trace_s *gts,
			       struct gtp_var *unused, int64_t *val)
{
	if (gts->ri)
		*val = (int64_t)(CORE_ADDR)gts->ri->task;
	else
		*val = (int64_t)(CORE_ADDR)get_current();

	return 0;
}

static struct gtp_var_hooks	gtp_current_task_hooks = {
	.agent_get_val = gtp_current_task_hooks_get_val,
};

static int
gtp_current_task_pid_hooks_get_val(struct gtp_trace_s *gts,
				   struct gtp_var *unused2, int64_t *val)
{
	if (gts->ri)
		*val = (uint64_t)(CORE_ADDR)gts->ri->task->pid;
	else
		*val = (uint64_t)(CORE_ADDR)get_current()->pid;

	return 0;
}

static struct gtp_var_hooks	gtp_current_task_pid_hooks = {
	.agent_get_val = gtp_current_task_pid_hooks_get_val,
};

static int
gtp_current_thread_info_hooks_get_val(struct gtp_trace_s *unused1,
				      struct gtp_var *unused2, int64_t *val)
{
	*val = (int64_t)(CORE_ADDR)current_thread_info();

	return 0;
}

static struct gtp_var_hooks	gtp_current_thread_info_hooks = {
	.agent_get_val = gtp_current_thread_info_hooks_get_val,
};

static int
gtp_current_task_user_hooks_get_val(struct gtp_trace_s *unused1,
				    struct gtp_var *unused2, int64_t *val)
{
	*val = (int64_t)user_mode(task_pt_regs(get_current()));

	return 0;
}

static struct gtp_var_hooks	gtp_current_task_user_hooks = {
	.agent_get_val = gtp_current_task_user_hooks_get_val,
};

static int
gtp_current_hooks_set_val(struct gtp_trace_s *gts, struct gtp_var *gtv,
			  int64_t val)
{
	if (gts->tmp_regs == NULL)
		gts->tmp_regs = (struct pt_regs *)(unsigned long)val;
	else {
		printk(KERN_WARNING "KGTP: $current cannot be set twice.");
		return -1;
	}

	return 0;
}

static struct gtp_var_hooks	gtp_current_hooks = {
	.agent_set_val = gtp_current_hooks_set_val,
};

static int
gtp_clock_hooks_get_val(struct gtp_trace_s *unused1,
			struct gtp_var *unused2, int64_t *val)
{
	*val = (int64_t)GTP_LOCAL_CLOCK;

	return 0;
}

static struct gtp_var_hooks	gtp_clock_hooks = {
	.gdb_get_val = gtp_clock_hooks_get_val,
	.agent_get_val = gtp_clock_hooks_get_val,
};

static int
gtp_cooked_clock_hooks_get_val(struct gtp_trace_s *unused1,
			       struct gtp_var *unused2, int64_t *val)
{
	*val = (int64_t)(__get_cpu_var(local_clock_current)
				- __get_cpu_var(local_clock_offset));

	return 0;
}

static struct gtp_var_hooks	gtp_cooked_clock_hooks = {
	.agent_get_val = gtp_cooked_clock_hooks_get_val,
};

#ifdef CONFIG_X86
static int
gtp_rdtsc_hooks_get_val(struct gtp_trace_s *unused1,
			struct gtp_var *unused2, int64_t *val)
{
	unsigned long long a;

	rdtscll(a);
	*val = (int64_t)a;

	return 0;
}

static struct gtp_var_hooks	gtp_rdtsc_hooks = {
	.gdb_get_val = gtp_rdtsc_hooks_get_val,
	.agent_get_val = gtp_rdtsc_hooks_get_val,
};

static int
gtp_cooked_rdtsc_hooks_get_val(struct gtp_trace_s *unused1,
			       struct gtp_var *unused2, int64_t *val)
{
	*val = (int64_t)(__get_cpu_var(rdtsc_current)
				- __get_cpu_var(rdtsc_offset));

	return 0;
}

static struct gtp_var_hooks	gtp_cooked_rdtsc_hooks = {
	.agent_get_val = gtp_cooked_rdtsc_hooks_get_val,
};
#endif

#ifdef GTP_RB
static int
gtp_rb_discard_page_number_hooks_get_val(struct gtp_trace_s *unused1,
					 struct gtp_var *unused2, int64_t *val)
{
	*val = (int64_t)atomic_read(&gtp_rb_discard_page_number);

	return 0;
}

static struct gtp_var_hooks	gtp_rb_discard_page_number_hooks = {
	.gdb_get_val = gtp_rb_discard_page_number_hooks_get_val,
};
#endif

static int
gtp_printk_tmp_hooks_get_val(struct gtp_trace_s *gts,
			     struct gtp_var *unused, int64_t *val)
{
	*val = gts->printk_tmp;

	return 0;
}

static int
gtp_printk_tmp_hooks_set_val(struct gtp_trace_s *gts,
			     struct gtp_var *unused, int64_t val)
{
	gts->printk_tmp = val;

	return 0;
}

static struct gtp_var_hooks	gtp_printk_tmp_hooks = {
	.agent_get_val = gtp_printk_tmp_hooks_get_val,
	.agent_set_val = gtp_printk_tmp_hooks_set_val,
};

static int
gtp_printk_level_hooks_set_val(struct gtp_trace_s *gts,
			       struct gtp_var *unused, int64_t val)
{
	gts->printk_level = (unsigned int)val;

	return 0;
}

static struct gtp_var_hooks	gtp_printk_level_hooks = {
	.agent_set_val = gtp_printk_level_hooks_set_val,
};

static int
gtp_printk_format_hooks_set_val(struct gtp_trace_s *gts,
				struct gtp_var *unused, int64_t val)
{
	gts->printk_format = (unsigned int)val;

	return 0;
}

static struct gtp_var_hooks	gtp_printk_format_hooks = {
	.agent_set_val = gtp_printk_format_hooks_set_val,
};

static int
gtp_dump_stack_hooks_get_val(struct gtp_trace_s *gts,
			     struct gtp_var *unused1, int64_t *val)
{
	printk(KERN_NULL "CPU%d gtp %d %p:", smp_processor_id(),
	       (int)gts->tpe->num, (void *)(CORE_ADDR)gts->tpe->addr);
	dump_stack();
	*val = 0;

	return 0;
}

static struct gtp_var_hooks	gtp_dump_stack_hooks = {
	.agent_get_val = gtp_dump_stack_hooks_get_val,
};

static int
gtp_pipe_trace_hooks_get_val(struct gtp_trace_s *unused1,
			     struct gtp_var *unused2, int64_t *val)
{
	*val = (int64_t)gtp_pipe_trace;

	return 0;
}

static int
gtp_pipe_trace_hooks_set_val(struct gtp_trace_s *unused1,
			     struct gtp_var *unused2, int64_t val)
{
	gtp_pipe_trace = (int)val;

	return 0;
}

static struct gtp_var_hooks	gtp_pipe_trace_hooks = {
	.gdb_get_val = gtp_pipe_trace_hooks_get_val,
	.gdb_set_val = gtp_pipe_trace_hooks_set_val,
};

static int
gtp_cpu_number_hooks_get_val(struct gtp_trace_s *unused1,
			     struct gtp_var *unused2, int64_t *val)
{
	*val = (int64_t)gtp_cpu_number;

	return 0;
}

static struct gtp_var_hooks	gtp_cpu_number_hooks = {
	.gdb_get_val = gtp_cpu_number_hooks_get_val,
	.agent_get_val = gtp_cpu_number_hooks_get_val,
};

static void	gtp_pc_pe_en(int enable);

static int
gtp_pc_pe_en_hooks_set_val(struct gtp_trace_s *unused1,
			   struct gtp_var *unused2, int64_t val)
{
	gtp_pc_pe_en((int)val);

	return 0;
}

static struct gtp_var_hooks	gtp_pc_pe_en_hooks = {
	.agent_set_val = gtp_pc_pe_en_hooks_set_val,
};

static int
gtp_xtime_hooks_agent_get_val(struct gtp_trace_s *gts,
			      struct gtp_var *gtv, int64_t *val)
{
	if (gts->xtime.tv_sec == 0 && gts->xtime.tv_nsec == 0)
		getnstimeofday(&gts->xtime);

	if (gtv->num == GTP_VAR_XTIME_SEC_ID)
		*val = (int64_t)gts->xtime.tv_sec;
	else
		*val = (int64_t)gts->xtime.tv_nsec;

	return 0;
}

static int
gtp_xtime_hooks_gdb_get_val(struct gtp_trace_s *gts,
			    struct gtp_var *gtv, int64_t *val)
{
	struct timespec	time;

	getnstimeofday(&time);
	if (gtv->num == GTP_VAR_XTIME_SEC_ID)
		*val = (int64_t)time.tv_sec;
	else
		*val = (int64_t)time.tv_nsec;

	return 0;
}

static struct gtp_var_hooks	gtp_xtime_hooks = {
	.agent_get_val = gtp_xtime_hooks_agent_get_val,
	.gdb_get_val = gtp_xtime_hooks_gdb_get_val,
};

static int
gtp_ignore_error_hooks_get_val(struct gtp_trace_s *unused1,
			       struct gtp_var *unused2, int64_t *val)
{
	*val = (int64_t)gtp_start_ignore_error;

	return 0;
}

static int
gtp_ignore_error_hooks_set_val(struct gtp_trace_s *unused1,
			       struct gtp_var *unused2, int64_t val)
{
	gtp_start_ignore_error = (int)val;

	return 0;
}

static struct gtp_var_hooks	gtp_ignore_error_hooks = {
	.gdb_get_val = gtp_ignore_error_hooks_get_val,
	.gdb_set_val = gtp_ignore_error_hooks_set_val,
};

static int
gtp_last_errno_hooks_get_val(struct gtp_trace_s *unused1,
			     struct gtp_var *unused2, int64_t *val)
{
	*val = (int64_t)gtp_start_last_errno;

	return 0;
}

static struct gtp_var_hooks	gtp_last_errno_hooks = {
	.gdb_get_val = gtp_last_errno_hooks_get_val,
};

static int
gtp_hardirq_count_hooks_get_val(struct gtp_trace_s *unused1,
				struct gtp_var *unused2, int64_t *val)
{
	*val = (int64_t)hardirq_count();

	return 0;
}

static struct gtp_var_hooks	gtp_hardirq_count_hooks = {
	.agent_get_val = gtp_hardirq_count_hooks_get_val,
};

static int
gtp_softirq_count_hooks_get_val(struct gtp_trace_s *unused1,
				struct gtp_var *unused2, int64_t *val)
{
	*val = (int64_t)softirq_count();

	return 0;
}

static struct gtp_var_hooks	gtp_softirq_count_hooks = {
	.agent_get_val = gtp_softirq_count_hooks_get_val,
};

static int
gtp_irq_count_hooks_get_val(struct gtp_trace_s *unused1,
			    struct gtp_var *unused2, int64_t *val)
{
	*val = (int64_t)irq_count();

	return 0;
}

static struct gtp_var_hooks	gtp_irq_count_hooks = {
	.agent_get_val = gtp_irq_count_hooks_get_val,
};

static int
gtp_bt_hooks_get_val(struct gtp_trace_s *unused1,
		     struct gtp_var *unused2, int64_t *val)
{
	*val = (int64_t)gtp_bt_size;

	return 0;
}

static int
gtp_bt_hooks_set_val(struct gtp_trace_s *unused1,
		     struct gtp_var *unused2, int64_t val)
{
	gtp_bt_size = (int)val;

	return 0;
}

static struct gtp_var_hooks	gtp_bt_hooks = {
	.gdb_get_val = gtp_bt_hooks_get_val,
	.gdb_set_val = gtp_bt_hooks_set_val,
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))

static void	gtp_handler_enable_disable(struct gtp_trace_s *gts,
					   ULONGEST val, int enable);

static int
gtp_enable_disable_hooks_set_val(struct gtp_trace_s *gts,
				 struct gtp_var *gtv, int64_t val)
{
	gtp_handler_enable_disable(gts, val,
				   (gtv->num == GTP_VAR_ENABLE_ID));

	return 0;
}

static struct gtp_var_hooks	gtp_enable_disable_hooks = {
	.agent_set_val = gtp_enable_disable_hooks_set_val,
};
#endif

#ifdef CONFIG_X86
static int
gtp_watch_type_hooks_set_val(struct gtp_trace_s *gts,
			     struct gtp_var *gtv, int64_t val)
{
	if (GTP_IS_WATCH(gts->tpe)) {
		printk(KERN_WARNING "Cannot set $watch_type in hardware breakpoint handler.\n");
		return -1;
	}
	if (val != gtp_watch_exec && val != gtp_watch_write
	    && val != gtp_watch_read_write) {
		printk(KERN_WARNING "$watch_type just support set to 0, 1 or 2.\n");
		return -1;
	}

	gts->watch_type = val;

	return 0;
}

static int
gtp_watch_type_hooks_get_val(struct gtp_trace_s *gts,
			     struct gtp_var *gtv, int64_t *val)
{
	if (!GTP_IS_WATCH(gts->tpe))
		*val = gts->watch_type;
	else
		*val = gts->hwb->type;

	return 0;
}

static struct gtp_var_hooks	gtp_watch_type_hooks = {
	.agent_set_val = gtp_watch_type_hooks_set_val,
	.agent_get_val = gtp_watch_type_hooks_get_val,
};

static int
gtp_watch_size_hooks_set_val(struct gtp_trace_s *gts,
			     struct gtp_var *gtv, int64_t val)
{
	if (GTP_IS_WATCH(gts->tpe)) {
		printk(KERN_WARNING "Cannot set $watch_size in hardware breakpoint handler.\n");
		return -1;
	}
	if (val != 1 && val != 2 && val != 4 && val != 8) {
		printk(KERN_WARNING "$watch_size just support set to 1, 2, 4 or 8.\n");
		return -1;
	}

	gts->watch_size = val;

	return 0;
}

static int
gtp_watch_size_hooks_get_val(struct gtp_trace_s *gts,
			     struct gtp_var *gtv, int64_t *val)
{
	if (!GTP_IS_WATCH(gts->tpe))
		*val = gts->watch_size;
	else
		*val = gts->hwb->size;

	return 0;
}

static struct gtp_var_hooks	gtp_watch_size_hooks = {
	.agent_set_val = gtp_watch_size_hooks_set_val,
	.agent_get_val = gtp_watch_size_hooks_get_val,
};

static struct gtp_entry *
gtp_list_find_watch_num(ULONGEST num)
{
	struct gtp_entry	*tpe;

	for (tpe = gtp_list; tpe; tpe = tpe->next) {
		if (tpe->type == gtp_entry_watch_dynamic
		    && tpe->num == num)
			return tpe;
	}

	return NULL;
}

static struct gtp_entry *
gtp_list_find_watch_addr(ULONGEST addr)
{
	struct gtp_entry	*tpe;

	for (tpe = gtp_list; tpe; tpe = tpe->next) {
		if (tpe->type == gtp_entry_watch_dynamic
		    && tpe->addr == addr)
			return tpe;
	}

	return NULL;
}

static int
gtp_watch_set_hooks_set_val(struct gtp_trace_s *gts,
			    struct gtp_var *gtv, int64_t val)
{
	struct gtp_entry	*tpe;

	if (GTP_IS_WATCH(gts->tpe)) {
		printk(KERN_WARNING "Cannot set $watch_id in hardware breakpoint handler.\n");
		return -1;
	}

	if (gtv->num == GTP_WATCH_SET_ID_ID)
		tpe = gtp_list_find_watch_num(val);
	else
		tpe = gtp_list_find_watch_addr(val);
	if (!tpe) {
		printk(KERN_WARNING "Cannot find dynamic watch tracepoint %s is %lld.\n",
		       (gtv->num == GTP_WATCH_SET_ID_ID) ? "id" : "address", val);
		return -1;
	}

	gts->watch_tpe = tpe;
	gts->watch_type = gtp_watch_write;
	gts->watch_size = 1;

	return 0;
}

static struct gtp_var_hooks	gtp_watch_set_hooks = {
	.agent_set_val = gtp_watch_set_hooks_set_val,
};

static int gtp_register_hwb(const struct gtp_hwb_s *arg, int nowait);
static int gtp_unregister_hwb(CORE_ADDR addr, int sync);

static int
gtp_watch_start_hooks_set_val(struct gtp_trace_s *gts,
			      struct gtp_var *gtv, int64_t val)
{
	struct gtp_hwb_s	arg;

	if (gts->watch_tpe == NULL) {
		printk(KERN_WARNING "Cannot set $watch_id in hardware breakpoint handler.\n");
		return -1;
	}

	arg.addr = val;
	arg.size = gts->watch_size;
	arg.type = gts->watch_type;
	arg.trace_num = gts->tpe->num;
	arg.trace_addr = gts->tpe->addr;
	arg.watch = gts->watch_tpe;
	gts->watch_start_ret = gtp_register_hwb(&arg, 1);

	return 0;
}

static int
gtp_watch_start_hooks_get_val(struct gtp_trace_s *gts,
			      struct gtp_var *gtv, int64_t *val)
{
	*val = gts->watch_start_ret;

	return 0;
}

static struct gtp_var_hooks	gtp_watch_start_hooks = {
	.agent_set_val = gtp_watch_start_hooks_set_val,
	.agent_get_val = gtp_watch_start_hooks_get_val,
};

static int
gtp_watch_stop_hooks_set_val(struct gtp_trace_s *gts,
			     struct gtp_var *gtv, int64_t val)
{
	gts->watch_stop_ret = gtp_unregister_hwb(val, 1);

	return 0;
}

static int
gtp_watch_stop_hooks_get_val(struct gtp_trace_s *gts,
			      struct gtp_var *gtv, int64_t *val)
{
	*val = gts->watch_stop_ret;

	return 0;
}

static struct gtp_var_hooks	gtp_watch_stop_hooks = {
	.agent_set_val = gtp_watch_stop_hooks_set_val,
	.agent_get_val = gtp_watch_stop_hooks_get_val,
};

static int
gtp_watch_get_val(struct gtp_trace_s *gts, struct gtp_var *gtv,
		  int64_t *val)
{
	if (!GTP_IS_WATCH(gts->tpe))
		return -1;

	switch (gtv->num) {
	case GTP_WATCH_TRACE_NUM_ID:
		*val = gts->hwb->trace_num;
		break;
	case GTP_WATCH_TRACE_ADDR_ID:
		*val = gts->hwb->trace_addr;
		break;
	case GTP_WATCH_ADDR_ID:
		*val = gts->hwb->addr;
		break;
	case GTP_WATCH_COUNT_ID:
		*val = gts->hwb->count;
		break;
	default:
		return -1;
		break;
	}

	return 0;
}

static struct gtp_var_hooks	gtp_watch_get_hooks = {
	.agent_get_val = gtp_watch_get_val,
};

static int	gtp_get_addr_val(CORE_ADDR addr, int size, int64_t *val);

static int
gtp_watch_val_get_val(struct gtp_trace_s *gts, struct gtp_var *gtv,
		      int64_t *val)
{
	int	ret;

	if (!GTP_IS_WATCH(gts->tpe))
		return -EINVAL;

	if (gts->hwb_current_val_gotten) {
		*val = gts->hwb_current_val;
		return 0;
	}

	ret = gtp_get_addr_val(gts->hwb->addr, gts->hwb->size, val);
	if (ret == 0) {
		gts->hwb_current_val = *val;
		gts->hwb_current_val_gotten = 1;
	}

	return ret;
}

static struct gtp_var_hooks	gtp_watch_val_hooks = {
	.agent_get_val = gtp_watch_val_get_val,
};

static int
gtp_watch_prev_val_get_val(struct gtp_trace_s *gts, struct gtp_var *gtv,
			   int64_t *val)
{
	if (!GTP_IS_WATCH(gts->tpe))
		return -EINVAL;

	*val = gts->hwb->prev_val;
	return 0;
}

static struct gtp_var_hooks	gtp_watch_prev_val_hooks = {
	.agent_get_val = gtp_watch_prev_val_get_val,
};
#endif

#ifdef CONFIG_X86
static int
gtp_get_addr_val(CORE_ADDR addr, int size, int64_t *val)
{
	int	ret = -EINVAL;

	union {
		union {
			uint8_t	bytes[1];
			uint8_t	val;
		} u8;
		union {
			uint8_t	bytes[2];
			uint16_t val;
		} u16;
		union {
			uint8_t bytes[4];
			uint32_t val;
		} u32;
		union {
			uint8_t bytes[8];
			ULONGEST val;
		} u64;
	} cnv;

	switch (size) {
	case 1:
		ret = probe_kernel_read(cnv.u8.bytes, (void *)addr, 1);
		if (ret)
			goto error;
		*val = (int64_t) cnv.u8.val;
		break;
	case 2:
		ret = probe_kernel_read(cnv.u16.bytes, (void *)addr, 2);
		if (ret)
			goto error;
		*val = (int64_t) cnv.u16.val;
		break;
	case 4:
		ret = probe_kernel_read(cnv.u32.bytes, (void *)addr, 4);
		if (ret)
			goto error;
		*val = (int64_t) cnv.u32.val;
		break;
	case 8:
		ret = probe_kernel_read(cnv.u64.bytes, (void *)addr, 8);
		if (ret)
			goto error;
		*val = (int64_t) cnv.u64.val;
		break;
	default:
		goto error;
		break;
	}

	return 0;

error:
	printk(KERN_WARNING "KGTP: fail get value from address %p size %d.\n",
	       (void *)addr, size);
	return ret;
}
#endif

#ifdef GTP_RB
static int
gtp_step_count_hooks_get_val(struct gtp_trace_s *gts, struct gtp_var *gtv,
			     int64_t *val)
{
	if (gts->step)
		*val = gts->tpe->step - gts->step + 1;
	else
		*val = 0;

	return 0;
}

static struct gtp_var_hooks	gtp_step_count_hooks = {
	.agent_get_val = gtp_step_count_hooks_get_val,
};

static DEFINE_PER_CPU(int64_t, gtp_step_id);

static int
gtp_step_id_hooks_get_val(struct gtp_trace_s *gts, struct gtp_var *gtv,
			  int64_t *val)
{
	if (!gts->step) {
		if (++ __get_cpu_var(gtp_step_id) == 0)
			__get_cpu_var(gtp_step_id) = 1;
	}

	*val = __get_cpu_var(gtp_step_id);

	return 0;
}

static struct gtp_var_hooks	gtp_step_id_hooks = {
	.agent_get_val = gtp_step_id_hooks_get_val,
};
#endif

static int
gtp_inferior_pid_get_val(struct gtp_trace_s *gts, struct gtp_var *gtv,
			 int64_t *val)
{
	*val = gtp_current_pid;
	return 0;
}

static struct gtp_var_hooks	gtp_inferior_pid_hooks = {
	.agent_get_val = gtp_inferior_pid_get_val,
	.gdb_get_val = gtp_inferior_pid_get_val,
};

static int
gtp_task_pt_regs_get_val(struct gtp_trace_s *gts, struct gtp_var *gtv,
			 int64_t *val)
{
#if defined(CONFIG_X86_32) || defined(CONFIG_ARM) || defined(CONFIG_32BIT)
	*val = (uint32_t)task_pt_regs(get_current());
#else
	*val = (uint64_t)task_pt_regs(get_current());
#endif
	return 0;
}

static struct gtp_var_hooks	gtp_task_pt_regs_hooks = {
	.agent_get_val = gtp_task_pt_regs_get_val,
};

static int
gtp_var_special_add_all(void)
{
	struct gtp_var	*var;

	var = gtp_var_special_add(GTP_VAR_VERSION_ID, 0, GTP_VERSION,
				  "gtp_version", &gtp_version_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);

	var = gtp_var_special_add(GTP_VAR_CPU_ID, 0, 0, "cpu_id",
				  &gtp_cpu_id_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);

	var = gtp_var_special_add(GTP_VAR_CURRENT_TASK_ID, 0, 0,
				  "current_task", &gtp_current_task_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);

	var = gtp_var_special_add(GTP_VAR_CURRENT_TASK_PID_ID, 0, 0,
				  "current_task_pid",
				  &gtp_current_task_pid_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);

	var = gtp_var_special_add(GTP_VAR_CURRENT_THREAD_INFO_ID, 0, 0,
				  "current_thread_info",
				  &gtp_current_thread_info_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);

	var = gtp_var_special_add(GTP_VAR_CURRENT_TASK_USER_ID, 0, 0,
				  "current_task_user",
				  &gtp_current_task_user_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);

	var = gtp_var_special_add(GTP_VAR_CURRENT_ID, 0, 0, "current",
				  &gtp_current_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);

	var = gtp_var_special_add(GTP_VAR_CLOCK_ID, 0, 0, "clock",
				  &gtp_clock_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);

	var = gtp_var_special_add(GTP_VAR_COOKED_CLOCK_ID, 0, 0,
				  "cooked_clock", &gtp_cooked_clock_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);

#ifdef CONFIG_X86
	var = gtp_var_special_add(GTP_VAR_RDTSC_ID, 0, 0, "rdtsc",
				  &gtp_rdtsc_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);

	var = gtp_var_special_add(GTP_VAR_COOKED_RDTSC_ID, 0, 0,
				  "cooked_rdtsc", &gtp_cooked_rdtsc_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);
#endif

#ifdef GTP_RB
	var = gtp_var_special_add(GTP_VAR_GTP_RB_DISCARD_PAGE_NUMBER, 0, 0,
				  "gtp_rb_discard_page_number",
				  &gtp_rb_discard_page_number_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);
#endif

	var = gtp_var_special_add(GTP_VAR_PRINTK_TMP_ID, 0, 0,
				  "printk_tmp", &gtp_printk_tmp_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);

	var = gtp_var_special_add(GTP_VAR_PRINTK_LEVEL_ID, 0, 8,
				  "printk_level", &gtp_printk_level_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);

	var = gtp_var_special_add(GTP_VAR_PRINTK_FORMAT_ID, 0, 0,
				  "printk_format", &gtp_printk_format_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);

	var = gtp_var_special_add(GTP_VAR_DUMP_STACK_ID, 0, 0,
				  "dump_stack", &gtp_dump_stack_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);

	var = gtp_var_special_add(GTP_VAR_SELF_TRACE_ID, 0, 0,
				  "self_trace", NULL);
	if (IS_ERR(var))
		return PTR_ERR(var);

	var = gtp_var_special_add(GTP_VAR_PIPE_TRACE_ID, 0, 0,
				  "pipe_trace", &gtp_pipe_trace_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);

	var = gtp_var_special_add(GTP_VAR_CPU_NUMBER_ID, 0, 0,
				  "cpu_number", &gtp_cpu_number_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);

	var = gtp_var_special_add(GTP_VAR_PC_PE_EN_ID, 0, 0,
				  "p_pe_en", &gtp_pc_pe_en_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);

	var = gtp_var_special_add(GTP_VAR_KRET_ID, 0, 0,
				  "kret", NULL);
	if (IS_ERR(var))
		return PTR_ERR(var);

	var = gtp_var_special_add(GTP_VAR_XTIME_SEC_ID, 0, 0,
				  "xtime_sec", &gtp_xtime_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);

	var = gtp_var_special_add(GTP_VAR_XTIME_NSEC_ID, 0, 0,
				  "xtime_nsec", &gtp_xtime_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);

	var = gtp_var_special_add(GTP_VAR_IGNORE_ERROR_ID, 0, 0,
				  "ignore_error", &gtp_ignore_error_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);

	var = gtp_var_special_add(GTP_VAR_LAST_ERRNO_ID, 0, 0,
				  "last_errno", &gtp_last_errno_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);

	var = gtp_var_special_add(GTP_VAR_HARDIRQ_COUNT_ID, 0, 0,
				  "hardirq_count", &gtp_hardirq_count_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);

	var = gtp_var_special_add(GTP_VAR_SOFTIRQ_COUNT_ID, 0, 0,
				  "softirq_count", &gtp_softirq_count_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);

	var = gtp_var_special_add(GTP_VAR_IRQ_COUNT_ID, 0, 0,
				  "irq_count", &gtp_irq_count_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);

	var = gtp_var_special_add(GTP_VAR_BT_ID, 0, 512, "bt",
				  &gtp_bt_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
	var = gtp_var_special_add(GTP_VAR_ENABLE_ID, 0, 0,
				  "enable", &gtp_enable_disable_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);

	var = gtp_var_special_add(GTP_VAR_DISABLE_ID, 0, 0,
				  "disable", &gtp_enable_disable_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);
#endif

#ifdef CONFIG_X86
	var = gtp_var_special_add(GTP_WATCH_STATIC_ID, 0, 0,
				  "watch_static", NULL);
	if (IS_ERR(var))
		return PTR_ERR(var);
	var = gtp_var_special_add(GTP_WATCH_TYPE_ID, 0, 0,
				  "watch_type", &gtp_watch_type_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);
	var = gtp_var_special_add(GTP_WATCH_SIZE_ID, 0, 1,
				  "watch_size", &gtp_watch_size_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);
	var = gtp_var_special_add(GTP_WATCH_SET_ID_ID, 0, 0,
				  "watch_set_id", &gtp_watch_set_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);
	var = gtp_var_special_add(GTP_WATCH_SET_ADDR_ID, 0, 0,
				  "watch_set_addr", &gtp_watch_set_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);
	var = gtp_var_special_add(GTP_WATCH_START_ID, 0, 0,
				  "watch_start", &gtp_watch_start_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);
	var = gtp_var_special_add(GTP_WATCH_STOP_ID, 0, 0,
				  "watch_stop", &gtp_watch_stop_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);
	var = gtp_var_special_add(GTP_WATCH_TRACE_NUM_ID, 0, 0,
				  "watch_trace_num", &gtp_watch_get_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);
	var = gtp_var_special_add(GTP_WATCH_TRACE_ADDR_ID, 0, 0,
				  "watch_trace_addr", &gtp_watch_get_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);
	var = gtp_var_special_add(GTP_WATCH_ADDR_ID, 0, 0,
				  "watch_addr", &gtp_watch_get_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);
	var = gtp_var_special_add(GTP_WATCH_VAL_ID, 0, 0,
				  "watch_val", &gtp_watch_val_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);
	var = gtp_var_special_add(GTP_WATCH_PREV_VAL_ID, 0, 0,
				  "watch_prev_val", &gtp_watch_prev_val_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);
	var = gtp_var_special_add(GTP_WATCH_COUNT_ID, 0, 0,
				  "watch_count", &gtp_watch_get_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);
#endif
	var = gtp_var_special_add(GTP_STEP_COUNT_ID, 0, 0,
				  "step_count", &gtp_step_count_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);
#ifdef GTP_RB
	var = gtp_var_special_add(GTP_STEP_ID_ID, 0, 0,
				  "step_id", &gtp_step_id_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);
#endif

	var = gtp_var_special_add(GTP_INFERIOR_PID_ID, 0, 0,
				  "inferior_pid", &gtp_inferior_pid_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);

	var = gtp_var_special_add(GTP_TASK_PT_REGS_ID, 0, 0,
				  "task_pt_regs", &gtp_task_pt_regs_hooks);
	if (IS_ERR(var))
		return PTR_ERR(var);

	return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)) \
    || (RHEL_RELEASE_CODE != 0 && RHEL_RELEASE_CODE < RHEL_RELEASE_VERSION(5,6))
#ifndef __HAVE_ARCH_STRCASECMP
int strcasecmp(const char *s1, const char *s2)
{
	int c1, c2;

	do {
		c1 = tolower(*s1++);
		c2 = tolower(*s2++);
	} while (c1 == c2 && c1 != 0);
	return c1 - c2;
}
#endif

#ifndef __HAVE_ARCH_STRNCASECMP
int strncasecmp(const char *s1, const char *s2, size_t n)
{
	int c1, c2;

	do {
		c1 = tolower(*s1++);
		c2 = tolower(*s2++);
	} while ((--n > 0) && c1 == c2 && c1 != 0);
	return c1 - c2;
}
#endif
#endif

struct gtp_realloc_s {
	char	*buf;
	size_t	size;
	size_t	real_size;
};

static int
gtp_realloc_alloc(struct gtp_realloc_s *grs, size_t size)
{
	if (size) {
		grs->buf = vmalloc(size);
		if (!grs->buf)
			return -ENOMEM;
	} else
		grs->buf = NULL;

	grs->size = 0;
	grs->real_size = size;

	return 0;
}

static char *
gtp_realloc(struct gtp_realloc_s *grs, size_t size, int is_end)
{
	char	*tmp;

	if (unlikely((grs->real_size < grs->size + size)
		     || (is_end && grs->real_size != grs->size + size))) {
		grs->real_size = grs->size + size;
		if (!is_end)
			grs->real_size += 100;

		tmp = vmalloc(grs->real_size);
		if (!tmp) {
			vfree(grs->buf);
			memset(grs, 0, sizeof(struct gtp_realloc_s));
			return NULL;
		}

		memcpy(tmp, grs->buf, grs->size);
		if (grs->buf)
			vfree(grs->buf);
		grs->buf = tmp;
	}

	grs->size += size;
	return grs->buf + grs->size - size;
}

static int
gtp_realloc_str(struct gtp_realloc_s *grs, char *str, int is_end)
{
	char	*wbuf;
	int	str_len = strlen(str);

	wbuf = gtp_realloc(grs, str_len, is_end);
	if (wbuf == NULL)
		return -ENOMEM;

	memcpy(wbuf, str, str_len);

	return 0;
}

static inline void
gtp_realloc_reset(struct gtp_realloc_s *grs)
{
	grs->size = 0;
}

static inline int
gtp_realloc_is_alloced(struct gtp_realloc_s *grs)
{
	return (grs->buf != NULL);
}

static inline int
gtp_realloc_is_empty(struct gtp_realloc_s *grs)
{
	return (grs->size == 0);
}

static inline void
gtp_realloc_sub_size(struct gtp_realloc_s *grs, size_t size)
{
	grs->size -= size;
}

#ifdef CONFIG_X86
ULONGEST
gtp_action_reg_read(struct gtp_trace_s *gts, int num)
{
	ULONGEST	ret;

	switch (num) {
#ifdef CONFIG_X86_32
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,25))
	case 0:
		ret = gts->regs->ax;
		break;
	case 1:
		ret = gts->regs->cx;
		break;
	case 2:
		ret = gts->regs->dx;
		break;
	case 3:
		ret = gts->regs->bx;
		break;
	case 4:
		ret = gts->x86_32_sp;
		break;
	case 5:
		ret = gts->regs->bp;
		break;
	case 6:
		ret = gts->regs->si;
		break;
	case 7:
		ret = gts->regs->di;
		break;
	case 8:
		if (GTP_X86_NEED_ADJUST_PC(gts))
			ret = gts->regs->ip - 1;
		else
			ret = gts->regs->ip;
		break;
	case 9:
		ret = gts->regs->flags;
		break;
	case 10:
		ret = gts->regs->cs;
		break;
	case 11:
		ret = gts->regs->ss;
		break;
	case 12:
		ret = gts->regs->ds;
		break;
	case 13:
		ret = gts->regs->es;
		break;
	case 14:
		ret = gts->regs->fs;
		break;
	case 15:
		ret = gts->regs->gs;
		break;
#else
	case 0:
		ret = gts->regs->eax;
		break;
	case 1:
		ret = gts->regs->ecx;
		break;
	case 2:
		ret = gts->regs->edx;
		break;
	case 3:
		ret = gts->regs->ebx;
		break;
	case 4:
		ret = gts->x86_32_sp;
		break;
	case 5:
		ret = gts->regs->ebp;
		break;
	case 6:
		ret = gts->regs->esi;
		break;
	case 7:
		ret = gts->regs->edi;
		break;
	case 8:
		if (GTP_X86_NEED_ADJUST_PC(gts))
			ret = gts->regs->eip - 1;
		else
			ret = gts->regs->eip;
		break;
	case 9:
		ret = gts->regs->eflags;
		break;
	case 10:
		ret = gts->regs->xcs;
		break;
	case 11:
		ret = gts->regs->xss;
		break;
	case 12:
		ret = gts->regs->xds;
		break;
	case 13:
		ret = gts->regs->xes;
		break;
	case 14:
		/* ret = gts->regs->xfs; */
		ret = 0;
		break;
	case 15:
		/* ret = gts->regs->xgs; */
		ret = 0;
		break;
#endif
#else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,25))
	case 0:
		ret = gts->regs->ax;
		break;
	case 1:
		ret = gts->regs->bx;
		break;
	case 2:
		ret = gts->regs->cx;
		break;
	case 3:
		ret = gts->regs->dx;
		break;
	case 4:
		ret = gts->regs->si;
		break;
	case 5:
		ret = gts->regs->di;
		break;
	case 6:
		ret = gts->regs->bp;
		break;
	case 7:
		ret = gts->regs->sp;
		break;
	case 16:
		if (GTP_X86_NEED_ADJUST_PC(gts))
			ret = gts->regs->ip - 1;
		else
			ret = gts->regs->ip;
		break;
	case 17:
		ret = gts->regs->flags;
		break;
#else
	case 0:
		ret = gts->regs->rax;
		break;
	case 1:
		ret = gts->regs->rbx;
		break;
	case 2:
		ret = gts->regs->rcx;
		break;
	case 3:
		ret = gts->regs->rdx;
		break;
	case 4:
		ret = gts->regs->rsi;
		break;
	case 5:
		ret = gts->regs->rdi;
		break;
	case 6:
		ret = gts->regs->rbp;
		break;
	case 7:
		ret = gts->regs->rsp;
		break;
	case 16:
		if (GTP_X86_NEED_ADJUST_PC(gts))
			ret = gts->regs->rip - 1;
		else
			ret = gts->regs->rip;
		break;
	case 17:
		ret = gts->regs->eflags;
		break;
#endif
	case 8:
		ret = gts->regs->r8;
		break;
	case 9:
		ret = gts->regs->r9;
		break;
	case 10:
		ret = gts->regs->r10;
		break;
	case 11:
		ret = gts->regs->r11;
		break;
	case 12:
		ret = gts->regs->r12;
		break;
	case 13:
		ret = gts->regs->r13;
		break;
	case 14:
		ret = gts->regs->r14;
		break;
	case 15:
		ret = gts->regs->r15;
		break;
	case 18:
		ret = gts->regs->cs;
		break;
	case 19:
		ret = gts->regs->ss;
		break;
#endif
	default:
		ret = 0;
		gts->tpe->reason = gtp_stop_access_wrong_reg;
		break;
	}

	return ret;
}
EXPORT_SYMBOL(gtp_action_reg_read);

static void
gtp_regs2ascii(struct pt_regs *regs, char *buf)
{
#ifdef CONFIG_X86_32
#ifdef GTP_DEBUG_V
	printk(GTP_DEBUG_V "gtp_regs2ascii: ax = 0x%x\n",
		(unsigned int) regs->ax);
	printk(GTP_DEBUG_V "gtp_regs2ascii: cx = 0x%x\n",
		(unsigned int) regs->cx);
	printk(GTP_DEBUG_V "gtp_regs2ascii: dx = 0x%x\n",
		(unsigned int) regs->dx);
	printk(GTP_DEBUG_V "gtp_regs2ascii: bx = 0x%x\n",
		(unsigned int) regs->bx);
	printk(GTP_DEBUG_V "gtp_regs2ascii: sp = 0x%x\n",
		(unsigned int) regs->sp);
	printk(GTP_DEBUG_V "gtp_regs2ascii: bp = 0x%x\n",
		(unsigned int) regs->bp);
	printk(GTP_DEBUG_V "gtp_regs2ascii: si = 0x%x\n",
		(unsigned int) regs->si);
	printk(GTP_DEBUG_V "gtp_regs2ascii: di = 0x%x\n",
		(unsigned int) regs->di);
	printk(GTP_DEBUG_V "gtp_regs2ascii: ip = 0x%x\n",
		(unsigned int) regs->ip);
	printk(GTP_DEBUG_V "gtp_regs2ascii: flags = 0x%x\n",
		(unsigned int) regs->flags);
	printk(GTP_DEBUG_V "gtp_regs2ascii: cs = 0x%x\n",
		(unsigned int) regs->cs);
	printk(GTP_DEBUG_V "gtp_regs2ascii: ss = 0x%x\n",
		(unsigned int) regs->ss);
	printk(GTP_DEBUG_V "gtp_regs2ascii: ds = 0x%x\n",
		(unsigned int) regs->ds);
	printk(GTP_DEBUG_V "gtp_regs2ascii: es = 0x%x\n",
		(unsigned int) regs->es);
	printk(GTP_DEBUG_V "gtp_regs2ascii: fs = 0x%x\n",
		(unsigned int) regs->fs);
	printk(GTP_DEBUG_V "gtp_regs2ascii: gs = 0x%x\n",
		(unsigned int) regs->gs);
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,25))
	sprintf(buf, "%08x", (unsigned int) swab32(regs->ax));
	buf += 8;
	sprintf(buf, "%08x", (unsigned int) swab32(regs->cx));
	buf += 8;
	sprintf(buf, "%08x", (unsigned int) swab32(regs->dx));
	buf += 8;
	sprintf(buf, "%08x", (unsigned int) swab32(regs->bx));
	buf += 8;
	sprintf(buf, "%08x", (unsigned int) swab32(regs->sp));
	buf += 8;
	sprintf(buf, "%08x", (unsigned int) swab32(regs->bp));
	buf += 8;
	sprintf(buf, "%08x", (unsigned int) swab32(regs->si));
	buf += 8;
	sprintf(buf, "%08x", (unsigned int) swab32(regs->di));
	buf += 8;
	sprintf(buf, "%08x", (unsigned int) swab32(regs->ip));
	buf += 8;
	sprintf(buf, "%08x", (unsigned int) swab32(regs->flags));
	buf += 8;
	sprintf(buf, "%08x", (unsigned int) swab32(regs->cs));
	buf += 8;
	sprintf(buf, "%08x", (unsigned int) swab32(regs->ss));
	buf += 8;
	sprintf(buf, "%08x", (unsigned int) swab32(regs->ds));
	buf += 8;
	sprintf(buf, "%08x", (unsigned int) swab32(regs->es));
	buf += 8;
	sprintf(buf, "%08x", (unsigned int) swab32(regs->fs));
	buf += 8;
	sprintf(buf, "%08x", (unsigned int) swab32(regs->gs));
	buf += 8;
#else
	sprintf(buf, "%08x", (unsigned int) swab32(regs->eax));
	buf += 8;
	sprintf(buf, "%08x", (unsigned int) swab32(regs->ecx));
	buf += 8;
	sprintf(buf, "%08x", (unsigned int) swab32(regs->edx));
	buf += 8;
	sprintf(buf, "%08x", (unsigned int) swab32(regs->ebx));
	buf += 8;
	sprintf(buf, "%08x", (unsigned int) swab32(regs->esp));
	buf += 8;
	sprintf(buf, "%08x", (unsigned int) swab32(regs->ebp));
	buf += 8;
	sprintf(buf, "%08x", (unsigned int) swab32(regs->esi));
	buf += 8;
	sprintf(buf, "%08x", (unsigned int) swab32(regs->edi));
	buf += 8;
	sprintf(buf, "%08x", (unsigned int) swab32(regs->eip));
	buf += 8;
	sprintf(buf, "%08x", (unsigned int) swab32(regs->eflags));
	buf += 8;
	sprintf(buf, "%08x", (unsigned int) swab32(regs->xcs));
	buf += 8;
	sprintf(buf, "%08x", (unsigned int) swab32(regs->xss));
	buf += 8;
	sprintf(buf, "%08x", (unsigned int) swab32(regs->xds));
	buf += 8;
	sprintf(buf, "%08x", (unsigned int) swab32(regs->xes));
	buf += 8;
	/* sprintf(buf, "%08x", (unsigned int) swab32(regs->xfs)); */
	sprintf(buf, "00000000");
	buf += 8;
	/* sprintf(buf, "%08x", (unsigned int) swab32(regs->xgs)); */
	sprintf(buf, "00000000");
	buf += 8;
#endif
#else
#ifdef GTP_DEBUG_V
	printk(GTP_DEBUG_V "gtp_regs2ascii: ax = 0x%lx\n", regs->ax);
	printk(GTP_DEBUG_V "gtp_regs2ascii: bx = 0x%lx\n", regs->bx);
	printk(GTP_DEBUG_V "gtp_regs2ascii: cx = 0x%lx\n", regs->cx);
	printk(GTP_DEBUG_V "gtp_regs2ascii: dx = 0x%lx\n", regs->dx);
	printk(GTP_DEBUG_V "gtp_regs2ascii: si = 0x%lx\n", regs->si);
	printk(GTP_DEBUG_V "gtp_regs2ascii: di = 0x%lx\n", regs->di);
	printk(GTP_DEBUG_V "gtp_regs2ascii: bp = 0x%lx\n", regs->bp);
	printk(GTP_DEBUG_V "gtp_regs2ascii: sp = 0x%lx\n", regs->sp);
	printk(GTP_DEBUG_V "gtp_regs2ascii: r8 = 0x%lx\n", regs->r8);
	printk(GTP_DEBUG_V "gtp_regs2ascii: r9 = 0x%lx\n", regs->r9);
	printk(GTP_DEBUG_V "gtp_regs2ascii: r10 = 0x%lx\n", regs->r10);
	printk(GTP_DEBUG_V "gtp_regs2ascii: r11 = 0x%lx\n", regs->r11);
	printk(GTP_DEBUG_V "gtp_regs2ascii: r12 = 0x%lx\n", regs->r12);
	printk(GTP_DEBUG_V "gtp_regs2ascii: r13 = 0x%lx\n", regs->r13);
	printk(GTP_DEBUG_V "gtp_regs2ascii: r14 = 0x%lx\n", regs->r14);
	printk(GTP_DEBUG_V "gtp_regs2ascii: r15 = 0x%lx\n", regs->r15);
	printk(GTP_DEBUG_V "gtp_regs2ascii: ip = 0x%lx\n", regs->ip);
	printk(GTP_DEBUG_V "gtp_regs2ascii: flags = 0x%lx\n", regs->flags);
	printk(GTP_DEBUG_V "gtp_regs2ascii: cs = 0x%lx\n", regs->cs);
	printk(GTP_DEBUG_V "gtp_regs2ascii: ss = 0x%lx\n", regs->ss);
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,25))
	sprintf(buf, "%016lx", (unsigned long) swab64(regs->ax));
	buf += 16;
	sprintf(buf, "%016lx", (unsigned long) swab64(regs->bx));
	buf += 16;
	sprintf(buf, "%016lx", (unsigned long) swab64(regs->cx));
	buf += 16;
	sprintf(buf, "%016lx", (unsigned long) swab64(regs->dx));
	buf += 16;
	sprintf(buf, "%016lx", (unsigned long) swab64(regs->si));
	buf += 16;
	sprintf(buf, "%016lx", (unsigned long) swab64(regs->di));
	buf += 16;
	sprintf(buf, "%016lx", (unsigned long) swab64(regs->bp));
	buf += 16;
	sprintf(buf, "%016lx", (unsigned long) swab64(regs->sp));
	buf += 16;
#else
	sprintf(buf, "%016lx", (unsigned long) swab64(regs->rax));
	buf += 16;
	sprintf(buf, "%016lx", (unsigned long) swab64(regs->rbx));
	buf += 16;
	sprintf(buf, "%016lx", (unsigned long) swab64(regs->rcx));
	buf += 16;
	sprintf(buf, "%016lx", (unsigned long) swab64(regs->rdx));
	buf += 16;
	sprintf(buf, "%016lx", (unsigned long) swab64(regs->rsi));
	buf += 16;
	sprintf(buf, "%016lx", (unsigned long) swab64(regs->rdi));
	buf += 16;
	sprintf(buf, "%016lx", (unsigned long) swab64(regs->rbp));
	buf += 16;
	sprintf(buf, "%016lx", (unsigned long) swab64(regs->rsp));
	buf += 16;
#endif
	sprintf(buf, "%016lx", (unsigned long) swab64(regs->r8));
	buf += 16;
	sprintf(buf, "%016lx", (unsigned long) swab64(regs->r9));
	buf += 16;
	sprintf(buf, "%016lx", (unsigned long) swab64(regs->r10));
	buf += 16;
	sprintf(buf, "%016lx", (unsigned long) swab64(regs->r11));
	buf += 16;
	sprintf(buf, "%016lx", (unsigned long) swab64(regs->r12));
	buf += 16;
	sprintf(buf, "%016lx", (unsigned long) swab64(regs->r13));
	buf += 16;
	sprintf(buf, "%016lx", (unsigned long) swab64(regs->r14));
	buf += 16;
	sprintf(buf, "%016lx", (unsigned long) swab64(regs->r15));
	buf += 16;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,25))
	sprintf(buf, "%016lx", (unsigned long) swab64(regs->ip));
	buf += 16;
	sprintf(buf, "%08x",
		(unsigned int) swab32((unsigned int)regs->flags));
	buf += 8;
#else
	sprintf(buf, "%016lx", (unsigned long) swab64(regs->rip));
	buf += 16;
	sprintf(buf, "%08x",
		(unsigned int) swab32((unsigned int)regs->eflags));
	buf += 8;
#endif
	sprintf(buf, "%08x",
		(unsigned int) swab32((unsigned int)regs->cs));
	buf += 8;
	sprintf(buf, "%08x",
		(unsigned int) swab32((unsigned int)regs->ss));
	buf += 8;
#endif
}

static void
gtp_regs2bin(struct pt_regs *regs, char *buf)
{
#ifdef CONFIG_X86_32
#ifdef GTP_DEBUG_V
	printk(GTP_DEBUG_V "gtp_regs2ascii: ax = 0x%x\n",
		(unsigned int) regs->ax);
	printk(GTP_DEBUG_V "gtp_regs2ascii: cx = 0x%x\n",
		(unsigned int) regs->cx);
	printk(GTP_DEBUG_V "gtp_regs2ascii: dx = 0x%x\n",
		(unsigned int) regs->dx);
	printk(GTP_DEBUG_V "gtp_regs2ascii: bx = 0x%x\n",
		(unsigned int) regs->bx);
	printk(GTP_DEBUG_V "gtp_regs2ascii: sp = 0x%x\n",
		(unsigned int) regs->sp);
	printk(GTP_DEBUG_V "gtp_regs2ascii: bp = 0x%x\n",
		(unsigned int) regs->bp);
	printk(GTP_DEBUG_V "gtp_regs2ascii: si = 0x%x\n",
		(unsigned int) regs->si);
	printk(GTP_DEBUG_V "gtp_regs2ascii: di = 0x%x\n",
		(unsigned int) regs->di);
	printk(GTP_DEBUG_V "gtp_regs2ascii: ip = 0x%x\n",
		(unsigned int) regs->ip);
	printk(GTP_DEBUG_V "gtp_regs2ascii: flags = 0x%x\n",
		(unsigned int) regs->flags);
	printk(GTP_DEBUG_V "gtp_regs2ascii: cs = 0x%x\n",
		(unsigned int) regs->cs);
	printk(GTP_DEBUG_V "gtp_regs2ascii: ss = 0x%x\n",
		(unsigned int) regs->ss);
	printk(GTP_DEBUG_V "gtp_regs2ascii: ds = 0x%x\n",
		(unsigned int) regs->ds);
	printk(GTP_DEBUG_V "gtp_regs2ascii: es = 0x%x\n",
		(unsigned int) regs->es);
	printk(GTP_DEBUG_V "gtp_regs2ascii: fs = 0x%x\n",
		(unsigned int) regs->fs);
	printk(GTP_DEBUG_V "gtp_regs2ascii: gs = 0x%x\n",
		(unsigned int) regs->gs);
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,25))
	memcpy(buf, &regs->ax, 4);
	buf += 4;
	memcpy(buf, &regs->cx, 4);
	buf += 4;
	memcpy(buf, &regs->dx, 4);
	buf += 4;
	memcpy(buf, &regs->bx, 4);
	buf += 4;
	memcpy(buf, &regs->sp, 4);
	buf += 4;
	memcpy(buf, &regs->bp, 4);
	buf += 4;
	memcpy(buf, &regs->si, 4);
	buf += 4;
	memcpy(buf, &regs->di, 4);
	buf += 4;
	memcpy(buf, &regs->ip, 4);
	buf += 4;
	memcpy(buf, &regs->flags, 4);
	buf += 4;
	memcpy(buf, &regs->cs, 4);
	buf += 4;
	memcpy(buf, &regs->ss, 4);
	buf += 4;
	memcpy(buf, &regs->ds, 4);
	buf += 4;
	memcpy(buf, &regs->es, 4);
	buf += 4;
	memcpy(buf, &regs->fs, 4);
	buf += 4;
	memcpy(buf, &regs->gs, 4);
	buf += 4;
#else
	memcpy(buf, &regs->eax, 4);
	buf += 4;
	memcpy(buf, &regs->ecx, 4);
	buf += 4;
	memcpy(buf, &regs->edx, 4);
	buf += 4;
	memcpy(buf, &regs->ebx, 4);
	buf += 4;
	memcpy(buf, &regs->esp, 4);
	buf += 4;
	memcpy(buf, &regs->ebp, 4);
	buf += 4;
	memcpy(buf, &regs->esi, 4);
	buf += 4;
	memcpy(buf, &regs->edi, 4);
	buf += 4;
	memcpy(buf, &regs->eip, 4);
	buf += 4;
	memcpy(buf, &regs->eflags, 4);
	buf += 4;
	memcpy(buf, &regs->xcs, 4);
	buf += 4;
	memcpy(buf, &regs->xss, 4);
	buf += 4;
	memcpy(buf, &regs->xds, 4);
	buf += 4;
	memcpy(buf, &regs->xes, 4);
	buf += 4;
	/* memcpy(buf, &regs->xfs, 4); */
	memset(buf, '\0', 4);
	buf += 4;
	/* memcpy(buf, &regs->xgs, 4); */
	memset(buf, '\0', 4);
	buf += 4;
#endif
#else
#ifdef GTP_DEBUG_V
	printk(GTP_DEBUG_V "gtp_regs2ascii: ax = 0x%lx\n", regs->ax);
	printk(GTP_DEBUG_V "gtp_regs2ascii: bx = 0x%lx\n", regs->bx);
	printk(GTP_DEBUG_V "gtp_regs2ascii: cx = 0x%lx\n", regs->cx);
	printk(GTP_DEBUG_V "gtp_regs2ascii: dx = 0x%lx\n", regs->dx);
	printk(GTP_DEBUG_V "gtp_regs2ascii: si = 0x%lx\n", regs->si);
	printk(GTP_DEBUG_V "gtp_regs2ascii: di = 0x%lx\n", regs->di);
	printk(GTP_DEBUG_V "gtp_regs2ascii: bp = 0x%lx\n", regs->bp);
	printk(GTP_DEBUG_V "gtp_regs2ascii: sp = 0x%lx\n", regs->sp);
	printk(GTP_DEBUG_V "gtp_regs2ascii: r8 = 0x%lx\n", regs->r8);
	printk(GTP_DEBUG_V "gtp_regs2ascii: r9 = 0x%lx\n", regs->r9);
	printk(GTP_DEBUG_V "gtp_regs2ascii: r10 = 0x%lx\n", regs->r10);
	printk(GTP_DEBUG_V "gtp_regs2ascii: r11 = 0x%lx\n", regs->r11);
	printk(GTP_DEBUG_V "gtp_regs2ascii: r12 = 0x%lx\n", regs->r12);
	printk(GTP_DEBUG_V "gtp_regs2ascii: r13 = 0x%lx\n", regs->r13);
	printk(GTP_DEBUG_V "gtp_regs2ascii: r14 = 0x%lx\n", regs->r14);
	printk(GTP_DEBUG_V "gtp_regs2ascii: r15 = 0x%lx\n", regs->r15);
	printk(GTP_DEBUG_V "gtp_regs2ascii: ip = 0x%lx\n", regs->ip);
	printk(GTP_DEBUG_V "gtp_regs2ascii: flags = 0x%lx\n", regs->flags);
	printk(GTP_DEBUG_V "gtp_regs2ascii: cs = 0x%lx\n", regs->cs);
	printk(GTP_DEBUG_V "gtp_regs2ascii: ss = 0x%lx\n", regs->ss);
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,25))
	memcpy(buf, &regs->ax, 8);
	buf += 8;
	memcpy(buf, &regs->bx, 8);
	buf += 8;
	memcpy(buf, &regs->cx, 8);
	buf += 8;
	memcpy(buf, &regs->dx, 8);
	buf += 8;
	memcpy(buf, &regs->si, 8);
	buf += 8;
	memcpy(buf, &regs->di, 8);
	buf += 8;
	memcpy(buf, &regs->bp, 8);
	buf += 8;
	memcpy(buf, &regs->sp, 8);
	buf += 8;
#else
	memcpy(buf, &regs->rax, 8);
	buf += 8;
	memcpy(buf, &regs->rbx, 8);
	buf += 8;
	memcpy(buf, &regs->rcx, 8);
	buf += 8;
	memcpy(buf, &regs->rdx, 8);
	buf += 8;
	memcpy(buf, &regs->rsi, 8);
	buf += 8;
	memcpy(buf, &regs->rdi, 8);
	buf += 8;
	memcpy(buf, &regs->rbp, 8);
	buf += 8;
	memcpy(buf, &regs->rsp, 8);
	buf += 8;
#endif
	memcpy(buf, &regs->r8, 8);
	buf += 8;
	memcpy(buf, &regs->r9, 8);
	buf += 8;
	memcpy(buf, &regs->r10, 8);
	buf += 8;
	memcpy(buf, &regs->r11, 8);
	buf += 8;
	memcpy(buf, &regs->r12, 8);
	buf += 8;
	memcpy(buf, &regs->r13, 8);
	buf += 8;
	memcpy(buf, &regs->r14, 8);
	buf += 8;
	memcpy(buf, &regs->r15, 8);
	buf += 8;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,25))
	memcpy(buf, &regs->ip, 8);
	buf += 8;
	memcpy(buf, &regs->flags, 4);
	buf += 4;
#else
	memcpy(buf, &regs->rip, 8);
	buf += 8;
	memcpy(buf, &regs->eflags, 4);
	buf += 4;
#endif
	memcpy(buf, &regs->cs, 4);
	buf += 4;
	memcpy(buf, &regs->ss, 4);
	buf += 4;
#endif
}
#endif

#ifdef CONFIG_MIPS
static ULONGEST
gtp_action_reg_read(struct gtp_trace_s *gts, int num)
{
	ULONGEST	ret;

	if (num > 90) {
		/* GDB convert the reg number to a GDB
		   [1 * gdbarch_num_regs .. 2 * gdbarch_num_regs) REGNUM
		   in function mips_dwarf_dwarf2_ecoff_reg_to_regnum.  */
		num -= 90;
	}

	if (num >= 0 && num <= 31) {
		ret = gts->regs->regs[num];
	} else {
		switch (num) {
		case 32:
			ret = gts->regs->cp0_status;
			break;
		case 33:
			ret = gts->regs->lo;
			break;
		case 34:
			ret = gts->regs->hi;
			break;
		case 35:
			ret = gts->regs->cp0_badvaddr;
			break;
		case 36:
			ret = gts->regs->cp0_cause;
			break;
		case 37:
			ret = gts->regs->cp0_epc;
			break;
		default:
			ret = 0;
			gts->tpe->reason = gtp_stop_access_wrong_reg;
			break;
		}
	}

	return ret;
};
EXPORT_SYMBOL(gtp_action_reg_read);

static void
gtp_regs2ascii(struct pt_regs *regs, char *buf)
{
#ifdef GTP_DEBUG_V
	{
		int	i;

		for (i = 0; i < 32; i++)
			printk(GTP_DEBUG_V "gtp_gdbrsp_g: r%d = 0x%lx\n", i,
			       regs->regs[i]);
	}
	printk(GTP_DEBUG_V "gtp_gdbrsp_g: status = 0x%lx\n",
	       regs->cp0_status);
	printk(GTP_DEBUG_V "gtp_gdbrsp_g: lo = 0x%lx\n", regs->lo);
	printk(GTP_DEBUG_V "gtp_gdbrsp_g: hi = 0x%lx\n", regs->hi);
	printk(GTP_DEBUG_V "gtp_gdbrsp_g: badvaddr = 0x%lx\n",
	       regs->cp0_badvaddr);
	printk(GTP_DEBUG_V "gtp_gdbrsp_g: cause = 0x%lx\n", regs->cp0_cause);
	printk(GTP_DEBUG_V "gtp_gdbrsp_g: pc = 0x%lx\n", regs->cp0_epc);
#endif

#ifdef CONFIG_32BIT
#define OUTFORMAT	"%08lx"
#define REGSIZE		8
#ifdef __LITTLE_ENDIAN
#define SWAB(a)		swab32(a)
#else
#define SWAB(a)		(a)
#endif
#else
#define OUTFORMAT	"%016lx"
#define REGSIZE		16
#ifdef __LITTLE_ENDIAN
#define SWAB(a)		swab64(a)
#else
#define SWAB(a)		(a)
#endif
#endif
	{
		int	i;

		for (i = 0; i < 32; i++) {
			sprintf(buf, OUTFORMAT,
				 (unsigned long) SWAB(regs->regs[i]));
			buf += REGSIZE;
		}
	}

	sprintf(buf, OUTFORMAT,
		 (unsigned long) SWAB(regs->cp0_status));
	buf += REGSIZE;
	sprintf(buf, OUTFORMAT,
		 (unsigned long) SWAB(regs->lo));
	buf += REGSIZE;
	sprintf(buf, OUTFORMAT,
		 (unsigned long) SWAB(regs->hi));
	buf += REGSIZE;
	sprintf(buf, OUTFORMAT,
		 (unsigned long) SWAB(regs->cp0_badvaddr));
	buf += REGSIZE;
	sprintf(buf, OUTFORMAT,
		 (unsigned long) SWAB(regs->cp0_cause));
	buf += REGSIZE;
	sprintf(buf, OUTFORMAT,
		 (unsigned long) SWAB(regs->cp0_epc));
	buf += REGSIZE;
#undef OUTFORMAT
#undef REGSIZE
#undef SWAB
}

static void
gtp_regs2bin(struct pt_regs *regs, char *buf)
{
#ifdef GTP_DEBUG_V
	{
		int	i;

		for (i = 0; i < 32; i++)
			printk(GTP_DEBUG_V "gtp_gdbrsp_g: r%d = 0x%lx\n", i,
			       regs->regs[i]);
	}
	printk(GTP_DEBUG_V "gtp_gdbrsp_g: status = 0x%lx\n",
	       regs->cp0_status);
	printk(GTP_DEBUG_V "gtp_gdbrsp_g: lo = 0x%lx\n", regs->lo);
	printk(GTP_DEBUG_V "gtp_gdbrsp_g: hi = 0x%lx\n", regs->hi);
	printk(GTP_DEBUG_V "gtp_gdbrsp_g: badvaddr = 0x%lx\n",
	       regs->cp0_badvaddr);
	printk(GTP_DEBUG_V "gtp_gdbrsp_g: cause = 0x%lx\n", regs->cp0_cause);
	printk(GTP_DEBUG_V "gtp_gdbrsp_g: pc = 0x%lx\n", regs->cp0_epc);
#endif

#ifdef CONFIG_32BIT
#define REGSIZE		4
#else
#define REGSIZE		8
#endif
	{
		int	i;

		for (i = 0; i < 32; i++) {
			memcpy(buf, &regs->regs[i], REGSIZE);
			buf += REGSIZE;
		}
	}
	memcpy(buf, &regs->cp0_status, REGSIZE);
	buf += REGSIZE;
	memcpy(buf, &regs->lo, REGSIZE);
	buf += REGSIZE;
	memcpy(buf, &regs->hi, REGSIZE);
	buf += REGSIZE;
	memcpy(buf, &regs->cp0_badvaddr, REGSIZE);
	buf += REGSIZE;
	memcpy(buf, &regs->cp0_cause, REGSIZE);
	buf += REGSIZE;
	memcpy(buf, &regs->cp0_epc, REGSIZE);
	buf += REGSIZE;
#undef REGSIZE
}
#endif

#ifdef CONFIG_ARM
ULONGEST
gtp_action_reg_read(struct gtp_trace_s *gts, int num)
{
	if (num >= 0 && num < 16)
		return gts->regs->uregs[num];
	else if (num == 25)
		return gts->regs->uregs[16];

	gts->tpe->reason = gtp_stop_access_wrong_reg;
	return 0;
}
EXPORT_SYMBOL(gtp_action_reg_read);

static void
gtp_regs2ascii(struct pt_regs *regs, char *buf)
{
#ifdef __LITTLE_ENDIAN
#define SWAB(a)		swab32(a)
#else
#define SWAB(a)		(a)
#endif
	int	i;

	for (i = 0; i < 16; i++) {
#ifdef GTP_DEBUG_V
		printk(GTP_DEBUG_V "gtp_gdbrsp_g: r%d = 0x%lx\n",
		       i, regs->uregs[i]);
#endif
		sprintf(buf, "%08lx", (unsigned long) SWAB(regs->uregs[i]));
		buf += 8;
	}

	/* f0-f7 fps */
	memset(buf, '0', 200);
	buf += 200;

#ifdef GTP_DEBUG_V
	printk(GTP_DEBUG_V "gtp_gdbrsp_g: cpsr = 0x%lx\n", regs->uregs[16]);
#endif
	sprintf(buf, "%08lx",
		 (unsigned long) SWAB(regs->uregs[16]));
	buf += 8;
#undef SWAB
}

static void
gtp_regs2bin(struct pt_regs *regs, char *buf)
{
	int	i;

	for (i = 0; i < 16; i++) {
#ifdef GTP_DEBUG_V
		printk(GTP_DEBUG_V "gtp_gdbrsp_g: r%d = 0x%lx\n",
		       i, regs->uregs[i]);
#endif
		memcpy(buf, &regs->uregs[i], 4);
		buf += 4;
	}

	/* f0-f7 fps */
	memset(buf, '\0', 100);
	buf += 100;

#ifdef GTP_DEBUG_V
	printk(GTP_DEBUG_V "gtp_gdbrsp_g: cpsr = 0x%lx\n", regs->uregs[16]);
#endif
	memcpy(buf, &regs->uregs[16], 4);
	buf += 4;
}
#endif

#ifdef GTP_PERF_EVENTS
#if KGTP_API_VERSION_LOCAL < 20120808
#include "perf_event.c"
#endif

static DEFINE_PER_CPU(int, pc_pe_list_all_disabled);
static DEFINE_PER_CPU(struct gtp_var_perf_event *, pc_pe_list);

static void
pc_pe_list_disable(void)
{
	struct gtp_var_perf_event *ppl;

	if (__get_cpu_var(pc_pe_list_all_disabled))
		return;

	for (ppl = __get_cpu_var(pc_pe_list); ppl; ppl = ppl->pc_next) {
		if (ppl->en)
#if (KGTP_API_VERSION_LOCAL < 20120808)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0))
			__gtp_perf_event_disable(ppl->event);
#else
			perf_event_disable(ppl->event);
#endif
#else
			local_perf_event_disable(ppl->event);
#endif
	}
}

static void
pc_pe_list_enable(void)
{
	struct gtp_var_perf_event *ppl;

	if (__get_cpu_var(pc_pe_list_all_disabled))
		return;

	for (ppl = __get_cpu_var(pc_pe_list); ppl; ppl = ppl->pc_next) {
		if (ppl->en)
#if (KGTP_API_VERSION_LOCAL < 20120808)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0))
			__gtp_perf_event_enable(ppl->event);
#else
			perf_event_enable(ppl->event);
#endif
#else
			local_perf_event_enable(ppl->event);
#endif
	}
}

static void
gtp_pc_pe_en(int enable)
{
	struct gtp_var_perf_event *ppl = __get_cpu_var(pc_pe_list);

	for (ppl = __get_cpu_var(pc_pe_list); ppl; ppl = ppl->pc_next)
		ppl->en = enable;

	__get_cpu_var(pc_pe_list_all_disabled) = !enable;
}

static void
gtp_pe_set_en(struct gtp_var_perf_event *pts, int enable)
{
	if (pts->event->cpu != smp_processor_id()) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0)	\
     && KGTP_API_VERSION_LOCAL < 20120808)
		if (enable)
			gtp_perf_event_enable(pts->event);
		else
			gtp_perf_event_disable(pts->event);
#else
		if (enable)
			perf_event_enable(pts->event);
		else
			perf_event_disable(pts->event);
#endif
	}
	pts->en = enable;
}
#else
static void
gtp_pc_pe_en(int enable)
{
}
#endif	/* GTP_PERF_EVENTS */

/* Following part is for gtp_task_read.  */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0))
static inline int is_cow_mapping(unsigned int flags)
#else
static inline int is_cow_mapping(vm_flags_t flags)
#endif
{
	return (flags & (VM_SHARED | VM_MAYWRITE)) == VM_MAYWRITE;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
#ifdef __HAVE_ARCH_PTE_SPECIAL
# define HAVE_PTE_SPECIAL 1
#else
# define HAVE_PTE_SPECIAL 0
#endif
#endif
struct page *
gtp_vm_normal_page(struct vm_area_struct *vma, unsigned long addr,
				pte_t pte)
{
	unsigned long pfn = pte_pfn(pte);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
	if (HAVE_PTE_SPECIAL) {
		if (likely(!pte_special(pte)))
			goto check_pfn;
		/* XXX: not support is_zero_pfn.  */

		return NULL;
	}
#endif

	/* !HAVE_PTE_SPECIAL case follows: */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
	if (unlikely(vma->vm_flags & (VM_PFNMAP|VM_MIXEDMAP))) {
#else
	if (unlikely(vma->vm_flags & (VM_PFNMAP))) {
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
		if (vma->vm_flags & VM_MIXEDMAP) {
			if (!pfn_valid(pfn))
				return NULL;
			goto out;
		} else {
#endif
			unsigned long off;
			off = (addr - vma->vm_start) >> PAGE_SHIFT;
			if (pfn == vma->vm_pgoff + off)
				return NULL;
			if (!is_cow_mapping(vma->vm_flags))
				return NULL;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
		}
#endif
	}

	/* XXX: is_zero_pfn is not support.
	if (is_zero_pfn(pfn))
		return NULL;
	*/
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
check_pfn:
#endif
	/* XXX: highest_memmap_pfn is not support.
	if (unlikely(pfn > highest_memmap_pfn)) {
		print_bad_pte(vma, addr, pte, NULL);
		return NULL;
	}
	*/

	/*
	 * NOTE! We still have PageReserved() pages in the page tables.
	 * eg. VDSO mappings can cause them to exist.
	 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
out:
#endif
	return pfn_to_page(pfn);
}

/* Example: follow_page */

struct page *
gtp_follow_page(struct vm_area_struct *vma, unsigned long address,
		unsigned int flags)
{
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *ptep, pte;
	spinlock_t *ptl;
	struct page *page;
	struct mm_struct *mm = vma->vm_mm;

	/* XXX: not support follow_huge_addr.  */

	page = NULL;
	pgd = pgd_offset(mm, address);
	if (pgd_none(*pgd) || unlikely(pgd_bad(*pgd)))
		goto no_page_table;

	pud = pud_offset(pgd, address);
	if (pud_none(*pud))
		goto no_page_table;

	/* XXX: not support pud_huge. */

	if (unlikely(pud_bad(*pud)))
		goto no_page_table;

	pmd = pmd_offset(pud, address);
	if (pmd_none(*pmd))
		goto no_page_table;

	/* XXX: not support pmd_huge. */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,38))
	if (unlikely(pmd_bad(*pmd)))
		goto no_page_table;
#else
	if (pmd_trans_huge(*pmd)) {
		/* XXX: not support wait_split_huge_page.  */
		goto no_page_table;
	}
#endif

	if (unlikely(pmd_bad(*pmd)))
		goto no_page_table;

	ptep = pte_offset_map_lock(mm, pmd, address, &ptl);

	pte = *ptep;
	if (!pte_present(pte))
		goto no_page;
	if ((flags & FOLL_WRITE) && !pte_write(pte))
		goto unlock;

	page = gtp_vm_normal_page(vma, address, pte);
	if (unlikely(!page)) {
		/* XXX: not support is_zero_pfn.  */
		goto bad_page;
	}

	if (flags & FOLL_GET) {
		/* XXX: not support get_page_foll(page) */
		get_page(page);
	}
unlock:
	pte_unmap_unlock(ptep, ptl);
	return page;

bad_page:
	pte_unmap_unlock(ptep, ptl);
	return ERR_PTR(-EFAULT);

no_page:
	pte_unmap_unlock(ptep, ptl);
	if (!pte_none(pte))
		return page;

no_page_table:
	return page;
}

/* Example: __get_user_pages */

static int
gtp_get_user_page(struct mm_struct *mm, unsigned long start,
		  struct page **pages, struct vm_area_struct **vmas)
{
	struct vm_area_struct	*vma;
	struct page		*page;

	/* XXX: not use find_extend_vma because cannot get
	   find_vma_prev and expand_stack.  */
	vma = find_vma(mm, start);
	if (vma == NULL || vma->vm_flags & VM_LOCKED)
		return 0;

	/* XXX: not use get_gate_vma because not support vm_normal_page. */
	if (!vma ||
		    (vma->vm_flags & (VM_IO | VM_PFNMAP)) ||
		    !(VM_MAYREAD & vma->vm_flags))
			return -EFAULT;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36))
	if (is_vm_hugetlb_page(vma)) {
		/* XXX: not support follow_hugetlb_page.  */
		return 0;
	}
#endif
	page = gtp_follow_page(vma, start, FOLL_GET);
	if (IS_ERR(page))
		return PTR_ERR(page);
	if (pages) {
		pages[0] = page;
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,19))
#if defined(CONFIG_ARM) && (KGTP_API_VERSION_LOCAL < 20120917)
		printk(KERN_WARNING "You use KGTP $current but your Kernel "
				    "doesn't support flush_anon_page.  "
				    "If you want use it, "
				    "please patch the patch of KGTP "
				    "(http://code.google.com/p/kgtp/wiki/HOWTO#Use_KGTP_patch_for_Linux_kernel).\n");
#else
		flush_anon_page(vma, page, start);
#endif
#else
		flush_anon_page(page, start);
#endif
		flush_dcache_page(page);
	}
	if (vmas)
		vmas[0] = vma;

	return 1;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,39))
static struct task_struct	gtp_fake_task;
#ifdef CONFIG_IA32_EMULATION
static struct thread_info	gtp_fake_thread;
#endif
#endif

/* Example: access_remote_vm */

static int
gtp_task_read(pid_t pid, struct task_struct *tsk, unsigned long addr,
	      void *buf, int len, int in_kprobe_handler)
{
	int			ret = -ESRCH;
	struct mm_struct	*mm;
	struct vm_area_struct	*vma;
	void			*old_buf = buf;

	if (in_kprobe_handler) {
		/* The tsk cannot be NULL.  */
		/* Example: get_task_mm */
		ret = -ENXIO;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
		if (tsk->flags & PF_KTHREAD)
#else
		if (tsk->flags & PF_BORROWED_MM)
#endif
			return ret;
		task_lock(tsk);
		mm = tsk->mm;
	} else {
		/* Example: ptrace_get_task_struct
		issue 131: For the Linux kernel 2.6.23 and older version,
		it should use read_lock(&tasklist_lock) and
		read_unlock(&tasklist_lock).
		But tasklist_lock is not exported.
		And find_task_by_pid use RCU.  So use rcu_read_lock and
		rcu_read_unlock to handle it.  */
		ret = -ESRCH;
		rcu_read_lock();
		if (!tsk) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))
			tsk = pid_task(find_vpid(pid), PIDTYPE_PID);
#else
			tsk = find_task_by_pid(pid);
#endif
			if (!tsk) {
				rcu_read_unlock();
				return ret;
			}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,39))
#ifdef CONFIG_IA32_EMULATION
			/* This part for get_gate_vma.  */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22))
#ifndef __HAVE_THREAD_FUNCTIONS
			gtp_fake_task.thread_info = &gtp_fake_thread;
#else
			task_stack_page(&gtp_fake_task) = &gtp_fake_thread;
#endif
#else
			task_stack_page(&gtp_fake_task) = &gtp_fake_thread;
#endif
			task_thread_info(&gtp_fake_task)->flags = task_thread_info(tsk)->flags;
#endif
#endif
		}
		mm = get_task_mm(tsk);
		rcu_read_unlock();
	}
	if (!mm)
		goto out;

	if (in_kprobe_handler) {
		if (!down_read_trylock(&mm->mmap_sem))
			goto out;
	} else
		down_read(&mm->mmap_sem);

	while (len) {
		int bytes, offset;
		void *maddr;
		struct page *page = NULL;

		if (in_kprobe_handler)
			ret = gtp_get_user_page(mm, addr, &page, &vma);
		else {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,39))
			ret = get_user_pages(&gtp_fake_task, mm, addr, 1, 0, 1, &page,
				     &vma);
#else
			ret = get_user_pages(NULL, mm, addr, 1, 0, 1, &page, &vma);
#endif
		}
		if (ret <= 0) {
			/*
			 * Check if this is a VM_IO | VM_PFNMAP VMA, which
			 * we can access using slightly different code.
			 */
#ifdef CONFIG_HAVE_IOREMAP_PROT
			vma = find_vma(mm, addr);
			if (!vma || vma->vm_start > addr)
				break;
			if (vma->vm_ops && vma->vm_ops->access)
				ret = vma->vm_ops->access(vma, addr, buf,
							  len, 0);
			if (ret <= 0)
#endif
				break;
			bytes = ret;
		} else {
			bytes = len;
			offset = addr & (PAGE_SIZE-1);
			if (bytes > PAGE_SIZE-offset)
				bytes = PAGE_SIZE-offset;

			if (in_kprobe_handler)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0))
				maddr = kmap_atomic(page);
#else
				maddr = kmap_atomic(page, KM_IRQ1);
#endif
			else
				maddr = kmap(page);
			copy_from_user_page(vma, page, addr,
					    buf, maddr + offset, bytes);
			if (in_kprobe_handler)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0))
				kunmap_atomic(maddr);
#else
				kunmap_atomic(maddr, KM_IRQ1);
#endif
			else
				kunmap(page);
			page_cache_release(page);
		}
		len -= bytes;
		buf += bytes;
		addr += bytes;
	}
	ret = buf - old_buf;

	up_read(&mm->mmap_sem);
	if (!in_kprobe_handler)
		mmput(mm);
out:
	if (in_kprobe_handler)
		task_unlock(tsk);
	return ret;
}

static long
gtp_task_handler_read(void *dst, void *src, size_t size)
{
	if (gtp_task_read(0, get_current(),
			  (unsigned long)src, dst, size, 1) != size)
		return -EFAULT;

	return 0;
}

#ifdef GTP_FRAME_SIMPLE
static char *
gtp_frame_next(char *frame)
{
	switch (FID(frame)) {
	case FID_HEAD:
		frame += FRAME_ALIGN(GTP_FRAME_HEAD_SIZE);
		break;
	case FID_REG:
		frame += FRAME_ALIGN(GTP_FRAME_REG_SIZE);
		break;
	case FID_MEM: {
			struct gtp_frame_mem	*gfm;

			gfm = (struct gtp_frame_mem *) (frame + FID_SIZE
							+ sizeof(char *));
			frame += FRAME_ALIGN(GTP_FRAME_MEM_SIZE + gfm->size);
		}
		break;
	case FID_VAR:
		frame += FRAME_ALIGN(GTP_FRAME_VAR_SIZE);
		break;
	case FID_END:
		frame = gtp_frame_end;
		break;
	default:
		return NULL;
		break;
	}

	return frame;
}
#endif

#ifdef GTP_FRAME_SIMPLE
#ifdef FRAME_ALLOC_RECORD
ULONGEST	frame_alloc_size;
ULONGEST	frame_alloc_size_hole;
#endif

static char *
gtp_frame_alloc(size_t size)
{
	char	*ret = NULL;

#ifdef FRAME_ALLOC_RECORD
	frame_alloc_size += size;
	frame_alloc_size_hole += (FRAME_ALIGN(size) - size);
#endif

	size = FRAME_ALIGN(size);

	if (size > GTP_FRAME_SIZE)
		return NULL;

	spin_lock(&gtp_frame_lock);

	if (gtp_frame_w_start + size > gtp_frame_end) {
		if (gtp_circular) {
			gtp_frame_is_circular = 1;
#ifdef FRAME_ALLOC_RECORD
			if (gtp_frame_w_start != gtp_frame_end
			    && gtp_frame_end - gtp_frame_w_start < FID_SIZE) {
				printk(KERN_WARNING "Frame align wrong."
						    "start = %p end = %p\n",
				       gtp_frame_w_start, gtp_frame_end);
				goto out;
			}
#endif
			if (gtp_frame_w_start != gtp_frame_end)
				FID(gtp_frame_w_start) = FID_END;
			gtp_frame_w_start = gtp_frame;
			gtp_frame_r_start = gtp_frame;
		} else
			goto out;
	}

	if (gtp_frame_is_circular) {
		while (gtp_frame_w_start <= gtp_frame_r_start
		       && gtp_frame_w_start + size > gtp_frame_r_start) {
			char *tmp = gtp_frame_next(gtp_frame_r_start);
			if (tmp == NULL)
				goto out;
			if (tmp == gtp_frame_end)
				gtp_frame_r_start = gtp_frame;
			else
				gtp_frame_r_start = tmp;
		}
	}

	ret = gtp_frame_w_start;
	gtp_frame_w_start += size;

out:
	spin_unlock(&gtp_frame_lock);
	return ret;
}
#endif

#define GTP_PRINTK_FORMAT_A	0
#define GTP_PRINTK_FORMAT_D	1
#define GTP_PRINTK_FORMAT_U	2
#define GTP_PRINTK_FORMAT_X	3
#define GTP_PRINTK_FORMAT_S	4
#define GTP_PRINTK_FORMAT_B	5

#ifdef GTP_FTRACE_RING_BUFFER
#define GTP_FRAME_RINGBUFFER_ALLOC(size)				\
	do {								\
		rbe = ring_buffer_lock_reserve(gtp_frame, size);	\
		if (rbe == NULL) {					\
			gts->tpe->reason = gtp_stop_frame_full;		\
			return -1;					\
		}							\
		tmp = ring_buffer_event_data(rbe);			\
	} while (0)
#endif

static int	gtp_collect_var(struct gtp_trace_s *gts, int num);
#ifdef GTP_RB
static int	gtp_var_array_step_id_id = 0;
#endif

static int
gtp_action_head(struct gtp_trace_s *gts)
{
	char				*tmp;
	ULONGEST			*trace_nump;
#ifdef GTP_FTRACE_RING_BUFFER
	struct ring_buffer_event	*rbe;
#endif

#ifdef GTP_RB
	gts->next = (struct gtp_rb_s *)this_cpu_ptr(gtp_rb);
#endif

	/* Get the head.  */
#ifdef GTP_FTRACE_RING_BUFFER
	GTP_FRAME_RINGBUFFER_ALLOC(GTP_FRAME_HEAD_SIZE);
#endif
#if defined(GTP_FRAME_SIMPLE) || defined(GTP_RB)
#ifdef GTP_RB
	GTP_RB_LOCK(gts->next);
	tmp = gtp_rb_alloc(gts->next, GTP_FRAME_HEAD_SIZE, 0);
#endif
#ifdef GTP_FRAME_SIMPLE
	tmp = gtp_frame_alloc(GTP_FRAME_HEAD_SIZE);
#endif
	if (!tmp) {
		gts->tpe->reason = gtp_stop_frame_full;
		return -1;
	}
#endif

	FID(tmp) = FID_HEAD;
	tmp += FID_SIZE;

#ifdef GTP_RB
	gts->id = gtp_rb_clock();
	*(u64 *)tmp = gts->id;
	tmp += sizeof(u64);
#endif

#ifdef GTP_FRAME_SIMPLE
	gts->next = (char **)tmp;
	*(gts->next) = NULL;
	tmp += sizeof(char *);
#endif

	trace_nump = (ULONGEST *)tmp;
	*trace_nump = gts->tpe->num;
	tmp += sizeof(ULONGEST);

#ifdef GTP_RB
	*(void **)tmp = gtp_rb_prev_frame_get(gts->next);
	gtp_rb_prev_frame_set(gts->next, (void *)(tmp + sizeof(void *)
						  - GTP_FRAME_HEAD_SIZE));
#endif

#ifdef GTP_FTRACE_RING_BUFFER
	ring_buffer_unlock_commit(gtp_frame, rbe);
	gts->next = (char *)1;
#endif

	atomic_inc(&gtp_frame_create);

#ifdef GTP_RB
	/* Auto collect $step_id.  */
	if (gts->tpe->step) {
		if (gtp_collect_var(gts, gtp_var_array_step_id_id))
			return -1;
	}
#endif

	return 0;
}

static int
gtp_action_printk(struct gtp_trace_s *gts, ULONGEST addr, size_t size)
{
	unsigned int	printk_format = gts->printk_format;
	char		*pbuf = __get_cpu_var(gtp_printf);

	if (gts->printk_str == NULL) {
		gts->tpe->reason = gtp_stop_agent_expr_code_error;
		printk(KERN_WARNING "gtp_action_printk: id:%d addr:%p "
				    "printk doesn't have var name.  Please "
				    "check actions of it.\n",
			(int)gts->tpe->num, (void *)(CORE_ADDR)gts->tpe->addr);
		return -1;
	}

	if (size) {
		if (size > GTP_PRINTF_MAX - 1)
			size = GTP_PRINTF_MAX - 1;
		if (gts->printk_format != GTP_PRINTK_FORMAT_S
		    && gts->printk_format != GTP_PRINTK_FORMAT_B
		    && size > 8)
			size = 8;
		if (gts->read_memory(pbuf, (void *)(CORE_ADDR)addr, size)) {
			gts->tpe->reason = gtp_stop_efault;
			printk(KERN_WARNING "gtp_action_printk: id:%d addr:%p "
					    "read %p %u get error.\n",
			       (int)gts->tpe->num,
			       (void *)(CORE_ADDR)gts->tpe->addr,
			       (void *)(CORE_ADDR)addr,
			       (unsigned int)size);
			return -1;
		}
	} else {
		size = sizeof(ULONGEST);
		memcpy(pbuf, &addr, sizeof(ULONGEST));
	}

	if (printk_format == GTP_PRINTK_FORMAT_A) {
		if (size == 1 || size == 2 || size == 4 || size == 8)
			printk_format = GTP_PRINTK_FORMAT_U;
		else
			printk_format = GTP_PRINTK_FORMAT_B;
	}

	switch (printk_format) {
	case GTP_PRINTK_FORMAT_D:
		switch (size) {
		case 1:
			printk(KERN_NULL "<%d>CPU%d %s%d\n",
			       gts->printk_level, smp_processor_id(),
			       gts->printk_str->src, pbuf[0]);
			break;
		case 2:
			printk(KERN_NULL "<%d>CPU%d %s%d\n",
			       gts->printk_level, smp_processor_id(),
			       gts->printk_str->src,
			       (int)(*(short *)pbuf));
			break;
		case 4:
			printk(KERN_NULL "<%d>CPU%d %s%d\n",
			       gts->printk_level, smp_processor_id(),
			       gts->printk_str->src, *(int *)pbuf);
			break;
		case 8:
			printk(KERN_NULL "<%d>CPU%d %s%lld\n",
			       gts->printk_level, smp_processor_id(),
			       gts->printk_str->src, *(long long *)pbuf);
			break;
		default:
			printk(KERN_WARNING "gtp_action_printk: id:%d addr:%p "
					    "size %d cannot printk.\n",
			       (int)gts->tpe->num,
			       (void *)(CORE_ADDR)gts->tpe->addr,
			       (unsigned int)size);
			gts->tpe->reason = gtp_stop_agent_expr_code_error;
			return -1;
			break;
		}
		break;
	case GTP_PRINTK_FORMAT_U:
		switch (size) {
		case 1:
			printk(KERN_NULL "<%d>CPU%d %s%u\n",
			       gts->printk_level, smp_processor_id(),
			       gts->printk_str->src, pbuf[0]);
			break;
		case 2:
			printk(KERN_NULL "<%d>CPU%d %s%u\n",
			       gts->printk_level, smp_processor_id(),
			       gts->printk_str->src, (int)(*(short *)pbuf));
			break;
		case 4:
			printk(KERN_NULL "<%d>CPU%d %s%u\n",
			       gts->printk_level, smp_processor_id(),
			       gts->printk_str->src, *(int *)pbuf);
			break;
		case 8:
			printk(KERN_NULL "<%d>CPU%d %s%llu\n",
			       gts->printk_level, smp_processor_id(),
			       gts->printk_str->src, *(long long *)pbuf);
			break;
		default:
			printk(KERN_WARNING "gtp_action_printk: id:%d addr:%p"
					    "size %d cannot printk.\n",
			       (int)gts->tpe->num,
			       (void *)(CORE_ADDR)gts->tpe->addr,
			       (unsigned int)size);
			gts->tpe->reason = gtp_stop_agent_expr_code_error;
			return -1;
			break;
		}
		break;
	case GTP_PRINTK_FORMAT_X:
		switch (size) {
		case 1:
			printk(KERN_NULL "<%d>CPU%d %s0x%x\n",
			       gts->printk_level, smp_processor_id(),
			       gts->printk_str->src, pbuf[0]);
			break;
		case 2:
			printk(KERN_NULL "<%d>CPU%d %s0x%x\n",
			       gts->printk_level, smp_processor_id(),
			       gts->printk_str->src, (int)(*(short *)pbuf));
			break;
		case 4:
			printk(KERN_NULL "<%d>CPU%d %s0x%x\n",
			       gts->printk_level, smp_processor_id(),
			       gts->printk_str->src, *(int *)pbuf);
			break;
		case 8:
			printk(KERN_NULL "<%d>CPU%d %s0x%llx\n",
			       gts->printk_level, smp_processor_id(),
			       gts->printk_str->src, *(long long *)pbuf);
			break;
		default:
			printk(KERN_WARNING "gtp_action_printk: id:%d addr:%p "
					    "size %d cannot printk.\n",
			       (int)gts->tpe->num,
			       (void *)(CORE_ADDR)gts->tpe->addr,
			       (unsigned int)size);
			gts->tpe->reason = gtp_stop_agent_expr_code_error;
			return -1;
			break;
		}
		break;
	case GTP_PRINTK_FORMAT_S:
		pbuf[GTP_PRINTF_MAX - 1] = '\0';
		printk("<%d>CPU%d %s%s\n", gts->printk_level,
		       smp_processor_id(), gts->printk_str->src, pbuf);
		break;
	case GTP_PRINTK_FORMAT_B: {
			size_t	i;

			printk(KERN_NULL "<%d>CPU%d %s", gts->printk_level,
			       smp_processor_id(), gts->printk_str->src);
			for (i = 0; i < size; i++)
				printk("%02x", (unsigned int)pbuf[i]);
			printk("\n");
		}
		break;
	default:
		printk(KERN_WARNING "gtp_action_printk: id:%d addr:%p "
				    "printk format %u is not support.\n",
		       (int)gts->tpe->num, (void *)(CORE_ADDR)gts->tpe->addr,
		       gts->printk_format);
		gts->tpe->reason = gtp_stop_agent_expr_code_error;
		return -1;
		break;
	}

	gts->printk_str = gts->printk_str->next;

	return 0;
}

static int
gtp_action_memory_read(struct gtp_trace_s *gts, int reg, CORE_ADDR addr,
		       size_t size)
{
	char				*tmp;
	struct gtp_frame_mem		*fm;
#ifdef GTP_FTRACE_RING_BUFFER
	struct ring_buffer_event	*rbe;
#endif
#ifdef GTP_RB
	size_t 				left;
#endif

	if (reg >= 0)
		addr += (CORE_ADDR) gtp_action_reg_read(gts, reg);
	if (gts->tpe->reason != gtp_stop_normal)
		return -1;

	if (gts->next == NULL) {
		if (gtp_action_head(gts))
			return -1;
	}
#ifdef GTP_RB
	left = size;
	while (left) {
		size_t 	cur_size;

		/* Get cur_size.  */
		if (gtp_rb_alloc_max(gts->next) < GTP_FRAME_MEM_SIZE) {
			/* Current rb page is not big enough, return
			   size of whole page because we will use next
			   page.  */
			cur_size = GTP_RB_DATA_MAX - GTP_FRAME_MEM_SIZE;
		} else
			cur_size = gtp_rb_alloc_max(gts->next) - GTP_FRAME_MEM_SIZE;
		cur_size = min(cur_size, left);

		/* Alloc buffer from RB.  */
		tmp = gtp_rb_alloc(gts->next, GTP_FRAME_MEM_SIZE + cur_size, gts->id);
		if (!tmp) {
			gts->tpe->reason = gtp_stop_frame_full;
			return -1;
		}

		/* Write data to buffer.  */
		FID(tmp) = FID_MEM;
		tmp += FID_SIZE;
		fm = (struct gtp_frame_mem *)tmp;
		fm->addr = addr + size - left;
		fm->size = cur_size;
		tmp += sizeof(struct gtp_frame_mem);
#ifdef GTP_DEBUG_V
		printk(GTP_DEBUG_V "gtp_action_memory_read: id:%d addr:%p %p %u\n",
		       (int)gts->tpe->num, (void *)(CORE_ADDR)gts->tpe->addr,
		       (void *)fm->addr, (unsigned int)cur_size);
#endif

		if (gts->read_memory(tmp, (void *)fm->addr, cur_size)) {
			gts->tpe->reason = gtp_stop_efault;
			GTP_RB_RELEASE(gts->next);
			printk(KERN_WARNING "gtp_action_memory_read: id:%d addr:%p "
					    "read %p %u get error.\n",
			       (int)gts->tpe->num, (void *)(CORE_ADDR)gts->tpe->addr,
			       (void *)fm->addr, (unsigned int)cur_size);
			return -1;
		}

		left -= cur_size;
	}
#else
#ifdef GTP_FTRACE_RING_BUFFER
	GTP_FRAME_RINGBUFFER_ALLOC(GTP_FRAME_MEM_SIZE + size);
#endif
#if defined(GTP_FRAME_SIMPLE)
	tmp = gtp_frame_alloc(GTP_FRAME_MEM_SIZE + size);
	if (!tmp) {
		gts->tpe->reason = gtp_stop_frame_full;
		return -1;
	}
	*gts->next = tmp;
#endif

	FID(tmp) = FID_MEM;
	tmp += FID_SIZE;

#ifdef GTP_FRAME_SIMPLE
	gts->next = (char **)tmp;
	*gts->next = NULL;
	tmp += sizeof(char *);
#endif

	fm = (struct gtp_frame_mem *)tmp;
	fm->addr = addr;
	fm->size = size;
	tmp += sizeof(struct gtp_frame_mem);

#ifdef GTP_DEBUG_V
	printk(GTP_DEBUG_V "gtp_action_memory_read: id:%d addr:%p %p %u\n",
	       (int)gts->tpe->num, (void *)(CORE_ADDR)gts->tpe->addr,
	       (void *)addr, (unsigned int)size);
#endif

	if (gts->read_memory(tmp, (void *)addr, size)) {
		gts->tpe->reason = gtp_stop_efault;
#ifdef GTP_FRAME_SIMPLE
		memset(tmp, 0, size);
#endif
#ifdef GTP_FTRACE_RING_BUFFER
		ring_buffer_discard_commit(gtp_frame, rbe);
#endif
		printk(KERN_WARNING "gtp_action_memory_read: id:%d addr:%p "
				    "read %p %u get error.\n",
		       (int)gts->tpe->num, (void *)(CORE_ADDR)gts->tpe->addr,
		       (void *)addr, (unsigned int)size);
		return -1;
	}

#ifdef GTP_FTRACE_RING_BUFFER
	ring_buffer_unlock_commit(gtp_frame, rbe);
#endif
#endif

	return 0;
}

static int
gtp_action_r(struct gtp_trace_s *gts, struct action *ae)
{
	struct pt_regs			*regs;
	char				*tmp;
#ifdef GTP_FTRACE_RING_BUFFER
	struct ring_buffer_event	*rbe;
#endif

	if (gts->next == NULL) {
		if (gtp_action_head(gts))
			return -1;
	}

#ifdef GTP_FTRACE_RING_BUFFER
	GTP_FRAME_RINGBUFFER_ALLOC(GTP_FRAME_REG_SIZE);
#endif
#if defined(GTP_FRAME_SIMPLE) || defined(GTP_RB)
#ifdef GTP_RB
	tmp = gtp_rb_alloc(gts->next, GTP_FRAME_REG_SIZE, gts->id);
#endif
#ifdef GTP_FRAME_SIMPLE
	tmp = gtp_frame_alloc(GTP_FRAME_REG_SIZE);
#endif
	if (!tmp) {
		gts->tpe->reason = gtp_stop_frame_full;
		return -1;
	}
#ifdef GTP_FRAME_SIMPLE
	*gts->next = tmp;
#endif
#endif

	FID(tmp) = FID_REG;
	tmp += FID_SIZE;

#ifdef GTP_FRAME_SIMPLE
	gts->next = (char **)tmp;
	*gts->next = NULL;
	tmp += sizeof(char *);
#endif

	regs = (struct pt_regs *)tmp;

	memcpy(regs, gts->regs, sizeof(struct pt_regs));
#ifdef CONFIG_X86_32
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,25))
	regs->sp = gts->x86_32_sp;
#else
	regs->esp = gts->x86_32_sp;
#endif
#endif	/* CONFIG_X86_32 */

	if (gts->ri)
		GTP_REGS_PC(regs) = (CORE_ADDR)gts->ri->ret_addr;
#ifdef CONFIG_X86
	else if (GTP_X86_NEED_ADJUST_PC(gts))
		GTP_REGS_PC(regs) -= 1;
#endif	/* CONFIG_X86 */

#ifdef GTP_FTRACE_RING_BUFFER
	ring_buffer_unlock_commit(gtp_frame, rbe);
#endif

	return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
static DEFINE_SPINLOCK(gtp_handler_enable_disable_loc);

static void
gtp_handler_enable_disable(struct gtp_trace_s *gts, ULONGEST val, int enable)
{
	struct gtp_entry	*tpe;

	spin_lock(&gtp_handler_enable_disable_loc);
	for (tpe = gtp_list; tpe; tpe = tpe->next) {
		if (val == 0 || tpe->num == val || gts->tpe != tpe)  {
			/* Following code can insert this task
				into tasklet without wake up softirqd.  */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,13,0))
			preempt_count_add(HARDIRQ_OFFSET);
#else
			add_preempt_count(HARDIRQ_OFFSET);
#endif
			if (enable && tpe->disable)
				tasklet_schedule(&gts->tpe->enable_tasklet);
			else if (!enable && !tpe->disable)
				tasklet_schedule(&gts->tpe->disable_tasklet);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,13,0))
			preempt_count_sub(HARDIRQ_OFFSET);
#else
			sub_preempt_count(HARDIRQ_OFFSET);
#endif
			tpe->disable = !tpe->disable;
		}
	}
	spin_unlock(&gtp_handler_enable_disable_loc);
}
#endif

static int
gtp_var_agent_get_val(struct gtp_trace_s *gts, int num, int64_t *val)
{
	int		ret = 0;
	struct gtp_var	*var;

	var = gtp_var_array[num];
	switch (var->type) {
	case gtp_var_special:
		ret = var->u.hooks->agent_get_val(gts, var, val);
		break;
	case gtp_var_per_cpu:
		*val = gtp_var_get_pc(var)->u.val;
		break;
	case gtp_var_normal:
		*val = var->u.val;
		break;
#ifdef GTP_PERF_EVENTS
	case gtp_var_perf_event_per_cpu:
	case gtp_var_perf_event: {
			struct gtp_var_pe	*pe = gtp_var_get_pe(var);
			pe->pe->val = perf_event_read_value(pe->pe->event,
							    &(pe->pe->enabled),
							    &(pe->pe->running));
			switch (pe->ptid) {
			case pe_tv_val:
				*val = (int64_t)(pe->pe->val);
				break;
			case pe_tv_enabled:
				*val = (int64_t)(pe->pe->enabled);
				break;
			case pe_tv_running:
				*val = (int64_t)(pe->pe->running);
				break;
			default:
				/* This just to handle warning of gcc.  */
				break;
			}
		}
		break;
#endif
	}

	return ret;
}

static int
gtp_var_agent_set_val(struct gtp_trace_s *gts, int num, int64_t val)
{
	int		ret = 0;
	struct gtp_var	*var;

	var = gtp_var_array[num];
	switch (var->type) {
	case gtp_var_special:
		ret = var->u.hooks->agent_set_val(gts, var, val);
		break;
	case gtp_var_per_cpu:
		gtp_var_get_pc(var)->u.val = val;
		break;
	case gtp_var_normal:
		var->u.val = val;
		break;
#ifdef GTP_PERF_EVENTS
	case gtp_var_perf_event_per_cpu:
	case gtp_var_perf_event: {
		struct gtp_var_pe	*pe = gtp_var_get_pe(var);
		pe->pe->val = perf_event_read_value(pe->pe->event,
						    &(pe->pe->enabled),
						    &(pe->pe->running));
		if (pe->ptid == pe_tv_en)
			gtp_pe_set_en(pe->pe, (int)val);
		else {
			/* For pe_tv_val.  */
#if (KGTP_API_VERSION_LOCAL < 20120808)
			gtp_perf_event_set(pe->pe->event, (u64)val);
#else
			perf_event_set(pe->pe->event, (u64)val);
#endif
		}
	}
		break;
#endif
	}

	return ret;
}

/* The number is not the ID of tvar, it is the ID of gtp_var_array. */

static int
gtp_collect_var(struct gtp_trace_s *gts, int num)
{
	struct gtp_frame_var		*fvar;
	char				*tmp;
#ifdef GTP_FTRACE_RING_BUFFER
	struct ring_buffer_event	*rbe;
#endif

	if (gts->next == NULL) {
		if (gtp_action_head(gts))
			return -1;
	}

#ifdef GTP_FTRACE_RING_BUFFER
	GTP_FRAME_RINGBUFFER_ALLOC(GTP_FRAME_VAR_SIZE);
#endif
#if defined(GTP_FRAME_SIMPLE) || defined(GTP_RB)
#ifdef GTP_RB
	tmp = gtp_rb_alloc(gts->next, GTP_FRAME_VAR_SIZE, gts->id);
#endif
#ifdef GTP_FRAME_SIMPLE
	tmp = gtp_frame_alloc(GTP_FRAME_VAR_SIZE);
#endif
	if (!tmp) {
		gts->tpe->reason = gtp_stop_frame_full;
		return -1;
	}
#ifdef GTP_FRAME_SIMPLE
	*gts->next = tmp;
#endif
#endif

	FID(tmp) = FID_VAR;
	tmp += FID_SIZE;

#ifdef GTP_FRAME_SIMPLE
	gts->next = (char **)tmp;
	*gts->next = NULL;
	tmp += sizeof(char *);
#endif

	fvar = (struct gtp_frame_var *) tmp;
	fvar->num = gtp_var_array[num]->num;
	if (gtp_var_agent_get_val(gts, num, &(fvar->val))) {
		fvar->val = 0;
		gts->tpe->reason = gtp_stop_agent_expr_code_error;
		return -1;
	}

#ifdef GTP_FTRACE_RING_BUFFER
	ring_buffer_unlock_commit(gtp_frame, rbe);
#endif

	return 0;
}

#define STACK_MAX	32
static DEFINE_PER_CPU(ULONGEST[STACK_MAX], action_x_stack);

static int
gtp_action_x(struct gtp_trace_s *gts, struct action *ae)
{
	int		ret = 0;
	unsigned int	pc = 0, sp = 0;
	ULONGEST	top = 0;
	int		arg;
	union {
		union {
			uint8_t	bytes[1];
			uint8_t	val;
		} u8;
		union {
			uint8_t	bytes[2];
			uint16_t val;
		} u16;
		union {
			uint8_t bytes[4];
			uint32_t val;
		} u32;
		union {
			uint8_t bytes[8];
			ULONGEST val;
		} u64;
	} cnv;
	uint8_t		*ebuf = ae->u.exp.buf;
	int		psize = GTP_PRINTF_MAX;
	char		*pbuf = __get_cpu_var(gtp_printf);
	ULONGEST	*stack = __get_cpu_var(action_x_stack);

	if (unlikely(ae->u.exp.need_var_lock))
		spin_lock(&gtp_var_lock);

	while (1) {
#ifdef GTP_DEBUG_V
		printk(GTP_DEBUG_V "gtp_parse_x: cmd %x\n", ebuf[pc]);
#endif

		switch (ebuf[pc++]) {
		/* add */
		case 0x02:
			top += stack[--sp];
			break;

		case op_check_add:
			if (sp)
				top += stack[--sp];
			else
				goto code_error_out;
			break;

		/* sub */
		case 0x03:
			top = stack[--sp] - top;
			break;

		case op_check_sub:
			if (sp)
				top = stack[--sp] - top;
			else
				goto code_error_out;
			break;

		/* mul */
		case 0x04:
			top *= stack[--sp];
			break;

		case op_check_mul:
			if (sp)
				top *= stack[--sp];
			else
				goto code_error_out;
			break;

#ifndef CONFIG_MIPS
		/* div_signed */
		case 0x05:
			if (top) {
				LONGEST l = (LONGEST) stack[--sp];
				do_div(l, (LONGEST) top);
				top = l;
			} else
				goto code_error_out;
			break;

		case op_check_div_signed:
			if (top && sp) {
				LONGEST l = (LONGEST) stack[--sp];
				do_div(l, (LONGEST) top);
				top = l;
			} else
				goto code_error_out;
			break;

		/* div_unsigned */
		case 0x06:
			if (top) {
				ULONGEST ul = stack[--sp];
				do_div(ul, top);
				top = ul;
			} else
				goto code_error_out;
			break;

		case op_check_div_unsigned:
			if (top && sp) {
				ULONGEST ul = stack[--sp];
				do_div(ul, top);
				top = ul;
			} else
				goto code_error_out;
			break;

		/* rem_signed */
		case 0x07:
			if (top) {
				LONGEST l1 = (LONGEST) stack[--sp];
				LONGEST l2 = (LONGEST) top;
				top = do_div(l1, l2);
			} else
				goto code_error_out;
			break;

		case op_check_rem_signed:
			if (top && sp) {
				LONGEST l1 = (LONGEST) stack[--sp];
				LONGEST l2 = (LONGEST) top;
				top = do_div(l1, l2);
			} else
				goto code_error_out;
			break;

		/* rem_unsigned */
		case 0x08:
			if (top) {
				ULONGEST ul1 = stack[--sp];
				ULONGEST ul2 = top;
				top = do_div(ul1, ul2);
			} else
				goto code_error_out;
			break;

		case op_check_rem_unsigned:
			if (top && sp) {
				ULONGEST ul1 = stack[--sp];
				ULONGEST ul2 = top;
				top = do_div(ul1, ul2);
			} else
				goto code_error_out;
			break;
#endif

		/* lsh */
		case 0x09:
			top = stack[--sp] << top;
			break;

		case op_check_lsh:
			if (sp)
				top = stack[--sp] << top;
			else
				goto code_error_out;
			break;

		/* rsh_signed */
		case 0x0a:
			top = ((LONGEST) stack[--sp]) >> top;
			break;

		case op_check_rsh_signed:
			if (sp)
				top = ((LONGEST) stack[--sp]) >> top;
			else
				goto code_error_out;
			break;

		/* rsh_unsigned */
		case 0x0b:
			top = stack[--sp] >> top;
			break;

		case op_check_rsh_unsigned:
			if (sp)
				top = stack[--sp] >> top;
			else
				goto code_error_out;
			break;

		/* trace */
		case 0x0c:
			--sp;
			if (!(gts->tpe->flags & GTP_ENTRY_FLAGS_HAVE_PRINTK)) {
				if (gtp_action_memory_read
					(gts, -1,
						(CORE_ADDR) stack[sp],
						(size_t) top))
					goto out;
			}
			top = stack[--sp];
			break;

		case op_check_trace:
			if (sp > 1) {
				if (gtp_action_memory_read
					(gts, -1, (CORE_ADDR) stack[--sp],
					(size_t) top)) {
					/* gtp_action_memory_read will
						set error status with itself
						if it got error. */
					goto out;
				}
				top = stack[--sp];
			} else
				goto code_error_out;
			break;

		/* trace_printk */
		case op_trace_printk:
			if (gtp_action_printk(gts,
						(ULONGEST)stack[--sp],
						(size_t) top))
				goto out;
			top = stack[--sp];
			break;

		/* trace_quick */
		case 0x0d:
			if (!(gts->tpe->flags & GTP_ENTRY_FLAGS_HAVE_PRINTK)) {
				if (gtp_action_memory_read
					(gts, -1, (CORE_ADDR) top,
						(size_t) ebuf[pc]))
					goto out;
			}
			pc++;
			break;

		/* trace_quick_printk */
		case op_trace_quick_printk:
			if (gtp_action_printk(gts, (ULONGEST) top,
						(size_t) ebuf[pc++]))
				goto out;
			break;

		/* log_not */
		case 0x0e:
			top = !top;
			break;

		/* bit_and */
		case 0x0f:
			top &= stack[--sp];
			break;

		case op_check_bit_and:
			if (sp)
				top &= stack[--sp];
			else
				goto code_error_out;
			break;

		/* bit_or */
		case 0x10:
			top |= stack[--sp];
			break;

		case op_check_bit_or:
			if (sp)
				top |= stack[--sp];
			else
				goto code_error_out;
			break;

		/* bit_xor */
		case 0x11:
			top ^= stack[--sp];
			break;

		case op_check_bit_xor:
			if (sp)
				top ^= stack[--sp];
			else
				goto code_error_out;
			break;

		/* bit_not */
		case 0x12:
			top = ~top;
			break;

		/* equal */
		case 0x13:
			top = (stack[--sp] == top);
			break;

		case op_check_equal:
			if (sp)
				top = (stack[--sp] == top);
			else
				goto code_error_out;
			break;

		/* less_signed */
		case 0x14:
			top = (((LONGEST) stack[--sp])
				< ((LONGEST) top));
			break;

		case op_check_less_signed:
			if (sp)
				top = (((LONGEST) stack[--sp])
					< ((LONGEST) top));
			else
				goto code_error_out;
			break;

		/* less_unsigned */
		case 0x15:
			top = (stack[--sp] < top);
			break;

		case op_check_less_unsigned:
			if (sp)
				top = (stack[--sp] < top);
			else
				goto code_error_out;
			break;

		/* ext */
		case 0x16:
			arg = ebuf[pc++];
			if (arg < (sizeof(LONGEST)*8)) {
				LONGEST mask = 1 << (arg - 1);
				top &= ((LONGEST) 1 << arg) - 1;
				top = (top ^ mask) - mask;
			}
			break;

		/* ref8 */
		case 0x17:
			if (gts->read_memory
				(cnv.u8.bytes,
				(void *)(CORE_ADDR)top, 1))
				goto code_error_out;
			top = (ULONGEST) cnv.u8.val;
			break;

		/* ref16 */
		case 0x18:
			if (gts->read_memory
				(cnv.u16.bytes,
				(void *)(CORE_ADDR)top, 2))
				goto code_error_out;
			top = (ULONGEST) cnv.u16.val;
			break;

		/* ref32 */
		case 0x19:
			if (gts->read_memory
				(cnv.u32.bytes,
				(void *)(CORE_ADDR)top, 4))
				goto code_error_out;
			top = (ULONGEST) cnv.u32.val;
			break;

		/* ref64 */
		case 0x1a:
			if (gts->read_memory
				(cnv.u64.bytes,
				(void *)(CORE_ADDR)top, 8))
				goto code_error_out;
			top = (ULONGEST) cnv.u64.val;
			break;

		/* if_goto */
		case 0x20:
			if (top)
				pc = (ebuf[pc] << 8)
					+ (ebuf[pc + 1]);
			else
				pc += 2;
			/* pop */
			top = stack[--sp];
			break;

		case op_check_if_goto:
			if (top)
				pc = (ebuf[pc] << 8)
					+ (ebuf[pc + 1]);
			else
				pc += 2;
			/* pop */
			if (sp)
				top = stack[--sp];
			else
				goto code_error_out;
			break;

		/* goto */
		case 0x21:
			pc = (ebuf[pc] << 8) + (ebuf[pc + 1]);
			break;

		/* const8 */
		case 0x22:
			stack[sp++] = top;
			top = ebuf[pc++];
			break;

		/* const16 */
		case 0x23:
			stack[sp++] = top;
			top = ebuf[pc++];
			top = (top << 8) + ebuf[pc++];
			break;

		/* const32 */
		case 0x24:
			stack[sp++] = top;
			top = ebuf[pc++];
			top = (top << 8) + ebuf[pc++];
			top = (top << 8) + ebuf[pc++];
			top = (top << 8) + ebuf[pc++];
			break;

		/* const64 */
		case 0x25:
			stack[sp++] = top;
			top = ebuf[pc++];
			top = (top << 8) + ebuf[pc++];
			top = (top << 8) + ebuf[pc++];
			top = (top << 8) + ebuf[pc++];
			top = (top << 8) + ebuf[pc++];
			top = (top << 8) + ebuf[pc++];
			top = (top << 8) + ebuf[pc++];
			top = (top << 8) + ebuf[pc++];
			break;

		/* reg */
		case 0x26:
			stack[sp++] = top;
			arg = ebuf[pc++];
			arg = (arg << 8) + ebuf[pc++];
			top = gtp_action_reg_read(gts, arg);
			if (gts->tpe->reason != gtp_stop_normal)
				goto error_out;
			break;

		/* end */
		case 0x27:
			if (gts->run)
				*(gts->run) = (int)top;
			goto out;
			break;

		/* dup */
		case 0x28:
			stack[sp++] = top;
			break;

		/* pop */
		case 0x29:
			top = stack[--sp];
			break;

		case op_check_pop:
			if (sp)
				top = stack[--sp];
			else
				goto code_error_out;
			break;

		/* zero_ext */
		case 0x2a:
			arg = ebuf[pc++];
			if (arg < (sizeof(LONGEST)*8))
				top &= ((LONGEST) 1 << arg) - 1;
			break;

		/* swap */
		case 0x2b:
			stack[sp] = top;
			top = stack[sp - 1];
			stack[sp - 1] = stack[sp];
			break;

		case op_check_swap:
			if (sp) {
				stack[sp] = top;
				top = stack[sp - 1];
				stack[sp - 1] = stack[sp];
			} else
				goto code_error_out;
			break;

		/* getv */
		case 0x2c: {
				int64_t	val;

				arg = ebuf[pc++];
				arg = (arg << 8) + ebuf[pc++];

				stack[sp++] = top;

				if (gtp_var_agent_get_val(gts, arg, &val))
					goto code_error_out;

				top = (ULONGEST)val;
			}
			break;

		/* setv */
		case 0x2d:
			arg = ebuf[pc++];
			arg = (arg << 8) + ebuf[pc++];
			if (gtp_var_agent_set_val(gts, arg, (int64_t)top))
				goto code_error_out;
			break;

		/* tracev */
		case 0x2e:
			arg = ebuf[pc++];
			arg = (arg << 8) + ebuf[pc++];

			if (gtp_collect_var(gts, arg)) {
				/* gtp_collect_var will set error
				   status with itself if it got error.  */
				goto error_out;
			}
			break;

		/* tracev_printk */
		case op_tracev_printk: {
				uint64_t	u64;
				arg = ebuf[pc++];
				arg = (arg << 8) + ebuf[pc++];

				if (gtp_var_agent_get_val(gts, arg, &u64))
					goto code_error_out;
				if (gtp_action_printk(gts, u64, 0)) {
					/* gtp_action_printk will set error status
					   with itself if it got error. */
					goto error_out;
				}
			}
			break;
		}

		if (ae->type != 'X' && unlikely(sp > STACK_MAX - 5)) {
			printk(KERN_WARNING "gtp_action_x: stack overflow.\n");
			gts->tpe->reason
				= gtp_stop_agent_expr_stack_overflow;
			goto error_out;
		}
	}
code_error_out:
	gts->tpe->reason = gtp_stop_agent_expr_code_error;
error_out:
	ret = -1;
	printk(KERN_WARNING "gtp_action_x: tracepoint %d addr:%p"
			    "action X get error in pc %u.\n",
		(int)gts->tpe->num, (void *)(CORE_ADDR)gts->tpe->addr, pc);
out:
	if (unlikely(psize != GTP_PRINTF_MAX)) {
		unsigned long	flags;

		local_irq_save(flags);
		printk("%s", pbuf - (GTP_PRINTF_MAX - psize));
		local_irq_restore(flags);
	}
	if (unlikely(ae->u.exp.need_var_lock))
		spin_unlock(&gtp_var_lock);
	return ret;
}

#if defined(GTP_FTRACE_RING_BUFFER) || defined(GTP_RB)
/* Wake up gtpframe pipe.  */
static void
gtp_handler_wakeup(void)
{
#ifdef GTP_FTRACE_RING_BUFFER
	FID_TYPE	eid = FID_END;
	ring_buffer_write(gtp_frame, FID_SIZE, &eid);
#endif

	if (atomic_read(&gtpframe_pipe_wq_v) > 0) {
		atomic_dec(&gtpframe_pipe_wq_v);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,13,0))
		preempt_count_add(HARDIRQ_OFFSET);
#else
		add_preempt_count(HARDIRQ_OFFSET);
#endif
		tasklet_schedule(&gtpframe_pipe_wq_tasklet);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,13,0))
		preempt_count_sub(HARDIRQ_OFFSET);
#else
		sub_preempt_count(HARDIRQ_OFFSET);
#endif
	}
}
#endif

#ifdef CONFIG_X86
/* while-stepping stop.  */

static void
gtp_step_stop(struct pt_regs *regs)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,25))
	regs->flags &= ~(X86_EFLAGS_TF);
#else
	regs->eflags &= ~(X86_EFLAGS_TF);
#endif
	if (__get_cpu_var(gtp_step).irq_need_open) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,25))
		regs->flags |= X86_EFLAGS_IF;
#else
		regs->eflags |= X86_EFLAGS_IF;
#endif
	}
	__get_cpu_var(gtp_step).step = 0;
	__get_cpu_var(gtp_step).tpe = NULL;
	__get_cpu_var(gtp_step).irq_need_open = 0;
}
#endif

static void
gtp_handler(struct gtp_trace_s *gts)
{
	struct list_head	*head, *cur;
	struct action		*ae;

#ifdef GTP_DEBUG_V
	printk(GTP_DEBUG_V "gtp_handler: tracepoint %d %p\n",
	       (int)gts->tpe->num, (void *)(CORE_ADDR)gts->tpe->addr);
#endif
#ifdef CONFIG_X86
	if (gts->step == 0 && __get_cpu_var(gtp_step).step)
		gtp_step_stop(gts->regs);
#endif

	/* For the gtp_entry_uprobe, gtp_up_handler will setup gts to OK
	   right status. */
	if (gts->tpe->type != gtp_entry_uprobe) {
		gts->read_memory = (void *)probe_kernel_read;
		if (gts->tpe->flags & GTP_ENTRY_FLAGS_CURRENT_TASK) {
			/* Get regs.  */
			if (gts->tpe->get_regs) {
				if (gtp_action_x(gts, gts->tpe->get_regs)
				|| gts->tmp_regs == NULL)
					goto tpe_stop;
				gts->regs = gts->tmp_regs;
			} else
				gts->regs = task_pt_regs(get_current());

			if (user_mode(gts->regs)) {
				gts->read_memory = gtp_task_handler_read;
#ifdef CONFIG_X86_32
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,25))
				gts->x86_32_sp = gts->regs->sp;
#else
				gts->x86_32_sp = gts->regs->esp;
#endif
#endif
			} else {
#ifdef CONFIG_X86_32
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,25))
				gts->x86_32_sp = (unsigned long)&gts->regs->sp;
#else
				gts->x86_32_sp = (unsigned long)&gts->regs->esp;
#endif
#endif
			}
		} else {
#ifdef CONFIG_X86_32
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,25))
			gts->x86_32_sp = (unsigned long)&gts->regs->sp;
#else
			gts->x86_32_sp = (unsigned long)&gts->regs->esp;
#endif
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0)) && defined CONFIG_UPROBES
			if (gts->tpe->type == gtp_entry_uprobe)
				gts->read_memory = gtp_task_handler_read;
#endif
		}
	}

	if ((gts->tpe->flags & GTP_ENTRY_FLAGS_REG) == 0)
		return;

#if defined(GTP_FTRACE_RING_BUFFER) || defined(GTP_RB)
	if (!gtp_pipe_trace && get_current()->pid == gtp_gtpframe_pipe_pid)
		return;
#endif

	if ((gts->tpe->flags & GTP_ENTRY_FLAGS_SELF_TRACE) == 0
	    && (get_current()->pid == gtp_gtp_pid
		|| get_current()->pid == gtp_gtpframe_pid)) {
			return;
	}

	if (gts->tpe->flags & GTP_ENTRY_FLAGS_HAVE_PRINTK) {
		gts->printk_level = 8;
		gts->printk_str = gts->tpe->printk_str;
	}

	/* Condition.  */
	if (gts->tpe->cond) {
		int	run;

		gts->run = &run;
		if (gtp_action_x(gts, gts->tpe->cond))
			goto tpe_stop;
		if (!run)
			return;
	}

	gts->run = NULL;

	/* Pass.  */
	if (gts->step == 0 && gts->tpe->flags & GTP_ENTRY_FLAGS_HAVE_PASS) {
		if (atomic_dec_return(&gts->tpe->current_pass) < 0)
			goto tpe_stop;
	}

	/* Handle actions.  */
	head = gts->step ? &gts->tpe->step_action_list :
			   &gts->tpe->action_list;
	list_for_each(cur, head) {
		ae = list_entry(cur, struct action, node);
		switch (ae->type) {
		case 'R':
			if (gtp_action_r(gts, ae))
				goto tpe_stop;
			break;
		case 'X':
		case 0xff:
			if (gtp_action_x(gts, ae))
				goto tpe_stop;
			break;
		case 'M':
			if (gtp_action_memory_read(gts, ae->u.m.regnum,
						   ae->u.m.offset,
						   ae->u.m.size))
				goto tpe_stop;
			break;
		}
	}

#if defined(GTP_FTRACE_RING_BUFFER) || defined(GTP_RB)
	if (gts->next) {
#ifdef GTP_RB
		GTP_RB_UNLOCK(gts->next);
#endif
		gtp_handler_wakeup();
	}
#endif

	return;

tpe_stop:
#if defined(GTP_FTRACE_RING_BUFFER) || defined(GTP_RB)
	if (gts->next) {
#ifdef GTP_RB
		GTP_RB_UNLOCK(gts->next);
#endif
		gtp_handler_wakeup();
	}
#endif

	gts->tpe->flags &= ~GTP_ENTRY_FLAGS_REG;

	/* XXX: need add stop code for other types of tracepoint.  */
	if (gts->tpe->type == gtp_entry_kprobe) {
		/* Following code can insert this task into tasklet without
		   wake up softirqd.  */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,13,0))
		preempt_count_add(HARDIRQ_OFFSET);
#else
		add_preempt_count(HARDIRQ_OFFSET);
#endif
		tasklet_schedule(&gts->tpe->u.kp.stop_tasklet);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,13,0))
		preempt_count_sub(HARDIRQ_OFFSET);
#else
		sub_preempt_count(HARDIRQ_OFFSET);
#endif
	}

#ifdef GTP_DEBUG_V
	printk(GTP_DEBUG_V "gtp_handler: tracepoint %d %p stop.\n",
		(int)gts->tpe->num, (void *)(CORE_ADDR)gts->tpe->addr);
#endif
	return;
}

static DEFINE_PER_CPU(int, gtp_handler_began);

#ifdef CONFIG_X86
static int	gtp_access_cooked_rdtsc;
#endif
static int	gtp_access_cooked_clock;
#ifdef GTP_PERF_EVENTS
static int	gtp_have_pc_pe;
#endif

static void
gtp_handler_begin(void)
{
	if (!__get_cpu_var(gtp_handler_began)) {
#ifdef CONFIG_X86
		if (gtp_access_cooked_rdtsc) {
			u64	a;

			rdtscll(a);
			__get_cpu_var(rdtsc_current) = a;
		}
#endif

		if (gtp_access_cooked_clock)
			__get_cpu_var(local_clock_current) = GTP_LOCAL_CLOCK;

#ifdef GTP_PERF_EVENTS
		if (gtp_have_pc_pe)
			pc_pe_list_disable();
#endif

		__get_cpu_var(gtp_handler_began) = 1;
	}
}

static void
gtp_handler_end(void)
{
	if (__get_cpu_var(gtp_handler_began)) {
#ifdef GTP_PERF_EVENTS
		if (gtp_have_pc_pe)
			pc_pe_list_enable();
#endif

		if (gtp_access_cooked_clock) {
			__get_cpu_var(local_clock_offset) += GTP_LOCAL_CLOCK
					- __get_cpu_var(local_clock_current);
			__get_cpu_var(local_clock_current) = 0;
		}

#ifdef CONFIG_X86
		if (gtp_access_cooked_rdtsc) {
			u64	a;

			rdtscll(a);
			__get_cpu_var(rdtsc_offset) += a
					- __get_cpu_var(rdtsc_current);
			__get_cpu_var(rdtsc_current) = 0;
		}
#endif

		__get_cpu_var(gtp_handler_began) = 0;
	}
}

static inline void
gtp_kp_pre_handler_1(struct kprobe *p, struct pt_regs *regs)
{
	struct kretprobe	*kpret;
	struct gtp_kp		*gkp;
	union gtp_entry_u	*u;
	struct gtp_trace_s	gts;

	memset(&gts, 0, sizeof(struct gtp_trace_s));

	kpret = container_of(p, struct kretprobe, kp);
	gkp = container_of(kpret, struct gtp_kp, kpret);
	u = container_of(gkp, union gtp_entry_u, kp);
	gts.tpe = container_of(u, struct gtp_entry, u);
	gts.regs = regs;

	gtp_handler(&gts);
}

static inline void
gtp_kp_post_handler_1(struct kprobe *p, struct pt_regs *regs,
		      unsigned long flags)
{
	struct kretprobe	*kpret;
	struct gtp_kp		*gkp;
	union gtp_entry_u	*u;
	struct gtp_entry	*tpe;
	struct gtp_trace_s	gts;

	kpret = container_of(p, struct kretprobe, kp);
	gkp = container_of(kpret, struct gtp_kp, kpret);
	u = container_of(gkp, union gtp_entry_u, kp);
	tpe = container_of(u, struct gtp_entry, u);

	if (tpe->step == 1) {
		memset(&gts, 0, sizeof(struct gtp_trace_s));

		gts.tpe = tpe;
		gts.regs = regs;
		gts.step = tpe->step;

		gtp_handler(&gts);
	}

#ifdef CONFIG_X86
	if (tpe->step > 1) {
		/* Let while-stepping begin.  */
		/*XXX if there a another one, maybe we need add end frame to let reader know that this while step stop.  */
		__get_cpu_var(gtp_step).step = tpe->step;
		__get_cpu_var(gtp_step).tpe = tpe;
		#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,25))
		if (regs->flags & X86_EFLAGS_IF)
		#else
		if (regs->eflags & X86_EFLAGS_IF)
		#endif
			__get_cpu_var(gtp_step).irq_need_open = 1;
		else
			__get_cpu_var(gtp_step).irq_need_open = 0;
		#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,25))
		regs->flags |= X86_EFLAGS_TF;
		regs->flags &= ~(X86_EFLAGS_IF);
		#else
		regs->eflags |= X86_EFLAGS_TF;
		regs->eflags &= ~(X86_EFLAGS_IF);
		#endif
	}
#endif
}

static inline void
gtp_kp_ret_handler_1(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	struct gtp_trace_s	gts;
	struct gtp_kp		*gkp;
	union gtp_entry_u	*u;

	memset(&gts, 0, sizeof(struct gtp_trace_s));

	gkp = container_of(ri->rp, struct gtp_kp, kpret);
	u = container_of(gkp, union gtp_entry_u, kp);
	gts.tpe = container_of(u, struct gtp_entry, u);
	gts.regs = regs;
	gts.ri = ri;

	gtp_handler(&gts);
}

static int
gtp_kp_pre_handler_plus_step(struct kprobe *p, struct pt_regs *regs)
{
	gtp_handler_begin();

	gtp_kp_pre_handler_1(p, regs);

	return 0;
}

static int
gtp_kp_pre_handler_plus(struct kprobe *p, struct pt_regs *regs)
{
	gtp_handler_begin();

	gtp_kp_pre_handler_1(p, regs);

	gtp_handler_end();

	return 0;
}

static int
gtp_kp_pre_handler(struct kprobe *p, struct pt_regs *regs)
{
	gtp_kp_pre_handler_1(p, regs);

	return 0;
}

/* Only available when tpe->step is true.  */

static void
gtp_kp_post_handler_plus(struct kprobe *p, struct pt_regs *regs,
			 unsigned long flags)
{
	gtp_kp_post_handler_1(p, regs, flags);

	gtp_handler_end();
}

/* Only available when tpe->step is true.  */

static void
gtp_kp_post_handler(struct kprobe *p, struct pt_regs *regs,
			 unsigned long flags)
{
	gtp_kp_post_handler_1(p, regs, flags);
}

static int
gtp_kp_ret_handler_plus(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	gtp_handler_begin();

	gtp_kp_ret_handler_1(ri, regs);

	gtp_handler_end();

	return 0;
}

static int
gtp_kp_ret_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	gtp_kp_ret_handler_1(ri, regs);

	return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0)) && defined CONFIG_UPROBES
static int
gtp_up_handler(struct uprobe_consumer *self, struct pt_regs *regs)
{
	struct gtp_up		*gup;
	union gtp_entry_u	*u;
	struct gtp_trace_s	gts;

	memset(&gts, 0, sizeof(struct gtp_trace_s));

	gup = container_of(self, struct gtp_up, uc);
	u = container_of(gup, union gtp_entry_u, up);
	gts.tpe = container_of(u, struct gtp_entry, u);

	gts.regs = regs;
	gts.read_memory = gtp_task_handler_read;
#ifdef CONFIG_X86_32
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,25))
	gts.x86_32_sp = gts.regs->sp;
#else
	gts.x86_32_sp = gts.regs->esp;
#endif
#endif
	gtp_handler(&gts);

	return 0;
}
#endif

static struct action *
gtp_action_alloc(char type)
{
	struct action	*ret;

	ret = (struct action *)kzalloc(sizeof (struct action), GFP_KERNEL);
	if (ret)
		ret->type = type;

	return ret;
}

static void
gtp_action_release(struct action *ae)
{
	if (ae == NULL)
		return;
	if ((ae->type == 'X' || ae->type == 0xff) && ae->u.exp.buf)
		kfree(ae->u.exp.buf);
	kfree(ae);
}

static void
gtp_action_list_release(struct list_head *list)
{
	struct list_head	*cur, *tmp;
	struct action		*ae;

	list_for_each_safe(cur, tmp, list) {
		ae = list_entry(cur, struct action, node);
		list_del(&ae->node);
		gtp_action_release(ae);
	}
}

static void
gtp_src_release(struct gtpsrc *src)
{
	struct gtpsrc	*src2;

	while (src) {
		src2 = src;
		src = src->next;
		kfree(src2->src);
		kfree(src2);
	}
}

static void
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,19))
gtp_tracepoint_stop(struct work_struct *work)
{
	struct gtp_kp		*gkp = container_of(work,
						    struct gtp_kp,
						    stop_work);
	union gtp_entry_u	*u = container_of(gkp,
						  union gtp_entry_u,
						  kp);
	struct gtp_entry	*tpe = container_of(u,
						    struct gtp_entry,
						    u);
#else
gtp_tracepoint_stop(void *p)
{
	struct gtp_entry	*tpe = p;
#endif

#ifdef GTP_DEBUG
	printk(GTP_DEBUG "gtp_tracepoint_stop: tracepoint %d %p\n",
	       (int)tpe->num, (void *)(CORE_ADDR)tpe->addr);
#endif


	if (tpe->type == gtp_entry_kprobe) {
		if (tpe->flags & GTP_ENTRY_FLAGS_IS_KRETPROBE)
			unregister_kretprobe(&tpe->u.kp.kpret);
		else
			unregister_kprobe(&tpe->u.kp.kpret.kp);
	}
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
static void
gtp_tracepoint_enable(struct work_struct *work)
{
	struct gtp_entry	*tpe = container_of(work,
						    struct gtp_entry,
						    enable_work);

#ifdef GTP_DEBUG
	printk(GTP_DEBUG "gtp_tracepoint_enable: tracepoint %d %p\n",
	       (int)tpe->num, (void *)(CORE_ADDR)tpe->addr);
#endif

	if (tpe->type == gtp_entry_kprobe) {
		if (tpe->flags & GTP_ENTRY_FLAGS_IS_KRETPROBE)
			enable_kretprobe(&tpe->u.kp.kpret);
		else
			enable_kprobe(&tpe->u.kp.kpret.kp);
	}
}

static void
gtp_tracepoint_disable(struct work_struct *work)
{
	struct gtp_entry	*tpe = container_of(work,
						    struct gtp_entry,
						    disable_work);

#ifdef GTP_DEBUG
	printk(GTP_DEBUG "gtp_tracepoint_disable: tracepoint %d %p\n",
	       (int)tpe->num, (void *)(CORE_ADDR)tpe->addr);
#endif

	if (tpe->type == gtp_entry_kprobe) {
		if (tpe->flags & GTP_ENTRY_FLAGS_IS_KRETPROBE)
			disable_kretprobe(&tpe->u.kp.kpret);
		else
			disable_kprobe(&tpe->u.kp.kpret.kp);
	}
}
#endif

/* Alloc a gtp_entry and add it to gtp_list.  */

static struct gtp_entry *
gtp_list_add(ULONGEST num, ULONGEST addr)
{
	struct gtp_entry	*ret = kzalloc(sizeof(struct gtp_entry),
					       GFP_KERNEL);

	if (!ret)
		goto out;
	ret->type = gtp_entry_kprobe;
	ret->num = num;
	ret->addr = addr;

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,19))
	INIT_WORK(&ret->u.kp.stop_work, gtp_tracepoint_stop);
#else
	INIT_WORK(&ret->u.kp.stop_work, gtp_tracepoint_stop, ret);
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
	INIT_WORK(&ret->enable_work, gtp_tracepoint_enable);
	INIT_WORK(&ret->disable_work, gtp_tracepoint_disable);
#endif
	INIT_LIST_HEAD(&ret->action_list);
	INIT_LIST_HEAD(&ret->step_action_list);

	ret->pid = gtp_current_pid;

	/* Add to gtp_list.  */
	ret->next = gtp_list;
	gtp_list = ret;

out:
	return ret;
}

static struct gtp_entry *
gtp_list_find(ULONGEST num, ULONGEST addr)
{
	struct gtp_entry	*tpe;

	for (tpe = gtp_list; tpe; tpe = tpe->next) {
		if (tpe->num == num && tpe->addr == addr)
			return tpe;
	}

	return NULL;
}

/* If more than one gtp entry have same num, return NULL.  */

static struct gtp_entry *
gtp_list_find_without_addr_do_check(ULONGEST num)
{
	struct gtp_entry	*tpe, *ret = NULL;

	for (tpe = gtp_list; tpe; tpe = tpe->next) {
		if (tpe->num == num) {
			if (ret)
				return NULL;
			else
				ret = tpe;
		}
	}

	return ret;
}

static void
gtp_list_release(void)
{
	struct gtp_entry	*tpe;

	while (gtp_list) {
		tpe = gtp_list;
		gtp_list = gtp_list->next;
		gtp_action_release(tpe->cond);
		gtp_action_list_release(&tpe->action_list);
		gtp_action_list_release(&tpe->step_action_list);
		gtp_src_release(tpe->src);
		gtp_src_release(tpe->action_cmd);
		gtp_src_release(tpe->printk_str);
		kfree(tpe);
	}

	current_gtp = NULL;
	current_gtp_action_cmd = NULL;
	current_gtp_src = NULL;
}

#ifdef GTP_FTRACE_RING_BUFFER
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,34)) || defined(GTP_SELF_RING_BUFFER)
static void
gtp_frame_iter_open(void)
{
	int	cpu;


	for_each_possible_cpu(cpu)
		gtp_frame_iter[cpu] = ring_buffer_read_prepare(gtp_frame, cpu);
	ring_buffer_read_prepare_sync();
	for_each_possible_cpu(cpu) {
		ring_buffer_read_start(gtp_frame_iter[cpu]);
	}
}
#else
static void
gtp_frame_iter_open(void)
{
	int	cpu;

	for_each_possible_cpu(cpu) {
		gtp_frame_iter[cpu] = ring_buffer_read_start(gtp_frame, cpu);
		ring_buffer_iter_reset(gtp_frame_iter[cpu]);
	}
}
#endif

static void
gtp_frame_iter_reset(void)
{
	int	cpu;

	for_each_possible_cpu(cpu)
		ring_buffer_iter_reset(gtp_frame_iter[cpu]);
	gtp_frame_current_num = -1;
}

static int
gtp_frame_iter_peek_head(void)
{
	int	cpu;
	int	ret = -1;
	u64	min = 0;

	for_each_possible_cpu(cpu) {
		struct ring_buffer_event	*rbe;
		char				*tmp;
		u64				ts;

		while (1) {
			rbe = ring_buffer_iter_peek(gtp_frame_iter[cpu], &ts);
			if (rbe == NULL)
				break;
			tmp = ring_buffer_event_data(rbe);
			if (FID(tmp) == FID_HEAD)
				break;
			ring_buffer_read(gtp_frame_iter[cpu], NULL);
		}

		if (rbe) {
			if ((min && ts < min) || !min) {
				min = ts;
				ret = cpu;
			}
		}
	}

	if (ret < 0)
		gtp_frame_current_num = -1;
	else
		gtp_frame_current_num++;
	return ret;
}

static void
gtp_frame_iter_close(void)
{
	int	cpu;

	for_each_possible_cpu(cpu) {
		if (gtp_frame_iter[cpu]) {
			ring_buffer_read_finish(gtp_frame_iter[cpu]);
			gtp_frame_iter[cpu] = NULL;
		}
	}
}
#endif

static void
gtp_frame_reset(void)
{
	gtp_frame_current_num = -1;
#ifdef GTP_FRAME_SIMPLE
	gtp_frame_r_start = gtp_frame;
	gtp_frame_w_start = gtp_frame;
	gtp_frame_end = gtp_frame + GTP_FRAME_SIZE;
	gtp_frame_is_circular = 0;
	gtp_frame_current = NULL;
#endif
#ifdef GTP_FTRACE_RING_BUFFER
	gtp_frame_iter_close();
	if (gtp_frame)
		ring_buffer_reset(gtp_frame);
#endif
#ifdef GTP_RB
	gtp_rb_reset();
#endif
	atomic_set(&gtp_frame_create, 0);
	if (gtp_frame_file) {
		vfree(gtp_frame_file);
		gtp_frame_file = NULL;
		gtp_frame_file_size = 0;
	}
}

static int
hex2int(char hex, int *i)
{
	if ((hex >= '0') && (hex <= '9')) {
		*i = hex - '0';
		return 1;
	}
	if ((hex >= 'a') && (hex <= 'f')) {
		*i = hex - 'a' + 10;
		return 1;
	}
	if ((hex >= 'A') && (hex <= 'F')) {
		*i = hex - 'A' + 10;
		return 1;
	}

	return 0;
}

static char *
hex2ulongest(char *pkg, ULONGEST *u64)
{
	int	i;

	if (u64)
		*u64 = 0;
	while (hex2int(pkg[0], &i)) {
		pkg++;
		if (u64) {
			*u64 = (*u64) << 4;
			*u64 |= i & 0xf;
		}
	}

	return pkg;
}

static char *
hex2string(char *pkg, char *out)
{
	char	*ret = out;
	int	i, j;

	while (hex2int(pkg[0], &i) && hex2int(pkg[1], &j)) {
		out[0] = i * 16 + j;
		pkg += 2;
		out += 1;
	}
	out[0] = '\0';

	return ret;
}

static void
gtpro_list_clear(void)
{
	struct gtpro_entry	*e;

	while (gtpro_list) {
		e = gtpro_list;
		gtpro_list = gtpro_list->next;
		kfree(e);
	}
}

static struct gtpro_entry *
gtpro_list_add(CORE_ADDR start, CORE_ADDR end)
{
	struct gtpro_entry	*e;

	e = kmalloc(sizeof(struct gtpro_entry), GFP_KERNEL);
	if (e == NULL)
		goto out;

#ifdef GTP_DEBUG
	printk(GTP_DEBUG "gtpro_list_add: %p %p\n", (void *)start, (void *)end);
#endif

	e->start = start;
	e->end = end;

	e->next = gtpro_list;
	gtpro_list = e;

out:
	return e;
}

static int
gtp_src_add(char *begin, char *end, struct gtpsrc **src_list)
{
	struct gtpsrc	*src, *srctail;

	src = kmalloc(sizeof(struct gtpsrc), GFP_KERNEL);
	if (src == NULL)
		return -ENOMEM;
	src->next = NULL;
	src->src = gtp_strdup(begin, end);
	if (src->src == NULL) {
		kfree(src);
		return -ENOMEM;
	}

	if (*src_list) {
		for (srctail = *src_list; srctail->next;
		      srctail = srctail->next)
			;
		srctail->next = src;
	} else
		*src_list = src;

	return 0;
}

#ifdef CONFIG_X86
static void
gtp_hw_breakpoint_handler_1 (struct gtp_hwb_s *hwb, struct pt_regs *regs)
{
	struct gtp_trace_s	gts;

	preempt_disable();

	memset(&gts, 0, sizeof(struct gtp_trace_s));
	gts.tpe = hwb->watch;
	gts.regs = regs;
	gts.hwb = hwb;
	gtp_handler(&gts);

	preempt_enable_no_resched();

	/* Update hwb->prev_val.  */
	if (gts.hwb_current_val_gotten)
		hwb->prev_val = gts.hwb_current_val;
	else
		gtp_get_addr_val(hwb->addr, hwb->size, &(hwb->prev_val));
}

#define ADDR_PREFIX_OPCODE 0x67
#define DATA_PREFIX_OPCODE 0x66
#define LOCK_PREFIX_OPCODE 0xf0
#define CS_PREFIX_OPCODE 0x2e
#define DS_PREFIX_OPCODE 0x3e
#define ES_PREFIX_OPCODE 0x26
#define FS_PREFIX_OPCODE 0x64
#define GS_PREFIX_OPCODE 0x65
#define SS_PREFIX_OPCODE 0x36
#define REPNE_PREFIX_OPCODE 0xf2
#define REPE_PREFIX_OPCODE  0xf3

static int
gtp_step_check_insn(struct pt_regs *regs)
{
	uint32_t	opcode;
	uint8_t		opcode8;
	unsigned long	pc = GTP_REGS_PC(regs);

	/* prefixes */
	while (1) {
		if (probe_kernel_read(&opcode8, (void *)pc, 1))
			return -1;
		pc++;
		switch (opcode8) {
		case REPE_PREFIX_OPCODE:
		case REPNE_PREFIX_OPCODE:
		case LOCK_PREFIX_OPCODE:
		case CS_PREFIX_OPCODE:
		case SS_PREFIX_OPCODE:
		case DS_PREFIX_OPCODE:
		case ES_PREFIX_OPCODE:
		case FS_PREFIX_OPCODE:
		case GS_PREFIX_OPCODE:
		case DATA_PREFIX_OPCODE:
		case ADDR_PREFIX_OPCODE:
#ifndef CONFIG_X86_32
		case 0x40 ... 0x4f:
#endif
			break;
		default:
			goto out_prefixes;
		}
	}
out_prefixes:

	opcode = (uint32_t)opcode8;
reswitch:
	switch (opcode) {
	case 0x0f:
		if (probe_kernel_read(&opcode8, (void *)pc, 1))
			return -1;
		opcode = (uint32_t) opcode8 | 0x0f00;
		goto reswitch;
		break;
	case 0xfb:
		/* sti */
		__get_cpu_var(gtp_step).irq_need_open = 1;
		GTP_REGS_PC(regs) = pc;
		break;
	case 0xfa:
		/* cli */
		__get_cpu_var(gtp_step).irq_need_open = 0;
		GTP_REGS_PC(regs) = pc;
		break;
	case 0x0f07:
		/* sysret */
		return 1;
		break;
	};

	return 0;
}

static int
gtp_notifier_call(struct notifier_block *self, unsigned long cmd,
		   void *ptr)
{
	int		ret = NOTIFY_DONE;
	unsigned long	flags;
	struct die_args *args;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33))
	int		i;
#endif
	unsigned long	dr6;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33))
	unsigned long	*dr6_p;
#endif

	if (cmd != DIE_DEBUG)
		return ret;

	local_irq_save(flags);
	args = ptr;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33))
	/* Get from X86 hw_breakpoint_handler.  */
	dr6_p = (unsigned long *)ERR_PTR(args->err);
	dr6 = *dr6_p;
#else
	dr6 = args->err;
#endif
	gtp_set_debugreg(GTP_HWB_DR7_DEF, 7);

	/* Handle while-stepping.  */
	spin_lock(&__get_cpu_var(gtp_step).lock);
	if ((dr6 & 0x4000) != 0) {
		/* Clear the bit that handle by KGTP.  */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33))
		(*dr6_p) &= ~(0x4000);
#else
 		dr6 &= ~(0x4000);
#endif
		if (!__get_cpu_var(gtp_step).tpe || user_mode(args->regs))
			gtp_step_stop(args->regs);
		else {
			int need_stop = gtp_step_check_insn(args->regs);
			if (need_stop < 0)
				printk(KERN_WARNING "KGTP: check insn in %p got error.",
				       (void *)GTP_REGS_PC(args->regs));

			preempt_disable();
			{
				struct gtp_trace_s	gts;

				memset(&gts, 0, sizeof(struct gtp_trace_s));
				gts.tpe = __get_cpu_var(gtp_step).tpe;
				gts.regs = args->regs;
				gts.step = __get_cpu_var(gtp_step).step;
				gtp_handler(&gts);
			}
			preempt_enable_no_resched();

			if (__get_cpu_var(gtp_step).step > 1 && !need_stop) {
				/* XXX: not sure need set eflags each step.  */
#if 0
				#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,25))
				args->regs->flags |= X86_EFLAGS_TF;
				args->regs->flags &= ~(X86_EFLAGS_IF);
				#else
				args->regs->eflags |= X86_EFLAGS_TF;
				args->regs->eflags &= ~(X86_EFLAGS_IF);
				#endif
#endif
				__get_cpu_var(gtp_step).step--;
			} else {
				/*XXX: maybe need add a end frame.  */
				gtp_step_stop(args->regs);
			}
		}
	}
	spin_unlock(&__get_cpu_var(gtp_step).lock);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33))
	/* Handle watch traceppoint.  */
	if ((dr6 & 0xf) == 0)
		goto out;
	read_lock(&gtp_hwb_lock);

	for (i = 0; i < HWB_NUM; i++) {
		if ((dr6 & (0x1 << i)) == 0)
			continue;
		/* Clear the bit that handle by KGTP.  */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33))
		(*dr6_p) &= ~(0x1 << i);
#else
		dr6 &= ~(0x1 << i);
#endif
		if (gtp_hwb[i].watch == NULL)
			continue;
		/* Check if gtp_hwb is updated in other CPU.  */
		if (__get_cpu_var(gtp_hwb_sync_count_local) != gtp_hwb_sync_count) {
			unsigned long	addr;

			gtp_get_debugreg(addr, i);
			if (addr != gtp_hwb[i].addr)
				continue;
		}
		gtp_hw_breakpoint_handler_1(&gtp_hwb[i], args->regs);
	}

	/* If the HWB need update in this CPU, just update it.  */
	if (__get_cpu_var(gtp_hwb_sync_count_local) != gtp_hwb_sync_count) {
		gtp_set_debugreg(gtp_hwb_drx[0], 0);
		gtp_set_debugreg(gtp_hwb_drx[1], 1);
		gtp_set_debugreg(gtp_hwb_drx[2], 2);
		gtp_set_debugreg(gtp_hwb_drx[3], 3);
		__get_cpu_var(gtp_hwb_sync_count_local) = gtp_hwb_sync_count;
	}

	gtp_set_debugreg(gtp_hwb_dr7, 7);
	read_unlock(&gtp_hwb_lock);
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33))
out:
	gtp_set_debugreg(dr6, 6);
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33))
	/* If have some other traps, let other handler handle it.  */
	if (((*dr6_p) & GTP_HWB_DR6_MASK) == 0)
		ret = NOTIFY_STOP;
	current->thread.debugreg6 = *dr6_p;
#else
	if ((dr6 & GTP_HWB_DR6_MASK) == 0)
		ret = NOTIFY_STOP;
	current->thread.debugreg6 = dr6;
#endif

	local_irq_restore(flags);
	return ret;
}

static struct notifier_block gtp_notifier = {
	.notifier_call = gtp_notifier_call,
	.priority = 0x7ffffffe /* we need to be notified after kprobe.  */
};
#endif

#ifdef CONFIG_X86
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33))
static void
gtp_hw_breakpoint_handler(int num, struct pt_regs *regs)
{
	read_lock(&gtp_hwb_lock);

	/* Handle the warning of new gcc.  */
	if (num < 0 || num >= HWB_NUM)
		goto out;

	if (gtp_hwb[num].watch == NULL)
		goto out;
	if (__get_cpu_var(gtp_hwb_sync_count_local) != gtp_hwb_sync_count) {
		unsigned long	addr;

		gtp_get_debugreg(addr, num);
		if (addr != gtp_hwb[num].addr) {
			gtp_set_debugreg(gtp_hwb[num].addr, num);
			goto out;
		}
	}

	gtp_hw_breakpoint_handler_1(&gtp_hwb[num], regs);
out:
	read_unlock(&gtp_hwb_lock);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,1,0))
static void
gtp_hw_breakpoint_0_handler(struct perf_event *bp, struct perf_sample_data *data,
			    struct pt_regs *regs)
{
	gtp_hw_breakpoint_handler(breakinfo[0].num, regs);
}

static void
gtp_hw_breakpoint_1_handler(struct perf_event *bp, struct perf_sample_data *data,
			    struct pt_regs *regs)
{
	gtp_hw_breakpoint_handler(breakinfo[1].num, regs);
}

static void
gtp_hw_breakpoint_2_handler(struct perf_event *bp, struct perf_sample_data *data,
			    struct pt_regs *regs)
{
	gtp_hw_breakpoint_handler(breakinfo[2].num, regs);
}

static void
gtp_hw_breakpoint_3_handler(struct perf_event *bp, struct perf_sample_data *data,
			    struct pt_regs *regs)
{
	gtp_hw_breakpoint_handler(breakinfo[3].num, regs);
}
#else
static void
gtp_hw_breakpoint_0_handler(struct perf_event *bp, int nmi,
			    struct perf_sample_data *data,
			    struct pt_regs *regs)
{
	gtp_hw_breakpoint_handler(breakinfo[0].num, regs);
}

static void
gtp_hw_breakpoint_1_handler(struct perf_event *bp, int nmi,
			    struct perf_sample_data *data,
			    struct pt_regs *regs)
{
	gtp_hw_breakpoint_handler(breakinfo[1].num, regs);
}

static void
gtp_hw_breakpoint_2_handler(struct perf_event *bp, int nmi,
			    struct perf_sample_data *data,
			    struct pt_regs *regs)
{
	gtp_hw_breakpoint_handler(breakinfo[2].num, regs);
}

static void
gtp_hw_breakpoint_3_handler(struct perf_event *bp, int nmi,
			    struct perf_sample_data *data,
			    struct pt_regs *regs)
{
	gtp_hw_breakpoint_handler(breakinfo[3].num, regs);
}
#endif
#endif

static unsigned int
gtp_hwb_size_to_arch(int size)
{
	unsigned int	ret;

	switch (size) {
	default:
	case 1:
		ret = 0;
		break;
	case 2:
		ret = 1;
		break;
	case 4:
		ret = 3;
		break;
	case 8:
		ret = 2;
		break;
	}

	return ret;
}

static unsigned int
gtp_hwb_type_to_arch(int type)
{
	unsigned int	ret;

	switch (type) {
	default:
	case gtp_watch_write:
		ret = 1;
		break;
	case gtp_watch_exec:
		ret = 0;
		break;
	case gtp_watch_read_write:
		ret = 3;
		break;
	}

	return ret;
}

static int
gtp_register_hwb(const struct gtp_hwb_s *arg, int nowait)
{
	unsigned long		flags;
	struct gtp_hwb_s	*hwb = NULL;
	int			ret = -EBUSY;

	write_lock_irqsave(&gtp_hwb_lock, flags);
	if (!list_empty(&gtp_hwb_unused_list)) {
		int	num;
		int64_t	prev_val;

		/* Get the value from address that will watch as prev value.  */
		ret = gtp_get_addr_val(arg->addr, arg->size, &prev_val);
		if (ret)
			goto out;

		hwb = list_first_entry(&gtp_hwb_unused_list, struct gtp_hwb_s, node);
		list_del_init(&hwb->node);
		list_add(&hwb->node, &gtp_hwb_used_list);
		num = hwb->num;
		memcpy((void *)&arg->node, (void *)&hwb->node, sizeof (hwb->node));
		memcpy(hwb, arg, sizeof(struct gtp_hwb_s));
		hwb->num = num;
		hwb->prev_val = prev_val;

		/* Update gtp_hwb_dr7 and gtp_hwb_drx[num].  */
		/* Set Gx.  */
		gtp_hwb_dr7 |= 2 << (num << 1);
		/* Clear RWx and LENx.  */
		gtp_hwb_dr7 &= ~(0xf0000 << (num << 2));
		/* Set RWx and LENx.  */
		gtp_hwb_dr7 |= ((gtp_hwb_size_to_arch(hwb->size) << 2)
				| gtp_hwb_type_to_arch(hwb->type))
			       << ((num << 2) + 16);
		/* Update DRx.  */
		gtp_hwb_drx[num] = hwb->addr;

		/* Set gtp_hwb_dr7 and gtp_hwb_drx[num] to hwb.  */
		gtp_set_debugreg(gtp_hwb_drx[num], num);
		gtp_set_debugreg(gtp_hwb_dr7, 7);

		gtp_hwb_sync_count++;
		hwb->count = gtp_hwb_sync_count;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
		if (nowait) {
			/* Send ipi to let other cpu update.  */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
			arch_send_call_function_ipi_mask(&__get_cpu_var(gtp_hwb_sync_cpu_mask));
#else
			arch_send_call_function_ipi(&__get_cpu_var(gtp_hwb_sync_cpu_mask));
#endif
		}
#endif

		ret = 0;
	}
out:
	write_unlock_irqrestore(&gtp_hwb_lock, flags);

	if (ret == 0 && !nowait)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
		smp_call_function(gtp_hwb_sync, NULL, 1);
#else
		smp_call_function(gtp_hwb_sync, NULL, 0, 1);
#endif

	return ret;
}

static int
gtp_unregister_hwb(CORE_ADDR addr, int sync)
{
	unsigned long		flags;
	struct list_head	*pos;
	struct gtp_hwb_s	*hwb = NULL;
	int			ret = -ENXIO;

	write_lock_irqsave(&gtp_hwb_lock, flags);
	list_for_each(pos, &gtp_hwb_used_list) {
		hwb = list_entry(pos, struct gtp_hwb_s, node);
		if (hwb->addr == addr)
			break;
	}
	if (hwb) {
		list_del_init(&hwb->node);
		list_add_tail(&hwb->node, &gtp_hwb_unused_list);
		hwb->watch = NULL;

		/* Update gtp_hwb_dr7.  */
		/* Clear Gx.  */
		gtp_hwb_dr7 &= ~(2 << (hwb->num << 1));

		if (sync) {
			gtp_hwb_sync_count++;

			/* Sync gtp_hwb_dr7 update to hwb.  */
			gtp_set_debugreg(gtp_hwb_dr7, 7);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
			/* Send ipi to let other cpu update.  */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
			arch_send_call_function_ipi_mask(&__get_cpu_var(gtp_hwb_sync_cpu_mask));
#else
			arch_send_call_function_ipi(&__get_cpu_var(gtp_hwb_sync_cpu_mask));
#endif
#endif
		}

		ret = 0;
	}
	write_unlock_irqrestore(&gtp_hwb_lock, flags);

	return ret;
}
#endif

static int
gtp_gdbrsp_qtstop(void)
{
	struct gtp_entry	*tpe;
#ifdef GTP_PERF_EVENTS
	struct list_head	*cur;
#endif

#ifdef GTP_DEBUG
	printk(GTP_DEBUG "gtp_gdbrsp_qtstop\n");
#endif

#ifdef FRAME_ALLOC_RECORD
	printk(KERN_WARNING "frame_alloc_size = %llu, "
			    "frame_alloc_size_hole = %llu\n",
	       frame_alloc_size, frame_alloc_size_hole);
	frame_alloc_size = 0;
	frame_alloc_size_hole = 0;
#endif

	if (!gtp_start)
		return -EBUSY;

	flush_workqueue(gtp_wq);

	for (tpe = gtp_list; tpe; tpe = tpe->next) {
		if ((tpe->type != gtp_entry_kprobe
		     && tpe->type != gtp_entry_uprobe)
		    || (tpe->flags & GTP_ENTRY_FLAGS_REG) == 0)
			continue;

		if (tpe->type == gtp_entry_kprobe) {
			if (tpe->flags & GTP_ENTRY_FLAGS_IS_KRETPROBE)
				unregister_kretprobe(&tpe->u.kp.kpret);
			else
				unregister_kprobe(&tpe->u.kp.kpret.kp);
			tasklet_kill(&tpe->u.kp.stop_tasklet);
		} else {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0)) && defined CONFIG_UPROBES
			uprobe_unregister(tpe->u.up.inode, tpe->u.up.offset,
					  &tpe->u.up.uc);
#endif
		}

		tpe->flags &= ~GTP_ENTRY_FLAGS_REG;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
		tasklet_kill(&tpe->disable_tasklet);
		tasklet_kill(&tpe->enable_tasklet);
#endif
	}

#ifdef CONFIG_X86
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33))
	if (gtp_have_step || gtp_have_watch_tracepoint)
#else
	if (gtp_have_step)
#endif
		unregister_die_notifier(&gtp_notifier);

	{
		/* Init data of while-stepping.  */
		int	cpu;
		for_each_possible_cpu(cpu) {
			struct gtp_step_s	*step = &per_cpu(gtp_step, cpu);

			spin_lock(&step->lock);
			step->step = 0;
			step->tpe = NULL;
			spin_unlock(&step->lock);
		}
	}
#endif

#ifdef CONFIG_X86
	/* Stop hwb.  */
	if (gtp_have_watch_tracepoint) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33))
		{
			int	i;

			/* Register hw breakpoints.  */
			for (i = 0; i < HWB_NUM; i++) {
				unregister_wide_hw_breakpoint(breakinfo[i].pev);
				breakinfo[i].pev = NULL;
			}
		}
#endif

		gtp_hwb_stop(NULL);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
		smp_call_function(gtp_hwb_stop, NULL, 1);
#else
		smp_call_function(gtp_hwb_stop, NULL, 0, 1);
#endif

		for (tpe = gtp_list; tpe; tpe = tpe->next) {
			if (tpe->type == gtp_entry_kprobe
			    || (tpe->flags & GTP_ENTRY_FLAGS_REG) == 0
			    || tpe->disable)
				continue;

			if (tpe->type == gtp_entry_watch_static)
				gtp_unregister_hwb(tpe->addr, 0);

			tpe->flags &= ~GTP_ENTRY_FLAGS_REG;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
			tasklet_kill(&tpe->disable_tasklet);
			tasklet_kill(&tpe->enable_tasklet);
#endif
		}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
		unregister_kprobe(&gtp_ipi_kp);
#endif
	}
#endif

#ifdef GTP_PERF_EVENTS
	list_for_each(cur, &gtp_var_list) {
		struct gtp_var		*var;
		struct gtp_var_pe	*pe;

		var = list_entry(cur, struct gtp_var, node);
		if (var->type != gtp_var_perf_event
		    && var->type != gtp_var_perf_event_per_cpu)
			continue;
		if (var->type == gtp_var_perf_event_per_cpu
		    && var->u.pc.cpu < 0)
			continue;

		pe = gtp_var_get_pe(var);
		if (pe->pe->event == NULL)
			continue;

		pe->pe->val = perf_event_read_value(pe->pe->event,
						      &(pe->pe->enabled),
						      &(pe->pe->running));
		perf_event_release_kernel(pe->pe->event);
		pe->pe->event = NULL;
	}
#endif

	if (gtp_var_array) {
		kfree(gtp_var_array);
		gtp_var_array = NULL;
	}

#ifdef GTP_FTRACE_RING_BUFFER
	if (gtp_frame) {
		gtp_frame_iter_open();
		gtp_frame_iter_reset();
	}
#endif

	gtp_start = 0;
#if defined(GTP_FTRACE_RING_BUFFER) || defined(GTP_RB)
	if (atomic_read(&gtpframe_pipe_wq_v) > 0) {
		atomic_dec(&gtpframe_pipe_wq_v);
		tasklet_schedule(&gtpframe_pipe_wq_tasklet);
	}
	tasklet_kill(&gtpframe_pipe_wq_tasklet);
#endif
	wake_up_interruptible_nr(&gtpframe_wq, 1);

	return 0;
}

static int
gtp_gdbrsp_qtinit(void)
{
#ifdef GTP_DEBUG
	printk(GTP_DEBUG "gtp_gdbrsp_qtinit\n");
#endif

	if (gtp_start)
		gtp_gdbrsp_qtstop();

	gtp_list_release();

#ifdef GTP_RB
	if (!GTP_RB_PAGE_IS_EMPTY)
#elif defined(GTP_FRAME_SIMPLE) || defined(GTP_FTRACE_RING_BUFFER)
	if (gtp_frame)
#endif
		gtp_frame_reset();

	gtpro_list_clear();
#ifdef CONFIG_X86
	gtp_have_watch_tracepoint = 0;
	gtp_have_step = 0;
#endif

	gtp_var_release(0);

#ifdef CONFIG_X86
	gtp_access_cooked_rdtsc = 0;
#endif
	gtp_access_cooked_clock = 0;
#ifdef GTP_PERF_EVENTS
	gtp_have_pc_pe = 0;
#endif

	return 0;
}

struct gtp_x_loop {
	struct gtp_x_loop	*next;
	unsigned int		addr;
	int			non_goto_done;
};

static struct gtp_x_loop *
gtp_x_loop_find(struct gtp_x_loop *list, unsigned int pc)
{
	struct gtp_x_loop	*ret = NULL;

	for (ret = list; ret; ret = ret->next) {
		if (ret->addr == pc)
			break;
	}

	return ret;
}

static struct gtp_x_loop *
gtp_x_loop_add(struct gtp_x_loop **list, unsigned int pc, int non_goto_done)
{
	struct gtp_x_loop	*ret;

	ret = kmalloc(sizeof(struct gtp_x_loop), GFP_KERNEL);
	if (!ret)
		goto out;

	ret->addr = pc;
	ret->non_goto_done = non_goto_done;

	ret->next = *list;
	*list = ret;

out:
	return ret;
}

struct gtp_x_if_goto {
	struct gtp_x_if_goto	*next;
	unsigned int		ip;
	unsigned int		sp;
};

static struct gtp_x_if_goto *
gtp_x_if_goto_add(struct gtp_x_if_goto **list, unsigned int pc, unsigned int sp)
{
	struct gtp_x_if_goto	*ret;

	ret = kmalloc(sizeof(struct gtp_x_loop), GFP_KERNEL);
	if (!ret)
		goto out;

	ret->ip = pc;
	ret->sp = sp;

	ret->next = *list;
	*list = ret;

out:
	return ret;
}

struct gtp_check_s {
	struct gtp_entry	*tpe;
	struct action		*ae;
	uint8_t			*ebuf;

	struct list_head	var_rw_flags;
	struct list_head	var_pc_checked;
};

/* This struct is used to check if a var is be read and wrotten in a actions.
   If so, to ensure its atomicity, need open NEED_VAR_LOCK to let this
   actions lock GTP_VAR_LOCK when it executes.   */

struct gtp_var_rw_flags {
	struct list_head	node;
	unsigned int		num;
	unsigned int		flags;
};

/* This struct to mark a pc of a actions is checked by gtp_check_getv
   or gtp_check_setv in before.  So it doesn't need be checked again
   by these two functions.  */

struct gtp_var_pc_checked {
	struct list_head	node;
	unsigned int		pc;
};

static void
gtp_check_init(struct gtp_entry *tpe, struct action *ae, uint8_t *ebuf,
	       struct gtp_check_s *check)
{
	check->tpe = tpe;
	check->ae = ae;
	check->ebuf = ebuf;
	INIT_LIST_HEAD(&check->var_rw_flags);
	INIT_LIST_HEAD(&check->var_pc_checked);
}

static void
gtp_check_release(struct gtp_check_s *check)
{
	struct list_head		*cur, *tmp;
	struct gtp_var_pc_checked	*pc_checked;
	struct gtp_var_rw_flags		*rw_flags;

	list_for_each_safe(cur, tmp, &check->var_pc_checked) {
		pc_checked = list_entry(cur, struct gtp_var_pc_checked, node);
		list_del(&pc_checked->node);
		kfree(pc_checked);
	}

	list_for_each_safe(cur, tmp, &check->var_rw_flags) {
		rw_flags = list_entry(cur, struct gtp_var_rw_flags, node);
		list_del(&rw_flags->node);
		kfree(rw_flags);
	}
}

static int
gtp_check_var_add(struct gtp_check_s *check, unsigned int pc,
		  unsigned int num, unsigned int flag)
{
	struct gtp_var_pc_checked	*pc_checked;

	/* Get pc_checked.  */
	pc_checked = kmalloc(sizeof(struct gtp_var_pc_checked), GFP_KERNEL);
	if (pc_checked == NULL)
		return -ENOMEM;
	pc_checked->pc = pc;

	if (check->ae->u.exp.need_var_lock == 0) {
		struct list_head		*cur;
		struct gtp_var_rw_flags		*rw_flags;

		/* Get rw_flags.  */
		list_for_each(cur, &check->var_rw_flags) {
			rw_flags = list_entry(cur, struct gtp_var_rw_flags,
					      node);
			if (rw_flags->num == num)
				break;
			rw_flags = NULL;
		}
		if (rw_flags == NULL) {
			/* TSV num is not available in check->var_rw_flags.
			So alloc and add it.  */
			rw_flags = kmalloc(sizeof(struct gtp_var_rw_flags),
					   GFP_KERNEL);
			if (rw_flags == NULL) {
				kfree(rw_flags);
				return -ENOMEM;
			}
			rw_flags->num = num;
			rw_flags->flags = 0;
			list_add_tail(&rw_flags->node, &check->var_rw_flags);
		}
		rw_flags->flags |= flag;
		if ((rw_flags->flags & 1) && (rw_flags->flags & 2))
			check->ae->u.exp.need_var_lock = 1;
	}

	/* Add pc_checked to check.  */
	list_add_tail(&pc_checked->node, &check->var_pc_checked);

	return 0;
}

static int
gtp_var_pc_checked_find(struct gtp_check_s *check, unsigned int pc)
{
	struct list_head		*cur;
	struct gtp_var_pc_checked	*pc_checked;

	list_for_each(cur, &check->var_pc_checked) {
		pc_checked = list_entry(cur, struct gtp_var_pc_checked, node);
		if (pc_checked->pc == pc)
			return 1;
	}

	return 0;
}

static int
gtp_add_backtrace_actions(struct gtp_entry *tpe, int step)
{
	struct list_head	*head, *cur;
	struct action		*ae;
	int			got_r = 0, got_m = 0;

	head = step ? &tpe->step_action_list : &tpe->action_list;

	list_for_each(cur, head) {
		ae = list_entry(cur, struct action, node);
		if (ae->type == 'R')
			got_r = 1;
		else if (ae->type == 'M' && ae->u.m.regnum == GTP_SP_NUM
			  && ae->u.m.offset == 0 && ae->u.m.size >= gtp_bt_size)
			got_m = 1;

		if (got_r && got_m)
			return 1;
	}

	if (!got_r) {
		ae = gtp_action_alloc('R');
		if (!ae)
			return -ENOMEM;
		list_add_tail(&ae->node, head);
	}

	if (!got_m) {
		ae = gtp_action_alloc('M');
		if (!ae)
			return -ENOMEM;
		ae->u.m.regnum = GTP_SP_NUM;
		ae->u.m.size = gtp_bt_size;
		list_add_tail(&ae->node, head);
	}

	return 1;
}

static int
gtp_check_getv(struct gtp_check_s *check, int step, unsigned int pc)
{
	int		ret = -EINVAL;
	int		arg;
	struct gtp_var	*var;
	unsigned int	flags = 0;

	if (pc + 1 >= check->ae->u.exp.size)
		goto out;
	arg = check->ebuf[pc++];
	arg = (arg << 8) + check->ebuf[pc++];

	if (gtp_var_pc_checked_find(check, pc))
		return 0;

	var = gtp_var_find_num(arg);
	if (var == NULL) {
		printk(KERN_WARNING "Action try to get TSV %d that doesn't exist.\n",
			arg);
		goto out;
	}

	switch (var->type) {
	case gtp_var_special:
		if (arg == GTP_VAR_SELF_TRACE_ID) {
			check->tpe->flags |= GTP_ENTRY_FLAGS_SELF_TRACE;
			ret = 1;
			goto out;
		} else if (arg == GTP_VAR_BT_ID) {
			ret = gtp_add_backtrace_actions (check->tpe, step);
			goto out;
		} else if (arg == GTP_VAR_CURRENT_ID) {
			check->tpe->flags |= GTP_ENTRY_FLAGS_CURRENT_TASK;
			ret = 1;
			goto out;
		}

		if (arg == GTP_VAR_COOKED_CLOCK_ID)
			gtp_access_cooked_clock = 1;
#ifdef CONFIG_X86
		else if (arg == GTP_VAR_COOKED_RDTSC_ID)
			gtp_access_cooked_rdtsc = 1;
#endif
		if (!var->u.hooks || (var->u.hooks
				      && !var->u.hooks->agent_get_val)) {
			printk(KERN_WARNING "Action try to get special TSV %d that cannot be get.\n",
				arg);
			ret = -EINVAL;
			goto out;
		}
		break;

#ifdef GTP_PERF_EVENTS
	case gtp_var_perf_event_per_cpu:
	case gtp_var_perf_event: {
			struct gtp_var_pe	*pe = gtp_var_get_pe(var);

			if (pe->ptid != pe_tv_val
			    && pe->ptid != pe_tv_enabled
			    && pe->ptid != pe_tv_running) {
				printk(KERN_WARNING "Action try to get perf event TSV %d that cannot be get.\n",
				arg);
				goto out;
			}
			if (var->type == gtp_var_perf_event)
				flags = 1;
		}
		break;
#endif

	case gtp_var_normal:
		flags = 1;
		break;
	}

	ret = gtp_check_var_add(check, pc, arg, flags);
	if (ret)
		goto out;
	ret = -EINVAL;

	/* Change the num of var to the num of gtp_var_array.
	   It will make this insn speed up. */
	arg = gtp_var_array_find_num(var);
	if (arg < 0) {
		printk(KERN_WARNING "Action try to set TSV %d that does't inside the gtp var array.\n",
			var->num);
		goto out;
	}
	check->ebuf[pc - 2] = (uint8_t)(arg >> 8);
	check->ebuf[pc - 1] = (uint8_t)(arg & 0xff);
	ret = 0;

out:
	return ret;
}

static int
gtp_check_setv(struct gtp_check_s *check, int step, unsigned int pc,
	       int loop, ULONGEST *stack, ULONGEST top)
{
	int		arg;
	struct gtp_var	*var;
	int		ret = -EINVAL;
	unsigned int	flags = 0;
	int		is_current = 0;

	if (pc + 1 >= check->ae->u.exp.size)
		goto out;
	arg = check->ebuf[pc++];
	arg = (arg << 8) + check->ebuf[pc++];

	if (gtp_var_pc_checked_find(check, pc))
		return 0;

	var = gtp_var_find_num(arg);
	if (var == NULL) {
		printk(KERN_WARNING "Action try to set TSV %d that doesn't exist.\n",
			arg);
		goto out;
	}

	switch (var->type) {
	case gtp_var_special:
		switch (arg) {
		case GTP_VAR_SELF_TRACE_ID:
			check->tpe->flags |= GTP_ENTRY_FLAGS_SELF_TRACE;
			ret = 1;
			goto out;
			break;
		case GTP_VAR_KRET_ID:
			/* XXX: still not set it value to maxactive.  */
			check->tpe->flags |= GTP_ENTRY_FLAGS_IS_KRETPROBE;
			ret = 1;
			goto out;
			break;
		case GTP_VAR_BT_ID:
			ret = gtp_add_backtrace_actions (check->tpe, step);
			goto out;
			break;
		case GTP_VAR_CURRENT_ID:
			check->tpe->flags |= GTP_ENTRY_FLAGS_CURRENT_TASK;
			is_current = 1;
			break;
		case GTP_VAR_PRINTK_LEVEL_ID:
			if (loop || step) {
				printk(KERN_WARNING "Loop or step action doesn't support printk.\n");
				goto out;
			} else
				check->tpe->flags |= GTP_ENTRY_FLAGS_HAVE_PRINTK;
			break;
#ifdef CONFIG_X86
		case GTP_WATCH_STATIC_ID:
			if (stack == NULL || top > 1) {
				printk(KERN_WARNING "$watch_static just support set to 0 or 1.\n");
				goto out;
			}
			/* Init watch struct inside gtp_entry.  */
			if (check->tpe->type != gtp_entry_watch_static
			    && check->tpe->type != gtp_entry_watch_dynamic) {
				check->tpe->type = gtp_entry_watch_dynamic;
				check->tpe->u.watch.type = gtp_watch_write;
				check->tpe->u.watch.size = 1;
			}
			gtp_have_watch_tracepoint = 1;
			if (top == 1)
				check->tpe->type = gtp_entry_watch_static;
			else
				check->tpe->type = gtp_entry_watch_dynamic;
			ret = 1;
			goto out;
			break;
		case GTP_WATCH_TYPE_ID:
			if (stack && (check->tpe->type == gtp_entry_watch_static
				      || check->tpe->type == gtp_entry_watch_dynamic)) {
				if (top != gtp_watch_exec
				    && top != gtp_watch_write
				    && top != gtp_watch_read_write) {
					printk(KERN_WARNING "$watch_type just support set to 0, 1 or 2.\n");
					goto out;
				}
				check->tpe->u.watch.type = top;
				ret = 1;
				goto out;
			}
			break;
		case GTP_WATCH_SIZE_ID:
			if (stack && (check->tpe->type == gtp_entry_watch_static
				      || check->tpe->type == gtp_entry_watch_dynamic)) {
				if (top != 1 && top != 2 && top != 4
				    && top != 8) {
					printk(KERN_WARNING "$watch_size just support set to 1, 2, 4 or 8.\n");
					goto out;
				}
				check->tpe->u.watch.size = top;
				ret = 1;
				goto out;
			}
			break;
#endif
		case GTP_INFERIOR_PID_ID:
			if (stack) {
				check->tpe->pid = (pid_t)top;
				ret = 1;
				goto out;
			}
			break;
		}

		if (!var->u.hooks || (var->u.hooks
				      && !var->u.hooks->agent_set_val)) {
			printk(KERN_WARNING "Action try to set special TSV %d that cannot be get.\n",
			       arg);
			goto out;
		}
		break;

#ifdef GTP_PERF_EVENTS
	case gtp_var_perf_event_per_cpu:
	case gtp_var_perf_event: {
			struct gtp_var_pe	*pe = gtp_var_get_pe(var);

			if (pe->ptid != pe_tv_en
			    && pe->ptid != pe_tv_val) {
				printk(KERN_WARNING "Action try to set perf event TSV %d that cannot be set.\n",
				arg);
				goto out;
			}
			if (var->type == gtp_var_perf_event) {
				flags = 2;
			}
		}
		break;
#endif

	case gtp_var_normal:
		flags = 2;
		break;
	}

	ret = gtp_check_var_add(check, pc, arg, flags);
	if (ret)
		goto out;
	ret = -EINVAL;

	/* Change the num of var to the num of gtp_var_array.
	   It will make this insn speed up. */
	arg = gtp_var_array_find_num(var);
	if (arg < 0) {
		printk(KERN_WARNING "Action try to set TSV %d that does't inside the gtp var array.\n",
		       var->num);
		goto out;
	}
	check->ebuf[pc - 2] = (uint8_t)(arg >> 8);
	check->ebuf[pc - 1] = (uint8_t)(arg & 0xff);
	/* Handle GTP_VAR_CURRENT_ID, it need return 2.  */
	if (is_current)
		ret = 2;
	else
		ret = 0;

out:
	return ret;
}

/* This is the first check.
   1. Get the max size of stack need (sp_max).
      Check if it bigger than SP_MAX.
   2. Check TSV.
      Check if normal TSV id is right.
      Check special TSV, if need change insn code to op_special_getv,
      op_special_setv or op_special_tracev.
   3. If this is loop, change ae->type to 0xff and return.  */

static int
gtp_check_x_simple(struct gtp_entry *tpe, struct action *ae, int step)
{
	int			ret = -EINVAL;
	unsigned int		pc = 0, sp = 0;
	struct gtp_x_if_goto	*glist = NULL, *gtmp;
	uint8_t			*ebuf = ae->u.exp.buf;
	int			last_trace_pc = -1;
	unsigned int		sp_max = 0;
	ULONGEST		top = 0;
	ULONGEST		stack_space[STACK_MAX + 10];
	ULONGEST		*stack = stack_space;
	/* If true, the actions set $current.  */
	int			is_set_current = 0;
	struct gtp_check_s	check;

	gtp_check_init(tpe, ae, ebuf, &check);
reswitch:
	while (pc < ae->u.exp.size) {
#ifdef GTP_DEBUG
		printk(GTP_DEBUG "gtp_check_x_simple: cmd %u %x\n", pc,
		       ebuf[pc]);
#endif
		if (stack) {
			int	arg;

			switch (ebuf[pc]) {
			/* add */
			case 0x02:
				if (sp)
					top += stack[sp - 1];
				break;

			/* sub */
			case 0x03:
				if (sp)
					top = stack[sp - 1] - top;
				break;

			/* mul */
			case 0x04:
				if (sp)
					top *= stack[sp - 1];
				break;

#ifndef CONFIG_MIPS
			/* div_signed */
			case 0x05:
				if (top && sp) {
					LONGEST l = (LONGEST) stack[sp - 1];
					do_div(l, (LONGEST) top);
					top = l;
				} else if (top == 0) {
					printk(KERN_WARNING "gtp_check_x_simple: div_signed "
							    "0 in %d.\n", pc);
					goto release_out;
				}
				break;

			/* div_unsigned */
			case 0x06:
				if (top && sp) {
					ULONGEST ul = stack[sp - 1];
					do_div(ul, top);
					top = ul;
				} else if (top == 0) {
					printk(KERN_WARNING "gtp_check_x_simple: div_unsigned "
							    "0 in %d.\n", pc);
					goto release_out;
				}
				break;

			/* rem_signed */
			case 0x07:
				if (top && sp) {
					LONGEST l1 = (LONGEST) stack[sp - 1];
					LONGEST l2 = (LONGEST) top;
					top = do_div(l1, l2);
				} else if (top == 0) {
					printk(KERN_WARNING "gtp_check_x_simple: rem_signed "
							    "0 in %d.\n", pc);
					goto release_out;
				}
				break;

			/* rem_unsigned */
			case 0x08:
				if (top && sp) {
					ULONGEST ul1 = stack[sp - 1];
					ULONGEST ul2 = top;
					top = do_div(ul1, ul2);
				} else if (top == 0) {
					printk(KERN_WARNING "gtp_check_x_simple: rem_unsigned "
							    "0 in %d.\n", pc);
					goto release_out;
				}
				break;
#endif
			/* lsh */
			case 0x09:
				if (sp)
					top = stack[sp - 1] << top;
				break;

			/* rsh_signed */
			case 0x0a:
				if (sp)
					top = ((LONGEST) stack[sp - 1]) >> top;
				break;

			/* rsh_unsigned */
			case 0x0b:
				if (sp)
					top = stack[sp - 1] >> top;
				break;

			/* log_not */
			case 0x0e:
				top = !top;
				break;

			/* bit_and */
			case 0x0f:
				if (sp)
					top &= stack[sp - 1];
				break;

			/* bit_or */
			case 0x10:
				if (sp)
					top |= stack[sp - 1];
				break;

			/* bit_xor */
			case 0x11:
				if (sp)
					top ^= stack[sp - 1];
				break;

			/* bit_not */
			case 0x12:
				top = ~top;
				break;

			/* equal */
			case 0x13:
				if (sp)
					top = (stack[sp - 1] == top);
				break;

			/* less_signed */
			case 0x14:
				if (sp)
					top = (((LONGEST) stack[sp - 1])
						< ((LONGEST) top));
				break;

			/* less_unsigned */
			case 0x15:
				if (sp)
					top = (stack[sp - 1] < top);
				break;

			/* ext */
			case 0x16:
 				arg = ebuf[pc + 1];
				if (arg < (sizeof(LONGEST)*8)) {
					LONGEST mask = 1 << (arg - 1);
					top &= ((LONGEST) 1 << arg) - 1;
					top = (top ^ mask) - mask;
				}
				break;

			/* const8 */
			case 0x22:
				stack[sp + 1] = top;
				top = ebuf[pc + 1];
				break;

			/* const16 */
			case 0x23:
				stack[sp + 1] = top;
				top = ebuf[pc + 1];
				top = (top << 8) + ebuf[pc + 2];
				break;

			/* const32 */
			case 0x24:
				stack[sp + 1] = top;
				top = ebuf[pc + 1];
				top = (top << 8) + ebuf[pc + 2];
				top = (top << 8) + ebuf[pc + 3];
				top = (top << 8) + ebuf[pc + 4];
				break;

			/* const64 */
			case 0x25:
				stack[sp + 1] = top;
				top = ebuf[pc + 1];
				top = (top << 8) + ebuf[pc + 2];
				top = (top << 8) + ebuf[pc + 3];
				top = (top << 8) + ebuf[pc + 4];
				top = (top << 8) + ebuf[pc + 5];
				top = (top << 8) + ebuf[pc + 6];
				top = (top << 8) + ebuf[pc + 7];
				top = (top << 8) + ebuf[pc + 8];
				break;

			/* dup */
			case 0x28:
				stack[sp + 1] = top;
				break;

			/* pop */
			case 0x29:
				if (sp)
					top = stack[sp - 1];
				break;

			/* zero_ext */
			case 0x2a:
				arg = ebuf[pc + 1];
				if (arg < (sizeof(LONGEST)*8))
					top &= ((LONGEST) 1 << arg) - 1;
				break;

			/* swap */
			case 0x2b:
				if (sp) {
					stack[sp] = top;
					top = stack[sp - 1];
					stack[sp - 1] = stack[sp];
				}
				break;

			/* trace */
			case 0x0c:
			/* trace_quick */
			case 0x0d:
			/* ref8 */
			case 0x17:
			/* ref16 */
			case 0x18:
			/* ref32 */
			case 0x19:
			/* ref64 */
			case 0x1a:
			/* if_goto */
			case 0x20:
			/* reg */
			case 0x26:
			/* getv */
			case 0x2c:
				stack = NULL;
				break;
			}
		}
		switch (ebuf[pc++]) {
		/* add */
		case 0x02:
		/* sub */
		case 0x03:
		/* mul */
		case 0x04:
		/* lsh */
		case 0x09:
		/* rsh_signed */
		case 0x0a:
		/* rsh_unsigned */
		case 0x0b:
		/* bit_and */
		case 0x0f:
		/* bit_or */
		case 0x10:
		/* bit_xor */
		case 0x11:
		/* equal */
		case 0x13:
		/* less_signed */
		case 0x14:
		/* less_unsigned */
		case 0x15:
		/* pop */
		case 0x29:
		/* swap */
		case 0x2b:
			if (sp < 1) {
				printk(KERN_WARNING "gtp_check_x_simple: stack "
						    "overflow in %d.\n",
				       pc - 1);
				goto release_out;
			} else {
				if (ebuf[pc - 1] != 0x2b)
					sp--;
			}
			break;

		/* trace */
		case 0x0c:
			if (tpe->flags & GTP_ENTRY_FLAGS_HAVE_PRINTK)
				last_trace_pc = pc - 1;

			if (sp < 2) {
				printk(KERN_WARNING "gtp_check_x_simple: stack "
						    "overflow in %d.\n",
				       pc - 1);
				goto release_out;
			} else
				sp -= 2;
			break;

		/* log_not */
		case 0x0e:
		/* bit_not */
		case 0x12:
		/* ref8 */
		case 0x17:
		/* ref16 */
		case 0x18:
		/* ref32 */
		case 0x19:
		/* ref64 */
		case 0x1a:
			break;

		/* dup */
		case 0x28:
			sp++;
			if (sp_max < sp)
				sp_max = sp;
			break;

		/* const8 */
		case 0x22:
			sp++;
			if (sp_max < sp)
				sp_max = sp;
		/* ext */
		case 0x16:
		/* zero_ext */
		case 0x2a:
			if (pc >= ae->u.exp.size)
				goto release_out;
			pc++;
			break;

		/* trace_quick */
		case 0x0d:
			if (tpe->flags & GTP_ENTRY_FLAGS_HAVE_PRINTK)
				last_trace_pc = pc - 1;

			if (pc >= ae->u.exp.size)
				goto release_out;
			pc++;
			break;

		/* const16 */
		case 0x23:
		/* reg */
		case 0x26:
			if (pc + 1 >= ae->u.exp.size)
				goto release_out;
			pc += 2;

			sp++;
			if (sp_max < sp)
				sp_max = sp;
			break;

		/* const32 */
		case 0x24:
			if (pc + 3 >= ae->u.exp.size)
				goto release_out;
			pc += 4;

			sp++;
			if (sp_max < sp)
				sp_max = sp;
			break;

		/* const64 */
		case 0x25:
			if (pc + 7 >= ae->u.exp.size)
				goto release_out;
			pc += 8;

			sp++;
			if (sp_max < sp)
				sp_max = sp;
			break;

		/* if_goto */
		case 0x20:
			if (tpe->flags & GTP_ENTRY_FLAGS_HAVE_PRINTK) {
				printk(KERN_WARNING "If_goto action doesn't"
				       "support printk.\n");
				goto release_out;
			}
			if (pc + 1 >= ae->u.exp.size)
				goto release_out;

			{
				unsigned int	dpc = (ebuf[pc] << 8)
						      + ebuf[pc + 1];

				if (dpc < pc) {
					/* This action X include loop. */
					if (is_set_current)
						printk(KERN_WARNING "KGTP: cannot set $current inside loop.\n");
					else {
						ae->type = 0xff;
						ret = 0;
					}
					goto release_out;
				}

				if (!gtp_x_if_goto_add(&glist, dpc, sp)) {
					ret = -ENOMEM;
					goto release_out;
				}
			}

			pc += 2;
			break;

		/* goto */
		case 0x21:
			if (pc + 1 >= ae->u.exp.size)
				goto release_out;

			{
				unsigned int	dpc = (ebuf[pc] << 8)
						      + ebuf[pc + 1];

				if (dpc < pc) {
					/* This action X include loop. */
					if (is_set_current)
						printk(KERN_WARNING "KGTP: cannot set $current inside loop.\n");
					else {
						ae->type = 0xff;
						ret = 0;
					}
					goto release_out;
				}

				pc = dpc;
			}
			break;

		/* end */
		case 0x27:
			goto out;
			break;

		/* getv */
		case 0x2c: {
				int lret = gtp_check_getv(&check, step, pc);
				if (lret != 0) {
					ret = lret;
					goto release_out;
				}
				pc += 2;

				sp++;
				if (sp_max < sp)
					sp_max = sp;
			}
			break;

		/* setv */
		case 0x2d: {
				int lret = gtp_check_setv(&check, step, pc,
							  0, stack, top);
				if (lret == 1 || lret < 0) {
					ret = lret;
					goto release_out;
				}
				if (lret == 2)
					is_set_current = 1;
				pc += 2;
			}
			break;

		/* tracev */
		case 0x2e: {
				int	lret;

				if (tpe->flags & GTP_ENTRY_FLAGS_HAVE_PRINTK)
					last_trace_pc = pc - 1;

				lret = gtp_check_getv(&check, step, pc);
				if (lret != 0) {
					ret = lret;
					goto release_out;
				}
				pc += 2;
			}
			break;

		/* div_signed */
		case 0x05:
		/* div_unsigned */
		case 0x06:
		/* rem_signed */
		case 0x07:
		/* rem_unsigned */
		case 0x08:
#ifdef CONFIG_MIPS
			/* XXX, mips don't have 64 bit div.  */
			goto release_out;
#endif
			if (sp < 1) {
				printk(KERN_WARNING "gtp_check_x_simple: stack "
						    "overflow in %d.\n",
				       pc - 1);
				goto release_out;
			} else
				sp--;
			break;

		/* float */
		case 0x01:
		/* ref_float */
		case 0x1b:
		/* ref_double */
		case 0x1c:
		/* ref_long_double */
		case 0x1d:
		/* l_to_d */
		case 0x1e:
		/* d_to_l */
		case 0x1f:
		/* trace16 */
		case 0x30:
		default:
			goto release_out;
			break;
		}
	}
	goto release_out;

out:
#ifdef GTP_DEBUG
	printk(GTP_DEBUG "sp_max = %d\n", sp_max);
#endif
	if (sp_max >= STACK_MAX) {
		printk(KERN_WARNING "gtp_check_x_simple: stack overflow, "
				    "current %d, max %d.\n",
		       sp_max, STACK_MAX);
		goto release_out;
	}
	if (glist) {
		pc = glist->ip;
		sp = glist->sp;
		gtmp = glist;
		glist = glist->next;
		kfree(gtmp);
		goto reswitch;
	}
	if (is_set_current)
		ret = 2;
	else
		ret = 0;
#ifdef GTP_DEBUG
	printk(GTP_DEBUG "gtp_check_x_simple: Code is OK. sp_max is %d.\n",
	       sp_max);
#endif

release_out:
	while (glist) {
		gtmp = glist;
		glist = glist->next;
		kfree(gtmp);
	}
	gtp_check_release(&check);

	if (!is_set_current && (tpe->flags & GTP_ENTRY_FLAGS_HAVE_PRINTK)
	    && last_trace_pc > -1) {
		/* Set the last trace code to printk code.  */
		switch (ebuf[last_trace_pc]) {
		/* trace */
		case 0x0c:
			ebuf[last_trace_pc] = op_trace_printk;
			break;
		/* trace_quick */
		case 0x0d:
			ebuf[last_trace_pc] = op_trace_quick_printk;
			break;
		/* tracev */
		case 0x2e:
			ebuf[last_trace_pc] = op_tracev_printk;
			break;
		}
	}

	return ret;
}

/* The second check.
   Special check for loop.
   Different with gtp_check_x_simple is it will not check sp_max.  */

static int
gtp_check_x_loop(struct gtp_entry *tpe, struct action *ae, int step)
{
	int			ret = -EINVAL;
	unsigned int		pc = 0;
	struct gtp_x_loop	*glist = NULL, *gtmp;
	uint8_t			*ebuf = ae->u.exp.buf;
	struct gtp_check_s	check;

	printk(KERN_WARNING "Action of tracepoint %d have loop.\n",
	       (int)tpe->num);

	gtp_check_init(tpe, ae, ebuf, &check);
	tpe->flags &= ~GTP_ENTRY_FLAGS_HAVE_PRINTK;
reswitch:
	while (pc < ae->u.exp.size) {
#ifdef GTP_DEBUG
		printk(GTP_DEBUG "gtp_check_x_loop: cmd %x\n", ebuf[pc]);
#endif
		switch (ebuf[pc++]) {
		/* add */
		case 0x02:
			ebuf[pc - 1] = op_check_add;
			break;
		/* sub */
		case 0x03:
			ebuf[pc - 1] = op_check_sub;
			break;
		/* mul */
		case 0x04:
			ebuf[pc - 1] = op_check_mul;
			break;
		/* lsh */
		case 0x09:
			ebuf[pc - 1] = op_check_lsh;
			break;
		/* rsh_signed */
		case 0x0a:
			ebuf[pc - 1] = op_check_rsh_signed;
			break;
		/* rsh_unsigned */
		case 0x0b:
			ebuf[pc - 1] = op_check_rsh_unsigned;
			break;
		/* bit_and */
		case 0x0f:
			ebuf[pc - 1] = op_check_bit_and;
			break;
		/* bit_or */
		case 0x10:
			ebuf[pc - 1] = op_check_bit_or;
			break;
		/* bit_xor */
		case 0x11:
			ebuf[pc - 1] = op_check_bit_xor;
			break;
		/* equal */
		case 0x13:
			ebuf[pc - 1] = op_check_equal;
			break;
		/* less_signed */
		case 0x14:
			ebuf[pc - 1] = op_check_less_signed;
			break;
		/* less_unsigned */
		case 0x15:
			ebuf[pc - 1] = op_check_less_unsigned;
			break;
		/* pop */
		case 0x29:
			ebuf[pc - 1] = op_check_pop;
			break;
		/* swap */
		case 0x2b:
			ebuf[pc - 1] = op_check_swap;
			break;

		/* trace */
		case 0x0c:
			ebuf[pc - 1] = op_check_trace;
			break;

		/* log_not */
		case 0x0e:
		/* bit_not */
		case 0x12:
		/* ref8 */
		case 0x17:
		/* ref16 */
		case 0x18:
		/* ref32 */
		case 0x19:
		/* ref64 */
		case 0x1a:
		/* dup */
		case 0x28:
			break;

		/* const8 */
		case 0x22:
		/* ext */
		case 0x16:
		/* zero_ext */
		case 0x2a:
		/* trace_quick */
		case 0x0d:
			if (pc >= ae->u.exp.size)
				goto release_out;
			pc++;
			break;

		/* const16 */
		case 0x23:
		/* reg */
		case 0x26:
			if (pc + 1 >= ae->u.exp.size)
				goto release_out;
			pc += 2;
			break;

		/* const32 */
		case 0x24:
			if (pc + 3 >= ae->u.exp.size)
				goto release_out;
			pc += 4;
			break;

		/* const64 */
		case 0x25:
			if (pc + 7 >= ae->u.exp.size)
				goto release_out;
			pc += 8;
			break;

		/* if_goto */
		case 0x20:
		case op_check_if_goto:
			ebuf[pc - 1] = op_check_if_goto;

			if (pc + 1 >= ae->u.exp.size)
				goto release_out;

			gtmp = gtp_x_loop_find(glist, pc);
			if (gtmp) {
				if (gtmp->non_goto_done)
					goto out;
				else {
					gtmp->non_goto_done = 1;
					pc += 2;
				}
			} else {
				if (!gtp_x_loop_add(&glist, pc, 0)) {
					ret = -ENOMEM;
					goto release_out;
				}
				pc = (ebuf[pc] << 8) + ebuf[pc + 1];
			}
			break;

		/* goto */
		case 0x21:
			if (pc + 1 >= ae->u.exp.size)
				goto release_out;

			gtmp = gtp_x_loop_find(glist, pc);
			if (gtmp)
				goto out;
			else {
				if (!gtp_x_loop_add(&glist, pc, 1)) {
					ret = -ENOMEM;
					goto release_out;
				}
			}

			pc = (ebuf[pc] << 8) + (ebuf[pc + 1]);
			break;

		/* end */
		case 0x27:
			goto out;
			break;

		/* getv */
		case 0x2c:
		/* tracev */
		case 0x2e: {
				int lret = gtp_check_getv(&check, step, pc);
				if (lret != 0) {
					ret = lret;
					goto release_out;
				}
				pc += 2;
			}
			break;

		/* setv */
		case 0x2d: {
				int lret = gtp_check_setv(&check, step, pc,
							  1, NULL, 0);
				if (lret == 1 || lret < 0) {
					ret = lret;
					goto release_out;
				}
				if (lret == 2) {
					printk(KERN_WARNING "KGTP: cannot set $current inside loop.\n");
					goto release_out;
				}
			}
			break;

		/* div_signed */
		case 0x05:
#ifdef CONFIG_MIPS
			/* XXX, mips don't have 64 bit div.  */
			printk(KERN_WARNING "MIPS don't have 64 bit div.\n");
			goto release_out;
#endif
			ebuf[pc - 1] = op_check_div_signed;
			break;
		/* div_unsigned */
		case 0x06:
#ifdef CONFIG_MIPS
			/* XXX, mips don't have 64 bit div.  */
			printk(KERN_WARNING "MIPS don't have 64 bit div.\n");
			goto release_out;
#endif
			ebuf[pc - 1] = op_check_div_unsigned;
			break;
		/* rem_signed */
		case 0x07:
#ifdef CONFIG_MIPS
			/* XXX, mips don't have 64 bit div.  */
			printk(KERN_WARNING "MIPS don't have 64 bit div.\n");
			goto release_out;
#endif
			ebuf[pc - 1] = op_check_rem_signed;
			break;
		/* rem_unsigned */
		case 0x08:
#ifdef CONFIG_MIPS
			/* XXX, mips don't have 64 bit div.  */
			printk(KERN_WARNING "MIPS don't have 64 bit div.\n");
			goto release_out;
#endif
			ebuf[pc - 1] = op_check_rem_unsigned;
			break;

		/* float */
		case 0x01:
		/* ref_float */
		case 0x1b:
		/* ref_double */
		case 0x1c:
		/* ref_long_double */
		case 0x1d:
		/* l_to_d */
		case 0x1e:
		/* d_to_l */
		case 0x1f:
		/* trace16 */
		case 0x30:
		default:
			goto release_out;
			break;
		}
	}
	goto release_out;

out:
	for (gtmp = glist; gtmp; gtmp = gtmp->next) {
		if (!gtmp->non_goto_done)
			break;
	}
	if (gtmp) {
		pc = gtmp->addr + 2;
		gtmp->non_goto_done = 1;
		goto reswitch;
	}
	ret = 0;

release_out:
	while (glist) {
		gtmp = glist;
		glist = glist->next;
		kfree(gtmp);
	}
	gtp_check_release(&check);

	return ret;
}

static int
gtp_check_x(struct gtp_entry *tpe, struct action *ae, int step)
{
	int	ret = gtp_check_x_simple(tpe, ae, step);

	if (ret != 0 || ae->type == 'X')
		return ret;

	return gtp_check_x_loop(tpe, ae, step);
}

#if defined(GTP_FTRACE_RING_BUFFER) || defined(GTP_RB)
static void
gtpframe_pipe_wq_wake_up(unsigned long data)
{
	/* About why KGTP use a tasklet to wake up:
	   When a tracepoint that is inserted to "schedule" function
	   call wake up inside its handler, the kernel maybe will deadlock.
	   "tasklet_schedule" is a small function and it can be
	   very easy controlled to wake up softirqd or not
	   (preempt_count_add(HARDIRQ_OFFSET) can control it).
	   So KGTP just use it to wake up a task.  */
	wake_up_interruptible_nr(&gtpframe_pipe_wq, 1);
}
#endif

static void
gtp_wq_add_work(unsigned long data)
{
	/* Same with prev function, queue_work will wake up sometimes.  */
	queue_work(gtp_wq, (struct work_struct *)data);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0)) && defined CONFIG_UPROBES
int
gtp_uprobe_register(struct gtp_entry *tpe)
{
	int			ret = -EINVAL;
	struct task_struct	*tsk;
	struct mm_struct	*mm;
	struct vm_area_struct	*vma;

	if ((tpe->flags & GTP_ENTRY_FLAGS_CURRENT_TASK)
	    || (tpe->flags & GTP_ENTRY_FLAGS_CURRENT_TASK)) {
		printk(KERN_WARNING "KGTP: cannot use $current or $kret in actions of tracepoint %d %p.\n",
		       (int)tpe->num, (void *)(CORE_ADDR)tpe->addr);
		return -EINVAL;
	}

	if (tpe->disable)
		return 0;

	/* Get mm.  */
	rcu_read_lock();
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))
	tsk = pid_task(find_vpid(tpe->pid), PIDTYPE_PID);
#else
	tsk = find_task_by_pid(tpe->pid);
#endif
	if (!tsk) {
		rcu_read_unlock();
		return -ESRCH;
	}
	mm = get_task_mm(tsk);
	rcu_read_unlock();
	if (mm == NULL)
		return -ESRCH;
	down_read(&mm->mmap_sem);

	/* Get inode "file_inode(vma->vm_file)" and
	   offset "tpe->addr - vma->vm_start".  */
	vma = find_vma(mm, tpe->addr);
	if (vma == NULL)
		goto out;
	if (tpe->addr < vma->vm_start)
		goto out;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0))
	tpe->u.up.inode = file_inode(vma->vm_file);
#else
	tpe->u.up.inode = vma->vm_file->f_path.dentry->d_inode;
#endif
	tpe->u.up.offset = tpe->addr - vma->vm_start;
	tpe->u.up.uc.handler = gtp_up_handler;

	ret = 0;
out:
	up_read(&mm->mmap_sem);
	mmput(mm);

	if (ret == 0)
		ret = uprobe_register (tpe->u.up.inode, tpe->u.up.offset, &tpe->u.up.uc);

	return ret;
}
#endif

static int
gtp_gdbrsp_qtstart(void)
{
	int			ret = -EINVAL;
	int			cpu;
	struct gtp_entry	*tpe;
	int			i;
	struct gtp_var		*var;
	struct list_head	*cur;
#ifdef CONFIG_X86
	unsigned long		flags;
#endif

#ifdef GTP_DEBUG
	printk(GTP_DEBUG "gtp_gdbrsp_qtstart\n");
#endif

	if (gtp_start)
		return -EBUSY;

#ifdef GTP_FTRACE_RING_BUFFER
	if (!tracing_is_on()) {
		printk(KERN_WARNING "qtstart: Ring buffer is off.  Please use "
		       "command "
		       "\"echo 1 > /sys/kernel/debug/tracing/tracing_on\" "
		       "open it.\n");
		return -EIO;
	}
#endif

	/* Setup the gtp_var_array.
	   It must be setup before action because action need it.  */
	gtp_var_array = kcalloc(gtp_var_num, sizeof(struct gtp_var *),
				GFP_KERNEL);
	if (!gtp_var_array)
		return -ENOMEM;
	i = 0;
	list_for_each(cur, &gtp_var_list) {
		var = list_entry(cur, struct gtp_var, node);
		gtp_var_array[i] = var;
		i++;
	}
#ifdef GTP_RB
	var = gtp_var_find_num(GTP_STEP_ID_ID);
	if (var == NULL) {
		printk(KERN_WARNING "KGTP: cannot get $step_id.\n");
		return -EINVAL;
	}
	gtp_var_array_step_id_id = gtp_var_array_find_num(var);
	if (gtp_var_array_step_id_id < 0) {
		printk(KERN_WARNING "KGTP: cannot get $step_id.\n");
		return -EINVAL;
	}
#endif

	/* Check and setup actions.  */
	for (tpe = gtp_list; tpe; tpe = tpe->next) {
		struct list_head	*head, *cur, *tmp;
		struct action		*ae;

		/* Check tpe->step for old version GDB without patch
		   http://sourceware.org/ml/gdb-cvs/2013-04/msg00044.html  */
		if (tpe->step != 0 && list_empty(&tpe->step_action_list))
			tpe->step = 0;

		/* Check cond.  */
		if (tpe->cond) {
			int	lret = gtp_check_x(tpe, tpe->cond, 0);
			if (lret == 1) {
				printk(KERN_WARNING "KGTP: cannot set special TSV inside condition.\n");
				goto out;
			} else if (lret == 2) {
				printk(KERN_WARNING "KGTP: cannot set $current inside condition.\n");
				goto out;
			} else if (lret != 0) {
				ret = lret;
				printk(KERN_WARNING "KGTP: gtp_check_x get error %d\n",
				       lret);
				goto out;
			}
		}

		/* Check X.  */
		head = &tpe->action_list;
next_list:
		list_for_each_safe(cur, tmp, head) {
			ae = list_entry(cur, struct action, node);
			if (ae->type == 'X' || ae->type == 0xff) {
				int lret = gtp_check_x(tpe, ae,
						       (head == &tpe->step_action_list));
				if (lret == 1) {
					/* Remove ae from action_list.  */
					list_del(&ae->node);
					gtp_action_release(ae);
				} else if (lret == 2) {
					/* Set this actions to get_regs.  */
					gtp_action_release(tpe->get_regs);
					list_del(&ae->node);
					tpe->get_regs = ae;
				} else if (lret != 0) {
					printk(KERN_WARNING "KGTP: gtp_check_x get error %d\n",
					       lret);
					goto out;
				}
			}
		}
		if (head == &tpe->action_list) {
			/* Begin to check step_action_list.  */
			head = &tpe->step_action_list;
			goto next_list;
		}

		/* Check the tracepoint that have printk.  */
		if (tpe->flags & GTP_ENTRY_FLAGS_HAVE_PRINTK) {
			struct gtpsrc	*src, *srctail = NULL;

			list_for_each_safe(cur, tmp, &tpe->action_list) {
				ae = list_entry(cur, struct action, node);
				switch (ae->type) {
				case 'R':
					/* Remove it. */
					list_del(&ae->node);
					gtp_action_release(ae);
					break;
				case 'M':
					printk(KERN_WARNING "qtstart: action "
					       "of tp %d %p is not right.  "
					       "Please put global variable to "
					       "trace state variable "
					       "$printk_tmp before print it.\n",
					       (int)tpe->num,
					       (void *)(CORE_ADDR)tpe->addr);
					ret = -EINVAL;
					goto out;
					break;
				}
			}

			for (src = tpe->src; src; src = src->next) {
				int		i;
				char		str[strlen(src->src) >> 1];
				char		*var = NULL;
				ULONGEST	num;
				char		tmp[30];
				struct gtpsrc	*ksrc;

#ifdef GTP_DEBUG
				printk(GTP_DEBUG "gtp_gdbrsp_qtstart: action "
						 "%s\n", src->src);
#endif
				/* Get the action in str.  */
				if (strncmp("cmd:0:", src->src,
					    strlen("cmd:0:")))
					continue;
				var = hex2ulongest(src->src + 6, &num);
				if (var[0] == '\0')
					goto out;
				var++;
				hex2string(var, str);
				if (strlen(str) != num)
					goto out;
#ifdef GTP_DEBUG
				printk(GTP_DEBUG "gtp_gdbrsp_qtstart: action "
						 "command %s\n", str);
#endif

				if (strncmp("collect ", str,
					    strlen("collect ")))
					continue;
				for (i = strlen("collect "); ; i++) {
					if (str[i] != ' ') {
						var = str + i;
						break;
					}
					if (str[i] == '\0')
						break;
				}
				if (!var) {
					printk(KERN_WARNING "qtstart: cannot "
							    "get the var name "
							    "from tp %d %p"
							    "command %s.\n",
					       (int)tpe->num,
					       (void *)(CORE_ADDR)tpe->addr,
					       str);
					goto out;
				}
				if (strcmp(var, "$args") == 0
				    || strcmp(var, "$local") == 0) {
					printk(KERN_WARNING "qtstart: cannot "
							    "print $args and "
							    "$local.\n");
					goto out;
				}
				if (strcmp(var, "$reg") == 0)
					continue;

				if (var[0] == '$') {
					/* Check if var is TSV that cannot get in action.  */
					char		src[3 + strlen(var) * 2];
					struct gtp_var	*tsv;

					strcpy(src, "1:");
					string2hex (var + 1, src + 2);
					tsv = gtp_var_find_src(src);
					if (tsv
					    && tsv->type == gtp_var_special
					    && !tsv->u.hooks->agent_get_val)
						continue;
				}

				ksrc = kmalloc(sizeof(struct gtpsrc),
					       GFP_KERNEL);
				if (ksrc == NULL) {
					ret = -ENOMEM;
					goto out;
				}
				ksrc->next = NULL;

				snprintf(tmp, 30, "gtp %d %p:", (int)tpe->num,
					 (void *)(CORE_ADDR)tpe->addr);
				ksrc->src = kmalloc(strlen(tmp)
						   + strlen(var) + 2,
						   GFP_KERNEL);
				if (ksrc->src == NULL) {
					kfree(ksrc);
					ret = -ENOMEM;
					goto out;
				}
				sprintf(ksrc->src, "%s%s=", tmp, var);

#ifdef GTP_DEBUG
				printk(GTP_DEBUG "gtp_gdbrsp_qtstart: new "
						 "printk var %s\n", ksrc->src);
#endif

				if (tpe->printk_str)
					srctail->next = ksrc;
				else
					tpe->printk_str = ksrc;
				srctail = ksrc;
			}
		}
	}

#if defined(GTP_FTRACE_RING_BUFFER)			\
    && (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,39))	\
    && !defined(GTP_SELF_RING_BUFFER)
	if (gtp_frame && gtp_circular_is_changed) {
		ring_buffer_free(gtp_frame);
		gtp_frame = NULL;
	}
	gtp_circular_is_changed = 0;
#endif

#ifdef GTP_RB
	if (GTP_RB_PAGE_IS_EMPTY) {
		if (gtp_rb_page_alloc(GTP_FRAME_SIZE) != 0) {
			ret = -ENOMEM;
			goto out;
		}
#endif
#if defined(GTP_FRAME_SIMPLE) || defined(GTP_FTRACE_RING_BUFFER)
	if (!gtp_frame) {
#ifdef GTP_FRAME_SIMPLE
		gtp_frame = vmalloc(GTP_FRAME_SIZE);
#endif
#ifdef GTP_FTRACE_RING_BUFFER
		gtp_frame = ring_buffer_alloc(GTP_FRAME_SIZE,
					      gtp_circular ? RB_FL_OVERWRITE
							     : 0);
#endif
		if (!gtp_frame) {
			ret = -ENOMEM;
			goto out;
		}
#endif

		gtp_frame_reset();
	}

	for_each_possible_cpu(cpu) {
#ifdef CONFIG_X86
		per_cpu(rdtsc_current, cpu) = 0;
		per_cpu(rdtsc_offset, cpu) = 0;
#endif
		per_cpu(local_clock_current, cpu) = 0;
		per_cpu(local_clock_offset, cpu) = 0;
		per_cpu(gtp_handler_began, cpu) = 0;
	}

	gtp_start = 1;

#ifdef GTP_PERF_EVENTS
	/* Clear pc_pe_list.  */
	for_each_possible_cpu(cpu) {
		per_cpu(pc_pe_list, cpu) = NULL;
		per_cpu(pc_pe_list_all_disabled, cpu) = 1;
	}
	list_for_each(cur, &gtp_var_list) {
		struct gtp_var_perf_event	*pe;
		var = list_entry(cur, struct gtp_var, node);

		if (var->type != gtp_var_perf_event
		    && var->type != gtp_var_perf_event_per_cpu)
			continue;
		if (var->type == gtp_var_perf_event_per_cpu
		    && var->u.pc.cpu < 0)
			continue;
		pe = gtp_var_get_pe(var)->pe;
		if (pe->event)
			continue;

#ifdef GTP_DEBUG
		printk(GTP_DEBUG "gtp_gdbrsp_qtstart:"
				 "create perf_event CPU%d %d %d.\n",
		       (int)pe->cpu, (int)pe->attr.type, (int)pe->attr.config);
#endif

		/* Get event.  */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,1,0)) \
       || (RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(6,3))
		pe->event = perf_event_create_kernel_counter(&(pe->attr),
							     pe->cpu,
							     NULL, NULL,
							     NULL);
#elif (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)) \
       || (RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(6,1))
		pe->event =
			perf_event_create_kernel_counter(&(pe->attr),
							 pe->cpu,
							 NULL, NULL);
#else
		pe->event =
			perf_event_create_kernel_counter(&(pe->attr),
							 pe->cpu,
							 -1, NULL);
#endif
		if (IS_ERR(pe->event)) {
			int	ret = PTR_ERR(pe->event);

			printk(KERN_WARNING "gtp_gdbrsp_qtstart:"
			       "create perf_event CPU%d %d %d got error.\n",
			       (int)pe->cpu, (int)pe->attr.type,
			       (int)pe->attr.config);
			pe->event = NULL;
			gtp_gdbrsp_qtstop();
			return ret;
		}

		/* Add event to pc_pe_list.  */
		if (pe->cpu >= 0) {
			struct gtp_var_perf_event *ppl = per_cpu(pc_pe_list,
								 pe->cpu);
			if (ppl == NULL) {
				per_cpu(pc_pe_list, pe->cpu) = pe;
				pe->pc_next = NULL;
			} else {
				pe->pc_next = ppl;
				per_cpu(pc_pe_list,
					pe->cpu) = pe;
			}
			if (pe->en)
				per_cpu(pc_pe_list_all_disabled, pe->cpu)
					= 0;
		}
	}
#endif

#if defined(GTP_FTRACE_RING_BUFFER) || defined(GTP_RB)
	tasklet_init(&gtpframe_pipe_wq_tasklet, gtpframe_pipe_wq_wake_up, 0);
#endif

	/* Init tracepoint and do last tracepoint check. */
#ifdef CONFIG_X86
	if (gtp_have_step && gtp_have_watch_tracepoint) {
		printk(KERN_WARNING "KGTP: watch tracepoint cannot work together with while-stepping.\n");
		gtp_gdbrsp_qtstop();
		return -EINVAL;
	}
#endif

	gtp_start_last_errno = 0;
	for (tpe = gtp_list; tpe; tpe = tpe->next) {
		if ((tpe->flags & GTP_ENTRY_FLAGS_IS_KRETPROBE)
		    && (tpe->step || tpe->type != gtp_entry_kprobe
#ifdef CONFIG_X86
		    || gtp_have_step)) {
#else
		    )) {
#endif
			printk(KERN_WARNING "KGTP: $kret cannot use with while-stepping or watch.\n");
			gtp_gdbrsp_qtstop();
			return -EINVAL;
		}
		tpe->reason = gtp_stop_normal;
		if (tpe->flags & GTP_ENTRY_FLAGS_HAVE_PASS)
			atomic_set(&tpe->current_pass, tpe->pass);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
		tasklet_init(&tpe->enable_tasklet, gtp_wq_add_work,
			     (unsigned long)&tpe->enable_work);
		tasklet_init(&tpe->disable_tasklet, gtp_wq_add_work,
			     (unsigned long)&tpe->disable_work);
#endif
	}

#ifdef CONFIG_X86
	/* Start hwb.  */
	if (gtp_have_watch_tracepoint) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33))
		{
			struct perf_event_attr attr;

			hw_breakpoint_init(&attr);
			attr.bp_len = HW_BREAKPOINT_LEN_1;
			attr.bp_type = HW_BREAKPOINT_W;

			/* Register hw breakpoints.  */
			for (i = 0; i < HWB_NUM; i++) {
				perf_overflow_handler_t	triggered;
				/* Step 1: Set the id to addr and let following code check it.
				   Make sure the num of a hw. */
				attr.bp_addr = i;
				switch (i) {
				case 0:
					triggered = gtp_hw_breakpoint_0_handler;
					break;
				case 1:
					triggered = gtp_hw_breakpoint_1_handler;
					break;
				case 2:
					triggered = gtp_hw_breakpoint_2_handler;
					break;
				case 3:
					triggered = gtp_hw_breakpoint_3_handler;
					break;
				}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,1,0))
				breakinfo[i].pev = register_wide_hw_breakpoint(&attr, triggered, NULL);
#else
				breakinfo[i].pev = register_wide_hw_breakpoint(&attr, triggered);
#endif
				if (IS_ERR((void * __force)breakinfo[i].pev)) {
					printk(KERN_WARNING "KGTP: Could not allocate hw breakpoints.\n");
					breakinfo[i].pev = NULL;
					gtp_gdbrsp_qtstop();
					return PTR_ERR((void * __force)breakinfo[i].pev);
				}
			}

			/* Step 2: Make sure breakinfo[i] is which hw breakpoint and set
			   it to breakinfo[i].num.  */
			for (i = 0; i < HWB_NUM; i++) {
				unsigned long	num;

				gtp_get_debugreg(num, i);
				breakinfo[num].num = i;
			}
		}
#endif

		gtp_hwb_stop(NULL);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
		smp_call_function(gtp_hwb_stop, NULL, 1);
#else
		smp_call_function(gtp_hwb_stop, NULL, 0, 1);
#endif

		write_lock_irqsave(&gtp_hwb_lock, flags);
		INIT_LIST_HEAD(&gtp_hwb_used_list);
		INIT_LIST_HEAD(&gtp_hwb_unused_list);
		for (i = 0; i < HWB_NUM; i++) {
			gtp_hwb[i].num = i;
			gtp_hwb[i].watch = NULL;
			gtp_hwb_drx[i] = 0;
			list_add(&(gtp_hwb[i].node), &gtp_hwb_unused_list);
		}
		gtp_hwb_dr7 = GTP_HWB_DR7_DEF;

		gtp_hwb_sync_count = 0;
		for_each_possible_cpu(cpu) {
			per_cpu(gtp_hwb_sync_count_local, cpu) = gtp_hwb_sync_count;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
			cpumask_copy(&(per_cpu(gtp_hwb_sync_cpu_mask, cpu)),
				     cpu_online_mask);
			cpumask_clear_cpu(cpu, &(per_cpu(gtp_hwb_sync_cpu_mask, cpu)));
#endif
		}
		write_unlock_irqrestore(&gtp_hwb_lock, flags);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
		/* Register kprobe handler for IPI.  */
		memset (&gtp_ipi_kp, '\0', sizeof(gtp_ipi_kp));
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0))
		gtp_ipi_kp.symbol_name = "generic_smp_call_function_single_interrupt";
#else
		gtp_ipi_kp.symbol_name = "generic_smp_call_function_interrupt";
#endif
		gtp_ipi_kp.pre_handler = gtp_ipi_handler;
		ret = register_kprobe(&gtp_ipi_kp);
		if (ret) {
			printk(KERN_WARNING "KGTP: try to register handler on IPI got error.\n");
			gtp_gdbrsp_qtstop();
			return ret;
		}
#endif

		/* Register static watch.  */
		for (tpe = gtp_list; tpe; tpe = tpe->next) {
			struct gtp_hwb_s	arg;

			if (tpe->type == gtp_entry_kprobe || tpe->disable)
				continue;

			if (tpe->type == gtp_entry_watch_static) {
				arg.addr = tpe->addr;
				arg.size = tpe->u.watch.size;
				arg.type = tpe->u.watch.type;
				arg.trace_num = tpe->num;
				arg.trace_addr = tpe->addr;
				arg.watch = tpe;
				ret = gtp_register_hwb(&arg, 0);
				if (ret < 0) {
					printk(KERN_WARNING "gtp_gdbrsp_qtstart: register watchpoint %d %p got error.\n",
					(int)tpe->num, (void *)(CORE_ADDR)tpe->addr);
					if (gtp_start_ignore_error) {
						gtp_start_last_errno = (uint64_t)ret;
						continue;
					} else {
						gtp_gdbrsp_qtstop();
						return ret;
					}
				}
			}

			tpe->flags |= GTP_ENTRY_FLAGS_REG;
		}
	}
#endif

#ifdef CONFIG_X86
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33))
	if (gtp_have_step || gtp_have_watch_tracepoint) {
#else
	if (gtp_have_step) {
#endif
		/* Register notifier.  */
		ret = register_die_notifier(&gtp_notifier);
		if (ret) {
			gtp_gdbrsp_qtstop();
			return ret;
		}
	}
#endif

	/* Register kprobe.  */
	for (tpe = gtp_list; tpe; tpe = tpe->next) {
		if (tpe->type != gtp_entry_kprobe
		    && tpe->type != gtp_entry_uprobe)
			continue;

		tasklet_init(&tpe->u.kp.stop_tasklet, gtp_wq_add_work,
			     (unsigned long)&tpe->u.kp.stop_work);

		if (tpe->pid != gtpd_task->pid) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0)) && defined CONFIG_UPROBES
			tpe->type = gtp_entry_uprobe;
			ret = gtp_uprobe_register(tpe);
#else
			printk(KERN_WARNING "Cannot trace user program because Linux Kernel config doesn't open UPROBES.  Please open it in 'Kernel hacking->Tracers->Enable uprobes-based dynamic events' if you need it.\n");
			tasklet_kill(&tpe->u.kp.stop_tasklet);
			gtp_gdbrsp_qtstop();
			return -ENOSYS;
#endif
		} else {
#ifdef CONFIG_KPROBES
			/* This part is for Kprobe.  */
			tpe->type = gtp_entry_kprobe;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
			if (tpe->disable)
				tpe->u.kp.kpret.kp.flags |= KPROBE_FLAG_DISABLED;
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
			if (tpe->addr != 0) {
#else
			if (tpe->disable == 0 && tpe->addr != 0) {
#endif
				tpe->u.kp.kpret.kp.addr = (kprobe_opcode_t *) (CORE_ADDR)tpe->addr;
				if (tpe->flags & GTP_ENTRY_FLAGS_IS_KRETPROBE) {
					if (gtp_access_cooked_clock
#ifdef CONFIG_X86
					|| gtp_access_cooked_rdtsc
#endif
#ifdef GTP_PERF_EVENTS
					|| gtp_have_pc_pe
#endif
					)
						tpe->u.kp.kpret.handler = gtp_kp_ret_handler_plus;
					else
						tpe->u.kp.kpret.handler = gtp_kp_ret_handler;

					ret = register_kretprobe(&tpe->u.kp.kpret);
				} else {
					if (gtp_access_cooked_clock
#ifdef CONFIG_X86
					|| gtp_access_cooked_rdtsc
#endif
#ifdef GTP_PERF_EVENTS
					|| gtp_have_pc_pe
#endif
					) {
#ifdef CONFIG_X86
						if (tpe->step || gtp_have_step) {
#else
						if (tpe->step) {
#endif
							tpe->u.kp.kpret.kp.pre_handler = gtp_kp_pre_handler_plus_step;
							tpe->u.kp.kpret.kp.post_handler = gtp_kp_post_handler_plus;
						} else
							tpe->u.kp.kpret.kp.pre_handler = gtp_kp_pre_handler_plus;
					} else {
						tpe->u.kp.kpret.kp.pre_handler = gtp_kp_pre_handler;
#ifdef CONFIG_X86
						if (tpe->step || gtp_have_step)
#else
						if (tpe->step)
#endif
							tpe->u.kp.kpret.kp.post_handler = gtp_kp_post_handler;
					}
					ret = register_kprobe(&tpe->u.kp.kpret.kp);
				}
			}
#else
			printk(KERN_WARNING "Cannot trace Linux kernel because Linux Kernel config doesn't open KPROBES.  Please open it in 'General setup->Kprobes' if you need it.\n");
			tasklet_kill(&tpe->u.kp.stop_tasklet);
			gtp_gdbrsp_qtstop();
			return -ENOSYS;
#endif
		}

		if (ret < 0) {
			printk(KERN_WARNING "KGTP: register tracepoint %d %p got error.\n",
			(int)tpe->num, (void *)(CORE_ADDR)tpe->addr);
			if (gtp_start_ignore_error) {
				gtp_start_last_errno = (uint64_t)ret;
				continue;
			} else {
				tasklet_kill(&tpe->u.kp.stop_tasklet);
				gtp_gdbrsp_qtstop();
				return ret;
			}
		}
		tpe->flags |= GTP_ENTRY_FLAGS_REG;
	}
	ret = 0;
out:
	if (ret != 0) {
		kfree(gtp_var_array);
		gtp_var_array = NULL;
	}
	return ret;
}

/* This part is for the "monitor disable".  */

struct gtp_disable_s {
	struct list_head	node;
	CORE_ADDR		addr;
};

static LIST_HEAD(gtp_disable_list);

static void
gtp_disable_release(void)
{
	struct gtp_disable_s	*d;
	struct list_head	*cur, *tmp;

	list_for_each_safe(cur, tmp, &gtp_disable_list) {
		d = list_entry(cur, struct gtp_disable_s, node);
		list_del(&d->node);
		kfree(d);
	}
}

static int
gtp_disable_find(CORE_ADDR addr)
{
	struct gtp_disable_s	*d;
	struct list_head	*cur;

	list_for_each(cur, &gtp_disable_list) {
		d = list_entry(cur, struct gtp_disable_s, node);
		if (d->addr == addr)
			return 1;
	}

	return 0;
}

static int
gtp_parse_x(struct gtp_entry *tpe, struct action *ae, char **pkgp)
{
	ULONGEST	size;
	int		ret = -EINVAL, i, h, l;
	char		*pkg = *pkgp;

	if (pkg[0] == '\0')
		goto out;
	pkg = hex2ulongest(pkg, &size);
	if (pkg[0] != ',')
		goto out;
	ae->u.exp.size = (unsigned int)size;
	pkg++;

	ae->u.exp.buf = kmalloc(ae->u.exp.size, GFP_KERNEL);
	if (!ae->u.exp.buf) {
		ret = -ENOMEM;
		goto out;
	}

	for (i = 0; i < ae->u.exp.size
		    && hex2int(pkg[0], &h) && hex2int(pkg[1], &l);
	     i++) {
#ifdef GTP_DEBUG
		printk(GTP_DEBUG "gtp_parse_x: %s %d %d\n", pkg, h, l);
#endif
		ae->u.exp.buf[i] = (h << 4) | l;
		pkg += 2;
#ifdef GTP_DEBUG
		printk(GTP_DEBUG "gtp_parse_x: %x\n", ae->u.exp.buf[i]);
#endif
	}
	if (i != ae->u.exp.size) {
		kfree(ae->u.exp.buf);
		goto out;
	}

	ae->u.exp.need_var_lock = 0;
	ret = 0;

out:
	*pkgp = pkg;
	return ret;
}

static int
gtp_gdbrsp_qtdp(char *pkg)
{
	int			addnew = 1;
	ULONGEST		num, addr;
	struct gtp_entry	*tpe;

#ifdef GTP_DEBUG
	printk(GTP_DEBUG "gtp_gdbrsp_qtdp: %s\n", pkg);
#endif

	if (gtp_start)
		return -EBUSY;

	if (pkg[0] == '-') {
		pkg++;
		addnew = 0;
	}

	/* Get num and addr.  */
	if (pkg[0] == '\0')
		return -EINVAL;
	pkg = hex2ulongest(pkg, &num);
	if (pkg[0] == '\0')
		return -EINVAL;
	pkg++;
	pkg = hex2ulongest(pkg, &addr);
	if (pkg[0] == '\0')
		return -EINVAL;
	pkg++;

	tpe = gtp_list_find(num, addr);
	if (addnew) {
		ULONGEST	ulongtmp;

		if (tpe)
			return -EINVAL;

		tpe = gtp_list_add(num, addr);
		if (tpe == NULL)
			return -ENOMEM;

		if (pkg[0] == '\0')
			return -EINVAL;
		if (pkg[0] == 'D')
			tpe->disable = 1;
		else if (gtp_disable_find((CORE_ADDR)addr))
			tpe->disable = 1;
		pkg++;

		/* Get step.  */
		if (pkg[0] == '\0')
			return -EINVAL;
		pkg++;
		pkg = hex2ulongest(pkg, &ulongtmp);
		if (pkg[0] == '\0')
			return -EINVAL;
#ifndef CONFIG_X86
		if (ulongtmp > 1) {
			printk(KERN_WARNING "KGTP in this ARCH support one step, X86 support more than one step.\n");
			return -EINVAL;
		}
#endif
		tpe->step = (int)ulongtmp;
#ifdef CONFIG_X86
		if (tpe->step > 1)
			gtp_have_step = 1;
#else
		if (tpe->step > 1)
			tpe->step = 1;
#endif

		/* Get pass.  */
		if (pkg[0] == '\0')
			return -EINVAL;
		pkg++;
		pkg = hex2ulongest(pkg, &tpe->pass);
		if (tpe->pass != 0)
			tpe->flags |= GTP_ENTRY_FLAGS_HAVE_PASS;
	}

	if (tpe) {
		/* Add action to tpe.  */
		int	step_action = 0;

		if (pkg[0] == 'S') {
			if (tpe->step == 0)
				return -EINVAL;
			pkg++;
			step_action = 1;
		} else if (!list_empty(&tpe->step_action_list))
			step_action = 1;
		while (pkg[0]) {
			struct action	*ae = NULL;
			char *pkg_cmd = pkg;

#ifdef GTP_DEBUG
			printk(GTP_DEBUG "gtp_gdbrsp_qtdp: %s\n", pkg);
#endif
			switch (pkg[0]) {
			case ':':
				pkg++;
				break;
			case 'M': {
					int		is_neg = 0;
					ULONGEST	ulongtmp;

					ae = gtp_action_alloc(pkg[0]);
					if (!ae)
						return -ENOMEM;
					pkg++;
					if (pkg[0] == '-') {
						is_neg = 1;
						pkg++;
					}
					pkg = hex2ulongest(pkg, &ulongtmp);
					ae->u.m.regnum = (int)ulongtmp;
					if (is_neg)
						ae->u.m.regnum
						  = -ae->u.m.regnum;
					if (pkg[0] == '\0') {
						kfree(ae);
						return -EINVAL;
					}
					pkg++;
					pkg = hex2ulongest(pkg, &ulongtmp);
					ae->u.m.offset = (CORE_ADDR)ulongtmp;
					if (pkg[0] == '\0') {
						kfree(ae);
						return -EINVAL;
					}
					pkg++;
					pkg = hex2ulongest(pkg, &ulongtmp);
					ae->u.m.size = (size_t)ulongtmp;
				}
				break;
			case 'R':
				/* XXX: reg_mask is ignore.  */
				ae = gtp_action_alloc(pkg[0]);
				if (!ae)
					return -ENOMEM;
				pkg++;
				pkg = hex2ulongest(pkg,
						   &ae->u.reg_mask);
				break;
			case 'X': {
					int	ret;

					ae = gtp_action_alloc(pkg[0]);
					if (!ae)
						return -ENOMEM;
					pkg++;
					ret = gtp_parse_x(tpe, ae, &pkg);
					if (ret) {
						gtp_action_release(ae);
						ae = NULL;

						if (ret < 0)
							return ret;
					}
				}
				break;
			case '-':
				pkg++;
				break;
			default:
				/* XXX: Not support.  */
				return 1;
			}

			if (ae) {
				/* Save the cmd.  */
				if (gtp_src_add (pkg_cmd, pkg, &(tpe->action_cmd))) {
					gtp_action_release(ae);
					return -ENOMEM;
				}
				/* Add ae to tpe.  */
				if ((ae->type == 'X' || ae->type == 0xff)
				    && addnew && !tpe->cond) {
					tpe->cond = ae;
				} else if (!step_action)
					list_add_tail(&ae->node, &tpe->action_list);
				else
					list_add_tail(&ae->node, &tpe->step_action_list);
			}
		}
	} else
		return -EINVAL;

	return 0;
}

static int
gtp_gdbrsp_qtdpsrc(char *pkg)
{
	ULONGEST		num, addr;
	struct gtp_entry	*tpe;

#ifdef GTP_DEBUG
	printk(GTP_DEBUG "gtp_gdbrsp_qtdpsrc: %s\n", pkg);
#endif

	if (gtp_start)
		return -EBUSY;

	/* Get num and addr.  */
	if (pkg[0] == '\0')
		return -EINVAL;
	pkg = hex2ulongest(pkg, &num);
	if (pkg[0] == '\0')
		return -EINVAL;
	pkg++;
	pkg = hex2ulongest(pkg, &addr);
	if (pkg[0] == '\0')
		return -EINVAL;
	pkg++;
	tpe = gtp_list_find(num, addr);
	if (tpe == NULL)
		return -EINVAL;

	return gtp_src_add(pkg, NULL, &(tpe->src));
}

static void gtp_plugin_mod_get(void);
static void gtp_plugin_mod_put(void);

static int
gtp_gdbrsp_qtdisconnected(char *pkg)
{
	ULONGEST setting;

	if (pkg[0] == '\0')
		return -EINVAL;

	hex2ulongest(pkg, &setting);

	if (gtp_disconnected_tracing != (int)setting) {
		if (setting)
			gtp_plugin_mod_get();
		else
			gtp_plugin_mod_put();

		gtp_disconnected_tracing = (int)setting;
	}

	return 0;
}

static int
gtp_gdbrsp_qtbuffer(char *pkg)
{
	if (strncmp("circular:", pkg, 9) == 0) {
		ULONGEST setting;

		pkg += 9;
		if (pkg[0] == '\0')
			return -EINVAL;
		hex2ulongest(pkg, &setting);

#ifdef GTP_FTRACE_RING_BUFFER
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,38)) \
    || defined(GTP_SELF_RING_BUFFER)
		gtp_circular = (int)setting;
		if (gtp_frame)
			ring_buffer_change_overwrite(gtp_frame, (int)setting);
#else
		if (gtp_circular != (int)setting)
			gtp_circular_is_changed = 1;
#endif
#endif
		gtp_circular = (int)setting;

		return 0;
	}

	return 1;
}

static int
gtp_frame_head_find_num(int num)
{
#ifdef GTP_FRAME_SIMPLE
	int	tfnum = 0;
	char	*tmp = gtp_frame_r_start;

	do {
		if (tmp == gtp_frame_end)
			tmp = gtp_frame;

		if (FID(tmp) == FID_HEAD) {
			if (tfnum == num) {
				gtp_frame_current_num = num;
				gtp_frame_current = tmp;
				return 0;
			}
			tfnum++;
		}

		tmp = gtp_frame_next(tmp);
		if (!tmp)
			break;
	} while (tmp != gtp_frame_w_start);
#endif
#ifdef GTP_FTRACE_RING_BUFFER
	if (gtp_frame_current_num >= num)
		gtp_frame_iter_reset();

	while (1) {
		int	cpu;

		cpu = gtp_frame_iter_peek_head();
		if (cpu < 0)
			break;

		if (num == gtp_frame_current_num)
			return cpu;

		ring_buffer_read(gtp_frame_iter[cpu], NULL);
	}
#endif
#ifdef GTP_RB
	if (num < gtp_frame_current_num)
		gtp_rb_read_reset();

	while (1) {
		if (gtp_frame_current_num == num)
			return 0;

		if (gtp_rb_read() != 0)
			break;
	}
#endif

	return -1;
}

static int
gtp_frame_head_find_addr(int inside, unsigned long lo,
			 unsigned long hi)
{
#ifdef GTP_FRAME_SIMPLE
	int	tfnum = gtp_frame_current_num;
	char	*tmp;

	if (gtp_frame_current)
		tmp = gtp_frame_current;
	else
		tmp = gtp_frame_r_start;

	do {
		if (tmp == gtp_frame_end)
			tmp = gtp_frame;

		if (FID(tmp) == FID_HEAD) {
			if (tfnum != gtp_frame_current_num) {
				char		*next;
				struct pt_regs	*regs = NULL;

				for (next = *(char **)(tmp + FID_SIZE); next;
				     next = *(char **)(next + FID_SIZE)) {
					if (FID(next) == FID_REG) {
						regs = (struct pt_regs *)
						       (next + FID_SIZE
							+ sizeof(char *));
						break;
					}
				}
				if (regs
				    && ((inside
					 && GTP_REGS_PC(regs) >= lo
					 && GTP_REGS_PC(regs) <= hi)
					|| (!inside
					    && (GTP_REGS_PC(regs) < lo
						|| GTP_REGS_PC(regs) > hi)))) {
					gtp_frame_current_num = tfnum;
					gtp_frame_current = tmp;
					return 0;
				}
			}
			tfnum++;
		}

		tmp = gtp_frame_next(tmp);
		if (!tmp)
			break;
	} while (tmp != gtp_frame_w_start);
#endif
#ifdef GTP_FTRACE_RING_BUFFER
	while (1) {
		int				cpu;
		struct ring_buffer_event	*rbe;
		char				*tmp;
		struct pt_regs			*regs = NULL;

		cpu = gtp_frame_iter_peek_head();
		if (cpu < 0)
			break;

		while (1) {
			ring_buffer_read(gtp_frame_iter[cpu], NULL);
			rbe = ring_buffer_iter_peek(gtp_frame_iter[cpu], NULL);
			if (rbe == NULL)
				break;

			tmp = ring_buffer_event_data(rbe);
			if (FID(tmp) == FID_HEAD)
				break;
			if (FID(tmp) == FID_REG) {
				regs = (struct pt_regs *)(tmp + FID_SIZE);
				break;
			}
		}

		if (regs
		    && ((inside
			  && GTP_REGS_PC(regs) >= lo
			  && GTP_REGS_PC(regs) <= hi)
			|| (!inside
			    && (GTP_REGS_PC(regs) < lo
				|| GTP_REGS_PC(regs) > hi))))
			return gtp_frame_head_find_num(gtp_frame_current_num);
	}
#endif
#ifdef GTP_RB
	struct gtp_rb_walk_s	rbws;

	if (gtp_frame_current_num < 0) {
		if (gtp_rb_read() != 0)
			return -1;
	}

	rbws.flags = GTP_RB_WALK_PASS_PAGE | GTP_RB_WALK_CHECK_END
		     | GTP_RB_WALK_CHECK_ID | GTP_RB_WALK_CHECK_TYPE;
	rbws.type = FID_REG;

	while (1) {
		char	*tmp;

		rbws.end = gtp_frame_current_rb->w;
		rbws.id = gtp_frame_current_id;
		tmp = gtp_rb_walk(&rbws, gtp_frame_current_rb->rp);
		if (rbws.reason == gtp_rb_walk_type) {
			struct pt_regs	*regs
				= (struct pt_regs *)(tmp + FID_SIZE);

			if ((inside && GTP_REGS_PC(regs) >= lo
			     && GTP_REGS_PC(regs) <= hi)
			    || (!inside && (GTP_REGS_PC(regs) < lo
					    || GTP_REGS_PC(regs) > hi))) {
				return 0;
			}
		}

		if (gtp_rb_read() != 0)
			break;
	}
#endif

	return -1;
}

static int
gtp_frame_head_find_trace(ULONGEST trace)
{
#ifdef GTP_FRAME_SIMPLE
	int	tfnum = gtp_frame_current_num;
	char	*tmp;

	if (gtp_frame_current)
		tmp = gtp_frame_current;
	else
		tmp = gtp_frame_r_start;

	do {
		if (tmp == gtp_frame_end)
			tmp = gtp_frame;

		if (FID(tmp) == FID_HEAD) {
			if (tfnum != gtp_frame_current_num) {
				if (trace == *(ULONGEST *) (tmp + FID_SIZE
							    + sizeof(char *))) {
					gtp_frame_current_num = tfnum;
					gtp_frame_current = tmp;
					return 0;
				}
			}
			tfnum++;
		}

		tmp = gtp_frame_next(tmp);
		if (!tmp)
			break;
	} while (tmp != gtp_frame_w_start);
#endif
#ifdef GTP_FTRACE_RING_BUFFER
	while (1) {
		int				cpu;
		struct ring_buffer_event	*rbe;
		char				*tmp;

		cpu = gtp_frame_iter_peek_head();
		if (cpu < 0)
			break;

		rbe = ring_buffer_iter_peek(gtp_frame_iter[cpu], NULL);
		if (rbe == NULL) {
			/* It will not happen, just for safe.  */
			return -1;
		}
		tmp = ring_buffer_event_data(rbe);
		if (trace == *(ULONGEST *) (tmp + FID_SIZE))
			return cpu;

		ring_buffer_read(gtp_frame_iter[cpu], NULL);
	}
#endif
#ifdef GTP_RB
	if (gtp_frame_current_num < 0) {
		if (gtp_rb_read() != 0)
			return -1;
	}

	while (1) {
		if (gtp_frame_current_tpe == trace)
			return 0;

		if (gtp_rb_read() != 0)
			break;
	}
#endif

	return -1;
}

static int
gtp_gdbrsp_qtframe(char *pkg)
{
	int	ret = -1;
#if defined(GTP_FTRACE_RING_BUFFER) || defined(GTP_RB)
	int	old_num = gtp_frame_current_num;
#endif

	if (gtp_start)
		return -EBUSY;

	if (gtp_gtpframe_pipe_pid >= 0)
		return -EBUSY;

#ifdef GTP_DEBUG
	printk(GTP_DEBUG "gtp_gdbrsp_qtframe: %s\n", pkg);
#endif

	if (atomic_read(&gtp_frame_create) == 0)
		goto out;

	if (strncmp(pkg, "pc:", 3) == 0) {
		ULONGEST	addr;

		pkg += 3;

		if (pkg[0] == '\0')
			return -EINVAL;
		hex2ulongest(pkg, &addr);

		ret = gtp_frame_head_find_addr(1, (unsigned long)addr,
					       (unsigned long)addr);
	} else if (strncmp(pkg, "tdp:", 4) == 0) {
		ULONGEST	trace;

		pkg += 4;

		if (pkg[0] == '\0')
			return -EINVAL;
		hex2ulongest(pkg, &trace);

		ret = gtp_frame_head_find_trace(trace);
	} else if (strncmp(pkg, "range:", 6) == 0) {
		ULONGEST	start, end;

		pkg += 6;

		if (pkg[0] == '\0')
			return -EINVAL;
		pkg = hex2ulongest(pkg, &start);
		if (pkg[0] == '\0')
			return -EINVAL;
		pkg++;
		hex2ulongest(pkg, &end);

		ret = gtp_frame_head_find_addr(1, (unsigned long)start,
					       (unsigned long)end);
	} else if (strncmp(pkg, "outside:", 8) == 0) {
		ULONGEST	start, end;

		pkg += 8;

		if (pkg[0] == '\0')
			return -EINVAL;
		pkg = hex2ulongest(pkg, &start);
		if (pkg[0] == '\0')
			return -EINVAL;
		pkg++;
		hex2ulongest(pkg, &end);

		ret = gtp_frame_head_find_addr(0, (unsigned long)start,
					       (unsigned long)end);
	} else {
		ULONGEST	num;

		if (pkg[0] == '\0')
			return -EINVAL;
		hex2ulongest(pkg, &num);

		if (((int) num) < 0) {
			/* Return to current.  */
#ifdef GTP_FRAME_SIMPLE
			gtp_frame_current = NULL;
			gtp_frame_current_num = -1;
#endif
#ifdef GTP_FTRACE_RING_BUFFER
			gtp_frame_iter_reset();
#endif
#ifdef GTP_RB
			gtp_rb_read_reset();
#endif

			return 0;
		}
		ret = gtp_frame_head_find_num((int) num);
	}

out:
	if (ret < 0) {
#if defined(GTP_FTRACE_RING_BUFFER) || defined(GTP_RB)
		/* Set frame back to old_num.  */
		if (old_num < 0)
#ifdef GTP_FTRACE_RING_BUFFER
			gtp_frame_iter_reset();
#endif
#ifdef GTP_RB
			gtp_rb_read_reset();
#endif
		else
			gtp_frame_head_find_num(old_num);
#endif
		snprintf(gtp_rw_bufp, GTP_RW_BUFP_MAX, "F-1");
		gtp_rw_bufp += 3;
		gtp_rw_size += 3;
	} else {
#ifdef GTP_FRAME_SIMPLE
		gtp_frame_current_tpe = *(ULONGEST *)(gtp_frame_current
						      + FID_SIZE
						      + sizeof(char *));
#endif
#ifdef GTP_FTRACE_RING_BUFFER
		struct ring_buffer_event	*rbe;
		char				*tmp;

		rbe = ring_buffer_read(gtp_frame_iter[ret],
				       &gtp_frame_current_clock);
		if (rbe == NULL) {
			/* It will not happen, just for safe.  */
			ret = -1;
			goto out;
		}
		gtp_frame_current_cpu = ret;
		tmp = ring_buffer_event_data(rbe);
		gtp_frame_current_tpe = *(ULONGEST *)(tmp + FID_SIZE);
#endif
		snprintf(gtp_rw_bufp, GTP_RW_BUFP_MAX, "F%xT%x",
			 gtp_frame_current_num,
			 (unsigned int) gtp_frame_current_tpe);
		gtp_rw_size += strlen(gtp_rw_bufp);
		gtp_rw_bufp += strlen(gtp_rw_bufp);
	}
	return 1;
}

static int
gtp_gdbrsp_qtro(char *pkg)
{
	ULONGEST	start, end;

	gtpro_list_clear();

	while (pkg[0]) {
		pkg = hex2ulongest(pkg, &start);
		if (pkg[0] != ',')
			return -EINVAL;
		pkg++;
		pkg = hex2ulongest(pkg, &end);
		if (pkg[0])
			pkg++;

		if (gtpro_list_add((CORE_ADDR)start, (CORE_ADDR)end) == NULL)
			return -ENOMEM;
	}

	return 0;
}

static int
gtp_gdbrsp_qtdv(char *pkg)
{
	int				ret = -EINVAL;
	ULONGEST			num, val;
	struct gtp_var			*var = NULL;
	char				*src;
	int				src_size;
	int				per_cpu_alloced = 0;
#ifdef GTP_PERF_EVENTS
	int				pe_alloced = 0;
#endif

	pkg = hex2ulongest(pkg, &num);
	if (pkg[0] != ':')
		goto error_out;
	pkg++;
	pkg = hex2ulongest(pkg, &val);
	if (pkg[0] != ':')
		goto error_out;

	var = gtp_var_find_num(num);
	if (var) {
		if (var->type == gtp_var_special) {
			if (var->u.hooks && var->u.hooks->gdb_set_val) {
				ret = var->u.hooks->gdb_set_val(NULL,
								var, val);
				if (ret != 0)
					goto error_out;
			}
			return 0;
		} else
			goto error_out;
	}

	pkg ++;
	src = pkg;
	src_size = strlen(src);

	/* Remove "0:" for following code.  */
	if (strlen(pkg) <= 2)
		goto error_out;
	pkg += 2;

	/* Check if this is a "p_" or "per_cpu_" trace state variable.  */
	if (strncasecmp(pkg, "705f", 4) == 0
	    || strncasecmp(pkg, "7065725f6370755f", 16) == 0) {
		int				name_size;
		char				*id_s;
		int				mul = 1;
		struct list_head		*cur;
		struct gtp_var			*cvar;
		int				per_cpu_id = -1;

		if (strncasecmp(pkg, "705f", 4) == 0)
			pkg += 4;
		else
			pkg += 16;
		name_size = strlen(pkg);

		/* Get the cpu id of this variable.  */
		if (name_size % 2 != 0)
			goto error_out;
		for (id_s = pkg + name_size - 2; id_s > pkg; id_s -= 2) {
			int	i, j;

			if (!hex2int(id_s[0], &i))
				goto error_out;
			if (!hex2int(id_s[1], &j))
				goto error_out;
			j |= (i << 4);
			if (j < 0x30 || j > 0x39)
				break;
			j -= 0x30;
			if (per_cpu_id < 0)
				per_cpu_id = 0;
			per_cpu_id += mul * j;
			mul *= 10;
			/* src_size will not include the cpu id.  */
			src_size -= 2;
		}
		if (per_cpu_id >= gtp_cpu_number) {
			printk(KERN_WARNING "gtp_gdbrsp_qtdv: id %d is bigger "
					    "than cpu number %d.\n",
			       per_cpu_id, gtp_cpu_number);
			goto error_out;
		}

		var = gtp_var_alloc(per_cpu_id, (unsigned int)num, 0,
				    (int64_t)val, src);
		if (IS_ERR(var)) {
			ret = PTR_ERR(var);
			var = NULL;
			goto error_out;
		}

		/* Setup var.  */
		var->type = gtp_var_per_cpu;
		var->u.pc.cpu = per_cpu_id;
		/* Find the per cpu struct.  */
		list_for_each(cur, &gtp_var_list) {
			cvar = list_entry(cur, struct gtp_var, node);
#ifdef GTP_PERF_EVENTS
			if (cvar->type != gtp_var_per_cpu
			    && cvar->type != gtp_var_perf_event_per_cpu)
#else
			if (cvar->type != gtp_var_per_cpu)
#endif
				continue;

			if (strncmp (cvar->src, src, src_size) == 0) {
				int	csize;

				/* Following part code to make sure
				   cvar->src without ID is same with
				   var->src without id.  */
				csize = strlen(cvar->src);
				if (csize % 2 != 0) {
					printk(KERN_WARNING "Src %s of TSR %u is not right.\n",
					       cvar->src, cvar->num);
					continue;
				}
				for (csize -= 2; csize >= src_size; csize -= 2) {
					int	i, j;

					if (!hex2int(cvar->src[csize], &i))
						break;
					if (!hex2int(cvar->src[csize + 1], &j))
						break;
					j |= (i << 4);
					if (j < 0x30 || j > 0x39)
						break;
				}
				if (csize >= src_size)
					continue;

				var->u.pc.pc = cvar->u.pc.pc;
				break;
			}
		}
		if (var->u.pc.pc == NULL) {
			int	cpu;

			var->u.pc.pc = alloc_percpu(struct gtp_var_per_cpu);
			if (var->u.pc.pc == NULL) {
				ret = -ENOMEM;
				goto error_out;
			}
			for_each_possible_cpu(cpu)
				memset(per_cpu_ptr(var->u.pc.pc, cpu), '\0',
				       sizeof(struct gtp_var_per_cpu));

			per_cpu_alloced = 1;
#ifdef GTP_DEBUG
			printk(GTP_DEBUG "gtp_gdbrsp_qtdv: Create a "
					 "new per_cpu list for %s and set var "
					 "to cpu %d.\n",
			       src, var->u.pc.cpu);
#endif
		} else {
#ifdef GTP_DEBUG
			printk(GTP_DEBUG "gtp_gdbrsp_qtdv: Find a "
					 "per_cpu list for %s and set var "
					 "to cpu %d.\n",
			       src, var->u.pc.cpu);
#endif
		}
		if (var->u.pc.cpu >= 0)
			gtp_var_get_pc(var)->u.val = val;
	} else {
		var = gtp_var_alloc(-1, (unsigned int)num, 0, (int64_t)val,
				    src);
		if (IS_ERR(var)) {
			ret = PTR_ERR(var);
			var = NULL;
			goto error_out;
		}
		/* Setup var.  */
		var->type = gtp_var_normal;
		var->u.val = val;
	}

	/* Check if this is a "pe_" OR "perf_event_" trace state variable.  */
	if (strncasecmp(pkg, "70655f", 6) == 0
	    || strncasecmp(pkg, "706572665f6576656e745f", 22) == 0) {
#ifdef GTP_PERF_EVENTS
		enum pe_tv_id		ptid;
		struct list_head	*cur;
		struct gtp_var_pe	*cpe = NULL, *pe;

		if (strncasecmp(pkg, "70655f", 6) == 0)
			pkg += 6;
		else
			pkg += 22;

		if (strncasecmp(pkg, "6370755f", 8) == 0) {
			/* "cpu_" */
			pkg += 8;
			ptid = pe_tv_cpu;
		} else if (strncasecmp(pkg, "747970655f", 10) == 0) {
			/* "type_" */
			pkg += 10;
			ptid = pe_tv_type;
		} else if (strncasecmp(pkg, "636f6e6669675f", 14) == 0) {
			/* "config_" */
			pkg += 14;
			ptid = pe_tv_config;
		} else if (strncasecmp(pkg, "656e5f", 6) == 0) {
			/* "en_" */
			pkg += 6;
			ptid = pe_tv_en;
		} else if (strncasecmp(pkg, "76616c5f", 8) == 0) {
			/* "val_" */
			pkg += 8;
			ptid = pe_tv_val;
		} else if (strncasecmp(pkg, "656e61626c65645f", 16) == 0) {
			/* "enabled_" */
			pkg += 16;
			ptid = pe_tv_enabled;
		} else if (strncasecmp(pkg, "72756e6e696e675f", 16) == 0) {
			/* "running_" */
			pkg += 16;
			ptid = pe_tv_running;
		} else
			goto error_out;

		if (strlen(pkg) <= 0)
			goto error_out;

		if (var->type == gtp_var_per_cpu) {
			var->type = gtp_var_perf_event_per_cpu;
			if (var->u.pc.cpu < 0)
				goto out;
		}
		else
			var->type = gtp_var_perf_event;

		/* Find the pe_tv that name is pkg.  */
		list_for_each(cur, &gtp_var_list) {
			struct gtp_var	*cvar = list_entry(cur,
							   struct gtp_var,
							   node);
			if (var->type == cvar->type
			    && !(cvar->type == gtp_var_perf_event_per_cpu
				 && cvar->u.pc.cpu < 0)) {
				cpe = gtp_var_get_pe(cvar);
				if (strcmp(cpe->pe->name, pkg) == 0)
					break;
			}
		}

		pe = gtp_var_get_pe(var);
		pe->ptid = ptid;

		if (cur == &gtp_var_list) {
			if (var->type == gtp_var_perf_event_per_cpu)
				pe->pe = kzalloc(sizeof(struct gtp_var_perf_event),
						 GFP_KERNEL);
			else
				pe->pe = kmalloc_node(sizeof(struct gtp_var_perf_event),
						      GFP_KERNEL | __GFP_ZERO,
						      cpu_to_node(var->u.pc.cpu));
			if (pe->pe == NULL) {
				ret = -ENOMEM;
				goto error_out;
			}

			/* Init the value in pe to default value.  */
			pe_alloced = 1;
			pe->pe->name = gtp_strdup(pkg, NULL);
			if (pe->pe->name == NULL) {
				ret = -ENOMEM;
				goto error_out;
			}
			pe->pe->en = 0;
			pe->pe->attr.type = PERF_TYPE_HARDWARE;
			pe->pe->attr.config = PERF_COUNT_HW_CPU_CYCLES;
			pe->pe->attr.disabled = 1;
			pe->pe->attr.pinned = 1;
			pe->pe->attr.size = sizeof(struct perf_event_attr);
			if (var->type == gtp_var_perf_event_per_cpu)
				pe->pe->cpu = var->u.pc.cpu;
		} else
			pe->pe = cpe->pe;

		/* Set current val to pe.  */
		switch (ptid) {
		case pe_tv_cpu:
			pe->pe->cpu = (int)(LONGEST)val;
			break;
		case pe_tv_type:
			pe->pe->attr.type = val;
			break;
		case pe_tv_config:
			pe->pe->attr.config = val;
			break;
		case pe_tv_en:
			if (val) {
				pe->pe->attr.disabled = 0;
				pe->pe->en = 1;
			} else {
				pe->pe->attr.disabled = 1;
				pe->pe->en = 0;
			}
			break;
		case pe_tv_val:
		case pe_tv_enabled:
		case pe_tv_running:
			break;
		default:
			goto error_out;
			break;
		}

		gtp_have_pc_pe = 1;
#else
		printk(KERN_WARNING "Current Kernel doesn't open "
				    "GTP_PERF_EVENTS\n");
		ret = -ENXIO;
		goto error_out;
#endif
	}
#ifdef GTP_PERF_EVENTS
out:
#endif
	list_add(&var->node, &gtp_var_list);
	gtp_var_num++;

	return 0;

error_out:
#ifdef GTP_PERF_EVENTS
	if (pe_alloced)
		kfree(gtp_var_get_pe(var)->pe);
#endif
	if (per_cpu_alloced)
		free_percpu(var->u.pc.pc);

	if (var) {
		kfree(var->src);
		kfree(var);
	}

	return ret;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
static int
gtp_gdbrsp_qtenable_qtdisable(char *pkg, int enable)
{
	ULONGEST		num, addr;
	struct gtp_entry	*tpe;
	int			ret;

	pkg = hex2ulongest(pkg, &num);
	if (pkg[0] != ':')
		return -EINVAL;
	pkg++;
	hex2ulongest(pkg, &addr);

	tpe = gtp_list_find(num, addr);
	if (tpe == NULL)
		return -EINVAL;
	if (tpe->type != gtp_entry_kprobe) {
		printk(KERN_WARNING "gtp_write: this tracepoint doesn't support enable and disable.\n");
		return -EINVAL;
	}

	spin_lock(&gtp_handler_enable_disable_loc);

	if (tpe->flags & GTP_ENTRY_FLAGS_IS_KRETPROBE) {
		if (enable)
			ret = enable_kretprobe(&(tpe->u.kp.kpret));
		else
			ret = disable_kretprobe(&(tpe->u.kp.kpret));
	} else {
		if (enable)
			ret = enable_kprobe(&(tpe->u.kp.kpret.kp));
		else
			ret = disable_kprobe(&(tpe->u.kp.kpret.kp));
	}

	if (ret != 0)
		tpe->disable = enable ? 0 : 1;

	spin_unlock(&gtp_handler_enable_disable_loc);
	return ret;
}
#endif

static int
gtp_gdbrsp_QT(char *pkg)
{
	int	ret = 1;

#ifdef GTP_DEBUG
	printk(GTP_DEBUG "gtp_gdbrsp_QT: %s\n", pkg);
#endif

	if (strcmp("init", pkg) == 0)
		ret = gtp_gdbrsp_qtinit();
	else if (strcmp("Stop", pkg) == 0)
		ret = gtp_gdbrsp_qtstop();
	else if (strcmp("Start", pkg) == 0)
		ret = gtp_gdbrsp_qtstart();
	else if (strncmp("DP:", pkg, 3) == 0)
		ret = gtp_gdbrsp_qtdp(pkg + 3);
	else if (strncmp("DPsrc:", pkg, 6) == 0)
		ret = gtp_gdbrsp_qtdpsrc(pkg + 6);
	else if (strncmp("Disconnected:", pkg, 13) == 0)
		ret = gtp_gdbrsp_qtdisconnected(pkg + 13);
	else if (strncmp("Buffer:", pkg, 7) == 0)
		ret = gtp_gdbrsp_qtbuffer(pkg + 7);
	else if (strncmp("Frame:", pkg, 6) == 0)
		ret = gtp_gdbrsp_qtframe(pkg + 6);
	else if (strncmp("ro:", pkg, 3) == 0)
		ret = gtp_gdbrsp_qtro(pkg + 3);
	else if (strncmp("DV:", pkg, 3) == 0)
		ret = gtp_gdbrsp_qtdv(pkg + 3);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
	else if (strncmp("Enable:", pkg, 7) == 0)
		ret = gtp_gdbrsp_qtenable_qtdisable(pkg + 7, 1);
	else if (strncmp("Disable:", pkg, 8) == 0)
		ret = gtp_gdbrsp_qtenable_qtdisable(pkg + 8, 0);
#endif

#ifdef GTP_DEBUG
	printk(GTP_DEBUG "gtp_gdbrsp_QT: return %d\n", ret);
#endif

	return ret;
}

static int
gtp_get_status(struct gtp_entry *tpe, char *buf, int bufmax)
{
	int			size = 0;
	int			tfnum = 0;
	CORE_ADDR		tmpaddr;

#ifdef GTP_RB
	if (GTP_RB_PAGE_IS_EMPTY) {
#endif
#if defined(GTP_FRAME_SIMPLE) || defined(GTP_FTRACE_RING_BUFFER)
	if (!gtp_frame) {
#endif
		snprintf(buf, bufmax, "tnotrun:0;");
		buf += 10;
		size += 10;
		bufmax -= 10;
	} else if (!tpe || (tpe && tpe->reason == gtp_stop_normal)) {
		snprintf(buf, bufmax, "tstop:0;");
		buf += 8;
		size += 8;
		bufmax -= 8;
	} else {
		char	outtmp[100];

		switch (tpe->reason) {
		case gtp_stop_frame_full:
			snprintf(buf, bufmax, "tfull:%lx;",
				 (unsigned long)tpe->num);
			break;
		case gtp_stop_efault:
			snprintf(buf, bufmax, "terror:%s:%lx;",
				 string2hex("read memory false", outtmp),
				 (unsigned long)tpe->num);
			break;
		case gtp_stop_access_wrong_reg:
			snprintf(buf, bufmax, "terror:%s:%lx;",
				 string2hex("access wrong register", outtmp),
				 (unsigned long)tpe->num);
			break;
		case gtp_stop_agent_expr_code_error:
			snprintf(buf, bufmax, "terror:%s:%lx;",
				 string2hex("agent expression code error",
					    outtmp),
				 (unsigned long)tpe->num);
			break;
		case gtp_stop_agent_expr_stack_overflow:
			snprintf(buf, bufmax, "terror:%s:%lx;",
				string2hex("agent expression stack overflow",
					   outtmp),
				(unsigned long)tpe->num);
			break;
		default:
			buf[0] = '\0';
			break;
		}

		size += strlen(buf);
		bufmax -= strlen(buf);
		buf += strlen(buf);
	}

	if (atomic_read(&gtp_frame_create)) {
#ifdef GTP_FRAME_SIMPLE
		char	*tmp = gtp_frame_r_start;

		do {
			if (tmp == gtp_frame_end)
				tmp = gtp_frame;

			if (FID(tmp) == FID_HEAD)
				tfnum++;

			tmp = gtp_frame_next(tmp);
			if (!tmp)
				break;
		} while (tmp != gtp_frame_w_start);
#endif
#ifdef GTP_FTRACE_RING_BUFFER
		if (gtp_start) {
			/* XXX: It is just the number of entries.  */
			tfnum = (int)ring_buffer_entries(gtp_frame);
		} else {
			int	old_num = gtp_frame_current_num;
			int	cpu;

			gtp_frame_iter_reset();

			for_each_possible_cpu(cpu) {
				char				*tmp;
				struct ring_buffer_event	*rbe;

				while (1) {
					rbe = ring_buffer_read
						(gtp_frame_iter[cpu], NULL);
					if (rbe == NULL)
						break;
					tmp = ring_buffer_event_data(rbe);
					if (FID(tmp) == FID_HEAD)
						tfnum++;
				}
			}

			if (old_num == -1)
				gtp_frame_iter_reset();
			else if (old_num >= 0) {
				gtp_frame_head_find_num(old_num);
				ring_buffer_read
					(gtp_frame_iter[gtp_frame_current_cpu],
					 NULL);
			}
		}
#endif
#ifdef GTP_RB
		int			cpu;
		struct gtp_rb_walk_s	rbws;

		rbws.flags = GTP_RB_WALK_PASS_PAGE | GTP_RB_WALK_CHECK_END;

		for_each_possible_cpu(cpu) {
			struct gtp_rb_s	*rb
				= (struct gtp_rb_s *)per_cpu_ptr(gtp_rb, cpu);
			void		*tmp;
			unsigned long	flags;

			GTP_RB_LOCK_IRQ(rb, flags);
			rbws.end = rb->w;
			tmp = rb->r;
			while (1) {
				tmp = gtp_rb_walk(&rbws, tmp);
				if (rbws.reason != gtp_rb_walk_new_entry)
					break;
				tfnum++;
				tmp += FRAME_ALIGN(GTP_FRAME_HEAD_SIZE);
			}
			GTP_RB_UNLOCK_IRQ(rb, flags);
		}
#endif
	}

	snprintf(buf, bufmax, "tframes:%x;", tfnum);
	size += strlen(buf);
	bufmax -= strlen(buf);
	buf += strlen(buf);

	snprintf(buf, bufmax, "tcreated:%x;", atomic_read(&gtp_frame_create));
	size += strlen(buf);
	bufmax -= strlen(buf);
	buf += strlen(buf);

#ifdef GTP_FRAME_SIMPLE
	snprintf(buf, bufmax, "tsize:%x;", GTP_FRAME_SIZE);
#endif
#ifdef GTP_FTRACE_RING_BUFFER
	if (gtp_frame)
		snprintf(buf, bufmax, "tsize:%lx;",
			 ring_buffer_size(gtp_frame));
	else
		snprintf(buf, bufmax, "tsize:%x;",
			 GTP_FRAME_SIZE * num_online_cpus());
#endif
#ifdef GTP_RB
	snprintf(buf, bufmax, "tsize:%lx;",
		 gtp_rb_page_count * GTP_RB_DATA_MAX * num_online_cpus());
#endif
	size += strlen(buf);
	bufmax -= strlen(buf);
	buf += strlen(buf);

#ifdef GTP_FRAME_SIMPLE
	spin_lock(&gtp_frame_lock);
	if (gtp_frame_is_circular)
		tmpaddr = 0;
	else
		tmpaddr = GTP_FRAME_SIZE - (gtp_frame_w_start - gtp_frame);
	spin_unlock(&gtp_frame_lock);
#endif
#ifdef GTP_FTRACE_RING_BUFFER
	/* XXX: Ftrace ring buffer don't have interface to get the size of free
	   buffer. */
	tmpaddr = 0;
#endif
#ifdef GTP_RB
	if (atomic_read(&gtp_frame_create)) {
		int			cpu;

		tmpaddr = 0;
		for_each_possible_cpu(cpu) {
			struct gtp_rb_s	*rb
				= (struct gtp_rb_s *)per_cpu_ptr(gtp_rb, cpu);
			void		*tmp;
			unsigned long	flags;

			GTP_RB_LOCK_IRQ(rb, flags);
			tmpaddr += GTP_RB_END(rb->w) - rb->w;
			for (tmp = GTP_RB_NEXT(rb->w);
			     GTP_RB_HEAD(tmp) != GTP_RB_HEAD(rb->r);
			     tmp = GTP_RB_NEXT(tmp))
				tmpaddr += GTP_RB_DATA_MAX;
			tmpaddr += rb->r - GTP_RB_DATA(rb->r);
			GTP_RB_UNLOCK_IRQ(rb, flags);
		}
	} else {
		tmpaddr = gtp_rb_page_count * GTP_RB_DATA_MAX
			  * num_online_cpus();
	}
#endif
	snprintf(buf, bufmax, "tfree:%lx;", (unsigned long)tmpaddr);
	size += strlen(buf);
	bufmax -= strlen(buf);
	buf += strlen(buf);

	snprintf(buf, bufmax, "circular:%x;", gtp_circular);
	size += strlen(buf);
	bufmax -= strlen(buf);
	buf += strlen(buf);

	snprintf(buf, bufmax, "disconn:%x", gtp_disconnected_tracing);
	size += strlen(buf);
	bufmax -= strlen(buf);
	buf += strlen(buf);

	return size;
}

static int
gtp_gdbrsp_qtstatus(void)
{
	struct gtp_entry	*tpe;
	int			tmp;

	for (tpe = gtp_list; tpe; tpe = tpe->next) {
		if (tpe->reason != gtp_stop_normal)
			break;
	}

	if (gtp_start && tpe)	/* Tpe is stop, stop all tpes.  */
		gtp_gdbrsp_qtstop();

	snprintf(gtp_rw_bufp, GTP_RW_BUFP_MAX, "T%x;", gtp_start ? 1 : 0);
	gtp_rw_bufp += 3;
	gtp_rw_size += 3;

	tmp = gtp_get_status(tpe, gtp_rw_bufp, GTP_RW_BUFP_MAX);
	gtp_rw_bufp += tmp;
	gtp_rw_size += tmp;

	return 1;
}

#define GTP_REPORT_TRACEPOINT_MAX	(1 + 16 + 1 + 16 + 1 + 1 + 1 + \
					 20 + 1 + 16 + 1)

static void
gtp_report_tracepoint(struct gtp_entry *gtp, char *buf, int bufmax)
{
	snprintf(buf, bufmax, "T%lx:%lx:%c:%d:%lx", (unsigned long)gtp->num,
		 (unsigned long)gtp->addr, (gtp->disable ? 'D' : 'E'),
		 gtp->step, (unsigned long)gtp->pass);
}

static int
gtp_report_action_max(struct gtp_entry *gtp, struct gtpsrc *action)
{
	return 1 + 16 + 1 + 16 + 1 + strlen(action->src) + 1;
}

static void
gtp_report_action(struct gtp_entry *gtp, struct gtpsrc *action, char *buf,
		  int bufmax)
{
	snprintf(buf, bufmax, "A%lx:%lx:%s", (unsigned long)gtp->num,
		 (unsigned long)gtp->addr, action->src);
}

static int
gtp_report_src_max(struct gtp_entry *gtp, struct gtpsrc *src)
{
	return 1 + 16 + 1 + 16 + 1 + strlen(src->src) + 1;
}

static void
gtp_report_src(struct gtp_entry *gtp, struct gtpsrc *src, char *buf, int bufmax)
{
	snprintf(buf, bufmax, "Z%lx:%lx:%s", (unsigned long)gtp->num,
		 (unsigned long)gtp->addr, src->src);
}

static void
gtp_current_set_check(void)
{
	if (current_gtp_src == NULL)
		current_gtp = current_gtp->next;
}

static void
gtp_current_action_check(void)
{
	if (current_gtp_action_cmd == NULL) {
		current_gtp_src = current_gtp->src;
		gtp_current_set_check();
	}
}

static int
gtp_gdbrsp_qtfp(void)
{
	if (gtp_list) {
		current_gtp = gtp_list;
		gtp_report_tracepoint(current_gtp, gtp_rw_bufp,
				      GTP_RW_BUFP_MAX);
		gtp_rw_size += strlen(gtp_rw_bufp);
		gtp_rw_bufp += strlen(gtp_rw_bufp);
		current_gtp_action_cmd = current_gtp->action_cmd;
		gtp_current_action_check();
	} else {
		if (GTP_RW_BUFP_MAX > 1) {
			gtp_rw_bufp[0] = 'l';
			gtp_rw_size += 1;
			gtp_rw_bufp += 1;
		}
	}

	return 1;
}

static int
gtp_gdbrsp_qtsp(void)
{
	if (current_gtp_action_cmd) {
		gtp_report_action(current_gtp, current_gtp_action_cmd,
				  gtp_rw_bufp, GTP_RW_BUFP_MAX);
		gtp_rw_size += strlen(gtp_rw_bufp);
		gtp_rw_bufp += strlen(gtp_rw_bufp);
		current_gtp_action_cmd = current_gtp_action_cmd->next;
		gtp_current_action_check();
		goto out;
	}

	if (current_gtp_src) {
		gtp_report_src(current_gtp, current_gtp_src, gtp_rw_bufp,
			       GTP_RW_BUFP_MAX);
		gtp_rw_size += strlen(gtp_rw_bufp);
		gtp_rw_bufp += strlen(gtp_rw_bufp);
		current_gtp_src = current_gtp_src->next;
		gtp_current_set_check();
		goto out;
	}

	if (current_gtp) {
		gtp_report_tracepoint(current_gtp, gtp_rw_bufp,
				      GTP_RW_BUFP_MAX);
		gtp_rw_size += strlen(gtp_rw_bufp);
		gtp_rw_bufp += strlen(gtp_rw_bufp);
		current_gtp_action_cmd = current_gtp->action_cmd;
		gtp_current_action_check();
	} else {
		if (GTP_RW_BUFP_MAX > 1) {
			gtp_rw_bufp[0] = 'l';
			gtp_rw_size += 1;
			gtp_rw_bufp += 1;
		}
	}
out:
	return 1;
}

static int
gtp_gdbrsp_qtfsv(int f)
{
	if (f) {
		if (list_empty(&gtp_var_list))
			current_gtp_var = NULL;
		else
			current_gtp_var = list_first_entry(&gtp_var_list,
							   struct gtp_var,
							   node);
	}

	if (current_gtp_var) {
		snprintf(gtp_rw_bufp, GTP_RW_BUFP_MAX, "%x:%llx:%s",
			 current_gtp_var->num,
			 (unsigned long long)current_gtp_var->initial_val,
			 current_gtp_var->src);
		gtp_rw_size += strlen(gtp_rw_bufp);
		gtp_rw_bufp += strlen(gtp_rw_bufp);

		if (current_gtp_var->node.next != &gtp_var_list)
			current_gtp_var = list_first_entry(&(current_gtp_var->node),
							   struct gtp_var,
							   node);
		else
			current_gtp_var = NULL;
	} else {
		if (GTP_RW_BUFP_MAX > 1) {
			gtp_rw_bufp[0] = 'l';
			gtp_rw_size += 1;
			gtp_rw_bufp += 1;
		}
	}

	return 1;
}

#ifdef GTP_RB
static int
gtp_rb_traceframe_get_tv(void *buf, u64 id, unsigned int num, uint64_t *val)
{
	struct gtp_rb_walk_s	rbws;
	char			*tmp;

	/* Handle $cpu_id.  */
	if (num == GTP_VAR_CPU_ID) {
		*val = gtp_frame_current_rb->cpu;
		return 0;
	}

	rbws.flags = GTP_RB_WALK_PASS_PAGE | GTP_RB_WALK_CHECK_END
		     | GTP_RB_WALK_CHECK_ID | GTP_RB_WALK_CHECK_TYPE;
	rbws.end = gtp_frame_current_rb->w;
	rbws.id = id;
	rbws.type = FID_VAR;
	tmp = buf;

	while (1) {
		struct gtp_frame_var	*vr;

		tmp = gtp_rb_walk(&rbws, tmp);
		if (rbws.reason != gtp_rb_walk_type)
			break;

		vr = (struct gtp_frame_var *)(tmp + FID_SIZE);
		if (vr->num == num) {
			*val = vr->val;
			return 0;
		}

		tmp += FRAME_ALIGN(GTP_FRAME_VAR_SIZE);
	}

	return -1;
}
#endif

static int
gtp_gdbrsp_qtv(char *pkg)
{
	ULONGEST		num;
	struct gtp_var		*var = NULL;
	struct gtp_frame_var	*vr = NULL;
	uint64_t		val = 0;
	int			ret;

	pkg = hex2ulongest(pkg, &num);

#ifdef GTP_FRAME_SIMPLE
	if (gtp_start || !gtp_frame_current) {
#elif defined(GTP_FTRACE_RING_BUFFER) || defined(GTP_RB)
	if (gtp_start || gtp_frame_current_num < 0) {
#endif
		var = gtp_var_find_num(num);
		if (var == NULL)
			goto out;

		switch (var->type) {
		case gtp_var_special:
			if (var->u.hooks && var->u.hooks->gdb_get_val) {
				ret = var->u.hooks->gdb_get_val(NULL,
								var, &val);
				if (ret)
					return ret;
			} else
				var = NULL;
			break;
#ifdef GTP_PERF_EVENTS
		case gtp_var_perf_event:
		case gtp_var_perf_event_per_cpu: {
			struct gtp_var_pe	*pe = gtp_var_get_pe(var);
			if (pe->ptid == pe_tv_val
			    || pe->ptid == pe_tv_enabled
			    || pe->ptid == pe_tv_running) {
				if (gtp_start)
					pe->pe->val = perf_event_read_value(pe->pe->event,
									    &(pe->pe->enabled),
									    &(pe->pe->running));
			}
			switch (pe->ptid) {
			case pe_tv_val:
				val = (uint64_t)(pe->pe->val);
				break;
			case pe_tv_enabled:
				val = (uint64_t)(pe->pe->enabled);
				break;
			case pe_tv_running:
				val = (uint64_t)(pe->pe->running);
				break;
			default:
				break;
			}
		}
			break;
#endif
		case gtp_var_per_cpu:
			val = gtp_var_get_pc(var)->u.val;
			break;
		default:
			val = var->u.val;
			break;
		}
	} else {
#ifdef GTP_FRAME_SIMPLE
		char	*next;

		for (next = *(char **)(gtp_frame_current + FID_SIZE); next;
		     next = *(char **)(next + FID_SIZE)) {
			if (FID(next) == FID_VAR) {
				vr = (struct gtp_frame_var *)
				     (next + FID_SIZE + sizeof(char *));
				if (vr->num == (unsigned int)num)
					goto while_stop;
			}
		}
#endif
#ifdef GTP_FTRACE_RING_BUFFER
		int				is_first = 1;
		struct ring_buffer_event	*rbe;
		char				*tmp;

		/* Handle $cpu_id and $clock.  */
		if (num == GTP_VAR_CLOCK_ID) {
			val = gtp_frame_current_clock;
			goto output_value;
		}
		else if (num == GTP_VAR_CPU_ID) {
			val = gtp_frame_current_cpu;
			goto output_value;
		}
re_find:
		while (1) {
			rbe = ring_buffer_iter_peek
				(gtp_frame_iter[gtp_frame_current_cpu], NULL);
			if (rbe == NULL)
				break;
			tmp = ring_buffer_event_data(rbe);
			if (FID(tmp) == FID_HEAD)
				break;
			if (FID(tmp) == FID_VAR) {
				vr = (struct gtp_frame_var *)(tmp + FID_SIZE);
				if (vr->num == (unsigned int)num)
					goto while_stop;
			}
			ring_buffer_read(gtp_frame_iter[gtp_frame_current_cpu],
					 NULL);
		}
		if (is_first) {
			gtp_frame_head_find_num(gtp_frame_current_num);
			ring_buffer_read(gtp_frame_iter[gtp_frame_current_cpu],
					 NULL);
			is_first = 0;
			goto re_find;
		}
#endif
#if defined(GTP_FRAME_SIMPLE) || defined(GTP_FTRACE_RING_BUFFER)
		vr = NULL;
while_stop:
		if (vr)
			val = vr->val;
	}
#endif
#ifdef GTP_RB
		if (gtp_rb_traceframe_get_tv(gtp_frame_current_rb->rp,
					     gtp_frame_current_id,
					     (unsigned int)num, &val) == 0)
			goto output_value;
	}
#endif

out:
	if (var || vr) {
output_value:
		snprintf(gtp_rw_bufp, GTP_RW_BUFP_MAX, "V%08x%08x",
			 (unsigned int) (val >> 32),
			 (unsigned int) (val & 0xffffffff));
		gtp_rw_size += strlen(gtp_rw_bufp);
		gtp_rw_bufp += strlen(gtp_rw_bufp);
	} else {
		if (GTP_RW_BUFP_MAX > 1) {
			gtp_rw_bufp[0] = 'U';
			gtp_rw_size += 1;
			gtp_rw_bufp += 1;
		}
	}

	return 1;
}

static int
gtp_gdbrsp_qT(char *pkg)
{
	int	ret = 1;

#ifdef GTP_DEBUG
	printk(GTP_DEBUG "gtp_gdbrsp_qT: %s\n", pkg);
#endif

	if (strcmp("Status", pkg) == 0)
		ret = gtp_gdbrsp_qtstatus();
	else if (strcmp("fP", pkg) == 0)
		ret = gtp_gdbrsp_qtfp();
	else if (strcmp("sP", pkg) == 0)
		ret = gtp_gdbrsp_qtsp();
	else if (strcmp("fV", pkg) == 0)
		ret = gtp_gdbrsp_qtfsv(1);
	else if (strcmp("sV", pkg) == 0)
		ret = gtp_gdbrsp_qtfsv(0);
	else if (strncmp("V:", pkg, 2) == 0)
		ret = gtp_gdbrsp_qtv(pkg + 2);

	return ret;
}

#ifdef GTP_RB
static char		*gtp_traceframe_info;
static unsigned int	gtp_traceframe_info_len;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
/* The 2.6.30 and older version have __module_address.  */

static int		gtp_modules_traceframe_info_need_get;
static char		*gtp_modules_traceframe_info;
static unsigned int	gtp_modules_traceframe_info_len;

static int
gtp_modules_traceframe_info_get(void)
{
	struct module		*mod;
	struct gtp_realloc_s	grs;
	int			ret = 0;

	gtp_realloc_alloc(&grs, 0);

	if (gtp_modules_traceframe_info_len > 0) {
		vfree(gtp_modules_traceframe_info);
		gtp_modules_traceframe_info = NULL;
		gtp_modules_traceframe_info_len = 0;
	}

	mutex_lock(&module_mutex);
	list_for_each_entry_rcu(mod, &(THIS_MODULE->list), list) {
		if (__module_address((unsigned long)mod)) {
			char	buf[70];
            unsigned long module_core;
            unsigned long core_text_size;
            //unsigned long core_size;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,5,0))
            module_core = (unsigned long)mod->core_layout.base;
            core_text_size = mod->core_layout.text_size;
            //core_size = mod->core_layout.size;
#else
            module_core = (unsigned long)mod->module_core;
            core_text_size = mod->core_text_size;
            //core_size = mod->core_size;
#endif
			snprintf(buf, 70,
				 "<memory start=\"0x%lx\" length=\"0x%lx\"/>\n",
				 (unsigned long)module_core,
				 (unsigned long)core_text_size);
			ret = gtp_realloc_str(&grs, buf, 0);
			if (ret)
				goto out;
		}
	}
	gtp_modules_traceframe_info = grs.buf;
	gtp_modules_traceframe_info_len = grs.size;
out:
	mutex_unlock(&module_mutex);
	return ret;
}
#endif

static int
gtp_traceframe_info_get(void)
{
	struct gtp_realloc_s	grs;
	int			ret;
	struct gtp_rb_walk_s	rbws;
	char			*tmp;

	if (gtp_traceframe_info_len > 0) {
		vfree(gtp_traceframe_info);
		gtp_traceframe_info = NULL;
		gtp_traceframe_info_len = 0;
	}
	/* 40 is size for "<traceframe-info>\n</traceframe-info>\n" */
	ret = gtp_realloc_alloc(&grs, 40);
	if (ret != 0)
		return ret;

	ret = gtp_realloc_str(&grs, "<traceframe-info>\n", 0);
	if (ret != 0)
		return ret;

	rbws.flags = GTP_RB_WALK_PASS_PAGE
			| GTP_RB_WALK_CHECK_END
			| GTP_RB_WALK_CHECK_ID
			| GTP_RB_WALK_CHECK_TYPE;
	rbws.end = gtp_frame_current_rb->w;
	rbws.id = gtp_frame_current_id;
	rbws.type = FID_MEM;
	tmp = gtp_frame_current_rb->rp;

	while (1) {
		struct gtp_frame_mem	*mr;
		char			buf[70];

		tmp = gtp_rb_walk(&rbws, tmp);
		if (rbws.reason != gtp_rb_walk_type)
			break;
		mr = (struct gtp_frame_mem *) (tmp + FID_SIZE);
		snprintf(buf, 70,
				"<memory start=\"0x%llx\" length=\"0x%llx\"/>\n",
				(ULONGEST)mr->addr, (ULONGEST)mr->size);
		ret = gtp_realloc_str(&grs, buf, 0);
		if (ret != 0)
			return ret;
		tmp += FRAME_ALIGN(GTP_FRAME_MEM_SIZE + mr->size);
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
	if (gtp_modules_traceframe_info_need_get) {
		int	ret = gtp_modules_traceframe_info_get();
		if (ret != 0)
			return ret;
		gtp_modules_traceframe_info_need_get = 0;
	}
	if (gtp_modules_traceframe_info_len > 0) {
		tmp = gtp_realloc(&grs, gtp_modules_traceframe_info_len, 0);
		if (tmp == NULL)
			return -ENOMEM;
		memcpy(tmp, gtp_modules_traceframe_info,
		       gtp_modules_traceframe_info_len);
	}
#endif

	ret = gtp_realloc_str(&grs, "</traceframe-info>\n", 1);
	if (ret != 0)
		return ret;

	gtp_traceframe_info = grs.buf;
	gtp_traceframe_info_len = grs.size;

	return 0;
}

#ifdef GTP_RB

static uint64_t	gtp_replay_step_id = 0;
static ULONGEST	gtp_replay_step_tpe = 0;
/* Point to the first entry of step.  */
static void	*gtp_replay_step_begin = NULL;
/* Point to the address that after last entry.  */
static void	*gtp_replay_step_end = NULL;

static void
gtp_replay_reset(void)
{
	gtp_replay_step_id = 0;
	gtp_replay_step_tpe = 0;

	gtp_rb_read_reset();
}

#endif

static int
gtp_gdbrsp_qxfer_traceframe_info_read(char *pkg)
{
	ULONGEST	offset, len;

#ifdef GTP_FRAME_SIMPLE
	if (gtp_start || !gtp_frame_current)
#endif
#ifdef GTP_FTRACE_RING_BUFFER
	if (gtp_start || gtp_frame_current_num < 0)
#endif
#ifdef GTP_RB
	/* For gtp_replay_step_tpe, the KGTP is in replay mode.  Send traceframe_info
	   will make GDB got error with access the memory.  So return -EINVAL.  */
	if (gtp_start || gtp_frame_current_num < 0 || gtp_replay_step_tpe)
#endif
		return -EINVAL;

	pkg = hex2ulongest(pkg, &offset);
	if (pkg[0] != ',')
		return -EINVAL;
	pkg++;
	pkg = hex2ulongest(pkg, &len);
	if (len == 0)
		return -EINVAL;

	if (GTP_RW_BUFP_MAX < 10)
		return -EINVAL;

	if (offset == 0) {
		int	ret = gtp_traceframe_info_get();
		if (ret != 0)
			return ret;
	}

	if (len > GTP_RW_BUFP_MAX - 1)
		len = GTP_RW_BUFP_MAX - 1;

	if (len >= gtp_traceframe_info_len - offset) {
		len = gtp_traceframe_info_len - offset;
		gtp_rw_bufp[0] = 'l';
		gtp_rw_size += 1;
		gtp_rw_bufp += 1;
	} else {
		if (GTP_RW_BUFP_MAX > 1) {
			gtp_rw_bufp[0] = 'm';
			gtp_rw_size += 1;
			gtp_rw_bufp += 1;
		}
	}

	memcpy(gtp_rw_bufp, gtp_traceframe_info + offset, len);
	gtp_rw_size += len;
	gtp_rw_bufp += len;

	return 1;
}
#endif

static uint8_t	gtp_m_buffer[0xffff];

static int
gtp_gdbrsp_m(char *pkg)
{
	int		i;
	ULONGEST	addr, len;

	/* Get add and len.  */
	if (pkg[0] == '\0')
		return -EINVAL;
	pkg = hex2ulongest(pkg, &addr);
	if (pkg[0] != ',')
		return -EINVAL;
	pkg++;
	pkg = hex2ulongest(pkg, &len);
	if (len == 0)
		return -EINVAL;
	len &= 0xffff;
	len = (ULONGEST) min((int)(GTP_RW_BUFP_MAX / 2),
			     (int)len);

#ifdef GTP_DEBUG
	printk(GTP_DEBUG "gtp_gdbrsp_m: addr = 0x%lx len = %d\n",
		(unsigned long) addr, (int) len);
#endif

#ifdef GTP_FRAME_SIMPLE
	if (gtp_start || !gtp_frame_current) {
#elif defined(GTP_FTRACE_RING_BUFFER) || defined(GTP_RB)
	if (gtp_start || gtp_frame_current_num < 0) {
#endif
		if (gtp_current_pid != gtpd_task->pid) {
			int ret = gtp_task_read(gtp_current_pid, NULL, addr,
						gtp_m_buffer, (int)len, 0);
			if (ret < 0)
				return ret;
			if (ret != len)
				return -EFAULT;

			goto out;
		} else {
			if (probe_kernel_read(gtp_m_buffer,
					      (void *)(CORE_ADDR)addr,
					      (size_t)len))
				return -EFAULT;
		}
	} else {
#ifdef GTP_FRAME_SIMPLE
		char	*next;
#endif
		int	ret;

		/* XXX: Issue 1: The following part is for gtpro support.
		   It is not available because it make disassemble cannot
		   work when select a trace frame. */
#if 0
		struct gtpro_entry	*gtroe;

		memset(gtp_m_buffer, 0, len);

		/* Read the gtpro.  */
		for (gtroe = gtpro_list; gtroe; gtroe = gtroe->next) {
			CORE_ADDR	cur_start, cur_end;

			cur_start = max(gtroe->start, (CORE_ADDR)addr);
			cur_end = min(gtroe->end, ((CORE_ADDR)(addr + len)));
			if (cur_start < cur_end) {
#ifdef GTP_DEBUG
				printk(GTP_DEBUG "gtp_gdbrsp_m: ro read "
						 "start = 0x%lx end = 0x%lx\n",
				       (unsigned long) cur_start,
				       (unsigned long) cur_end);
#endif
				if (probe_kernel_read(gtp_m_buffer,
						       (void *)cur_start,
						       (size_t)(cur_end
								- cur_start)))
					return -EFAULT;
			}
		}
#endif
		ret = probe_kernel_read(gtp_m_buffer, (void *)(CORE_ADDR)addr,
					(size_t)len);
#ifdef GTP_FRAME_SIMPLE
		for (next = *(char **)(gtp_frame_current + FID_SIZE); next;
		     next = *(char **)(next + FID_SIZE)) {
			if (FID(next) == FID_MEM) {
				struct gtp_frame_mem	*mr;
				ULONGEST		cur_start, cur_end;
				uint8_t			*buf;

				mr = (struct gtp_frame_mem *)
				     (next + FID_SIZE + sizeof(char *));
				buf = next + GTP_FRAME_MEM_SIZE;
#ifdef GTP_DEBUG
				printk(GTP_DEBUG "gtp_gdbrsp_m: section "
						 "addr = 0x%lx size = %lu\n",
				       (unsigned long) mr->addr,
				       (unsigned long) mr->size);
#endif
				cur_start = max(((ULONGEST)mr->addr), addr);
				cur_end = min(((ULONGEST)mr->addr
						+ mr->size),
					       (addr + len));
#ifdef GTP_DEBUG
				printk(GTP_DEBUG "gtp_gdbrsp_m: read "
						 "start = 0x%lx end = 0x%lx\n",
				       (unsigned long) cur_start,
				       (unsigned long) cur_end);
#endif
				if (cur_start < cur_end) {
					memcpy(gtp_m_buffer + cur_start - addr,
						buf + cur_start - mr->addr,
						cur_end - cur_start);
					ret = 0;
				}
			}
		}
#endif
#ifdef GTP_FTRACE_RING_BUFFER
		gtp_frame_head_find_num(gtp_frame_current_num);
		ring_buffer_read(gtp_frame_iter[gtp_frame_current_cpu], NULL);

		while (1) {
			struct ring_buffer_event	*rbe;
			char				*tmp;

			rbe = ring_buffer_iter_peek
				(gtp_frame_iter[gtp_frame_current_cpu], NULL);
			if (rbe == NULL)
				break;
			tmp = ring_buffer_event_data(rbe);
			if (FID(tmp) == FID_HEAD)
				break;
			if (FID(tmp) == FID_MEM) {
				struct gtp_frame_mem	*mr;
				ULONGEST		cur_start, cur_end;
				uint8_t			*buf;

				mr = (struct gtp_frame_mem *)
				     (tmp + FID_SIZE);
				buf = tmp + GTP_FRAME_MEM_SIZE;
#ifdef GTP_DEBUG
				printk(GTP_DEBUG "gtp_gdbrsp_m: section "
						 "addr = 0x%lx size = %lu\n",
				       (unsigned long) mr->addr,
				       (unsigned long) mr->size);
#endif
				cur_start = max(((ULONGEST)mr->addr), addr);
				cur_end = min(((ULONGEST)mr->addr
						+ mr->size),
					       (addr + len));
#ifdef GTP_DEBUG
				printk(GTP_DEBUG "gtp_gdbrsp_m: read "
						 "start = 0x%lx end = 0x%lx\n",
				       (unsigned long) cur_start,
				       (unsigned long) cur_end);
#endif
				if (cur_start < cur_end) {
					memcpy(gtp_m_buffer + cur_start - addr,
						buf + cur_start - mr->addr,
						cur_end - cur_start);
					ret = 0;
				}
			}
			ring_buffer_read(gtp_frame_iter[gtp_frame_current_cpu],
					 NULL);
		}
#endif
#ifdef GTP_RB
		{
			struct gtp_rb_walk_s	rbws;
			char			*tmp;

			rbws.flags = GTP_RB_WALK_PASS_PAGE
				     | GTP_RB_WALK_CHECK_END
				     | GTP_RB_WALK_CHECK_ID
				     | GTP_RB_WALK_CHECK_TYPE;
			rbws.end = gtp_frame_current_rb->w;
			rbws.id = gtp_frame_current_id;
			rbws.type = FID_MEM;
			tmp = gtp_frame_current_rb->rp;

			while (1) {
				struct gtp_frame_mem	*mr;
				ULONGEST		cur_start, cur_end;
				uint8_t			*buf;

				tmp = gtp_rb_walk(&rbws, tmp);
				if (rbws.reason != gtp_rb_walk_type)
					break;

				mr = (struct gtp_frame_mem *) (tmp + FID_SIZE);
				buf = tmp + GTP_FRAME_MEM_SIZE;
#ifdef GTP_DEBUG
				printk(GTP_DEBUG "gtp_gdbrsp_m: section "
						 "addr = 0x%lx size = %lu\n",
				       (unsigned long) mr->addr,
				       (unsigned long) mr->size);
#endif
				cur_start = max(((ULONGEST)mr->addr), addr);
				cur_end = min(((ULONGEST)mr->addr
						+ mr->size),
					       (addr + len));
#ifdef GTP_DEBUG
				printk(GTP_DEBUG "gtp_gdbrsp_m: read "
						 "start = 0x%lx end = 0x%lx\n",
				       (unsigned long) cur_start,
				       (unsigned long) cur_end);
#endif
				if (cur_start < cur_end) {
					memcpy(gtp_m_buffer + cur_start - addr,
						buf + cur_start - mr->addr,
						cur_end - cur_start);
					ret = 0;
				}

				tmp += FRAME_ALIGN(GTP_FRAME_MEM_SIZE
						   + mr->size);
			}
		}
#endif
		if (ret)
			return -EFAULT;
	}

out:
	for (i = 0; i < (int)len; i++) {
#ifdef GTP_DEBUG
		printk(GTP_DEBUG "gtp_gdbrsp_m: %d %02x\n", i, gtp_m_buffer[i]);
#endif
		sprintf(gtp_rw_bufp, "%02x", gtp_m_buffer[i]);
		gtp_rw_bufp += 2;
		gtp_rw_size += 2;
	}

	return 1;
}

#ifdef GTP_RB
static struct pt_regs *
gtp_rb_traceframe_get_regs(void)
{
	if (gtp_frame_current_regs == NULL) {
		struct gtp_rb_walk_s	rbws;
		char			*tmp;

		rbws.flags = GTP_RB_WALK_PASS_PAGE | GTP_RB_WALK_CHECK_END
			     | GTP_RB_WALK_CHECK_ID | GTP_RB_WALK_CHECK_TYPE;
		rbws.end = gtp_frame_current_rb->w;
		rbws.id = gtp_frame_current_id;
		rbws.type = FID_REG;
		tmp = gtp_rb_walk(&rbws, gtp_frame_current_rb->rp);
		if (rbws.reason == gtp_rb_walk_type)
			gtp_frame_current_regs = (struct pt_regs *)(tmp + FID_SIZE);
	}

	return gtp_frame_current_regs;
}
#endif

static int
gtp_gdbrsp_g(void)
{
#ifdef GTP_FRAME_SIMPLE
	char		*next;
#endif
	struct pt_regs	*regs;

	if (GTP_RW_BUFP_MAX < GTP_REG_ASCII_SIZE)
		return -E2BIG;

#ifdef GTP_FRAME_SIMPLE
	if (gtp_start || !gtp_frame_current) {
#elif defined(GTP_FTRACE_RING_BUFFER) || defined(GTP_RB)
	if (gtp_start || gtp_frame_current_num < 0) {
#endif
		memset(gtp_rw_bufp, '0', GTP_REG_ASCII_SIZE);
		goto out;
	}

	/* Get the regs.  */
#if defined(GTP_FTRACE_RING_BUFFER) || defined(GTP_FRAME_SIMPLE)
	regs = NULL;
#endif
#ifdef GTP_FRAME_SIMPLE
	for (next = *(char **)(gtp_frame_current + FID_SIZE); next;
	     next = *(char **)(next + FID_SIZE)) {
		if (FID(next) == FID_REG) {
			regs = (struct pt_regs *)
			       (next + FID_SIZE + sizeof(char *));
			break;
		}
	}
#endif
#ifdef GTP_FTRACE_RING_BUFFER
	{
		int				is_first = 1;
		struct ring_buffer_event	*rbe;
		char				*tmp;

re_find:
		while (1) {
			rbe = ring_buffer_iter_peek
				(gtp_frame_iter[gtp_frame_current_cpu], NULL);
			if (rbe == NULL)
				break;
			tmp = ring_buffer_event_data(rbe);
			if (FID(tmp) == FID_HEAD)
				break;
			if (FID(tmp) == FID_REG) {
				regs = (struct pt_regs *)(tmp + FID_SIZE);
				is_first = 0;
				break;
			}
			ring_buffer_read(gtp_frame_iter[gtp_frame_current_cpu],
					 NULL);
		}
		if (is_first) {
			gtp_frame_head_find_num(gtp_frame_current_num);
			ring_buffer_read(gtp_frame_iter[gtp_frame_current_cpu],
					 NULL);
			is_first = 0;
			goto re_find;
		}
	}
#endif
#ifdef GTP_RB
	regs = gtp_rb_traceframe_get_regs();
#endif
	if (regs)
		gtp_regs2ascii(regs, gtp_rw_bufp);
	else {
		struct pt_regs		pregs;
		struct gtp_entry	*tpe;

		memset(&pregs, '\0', sizeof(struct pt_regs));
		tpe = gtp_list_find_without_addr_do_check(gtp_frame_current_tpe);
		if (tpe)
			GTP_REGS_PC(&pregs) = (unsigned long)tpe->addr;
		gtp_regs2ascii(&pregs, gtp_rw_bufp);
	}
out:
	gtp_rw_bufp += GTP_REG_ASCII_SIZE;
	gtp_rw_size += GTP_REG_ASCII_SIZE;

	return 1;
}

static int
gtp_gdbrsp_vAttach(char *pkg)
{
	ULONGEST		pid;

	if (pkg[0] == '\0')
		return -EINVAL;
	pkg = hex2ulongest(pkg, &pid);
	if (pid == 0)
		return -EINVAL;

	gtp_current_pid = (pid_t)pid;

	snprintf(gtp_rw_bufp, GTP_RW_BUFP_MAX, "S05");
	gtp_rw_bufp += 3;
	gtp_rw_size += 3;
	return 1;
}

static void
gtp_gdbrsp_D(char *pkg)
{
	if (pkg[0] == ';')
		pkg++;
	if (pkg[0] == 'p')
		pkg++;

	if (pkg[0] != '\0') {
		/* Try to get pid.  */
		ULONGEST	pid;

		pkg = hex2ulongest(pkg, &pid);
		if (gtp_current_pid == (pid_t)pid)
			gtp_current_pid = 0;
	} else
		gtp_current_pid = 0;
}

/* Handle H + OP + thread-id packet. */

static int
gtp_gdbrsp_H(char *pkg)
{
	ULONGEST		pid;

	if (pkg[0] != 'g')
		return -EINVAL;
	pkg++;
	pkg = hex2ulongest(pkg, &pid);
	if (pid)
		gtp_current_pid = pid;

#ifdef GTP_RB
	if (gtp_replay_step_id)
		gtp_replay_reset();
#endif

	return 0;
}

static int
gtp_gdbrsp_qRcmd(char *pkg)
{
	int	buf_size = strlen(pkg) / 2;
	char	buf[buf_size];

	if (buf_size * 2 != strlen(pkg))
		return -EINVAL;
	hex2string(pkg, buf);

#ifdef GTP_RB
 	if (strcmp(buf, "replay") == 0) {
		if (gtp_replay_step_id) {
			string2hex ("KGTP: already in step replay mode.\n", gtp_rw_bufp);
			gtp_rw_size += strlen(gtp_rw_bufp);
			gtp_rw_bufp += strlen(gtp_rw_bufp);
			return 1;
		}

		if (gtp_start || gtp_frame_current_num < 0) {
			string2hex ("KGTP: cannot goto step replay mode because doesn't select any frame.\n", gtp_rw_bufp);
			gtp_rw_size += strlen(gtp_rw_bufp);
			gtp_rw_bufp += strlen(gtp_rw_bufp);
			return 1;
		}

		if (gtp_rb_traceframe_get_tv(gtp_frame_current_rb->rp,
					     gtp_frame_current_id,
					     GTP_STEP_ID_ID,
					     &gtp_replay_step_id)) {
			string2hex ("KGTP: cannot goto step replay mode because current frame doesn't have $step_id.\n", gtp_rw_bufp);
			gtp_rw_size += strlen(gtp_rw_bufp);
			gtp_rw_bufp += strlen(gtp_rw_bufp);
			return 1;
		}

		gtp_replay_step_tpe = gtp_frame_current_tpe;
		gtp_replay_step_begin = NULL;
		gtp_replay_step_end = NULL;

		return 0;
	} else if (strcmp(buf, "replay stop") == 0) {
		if (gtp_replay_step_id == 0) {
			string2hex ("KGTP: not in step replay mode.\n", gtp_rw_bufp);
			gtp_rw_size += strlen(gtp_rw_bufp);
			gtp_rw_bufp += strlen(gtp_rw_bufp);
			return 1;
		}

		gtp_replay_reset();

		return 0;
	}
#endif
	else if (strncmp(buf, "disable", 7) == 0) {
		char			*bufp;
		ULONGEST		addr;
		struct gtp_disable_s	*d;

		if (buf_size == 7) {
			/* monitor disable */
			/* List all disable address.  */
			struct list_head	*cur;

			string2hex ("KGTP address disable list:\n", gtp_rw_bufp);
			gtp_rw_size += strlen(gtp_rw_bufp);
			gtp_rw_bufp += strlen(gtp_rw_bufp);
			list_for_each(cur, &gtp_disable_list) {
				char	addr_buf[30];

				d = list_entry(cur, struct gtp_disable_s, node);
				snprintf(addr_buf, 30,"0x%lx\n", (unsigned long)d->addr);
				string2hex (addr_buf, gtp_rw_bufp);
				gtp_rw_size += strlen(gtp_rw_bufp);
				gtp_rw_bufp += strlen(gtp_rw_bufp);
			}
			string2hex ("Type \"monitor disable address\" to add this address to this list.\n", gtp_rw_bufp);
			gtp_rw_size += strlen(gtp_rw_bufp);
			gtp_rw_bufp += strlen(gtp_rw_bufp);
			string2hex ("Type \"monitor disable clear\" to clear this list.\n", gtp_rw_bufp);
			gtp_rw_size += strlen(gtp_rw_bufp);
			gtp_rw_bufp += strlen(gtp_rw_bufp);
			return 1;
		}

		bufp = buf + 7;
		if (bufp[0] != ' ')
			return -EBUSY;
		bufp += 1;
		if (strcmp(bufp, "clear") == 0) {
			/* monitor disable clear */
			gtp_disable_release();
			string2hex ("KGTP: address disable list is cleared.\n", gtp_rw_bufp);
			gtp_rw_size += strlen(gtp_rw_bufp);
			gtp_rw_bufp += strlen(gtp_rw_bufp);
			return 1;
		}

		/* monitor disable address */
		if (strncasecmp(bufp, "0x", 2) != 0) {
			string2hex ("KGTP: please use address in hex format.\n", gtp_rw_bufp);
			gtp_rw_size += strlen(gtp_rw_bufp);
			gtp_rw_bufp += strlen(gtp_rw_bufp);
			return 1;
		}
		bufp += 2;
		hex2ulongest(bufp, &addr);
		d = kmalloc(sizeof(*d), GFP_KERNEL);
		if (d == NULL) {
			string2hex ("KGTP: Out of memory.\n", gtp_rw_bufp);
			gtp_rw_size += strlen(gtp_rw_bufp);
			gtp_rw_bufp += strlen(gtp_rw_bufp);
			return 1;
		}
		d->addr = (CORE_ADDR)addr;
		list_add_tail(&d->node, &gtp_disable_list);
		string2hex ("KGTP: this address has been added to address disable list.\n", gtp_rw_bufp);
		gtp_rw_size += strlen(gtp_rw_bufp);
		gtp_rw_bufp += strlen(gtp_rw_bufp);
	}

	return 1;
}

struct gtp_breakpoints_s {
	struct list_head	node;
	ULONGEST		addr;
};
static LIST_HEAD(gtp_breakpoints);

static void
gtp_breakpoints_release(void)
{
	struct gtp_breakpoints_s	*b;
	struct list_head		*cur, *tmp;

	list_for_each_safe(cur, tmp, &gtp_breakpoints) {
		b = list_entry(cur, struct gtp_breakpoints_s, node);
		list_del(&b->node);
		kfree(b);
	}
}

static struct gtp_breakpoints_s *
gtp_breakpoints_find(ULONGEST addr)
{
	struct list_head	*cur;

	list_for_each(cur, &gtp_breakpoints) {
		struct gtp_breakpoints_s	*b;

		b = list_entry(cur, struct gtp_breakpoints_s, node);
		if (b->addr == addr)
			return b;
	}

	return NULL;
}

static int
gtp_gdbrsp_breakpoint(char *pkg, int insert)
{
	ULONGEST			addr;
	struct gtp_breakpoints_s	*b = NULL;

	/* Get addr.  */
	if (pkg[0] == '\0')
		return -EINVAL;
	pkg = hex2ulongest(pkg, &addr);
	if (pkg[0] != ',')
		return -EINVAL;

	if (insert) {
		b = (struct gtp_breakpoints_s *)kmalloc(sizeof(*b), GFP_KERNEL);
		if (b == NULL)
			return -ENOMEM;
		b->addr = addr;
		list_add_tail(&b->node, &gtp_breakpoints);
	} else {
		b = gtp_breakpoints_find(addr);
		if (b) {
			list_del(&b->node);
			kfree(b);
		} else
			return -EINVAL;
	}

	return 0;
}

/* Get a new entry but still not have gtp_replay_step_end.
   Check if ENTRY is belong to the current step entry list.
   If yes, return true.  */

static int
gtp_traceframe_check(void *entry)
{
	if (*(u64 *)(entry + FID_SIZE + sizeof(u64)) != gtp_replay_step_tpe) {
		/* Check the tracepoint id.  */
		return 0;
	} else {
		uint64_t	val;

		/* Check $step_id.  */
		if (gtp_rb_traceframe_get_tv(entry + FRAME_ALIGN(GTP_FRAME_HEAD_SIZE),
					     *(u64 *)(entry + FID_SIZE),
					     GTP_STEP_ID_ID, &val))
			return 0;
		if (val != gtp_replay_step_id)
			return 0;
	}

	return 1;
}

/* Step forward one step.
   Return == 0 is OK.
   Return > 0 is to the end.  */

static int
gtp_gdbrsp_step_forward(void)
{
	struct gtp_rb_walk_s	rbws;
	void			*tmp;

	if (gtp_replay_step_id == 0) {
		printk(KERN_WARNING "KGTP: not in step replay mode.\n");
		return -EBUSY;
	}

	rbws.flags = GTP_RB_WALK_PASS_PAGE | GTP_RB_WALK_CHECK_END;
	if (gtp_replay_step_end)
		rbws.end = gtp_replay_step_end;
	else
		rbws.end = gtp_frame_current_rb->w;
	tmp = gtp_rb_walk(&rbws, gtp_frame_current_rb->rp);
	if (rbws.reason != gtp_rb_walk_new_entry)
		/* To the begin of gtp_frame_current_rb
		   or to the begin of this step.
		   Both of them can be set as the begin of this step.  */
		goto end_out;
	if (gtp_replay_step_end == NULL) {
		/* Get a new entry but still not have gtp_replay_step_end.
		   Need check if this is a entry of current step first.  */
		if (!gtp_traceframe_check(tmp))
			goto end_out;
	}

	gtp_frame_current_rb->rp = tmp;
	gtp_rb_update_gtp_frame_current();
	return 0;

end_out:
	/* Set it as gtp_replay_step_end if it is not belong to
	   current step.  */
	if (gtp_replay_step_end == NULL)
		gtp_replay_step_end = tmp;
	return 1;
}

/* Step reverse one step.
   Return == 0 is OK.
   Return > 0 is to the end.  */

static int
gtp_gdbrsp_step_reverse(void)
{
	void	*tmp;
	void	*current_rp;

	if (gtp_replay_step_id == 0) {
		printk(KERN_WARNING "KGTP: not in step replay mode.\n");
		return -EBUSY;
	}

	current_rp = gtp_frame_current_rb->rp - FRAME_ALIGN(GTP_FRAME_HEAD_SIZE);
	tmp = gtp_rb_walk_reverse(current_rp,
				  gtp_replay_step_begin ? gtp_replay_step_begin
							: gtp_frame_current_rb->r);
	if (tmp == NULL)
		goto end_out;

	if (gtp_replay_step_begin == NULL) {
		/* Get a new entry but still not have gtp_replay_step_begin.
		   Need check if this is a entry of current step first.  */
		if (!gtp_traceframe_check(tmp))
			goto end_out;
	}

	gtp_frame_current_rb->rp = tmp;
	gtp_rb_update_gtp_frame_current();

	return 0;

end_out:
	if (gtp_replay_step_begin == NULL)
		gtp_replay_step_begin = current;
	return 1;
}

static int
gtp_gdbrsp_resume(int step, int reverse)
{
	int	ret = 0;

	if (gtp_replay_step_id == 0) {
		printk(KERN_WARNING "KGTP: not in step replay mode.\n");
		return -EBUSY;
	}

	do {
		if (reverse)
			ret = gtp_gdbrsp_step_reverse();
		else
			ret = gtp_gdbrsp_step_forward();

		/* Check if in the begin or end of entry list.  */
		if (ret)
			break;

		/* Check if exec stop by breakpoints.  */
		if (!step && !list_empty(&gtp_breakpoints)) {
			struct pt_regs	*regs;

			regs = gtp_rb_traceframe_get_regs();
			if (regs == NULL)
				printk(KERN_WARNING "KGTP: a traceframe doesn't include regs.\n");
			else {
				if (gtp_breakpoints_find(GTP_REGS_PC(regs)) != NULL)
					break;
			}
		}
	} while (!step);

	if (ret)
		snprintf(gtp_rw_bufp, GTP_RW_BUFP_MAX,
			 "T05replaylog:%s;", reverse ? "begin" : "end");
	else
		snprintf(gtp_rw_bufp, GTP_RW_BUFP_MAX, "S05");
	gtp_rw_size += strlen(gtp_rw_bufp);
	gtp_rw_bufp += strlen(gtp_rw_bufp);

	return 1;
}

static DEFINE_SEMAPHORE(gtp_rw_lock);
static DECLARE_WAIT_QUEUE_HEAD(gtp_rw_wq);
static unsigned int	gtp_rw_count;
static unsigned int	gtp_frame_count;

static void
gtp_frame_count_get(void)
{
	if (gtp_frame_count == 0)
		gtp_plugin_mod_get();

	gtp_frame_count++;
}

static void
gtp_frame_count_put(void)
{
	gtp_frame_count--;
	if (gtp_frame_count == 0) {
		gtp_plugin_mod_put();

		if (!gtp_disconnected_tracing) {
			gtp_gdbrsp_qtstop();
			gtp_gdbrsp_qtinit();
#ifdef GTP_RB
			if (!GTP_RB_PAGE_IS_EMPTY)
				gtp_rb_page_free();
#endif
#if defined(GTP_FRAME_SIMPLE) || defined(GTP_FTRACE_RING_BUFFER)
			if (gtp_frame) {
#ifdef GTP_FRAME_SIMPLE
				vfree(gtp_frame);
#endif
#ifdef GTP_FTRACE_RING_BUFFER
				ring_buffer_free(gtp_frame);
#endif
				gtp_frame = NULL;
			}
#endif
		}
	}
}

static int
gtp_open(struct inode *inode, struct file *file)
{
	int	ret = 0;

	down(&gtp_rw_lock);
	if (gtp_gtp_pid >= 0) {
		if (get_current()->pid != gtp_gtp_pid) {
			ret = -EBUSY;
			goto out;
		}
	}
	gtp_noack_mode = 0;

	if (gtp_rw_count == 0) {
		gtp_read_ack = 0;
		gtp_rw_buf = vmalloc(GTP_RW_MAX);
		if (!gtp_rw_buf) {
			ret = -ENOMEM;
			goto out;
		}
		gtp_rw_bufp = gtp_rw_buf;
		gtp_rw_size = 0;
	}
	gtp_rw_count++;

	gtp_frame_count_get();

	gtp_gtp_pid_count++;
	if (gtp_gtp_pid < 0)
		gtp_gtp_pid = get_current()->pid;

out:
	up(&gtp_rw_lock);
	return ret;
}

static int
gtp_release(struct inode *inode, struct file *file)
{
#ifdef GTP_DEBUG
	printk(GTP_DEBUG "gtp_release\n");
#endif
	down(&gtp_rw_lock);
	gtp_rw_count--;
	if (gtp_rw_count == 0) {
		vfree(gtp_rw_buf);

		if (gtp_replay_step_id)
			gtp_replay_reset();

		gtp_breakpoints_release();
	}

	gtp_frame_count_put();

	gtp_gtp_pid_count--;
	if (gtp_gtp_pid_count == 0) {
		gtp_current_pid = gtpd_task->pid;
		gtp_gtp_pid = -1;
	}

	up(&gtp_rw_lock);

	return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
static int
gtp_ioctl(struct inode *inode, struct file *file,
	  unsigned int cmd, unsigned long arg)
{
#ifdef GTP_DEBUG
	printk(GTP_DEBUG "gtp_ioctl: %x\n", cmd);
#endif

	return 0;
}
#else
static long
gtp_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
#ifdef GTP_DEBUG
	printk(GTP_DEBUG "gtp_ioctl: %x\n", cmd);
#endif

	return 0;
}
#endif

static ssize_t
gtp_write(struct file *file, const char __user *buf, size_t size,
	  loff_t *ppos)
{
	char		*rsppkg = NULL;
	int		i, ret;
	unsigned char	csum;
	int		is_reverse;

	if (down_interruptible(&gtp_rw_lock))
		return -EINTR;

	if (size == 0) {
#ifdef GTP_DEBUG
		printk(GTP_DEBUG "gtp_write: try write 0 size.\n");
#endif
		goto error_out;
	}

	size = min_t(size_t, size, GTP_RW_MAX);
	if (copy_from_user(gtp_rw_buf, buf, size)) {
		size = -EFAULT;
		goto error_out;
	}

	if (gtp_rw_buf[0] == '+' || gtp_rw_buf[0] == '-'
	    || gtp_rw_buf[0] == '\3' || gtp_rw_buf[0] == '\n') {
		if (gtp_rw_buf[0] == '+')
			gtp_rw_size = 0;
		size = 1;
		goto out;
	}

	if (size < 4) {
		size = -EINVAL;
		goto error_out;
	}
	/* Check format and get the rsppkg.  */
	for (i = 0; i < size - 2; i++) {
		if (gtp_rw_buf[i] == '$')
			rsppkg = gtp_rw_buf + i + 1;
		else if (gtp_rw_buf[i] == '#')
			break;
	}
	if (rsppkg && gtp_rw_buf[i] == '#') {
		/* Format is OK.  Check crc.  */
		if (gtp_noack_mode < 1)
			gtp_read_ack = 1;
		size = i + 3;
		gtp_rw_buf[i] = '\0';
	} else {
		printk(KERN_WARNING "gtp_write: format error\n");
		size = -EINVAL;
		goto error_out;
	}

	wake_up_interruptible_nr(&gtp_rw_wq, 1);

	up(&gtp_rw_lock);
	if (down_interruptible(&gtp_rw_lock))
		return -EINTR;

#ifdef GTP_DEBUG
	printk(GTP_DEBUG "gtp_write: %s\n", rsppkg);
#endif

	/* Handle rsppkg and put return to gtp_rw_buf.  */
	gtp_rw_buf[0] = '$';
	gtp_rw_bufp = gtp_rw_buf + 1;
	gtp_rw_size = 0;
	ret = 1;
	is_reverse = 0;
	switch (rsppkg[0]) {
	case '?':
		if (gtp_current_pid == 0)
			snprintf(gtp_rw_bufp, GTP_RW_BUFP_MAX, "S05");
		else
			snprintf(gtp_rw_bufp, GTP_RW_BUFP_MAX, "T05;thread:p%x.%x;",
				 gtp_current_pid, gtp_current_pid);
		gtp_rw_size += strlen(gtp_rw_bufp);
		gtp_rw_bufp += strlen(gtp_rw_bufp);
		break;
	case 'g':
		ret = gtp_gdbrsp_g();
		break;
	case 'm':
		ret = gtp_gdbrsp_m(rsppkg + 1);
		break;
	case 'Q':
#ifdef GTP_RB
		/* This check for "tfind -1" and let GDB into step replay.
		   XXX: just test on X86_64.  */
		if (gtp_replay_step_id) {
			if (strcmp("QTFrame:ffffffff", rsppkg) == 0) {
				ret = 0;
				goto switch_done;
			} else
				gtp_replay_reset();
		}
#endif
		if (rsppkg[1] == 'T')
			ret = gtp_gdbrsp_QT(rsppkg + 2);
		else if (strncmp("QStartNoAckMode", rsppkg, 15) == 0) {
			ret = 0;
			gtp_noack_mode = -1;
		}
		break;
	case 'q':
		if (rsppkg[1] == 'T')
			ret = gtp_gdbrsp_qT(rsppkg + 2);
		else if (rsppkg[1] == 'C') {
			snprintf(gtp_rw_bufp, GTP_RW_BUFP_MAX, "QC%x",
				 gtp_current_pid);
			gtp_rw_size += strlen(gtp_rw_bufp);
			gtp_rw_bufp += strlen(gtp_rw_bufp);
			ret = 1;
		} else if (strncmp("qSupported", rsppkg, 10) == 0) {
#ifdef GTP_RB
			snprintf(gtp_rw_bufp, GTP_RW_BUFP_MAX,
				 "QStartNoAckMode+;ConditionalTracepoints+;"
				 "TracepointSource+;DisconnectedTracing+;"
				 "ReverseContinue+;ReverseStep+;"
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
				 "EnableDisableTracepoints+;"
#endif
				 "qXfer:traceframe-info:read+;");
#endif
#if defined(GTP_FRAME_SIMPLE) || defined(GTP_FTRACE_RING_BUFFER)
			snprintf(gtp_rw_bufp, GTP_RW_BUFP_MAX,
				 "QStartNoAckMode+;ConditionalTracepoints+;"
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
				 "EnableDisableTracepoints+;"
#endif
				 "TracepointSource+;DisconnectedTracing+;");
#endif
			gtp_rw_size += strlen(gtp_rw_bufp);
			gtp_rw_bufp += strlen(gtp_rw_bufp);
			ret = 1;
		}
#ifdef GTP_RB
		else if (strncmp("qXfer:traceframe-info:read::",
				   rsppkg, 28) == 0)
			ret = gtp_gdbrsp_qxfer_traceframe_info_read(rsppkg
								    + 28);
#endif
		else if (strncmp("qRcmd,", rsppkg, 6) == 0)
			ret = gtp_gdbrsp_qRcmd(rsppkg + 6);
		else if (strncmp("qAttached", rsppkg, 9) == 0) {
			snprintf(gtp_rw_bufp, GTP_RW_BUFP_MAX, "1");
			gtp_rw_size += 1;
			gtp_rw_bufp += 1;
		}
		break;
	case 'S':
	case 'C':
		ret = -1;
		break;
	case 'b':
		rsppkg[0] = rsppkg[1];
		is_reverse = 1;
	case 's':
	case 'c':
		ret = gtp_gdbrsp_resume (rsppkg[0] == 's', is_reverse);
		break;
	case 'v':
		if (strncmp("vAttach;", rsppkg, 8) == 0) {
#ifdef GTP_RB
			if (gtp_replay_step_id)
				gtp_replay_reset();
#endif
			ret = gtp_gdbrsp_vAttach(rsppkg + 8);
		} else if (strncmp("vKill;", rsppkg, 7) == 0) {
#ifdef GTP_RB
			if (gtp_replay_step_id)
				gtp_replay_reset();
#endif
			/* XXX:  When we add more code to support trace
			   user space program.  We need add more release
			   code to this part.
			   Release tracepoint for this tracepoint.  */
			ret = 0;
		}
		break;
	case 'D':
#ifdef GTP_RB
		if (gtp_replay_step_id)
			gtp_replay_reset();
#endif
		gtp_gdbrsp_D(rsppkg + 1);
		ret = 0;
		break;
	case 'H':
		ret = gtp_gdbrsp_H(rsppkg + 1);
		break;
	case 'Z':
	case 'z':
		if (rsppkg[1] == '0')
			ret = gtp_gdbrsp_breakpoint(rsppkg + 3,
						    (rsppkg[0] == 'Z'));
		break;
	}
switch_done:
	if (ret == 0) {
		snprintf(gtp_rw_bufp, GTP_RW_BUFP_MAX, "OK");
		gtp_rw_bufp += 2;
		gtp_rw_size += 2;
	} else if (ret < 0) {
		snprintf(gtp_rw_bufp, GTP_RW_BUFP_MAX, "E%02x", -ret);
		gtp_rw_bufp += 3;
		gtp_rw_size += 3;
	}

	gtp_rw_bufp[0] = '#';
	csum = 0;
	for (i = 1; i < gtp_rw_size + 1; i++)
		csum += gtp_rw_buf[i];
	gtp_rw_bufp[1] = INT2CHAR(csum >> 4);
	gtp_rw_bufp[2] = INT2CHAR(csum & 0x0f);
	gtp_rw_bufp = gtp_rw_buf;
	gtp_rw_size += 4;

out:
	wake_up_interruptible_nr(&gtp_rw_wq, 1);
error_out:
	up(&gtp_rw_lock);
	return size;
}

static ssize_t
gtp_read(struct file *file, char __user *buf, size_t size,
	 loff_t *ppos)
{
	int	err;

#ifdef GTP_DEBUG
	printk(GTP_DEBUG "gtp_read\n");
#endif

	if (size == 0)
		return 0;

	if (down_interruptible(&gtp_rw_lock))
		return -EINTR;

	if (gtp_noack_mode < 1 && gtp_read_ack) {
		err = put_user('+', buf);
		if (err) {
			size = -err;
			goto out;
		}
		gtp_read_ack = 0;
		size = 1;
		if (gtp_noack_mode < 0)
			gtp_noack_mode = 1;
		goto out;
	}

	size = min(gtp_rw_size, size);
	if (size == 0)
		goto out;
	if (copy_to_user(buf, gtp_rw_bufp, size)) {
		size = -EFAULT;
		goto out;
	}
	gtp_rw_bufp += size;
	gtp_rw_size -= size;

out:
	up(&gtp_rw_lock);
	return size;
}

static unsigned int
gtp_poll(struct file *file, poll_table *wait)
{
	unsigned int	mask = POLLOUT | POLLWRNORM;

#ifdef GTP_DEBUG
	printk(GTP_DEBUG "gtp_poll\n");
#endif

	down(&gtp_rw_lock);
	poll_wait(file, &gtp_rw_wq, wait);
	if (gtp_read_ack || gtp_rw_size)
		mask |= POLLIN | POLLRDNORM;
	up(&gtp_rw_lock);

	return mask;
}

static int
gtp_frame2file_r(struct gtp_realloc_s *grs, uint32_t *data_size, char *frame)
{
	char	*wbuf;

	wbuf = gtp_realloc(grs, GTP_REG_BIN_SIZE + 1, 0);
	if (!wbuf)
		return -1;

	wbuf[0] = 'R';
#ifdef GTP_FRAME_SIMPLE
	gtp_regs2bin((struct pt_regs *)(frame + FID_SIZE + sizeof(char *)),
		     wbuf + 1);
#endif
#if defined(GTP_FTRACE_RING_BUFFER) || defined(GTP_RB)
	gtp_regs2bin((struct pt_regs *)(frame + FID_SIZE), wbuf + 1);
#endif

	*data_size += GTP_REG_BIN_SIZE + 1;

	return 0;
}

static int
gtp_frame2file_m(struct gtp_realloc_s *grs, uint32_t *data_size, char *frame)
{
	struct gtp_frame_mem	*mr;
	uint8_t			*buf;
	ULONGEST		addr;
	size_t			remaining;

#ifdef GTP_FRAME_SIMPLE
	mr = (struct gtp_frame_mem *) (frame + FID_SIZE + sizeof(char *));
#endif
#if defined(GTP_FTRACE_RING_BUFFER) || defined(GTP_RB)
	mr = (struct gtp_frame_mem *) (frame + FID_SIZE);
#endif
	buf = frame + GTP_FRAME_MEM_SIZE;
	addr = mr->addr;
	remaining = mr->size;

	while (remaining > 0) {
		uint16_t	blocklen;
		char		*wbuf;
		size_t		sp;

		blocklen = remaining > 65535 ? 65535 : remaining;

		sp = 1 + sizeof(addr) + sizeof(blocklen) + blocklen;
		wbuf = gtp_realloc(grs, sp, 0);
		if (!wbuf)
			return -1;

		wbuf[0] = 'M';
		wbuf += 1;

		memcpy(wbuf, &addr, sizeof(addr));
		wbuf += sizeof(addr);

		memcpy(wbuf, &blocklen, sizeof(blocklen));
		wbuf += sizeof(blocklen);

		memcpy(wbuf, buf, blocklen);

		addr += blocklen;
		remaining -= blocklen;
		buf += blocklen;

		*data_size += sp;
	}

	return 0;
}

static int
gtp_frame2file_v(struct gtp_realloc_s *grs, uint32_t *data_size, char *frame)
{
	struct gtp_frame_var	*vr;
	size_t			sp = 1 + sizeof(unsigned int)
				     + sizeof(uint64_t);
	char			*wbuf;

	wbuf = gtp_realloc(grs, sp, 0);
	if (!wbuf)
		return -1;

#ifdef GTP_FRAME_SIMPLE
	vr = (struct gtp_frame_var *) (frame + FID_SIZE + sizeof(char *));
#endif
#if defined(GTP_FTRACE_RING_BUFFER) || defined(GTP_RB)
	vr = (struct gtp_frame_var *) (frame + FID_SIZE);
#endif

	wbuf[0] = 'V';
	wbuf += 1;

	memcpy(wbuf, &vr->num, sizeof(unsigned int));
	wbuf += sizeof(unsigned int);

	memcpy(wbuf, &vr->val, sizeof(uint64_t));
	wbuf += sizeof(uint64_t);

	*data_size += sp;

	return 0;
}

static int
#ifdef GTP_FRAME_SIMPLE
gtp_frame2file(struct gtp_realloc_s *grs, char *frame)
#endif
#ifdef GTP_FTRACE_RING_BUFFER
gtp_frame2file(struct gtp_realloc_s *grs, int cpu)
#endif
#ifdef GTP_RB
/* gtp_frame_current_rb will step inside this function.  */
gtp_frame2file(struct gtp_realloc_s *grs)
#endif
{
	int16_t				*tmp16p;
	char				*next;
	char				*wbuf;
	uint32_t			data_size;
#ifdef GTP_FTRACE_RING_BUFFER
	struct ring_buffer_event	*rbe;
	u64				clock;
#endif
#ifdef GTP_RB
	struct gtp_rb_walk_s		rbws;
#endif

	/* Head.  */
	tmp16p = (int16_t *)gtp_realloc(grs, 2, 0);
	if (!tmp16p)
		return -1;
#ifdef GTP_FRAME_SIMPLE
	*tmp16p = (int16_t)*(ULONGEST *)(frame + FID_SIZE + sizeof(char *));
#endif
#ifdef GTP_FTRACE_RING_BUFFER
	rbe = ring_buffer_read(gtp_frame_iter[cpu], &clock);
	if (rbe == NULL) {
		/* It will not happen, just for safe.  */
		return -1;
	}
	next = ring_buffer_event_data(rbe);
	*tmp16p = (int16_t)*(ULONGEST *)(next + FID_SIZE);
#endif
#ifdef GTP_RB
	*tmp16p = (int16_t)gtp_frame_current_tpe;
#endif
	/* This part is for the data_size.  */
	wbuf = gtp_realloc(grs, 4, 0);
	if (!wbuf)
		return -1;

	/* Body.  */
	data_size = 0;

#ifdef GTP_FTRACE_RING_BUFFER
	{
		/* Handle $cpu_id and $clock.  */
		struct gtp_frame_var	*vr;
		char			frame[GTP_FRAME_VAR_SIZE];

		vr = (struct gtp_frame_var *) (frame + FID_SIZE);
		vr->num = GTP_VAR_CLOCK_ID;
		vr->val = clock;
		if (gtp_frame2file_v(grs, &data_size, frame))
			return -1;
		vr->num = GTP_VAR_CPU_ID;
		vr->val = cpu;
		if (gtp_frame2file_v(grs, &data_size, frame))
			return -1;
	}
#endif

#ifdef GTP_RB
	{
		/* Handle $cpu_id.  */
		struct gtp_frame_var	*vr;
		char			tmp[GTP_FRAME_VAR_SIZE];

		vr = (struct gtp_frame_var *) (tmp + FID_SIZE);
		vr->num = GTP_VAR_CPU_ID;
		vr->val = gtp_frame_current_rb->cpu;
		if (gtp_frame2file_v(grs, &data_size, tmp))
			return -1;
	}
#endif

#ifdef GTP_FRAME_SIMPLE
	for (next = *(char **)(frame + FID_SIZE); next;
	     next = *(char **)(next + FID_SIZE)) {
#elif defined(GTP_FTRACE_RING_BUFFER)
	while (1) {
		rbe = ring_buffer_iter_peek(gtp_frame_iter[cpu], NULL);
		if (rbe == NULL)
			break;
		next = ring_buffer_event_data(rbe);
#endif
#ifdef GTP_RB
	rbws.flags = GTP_RB_WALK_PASS_PAGE | GTP_RB_WALK_CHECK_END
		     | GTP_RB_WALK_CHECK_ID | GTP_RB_WALK_STEP;
	rbws.end = gtp_frame_current_rb->w;
	rbws.id = gtp_frame_current_id;
	rbws.step = 0;
	next = gtp_rb_walk(&rbws, gtp_frame_current_rb->rp);
	rbws.step = 1;
	while (rbws.reason == gtp_rb_walk_step) {
#endif
		switch (FID(next)) {
		case FID_REG:
			if (gtp_frame2file_r(grs, &data_size, next))
				return -1;
			break;
		case FID_MEM:
			if (gtp_frame2file_m(grs, &data_size, next))
				return -1;
			break;
		case FID_VAR:
			if (gtp_frame2file_v(grs, &data_size, next))
				return -1;
			break;
#ifdef GTP_FTRACE_RING_BUFFER
		case FID_HEAD:
			goto out;
			break;
#endif
		}
#ifdef GTP_FTRACE_RING_BUFFER
		ring_buffer_read(gtp_frame_iter[cpu], NULL);
#endif
#ifdef GTP_RB
		next = gtp_rb_walk(&rbws, next);
#endif
	}

#ifdef GTP_FTRACE_RING_BUFFER
out:
#endif
#ifdef GTP_RB
	gtp_frame_current_rb->rp = next;
#endif
	/* Set the data_size.  */
	memcpy(grs->buf + grs->size - data_size - 4,
	       &data_size, 4);

	return 0;
}

static int
gtp_frame_file_header(struct gtp_realloc_s *grs, int is_end)
{
	char			*wbuf;
	struct gtp_entry	*tpe;
	struct gtp_var		*var;
	struct list_head	*cur;
	int			tmpsize;
	int			ret = -ENOMEM;

	/* Head. */
	wbuf = gtp_realloc(grs, 8, 0);
	strcpy(wbuf, "\x7fTRACE0\n");

	/* BUG: will be a new value.  */
	wbuf = gtp_realloc(grs, 100, 0);
	if (!wbuf)
		goto out;
	snprintf(wbuf, 100, "R %x\n", GTP_REG_BIN_SIZE);
	gtp_realloc_sub_size(grs, 100 - strlen(wbuf));

	if (gtp_realloc_str(grs, "status 0;", 0))
		goto out;

	wbuf = gtp_realloc(grs, 300, 0);
	if (!wbuf)
		goto out;
	for (tpe = gtp_list; tpe; tpe = tpe->next) {
		if (tpe->reason != gtp_stop_normal)
			break;
	}
	tmpsize = gtp_get_status(tpe, wbuf, 300);
	gtp_realloc_sub_size(grs, 300 - tmpsize);

	if (gtp_realloc_str(grs, "\n", 0))
		goto out;

	/* Tval. */
	list_for_each(cur, &gtp_var_list) {
		var = list_entry(cur, struct gtp_var, node);
		wbuf = gtp_realloc(grs, 200, 0);
		if (!wbuf)
			goto out;
		snprintf(wbuf, 200, "tsv %x:%llx:%s\n", var->num,
			 (unsigned long long)var->initial_val, var->src);
		gtp_realloc_sub_size(grs, 200 - strlen(wbuf));
	}

	/* Tracepoint.  */
	for (tpe = gtp_list; tpe; tpe = tpe->next) {
		struct gtpsrc	*src;

		/* Tpe.  */
		if (gtp_realloc_str(grs, "tp ", 0))
			goto out;
		wbuf = gtp_realloc(grs, GTP_REPORT_TRACEPOINT_MAX, 0);
		if (!wbuf)
			goto out;
		gtp_report_tracepoint(tpe, wbuf, GTP_REPORT_TRACEPOINT_MAX);
		gtp_realloc_sub_size(grs,
				     GTP_REPORT_TRACEPOINT_MAX - strlen(wbuf));
		if (gtp_realloc_str(grs, "\n", 0))
			goto out;
		/* Action.  */
		for (src = tpe->action_cmd; src; src = src->next) {
			if (gtp_realloc_str(grs, "tp ", 0))
				goto out;
			tmpsize = gtp_report_action_max(tpe, src);
			wbuf = gtp_realloc(grs, tmpsize, 0);
			if (!wbuf)
				goto out;
			gtp_report_action(tpe, src, wbuf, tmpsize);
			gtp_realloc_sub_size(grs, tmpsize - strlen(wbuf));
			if (gtp_realloc_str(grs, "\n", 0))
				goto out;
		}
		/* Src.  */
		for (src = tpe->src; src; src = src->next) {
			if (gtp_realloc_str(grs, "tp ", 0))
				goto out;
			tmpsize = gtp_report_src_max(tpe, src);
			wbuf = gtp_realloc(grs, tmpsize, 0);
			if (!wbuf)
				goto out;
			gtp_report_src(tpe, src, wbuf, tmpsize);
			gtp_realloc_sub_size(grs, tmpsize - strlen(wbuf));
			if (gtp_realloc_str(grs, "\n", 0))
				goto out;
		}
	}

	if (gtp_realloc_str(grs, "\n", is_end))
		goto out;

	ret = 0;
out:
	return ret;
}

static ssize_t
gtpframe_read(struct file *file, char __user *buf, size_t size,
	      loff_t *ppos)
{
	ssize_t	ret = -ENOMEM;
#if defined(GTP_FTRACE_RING_BUFFER) || defined(GTP_RB)
	/* -2 means don't need set the frame back old number.  */
	int	old_num = -2;
#endif

recheck:
	down(&gtp_rw_lock);
	if (gtp_start) {
		up(&gtp_rw_lock);
		if (wait_event_interruptible(gtpframe_wq,
					     !gtp_start) == -ERESTARTSYS)
			return -EINTR;
#ifdef GTP_DEBUG
		printk(GTP_DEBUG "gtpframe_read: goto recheck\n");
#endif
		goto recheck;
	}

	/* Set gtp_frame_file if need.  */
	if (!gtp_frame_file) {
		char			*wbuf;
#ifdef GTP_FRAME_SIMPLE
		char			*frame;
#endif
		struct gtp_realloc_s	gr;
		int			lret;

#ifdef GTP_FRAME_SIMPLE
		if (gtp_frame_is_circular)
			gr.real_size = GTP_FRAME_SIZE;
		else
			gr.real_size = gtp_frame_w_start - gtp_frame;
#endif
#ifdef GTP_FTRACE_RING_BUFFER
		gr.real_size =
			ring_buffer_entries(gtp_frame) * GTP_FRAME_HEAD_SIZE;
#endif
#ifdef GTP_RB
		if (atomic_read(&gtp_frame_create) != 0) {
			int	cpu;

			for_each_possible_cpu(cpu) {
				struct gtp_rb_s	*rb
				= (struct gtp_rb_s *)per_cpu_ptr(gtp_rb, cpu);
				void		*tmp;
				unsigned long	flags;

				GTP_RB_LOCK_IRQ(rb, flags);
				gr.real_size = GTP_RB_END(rb->r) - rb->r;
				for (tmp = GTP_RB_NEXT(rb->r);
				     GTP_RB_HEAD(tmp) != GTP_RB_HEAD(rb->w);
				     tmp = GTP_RB_NEXT(tmp))
					gr.real_size += GTP_RB_DATA_MAX;
				gr.real_size += rb->w - GTP_RB_DATA(rb->w);
				GTP_RB_UNLOCK_IRQ(rb, flags);
			}
		}
#endif
		gr.real_size += 200;
		lret = gtp_realloc_alloc(&gr, gr.real_size);
		if (lret != 0) {
			ret = lret;
			goto out;
		}

		lret = gtp_frame_file_header(&gr, 0);
		if (lret != 0) {
			ret = lret;
			goto out;
		}

		/* Frame.  */
		if (atomic_read(&gtp_frame_create) == 0)
			goto end;
#ifdef GTP_FRAME_SIMPLE
		frame = gtp_frame_r_start;
		do {
			if (frame == gtp_frame_end)
				frame = gtp_frame;

			if (FID(frame) == FID_HEAD) {
				if (gtp_frame2file(&gr, frame))
					goto out;
			}

			frame = gtp_frame_next(frame);
			if (!frame)
				break;
		} while (frame != gtp_frame_w_start);
#endif
#ifdef GTP_FTRACE_RING_BUFFER
		old_num = gtp_frame_current_num;
		gtp_frame_iter_reset();
		while (1) {
			int	cpu;

			cpu = gtp_frame_iter_peek_head();
			if (cpu < 0)
				break;

			if (gtp_frame2file(&gr, cpu))
				goto out;
		}
#endif
#ifdef GTP_RB
		old_num = gtp_frame_current_num;
		gtp_rb_read_reset();
		while (1) {
			if (gtp_rb_read() != 0)
				break;
			gtp_frame2file(&gr);
		}
#endif

end:
		/* End.  */
		wbuf = gtp_realloc(&gr, 2, 1);
		if (!wbuf)
			goto out;
		wbuf[0] = '\0';
		wbuf[1] = '\0';

		gtp_frame_file = gr.buf;
		gtp_frame_file_size = gr.size;
	}

	/* Set buf.  */
	ret = size;
	if (*ppos + ret > gtp_frame_file_size) {
		ret = gtp_frame_file_size - *ppos;
		if (ret <= 0) {
			ret = 0;
			goto out;
		}
	}
	if (copy_to_user(buf, gtp_frame_file + *ppos, ret)) {
		ret = -EFAULT;
		goto out;
	}
	*ppos += ret;

out:
#ifdef GTP_FTRACE_RING_BUFFER
	if (old_num == -1)
		gtp_frame_iter_reset();
	else if (old_num >= 0) {
		gtp_frame_head_find_num(old_num);
		ring_buffer_read(gtp_frame_iter[gtp_frame_current_cpu], NULL);
	}
#endif
#ifdef GTP_RB
	if (old_num == -1)
		gtp_rb_reset();
	else if (old_num >= 0)
		gtp_frame_head_find_num(old_num);
#endif
	up(&gtp_rw_lock);
	return ret;
}

static int
gtpframe_open(struct inode *inode, struct file *file)
{
recheck:
	down(&gtp_rw_lock);
#ifdef GTP_RB
	if (GTP_RB_PAGE_IS_EMPTY) {
#elif defined(GTP_FRAME_SIMPLE) || defined(GTP_FTRACE_RING_BUFFER)
	if (!gtp_frame) {
#endif
		up(&gtp_rw_lock);
#ifdef GTP_RB
		if (wait_event_interruptible(gtpframe_wq,
					     !GTP_RB_PAGE_IS_EMPTY)
		    == -ERESTARTSYS)
#elif defined(GTP_FRAME_SIMPLE) || defined(GTP_FTRACE_RING_BUFFER)
		if (wait_event_interruptible(gtpframe_wq,
					     gtp_frame) == -ERESTARTSYS)
#endif
			return -EINTR;
#ifdef GTP_DEBUG
		printk(GTP_DEBUG "gtpframe_open: goto recheck\n");
#endif
		goto recheck;
	}

	if (gtp_gtpframe_pipe_pid >= 0) {
		up(&gtp_rw_lock);
		return -EBUSY;
	}

	if (gtp_gtpframe_pid >= 0) {
		if (get_current()->pid != gtp_gtpframe_pid) {
			up(&gtp_rw_lock);
			return -EBUSY;
		}
	}

	gtp_frame_count_get();

	gtp_gtpframe_pid_count++;
	if (gtp_gtpframe_pid < 0)
		gtp_gtpframe_pid = get_current()->pid;

	up(&gtp_rw_lock);
	return 0;
}

static int
gtpframe_release(struct inode *inode, struct file *file)
{
	down(&gtp_rw_lock);
	gtp_frame_count_put();

	gtp_gtpframe_pid_count--;
	if (gtp_gtpframe_pid_count == 0)
		gtp_gtpframe_pid = -1;
	up(&gtp_rw_lock);

	return 0;
}

#if defined(GTP_FTRACE_RING_BUFFER) || defined(GTP_RB)
struct gtpframe_pipe_s {
	loff_t			begin;
	struct gtp_realloc_s	*grs;
	int			llseek_move;
#ifdef GTP_RB
	void			**page;
	u64			*page_id;
#endif
};

static int
gtpframe_pipe_open(struct inode *inode, struct file *file)
{
	int			ret = -ENOMEM;
	struct gtpframe_pipe_s	*gps = NULL;

	down(&gtp_rw_lock);

	if (gtp_frame_current_num >= 0 || gtp_gtpframe_pipe_pid >= 0) {
		ret = -EBUSY;
		goto out;
	}
	gtp_gtpframe_pipe_pid = get_current()->pid;

recheck:
#ifdef GTP_RB
	if (GTP_RB_PAGE_IS_EMPTY) {
#elif defined(GTP_FTRACE_RING_BUFFER)
	if (!gtp_frame) {
#endif
		up(&gtp_rw_lock);
		atomic_inc(&gtpframe_pipe_wq_v);
#ifdef GTP_RB
		if (wait_event_interruptible(gtpframe_pipe_wq,
			!GTP_RB_PAGE_IS_EMPTY) == -ERESTARTSYS) {
#elif defined(GTP_FTRACE_RING_BUFFER)
		if (wait_event_interruptible(gtpframe_pipe_wq,
					     gtp_frame) == -ERESTARTSYS) {
#endif
			ret = -EINTR;
			goto out;
		}
#ifdef GTP_DEBUG
		printk(GTP_DEBUG "gtpframe_pipe_open: goto recheck\n");
#endif
		down(&gtp_rw_lock);
		goto recheck;
	}

	gps = kzalloc(sizeof(struct gtpframe_pipe_s), GFP_KERNEL);
	if (gps == NULL)
		goto out;
	gps->grs = kzalloc(sizeof(struct gtp_realloc_s), GFP_KERNEL);
	if (gps->grs == NULL)
		goto out;
#ifdef GTP_RB
	gps->page = kcalloc(gtp_cpu_number, sizeof(void *), GFP_KERNEL);
	if (gps->page == NULL)
		goto out;
	gps->page_id = kcalloc(gtp_cpu_number, sizeof(u64), GFP_KERNEL);
	if (gps->page_id == NULL)
		goto out;
#endif

	file->private_data = gps;

	gtp_frame_count_get();

	ret = 0;
out:
	if (ret) {
		gtp_gtpframe_pipe_pid = -1;
		if (gps) {
			kfree(gps->grs);
#ifdef GTP_RB
			kfree(gps->page);
			kfree(gps->page_id);
#endif
			kfree(gps);
		}
	}
	up(&gtp_rw_lock);
	return ret;
}

static int
gtpframe_pipe_release(struct inode *inode, struct file *file)
{
	struct gtpframe_pipe_s	*gps = file->private_data;

	down(&gtp_rw_lock);
	gtp_frame_count_put();

	gtp_gtpframe_pipe_pid = -1;

	up(&gtp_rw_lock);

	if (gps) {
#ifdef GTP_RB
		int	cpu;

		for_each_possible_cpu(cpu) {
			struct gtp_rb_s	*rb
				= (struct gtp_rb_s *)per_cpu_ptr(gtp_rb, cpu);
			if (gps->page[cpu])
				gtp_rb_put_page(rb, gps->page[cpu], 0);
		}

		kfree(gps->page);
		kfree(gps->page_id);
#endif
		if (gps->grs) {
			if (gps->grs->buf)
				vfree(gps->grs->buf);
			kfree(gps->grs);
		}
		kfree(gps);
	}

	return 0;
}

#ifdef GTP_RB
static int
gtpframe_pipe_peek(struct gtpframe_pipe_s *gps)
{
	int			cpu;
	u64			min_id = ULLONG_MAX;
	int			ret = -1;
	struct gtp_rb_walk_s	rbws;

	rbws.flags = 0;

	for_each_possible_cpu(cpu) {
		struct gtp_rb_s	*rb
			= (struct gtp_rb_s *)per_cpu_ptr(gtp_rb, cpu);

		if (gps->page_id[cpu] == 0) {
			/* Get new page.  */
			if (gps->page[cpu] == NULL) {
get_new_page:
				gps->page[cpu] = gtp_rb_get_page(rb);
				if (gps->page[cpu] == NULL)
					continue;
			}
			/* Get new entry.  */
			gps->page[cpu] = gtp_rb_walk(&rbws, gps->page[cpu]);
			if (rbws.reason != gtp_rb_walk_new_entry) {
				/* Put the page back and get a new page.  */
				gtp_rb_put_page(rb, gps->page[cpu], 1);
				goto get_new_page;
			}
			/* Get id.  */
			gps->page_id[cpu] = *(u64 *)(gps->page[cpu] + FID_SIZE);
		}

		if (gps->page_id[cpu] < min_id) {
			min_id = gps->page_id[cpu];
			ret = cpu;
		}
	}

	return ret;
}
#else
static int
gtpframe_pipe_peek(void)
{
	u64				min = 0;
	u64				ts;
	int				cpu;
	struct ring_buffer_event	*rbe;
	char				*next;
	int				ret = -1;

	for_each_possible_cpu(cpu) {
		while (1) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)) \
    && !defined(GTP_SELF_RING_BUFFER)
			rbe = ring_buffer_peek(gtp_frame, cpu, &ts);
#else
			rbe = ring_buffer_peek(gtp_frame, cpu, &ts, NULL);
#endif
			if (rbe == NULL)
				break;
			next = ring_buffer_event_data(rbe);
			if (FID(next) == FID_HEAD)
				break;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)) \
    && !defined(GTP_SELF_RING_BUFFER)
			ring_buffer_consume(gtp_frame, cpu, &ts);
#else
			ring_buffer_consume(gtp_frame, cpu, &ts, NULL);
#endif
		}

		if (rbe) {
			if ((min && ts < min) || !min) {
				min = ts;
				ret = cpu;
			}
		}
	}

	return ret;
}
#endif

static int
#ifdef GTP_RB
gtpframe_pipe_get_entry(struct gtpframe_pipe_s *gps)
#endif
#ifdef GTP_FTRACE_RING_BUFFER
gtpframe_pipe_get_entry(struct gtp_realloc_s *grs)
#endif
{
	int				cpu;
	int16_t				*tmp16p;
	uint32_t			data_size;
#ifdef GTP_FTRACE_RING_BUFFER
	char				*next;
	struct ring_buffer_event	*rbe;
	u64				ts;
#endif

#ifdef GTP_RB
	struct gtp_rb_walk_s		rbws;
	struct gtp_realloc_s		*grs = gps->grs;
#endif
	/* Because this function only be called when gtp_realloc_is_empty,
	   so grs don't need reset. */

#ifdef GTP_RB
#define GTP_PIPE_PEEK	(cpu = gtpframe_pipe_peek(gps))
#endif
#ifdef GTP_FTRACE_RING_BUFFER
recheck:
#define GTP_PIPE_PEEK	(cpu = gtpframe_pipe_peek())
#endif
	GTP_PIPE_PEEK;
	if (cpu < 0) {
		/* Didn't get the buffer that have event.
		   Wait and recheck.*/
		atomic_inc(&gtpframe_pipe_wq_v);
		if (wait_event_interruptible(gtpframe_pipe_wq,
					     GTP_PIPE_PEEK >= 0)
			== -ERESTARTSYS)
			return -EINTR;
	}
#undef GTP_PIPE_PEEK

	/* Head.  */
#ifdef GTP_FTRACE_RING_BUFFER
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)) \
    && !defined(GTP_SELF_RING_BUFFER)
	rbe = ring_buffer_consume(gtp_frame, cpu, &ts);
#else
	rbe = ring_buffer_consume(gtp_frame, cpu, &ts, NULL);
#endif
	if (rbe == NULL)
		goto recheck;
	next = ring_buffer_event_data(rbe);
	if (FID(next) != FID_HEAD)
		goto recheck;
#endif
	tmp16p = (int16_t *)gtp_realloc(grs, 2, 0);
	if (!tmp16p)
		return -ENOMEM;
#ifdef GTP_RB
	*tmp16p = (int16_t)*(ULONGEST *)(gps->page[cpu] + FID_SIZE
					 + sizeof(u64));
	gps->page[cpu] += FRAME_ALIGN(GTP_FRAME_HEAD_SIZE);
#endif
#ifdef GTP_FTRACE_RING_BUFFER
	*tmp16p = (int16_t)*(ULONGEST *)(next + FID_SIZE);
#endif
	/* This part is for the data_size.  */
	if (gtp_realloc(grs, 4, 0) == NULL)
		return -ENOMEM;
	data_size = 0;

#ifdef GTP_RB
	{
		/* Handle $cpu_id.  */
		struct gtp_frame_var	*vr;
		char			frame[GTP_FRAME_VAR_SIZE];

		vr = (struct gtp_frame_var *) (frame + FID_SIZE);
		vr->num = GTP_VAR_CPU_ID;
		vr->val = cpu;
		if (gtp_frame2file_v(grs, &data_size, frame))
			return -ENOMEM;
	}
#endif
#ifdef GTP_FTRACE_RING_BUFFER
	{
		/* Handle $cpu_id and $clock.  */
		struct gtp_frame_var	*vr;
		char			frame[GTP_FRAME_VAR_SIZE];

		vr = (struct gtp_frame_var *) (frame + FID_SIZE);
		vr->num = GTP_VAR_CLOCK_ID;
		vr->val = ts;
		if (gtp_frame2file_v(grs, &data_size, frame))
			return -ENOMEM;
		vr->num = GTP_VAR_CPU_ID;
		vr->val = cpu;
		if (gtp_frame2file_v(grs, &data_size, frame))
			return -ENOMEM;
	}
#endif

#ifdef GTP_RB
	rbws.flags = GTP_RB_WALK_CHECK_ID | GTP_RB_WALK_STEP;
	rbws.id = gps->page_id[cpu];
re_walk:
	rbws.step = 0;
	gps->page[cpu] = gtp_rb_walk(&rbws, gps->page[cpu]);
	rbws.step = 1;
	while (rbws.reason == gtp_rb_walk_step) {
		switch (FID(gps->page[cpu])) {
		case FID_REG:
			if (gtp_frame2file_r(grs, &data_size, gps->page[cpu]))
				return -ENOMEM;
			break;

		case FID_MEM:
			if (gtp_frame2file_m(grs, &data_size, gps->page[cpu]))
				return -ENOMEM;
			break;

		case FID_VAR:
			if (gtp_frame2file_v(grs, &data_size, gps->page[cpu]))
				return -ENOMEM;
			break;
		}
		gps->page[cpu] = gtp_rb_walk(&rbws, gps->page[cpu]);
	}
	if (rbws.reason == gtp_rb_walk_end_page
	    || rbws.reason == gtp_rb_walk_error) {
		/* Put this page back.  */
		gtp_rb_put_page((struct gtp_rb_s *)per_cpu_ptr(gtp_rb, cpu),
				gps->page[cpu], 1);
		gps->page[cpu] = gtp_rb_get_page((struct gtp_rb_s *)per_cpu_ptr
							(gtp_rb, cpu));
		if (gps->page[cpu])
			goto re_walk;
	}
	gps->page_id[cpu] = 0;
#endif
#ifdef GTP_FTRACE_RING_BUFFER
	while (1) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)) \
    && !defined(GTP_SELF_RING_BUFFER)
#define GTP_PIPE_CONSUME (rbe = ring_buffer_consume(gtp_frame, cpu, NULL))
#else
#define GTP_PIPE_CONSUME (rbe = ring_buffer_consume(gtp_frame, cpu, NULL, NULL))
#endif
		GTP_PIPE_CONSUME;
		if (rbe == NULL) {
			if (!gtp_start)
				break;

			atomic_inc(&gtpframe_pipe_wq_v);
			if (wait_event_interruptible(gtpframe_pipe_wq,
							GTP_PIPE_CONSUME
							!= NULL)
					== -ERESTARTSYS)
				return -EINTR;
			continue;
		}
#undef GTP_PIPE_CONSUME
		next = ring_buffer_event_data(rbe);
		switch (FID(next)) {
		case FID_REG:
			if (gtp_frame2file_r(grs, &data_size, next))
				return -ENOMEM;
			break;

		case FID_MEM:
			if (gtp_frame2file_m(grs, &data_size, next))
				return -ENOMEM;
			break;

		case FID_VAR:
			if (gtp_frame2file_v(grs, &data_size, next))
				return -ENOMEM;
			break;

		case FID_HEAD:
		case FID_END:
			goto while_out;
			break;
		}
	}
while_out:
#endif
	/* Set the data_size.  */
	memcpy(grs->buf + grs->size - data_size - 4, &data_size, 4);

	return 0;
}

static ssize_t
gtpframe_pipe_read(struct file *file, char __user *buf, size_t size,
		   loff_t *ppos)
{
	ssize_t			ret;
	struct gtpframe_pipe_s	*gps = file->private_data;
	loff_t			entry_offset;

#ifdef GTP_DEBUG
	printk(GTP_DEBUG "gtpframe_pipe_read: size=%u *ppos=%lld\n",
	       (unsigned int)size, *ppos);
#endif

	if (!gtp_realloc_is_alloced(gps->grs)) {
		ret = gtp_realloc_alloc(gps->grs, 200);
		if (ret)
			goto out;
	} else if (*ppos < gps->begin
		   || *ppos >= (gps->begin + gps->grs->size)) {
		gtp_realloc_reset(gps->grs);

		if (gps->llseek_move) {
			/* clear user will return NULL.
			   Then GDB tfind got a fail.  */
			if (size > 2)
				size = 2;
			if (clear_user(buf, size)) {
				ret = -EFAULT;
				goto out;
			}
			gps->begin = 0;
			gps->llseek_move = 0;
			ret = size;
			goto out;
		}
	}

	if (gtp_realloc_is_empty(gps->grs)) {
		if (*ppos == 0) {
			ret = gtp_frame_file_header(gps->grs, 1);
			if (ret)
				goto out;
#ifdef GTP_DEBUG
			printk(GTP_DEBUG "gtpframe_pipe_read: Get header.\n");
#endif
		} else {
#ifdef GTP_RB
			ret = gtpframe_pipe_get_entry(gps);
#endif
#ifdef GTP_FTRACE_RING_BUFFER
			ret = gtpframe_pipe_get_entry(gps->grs);
#endif
			if (ret)
				goto out;
#ifdef GTP_DEBUG
			printk(GTP_DEBUG "gtpframe_pipe_read: Get entry.\n");
#endif
		}
		gps->begin = *ppos;
	}

#ifdef GTP_DEBUG
	printk(GTP_DEBUG "gtpframe_pipe_read: gps->begin=%lld "
			 "gps->grs->size=%u\n",
	       gps->begin, (unsigned int)gps->grs->size);
#endif

	entry_offset = *ppos - gps->begin;
	ret = size;
	if (entry_offset + size > gps->grs->size)
		ret = gps->grs->size - entry_offset;
	if (copy_to_user(buf, gps->grs->buf + entry_offset, ret)) {
		ret = -EFAULT;
		goto out;
	}
	*ppos += ret;

out:
	return ret;
}

static loff_t
gtpframe_pipe_llseek(struct file *file, loff_t offset, int origin)
{
	struct gtpframe_pipe_s	*gps = file->private_data;
	loff_t			ret = default_llseek(file, offset, origin);

	if (ret < 0)
		return ret;

	/* True means that GDB tfind to next frame entry.  */
	if (ret >= gps->begin + gps->grs->size && gps->begin)
		gps->llseek_move = 1;

	return ret;
}
#endif

static const struct file_operations gtp_operations = {
	.owner		= THIS_MODULE,
	.open		= gtp_open,
	.release	= gtp_release,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
	.ioctl		= gtp_ioctl,
#else
	.unlocked_ioctl	= gtp_ioctl,
	.compat_ioctl	= gtp_ioctl,
#endif
	.read		= gtp_read,
	.write		= gtp_write,
	.poll		= gtp_poll,
};

static const struct file_operations gtpframe_operations = {
	.owner		= THIS_MODULE,
	.open		= gtpframe_open,
	.release	= gtpframe_release,
	.read		= gtpframe_read,
	.llseek		= default_llseek,
};

#if defined(GTP_FTRACE_RING_BUFFER) || defined(GTP_RB)
static const struct file_operations gtpframe_pipe_operations = {
	.owner		= THIS_MODULE,
	.open		= gtpframe_pipe_open,
	.release	= gtpframe_pipe_release,
	.read		= gtpframe_pipe_read,
	.llseek		= gtpframe_pipe_llseek,
};
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
static int
gtp_modules_load_del_notify(struct notifier_block *self, unsigned long val,
			    void *data)
{
	if (val == MODULE_STATE_COMING)
		return 0;

	down(&gtp_rw_lock);
	gtp_modules_traceframe_info_need_get = 1;
	up(&gtp_rw_lock);

	return 0;
}

static struct notifier_block	gtp_modules_load_del_nb = {
	.notifier_call = gtp_modules_load_del_notify,
};
#endif

#ifndef USE_PROC
struct dentry	*gtp_dir;
struct dentry	*gtpframe_dir;
#if defined(GTP_FTRACE_RING_BUFFER) || defined(GTP_RB)
struct dentry	*gtpframe_pipe_dir;
#endif
#endif

struct gtp_plugin_mod
{
	struct list_head	node;
	struct module 		*mod;
};

static LIST_HEAD(gtp_plugin_mod_list);

static void
gtp_plugin_mod_get(void)
{
	struct gtp_plugin_mod	*plugin;
	struct list_head	*cur;

	list_for_each(cur, &gtp_plugin_mod_list) {
		plugin = list_entry(cur, struct gtp_plugin_mod, node);
		if (!try_module_get(plugin->mod))
			printk(KERN_WARNING "Try to get KGTP plugin module fail.\n");
	}
}

static void
gtp_plugin_mod_put(void)
{
	struct gtp_plugin_mod	*plugin;
	struct list_head	*cur;

	list_for_each(cur, &gtp_plugin_mod_list) {
		plugin = list_entry(cur, struct gtp_plugin_mod, node);
		module_put(plugin->mod);
	}
}

int
gtp_plugin_mod_register(struct module *mod)
{
	int 			ret = -EBUSY;
	struct gtp_plugin_mod	*plugin;

	down(&gtp_rw_lock);

	if (gtp_frame_count || gtp_disconnected_tracing)
		goto out;

	plugin = (struct gtp_plugin_mod *)kmalloc(sizeof(*plugin),
						  GFP_KERNEL);
	if (plugin == NULL) {
		ret = -ENOMEM;
		goto out;
	}
	plugin->mod = mod;

	list_add(&plugin->node, &gtp_plugin_mod_list);

	ret = 0;
out:
	up(&gtp_rw_lock);
	return ret;
}
EXPORT_SYMBOL(gtp_plugin_mod_register);

int
gtp_plugin_mod_unregister(struct module *mod)
{
	int 			ret = -EBUSY;
	struct gtp_plugin_mod	*plugin;
	struct list_head	*cur, *tmp;

	down(&gtp_rw_lock);

	if (gtp_frame_count || gtp_disconnected_tracing)
		goto out;

	list_for_each_safe(cur, tmp, &gtp_plugin_mod_list) {
		plugin = list_entry(cur, struct gtp_plugin_mod, node);
		if (plugin->mod == mod) {
			list_del(&plugin->node);
			kfree(plugin);
			ret = 0;
			goto out;
		}
	}
	ret = -EINVAL;

out:
	up(&gtp_rw_lock);
	return ret;
}
EXPORT_SYMBOL(gtp_plugin_mod_unregister);

struct gtp_var *
gtp_plugin_var_add(char *name, int64_t val, struct gtp_var_hooks *hooks)
{
	struct gtp_var	*var = ERR_PTR(-EBUSY);

	down(&gtp_rw_lock);

	if (gtp_frame_count || gtp_disconnected_tracing)
		goto out;

	var = gtp_var_special_add(0, 1, val, name, hooks);

out:
	up(&gtp_rw_lock);
	return var;
}
EXPORT_SYMBOL(gtp_plugin_var_add);

int
gtp_plugin_var_del(struct gtp_var *var)
{
	int			ret = -EBUSY;
	struct list_head	*cur, *tmp;

	down(&gtp_rw_lock);

	if (gtp_frame_count || gtp_disconnected_tracing)
		goto out;

	list_for_each_safe(cur, tmp, &gtp_var_list) {
		if (var == list_entry(cur, struct gtp_var, node)) {
			list_del(&var->node);
			gtp_var_num--;
			kfree(var->src);
			kfree(var);
			ret = 0;
			goto out;
		}
	}

	ret = -EINVAL;

out:
	up(&gtp_rw_lock);
	return ret;
}
EXPORT_SYMBOL(gtp_plugin_var_del);

static void
gtp_release_all_mod(void)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
	unregister_module_notifier(&gtp_modules_load_del_nb);
#endif

#ifdef USE_PROC
	remove_proc_entry("gtp", NULL);
	remove_proc_entry("gtpframe", NULL);
#if defined(GTP_FTRACE_RING_BUFFER) || defined(GTP_RB)
	remove_proc_entry("gtpframe_pipe", NULL);
#endif
#else
	if (gtp_dir)
		debugfs_remove(gtp_dir);
	if (gtpframe_dir)
		debugfs_remove(gtpframe_dir);
#if defined(GTP_FTRACE_RING_BUFFER) || defined(GTP_RB)
	if (gtpframe_pipe_dir)
		debugfs_remove(gtpframe_pipe_dir);
#endif
#endif

	gtp_gdbrsp_qtstop();
	gtp_gdbrsp_qtinit();
#ifdef GTP_RB
	if (!GTP_RB_PAGE_IS_EMPTY)
		gtp_rb_page_free();
#endif
#if defined(GTP_FRAME_SIMPLE) || defined(GTP_FTRACE_RING_BUFFER)
	if (gtp_frame) {
#ifdef GTP_FRAME_SIMPLE
		vfree(gtp_frame);
#endif
#ifdef GTP_FTRACE_RING_BUFFER
		ring_buffer_free(gtp_frame);
#endif
		gtp_frame = NULL;
	}
#endif

	if (gtp_wq)
		destroy_workqueue(gtp_wq);

#ifdef GTP_RB
	gtp_rb_release();
#endif
	gtp_var_release(1);

#ifdef GTP_RB
	if (gtp_traceframe_info)
		vfree(gtp_traceframe_info);
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
	if (gtp_modules_traceframe_info)
		vfree(gtp_modules_traceframe_info);
#endif

	gtp_disable_release();
}

static int __init gtp_init(void)
{
	int		ret;

#ifdef CONFIG_X86
	gtp_have_watch_tracepoint = 0;
	gtp_have_step = 0;
#endif
	gtp_gtp_pid = -1;
	gtp_gtp_pid_count = 0;
	gtp_gtpframe_pid = -1;
	gtp_gtpframe_pid_count = 0;
#if defined(GTP_FTRACE_RING_BUFFER) || defined(GTP_RB)
	gtp_gtpframe_pipe_pid = -1;
#endif
	gtp_list = NULL;
	gtp_read_ack = 0;
	gtp_rw_bufp = NULL;
	gtp_rw_size = 0;
	gtp_start = 0;
	gtp_disconnected_tracing = 0;
	gtp_circular = 0;
#if defined(GTP_FTRACE_RING_BUFFER)			\
    && (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,39))	\
    && !defined(GTP_SELF_RING_BUFFER)
	gtp_circular_is_changed = 0;
#endif
	gtp_var_array = NULL;
	current_gtp_var = NULL;
	gtp_var_num = 0;
#if defined(GTP_FRAME_SIMPLE) || defined(GTP_FTRACE_RING_BUFFER)
	gtp_frame = NULL;
#endif
	gtp_frame_current_num = -1;
	gtp_frame_current_tpe = 0;
#ifdef GTP_FRAME_SIMPLE
	gtp_frame_r_start = NULL;
	gtp_frame_w_start = NULL;
	gtp_frame_end = NULL;
	gtp_frame_current = NULL;
	gtp_frame_is_circular = 0;
#endif
#ifdef GTP_FTRACE_RING_BUFFER
	{
		int	cpu;

		for_each_possible_cpu(cpu)
			gtp_frame_iter[cpu] = NULL;
	}
	gtp_frame_current_cpu = 0;
#endif
#if defined(GTP_FTRACE_RING_BUFFER) || defined(GTP_RB)
	atomic_set(&gtpframe_pipe_wq_v, 0);
#endif
	atomic_set(&gtp_frame_create, 0);
	gtp_rw_count = 0;
	gtp_frame_count = 0;
	current_gtp = NULL;
	current_gtp_action_cmd = NULL;
	current_gtp_src = NULL;
	gtpro_list = NULL;
	gtp_frame_file = NULL;
	gtp_frame_file_size = 0;
#ifndef USE_PROC
	gtp_dir = NULL;
	gtpframe_dir = NULL;
#if defined(GTP_FTRACE_RING_BUFFER) || defined(GTP_RB)
	gtpframe_pipe_dir = NULL;
#endif
#endif
	{
		int	cpu;

		gtp_cpu_number = 0;
		for_each_possible_cpu(cpu) {
			if (cpu > gtp_cpu_number)
				gtp_cpu_number = cpu;
		}
		gtp_cpu_number++;
	}
	gtp_start_last_errno = 0;
	gtp_start_ignore_error = 0;
	gtp_pipe_trace = 0;
	gtp_bt_size = 512;
	gtp_noack_mode = 0;
#ifdef GTP_RB
	gtp_traceframe_info = NULL;
	gtp_traceframe_info_len = 0;
#endif

#ifdef CONFIG_X86
	{
		/* Init data of while-stepping.  */
		int	cpu;
		for_each_possible_cpu(cpu) {
			struct gtp_step_s	*step = &per_cpu(gtp_step, cpu);
			spin_lock_init(&step->lock);
			step->step = 0;
			step->tpe = NULL;
		}
	}
#endif

#ifdef GTP_RB
	ret = gtp_rb_init();
	if (ret != 0)
		goto out;
#endif

	ret = -ENOMEM;
	gtp_wq = create_singlethread_workqueue("gtpd");
	if (gtp_wq == NULL)
		goto out;

	{
		struct task_struct	*p;

		/* Get the task of "gtpd".  */
		gtpd_task = NULL;
		for_each_process (p) {
			if (strcmp(p->comm, "gtpd") == 0) {
				if (gtpd_task != NULL)
					printk(KERN_WARNING "KGTP: system have more than one gtpd.\n");
				gtpd_task = p;
			}
		}
		if (gtpd_task == NULL) {
			printk(KERN_WARNING "KGTP: cannot get gtpd task.\n");
			goto out;
		}
		gtp_current_pid = gtpd_task->pid;
	}

#ifdef USE_PROC
	if (proc_create("gtp", S_IFIFO | S_IRUSR | S_IWUSR, NULL,
			&gtp_operations) == NULL)
		goto out;
	if (proc_create("gtpframe", S_IFIFO | S_IRUSR, NULL,
			&gtpframe_operations) == NULL)
		goto out;
#if defined(GTP_FTRACE_RING_BUFFER) || defined(GTP_RB)
	if (proc_create("gtpframe_pipe", S_IFIFO | S_IRUSR, NULL,
			&gtpframe_pipe_operations) == NULL)
		goto out;
#endif
#else
	ret = -ENODEV;
	gtp_dir = debugfs_create_file("gtp", S_IRUSR | S_IWUSR, NULL,
				      NULL, &gtp_operations);
	if (gtp_dir == NULL || gtp_dir == ERR_PTR(-ENODEV)) {
		gtp_dir = NULL;
		goto out;
	}
	gtpframe_dir = debugfs_create_file("gtpframe", S_IRUSR, NULL,
					   NULL, &gtpframe_operations);
	if (gtpframe_dir == NULL || gtpframe_dir == ERR_PTR(-ENODEV)) {
		gtpframe_dir = NULL;
		goto out;
	}
#if defined(GTP_FTRACE_RING_BUFFER) || defined(GTP_RB)
	gtpframe_pipe_dir = debugfs_create_file("gtpframe_pipe",
						S_IRUSR, NULL, NULL,
						&gtpframe_pipe_operations);
	if (gtpframe_pipe_dir == NULL
	    || gtpframe_pipe_dir == ERR_PTR(-ENODEV)) {
		gtpframe_pipe_dir = NULL;
		goto out;
	}
#endif
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
	gtp_modules_traceframe_info_need_get = 1;
	gtp_modules_traceframe_info = NULL;
	gtp_modules_traceframe_info_len = 0;
	if (register_module_notifier(&gtp_modules_load_del_nb))
		goto out;
#endif

	ret = gtp_var_special_add_all();
	if (ret)
		goto out;

	ret = 0;
out:
	if (ret < 0)
		gtp_release_all_mod();
#ifdef GTP_DEBUG
	printk(GTP_DEBUG "gtp inserted\n");
#endif
	return ret;
}

static void __exit gtp_exit(void)
{
	gtp_release_all_mod();
}

module_init(gtp_init)
module_exit(gtp_exit)

MODULE_AUTHOR("Hui Zhu <teawater@gmail.com>");
MODULE_LICENSE("GPL");
