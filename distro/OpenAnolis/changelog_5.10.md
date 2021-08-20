| TASK | COMMIT |
|:----:|:------:|
|     fix #34284629 | bcc3073aed1f ck: jbd2: jbd2_seq_stats_next should increase position index<br> |
|     fix #34128212 | b4addcca6bd8 ck: arm64: Pass an appropriate invalidate physical address to KVM<br> |
|     to #33162645 | 0a1b7cf3cfe3 locking/qrwlock: Fix ordering in queued_write_lock_slowpath()<br>0c6d75821ebd Revert "ck: workaround for ev lost inside epoll"<br> |
|     to #34090783 | 69c5725ad737 cpupower: Add cpuid cap flag for MSR_AMD_HWCR support<br>b886db550e96 cpupower: Remove family arg to decode_pstates()<br>2780838e1161 cpupower: Condense pstate enabled bit checks in decode_pstates()<br>d5da1bb9f277 cpupower: Update family checks when decoding HW pstates<br>25ba91e71c7c cpupower: Remove unused pscur variable.<br>c6a692336774 cpupower: Add CPUPOWER_CAP_AMD_HW_PSTATE cpuid caps flag<br>98465b92e0ab cpupower: Correct macro name for CPB caps flag<br>49916e426178 cpupower: Update msr_pstate union struct naming<br> |
|     fix #33644430 | b90d8d11c4b9 configs: x86: resize log buffer<br> |
|     to #33623156 | fd51fb721110 ck: Revert "virtio_net: enable napi_tx by default"<br> |
|     fix #33777742 | 6c8eab578de4 ck: livepatch: Add atomic replace with stop machine model<br> |
|     fix #33162645 | abbb918c6e93 ck: workaround for ev lost inside epoll<br> |
|     fix #32765374 | f75e3b0b31aa ck: blk: fix wrong usage about __rq_for_each_bio()<br> |
|     to #32137220 | e0c253243d7a ck: livepatch: support forced patching with stop machine model<br>f05cc5efc71e ck: livepatch: adjust some printing level<br> |
|     to #31434803 | 45797d0fc5b7 ck: locking/qspinlock/x86: Fix performance regression caused by virt_spin_lock<br> |
|     to #32765374 | e5f03bdbb916 ck: blk: add iohang check function<br> |
|     to #32700522 | 57dfe3daccf3 ck: block: add counter to track io request's d2c time<br> |
|     to #32137220 | f9ecc2bd4d5e ck:livepatch: remove the pre_patch_callback/post_unpatch_callback from the stop_machine context<br>a9d335c9fb1b ck: livepatch: introduce stop machine consistency model<br> |
|     to #33148959 | 830de715bac5 Intel: 5G ISA: x86: Enumerate AVX512 FP16 CPUID feature flag<br> |
|     to #33332339 | 4e636444446d Intel: perf/x86/intel: Support CPUID 10.ECX to disable fixed counters<br>f61aedba941e Intel: perf/x86/intel: Add perf core PMU support for Sapphire Rapids<br>859eefcd7526 Intel: perf/x86/intel: Filter unsupported Topdown metrics event<br>ab59bf11db11 Intel: perf/x86/intel: Factor out intel_update_topdown_event()<br>2e8eaaa4d21f Intel: perf/core: Add PERF_SAMPLE_WEIGHT_STRUCT<br>046c38e30567 Intel: perf/intel: Remove Perfmon-v4 counter_freezing support<br> |
|     to #32828251 | 30de47cc9871 Intel EDAC/i10nm: Add Intel Sapphire Rapids server support<br>d19b1b4c109c Intel: EDAC: Add DDR5 new memory type<br> |
|     to #32137220 | ec77285eb870 ck: configs: livepatch/core: introduce consistency model macro<br>5626d68f3c8d ck: livepatch/core: introduce consistency model macro<br> |
|     to #16966300 | cd02b634b50e ck: kernel: Reduce tasklist_lock contention at fork and exit<br> |
|     to #33170825 | 2c1aa0a10010 ck: cpuidle-haltpoll: Switch on cpuidle-haltpoll by default<br> |
|     to #32251972 | ef82c1e9a379 ck: configs: arm64: enable reliable stacktraces<br>e1814f1db6c9 ck: arm64: add reliable stacktraces support<br> |
|     to #27440320 | 002cf886e49d ck: sched/fair: Introduce init buffer into CFS burst<br> |
|     to #32761176 | 4b053cc91420 ck: sched/fair: Add document for burstable CFS bandwidth control<br>97a38f7eae2b ck: sched/fair: Add cfs bandwidth burst statistics<br>f4e2f5bb13bc ck: sched/fair: Make CFS bandwidth controller burstable<br>031f0d6c17f3 ck: sched/fair: Introduce primitives for CFS bandwidth burst<br> |
|     fix #33174485 | a2e9315ac83e ck: panic: change the default value of crash_kexec_post_notifiers to true<br> |
|     fix #33174410 | 6cadff40ac79 ck: sched: disable auto group by default<br> |
|     to #28505918 | 38f1ec61a076 configs: net: Enable mptcp<br> |
|     to #32281777 | 78f4f8f7f4d6 ck: arm64: Support the VCPU preemption check<br>593cbfeb3d73 ck: arm64: Add interface to support VCPU preempted check<br> |
|     fix #32281677 | 1735f5dbc1c4 ck: arm64: Re-define the hypervisor call of pv poll control<br>dbf51d1b3bff ck: arm64: Fix the incorrect base address<br>3ea05f94afc9 configs: arm64: Enable the halt polling on ARM64 architecture<br>7c7c0115e851 ck: arm64: Add new configs to support halt polling<br>dd1739e135a5 ck: cpuidle: Allow enabling the halt polling on ARM64<br>eb0184db1bad ck: arm64: Add new parameter to control halt polling<br>372005b50fa7 ck: arm64: Introduce cpuidle_haltpoll.h header file<br>f1d6720f6585 ck: arm64: Add kvm_para.h header file<br>48d02e4354ca ck: arm64: Introduce default_idle() interface<br> |
|     to #32742571 | a4e72248861d ck: nitro_enclaves: Split mmio region and increase EIF_LOAD_OFFSET<br> |
|     to #32771280 | 848a72e1a119 ck: io_uring: support multiple rings to share same poll thread by specifying same cpu<br>26f855face9b io_uring: refactor io_sq_thread() handling<br> |
|     fix #31432533,#31440055,#31391070,#31432008 | 878f9df98f4d configs: x86: change preempt model to none for preformance tuning<br> |
|     fix #32999749 | c5ac3f649161 ck: arm64: Introduce command line parameter to disable CNP<br> |
|     fix #32342536 | e624eaee9d56 configs: enable kexec signature<br> |
|     fix #32550430 | 9d5402752ce4 configs: arm64: enable pvpanic<br> |
|     fix #32986237 | 7311e119903d configs: arm64: enable gpio keys<br> |
|     fix #32242188 | 58a9b311704c configs: x86: shrink log buffer for memory saving<br> |
|     to #32823221 | 440c8e45ec33 configs: update config to add PCIE EDR support<br> |
|     fix #32695160 | d20d2204ceba ck: pci/iohub-sriov: fix iohub-sriov VF probe issue<br> |
|     fix #32742562 | 19bf574d30f2 ck: introduce deferred_meminit boot parameter<br> |
|     to #32615589 | 0a649350324c ck: random: speed up the initialization of module<br>f3976f0d5a95 ck: random: introduce the initialization seed<br> |
|     fix #32281677 | e19572b3b82a ck: arm64: Support PV poll control feature<br> |
|     to #32682818 | aa07820c26fd configs: enable config for TCP_RT<br>bcf7779dc223 ck: tcp_rt module: fix bug of using vfree to release the memory obtained by kmalloc<br>aabbc8503d8d ck: tcp_rt: add Documentation for tcp-rt<br>6c87e1526e88 ck: tcp_rt module: peer ports add more statistics<br>6a2fc3cf1ef7 ck: tcp_rt module: support pports_range<br>4869dfd1cc00 ck: tcp_rt module: change the _tcp_rt_stats item type to u64<br>026fcee3c741 ck: tcp_rt module: use atomic64_xchg replace atomic64_read and atomic64_set<br>f65ba367308a ck: tcp_rt module: save tcp rtt when R record, change the unit to us<br>5163f90fc939 ck: tcp_rt module: P record add rt and tcp reorder info<br>7aa8b2d066c4 ck: tcp_rt module: simplify the parameter name<br>da581d6377ca ck: tcp_rt module: fix repeat stats for mrtt<br>324e7092a2f5 ck: tcp_rt module: change relay work mode<br>a79c5e174270 ck: tcp_rt module: change real to stats<br>f43c3bcbb4be ck: tcp_rt module: support ports_range<br>1518d6b8ed9b ck: tcp_rt module: add tcp_rt module<br>14470a7e82e3 ck: add tcprt framework to kernel<br> |
|     to #32751521 | 332921787655 ck: mm: add an interface to adjust the penalty time dynamically<br>730756a4dac8 ck: mm: support swap.high for cgroup v1<br>e625b605b01b ck: psi: Support PSI under cgroup v1<br> |
|     to #32655467 | a330e3bb64c3 ck: mm,memcg: record latency of memcg wmark reclaim<br>14e1cd602915 ck: mm: count the memory pressure when wmark meets.<br>1fdc5e95b49a ck: mm: memcontrol: make distance between wmark_low and wmark_high configurable<br>ca87ce307778 ck: mm: vmscan: make memcg kswapd set memcg state to dirty or writeback<br>d25570a290c3 ck: mm: memcontrol: treat memcg wmark reclaim work as kswapd<br>41283ceefe76 ck: mm: memcontrol: add background reclaim support for cgroupv2<br>c8284dcd29db ck: mm: memcontrol: support background async page reclaim<br> |
|     to #32794994 | 8b1116c43d2a configs: arm64: enable tk_core cacheline aligned<br>7255551b5b4b ck: arm64: adjust tk_core memory layout<br> |
|     to #32655321 | 688703c4f6d1 ck: mm, memcg: make direct swapin latency more accurate<br>ea3acbae3a15 ck: mm, memcg: optimize eventfds with rculist<br>34103c083bfe ck: mm, memcg: introduce memsli monitor<br>1324fc449322 configs: enable CONFIG_MEMSLI<br>5a2a7489b076 ck: mm, memcg: record memory stall latency in every memcg<br> |
|     to #32682624 | c967bee29dbe ck: net: track the pid who created socks<br> |
|     to #32682687 | 165525d5a7f9 ck: virtio_net: introduce TX timeout dev_watchdog handler<br> |
|     to #31513907 | 0487598438c8 configs: bump kernel to 5.10.12<br> |
|     to #32432696 | cea8257d22c9 configs: enable CONFIG_DEBUG_INFO_BTF config<br> |
|     to #32732900 | 54d3f2f59761 configs: enable SMC as a module<br> |
|     fix #32811304 | be956feb6970 ck: udp: Add udp sockopt UDP_USE_WRITE_QUEUE<br>9a80ffa9bc2d ck: udp: add uhash4 for established connection<br> |
|     to #32811285 | eacf83d925fd configs: enable CONFIG_TCP_SYNACK_TIMEOUT_PROC<br>92445ae53fcd ck: tcp: Add sysctl for tcp syn/synack retrans timeout<br> |
|     to #32811250 | e7d0cedec16e configs: enable CONFIG_TCP_INIT_CWND_PROC<br>6c33f79ee1a2 ck: tcp: Add sysctl for TCP_INIT_CWND<br> |
|     fix #32811226 | a07bb37fd998 ck: net/sched: Output Tx queue state on timeout<br> |
|     fix #32811205 | 1556ffb27336 ck: tcp: Add feature support for tunable tcp delayed ack<br> |
|     fix #32811164 | 77bdf532aa2b ck: tcp: add tcp_tw_timeout_inherit system wide option<br> |
|     fix #32811140 | 93efda060934 ck: net/tcp: Support tunable tcp timeout value in TIME-WAIT state<br> |
|     fix #32137220 | 462d505d787c ck: hotfix: Add Kernel hotfix enhancement<br> |
|     fix #32251972 | 1b5a93eea806 ck: hookers: add arm64 dependency<br>964c561226c3 ck: Hookers: add arm64 support<br>5f97f70ae574 ck: net: kernel hookers service for toa module<br>9d68ae492639 ck: arm64: use __kernel_text_address to tell unwind<br>625417aef0be ck: arm64: add livepatch support<br>488da784c9e2 ck: Revert "kallsyms: unexport kallsyms_lookup_name() and kallsyms_on_each_symbol()"<br> |
|     fix #31801703 | e658ed0f7338 ck: mm: add ratelimit printk to prevent softlockup<br>1e5fe7434efc ck: mm: limit the print message frequency when memcg oom triggers<br>1be912d60f62 ck: mm: restrict the print message frequency further when memcg oom triggers.<br> |
|     fix #32695021 | 8dc7781e9075 ck: block: replace reserved field with extended bio_flags<br> |
|     fix #32765620 | bd208f37c8ae ck: block: fix inflight statistics of part0<br> |
|     fix #32761462 | 788d9a802726 io_uring: add timeout support for io_uring_enter()<br> |
|     fix #32762141 | 386145c33b78 io_uring: only wake up sq thread while current task is in io worker context<br> |
|     fix #32727882 | 27408fdc02aa ck: fuse: add sysfs api to flush processing queue requests<br> |
|     fix #32694299 | e3bf4e86a4d3 ck: block-throttle: only do io statistics if needed<br> |
|     fix #32700049 | 61bf6ce15222 ck: blk-throttle: limit bios to fix amount of pages entering writeback prematurely<br> |
|     to #32697813 | 7986f83dc03b ck: block-throttle: add counters for completed io<br>6d65f33a5987 ck: block-throttle: code cleanup<br>a16eca0c8c71 ck: blk-throttle: add throttled io/bytes counter<br>3b8724ca5ca3 ck: blk-throttle: fix tg NULL pointer dereference<br>71ea5bb47c78 ck: blk-throttle: support io delay stats<br> |
|     fix #32696568 | ebfb670c0826 ck: fs: record page or bio info while process is waitting on it<br> |
|     fix #32692484 | a081b6a6a512 ck: io_uring: revert io_file_supports_async()<br> |
|     fix #32765212 | b18ba6673787 dm: add support for DM_TARGET_NOWAIT for various targets<br> |
|     fix #32693463 | b9a980f41d99 ck: io_uring: support ioctl<br> |
|     fix #32695160 | cfc50354df04 configs: update for new added config for Alibaba PCIe IOHub SRIOV<br>faf473dd204f ck: pci/iohub-sriov: Support for Alibaba PCIe IOHub SRIOV<br> |
|     fix #32695316 | 0d03d34b7590 ck: mm: add proc interface to control context readahead<br> |
|     fix #32695921 | 60893ac8d536 ck: jbd2: track slow handle which is preventing transaction committing<br> |
|     fix #32823536 | 3ca82be46f6e ck: iocost: add ioc_gq stat<br> |
|     fix #32700343 | 9afae90fd1b0 ck: nvme-pci: Disable dicard zero-out functionality on Intel's P3600 NVMe disk drive<br> |
|     fix #32630110 | cd5f2e40b44e ck: memcg: Point wb to root memcg/blkcg when offlining to avoid zombie<br> |
|     fix #32728070 | bed50b96f87e ck: jbd2: add proc entry to control whether doing buffer copy-out<br> |
|     fix #32824338 | bbbbf3a4b722 ck: ext4: don't submit unwritten extent while holding active jbd2 handle<br> |
|     fix #32824162 | 4a5aa1343c1a ck: fs,ext4: remove projid limit when create hard link<br> |
|     fix #32728396 | 6654aba0835e ck: jbd2: add new "stats" proc file<br> |
|     fix #32823803 | 756dc305c064 ck: jbd2: create jbd2-ckpt thread for journal checkpoint<br> |
|     fix #32823536 | 2f2bf4ad078e ck: iocost: add legacy interface file<br> |
|     fix #32764258 | dbd13eddacf9 ck: virtio_ring: Support using kernel booting parameter when compiled as module<br> |
|     to #32764699 | 57c9f569c275 ck: fs/writeback: wrap cgroup writeback v1 logic<br>559e878f44a5 ck: writeback: introduce cgwb_v1 boot param<br>0f182225afa1 ck: fs/writeback: Attach inode's wb to root if needed<br>0751a760bf8a ck: fs/writeback: fix double free of blkcg_css<br>ad1f539cb0dc ck: writeback: add debug info for memcg-blkcg link<br>1243c96ee4cd ck: writeback: add memcg_blkcg_link tree<br> |
|     fix #32763902 | fe79f3d7ff5f ck: kconfig: Disable x86 clocksource watchdog<br> |
|     fix #32764258 | 49dbf464faf2 ck: drivers/virtio: add vring_force_dma_api boot param<br> |
|     fix #32764087 | 9f4a784bf5d7 ck: block-throttle: enable hierarchical throttling even on traditional hierarchy<br> |
|     to #31513907 | 61e945104e05 configs: arm64: disable CONFIG_ARM64_TAGGED_ADDR_ABI for performance tuning<br>b3f96381b5be configs: arm64: disable ARMv8.3/4/5 features support for performance tuning<br>4448eff04ab2 configs: arm64: disable CONFIG_ARM64_PSEUDO_NMI for performance tuning<br>7b42881ce102 configs: arm64: Apply the Cloud Kernel customized configs<br>0a39cd908ffd configs: arm64: Align to CentOS 8<br>7e044a41909f configs: x86: Apply the Cloud Kernel customized configs<br>9866cff7bac0 configs: x86: Align to CentOS 8<br>6207395ab1ed configs: Adapt to kernel 5.10<br>2609aca45c37 configs: add in-tree configs<br> |
