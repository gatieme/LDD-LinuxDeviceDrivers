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
| 2022/11/07 | Song Liu <song@kernel.org> | [execmem_alloc for BPF programs](https://patchwork.kernel.org/project/linux-mm/cover/20221107223921.3451913-1-song@kernel.org/) | 692973 | v2 ☐☑ | [LORE v2,0/5](https://lore.kernel.org/r/20221107223921.3451913-1-song@kernel.org)[LORE v4,0/6](https://lore.kernel.org/r/20221117202322.944661-1-song@kernel.org)<br>*-*-*-*-*-*-*-* <br>[LORE v5,0/6](https://lore.kernel.org/r/20221128190245.2337461-1-song@kernel.org) |
| 2022/11/17 | Song Liu <song@kernel.org> | [execmem_alloc for BPF programs](https://patchwork.kernel.org/project/linux-mm/cover/20221117202322.944661-1-song@kernel.org/) | 696625 | v4 ☐☑ | |
| 2022/12/12 | Yafang Shao <laoar.shao@gmail.com> | [mm, bpf: Add BPF into /proc/meminfo](https://patchwork.kernel.org/project/linux-mm/cover/20221212003711.24977-1-laoar.shao@gmail.com/) | 703689 | v1 ☐☑ | [LORE v1,0/9](https://lore.kernel.org/r/20221212003711.24977-1-laoar.shao@gmail.com) |




| 2022/09/13 | Doug Berger <opendmb@gmail.com> | [mm: introduce Designated Movable Blocks](https://patchwork.kernel.org/project/linux-mm/cover/20220913195508.3511038-1-opendmb@gmail.com/) | 676674 | v1 ☐☑ | [LORE v1,0/21](https://lore.kernel.org/r/20220913195508.3511038-1-opendmb@gmail.com)<br>*-*-*-*-*-*-*-* <br>[LORE v4,0/9](https://lore.kernel.org/r/20230311003855.645684-1-opendmb@gmail.com) |
| 2022/10/19 | Andrei Vagin <avagin@gmail.com> | [seccomp: add the synchronous mode for seccomp_unotify](https://lore.kernel.org/all/20221020011048.156415-1-avagin@gmail.com) | TODO | v2 ☐☑✓ | [LORE v2,0/5](https://lore.kernel.org/all/20221020011048.156415-1-avagin@gmail.com) |


[Linux RSEQ Patches Updated For Allowing Faster getCPU () In C Libraries](https://www.phoronix.com/news/Linux-RSEQ-Faster-getCPU)


[Seccomp 机制与 seccomp notify 介绍](https://just4coding.com/2022/04/03/seccomp/)

git log --oneline android-mainline-5.10...HEAD | grep ANDROID | grep sched | grep -E "vendor|hook"



3882af1c70d0 ANDROID: ABI: Update symbols to unisoc whitelist for the scheduler 4st














https://lkml.org/lkml/2021/3/16/546


[Linux Patch Pending To Help Lower Alder Lake N & Raptor Lake P Power Consumption](https://www.phoronix.com/news/Linux-ADL-N-RPL-P-EPB-Tuning)







| 2022/10/26 | Joey Gouly <joey.gouly@arm.com> | [mm: In-kernel support for memory-deny-write-execute (MDWE)](https://patchwork.kernel.org/project/linux-mm/cover/20221026150457.36957-1-joey.gouly@arm.com/) | 689074 | v1 ☐☑ | [LORE RFC,0/4](https://lore.kernel.org/linux-arm-kernel/20220413134946.2732468-1-catalin.marinas@arm.com)<br>*-*-*-*-*-*-*-* <br>[LORE v1,0/2](https://lore.kernel.org/r/20221026150457.36957-1-joey.gouly@arm.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/2](https://lore.kernel.org/r/20230119160344.54358-1-joey.gouly@arm.com) |


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
| 2022/11/04 | zhaoyang.huang <zhaoyang.huang@unisoc.com> | [mm: introduce object accounting via backtrace on slub](https://patchwork.kernel.org/project/linux-mm/patch/1667550838-10639-1-git-send-email-zhaoyang.huang@unisoc.com/) | 692001 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/1667550838-10639-1-git-send-email-zhaoyang.huang@unisoc.com) |
| 2022/11/08 | Linus Torvalds <torvalds@linux-foundation.org> | [mm: introduce'encoded'page pointers with embedded extra bits](https://patchwork.kernel.org/project/linux-mm/patch/20221108194139.57604-1-torvalds@linux-foundation.org/) | 693373 | v1 ☐☑ | [LORE v1,0/4](https://lore.kernel.org/r/20221108194139.57604-1-torvalds@linux-foundation.org)<br>*-*-*-*-*-*-*-* <br>[LORE v1,0/4](https://lore.kernel.org/r/20221109203051.1835763-1-torvalds@linux-foundation.org) |
| 2022/11/09 | Chao Xu <amos.xuchao@gmail.com> | [mm/vmscan: simplify the nr assignment logic for pages to scan](https://patchwork.kernel.org/project/linux-mm/patch/20221109070416.620887-1-Chao.Xu9@zeekrlife.com/) | 693511 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221109070416.620887-1-Chao.Xu9@zeekrlife.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/1](https://lore.kernel.org/r/20221110113130.284290-1-Chao.Xu9@zeekrlife.com) |
| 2022/11/21 | Aleksey Romanov <avromanov@sberdevices.ru> | [Introduce merge identical pages mechanism](https://patchwork.kernel.org/project/linux-mm/cover/20221121190020.66548-1-avromanov@sberdevices.ru/) | 697795 | v1 ☐☑ | [LORE v1,0/4](https://lore.kernel.org/r/20221121190020.66548-1-avromanov@sberdevices.ru) |
| 2022/11/25 | Peter Xu <peterx@redhat.com> | [mm/thp: Re-apply mkdirty for small pages after split](https://patchwork.kernel.org/project/linux-mm/patch/20221125185857.3110155-1-peterx@redhat.com/) | 699295 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221125185857.3110155-1-peterx@redhat.com) |
| 2022/11/29 | Liam Howlett <Liam.Howlett@Oracle.com> | [VMA type safety through VMA iterator](https://patchwork.kernel.org/project/linux-mm/cover/20221129164352.3374638-1-Liam.Howlett@oracle.com/) | 700163 | v1 ☐☑ | [LORE v1,0/43](https://lore.kernel.org/r/20221129164352.3374638-1-Liam.Howlett@oracle.com) |
| 2022/12/01 | Mina Almasry <almasrymina@google.com> | [mm: disable top-tier fallback to reclaim on proactive reclaim](https://patchwork.kernel.org/project/linux-mm/patch/20221201233317.1394958-1-almasrymina@google.com/) | 701011 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221201233317.1394958-1-almasrymina@google.com) |
| 2022/12/02 | Alistair Popple <apopple@nvidia.com> | [mm/mmap: Properly unaccount memory on mas_preallocate() failure](https://patchwork.kernel.org/project/linux-mm/patch/20221202045339.2999017-1-apopple@nvidia.com/) | 701081 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221202045339.2999017-1-apopple@nvidia.com) |

| 2022/12/02 | Brian Foster <bfoster@redhat.com> | [proc: improve root readdir latency with many threads](https://patchwork.kernel.org/project/linux-mm/cover/20221202171620.509140-1-bfoster@redhat.com/) | 701317 | v3 ☐☑ | [LORE v3,0/5](https://lore.kernel.org/r/20221202171620.509140-1-bfoster@redhat.com) |
| 2022/12/05 | Li,Rongqing <lirongqing@baidu.com> | [mm: memcontrol: speedup memory cgroup resize](https://patchwork.kernel.org/project/linux-mm/patch/1670240992-28563-1-git-send-email-lirongqing@baidu.com/) | 701772 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/1670240992-28563-1-git-send-email-lirongqing@baidu.com) |


| 2022/11/09 | Baoquan He <bhe@redhat.com> | [mm/vmalloc.c: allow vread() to read out vm_map_ram areas](https://patchwork.kernel.org/project/linux-mm/cover/20221109033535.269229-1-bhe@redhat.com/) | 693476 | v1 ☐☑ | [LORE v1,0/3](https://lore.kernel.org/r/20221109033535.269229-1-bhe@redhat.com)<br>*-*-*-*-*-*-*-* <br>[LORE v1,0/7](https://lore.kernel.org/r/20221204013046.154960-1-bhe@redhat.com) |
\| 2022/12/06 | Fabio M. De Francesco <fmdefrancesco@gmail.com> | [mm/highmem: Add notes about conversions from kmap{,_atomic}()](https://patchwork.kernel.org/project/linux-mm/patch/20221206070029.7342-1-fmdefrancesco@gmail.com/) | 702076 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221206070029.7342-1-fmdefrancesco@gmail.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/1](https://lore.kernel.org/r/20221207225308.8290-1-fmdefrancesco@gmail.com) |
| 2022/12/06 | David Hildenbrand <david@redhat.com> | [mm: support `__HAVE_ARCH_PTE_SWP_EXCLUSIVE` on all architectures with swap PTEs](https://patchwork.kernel.org/project/linux-mm/cover/20221206144730.163732-1-david@redhat.com/) | 702222 | v1 ☐☑ | [LORE v1,0/26](https://lore.kernel.org/r/20221206144730.163732-1-david@redhat.com) |
| 2022/12/06 | Nico Pache <npache@redhat.com> | [vmscan: Scale file_is_tiny calculation based on priority](https://patchwork.kernel.org/project/linux-mm/patch/20221206222315.37631-1-npache@redhat.com/) | 702319 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221206222315.37631-1-npache@redhat.com) |
| 2022/12/07 | Vishal Moola (Oracle) <vishal.moola@gmail.com> | [Convert deactivate_page() to deactivate_folio()](https://patchwork.kernel.org/project/linux-mm/cover/20221207002158.418789-1-vishal.moola@gmail.com/) | 702344 | v1 ☐☑ | [LORE v1,0/3](https://lore.kernel.org/r/20221207002158.418789-1-vishal.moola@gmail.com) |
| 2022/12/07 | Kefeng Wang <wangkefeng.wang@huawei.com> | [mm: huge_memory: Convert madvise_free_huge_pmd to use a folio](https://patchwork.kernel.org/project/linux-mm/patch/20221207023431.151008-1-wangkefeng.wang@huawei.com/) | 702384 | v1 ☐☑ | [LORE v1,0/2](https://lore.kernel.org/r/20221207023431.151008-1-wangkefeng.wang@huawei.com) |
| 2022/12/08 | chengkaitao <pilgrimtao@gmail.com> | [mm: memcontrol: protect the memory in cgroup from being oom killed](https://patchwork.kernel.org/project/linux-mm/patch/20221208034644.3077-1-chengkaitao@didiglobal.com/) | 702809 | v2 ☐☑ | [LORE v2,0/1](https://lore.kernel.org/r/20221208034644.3077-1-chengkaitao@didiglobal.com) |
| 2022/12/08 | Wenchao Hao <haowenchao@huawei.com> | [cma:tracing: Print alloc result in trace_cma_alloc_finish](https://patchwork.kernel.org/project/linux-mm/patch/20221208142130.1501195-1-haowenchao@huawei.com/) | 702989 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221208142130.1501195-1-haowenchao@huawei.com) |
| 2022/12/08 | Peter Xu <peterx@redhat.com> | [mm/uffd: Always wr-protect pte in pte|pmd_mkuffd_wp()](https://patchwork.kernel.org/project/linux-mm/patch/20221208194628.766316-1-peterx@redhat.com/) | 703082 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221208194628.766316-1-peterx@redhat.com) |
| 2022/12/09 | Xander <xandermoerkerken@gmail.com> | [Added ability to vmalloc executable memory](https://patchwork.kernel.org/project/linux-mm/patch/20221209131052.64235-1-xander.moerkerken@omron.com/) | 703287 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221209131052.64235-1-xander.moerkerken@omron.com) |
| 2022/12/09 | Nhat Pham <nphamcs@gmail.com> | [[v3,2/4] workingset: refactor LRU refault to expose refault recency check](https://patchwork.kernel.org/project/linux-mm/patch/20221209172922.3143160-1-nphamcs@gmail.com/) | 703468 | v3 ☐☑ | [LORE v3,0/4](https://lore.kernel.org/r/20221209172922.3143160-1-nphamcs@gmail.com) |
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
| 2023/03/22 | Liu Shixin <liushixin2@huawei.com> | [Delay the initialization of zswap](https://patchwork.kernel.org/project/linux-mm/cover/20230322102006.780624-1-liushixin2@huawei.com/) | 732661 | v6 ☐☑ | [LORE v6,0/2](https://lore.kernel.org/r/20230322102006.780624-1-liushixin2@huawei.com))<br>*-*-*-*-*-*-*-* <br>[LORE v7,0/4](https://lore.kernel.org/r/20230325071420.2246461-1-liushixin2@huawei.com)<br>*-*-*-*-*-*-*-* <br>[LORE v9,0/3](https://lore.kernel.org/r/20230411093632.822290-1-liushixin2@huawei.com) |
| 2023/03/22 | Florian Schmidt <flosch@nutanix.com> | [[RFC] memcg v1: provide read access to memory.pressure_level](https://patchwork.kernel.org/project/linux-mm/patch/20230322142525.162469-1-flosch@nutanix.com/) | 732770 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20230322142525.162469-1-flosch@nutanix.com) |
| 2023/04/11 | Wen Yang <wenyang.linux@foxmail.com> | [mm: compaction: optimize compact_memory to comply with the admin-guide](https://patchwork.kernel.org/project/linux-mm/patch/tencent_FD958236269FD3A7996FFCF29E9BAA4EA809@qq.com/) | 738878 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/tencent_FD958236269FD3A7996FFCF29E9BAA4EA809@qq.com) |


| 2022/10/24 | Nick Terrell <nickrterrell@gmail.com> | [zstd: Update to upstream v1.5.2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=2aa14b1ab2c41a4fe41efae80d58bb77da91f19f) | [Updated Zstd Implementation Merged For Linux 6.2](https://www.phoronix.com/news/Linux-6.2-Zstd) | v1 ☐☑✓ | [LORE v1,0/2](https://lore.kernel.org/all/20221024202606.404049-1-nickrterrell@gmail.com) |
| 2022/11/10 | Nick Desaulniers <ndesaulniers@google.com> | [Makefile.debug: support for -gz=zstd](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=9f8fe647797a4bc049bc7cceaf3a63584678ba04) | 实现 DEBUG_INFO_COMPRESSED_ZSTD, 参见 phoronix 报道 [Linux 6.2 Allows For Zstd-Compressed Debug Information](https://www.phoronix.com/news/Linux-6.2-Zstd-Debug-Info) | v3 ☐☑✓ 6.2-rc1 | [LORE](https://lore.kernel.org/all/20221110195932.377841-1-ndesaulniers@google.com) |
| 2023/03/23 | Krcka, Tomas <krckatom@amazon.de> | [[v2] mm: Be less noisy during memory hotplug](https://patchwork.kernel.org/project/linux-mm/patch/20230323174349.35990-1-krckatom@amazon.de/) | 733270 | v2 ☐☑ | [LORE v2,0/1](https://lore.kernel.org/r/20230323174349.35990-1-krckatom@amazon.de) |
| 2023/03/25 | Mark Brown <broonie@kernel.org> | [regmap: Add basic maple tree register cache](https://patchwork.kernel.org/project/linux-mm/cover/20230325-regcache-maple-v1-0-1c76916359fb@kernel.org) | 733856 | v1 ☐☑ | [LORE v1,0/2](https://lore.kernel.org/r/20230325-regcache-maple-v1-0-1c76916359fb@kernel.org)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/2](https://lore.kernel.org/r/20230325-regcache-maple-v2-0-799dcab3ecb1@kernel.org)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/2](https://lore.kernel.org/r/20230325-regcache-maple-v3-0-23e271f93dc7@kernel.org) |


| 2023/03/27 | Matthew Wilcox <willy@infradead.org> | [Prevent ->map_pages from sleeping](https://patchwork.kernel.org/project/linux-mm/cover/20230327174515.1811532-1-willy@infradead.org/) | 734281 | v2 ☐☑ | [LORE v2,0/3](https://lore.kernel.org/r/20230327174515.1811532-1-willy@infradead.org) |

| 2023/03/28 | Alistair Popple <apopple@nvidia.com> | [mm: Take a page reference when removing device exclusive entries](https://patchwork.kernel.org/project/linux-mm/patch/20230328021434.292971-1-apopple@nvidia.com/) | 734394 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20230328021434.292971-1-apopple@nvidia.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/1](https://lore.kernel.org/r/20230330012519.804116-1-apopple@nvidia.com) |
| 2023/03/28 | Yosry Ahmed <yosryahmed@google.com> | [memcg: make rstat flushing irq and sleep friendly](https://patchwork.kernel.org/project/linux-mm/cover/20230328061638.203420-1-yosryahmed@google.com/) | 734466 | v1 ☐☑ | [LORE v1,0/9](https://lore.kernel.org/r/20230328061638.203420-1-yosryahmed@google.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/9](https://lore.kernel.org/r/20230328221644.803272-1-yosryahmed@google.com) |
| 2023/03/28 | Muchun Song <songmuchun@bytedance.com> | [Simplify kfence code](https://patchwork.kernel.org/project/linux-mm/cover/20230328095807.7014-1-songmuchun@bytedance.com/) | 734539 | v1 ☐☑ | [LORE v1,0/6](https://lore.kernel.org/r/20230328095807.7014-1-songmuchun@bytedance.com) |


| 2022/04/06 | Liao Chang <liaochang1@huawei.com> | [softirq: Introduce softirq throttling](https://lore.kernel.org/all/20220406022749.184807-1-liaochang1@huawei.com) | TODO | v1 ☐☑✓ | [LORE v1,0/3](https://lore.kernel.org/all/20220406022749.184807-1-liaochang1@huawei.com) |
| 2023/03/29 | Yicong Yang <yangyicong@huawei.com> | [arm64: support batched/deferred tlb shootdown during page reclamation](https://patchwork.kernel.org/project/linux-mm/cover/20230329035512.57392-1-yangyicong@huawei.com/) | 734835 | v8 ☐☑ | [LORE v8,0/2](https://lore.kernel.org/r/20230329035512.57392-1-yangyicong@huawei.com) |
| 2023/03/29 | Luis Chamberlain <mcgrof@kernel.org> | [module: avoid userspace pressure on unwanted allocations](https://patchwork.kernel.org/project/linux-mm/cover/20230329053149.3976378-1-mcgrof@kernel.org/) | 734852 | v1 ☐☑ | [LORE v1,0/7](https://lore.kernel.org/r/20230329053149.3976378-1-mcgrof@kernel.org) |
| 2023/03/30 | Longlong Xia <xialonglong1@huawei.com> | [mm: ksm: support hwpoison for ksm page](https://patchwork.kernel.org/project/linux-mm/cover/20230330074501.205092-1-xialonglong1@huawei.com/) | 735257 | v1 ☐☑ | [LORE v1,0/2](https://lore.kernel.org/r/20230330074501.205092-1-xialonglong1@huawei.com) |
| 2023/03/30 | Yosry Ahmed <yosryahmed@google.com> | [memcg: avoid flushing stats atomically where possible](https://patchwork.kernel.org/project/linux-mm/cover/20230330191801.1967435-1-yosryahmed@google.com/) | 735542 | v3 ☐☑ | [LORE v3,0/8](https://lore.kernel.org/r/20230330191801.1967435-1-yosryahmed@google.com) |
| 2023/03/30 | Shaun Tancheff <shaun.tancheff@gmail.com> | [memcg: Set memory min, low, high values along with max](https://patchwork.kernel.org/project/linux-mm/patch/20230330202232.355471-1-shaun.tancheff@gmail.com/) | 735566 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20230330202232.355471-1-shaun.tancheff@gmail.com) |


http://viz-js.com
https://mermaid.live

https://latex.codecogs.com
https://www.latexlive.com





| 2022/12/16 | Keith Busch <kbusch@meta.com> | [dmapool enhancements](https://patchwork.kernel.org/project/linux-mm/cover/20221216201625.2362737-1-kbusch@meta.com/) | 705206 | v1 ☐☑ | [LORE v1,0/11](https://lore.kernel.org/r/20221216201625.2362737-1-kbusch@meta.com) |
| 2023/02/01 | Alexander Halbuer <halbuer@sra.uni-hannover.de> | [mm: reduce lock contention of pcp buffer refill](https://patchwork.kernel.org/project/linux-mm/patch/20230201162549.68384-1-halbuer@sra.uni-hannover.de/) | 717782 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20230201162549.68384-1-halbuer@sra.uni-hannover.de) |
| 2020/02/27 | Valentin Schneider <valentin.schneider@arm.com> | [sched, arm64: enable CONFIG_SCHED_SMT for arm64](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=6f693dd5be08237b337f557c510d99addb9eb9ec) | TODO | v2 ☑✓ 5.7-rc1 | [LORE v2,0/2](https://lore.kernel.org/all/20200227191433.31994-1-valentin.schneider@arm.com) |
| 2022/12/02 | Brian Foster <bfoster@redhat.com> | [proc: improve root readdir latency with many threads](https://lore.kernel.org/all/20221202171620.509140-1-bfoster@redhat.com) | TODO | v3 ☐☑✓ | [LORE v3,0/5](https://lore.kernel.org/all/20221202171620.509140-1-bfoster@redhat.com) |
| 2023/01/09 | Yian Chen <yian.chen@intel.com> | [Enable LASS (Linear Address space Separation)](https://lore.kernel.org/all/20230110055204.3227669-1-yian.chen@intel.com) | 参见 LWN 报道 [Support for Intel's LASS](https://lwn.net/Articles/919683) 和 phoronix 报道 [Intel Posts Linux Patches For Linear Address Space Separation (LASS)](https://www.phoronix.com/news/Linear-Address-Space-Separation) | v1 ☐☑✓ | [LORE v1,0/7](https://lore.kernel.org/all/20230110055204.3227669-1-yian.chen@intel.com) |
| 2023/02/05 | Yafang Shao <laoar.shao@gmail.com> | [bpf, mm: introduce cgroup.memory=nobpf](https://patchwork.kernel.org/project/linux-mm/cover/20230205065805.19598-1-laoar.shao@gmail.com/) | 718891 | v1 ☐☑ | [LORE v1,0/5](https://lore.kernel.org/r/20230205065805.19598-1-laoar.shao@gmail.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/4](https://lore.kernel.org/r/20230210154734.4416-1-laoar.shao@gmail.com) |
| 2023/02/06 | Dan Williams <dan.j.williams@intel.com> | [CXL RAM and the'Soft Reserved'=> 'System RAM' default](https://patchwork.kernel.org/project/linux-mm/cover/167564534874.847146.5222419648551436750.stgit@dwillia2-xfh.jf.intel.com/) | 718969 | v1 ☐☑ | [LORE v1,0/18](https://lore.kernel.org/r/167564534874.847146.5222419648551436750.stgit@dwillia2-xfh.jf.intel.com) |
| 2023/02/06 | Alistair Popple <apopple@nvidia.com> | [mm: Introduce a cgroup to limit the amount of locked and pinned memory](https://patchwork.kernel.org/project/linux-mm/cover/cover.c238416f0e82377b449846dbb2459ae9d7030c8e.1675669136.git-series.apopple@nvidia.com/) | 719027 | v1 ☐☑| [LORE v1,0/19](https://lore.kernel.org/r/cover.c238416f0e82377b449846dbb2459ae9d7030c8e.1675669136.git-series.apopple@nvidia.com) |
| 2023/02/06 | Sergey Senozhatsky <senozhatsky@chromium.org> | [zsmalloc: fine-grained fullness grouping](https://patchwork.kernel.org/project/linux-mm/cover/20230206092559.2722946-1-senozhatsky@chromium.org/) | 719058 | v1 ☐☑ | [LORE v1,0/2](https://lore.kernel.org/r/20230206092559.2722946-1-senozhatsky@chromium.org) |
| 2023/02/06 | Charan Teja Kalla <quic_charante@quicinc.com> | [mm: shmem: support POSIX_FADV_[WILL|DONT]NEED for shmem files](https://patchwork.kernel.org/project/linux-mm/cover/cover.1675690847.git.quic_charante@quicinc.com/) | 719143 | v6 ☐☑ | [LORE v6,0/2](https://lore.kernel.org/r/cover.1675690847.git.quic_charante@quicinc.com) |
| 2023/02/11 | Matthew Wilcox <willy@infradead.org> | [New arch interfaces for manipulating multiple pages](https://patchwork.kernel.org/project/linux-mm/cover/20230211033948.891959-1-willy@infradead.org/) | 720910 | v1 ☐☑ | [LORE v1,0/7](https://lore.kernel.org/r/20230211033948.891959-1-willy@infradead.org) |
| 2023/02/13 | Deepak Gupta <debug@rivosinc.com> | [[v1,RFC,Zisslpcfi,05/20] mmap : Introducing new protection "PROT_SHADOWSTACK" for mmap](https://patchwork.kernel.org/project/linux-mm/patch/20230213045351.3945824-6-debug@rivosinc.com/) | 721169 | v1 ☐☑ | [LORE v1,0/20](https://lore.kernel.org/r/20230213045351.3945824-6-debug@rivosinc.com) |
| 2023/02/13 | Deepak Gupta <debug@rivosinc.com> | [[v1,RFC,Zisslpcfi,05/20] mmap : Introducing new protection "PROT_SHADOWSTACK" for mmap](https://patchwork.kernel.org/project/linux-mm/patch/20230213045351.3945824-6-debug@rivosinc.com/) | 721169 | v1 ☐☑ | [LORE v1,0/20](https://lore.kernel.org/r/20230213045351.3945824-6-debug@rivosinc.com) |
| 2023/02/17 | Alexander Halbuer <halbuer@sra.uni-hannover.de> | [mm, page_alloc: batch cma update on pcp buffer refill](https://patchwork.kernel.org/project/linux-mm/patch/20230217120504.87043-1-halbuer@sra.uni-hannover.de/) | 722919 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20230217120504.87043-1-halbuer@sra.uni-hannover.de) |



[[LSF/MM/BFP TOPIC] Storage: Copy Offload](https://lkml.kernel.org/linux-block/f0e19ae4-b37a-e9a3-2be7-a5afb334a5c3@nvidia.com)
[LSFMM: Copy offload](https://lwn.net/Articles/548347)
[Storage: Xcopy Offload](https://blog.csdn.net/flyingnosky/article/details/123533554)


| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2014/05/28 | Martin K. Petersen <martin.petersen@oracle.com> | [Copy offload](https://lore.kernel.org/all/1401335565-29865-1-git-send-email-martin.petersen@oracle.com) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/1401335565-29865-1-git-send-email-martin.petersen@oracle.com) |
| 2022/11/23 | Nitesh Shetty <nj.shetty@samsung.com> | [Implement copy offload support](https://lore.kernel.org/all/20221123055827.26996-1-nj.shetty@samsung.com) | TODO | v5 ☐☑✓ | [LORE v5,0/10](https://lore.kernel.org/all/20221123055827.26996-1-nj.shetty@samsung.com) |







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




| 2018/11/11 | Paul E. McKenney <paulmck@linux.ibm.com> | [Automate initrd generation for v4.21/v5.0](https://lore.kernel.org/all/20181111200127.GA9511@linux.ibm.com) | 内核中引入 nolibc, 参见 LWN 报道 [Nolibc: a minimal C-library replacement shipped with the kernel](https://lwn.net/Articles/920158) | v5 ☐☑✓ | [LORE v5,0/8](https://lore.kernel.org/all/20181111200127.GA9511@linux.ibm.com) |
| 2023/01/30 | Fan Wu <wufan@linux.microsoft.com> | [Integrity Policy Enforcement LSM (IPE)](https://lore.kernel.org/all/1675119451-23180-1-git-send-email-wufan@linux.microsoft.com) | TODO | v9 ☐☑✓ | [LORE v9,0/16](https://lore.kernel.org/all/1675119451-23180-1-git-send-email-wufan@linux.microsoft.com) |
| 2022/12/30 | Dmitrii Bundin <dmitrii.bundin.a@gmail.com> | [scripts/gdb: add mm introspection utils](https://patchwork.kernel.org/project/linux-mm/patch/20221230163512.23736-1-dmitrii.bundin.a@gmail.com) | 707744 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221230163512.23736-1-dmitrii.bundin.a@gmail.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/1](https://lore.kernel.org/r/20221231171258.7907-1-dmitrii.bundin.a@gmail.com)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/1](https://lore.kernel.org/r/20230101172312.21452-1-dmitrii.bundin.a@gmail.com) |




[McKenney: What Does It Mean To Be An RCU Implementation?](https://lwn.net/Articles/921351)

[GFP flags and the end of GFP_ATOMIC](https://lwn.net/Articles/920891)

[Linux Kernel Podcast](https://kernelpodcast.org)

[Reconsidering BPF ABI stability](https://lwn.net/Articles/921088)

| 2023/01/13 | Mel Gorman <mgorman@techsingularity.net> | [Discard `__GFP_ATOMIC`](https://lore.kernel.org/all/20230113111217.14134-1-mgorman@techsingularity.net) | TODO | v3 ☐☑✓ | [LORE v2,0/6](https://lore.kernel.org/all/20230109151631.24923-1-mgorman@techsingularity.net)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/6](https://lore.kernel.org/all/20230113111217.14134-1-mgorman@techsingularity.net) |

[Linux Developers Evaluating New"DOITM"Security Mitigation For Latest Intel CPUs](https://www.phoronix.com/review/intel-doitm-linux)



| 2023/01/09 | T.J. Mercier <tjmercier@google.com> | [Track exported dma-buffers with memcg](https://patchwork.kernel.org/project/linux-mm/cover/20230109213809.418135-1-tjmercier@google.com/) | 710263 | v1 ☐☑ | [LORE v1,0/4](https://lore.kernel.org/r/20230109213809.418135-1-tjmercier@google.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/4](https://lore.kernel.org/r/20230123191728.2928839-1-tjmercier@google.com) |
| 2023/01/13 | David Hildenbrand <david@redhat.com> | [mm: support `__HAVE_ARCH_PTE_SWP_EXCLUSIVE` on all architectureswith swap PTEs](https://patchwork.kernel.org/project/linux-mm/cover/20230113171026.582290-1-david@redhat.com/) | 711859 | v1 ☐☑ | [LORE v1,0/26](https://lore.kernel.org/r/20230113171026.582290-1-david@redhat.com) |
| 2023/01/17 | Jann Horn <jannh@google.com> | [fork, vmalloc: KASAN-poison backing pages of vmapped stacks](https://patchwork.kernel.org/project/linux-mm/patch/20230117163543.1049025-1-jannh@google.com/) | 712819 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20230117163543.1049025-1-jannh@google.com) |
| 2023/01/18 | Nicholas Piggin <npiggin@gmail.com> | [shoot lazy tlbs](https://patchwork.kernel.org/project/linux-mm/cover/20230118080011.2258375-1-npiggin@gmail.com/) | 713096 | v6 ☐☑ | [LORE v6,0/5](https://lore.kernel.org/r/20230118080011.2258375-1-npiggin@gmail.com) |
| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2022/01/08 | Andy Lutomirski <luto@kernel.org> | [mm, sched: Rework lazy mm handling](https://lore.kernel.org/all/cover.1641659630.git.luto@kernel.org) | TODO | v1 ☐☑✓ | [LORE v1,0/23](https://lore.kernel.org/all/cover.1641659630.git.luto@kernel.org) |

| 2023/03/03 | Marcelo Tosatti <mtosatti@redhat.com> | [fold per-CPU vmstats remotely](https://patchwork.kernel.org/project/linux-mm/cover/20230303195841.310844446@redhat.com/) | 726527 | v3 ☐☑ | [LORE v1,0/5](https://lore.kernel.org/r/20230201195013.881721887@redhat.com)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/11](https://lore.kernel.org/r/20230303195841.310844446@redhat.com)<br>*-*-*-*-*-*-*-* <br>[LORE v4,0/12](https://lore.kernel.org/r/20230305133657.255737580@redhat.com)<br>*-*-*-*-*-*-*-* <br>[LORE v5,0/12](https://lore.kernel.org/r/20230313162507.032200398@redhat.com)<br>*-*-*-*-*-*-*-* <br>[LORE v7,0/13](https://lore.kernel.org/r/20230320180332.102837832@redhat.com) |

| 2023/03/06 | James Houghton <jthoughton@google.com> | [mm: rmap: merge HugeTLB mapcount logic with THPs](https://patchwork.kernel.org/project/linux-mm/cover/20230306230004.1387007-1-jthoughton@google.com/) | 727125 | v1 ☐☑ | [LORE v1,0/2](https://lore.kernel.org/r/20230306230004.1387007-1-jthoughton@google.com) |
| 2023/03/08 | Mike Rapoport <rppt@kernel.org> | [Prototype for direct map awareness in page allocator](https://patchwork.kernel.org/project/linux-mm/cover/20230308094106.227365-1-rppt@kernel.org/) | 727808 | v1 ☐☑ | [LORE v1,0/5](https://lore.kernel.org/r/20230308094106.227365-1-rppt@kernel.org) |
| 2023/03/14 | chenjun (AM) <chenjun102@huawei.com> | [mm/slub: Reduce memory consumption in extreme scenarios](https://patchwork.kernel.org/project/linux-mm/patch/20230314123403.100158-1-chenjun102@huawei.com/) | 729899 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20230314123403.100158-1-chenjun102@huawei.com) |
| 2023/03/16 | Gou Hao <gouhao@uniontech.com> | [mm/slub: reduce the calculation times of'MAX_OBJS_PER_PAGE'](https://patchwork.kernel.org/project/linux-mm/patch/20230316012517.10479-1-gouhao@uniontech.com/) | 730565 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20230316012517.10479-1-gouhao@uniontech.com) |
| 2023/03/16 | Yang Yang <yang.yang29@zte.com.cn> | [[linux-next] mm: workingset: simplify the calculation of workingset size](https://patchwork.kernel.org/project/linux-mm/patch/202303161723055514455@zte.com.cn/) | 730688 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/202303161723055514455@zte.com.cn) |
| 2023/03/16 | Alexandre Ghiti <alexghiti@rivosinc.com> | [riscv: Use PUD/P4D/PGD pages for the linear mapping](https://patchwork.kernel.org/project/linux-mm/cover/20230316131711.1284451-1-alexghiti@rivosinc.com/) | 730795 | v8 ☐☑ | [LORE v8,0/4](https://lore.kernel.org/r/20230316131711.1284451-1-alexghiti@rivosinc.com) |
| 2023/03/16 | Ritesh Harjani (IBM) <ritesh.list@gmail.com> | [[RFCv1,WIP] ext2: Move direct-io to use iomap](https://patchwork.kernel.org/project/linux-mm/patch/eae9d2125de1887f55186668937df7475b0a33f4.1678977084.git.ritesh.list@gmail.com/) | 730837 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/eae9d2125de1887f55186668937df7475b0a33f4.1678977084.git.ritesh.list@gmail.com) |

### 4.7.5 sync wakeup
-------




[How to Use Performance Monitor Unit(PMU) of 64-bit ARMv8-A in Linux](https://blog.csdn.net/omnispace/article/details/79517182)

[Linux Will Stop Randomizing Per-CPU Entry Area When KASLR Is Not Active](https://www.phoronix.com/news/Linux-Random-Per-CPU-Entry-ASLR)

[Linux 6.4 Preparing DRM Deadline Hints To Help Influence GPU Frequency/Performance](https://www.phoronix.com/news/DMA-BUF-Fence-Deadline-6.4)


[Reducing direct-map fragmentation with `__GFP_UNMAPPED`](https://lwn.net/Articles/926020)
[Generic iterators for BPF](https://lwn.net/Articles/926041)


[Zephyr: a modular OS for resource-constrained devices](https://lwn.net/Articles/925924)



| 2023/04/01 | Xi Wang <xii@google.com> | [Morphing CFS into FDL, The Fair Deadline Scheduling Class](https://lore.kernel.org/all/20230401230556.2781604-1-xii@google.com) | TODO | v1 ☐☑✓ | [LORE v1,0/1](https://lore.kernel.org/all/20230401230556.2781604-1-xii@google.com) |
| 2023/03/30 | David Dai <davidai@google.com> | [Improve VM CPUfreq and task placement behavior](https://lore.kernel.org/all/20230331014356.1033759-1-davidai@google.com) | [CPUfreq/sched and VM guest workload problems](https://lpc.events/event/16/contributions/1195) | v2 ☐☑✓ | [LORE v2,0/6](https://lore.kernel.org/all/20230331014356.1033759-1-davidai@google.com) |








| 2023/04/26 | Bouska, Zdenek"<zdenek.bouska@siemens.com> | [Unfair qspinlocks on ARM64 without LSE atomics => 3ms delay in interrupt handling](https://patchwork.kernel.org/project/linux-mm/patch/20221230163512.23736-1-dmitrii.bundin.a@gmail.com) | 707744 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/all/AS1PR10MB567534190B05A4493674173BEB659@AS1PR10MB5675.EURPRD10.PROD.OUTLOOK.COM) |

[Re: [Question]: try to fix contention between expire_timers and try_to_del_timer_sync](https://lore.kernel.org/lkml/20170728092831.GA24839@arm.com)








使用 numa 扫描增强功能 [LORE v3,0/4](https://lore.kernel.org/all/cover.1677672277.git.raghavendra.kt@amd.com), 只允许以前访问过 vma 的线程进行扫描. 虽然这改善了显著的系统时间开销, 但也存在一些角落情况, 这确实需要一些放松, 例如 PeterZ 提出的担忧, 即属于不相交的 VMS 集合的线程之间的不公平可能会放大属于未扫描的一些任务的 vma 区域的副作用. 目前, 这是通过无条件地允许 mm 级别 (mm->numa_scan_seq) 的前两次扫描来处理的.<br> 其中一个具有类似副作用的测试是 numa01_THREAD_ALLOC, 其中通过主线程进行分配, 并将其划分为 24MB 的内存块以连续地归零.



(这是 LKP 测试默认运行的, 而 numa01 是 mmtests 默认运行的 (mmtests 由每个线程在整个 3GB 区域上操作)

因此, 为了解决这个问题, 这里的建议是：



1) 每个 vma 扫描计数器, 每次成功扫描 (可能扫描 256MB 或 sysctl_scan_size) 都会递增 2) 前几次进行无条件扫描 (准确地说, 正常扫描时会计算出一半的窗口)3) 在 vma 级别扫描整个 mm 时重置计数器 (这需要记住 mm->numa_scan_sequece)

有了这个补丁, 我看到 numa01_THREAD_ALLOC 情况有了很好的改善, 但请注意, 在 [1] 中, 当基准测试运行时, 系统时间大幅减少, 这个补丁增加了一些系统时间.






这是将根 cfg_rq runnable_avg 作为一种考虑 CPU 频率和 "migrate_util" 类型负载平衡最繁忙 CPU 选择的 CPU 争用的方法的想法的实现.



https://lkml.kernel.org/r/424e2c81-987d-f10e-106d-8b4c611768bc@arm.com 网站

| 2023/05/12 | Dietmar Eggemann <dietmar.eggemann@arm.com> | [sched: Consider CPU contention in frequency, EAS max util & load-balance busiest CPU selection](https://lore.kernel.org/all/20230512101029.342823-1-dietmar.eggemann@arm.com) | TODO | v2 ☐☑✓ | [LORE v1,0/1](https://lore.kernel.org/all/20230406155030.1989554-1-dietmar.eggemann@arm.com)[LORE v2,0/2](https://lore.kernel.org/all/20230512101029.342823-1-dietmar.eggemann@arm.com) |




[Discussion](https://lkml.kernel.org/r/424e2c81-987d-f10e-106d-8b4c611768bc@arm.com)


| 2023/03/07 | Jens Axboe <axboe@kernel.dk> | [Add FMODE_NOWAIT support to pipes](https://lore.kernel.org/all/20230308031033.155717-1-axboe@kernel.dk) | [Pipe FMODE_NOWAIT Support Sent In For Linux 6.4 As A 10~23x Performance Improvement](https://www.phoronix.com/news/Pipe-FMODE_NOWAIT-Linux-6.4). | v1 ☐☑✓ | [LORE v1,0/3](https://lore.kernel.org/all/20230308031033.155717-1-axboe@kernel.dk) |
| 2023/04/03 | Lorenzo Stoakes <lstoakes@gmail.com> | [permit write-sealed memfd read-only shared mappings](https://patchwork.kernel.org/project/linux-mm/cover/cover.1680560277.git.lstoakes@gmail.com/) | 736579 | v1 ☐☑ | [LORE v1,0/3](https://lore.kernel.org/r/cover.1680560277.git.lstoakes@gmail.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/3](https://lore.kernel.org/r/cover.1682890156.git.lstoakes@gmail.com) |
| 2023/04/04 | Shaun Tancheff <shaun.tancheff@gmail.com> | [memcg-v1: Enable setting memory min, low, high](https://patchwork.kernel.org/project/linux-mm/patch/20230404205013.31520-1-shaun.tancheff@gmail.com/) | 736981 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20230404205013.31520-1-shaun.tancheff@gmail.com)[LORE v1,0/1](https://lore.kernel.org/r/20230405110107.127156-1-shaun.tancheff@gmail.com) |
| 2023/04/05 | Ankit Agrawal <ankita@nvidia.com> | [Expose GPU memory as coherently CPU accessible](https://patchwork.kernel.org/project/linux-mm/cover/20230405180134.16932-1-ankita@nvidia.com/) | 737297 | v3 ☐☑ | [LORE v3,0/6](https://lore.kernel.org/r/20230405180134.16932-1-ankita@nvidia.com) |
| 2023/04/05 | Luis Chamberlain <mcgrof@kernel.org> | [kmod: simplify with a semaphore](https://patchwork.kernel.org/project/linux-mm/cover/20230405203505.1343562-1-mcgrof@kernel.org/) | 737345 | v2 ☐☑ | [LORE v2,0/2](https://lore.kernel.org/r/20230405203505.1343562-1-mcgrof@kernel.org) |
| 2023/04/06 | Jaewon Kim <jaewon31.kim@samsung.com> | [[v2] dma-buf/heaps: system_heap: Avoid DoS by limiting single allocations to half of all memory](https://patchwork.kernel.org/project/linux-mm/patch/20230406000854.25764-1-jaewon31.kim@samsung.com/) | 737382 | v2 ☐☑ | [LORE v2,0/1](https://lore.kernel.org/r/20230406000854.25764-1-jaewon31.kim@samsung.com) |
| 2023/04/06 | Shaun Tancheff <shaun.tancheff@gmail.com> | [memcg: Default value setting in memcg-v1](https://patchwork.kernel.org/project/linux-mm/patch/20230406091450.167779-1-shaun.tancheff@gmail.com/) | 737508 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20230406091450.167779-1-shaun.tancheff@gmail.com) |

| 2023/04/08 | Yang Yang <yang.yang29@zte.com.cn> | [[linux-next] delayacct: track delays from IRQ/SOFTIRQ](https://patchwork.kernel.org/project/linux-mm/patch/202304081728353557233@zte.com.cn/) | 738160 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/202304081728353557233@zte.com.cn) |
| 2023/04/10 | Gang Li <ligang.bdlg@bytedance.com> | [[v3] mm: oom: introduce cpuset oom](https://patchwork.kernel.org/project/linux-mm/patch/20230410025056.22103-1-ligang.bdlg@bytedance.com/) | 738331 | v3 ☐☑ | [LORE v3,0/1](https://lore.kernel.org/r/20230410025056.22103-1-ligang.bdlg@bytedance.com) |
| 2023/04/10 | Jaewon Kim <jaewon31.kim@samsung.com> | [[v3] dma-buf/heaps: system_heap: avoid too much allocation](https://patchwork.kernel.org/project/linux-mm/patch/20230410073228.23043-1-jaewon31.kim@samsung.com/) | 738348 | v3 ☐☑ | [LORE v3,0/1](https://lore.kernel.org/r/20230410073228.23043-1-jaewon31.kim@samsung.com) |
| 2023/04/17 | Vishal Moola <vishal.moola@gmail.com> | [Split ptdesc from struct page](https://patchwork.kernel.org/project/linux-mm/cover/20230417205048.15870-1-vishal.moola@gmail.com/) | 740662 | v1 ☐☑ | [LORE v1,0/33](https://lore.kernel.org/r/20230417205048.15870-1-vishal.moola@gmail.com) |
| 2023/04/18 | Романов Алексей Васильевич <avromanov@sberdevices.ru> | [Introduce objects folding mechanism](https://patchwork.kernel.org/project/linux-mm/cover/20230418062503.62121-1-avromanov@sberdevices.ru/) | 740774 | v1 ☐☑ | [LORE v1,0/5](https://lore.kernel.org/r/20230418062503.62121-1-avromanov@sberdevices.ru) |
| 2023/04/18 | Luis Chamberlain <mcgrof@kernel.org> | [[v4] module: add debug stats to help identify memory pressure](https://patchwork.kernel.org/project/linux-mm/patch/ZD7jaL8s+pPolwU1@bombadil.infradead.org/) | 741080 | v4 ☐☑ | [LORE v4,0/1](https://lore.kernel.org/r/ZD7jaL8s+pPolwU1@bombadil.infradead.org) |
| 2023/04/18 | Johannes Weiner <hannes@cmpxchg.org> | [mm: reliable huge page allocator](https://patchwork.kernel.org/project/linux-mm/cover/20230418191313.268131-1-hannes@cmpxchg.org/) | 741091 | v1 ☐☑ | [LORE v1,0/26](https://lore.kernel.org/r/20230418191313.268131-1-hannes@cmpxchg.org) |
| 2023/04/18 | Waiman Long <longman@redhat.com> | [mm/mmap: Map MAP_STACK to VM_STACK](https://patchwork.kernel.org/project/linux-mm/patch/20230418210230.3495922-1-longman@redhat.com/) | 741110 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20230418210230.3495922-1-longman@redhat.com) |
| 2023/04/18 | Luis Chamberlain <mcgrof@kernel.org> | [module: add debugging auto-load duplicate module support](https://patchwork.kernel.org/project/linux-mm/patch/20230418204636.791699-1-mcgrof@kernel.org/) | 741107 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20230418204636.791699-1-mcgrof@kernel.org) |
| 2023/04/18 | Waiman Long <longman@redhat.com> | [mm/mmap: Map MAP_STACK to VM_STACK](https://patchwork.kernel.org/project/linux-mm/patch/20230418210230.3495922-1-longman@redhat.com/) | 741110 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20230418210230.3495922-1-longman@redhat.com) |
| 2023/04/19 | Suren Baghdasaryan <surenb@google.com> | [[v3,1/1] mm: do not increment pgfault stats when page fault handler retries](https://patchwork.kernel.org/project/linux-mm/patch/20230419175836.3857458-1-surenb@google.com/) | 741439 | v3 ☐☑ | [LORE v3,0/1](https://lore.kernel.org/r/20230419175836.3857458-1-surenb@google.com) |
| 2023/04/19 | Atish Kumar Patra <atishp@rivosinc.com> | [RISC-V CoVE support](https://patchwork.kernel.org/project/linux-mm/cover/20230419222350.3604274-1-atishp@rivosinc.com/) | 741490 | v1 ☐☑ | [LORE v1,0/10](https://lore.kernel.org/r/20230419222350.3604274-1-atishp@rivosinc.com) |
| 2023/04/21 | Oscar Salvador <osalvador@suse.de> | [page_owner: print stacks and their counter](https://patchwork.kernel.org/project/linux-mm/cover/20230421101415.5734-1-osalvador@suse.de/) | 742055 | v4 ☐☑ | [LORE v4,0/3](https://lore.kernel.org/r/20230421101415.5734-1-osalvador@suse.de) |
| 2023/04/21 | Yosry Ahmed <yosryahmed@google.com> | [cgroup: eliminate atomic rstat flushing](https://patchwork.kernel.org/project/linux-mm/cover/20230421174020.2994750-1-yosryahmed@google.com/) | 742246 | v5 ☐☑ | [LORE v5,0/5](https://lore.kernel.org/r/20230421174020.2994750-1-yosryahmed@google.com) |
| 2023/04/21 | Luis Chamberlain <mcgrof@kernel.org> | [shmem: add support for blocksize> PAGE_SIZE](https://patchwork.kernel.org/project/linux-mm/cover/20230421214400.2836131-1-mcgrof@kernel.org/) | 742301 | v1 ☐☑ | [LORE v1,0/8](https://lore.kernel.org/r/20230421214400.2836131-1-mcgrof@kernel.org) |
| 2023/04/24 | Jeff Layton <jlayton@kernel.org> | [fs: multigrain timestamps](https://patchwork.kernel.org/project/linux-mm/cover/20230424151104.175456-1-jlayton@kernel.org/) | 742778 | v2 ☐☑ | [LORE v2,0/3](https://lore.kernel.org/r/20230424151104.175456-1-jlayton@kernel.org) |


Web Pilot 根据 URL 生成文章摘要, 总结, 翻译.
AskYourPDF 从 PDF 文件内容生成摘要, 分析内容等.
Chat with PDF
kagi.com/summarizer


[A kernel without buffer heads](https://lwn.net/Articles/930173)
[Sunsetting buffer heads](https://lwn.net/Articles/931809)

[Unprivileged BPF and authoritative security hooks](https://lwn.net/Articles/929746)
[Designated movable (memory) blocks](https://lwn.net/Articles/928795)
[The ongoing trouble with get_user_pages()](https://lwn.net/Articles/930667)
[A storage standards update at LSFMM+BPF](https://lwn.net/Articles/931282)
[Memory overcommit in containerized environments](https://lwn.net/Articles/931662)

[Live migration of virtual machines over CXL](https://lwn.net/Articles/931528)
[Memory-management changes for CXL](https://lwn.net/Articles/931416)

[The future of memory tiering](https://lwn.net/Articles/931421)
[Peer-to-peer DMA](https://lwn.net/Articles/931668)
[Computational storage](https://lwn.net/Articles/931949)
[The state of the page in 2023](https://lwn.net/Articles/931794)


[Fighting the zombie-memcg invasion](https://lwn.net/Articles/932070)

[[LSF/MM/BPF TOPIC] userspace control of memory management](https://lore.kernel.org/linux-mm/CAPTztWYAiroY3E8pwB+rnPGA1K9HLhkpQp1Gy9C1dEuS1FhWGg@mail.gmail.com)

[The 2023 LSFMM+BPF Summit](https://lwn.net/Articles/lsfmmbpf2023)
[An LSFMM development-process discussion](https://lwn.net/Articles/932215)


[Memory passthrough for virtual machines](https://lwn.net/Articles/931933)


BPF verifiery 已经做了很多工作来尽量确保加载进 kernel 的 BPF program 是不会导致安全问题的. 包括检查 memory 的访问, 以及模拟执行流程来确保 program 会在有限时间内结束, 等等. 其中很多检查都有助于确保 program 是安全的, 能避免某些类型的 bug, 其他的检查都是专门为了排查恶意 program 的, 如果 kernel 允许接受非特权用户的 BPF program 的话, 这些检查都是必须的.

大多数这类恶意 program 的检查工作都是在 2015 年的 4.4 kernel 里实现的. 尤其是其中很多工作是为了阻止 BPF program 把 kernel 指针值泄露给 user space. 这些指针对攻击者会很有用, 因为他们可以被用来推算出某些特定的数据结构或者代码的位置. 所以我们一定要避免被非特权的进程拿到这些指针数据. 在 kernel 4.7 里面


 其他还有一些 patch 是用来避免 BPF program 里的预测执行(speculative-execution)类型的攻击.



