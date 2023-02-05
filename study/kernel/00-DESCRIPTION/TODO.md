git push gatieme :refs/tags/master
git tag -d master


https://lore.kernel.org/patchwork/cover/347617
https://lore.kernel.org/patchwork/cover/381217
https://lore.kernel.org/patchwork/patch/1454884
https://lore.kernel.org/patchwork/cover/1209888
https://lore.kernel.org/patchwork/patch/1140064


https://lore.kernel.org/patchwork/cover/1433967

https://lore.kernel.org/patchwork/patch/830591
https://lore.kernel.org/patchwork/patch/616611

https://lore.kernel.org/patchwork/project/lkml/list/?submitter=13305



| 2022/08/19 | Alexei Starovoitov <alexei.starovoitov@gmail.com> | [bpf: BPF specific memory allocator.](https://patchwork.kernel.org/project/linux-mm/cover/20220819214232.18784-1-alexei.starovoitov@gmail.com/) | 669364 | v3 ☐☑ | [LORE v3,0/15](https://lore.kernel.org/r/20220819214232.18784-1-alexei.starovoitov@gmail.com)<br>*-*-*-*-*-*-*-* <br>[LORE v4,0/15](https://lore.kernel.org/r/20220826024430.84565-1-alexei.starovoitov@gmail.com)<br>*-*-*-*-*-*-*-* <br>[LORE v5,0/15](https://lore.kernel.org/r/20220901161547.57722-1-alexei.starovoitov@gmail.com)<br>*-*-*-*-*-*-*-* <br>[LORE v6,0/16](https://lore.kernel.org/r/20220902211058.60789-1-alexei.starovoitov@gmail.com) |
| 2022/09/02 | Yafang Shao <laoar.shao@gmail.com> | [bpf: Introduce selectable memcg for bpf map](https://patchwork.kernel.org/project/linux-mm/cover/20220902023003.47124-1-laoar.shao@gmail.com/) | 673444 | v3 ☐☑ | [LORE v3,0/13](https://lore.kernel.org/r/20220902023003.47124-1-laoar.shao@gmail.com) |




| 2022/09/13 | Doug Berger <opendmb@gmail.com> | [mm: introduce Designated Movable Blocks](https://patchwork.kernel.org/project/linux-mm/cover/20220913195508.3511038-1-opendmb@gmail.com/) | 676674 | v1 ☐☑ | [LORE v1,0/21](https://lore.kernel.org/r/20220913195508.3511038-1-opendmb@gmail.com) |


[Linux RSEQ Patches Updated For Allowing Faster getCPU () In C Libraries](https://www.phoronix.com/news/Linux-RSEQ-Faster-getCPU)







| 2022/09/26 | Zhongkun He <hezhongkun.hzk@bytedance.com> | [[RFC] proc: Add a new isolated /proc/pid/mempolicy type.](https://patchwork.kernel.org/project/linux-mm/patch/20220926091033.340-1-hezhongkun.hzk@bytedance.com/) | 680392 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20220926091033.340-1-hezhongkun.hzk@bytedance.com) |

| 2022/09/29 | Shradha Gupta <shradhagupta@linux.microsoft.com> | [Configurable order free page reporting in hyper-v](https://patchwork.kernel.org/project/linux-mm/cover/1664447081-14744-1-git-send-email-shradhagupta@linux.microsoft.com/) | 681863 | v1 ☐☑ | [LORE v1,0/2](https://lore.kernel.org/r/1664447081-14744-1-git-send-email-shradhagupta@linux.microsoft.com) |
| 2022/10/04 | Yosry Ahmed <yosryahmed@google.com> | [mm/vmscan: check references from all memcgs for swapbacked memory](https://patchwork.kernel.org/project/linux-mm/patch/20221004233446.787056-1-yosryahmed@google.com/) | 683116 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221004233446.787056-1-yosryahmed@google.com) |



| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2022/10/19 | Andrei Vagin <avagin@gmail.com> | [seccomp: add the synchronous mode for seccomp_unotify](https://lore.kernel.org/all/20221020011048.156415-1-avagin@gmail.com) | TODO | v2 ☐☑✓ | [LORE v2,0/5](https://lore.kernel.org/all/20221020011048.156415-1-avagin@gmail.com) |







[Seccomp 机制与 seccomp notify 介绍](https://just4coding.com/2022/04/03/seccomp/)
26e9a1ded892 sched/debug: fix dentry leak in update_sched_domain_debugfs

git log --oneline android-mainline-5.10...HEAD | grep ANDROID | grep sched | grep -E "vendor|hook"



3882af1c70d0 ANDROID: ABI: Update symbols to unisoc whitelist for the scheduler 4st














https://lkml.org/lkml/2021/3/16/546


[Linux Patch Pending To Help Lower Alder Lake N & Raptor Lake P Power Consumption](https://www.phoronix.com/news/Linux-ADL-N-RPL-P-EPB-Tuning)







| 2022/10/26 | Joey Gouly <joey.gouly@arm.com> | [mm: In-kernel support for memory-deny-write-execute (MDWE)](https://patchwork.kernel.org/project/linux-mm/cover/20221026150457.36957-1-joey.gouly@arm.com/) | 689074 | v1 ☐☑ | [LORE RFC,0/4](https://lore.kernel.org/linux-arm-kernel/20220413134946.2732468-1-catalin.marinas@arm.com)<br>*-*-*-*-*-*-*-* <br>[LORE v1,0/2](https://lore.kernel.org/r/20221026150457.36957-1-joey.gouly@arm.com) |


| 2022/11/02 | David Hildenbrand <david@redhat.com> | [mm/autonuma: replace savedwrite infrastructure](https://patchwork.kernel.org/project/linux-mm/cover/20221102191209.289237-1-david@redhat.com/) | 参见 LPC-2022 [Copy On Write, Get User Pages, and Mysterious Counters](https://lpc.events/event/16/contributions/1267/) | v1 ☐☑ | [LORE v1,0/5](https://lore.kernel.org/r/20220926152618.194810-1-david@redhat.com)<br>*-*-*-*-*-*-*-* <br>[LORE v1,0/6](https://lore.kernel.org/r/20221102191209.289237-1-david@redhat.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/7](https://lore.kernel.org/r/20221108174652.198904-1-david@redhat.com) |






| 2021/12/06 | Pradeep P V K <quic_pragalla@quicinc.com> | [fuse: give wakeup hints to the scheduler](https://lore.kernel.org/all/1638780405-38026-1-git-send-email-quic_pragalla@quicinc.com) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/1638780405-38026-1-git-send-email-quic_pragalla@quicinc.com) |
| 2022/12/08 | Qais Yousef <qyousef@layalina.io> | [sched/fair: Fixes for capacity inversion detection](https://lore.kernel.org/all/20221208145409.453308-1-qyousef@layalina.io) | TODO | v2 ☐☑✓ | [LORE](https://lore.kernel.org/all/20221208145409.453308-1-qyousef@layalina.io) |
| 2022/12/19 | Krzysztof Kozlowski <krzysztof.kozlowski@linaro.org> | [PM: Fixes for Realtime systems](https://lore.kernel.org/all/20221219151503.385816-1-krzysztof.kozlowski@linaro.org) | TODO | v2 ☐☑✓ | [LORE v2,0/5](https://lore.kernel.org/all/20221219151503.385816-1-krzysztof.kozlowski@linaro.org) |



[Call Depth Tracking Coming To Linux 6.2 To Help Recover Performance On Skylake-Era CPUs](https://www.phoronix.com/news/Call-Depth-Tracking-Linux-6.2)


khugepage_code 将选择命中率最高的节点作为首选节点, 并尝试在多个节点具有相同命中率记录的情况下进行一些平衡. 基本上它在概念上是这样的:

* 如果目标节点 <= last_target_node, 则从 last_target_node + 1 迭代到 MAX_NUMNODES(默认配置为 1024)

* 如果 max_value == node_load[nid], 则 target_node = nid

* 但也存在一种极端情况, 特别是对于 MADV_COLLAPSE, 不存在的结点可以作为首选结点返回. 假设系统有 2 个节点, target_node 为 0,last_target_node 为 1, 如果命中 MADV_COLLAPSE 路径, max_value 可能为 0, 那么它可能为 target_node 返回 2, 但实际上它不存在 (离线), 因此触发警告.

节点均衡是由 commit 9f1b868a13ac("mm: thp: khugepaged: add policy for finding target node") 以满足 "numactl—interleave=all". 但交叉只是一种暗示, 并没有什么硬性要求.

因此, 使用 nodemask 来记录具有相同命中记录的节点, 巨大的页面分配可能会回退到这些节点. 并删除__GFP_THISNODE, 因为它不允许回退. 如果 nodemask 为空 (没有设置节点), 这意味着只有一个节点的历史记录最多, nodemask 方法实际上类似于 `__GFP_THISNODE`.

| 2022/11/03 | Yang Shi <shy828301@gmail.com> | [[v2,1/2] mm: khugepaged: allow page allocation fallback to eligible nodes](https://patchwork.kernel.org/project/linux-mm/patch/20221103213641.7296-1-shy828301@gmail.com/) | 691842 | v2 ☐☑ | [LORE v2,0/2](https://lore.kernel.org/r/20221103213641.7296-1-shy828301@gmail.com) |
| 2022/11/04 | zhaoyang.huang <zhaoyang.huang@unisoc.com> | [[RFC] mm: introduce object accounting via backtrace on slub](https://patchwork.kernel.org/project/linux-mm/patch/1667550838-10639-1-git-send-email-zhaoyang.huang@unisoc.com/) | 692001 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/1667550838-10639-1-git-send-email-zhaoyang.huang@unisoc.com) |
| 2022/11/07 | Kirill A. Shutemov <kirill.shutemov@linux.intel.com> | [[PATCHv11.1,04/16] x86/mm: Handle LAM on context switch](https://patchwork.kernel.org/project/linux-mm/patch/20221107213558.27807-1-kirill.shutemov@linux.intel.com/) | 692964 | v1 ☐☑ | [LORE v1,0/16](https://lore.kernel.org/r/20221107213558.27807-1-kirill.shutemov@linux.intel.com) |
| 2022/11/07 | Song Liu <song@kernel.org> | [execmem_alloc for BPF programs](https://patchwork.kernel.org/project/linux-mm/cover/20221107223921.3451913-1-song@kernel.org/) | 692973 | v2 ☐☑ | [LORE v2,0/5](https://lore.kernel.org/r/20221107223921.3451913-1-song@kernel.org)[LORE v4,0/6](https://lore.kernel.org/r/20221117202322.944661-1-song@kernel.org)<br>*-*-*-*-*-*-*-* <br>[LORE v5,0/6](https://lore.kernel.org/r/20221128190245.2337461-1-song@kernel.org) |
| 2022/11/08 | Lukas Czerner <lczerner@redhat.com> | [shmem: user and group quota support for tmpfs](https://patchwork.kernel.org/project/linux-mm/cover/20221108133010.75226-1-lczerner@redhat.com/) | 693244 | v1 ☐☑ | [LORE v1,0/2](https://lore.kernel.org/r/20221108133010.75226-1-lczerner@redhat.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/3](https://lore.kernel.org/r/20221121142854.91109-1-lczerner@redhat.com) |
| 2022/11/08 | Linus Torvalds <torvalds@linux-foundation.org> | [[1/4] mm: introduce 'encoded' page pointers with embedded extra bits](https://patchwork.kernel.org/project/linux-mm/patch/20221108194139.57604-1-torvalds@linux-foundation.org/) | 693373 | v1 ☐☑ | [LORE v1,0/4](https://lore.kernel.org/r/20221108194139.57604-1-torvalds@linux-foundation.org)<br>*-*-*-*-*-*-*-* <br>[LORE v1,0/4](https://lore.kernel.org/r/20221109203051.1835763-1-torvalds@linux-foundation.org) |
| 2022/11/09 | Chao Xu <amos.xuchao@gmail.com> | [mm/vmscan: simplify the nr assignment logic for pages to scan](https://patchwork.kernel.org/project/linux-mm/patch/20221109070416.620887-1-Chao.Xu9@zeekrlife.com/) | 693511 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221109070416.620887-1-Chao.Xu9@zeekrlife.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/1](https://lore.kernel.org/r/20221110113130.284290-1-Chao.Xu9@zeekrlife.com) |
| 2022/11/09 | Joao Martins <joao.m.martins@oracle.com> | [[v3] mm/hugetlb_vmemmap: remap head page to newly allocated page](https://patchwork.kernel.org/project/linux-mm/patch/20221109200623.96867-1-joao.m.martins@oracle.com/) | 693796 | v3 ☐☑ | [LORE v3,0/1](https://lore.kernel.org/r/20221109200623.96867-1-joao.m.martins@oracle.com)<br>*-*-*-*-*-*-*-* <br>[LORE v4,0/1](https://lore.kernel.org/r/20221110121214.6297-1-joao.m.martins@oracle.com|
| 2022/11/10 | Lu Jialin <lujialin4@huawei.com> | [mm/memcontrol.c: drains percpu charge caches in memory.reclaim](https://patchwork.kernel.org/project/linux-mm/patch/20221110065316.67204-1-lujialin4@huawei.com/) | 693926 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221110065316.67204-1-lujialin4@huawei.com) |
| 2022/11/17 | Song Liu <song@kernel.org> | [execmem_alloc for BPF programs](https://patchwork.kernel.org/project/linux-mm/cover/20221117202322.944661-1-song@kernel.org/) | 696625 | v4 ☐☑ | |
| 2022/11/19 | Ananda Badmaev <a.badmaev@clicknet.pro> | [mm: add zblock - new allocator for use via zpool API](https://patchwork.kernel.org/project/linux-mm/patch/20221119082159.63636-1-a.badmaev@clicknet.pro/) | 697232 | v7 ☐☑ | [LORE v6,0/1](https://lore.kernel.org/r/20221104085856.18745-1-a.badmaev@clicknet.pro)<br>*-*-*-*-*-*-*-* <br>[LORE v9,0/1](https://lore.kernel.org/r/20221202040110.14291-1-a.badmaev@clicknet.pro)[LORE v7,0/1](https://lore.kernel.org/r/20221119082159.63636-1-a.badmaev@clicknet.pro)<br>*-*-*-*-*-*-*-* <br>[LORE v8,0/1](https://lore.kernel.org/r/20221121145435.41002-1-a.badmaev@clicknet.pro) |
| 2022/11/21 | Vlastimil Babka <vbabka@suse.cz> | [Introduce CONFIG_SLUB_TINY and deprecate SLOB](https://patchwork.kernel.org/project/linux-mm/cover/20221121171202.22080-1-vbabka@suse.cz/)| 697743 | v1 ☐☑ | [LORE v1,0/12](https://lore.kernel.org/r/20221121171202.22080-1-vbabka@suse.cz) |
| 2022/11/21 | Aleksey Romanov <avromanov@sberdevices.ru> | [Introduce merge identical pages mechanism](https://patchwork.kernel.org/project/linux-mm/cover/20221121190020.66548-1-avromanov@sberdevices.ru/) | 697795 | v1 ☐☑ | [LORE v1,0/4](https://lore.kernel.org/r/20221121190020.66548-1-avromanov@sberdevices.ru) |
| 2022/11/22 | Mina Almasry <almasrymina@google.com> | [[RFC,v1] mm: Add memory.demote for proactive demotion only](https://patchwork.kernel.org/project/linux-mm/patch/20221122203850.2765015-2-almasrymina@google.com/) | 698229 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221122203850.2765015-2-almasrymina@google.com) |
| 2022/11/24 | Jiasheng Jiang <jiasheng@iscas.ac.cn> | [mm/vmalloc: Add check for KMEM_CACHE](https://patchwork.kernel.org/project/linux-mm/patch/20221124040226.17953-1-jiasheng@iscas.ac.cn/) | 698747 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221124040226.17953-1-jiasheng@iscas.ac.cn) |
| 2022/11/25 | Peter Xu <peterx@redhat.com> | [mm/thp: Re-apply mkdirty for small pages after split](https://patchwork.kernel.org/project/linux-mm/patch/20221125185857.3110155-1-peterx@redhat.com/) | 699295 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221125185857.3110155-1-peterx@redhat.com) |
| 2022/11/25 | Jann Horn <jannh@google.com> | [[v3,1/3] mm/khugepaged: Take the right locks for page table retraction](https://patchwork.kernel.org/project/linux-mm/patch/20221125213714.4115729-1-jannh@google.com/) | 699315 | v3 ☐☑ | [LORE v3,0/3](https://lore.kernel.org/r/20221125213714.4115729-1-jannh@google.com)<br>*-*-*-*-*-*-*-* <br>[LORE v4,0/3](https://lore.kernel.org/r/20221128180252.1684965-1-jannh@google.com) |[LORE v4,0/3](https://lore.kernel.org/r/20221128180252.1684965-1-jannh@google.com)<br>*-*-*-*-*-*-*-* <br>[LORE v5,0/3](https://lore.kernel.org/r/20221129154730.2274278-1-jannh@google.com) |
| 2022/11/28 | Alexander Potapenko <glider@google.com> | [[1/2] lockdep: allow instrumenting lockdep.c with KMSAN](https://patchwork.kernel.org/project/linux-mm/patch/20221128094541.2645890-1-glider@google.com/) | 699627 | v1 ☐☑ | [LORE v1,0/2](https://lore.kernel.org/r/20221128094541.2645890-1-glider@google.com) |
| 2022/11/28 | Brian Foster <bfoster@redhat.com> | [filemap: skip write and wait if end offset precedes start](https://patchwork.kernel.org/project/linux-mm/cover/20221128155632.3950447-1-bfoster@redhat.com/) | 699767 | v2 ☐☑ | [LORE v2,0/2](https://lore.kernel.org/r/20221128155632.3950447-1-bfoster@redhat.com) |
| 2022/11/29 | Liam Howlett <Liam.Howlett@Oracle.com> | [VMA type safety through VMA iterator](https://patchwork.kernel.org/project/linux-mm/cover/20221129164352.3374638-1-Liam.Howlett@oracle.com/) | 700163 | v1 ☐☑ | [LORE v1,0/43](https://lore.kernel.org/r/20221129164352.3374638-1-Liam.Howlett@oracle.com) |
| 2022/11/30 |  <ye.xingchen@zte.com.cn> | [mm/huge_memory: add TRANSPARENT_HUGEPAGE_NEVER for THP](https://patchwork.kernel.org/project/linux-mm/patch/202211301651462590168@zte.com.cn/) | 700357 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/202211301651462590168@zte.com.cn) |
| 2022/11/30 | Matthew Wilcox <willy@infradead.org> | [Split page pools from struct page](https://patchwork.kernel.org/project/linux-mm/cover/20221130220803.3657490-1-willy@infradead.org/) | 700614 | v1 ☐☑ | [LORE v1,0/24](https://lore.kernel.org/r/20221130220803.3657490-1-willy@infradead.org) |
| 2022/12/01 | Mina Almasry <almasrymina@google.com> | [[v1] mm: disable top-tier fallback to reclaim on proactive reclaim](https://patchwork.kernel.org/project/linux-mm/patch/20221201233317.1394958-1-almasrymina@google.com/) | 701011 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221201233317.1394958-1-almasrymina@google.com) |
| 2022/12/02 | Alistair Popple <apopple@nvidia.com> | [mm/mmap: Properly unaccount memory on mas_preallocate() failure](https://patchwork.kernel.org/project/linux-mm/patch/20221202045339.2999017-1-apopple@nvidia.com/) | 701081 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221202045339.2999017-1-apopple@nvidia.com) |
| 2022/12/02 | David Hildenbrand <david@redhat.com> | [[RFC] mm/userfaultfd: enable writenotify while userfaultfd-wp is enabled for a VMA](https://patchwork.kernel.org/project/linux-mm/patch/20221202122748.113774-1-david@redhat.com/) | 701236 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221202122748.113774-1-david@redhat.com) |
| 2022/12/02 | Brian Foster <bfoster@redhat.com> | [proc: improve root readdir latency with many threads](https://patchwork.kernel.org/project/linux-mm/cover/20221202171620.509140-1-bfoster@redhat.com/) | 701317 | v3 ☐☑ | [LORE v3,0/5](https://lore.kernel.org/r/20221202171620.509140-1-bfoster@redhat.com) |
| 2022/11/09 | Baoquan He <bhe@redhat.com> | [mm/vmalloc.c: allow vread() to read out vm_map_ram areas](https://patchwork.kernel.org/project/linux-mm/cover/20221109033535.269229-1-bhe@redhat.com/) | 693476 | v1 ☐☑ | [LORE v1,0/3](https://lore.kernel.org/r/20221109033535.269229-1-bhe@redhat.com)<br>*-*-*-*-*-*-*-* <br>[LORE v1,0/7](https://lore.kernel.org/r/20221204013046.154960-1-bhe@redhat.com) |
| 2022/12/05 | Li,Rongqing <lirongqing@baidu.com> | [mm: memcontrol: speedup memory cgroup resize](https://patchwork.kernel.org/project/linux-mm/patch/1670240992-28563-1-git-send-email-lirongqing@baidu.com/) | 701772 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/1670240992-28563-1-git-send-email-lirongqing@baidu.com) |
| 2022/12/05 | Stephen Boyd <swboyd@chromium.org> | [pstore: Avoid kcore oops by vmap()ing with VM_IOREMAP](https://patchwork.kernel.org/project/linux-mm/patch/20221205233136.3420802-1-swboyd@chromium.org/) | 702003 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221205233136.3420802-1-swboyd@chromium.org) |
| 2022/12/06 | Fabio M. De Francesco <fmdefrancesco@gmail.com> | [mm/highmem: Add notes about conversions from kmap{,_atomic}()](https://patchwork.kernel.org/project/linux-mm/patch/20221206070029.7342-1-fmdefrancesco@gmail.com/) | 702076 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221206070029.7342-1-fmdefrancesco@gmail.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/1](https://lore.kernel.org/r/20221207225308.8290-1-fmdefrancesco@gmail.com) |
| 2022/12/06 | David Hildenbrand <david@redhat.com> | [mm: support `__HAVE_ARCH_PTE_SWP_EXCLUSIVE` on all architectures with swap PTEs](https://patchwork.kernel.org/project/linux-mm/cover/20221206144730.163732-1-david@redhat.com/) | 702222 | v1 ☐☑ | [LORE v1,0/26](https://lore.kernel.org/r/20221206144730.163732-1-david@redhat.com) |
| 2022/12/06 | Nico Pache <npache@redhat.com> | [[RFC] vmscan: Scale file_is_tiny calculation based on priority](https://patchwork.kernel.org/project/linux-mm/patch/20221206222315.37631-1-npache@redhat.com/) | 702319 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221206222315.37631-1-npache@redhat.com) |
| 2022/12/07 | Vishal Moola (Oracle) <vishal.moola@gmail.com> | [Convert deactivate_page() to deactivate_folio()](https://patchwork.kernel.org/project/linux-mm/cover/20221207002158.418789-1-vishal.moola@gmail.com/) | 702344 | v1 ☐☑ | [LORE v1,0/3](https://lore.kernel.org/r/20221207002158.418789-1-vishal.moola@gmail.com) |
| 2022/12/07 | Kefeng Wang <wangkefeng.wang@huawei.com> | [[1/2] mm: huge_memory: Convert madvise_free_huge_pmd to use a folio](https://patchwork.kernel.org/project/linux-mm/patch/20221207023431.151008-1-wangkefeng.wang@huawei.com/) | 702384 | v1 ☐☑ | [LORE v1,0/2](https://lore.kernel.org/r/20221207023431.151008-1-wangkefeng.wang@huawei.com) |
| 2022/12/08 | chengkaitao <pilgrimtao@gmail.com> | [[v2] mm: memcontrol: protect the memory in cgroup from being oom killed](https://patchwork.kernel.org/project/linux-mm/patch/20221208034644.3077-1-chengkaitao@didiglobal.com/) | 702809 | v2 ☐☑ | [LORE v2,0/1](https://lore.kernel.org/r/20221208034644.3077-1-chengkaitao@didiglobal.com) |
| 2022/12/08 | David Hildenbrand <david@redhat.com> | [[v1] mm/userfaultfd: enable writenotify while userfaultfd-wp is enabled for a VMA](https://patchwork.kernel.org/project/linux-mm/patch/20221208114137.35035-1-david@redhat.com/) | 702923 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221208114137.35035-1-david@redhat.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/1](https://lore.kernel.org/r/20221209080912.7968-1-david@redhat.com) |
| 2022/12/08 | Wenchao Hao <haowenchao@huawei.com> | [cma:tracing: Print alloc result in trace_cma_alloc_finish](https://patchwork.kernel.org/project/linux-mm/patch/20221208142130.1501195-1-haowenchao@huawei.com/) | 702989 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221208142130.1501195-1-haowenchao@huawei.com) |
| 2022/12/08 | Peter Xu <peterx@redhat.com> | [mm/uffd: Always wr-protect pte in pte|pmd_mkuffd_wp()](https://patchwork.kernel.org/project/linux-mm/patch/20221208194628.766316-1-peterx@redhat.com/) | 703082 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221208194628.766316-1-peterx@redhat.com) |
| 2022/12/09 | Xander <xandermoerkerken@gmail.com> | [Added ability to vmalloc executable memory](https://patchwork.kernel.org/project/linux-mm/patch/20221209131052.64235-1-xander.moerkerken@omron.com/) | 703287 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221209131052.64235-1-xander.moerkerken@omron.com) |
| 2022/12/09 | Nhat Pham <nphamcs@gmail.com> | [[v3,2/4] workingset: refactor LRU refault to expose refault recency check](https://patchwork.kernel.org/project/linux-mm/patch/20221209172922.3143160-1-nphamcs@gmail.com/) | 703468 | v3 ☐☑ | [LORE v3,0/4](https://lore.kernel.org/r/20221209172922.3143160-1-nphamcs@gmail.com) |
| 2022/12/12 | Yafang Shao <laoar.shao@gmail.com> | [mm, bpf: Add BPF into /proc/meminfo](https://patchwork.kernel.org/project/linux-mm/cover/20221212003711.24977-1-laoar.shao@gmail.com/) | 703689 | v1 ☐☑ | [LORE v1,0/9](https://lore.kernel.org/r/20221212003711.24977-1-laoar.shao@gmail.com) |
| 2022/12/12 | David Hildenbrand <david@redhat.com> | [[mm-stable] mm/gup_test: free memory allocated via kvcalloc() using kvfree()](https://patchwork.kernel.org/project/linux-mm/patch/20221212182018.264900-1-david@redhat.com/) | 703907 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221212182018.264900-1-david@redhat.com) |
| 2022/12/12 | Jason A. Donenfeld <Jason@zx2c4.com> | [mm: add VM_DROPPABLE for designating always lazily freeable mappings](https://patchwork.kernel.org/project/linux-mm/patch/20221212185347.1286824-2-Jason@zx2c4.com/) | 703918 | v12 ☐☑ | [LORE v12,0/6](https://lore.kernel.org/r/20221212185347.1286824-2-Jason@zx2c4.com)[LORE v14,0/7](https://lore.kernel.org/r/20230101162910.710293-3-Jason@zx2c4.com) |
| 2022/12/12 | Mike Kravetz <mike.kravetz@oracle.com> | [[1/2] hugetlb: really allocate vma lock for all sharable vmas](https://patchwork.kernel.org/project/linux-mm/patch/20221212235042.178355-1-mike.kravetz@oracle.com/) | 703972 | v1 ☐☑ | [LORE v1,0/2](https://lore.kernel.org/r/20221212235042.178355-1-mike.kravetz@oracle.com) |
| 2022/12/13 | Kefeng Wang <wangkefeng.wang@huawei.com> | [[-next,v3] mm: hwposion: support recovery from ksm_might_need_to_copy()](https://patchwork.kernel.org/project/linux-mm/patch/20221213030557.143432-1-wangkefeng.wang@huawei.com/) | 703997 | v3 ☐☑ | [LORE v3,0/1](https://lore.kernel.org/r/20221213030557.143432-1-wangkefeng.wang@huawei.com) |
| 2022/12/13 | Michal Hocko <mhocko@suse.com> | [memcg reclaim demotion wrt. isolation](https://patchwork.kernel.org/project/linux-mm/patch/Y5idFucjKVbjatqc@dhcp22.suse.cz/) | 704194 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/Y5idFucjKVbjatqc@dhcp22.suse.cz) |
| 2022/12/13 | Nico Pache <npache@redhat.com> | [[RFC,V2] mm: add the zero case to page[1].compound_nr in set_compound_order](https://patchwork.kernel.org/project/linux-mm/patch/20221213234505.173468-1-npache@redhat.com/) | 704325 | v2 ☐☑ | [LORE v2,0/1](https://lore.kernel.org/r/20221213234505.173468-1-npache@redhat.com) |
| 2022/12/14 | Peter Xu <peterx@redhat.com> | [[v2] mm/uffd: Always wr-protect pte in pte|pmd_mkuffd_wp()](https://patchwork.kernel.org/project/linux-mm/patch/20221214201533.1774616-1-peterx@redhat.com/) | 704573 | v2 ☐☑ | [LORE v2,0/1](https://lore.kernel.org/r/20221214201533.1774616-1-peterx@redhat.com) |
| 2022/12/14 | Yuanchu Xie <yuanchu@google.com> | [mm: multi-gen LRU: working set extensions](https://patchwork.kernel.org/project/linux-mm/cover/20221214225123.2770216-1-yuanchu@google.com/) | 704609 | v1 ☐☑ | [LORE v1,0/2](https://lore.kernel.org/r/20221214225123.2770216-1-yuanchu@google.com) |
| 2022/12/17 | Yafang Shao <laoar.shao@gmail.com> | [mm: page_ext: split page_ext flags](https://patchwork.kernel.org/project/linux-mm/cover/20221217105833.24851-1-laoar.shao@gmail.com/) | 705332 | v1 ☐☑ | [LORE v1,0/2](https://lore.kernel.org/r/20221217105833.24851-1-laoar.shao@gmail.com) |
| 2022/12/19 | Hao Sun <sunhao.th@gmail.com> | [mm: new primitive kvmemdup()](https://patchwork.kernel.org/project/linux-mm/patch/20221219042126.3396-1-sunhao.th@gmail.com/) | 705487 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221219042126.3396-1-sunhao.th@gmail.com))<br>*-*-*-*-*-*-*-*<br>[LORE v2,0/1](https://lore.kernel.org/r/20221221144245.27164-1-sunhao.th@gmail.com) |
| 2022/12/19 | Bagas Sanjaya <bagasdotme@gmail.com> | [docs: cgroup-v1: formatting improv for"Memory Resource Controller"doc](https://patchwork.kernel.org/project/linux-mm/cover/20221219042209.22898-1-bagasdotme@gmail.com/) | 705488 | v1 ☐☑ | [LORE v1,0/10](https://lore.kernel.org/r/20221219042209.22898-1-bagasdotme@gmail.com) |
| 2022/12/19 | David Hildenbrand <david@redhat.com> | [mm/nommu: don't use VM_MAYSHARE for MAP_PRIVATE mappings](https://patchwork.kernel.org/project/linux-mm/cover/20221219163013.259423-1-david@redhat.com/) | 705652 | v1 ☐☑ | [LORE v1,0/2](https://lore.kernel.org/r/20221219163013.259423-1-david@redhat.com) |
| 2022/12/20 | Muhammad Usama Anjum <usama.anjum@collabora.com> | [[RFC] mm: implement granular soft-dirty vma support](https://patchwork.kernel.org/project/linux-mm/patch/20221220162606.1595355-1-usama.anjum@collabora.com/) | 705984 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221220162606.1595355-1-usama.anjum@collabora.com) |
| 2022/12/20 | Uladzislau Rezki <urezki@gmail.com> | [[1/2] mm: vmalloc: Avoid a double lookup of freed VA in a tree](https://patchwork.kernel.org/project/linux-mm/patch/20221220182704.181657-1-urezki@gmail.com/) | 706007 | v1 ☐☑ | [LORE v1,0/2](https://lore.kernel.org/r/20221220182704.181657-1-urezki@gmail.com) |
| 2022/12/20 | Roman Gushchin <roman.gushchin@linux.dev> | [mm: kmem: optimize obj_cgroup pointer retrieval](https://patchwork.kernel.org/project/linux-mm/cover/20221220182745.1903540-1-roman.gushchin@linux.dev/) | 706008 | v1 ☐☑ | [LORE v1,0/2](https://lore.kernel.org/r/20221220182745.1903540-1-roman.gushchin@linux.dev) |
| 2022/12/20 | Roman Gushchin <roman.gushchin@linux.dev> | [[RFC] ipc/mqueue: introduce msg cache](https://patchwork.kernel.org/project/linux-mm/patch/20221220184813.1908318-1-roman.gushchin@linux.dev/) | 706018 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221220184813.1908318-1-roman.gushchin@linux.dev) |
| 2022/12/21 | Marcelo Tosatti <mtosatti@redhat.com> | [Ensure quiet_vmstat() is called when returning to userpace and when idle tick is stopped](https://patchwork.kernel.org/project/linux-mm/cover/20221221165801.362118576@redhat.com/) | 706283 | v11 ☐☑ | [LORE v11,0/6](https://lore.kernel.org/r/20221221165801.362118576@redhat.com)<br>*-*-*-*-*-*-*-* <br>[LORE v12,0/6](https://lore.kernel.org/r/20221227121137.102497574@redhat.com) |
| 2022/12/21 | Uladzislau Rezki <urezki@gmail.com> | [[v2,1/3] mm: vmalloc: Avoid of calling `__find_vmap_area()` twise in __vunmap()](https://patchwork.kernel.org/project/linux-mm/patch/20221221174454.1085130-1-urezki@gmail.com/) | 706294 | v2 ☐☑ | [LORE v2,0/3](https://lore.kernel.org/r/20221221174454.1085130-1-urezki@gmail.com) |
| 2022/12/22 | Yuanchu Xie <yuanchu@google.com> | [[1/2] mm: add vma_has_locality()](https://patchwork.kernel.org/project/linux-mm/patch/20221222061341.381903-1-yuanchu@google.com/) | 706396 | v1 ☐☑ | [LORE v1,0/2](https://lore.kernel.org/r/20221222061341.381903-1-yuanchu@google.com) |
| 2022/12/23 | Zach O'Keefe <zokeefe@google.com> | [mm/shmem: restore SHMEM_HUGE_DENY precedence over MADV_COLLAPSE](https://patchwork.kernel.org/project/linux-mm/patch/20221223003833.2793963-1-zokeefe@google.com/) | 706614 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221223003833.2793963-1-zokeefe@google.com) |
| 2022/12/23 | Zach O'Keefe <zokeefe@google.com> | [mm/MADV_COLLAPSE: don't expand collapse when vm_end is past requested end](https://patchwork.kernel.org/project/linux-mm/patch/20221223003953.2795313-1-zokeefe@google.com/) | 706615 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221223003953.2795313-1-zokeefe@google.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/2](https://lore.kernel.org/r/20221224081203.3193960-1-zokeefe@google.com)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/2](https://lore.kernel.org/r/20221224082035.3197140-1-zokeefe@google.com) |
| 2022/12/23 | Soichiro Ueda <the.latticeheart@gmail.com> | [virtio_balloon: high order allocation](https://patchwork.kernel.org/project/linux-mm/patch/20221223093527.12424-1-the.latticeheart@gmail.com/) | 706715 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221223093527.12424-1-the.latticeheart@gmail.com) |


| 2022/10/24 | Nick Terrell <nickrterrell@gmail.com> | [zstd: Update to upstream v1.5.2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=2aa14b1ab2c41a4fe41efae80d58bb77da91f19f) | [Updated Zstd Implementation Merged For Linux 6.2](https://www.phoronix.com/news/Linux-6.2-Zstd) | v1 ☐☑✓ | [LORE v1,0/2](https://lore.kernel.org/all/20221024202606.404049-1-nickrterrell@gmail.com) |
| 2022/11/10 | Nick Desaulniers <ndesaulniers@google.com> | [Makefile.debug: support for -gz=zstd](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=9f8fe647797a4bc049bc7cceaf3a63584678ba04) | 实现 DEBUG_INFO_COMPRESSED_ZSTD, 参见 phoronix 报道 [Linux 6.2 Allows For Zstd-Compressed Debug Information](https://www.phoronix.com/news/Linux-6.2-Zstd-Debug-Info) | v3 ☐☑✓ 6.2-rc1 | [LORE](https://lore.kernel.org/all/20221110195932.377841-1-ndesaulniers@google.com) |



| 2022/04/06 | Liao Chang <liaochang1@huawei.com> | [softirq: Introduce softirq throttling](https://lore.kernel.org/all/20220406022749.184807-1-liaochang1@huawei.com) | TODO | v1 ☐☑✓ | [LORE v1,0/3](https://lore.kernel.org/all/20220406022749.184807-1-liaochang1@huawei.com) |


http://viz-js.com
https://mermaid.live

https://latex.codecogs.com
https://www.latexlive.com



| 2022/11/15 | Nhat Pham <nphamcs@gmail.com> | [cachestat: a new syscall for page cache state of files](https://patchwork.kernel.org/project/linux-mm/cover/20221115182901.2755368-1-nphamcs@gmail.com/) | 695661 | v1 ☐☑ | [LORE v1,0/4](https://lore.kernel.org/r/20221115182901.2755368-1-nphamcs@gmail.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/4](https://lore.kernel.org/r/20221205175140.1543229-1-nphamcs@gmail.com)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/4](https://lore.kernel.org/r/20221208223104.1554368-1-nphamcs@gmail.com)<br>*-*-*-*-*-*-*-* <br>[LORE v4,0/4](https://lore.kernel.org/r/20221216192149.3902877-1-nphamcs@gmail.com) |
| 2022/12/16 | Keith Busch <kbusch@meta.com> | [dmapool enhancements](https://patchwork.kernel.org/project/linux-mm/cover/20221216201625.2362737-1-kbusch@meta.com/) | 705206 | v1 ☐☑ | [LORE v1,0/11](https://lore.kernel.org/r/20221216201625.2362737-1-kbusch@meta.com) |




MGLRU 合入后, 引起了不少场景的性能劣化, 参见 phoronix 报道 [An MGLRU Performance Regression Fix Is On The Way Plus Another Optimization](https://www.phoronix.com/news/MGLRU-SVT-Performance-Fix).






| 2020/02/27 | Valentin Schneider <valentin.schneider@arm.com> | [sched, arm64: enable CONFIG_SCHED_SMT for arm64](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=6f693dd5be08237b337f557c510d99addb9eb9ec) | TODO | v2 ☑✓ 5.7-rc1 | [LORE v2,0/2](https://lore.kernel.org/all/20200227191433.31994-1-valentin.schneider@arm.com) |
| 2022/12/02 | Brian Foster <bfoster@redhat.com> | [proc: improve root readdir latency with many threads](https://lore.kernel.org/all/20221202171620.509140-1-bfoster@redhat.com) | TODO | v3 ☐☑✓ | [LORE v3,0/5](https://lore.kernel.org/all/20221202171620.509140-1-bfoster@redhat.com) |

| 2023/01/09 | Yian Chen <yian.chen@intel.com> | [Enable LASS (Linear Address space Separation)](https://lore.kernel.org/all/20230110055204.3227669-1-yian.chen@intel.com) | 参见 LWN 报道 [Support for Intel's LASS](https://lwn.net/Articles/919683) 和 phoronix 报道 [Intel Posts Linux Patches For Linear Address Space Separation (LASS)](https://www.phoronix.com/news/Linear-Address-Space-Separation) | v1 ☐☑✓ | [LORE v1,0/7](https://lore.kernel.org/all/20230110055204.3227669-1-yian.chen@intel.com) |



[[LSF/MM/BFP TOPIC] Storage: Copy Offload](https://lkml.kernel.org/linux-block/f0e19ae4-b37a-e9a3-2be7-a5afb334a5c3@nvidia.com)
[LSFMM: Copy offload](https://lwn.net/Articles/548347)
[Storage: Xcopy Offload](https://blog.csdn.net/flyingnosky/article/details/123533554)


| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2014/05/28 | Martin K. Petersen <martin.petersen@oracle.com> | [Copy offload](https://lore.kernel.org/all/1401335565-29865-1-git-send-email-martin.petersen@oracle.com) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/1401335565-29865-1-git-send-email-martin.petersen@oracle.com) |
| 2022/11/23 | Nitesh Shetty <nj.shetty@samsung.com> | [Implement copy offload support](https://lore.kernel.org/all/20221123055827.26996-1-nj.shetty@samsung.com) | TODO | v5 ☐☑✓ | [LORE v5,0/10](https://lore.kernel.org/all/20221123055827.26996-1-nj.shetty@samsung.com) |




[调度器 34—RT 负载均衡](https://www.cnblogs.com/hellokitty2/p/15974333.html)
[实时调度负载均衡](https://github.com/freelancer-leon/notes/blob/master/kernel/sched/sched_rt_load_balance.md)


[Latencies, schedulers, interrupts oh my! The epic story of a Linux Kernel upgrade](https://www.nutanix.dev/2021/12/09/latencies-schedulers-interrupts-oh-my-the-epic-story-of-a-linux-kernel-upgrade)

[RISC-V Hibernation Support / Suspend-To-Disk Nears The Linux Kernel](https://www.phoronix.com/news/RISC-V-Hibernation-Linux)
[Intel Preparing New Linux"PerfMon"Performance Monitoring Support For IOMMU](https://www.phoronix.com/news/Intel-IOMMU-VT-d-4.0-PerfMon)

[ARM64 手动搭建 kdump 环境](https://blog.csdn.net/m0_37797953/article/details/107491356)
[crash 命令 —— list](https://www.cnblogs.com/pengdonglin137/p/16046328.html)
[CRASH 安装和调试](https://www.cnblogs.com/Linux-tech/p/14110330.html)
[fujitsu/crash-trace](https://github.com/fujitsu/crash-trace)
[How to display or retrieve ftrace data from the kernel crash dump?](https://access.redhat.com/solutions/239433)









5.7-rc1 [psi: Optimize switching tasks inside shared cgroups](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=36b238d5717279163859fb6ba0f4360abcafab83)

5.13-rc1 [psi: Optimize task switch inside shared cgroups](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=4117cebf1a9fcbf35b9aabf0e37b6c5eea296798)

5.13-rc1 [psi: Fix psi state corruption when schedule() races with cgroup move](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d583d360a620e6229422b3455d0be082b8255f5e)
| 2023/01/13 | Vincent Guittot <vincent.guittot@linaro.org> | [sched/fair: unlink misfit task from cpu overutilized](https://lore.kernel.org/all/20230113134056.257691-1-vincent.guittot@linaro.org) | TODO | v3 ☐☑✓ | [LORE](https://lore.kernel.org/all/20230113134056.257691-1-vincent.guittot@linaro.org) |

通过考虑 uclamp_min, task misfit 和 cpu overutilization 之间的 1:1 关系不再成立, 因为一个 util_avg 较小的任务可能由于 uclamp_min 的约束而不适合大容量的 cpu.

在 util_fits_cpu() 中添加一个新状态, 以反映任务适合 CPU 的情况, 除了 uclamp_min 提示 (这是一种性能要求).

使用 - 1 表示 CPU 不适合只是因为 uclamp_min, 因此我们可以使用这个新值采取额外的操作, 以选择不符合 uclamp_min 提示的最佳 CPU.


| 2023/01/12 | Daniel Bristot de Oliveira <bristot@kernel.org> | [sched/idle: Make idle poll dynamic per-cpu](https://lore.kernel.org/all/20230112162426.217522-1-bristot@kernel.org) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20230112162426.217522-1-bristot@kernel.org) |



| 2023/01/12 | Daniel Bristot de Oliveira <bristot@kernel.org> | [sched/idle: Make idle poll dynamic per-cpu](https://lore.kernel.org/all/20230112162426.217522-1-bristot@kernel.org) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20230112162426.217522-1-bristot@kernel.org) |




[鲲鹏 gcc mcmodel 选项详解](https://bbs.huaweicloud.com/blogs/272527)
[GCC for openEuler -mcmodel 选项详解](https://cdn.modb.pro/db/524836)


[对于几个锁的对比总结 Part1](https://blog.csdn.net/He11o_Liu/article/details/81077867)
[论文分享: Smartlocks: Lock Acquisition Scheduling for Self-Aware Synchronization](https://blog.csdn.net/He11o_Liu/article/details/81077695)
[论文分享 SANL: 可扩展 NUMA-Aware 锁](https://blog.csdn.net/He11o_Liu/article/details/79255951)
[论文分享: Unlocking Energy](https://blog.csdn.net/He11o_Liu/article/details/81077777)
[论文分享: Non-scalable locks are dangerous](https://blog.csdn.net/He11o_Liu/article/details/80386839)
[转载 ---- 从 CPU cache 一致性的角度看 Linux spinlock 的不可伸缩性 (non-scalable)](https://blog.csdn.net/zhangshuaiisme/article/details/88147697)
[Scalable lock-free dynamic memory allocation 简要观感](https://blog.csdn.net/jollyjumper/article/details/53948391)
[从 CPU cache 一致性的角度看 Linux spinlock 的不可伸缩性 (non-scalable)](https://blog.csdn.net/dog250/article/details/80589442)
[[Paper 翻译]Scalable Lock-Free Dynamic Memory Allocation](https://blog.csdn.net/weixin_30457065/article/details/95622521)
[PV qspinlock 原理](https://blog.csdn.net/bemind1/article/details/118224344)














| 2023/01/26 | Waiman Long <longman@redhat.com> | [sched: Store restrict_cpus_allowed_ptr() call state](https://lore.kernel.org/all/20230127015527.466367-1-longman@redhat.com) | TODO | v3 ☐☑✓ | [LORE](https://lore.kernel.org/all/20230127015527.466367-1-longman@redhat.com) |
| 2023/01/20 | Wander Lairson Costa <wander@redhat.com> | [Fix put_task_struct() calls under PREEMPT_RT](https://lore.kernel.org/all/20230120150246.20797-1-wander@redhat.com) | TODO | v2 ☐☑✓ | [LORE v2,0/4](https://lore.kernel.org/all/20230120150246.20797-1-wander@redhat.com) |




| 2023/01/13 | Nathan Huckleberry <nhuck@google.com> | [workqueue: Add WQ_SCHED_FIFO](https://lore.kernel.org/all/20230113210703.62107-1-nhuck@google.com) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20230113210703.62107-1-nhuck@google.com) |
| 2018/11/11 | Paul E. McKenney <paulmck@linux.ibm.com> | [Automate initrd generation for v4.21/v5.0](https://lore.kernel.org/all/20181111200127.GA9511@linux.ibm.com) | 内核中引入 nolibc, 参见 LWN 报道 [Nolibc: a minimal C-library replacement shipped with the kernel](https://lwn.net/Articles/920158) | v5 ☐☑✓ | [LORE v5,0/8](https://lore.kernel.org/all/20181111200127.GA9511@linux.ibm.com) |
| 2023/01/30 | Fan Wu <wufan@linux.microsoft.com> | [Integrity Policy Enforcement LSM (IPE)](https://lore.kernel.org/all/1675119451-23180-1-git-send-email-wufan@linux.microsoft.com) | TODO | v9 ☐☑✓ | [LORE v9,0/16](https://lore.kernel.org/all/1675119451-23180-1-git-send-email-wufan@linux.microsoft.com) |
| 2022/12/30 | Dmitrii Bundin <dmitrii.bundin.a@gmail.com> | [scripts/gdb: add mm introspection utils](https://patchwork.kernel.org/project/linux-mm/patch/20221230163512.23736-1-dmitrii.bundin.a@gmail.com) | 707744 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221230163512.23736-1-dmitrii.bundin.a@gmail.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/1](https://lore.kernel.org/r/20221231171258.7907-1-dmitrii.bundin.a@gmail.com)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/1](https://lore.kernel.org/r/20230101172312.21452-1-dmitrii.bundin.a@gmail.com) |




[McKenney: What Does It Mean To Be An RCU Implementation?](https://lwn.net/Articles/921351)

[GFP flags and the end of GFP_ATOMIC](https://lwn.net/Articles/920891)

[Linux Kernel Podcast](https://kernelpodcast.org)

[Reconsidering BPF ABI stability](https://lwn.net/Articles/921088)

| 2023/01/13 | Mel Gorman <mgorman@techsingularity.net> | [Discard `__GFP_ATOMIC`](https://lore.kernel.org/all/20230113111217.14134-1-mgorman@techsingularity.net) | TODO | v3 ☐☑✓ | [LORE v2,0/6](https://lore.kernel.org/all/20230109151631.24923-1-mgorman@techsingularity.net)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/6](https://lore.kernel.org/all/20230113111217.14134-1-mgorman@techsingularity.net) |

[Linux Developers Evaluating New"DOITM"Security Mitigation For Latest Intel CPUs](https://www.phoronix.com/review/intel-doitm-linux)