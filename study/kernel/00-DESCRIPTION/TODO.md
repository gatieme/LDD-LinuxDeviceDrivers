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
| 2022/12/12 | David Hildenbrand <david@redhat.com> | [[mm-stable] mm/GUP_test: free memory allocated via kvcalloc() using kvfree()](https://patchwork.kernel.org/project/linux-mm/patch/20221212182018.264900-1-david@redhat.com/) | 703907 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20221212182018.264900-1-david@redhat.com) |
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
| 2023/02/13 | Deepak GUPta <debug@rivosinc.com> | [[v1,RFC,Zisslpcfi,05/20] mmap : Introducing new protection "PROT_SHADOWSTACK" for mmap](https://patchwork.kernel.org/project/linux-mm/patch/20230213045351.3945824-6-debug@rivosinc.com/) | 721169 | v1 ☐☑ | [LORE v1,0/20](https://lore.kernel.org/r/20230213045351.3945824-6-debug@rivosinc.com) |
| 2023/02/13 | Deepak GUPta <debug@rivosinc.com> | [[v1,RFC,Zisslpcfi,05/20] mmap : Introducing new protection "PROT_SHADOWSTACK" for mmap](https://patchwork.kernel.org/project/linux-mm/patch/20230213045351.3945824-6-debug@rivosinc.com/) | 721169 | v1 ☐☑ | [LORE v1,0/20](https://lore.kernel.org/r/20230213045351.3945824-6-debug@rivosinc.com) |
| 2023/02/17 | Alexander Halbuer <halbuer@sra.uni-hannover.de> | [mm, page_alloc: batch cma update on pcp buffer refill](https://patchwork.kernel.org/project/linux-mm/patch/20230217120504.87043-1-halbuer@sra.uni-hannover.de/) | 722919 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20230217120504.87043-1-halbuer@sra.uni-hannover.de) |






| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2014/05/28 | Martin K. Petersen <martin.petersen@oracle.com> | [Copy offload](https://lore.kernel.org/all/1401335565-29865-1-git-send-email-martin.petersen@oracle.com) | [Storage: Xcopy Offload](https://blog.csdn.net/flyingnosky/article/details/123533554) | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/1401335565-29865-1-git-send-email-martin.petersen@oracle.com) |
| 2022/11/23 | Nitesh Shetty <nj.shetty@samsung.com> | [Implement copy offload support](https://lore.kernel.org/all/20221123055827.26996-1-nj.shetty@samsung.com) | [[LSF/MM/BFP TOPIC] Storage: Copy Offload](https://lkml.kernel.org/linux-block/f0e19ae4-b37a-e9a3-2be7-a5afb334a5c3@nvidia.com), [LSFMM: Copy offload](https://lwn.net/Articles/548347), [Merging copy offload](https://lwn.net/Articles/935260) | v5 ☐☑✓ | [LORE v5,0/10](https://lore.kernel.org/all/20221123055827.26996-1-nj.shetty@samsung.com)<br>*-*-*-*-*-*-*-* <br>[LORE v11,0/9](https://lore.kernel.org/r/20230522104146.2856-1-nj.shetty@samsung.com) |

| 2023/05/22 | Jiri Olsa <jolsa@kernel.org> | [[RFC,stable,5.4,1/8] uaccess: Add strict non-pagefault kernel-space read function](https://patchwork.kernel.org/project/linux-mm/patch/20230522203352.738576-2-jolsa@kernel.org/) | 749930 | v1 ☐☑ | [LORE v1,0/8](https://lore.kernel.org/r/20230522203352.738576-2-jolsa@kernel.org) |
| 2023/05/22 | David Howells <dhowells@redhat.com> | [block: Use page pinning](https://patchwork.kernel.org/project/linux-mm/cover/20230522205744.2825689-1-dhowells@redhat.com/) | 749937 | v21 ☐☑ | [LORE v21,0/6](https://lore.kernel.org/r/20230522205744.2825689-1-dhowells@redhat.com) |
| 2023/05/23 | Jesper Dangaard Brouer <brouer@redhat.com> | [page_pool: new approach for leak detection and shutdown phase](https://patchwork.kernel.org/project/linux-mm/cover/168485351546.2849279.13771638045665633339.stgit@firesoul/) | 750272 | v4 ☐☑ | [LORE v4,0/2](https://lore.kernel.org/r/168485351546.2849279.13771638045665633339.stgit@firesoul) |






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

使用 -1 表示 CPU 不适合只是因为 uclamp_min, 因此我们可以使用这个新值采取额外的操作, 以选择不符合 uclamp_min 提示的最佳 CPU.


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

| 2023/03/03 | Marcelo Tosatti <mtosatti@redhat.com> | [fold per-CPU vmstats remotely](https://patchwork.kernel.org/project/linux-mm/cover/20230303195841.310844446@redhat.com/) | 726527 | v3 ☐☑ | [LORE v1,0/5](https://lore.kernel.org/r/20230201195013.881721887@redhat.com)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/11](https://lore.kernel.org/r/20230303195841.310844446@redhat.com)<br>*-*-*-*-*-*-*-* <br>[LORE v4,0/12](https://lore.kernel.org/r/20230305133657.255737580@redhat.com)<br>*-*-*-*-*-*-*-* <br>[LORE v5,0/12](https://lore.kernel.org/r/20230313162507.032200398@redhat.com)<br>*-*-*-*-*-*-*-* <br>[LORE v7,0/13](https://lore.kernel.org/r/20230320180332.102837832@redhat.com)<br>*-*-*-*-*-*-*-* <br>[LORE v8,0/13](https://lore.kernel.org/r/20230515180015.016409657@redhat.com) |
 |

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



| 2023/03/30 | David Dai <davidai@google.com> | [Improve VM CPUfreq and task placement behavior](https://lore.kernel.org/all/20230331014356.1033759-1-davidai@google.com) | [CPUfreq/sched and VM guest workload problems](https://lpc.events/event/16/contributions/1195) | v2 ☐☑✓ | [LORE v2,0/6](https://lore.kernel.org/all/20230331014356.1033759-1-davidai@google.com) |








| 2023/04/26 | Bouska, Zdenek"<zdenek.bouska@siemens.com> | [Unfair qspinlocks on ARM64 without LSE atomics => 3ms delay in interrupt handling](https://patchwork.kernel.org/project/linux-mm/patch/20221230163512.23736-1-dmitrii.bundin.a@gmail.com) | 707744 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/all/AS1PR10MB567534190B05A4493674173BEB659@AS1PR10MB5675.EURPRD10.PROD.OUTLOOK.COM) |

[Re: [Question]: try to fix contention between expire_timers and try_to_del_timer_sync](https://lore.kernel.org/lkml/20170728092831.GA24839@arm.com)








使用 numa 扫描增强功能 [LORE v3,0/4](https://lore.kernel.org/all/cover.1677672277.git.raghavendra.kt@amd.com), 只允许以前访问过 vma 的线程进行扫描. 虽然这改善了显著的系统时间开销, 但也存在一些角落情况, 这确实需要一些放松, 例如 PeterZ 提出的担忧, 即属于不相交的 VMS 集合的线程之间的不公平可能会放大属于未扫描的一些任务的 vma 区域的副作用. 目前, 这是通过无条件地允许 mm 级别 (mm->numa_scan_seq) 的前两次扫描来处理的.<br> 其中一个具有类似副作用的测试是 numa01_THREAD_ALLOC, 其中通过主线程进行分配, 并将其划分为 24MB 的内存块以连续地归零.



(这是 LKP 测试默认运行的, 而 numa01 是 mmtests 默认运行的 (mmtests 由每个线程在整个 3GB 区域上操作)

因此, 为了解决这个问题, 这里的建议是:



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
[Memory-management documentation and development process](https://lwn.net/Articles/932398)

BPF verifiery 已经做了很多工作来尽量确保加载进 kernel 的 BPF program 是不会导致安全问题的. 包括检查 memory 的访问, 以及模拟执行流程来确保 program 会在有限时间内结束, 等等. 其中很多检查都有助于确保 program 是安全的, 能避免某些类型的 bug, 其他的检查都是专门为了排查恶意 program 的, 如果 kernel 允许接受非特权用户的 BPF program 的话, 这些检查都是必须的.

大多数这类恶意 program 的检查工作都是在 2015 年的 4.4 kernel 里实现的. 尤其是其中很多工作是为了阻止 BPF program 把 kernel 指针值泄露给 user space. 这些指针对攻击者会很有用, 因为他们可以被用来推算出某些特定的数据结构或者代码的位置. 所以我们一定要避免被非特权的进程拿到这些指针数据. 在 kernel 4.7 里面

其他还有一些 patch 是用来避免 BPF program 里的预测执行 (speculative-execution) 类型的攻击.





| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2014/10/17 | Paul Gortmaker <paul.gortmaker@windriver.com> | [simple wait queue support (from -rt)](https://lore.kernel.org/all/1413591782-23453-1-git-send-email-paul.gortmaker@windriver.com) | Simple wait queues 支持在 preempt-rt 内核中已经存在了相当长的一段时间 (至少从 3.4 开始). 在今年的 RT 峰会上, 我们一致认为, 对其进行最终清理并将其纳入主流是有意义的. 它类似于普通的等待队列支持, 但没有一些使用较少的功能, 与普通的等待排队相比, 占用空间更小. 对于非 RT, 我们仍然可以从足迹减少系数中受益. 在本系列中, 我们将简单的等待队列部署在两个位置: (1) 用于完成量, (2) 用于 RCU 处理. 参考 LWN 报道 [Simple wait queues](https://lwn.net/Articles/577370). | v2 ☐☑✓ | [LORE v1,0/3](https://lore.kernel.org/all/1386810399-8973-1-git-send-email-paul.gortmaker@windriver.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/7](https://lore.kernel.org/all/1413591782-23453-1-git-send-email-paul.gortmaker@windriver.com) |
| 2016/02/19 | Daniel Wagner <wagi@monom.org> | [Simple wait queue support](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=abedf8e2419fb873d919dd74de2e84b510259339) | Simple wait queues 支持. | v8 ☐☑✓ 4.6-rc1 | [LORE v8,0/5](https://lore.kernel.org/all/1455871601-27484-1-git-send-email-wagi@monom.org) |



| 2023/06/14 | Liupu Wang <wangliupu@loongson.cn> | [LoongArch: Add SMT (Simultaneous Multi-Threading) support](https://lore.kernel.org/all/20230614093755.88881-1-wangliupu@loongson.cn) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20230614093755.88881-1-wangliupu@loongson.cn) |



| 2023/05/04 | Florent Revest <revest@chromium.org> | [MDWE without inheritance](https://patchwork.kernel.org/project/linux-mm/cover/20230504170942.822147-1-revest@chromium.org/) | 745049 | v1 ☐☑ | [LORE v1,0/4](https://lore.kernel.org/r/20230504170942.822147-1-revest@chromium.org)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/5](https://lore.kernel.org/r/20230517150321.2890206-1-revest@chromium.org) |




| 2023/05/08 | zhaoyang.huang <zhaoyang.huang@unisoc.com> | [Optimization of CMA allocation and reclaiming](https://patchwork.kernel.org/project/linux-mm/cover/1683538383-19685-1-git-send-email-zhaoyang.huang@unisoc.com/) | 745744 | v1 ☐☑ | [LORE v1,0/2](https://lore.kernel.org/r/1683538383-19685-1-git-send-email-zhaoyang.huang@unisoc.com) |
| 2023/05/15 | Jeff Xu <jeffxu@chromium.org> | [Memory Mapping (VMA) protection using PKU - set 1](https://patchwork.kernel.org/project/linux-mm/cover/20230515130553.2311248-1-jeffxu@chromium.org/) | 747576 | v1 ☐☑ | [LORE v1,0/6](https://lore.kernel.org/r/20230515130553.2311248-1-jeffxu@chromium.org)<br>*-*-*-*-*-*-*-* <br>[LORE v1,0/6](https://lore.kernel.org/r/20230519011915.846407-1-jeffxu@chromium.org)<br>*-*-*-*-*-*-*-* <br>[LORE v1,0/6](https://lore.kernel.org/r/20230519011915.846407-1-jeffxu@chromium.org) |
 |
| 2023/05/19 | Joel Fernandes <joel@joelfernandes.org> | [Optimize mremap during mutual alignment within PMD](https://patchwork.kernel.org/project/linux-mm/cover/20230519190934.339332-1-joel@joelfernandes.org/) | 749361 | v2 ☐☑ | [LORE v2,0/4](https://lore.kernel.org/r/20230519190934.339332-1-joel@joelfernandes.org)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/6](https://lore.kernel.org/r/20230524153239.3036507-1-joel@joelfernandes.org) |
| 2023/06/13 | Peter Xu <peterx@redhat.com> | [mm/GUP: Unify hugetlb, speed up thp](https://patchwork.kernel.org/project/linux-mm/cover/20230613215346.1022773-1-peterx@redhat.com/) | 756868 | v1 ☐☑ | [LORE v1,0/7](https://lore.kernel.org/r/20230613215346.1022773-1-peterx@redhat.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/8](https://lore.kernel.org/r/20230619231044.112894-1-peterx@redhat.com)<br>*-*-*-*-*-*-*-* <br>[LORE v4,0/8](https://lore.kernel.org/r/20230628215310.73782-1-peterx@redhat.com |
| 2023/06/16 | Huang, Ying <ying.huang@intel.com> | [memory tiering: calculate abstract distance based on ACPI HMAT](https://patchwork.kernel.org/project/linux-mm/cover/20230616070538.190042-1-ying.huang@intel.com/) | 757742 | v1 ☐☑ | [LORE v1,0/4](https://lore.kernel.org/r/20230616070538.190042-1-ying.huang@intel.com) |
| 2023/06/16 | Mike Rapoport <rppt@kernel.org> | [mm: jit/text allocator](https://patchwork.kernel.org/project/linux-mm/cover/20230616085038.4121892-1-rppt@kernel.org/) | 757762 | v2 ☐☑ | [LORE v2,0/12](https://lore.kernel.org/r/20230616085038.4121892-1-rppt@kernel.org) |
| 2023/06/16 | Aneesh Kumar K.V <aneesh.kumar@linux.ibm.com> | [Add support for DAX vmemmap optimization for ppc64](https://patchwork.kernel.org/project/linux-mm/cover/20230616110826.344417-1-aneesh.kumar@linux.ibm.com/) | 757832 | v2 ☐☑ | [LORE v2,0/16](https://lore.kernel.org/r/20230616110826.344417-1-aneesh.kumar@linux.ibm.com)<br>*-*-*-*-*-*-*-* <br>[LORE v4,0/13](https://lore.kernel.org/r/20230710160842.56300-1-aneesh.kumar@linux.ibm.com) |
| 2023/06/16 | GONG, Ruiqi <gongruiqi@huaweicloud.com> | [Randomized slab caches for kmalloc()](https://patchwork.kernel.org/project/linux-mm/cover/20230616111843.3677378-1-gongruiqi@huaweicloud.com/) | 757836 | v3 ☐☑ | [LORE v3,0/1](https://lore.kernel.org/r/20230616111843.3677378-1-gongruiqi@huaweicloud.com) |
| 2023/06/16 | Arjun Roy <arjunroy.kdev@gmail.com> | [[net-next,v2] tcp: Use per-vma locking for receive zerocopy](https://patchwork.kernel.org/project/linux-mm/patch/20230616193427.3908429-1-arjunroy.kdev@gmail.com/) | 757998 | v2 ☐☑ | [LORE v2,0/1](https://lore.kernel.org/r/20230616193427.3908429-1-arjunroy.kdev@gmail.com) |
| 2023/06/19 | Alexey Romanov <avromanov@sberdevices.ru> | [Add obj allocated counter for subpages](https://patchwork.kernel.org/project/linux-mm/cover/20230619143506.45253-1-avromanov@sberdevices.ru/) | 758404 | v1 ☐☑ | [LORE v1,0/2](https://lore.kernel.org/r/20230619143506.45253-1-avromanov@sberdevices.ru) |
| 2023/06/20 | Hugh Dickins <hughd@google.com> | [mm: free retracted page table by RCU](https://patchwork.kernel.org/project/linux-mm/cover/54cb04f-3762-987f-8294-91dafd8ebfb0@google.com/) | 758573 | v2 ☐☑ | [LORE v2,0/12](https://lore.kernel.org/r/54cb04f-3762-987f-8294-91dafd8ebfb0@google.com) |
| 2023/06/21 | Matthew Wilcox <willy@infradead.org> | [Remove pagevecs](https://patchwork.kernel.org/project/linux-mm/cover/20230621164557.3510324-1-willy@infradead.org/) | 759217 | v1 ☐☑ | [LORE v1,0/13](https://lore.kernel.org/r/20230621164557.3510324-1-willy@infradead.org) |
| 2023/06/21 | Yuanchu Xie <yuanchu@google.com> | [mm: working set reporting](https://patchwork.kernel.org/project/linux-mm/cover/20230621180454.973862-1-yuanchu@google.com/) | 759245 | v2 ☐☑ |  |
| 2023/06/22 | Kasireddy, Vivek <vivek.kasireddy@intel.com> | [udmabuf: Add back support for mapping hugetlb pages](https://patchwork.kernel.org/project/linux-mm/cover/20230622072710.3707315-1-vivek.kasireddy@intel.com/) | 759373 | v1 ☐☑ | [LORE v1,0/2](https://lore.kernel.org/r/20230622072710.3707315-1-vivek.kasireddy@intel.com) |
| 2023/06/22 | Ryan Roberts <ryan.roberts@arm.com> | [Transparent Contiguous PTEs for User Mappings](https://patchwork.kernel.org/project/linux-mm/cover/20230622144210.2623299-1-ryan.roberts@arm.com/) | 759528 | v1 ☐☑ | [LORE v1,0/14](https://lore.kernel.org/r/20230622144210.2623299-1-ryan.roberts@arm.com) |
| 2023/06/27 | zhaoyang.huang <zhaoyang.huang@unisoc.com> | [mm: introduce statistic for inode's gen&tier](https://patchwork.kernel.org/project/linux-mm/patch/1687857438-29142-1-git-send-email-zhaoyang.huang@unisoc.com/) | 760556 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/1687857438-29142-1-git-send-email-zhaoyang.huang@unisoc.com) |
| 2023/06/27 | Chuck Lever <cel@kernel.org> | [shmemfs stable directory offsets](https://patchwork.kernel.org/project/linux-mm/cover/168789864000.157531.11122232592994999253.stgit@manet.1015granger.net/) | 760743 | v5 ☐☑ | [LORE v5,0/3](https://lore.kernel.org/r/168789864000.157531.11122232592994999253.stgit@manet.1015granger.net) |
| 2023/07/10 | Yajun Deng <yajun.deng@linux.dev> | [dma-contiguous: support numa CMA for specified node](https://patchwork.kernel.org/project/liux-mm/patch/20230710074944.3501810-1-yajun.deng@linux.dev/) | 763917 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20230710074944.3501810-1-ajun.deng@linux.dev)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/1](https://lore.kernel.org/r/20230711110822.1105785-1-yajun.deng@linux.dev) |
| 2023/07/10 | Minchan Kim <minchan@kernel.org> | [zram: use frontswap for zram swap usecase](https://patchwork.kernel.org/project/linux-mm/cove/20230710221659.2473460-1-minchan@kernel.org/) | 764176 | v1 ☐☑ | [LORE v1,0/3](https://lore.kernel.org/r/20230710221659.2473460-1-minchan@kerne.org) |
| 2023/07/20 | Yosry Ahmed <yosryahmed@google.com> | [memory recharging for offline memcgs](https://patchwork.kernel.org/project/linux-mm/cover/20230720070825.992023-1-yosryahmed@google.com/) | 767656 | v1 ☐☑ | [LORE v1,0/8](https://lore.kernel.org/r/20230720070825.992023-1-yosryahmed@google.com) |
| 2023/07/23 | Hyeonggon Yoo <42.hyeyoo@gmail.com> | [An attempt to improve SLUB on NUMA / under memory pressure](https://patchwork.kernel.org/project/linux-mm/cover/20230723190906.4082646-1-42.hyeyoo@gmail.com/) | 768670 | v1 ☐☑ | [LORE v1,0/2](https://lore.kernel.org/r/20230723190906.4082646-1-42.hyeyoo@gmail.com) |
| 2023/07/23 | Hugh Dickins <hughd@google.com> | [[v3,11/13,fix] mm/khugepaged: delete khugepaged_collapse_pte_mapped_thps(): fix](https://patchwork.kernel.org/project/linux-mm/patch/bfc6cab2-497f-32bf-dd5-98dc1987e4a9@google.com/) | 768689 | v3 ☐☑ | [LORE v3,0/13](https://lore.kernel.org/r/bfc6cab2-497f-32bf-dd5-98dc1987e4a9@google.com) |
| 2023/07/24 | Zhongkun He <hezhongkun.hzk@bytedance.com> | [zram: memcg accounting](https://patchwork.kernel.org/project/linux-mm/cover/20230724062143.2244078-1-hezhongkun.hzk@bytedance.com/) | 768727 | v2 ☐☑ | [LORE v2,0/2](https://lore.kernel.org/r/20230724062143.2244078-1-hezhongkun.hzk@bytedance.com) |



| 2023/07/27 | Ryan Roberts <ryan.roberts@arm.com> | [Optimize large folio interaction with deferred split](https://patchwork.kernel.org/project/linux-mm/cover/20230727141837.3386072-1-ryan.roberts@arm.com/) | 770154 | v4 ☐☑ | [LORE v4,0/3](https://lore.kernel.org/r/20230727141837.3386072-1-ryan.roberts@arm.com) |





[Much ado about SBAT](https://lwn.net/Articles/938422)

[Challenges for KernelCI](https://lwn.net/Articles/939538)





[An ioctl() call to detect memory writes](https://lwn.net/Articles/940704)
[BPF iterators for filesystems](https://lwn.net/Articles/937326)
[Exceptions in BPF](https://lwn.net/Articles/938435)
[Beginning the software-interrupt lock pushdown](https://lwn.net/Articles/939973)
[Following up on file-position locking](https://lwn.net/Articles/940808)
[Out-of-memory victim selection with BPF](https://lwn.net/Articles/941614)




[一文读懂｜Linux 进程管理之 CFS 负载均衡](https://www.qinglite.cn/doc/4126647762640fe5c)
[CFS 任务的负载均衡 (load balance)](http://www.wowotech.net/process_management/load_balance_detail.html)
[步道师 Peter-CFS 任务的负载均衡](https://blog.csdn.net/melody157398/article/details/106449788)
[步道师 Peter-CFS 任务的负载均衡 (框架篇)](https://blog.csdn.net/melody157398/article/details/105445504/)
[内核工匠 - CFS 任务的负载均衡](https://blog.csdn.net/feelabclihu/article/details/106435849)


| 2023/07/28 | Fabio M. De Francesco <fmdefrancesco@gmail.com> | [Documentation/page_tables: Add info about MMU/TLB and Page Faults](https://patchwork.kernel.org/project/linux-mm/patch/20230728120054.12306-1-fmdefrancesco@gmail.com/) | 770552 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20230728120054.12306-1-fmdefrancesco@gmail.com) |
| 2023/08/04 | Zhongkun He <hezhongkun.hzk@bytedance.com> | [zram: memcg accounting](https://patchwork.kernel.org/project/linux-mm/cover/20230804075720.207943-1-hezhongkun.hzk@bytedance.com/) | 772958 | v2 ☐☑ | [LORE v2,0/2](https://lore.kernel.org/r/20230804075720.207943-1-hezhongkun.hzk@bytedance.com) |
| 2023/08/04 | Liam Ni <zhiguangni01@gmail.com> | [NUMA:Improve the efficiency of calculating pages loss](https://patchwork.kernel.org/project/linux-mm/patch/CACZJ9cUXiWxDb6hF4JFhWe7Np82k6LopVQ+_AoGFOccN4kjJqA@mail.gmail.com) | 773185 | v3 ☐☑ | [LORE v3,0/1](https://lore.kernel.org/r/CACZJ9cUXiWxDb6hF4JFhWe7Np82k6LopVQ+_AoGFOccN4kjJqA@mail.gmail.com) |
| 2023/08/08 | Yan Zhao <yan.y.zhao@intel.com> | [Reduce NUMA balance caused TLB-shootdowns in a VM](https://patchwork.kernel.org/project/linux-mm/cover/20230808071329.19995-1-yan.y.zhao@intel.com/) | 773948 | v1 ☐☑ | [LORE v1,0/3](https://lore.kernel.org/r/20230808071329.19995-1-yan.y.zhao@intel.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/5](https://lore.kernel.org/r/20230810085636.25914-1-yan.y.zhao@intel.com) |
| 2023/08/08 | Jinliang Zheng <alexjlzheng@gmail.com> | [writeback: remove redundant checks for root memcg](https://patchwork.kernel.org/project/linux-mm/patch/20230808084431.1632934-1-alexjlzheng@tencent.com/) | 773962 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20230808084431.1632934-1-alexjlzheng@tencent.com) |
| 2023/08/17 | Kasireddy, Vivek <vivek.kasireddy@intel.com> | [udmabuf: Add back support for mapping hugetlb pages (v3)](https://patchwork.kernel.org/project/linux-mm/cover/20230817064623.3424348-1-vivek.kasireddy@intel.com/) | 776879 | v3 ☐☑ | [LORE v3,0/2](https://lore.kernel.org/r/20230817064623.3424348-1-vivek.kasireddy@intel.com) |
| 2023/08/17 | Kasireddy, Vivek <vivek.kasireddy@intel.com> | [udmabuf: Add support for page migration out of movable zone or CMA](https://patchwork.kernel.org/project/linux-mm/cover/20230817064934.3424431-1-vivek.kasireddy@intel.com/) | 776880 | v1 ☐☑ | [LORE v1,0/3](https://lore.kernel.org/r/20230817064934.3424431-1-vivek.kasireddy@intel.com) |

| 2023/04/18 | Johannes Weiner <hannes@cmpxchg.org> | [mm: reliable huge page allocator](https://patchwork.kernel.org/project/linux-mm/cover/20230418191313.268131-1-hannes@cmpxchg.org/) | 741091 | v1 ☐☑ | [LORE v1,0/26](https://lore.kernel.org/r/20230418191313.268131-1-hannes@cmpxchg.org) |
| 2023/08/21 | Johannes Weiner <hannes@cmpxchg.org> | [mm: page_alloc: freelist migratetype hygiene](https://patchwork.kernel.org/project/linux-mm/cover/20230821183733.106619-1-hannes@cmpxchg.org/) | 777975 | v1 ☐☑ | [LORE v1,0/8](https://lore.kernel.org/r/20230821183733.106619-1-hannes@cmpxchg.org) |
| 2023/08/21 | Mateusz Guzik <mjguzik@gmail.com> | [execve scalability issues, part 1](https://patchwork.kernel.org/project/linux-mm/cover/20221202829.2163744-1-mjguzik@gmail.com/) | 778000 | v1 ☐☑ | [LORE v1,0/2](https://lore.kernel.org/r/20230821202829.2163744-1-mjguzik@gmail.com)
| 2023/08/21 | Yosry Ahmed <yosryahmed@google.com> | [memcg: non-unified flushing for userspace stats](https://patchwork.kernel.org/project/li-mm/cover/20230821205458.1764662-1-yosryahmed@google.com/) | 778004 | v1 ☐☑ | [LORE v1,0/3](https://lore.kernel.org/r/20230821205458.1764662-1sryahmed@google.com) |





[Why glibc's fstat() is slow](https://lwn.net/Articles/944214)

[Linux 6.7 To Make It Easier To Toggle Support For x86 32-bit Programs](https://www.phoronix.com/news/Linux-6.7-ia32_emulation-Boot)
[Red Hat Working On Delayed Module Signature Verification To Speed-Up Linux Boot Times](https://www.phoronix.com/news/Linux-Delay-Module-Verification)
[Linux's SLUB Allocator Preparing To Better Fend Off Cross-Cache Attacks](https://www.phoronix.com/news/Linux-SLUB-Prevent-Cross-Cache)


[Intel oneAPI Initiative Evolves Into The Unified Acceleration"UXL"Foundation](https://www.phoronix.com/review/oneapi-uxl-foundation)
[](https://www.phoronix.com/news/Linux-6.6-x86-microcode)




[Moving physical pages from user space](https://lwn.net/Articles/944115)
[Revisiting the kernel's preemption models (part 1)](https://lwn.net/Articles/944686)
[User-space spinlocks with help from rseq()](https://lwn.net/Articles/944895)
[Google Proposes New mseal() Memory Sealing Syscall For Linux](https://www.phoronix.com/news/Linux-mseal-Memory-Sealing)
[GCC features to help harden the kernel](https://lwn.net/Articles/946041)
[Rethinking multi-grain timestamps](https://lwn.net/Articles/946394)
[mseal() and what comes after](https://lwn.net/Articles/948129)
[Finer-grained BPF tokens](https://lwn.net/Articles/947173)


[Red-black trees for BPF programs](https://lwn.net/Articles/924128)





在目前的情况下, 我们应该看到从 perf-script、perf-schedule-timehist 和 tp_printk 中报告的调度任务状态的三种不同结果. tracepoint sched_switch 的. 不难看出前两个是建立在第三个的基础上的, 这也是我们看到这个的原因. 不一致性在于前两种方法不能跟上随着内核发展而报告的任务状态定义的内部变化. 在 tracepoint sched_switch 中导出任务状态的内部表示并不是一个好的做法, 而且根本不鼓励这样做容易破坏依赖于它的用户空间工具. 特别是当跟踪点由于其稳定性而被大量应用于许多可观测性工具中时性质, 这使得它们不再仅用于调试目的, 我们应该小心地决定应该向用户空间报告什么, 以及什么不应该.

因此, 要彻底解决上面提到的问题, 而不是选择与用户空间跟踪工具同步



Skip idle CPU search on busy system
Skip cpus with no sched domain attach
https://lore.kernel.org/all/20240220225622.2626569-1-qyousef@layalina.io/

[A Fix Is On The Way For A Previously-Reported Linux 5.18 Performance Regression](https://www.phoronix.com/news/Linux-5.18-NUMA-Regression-Fix)

Rotary Interactivity Favor Scheduler Version 3(Brain-Eating)  Update.
https://lore.kernel.org/all/CANQmPXgdaiSUV89Vk_xR3dRAyKeGK6x=CJwaSLTC_gv_ocNP1A@mail.gmail.com/
https://lore.kernel.org/all/20100911173732.551632040@efficios.com/
https://lore.kernel.org/all/20231116105425.84773-1-s921975628@gmail.com/
https://lore.kernel.org/all/20231215052652.917741-1-imran.f.khan@oracle.com/
https://lore.kernel.org/all/20240115105052.398761-1-khorenko@virtuozzo.com/
https://lore.kernel.org/all/20231215130501.24542-1-anna-maria@linutronix.de/
cba6167f0adb



| 2023/09/29 | Kees Cook <keescook@chromium.org> | [`chelsio: Annotate structs with __counted_by`](https://lore.kernel.org/all/20230929181042.work.990-kees@kernel.org) | TODO | v1 ☐☑✓ | [LORE v1,0/5](https://lore.kernel.org/all/20230929181042.work.990-kees@kernel.org) |
| 2023/10/25 | Steven Rostedt <rostedt@goodmis.org> | [sched: Extended Scheduler Time Slice](https://lore.kernel.org/all/20231025235413.597287e1@gandalf.local.home) | TODO | v2 ☐☑✓ | [LORE v1](https://lore.kernel.org/all/20231025054219.1acaa3dd@gandalf.local.home)[LORE v2](https://lore.kernel.org/all/20231025235413.597287e1@gandalf.local.home) |
| 2023/10/31 | Keisuke Nishimura <keisuke.nishimura@inria.fr> | [sched/fair: Fix the decision for load balance](https://lore.kernel.org/all/20231031133821.1570861-1-keisuke.nishimura@inria.fr) | TODO | v3 ☐☑✓ | [LORE](https://lore.kernel.org/all/20231031133821.1570861-1-keisuke.nishimura@inria.fr) |
| 2024/04/11 | Kyle Huey <me@kylehuey.com> | [Combine perf and bpf for fast eval of hw breakpoint conditions](https://lore.kernel.org/all/20240412015019.7060-1-khuey@kylehuey.com) | TODO | v6 ☐☑✓ | [LORE v6,0/7](https://lore.kernel.org/all/20240412015019.7060-1-khuey@kylehuey.com) |
| 2024/04/07 | Ingo Molnar <mingo@kernel.org> | [sched: Split out kernel/sched/fair_balance.c, numa_balancing.c and syscalls.c, plus other updates](https://lore.kernel.org/all/20240407084319.1462211-1-mingo@kernel.org) | TODO | v1 ☐☑✓ | [LORE v1,0/5](https://lore.kernel.org/all/20240407084319.1462211-1-mingo@kernel.org) |

| 2024/01/14 | Vincent Guittot <vincent.guittot@linaro.org> | [sched/fair: Fix frequency selection for non invariant case](https://lore.kernel.org/all/20240114183600.135316-1-vincent.guittot@linaro.org) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20240114183600.135316-1-vincent.guittot@linaro.org) |
| 2024/03/15 | Swapnil Sapkal <swapnil.sapkal@amd.com> | [Fix lb_imbalance reporting in /proc/schedstat](https://lore.kernel.org/all/20240315135501.1778620-1-swapnil.sapkal@amd.com) | TODO | v2 ☐☑✓ | [LORE v2,0/1](https://lore.kernel.org/all/20240315135501.1778620-1-swapnil.sapkal@amd.com) |
| 2024/05/03 | Andrii Nakryiko <andrii@kernel.org> | [ioctl()-based API to query VMAs from /proc/<pid>/maps](https://lore.kernel.org/all/20240504003006.3303334-1-andrii@kernel.org) | TODO | v1 ☐☑✓ | [LORE v1,0/5](https://lore.kernel.org/all/20240504003006.3303334-1-andrii@kernel.org) |
| 2024/02/25 | Frederic Weisbecker <frederic@kernel.org> | [timers/nohz cleanups and hotplug reorganization](https://lore.kernel.org/all/20240225225508.11587-1-frederic@kernel.org) | TODO | v3 ☐☑✓ | [LORE v3,0/16](https://lore.kernel.org/all/20240225225508.11587-1-frederic@kernel.org) |
| 2024/02/14 | David Hildenbrand <david@redhat.com> | [mm/memory: optimize unmap/zap with PTE-mapped THP](https://lore.kernel.org/all/20240214204435.167852-1-david@redhat.com) | TODO | v3 ☐☑✓ | [LORE v3,0/10](https://lore.kernel.org/all/20240214204435.167852-1-david@redhat.com) |
| 2024/03/29 | weilin.wang@intel.com <weilin.wang@intel.com> | [TPEBS counting mode support](https://lore.kernel.org/all/20240329191224.1046866-1-weilin.wang@intel.com) | TODO | v6 ☐☑✓ | [LORE v6,0/5](https://lore.kernel.org/all/20240329191224.1046866-1-weilin.wang@intel.com) |
| 2024/04/30 | Vitalii Bursov <vitaly@bursov.com> | [sched/fair: allow disabling sched_balance_newidle with sched_relax_domain_level](https://lore.kernel.org/all/cover.1714488502.git.vitaly@bursov.com) | TODO | v4 ☐☑✓ | [LORE v4,0/3](https://lore.kernel.org/all/cover.1714488502.git.vitaly@bursov.com) |
| 2024/02/29 | Thomas Gleixner <tglx@linutronix.de> | [x86/idle: Cure RCU violations and cleanups](https://lore.kernel.org/all/20240229141407.283316443@linutronix.de) | TODO | v1 ☐☑✓ | [LORE v1,0/6](https://lore.kernel.org/all/20240229141407.283316443@linutronix.de) |
| 2023/12/08 | Qais Yousef <qyousef@layalina.io> | [sched: cpufreq: Remove uclamp max-aggregation](https://lore.kernel.org/all/20231208015242.385103-1-qyousef@layalina.io) | TODO | v1 ☐☑✓ | [LORE v1,0/4](https://lore.kernel.org/all/20231208015242.385103-1-qyousef@layalina.io) |
| 2024/03/25 | Shrikanth Hegde <sshegde@linux.ibm.com> | [sched/fair: Simplify continue_balancing for newidle](https://lore.kernel.org/all/20240325153926.274284-1-sshegde@linux.ibm.com) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20240325153926.274284-1-sshegde@linux.ibm.com) |
| 2021/11/12 | Vincent Guittot <vincent.guittot@linaro.org> | [avoid spurious blocked load update](https://lore.kernel.org/all/20211112095857.7016-1-vincent.guittot@linaro.org) | TODO | v1 ☐☑✓ | [LORE v1,0/2](https://lore.kernel.org/all/20211112095857.7016-1-vincent.guittot@linaro.org) |
| 2024/03/27 | Bharata B Rao <bharata@amd.com> | [Hot page promotion optimization for large address space](https://lore.kernel.org/all/20240327160237.2355-1-bharata@amd.com) | TODO | v1 ☐☑✓ | [LORE v1,0/2](https://lore.kernel.org/all/20240327160237.2355-1-bharata@amd.com) |
| 2024/05/06 | Qais Yousef <qyousef@layalina.io> | [sched: Consolidate cpufreq updates](https://lore.kernel.org/all/20240505233103.168766-1-qyousef@layalina.io) | TODO | v2 ☐☑✓ | [LORE](https://lore.kernel.org/all/20240505233103.168766-1-qyousef@layalina.io)<br>*-*-*-*-*-*-*-* <br>[LORE v8](https://lore.kernel.org/all/20250209235204.110989-1-qyousef@layalina.io) |
| 2024/03/28 | mingyang.cui <mingyang.cui@horizon.ai> | [sched/fair: Fix forked task check in vruntime_normalized](https://lore.kernel.org/all/20240328062757.29803-1-mingyang.cui@horizon.ai) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20240328062757.29803-1-mingyang.cui@horizon.ai) |
| 2024/04/02 | Tio Zhang <tiozhang@didiglobal.com> | [sched: make softirq cputime accounting separately in irqtime](https://lore.kernel.org/all/20240402112415.GA17946@didi-ThinkCentre-M930t-N000) | 我们现在在延迟统计中只能获得 IRQ/SOFTIRQ 的总延迟, 但单独获得 SOFTIRQ 延迟和 IRQ 延迟将有助于用户以更方便的方式减少此类延迟. 对于 IRQ 延迟, 我们可以调整 IRQ-CPU 相关性或使用线程 IRQ. 对于 SOFTIRQ 延迟, 我们可以调整 rps/xps 或使用 NAPI 的内核线程. 因此, 这个补丁集试图使 SOFTIRQ 延迟在延迟统计中可观察到, 并在 taskstats 中可用. 补丁集同步更新了 `tools/accounting/getdelays.c` 同样为了向后兼容性, 我们不想改变原始 IRQ/SOFTIRQ 延迟的含义, 相反, 我们可以通过原始 IRQ/OFTIRQ 的延迟减去该补丁添加的 SOFTIREQ 延迟来获得真实的 IRQ(中断) 延迟. | v1 ☐☑✓ | [LORE v1,0/3](https://lore.kernel.org/all/20240402112415.GA17946@didi-ThinkCentre-M930t-N000) |
| 2024/03/29 | Chunxin Zang <spring.cxz@gmail.com> | [sched/fair: Reset vlag in dequeue when PLAGE_LAG is disabled](https://lore.kernel.org/all/20240329091933.340739-1-spring.cxz@gmail.com) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20240329091933.340739-1-spring.cxz@gmail.com) |
| 2024/04/03 | Pierre Gondois <pierre.gondois@arm.com> | [sched/fair|isolation: Correctly clear nohz.[nr_cpus|idle_cpus_mask] for isolated CPUs](https://lore.kernel.org/all/20240403150543.2793354-1-pierre.gondois@arm.com) | TODO | v1 ☐☑✓ | [LORE v1,0/7](https://lore.kernel.org/all/20240403150543.2793354-1-pierre.gondois@arm.com) |

[Dropping the page cache for filesystems](https://lwn.net/Articles/977486)


[udev-hid-bpf To Help Enable HID-BPF Use Rather Than Kernel Drivers To Fix HID Hardware](https://www.phoronix.com/news/udev-hid-bpf)
[Hutterer: udev-hid-bpf: quickstart tooling to fix your HID devices with eBPF](https://lwn.net/Articles/970702)
[Cleaning up after BPF exceptions](https://lwn.net/Articles/969185)
[Better linked-list traversal in BPF](https://lwn.net/Articles/964381)
[A proposal for shared memory in BPF programs](https://lwn.net/Articles/961941)
[Sleepable BPF programs](https://lwn.net/Articles/825415)
[A plan to make BPF kfuncs polymorphic](https://lwn.net/Articles/974102)
[Recent improvements to BPF's struct_ops mechanism](https://lwn.net/Articles/974848)
[An instruction-level BPF memory model](https://lwn.net/Articles/976071/)
[Comparing BPF performance between implementations](https://lwn.net/Articles/976317)
[Securing BPF programs before and after verification](https://lwn.net/Articles/977394)
[Simplifying the BPF verifier](https://lwn.net/Articles/977815)
[Nested bottom-half locking for realtime kernels](https://lwn.net/Articles/978189)
[Static keys for BPF](https://lwn.net/Articles/977993)
[BPF tracing performance](https://lwn.net/Articles/978335)
[Capturing stack traces asynchronously with BPF](https://lwn.net/Articles/978736)
[Updates to pahole](https://lwn.net/Articles/978727)
[Eliminating indirect calls for security modules](https://lwn.net/Articles/979683)
[A capability set for user namespaces](https://lwn.net/Articles/978846)
[A look inside the BPF verifier](https://lwn.net/Articles/982077)
[CRIB: checkpoint/restore in BPF](https://lwn.net/Articles/984313)
[Modernizing BPF for the next 10 years](https://lwn.net/Articles/977013/)
[HIVE: A Hardware-assisted Isolated Execution Environment for eBPF on AArch64](https://www.usenix.org/conference/usenixsecurity24/presentation/zhang-peihua)

[Direct-to-device networking](https://lwn.net/Articles/979549)


[FetchBPF: Customizable Prefetching Policies in Linux with eBPF](https://www.usenix.org/conference/atc24/presentation/cao)
[DINT: Fast In-Kernel Distributed Transactions with eBPF](https://www.usenix.org/conference/nsdi24/presentation/zhou-yang)
[Fast (Trapless) Kernel Probes Everywhere](https://atcosdi24.sched.com/event/1fLcd/fast-trapless-kernel-probes-everywhere), https://www.usenix.org/conference/atc24/presentation/jia


[Identifying On-/Off-CPU Bottlenecks Together with Blocked Samples](https://www.usenix.org/conference/osdi24/presentation/ahn)
[wPerf: Generic Off-CPU Analysis to Identify Bottleneck Waiting Events](https://www.usenix.org/conference/osdi18/presentation/zhou)

[Harvesting Memory-bound CPU Stall Cycles in Software with MSH](https://www.usenix.org/conference/osdi24/presentation/luo)

[OS2ATC-PPT](https://cloud.tsinghua.edu.cn/d/8e6c77ad1ad548acb909/)

[Wattch: a framework for architectural-level power analysis and optimizations](https://ieeexplore.ieee.org/document/854380)


[Dynamically sizing the kernel stack](https://lwn.net/Articles/974367)


[Intel Talks Up 2024 Xeon Sierra Forest & Granite Rapids At Hot Chips](https://www.phoronix.com/review/intel-hotchips-2023)


[Memory Sealing"mseal"System Call Merged For Linux 6.10](https://www.phoronix.com/news/Linux-6.10-Merges-mseal)


| 2024/05/27 | Tero Kristo <tero.kristo@linux.intel.com> | [Intel Performance Limit Reasons support](https://lore.kernel.org/all/20240527133400.483634-1-tero.kristo@linux.intel.com) | 英特尔性能限制原因可以报告为什么性能 / 功耗在芯片级别或芯片内的单个 CPU 内核受到限制. 今天, 随着 Linux 内核邮件列表的一组新补丁的发布, 英特尔 PLR 硬件性能事件报告正在连接中. 这些信息通过 DebugFS 传达到用户空间, 供管理员直接访问, 或者供其他应用程序了解为什么他们的 CPU 性能可能受到限制. 对于英特尔性能限制原因提出的 Linux 补丁, 信息通过 `/sys/kernel/debug/tpmi-*/plr/domain*/status` 上报. 但是, 与 DebugFS 一样, `/sys/kernel/debug/` 的访问通常需要 ROOT 权限. 该组补丁提供了一个 INTEL_PLR_TPMI 的 Kconfig 选项, 用于控制此英特尔 PLR 驱动程序. 参见 phoronix 报道 [Intel Performance Limit Reasons For Linux To Report Why Your CPU Is Downclocking](https://www.phoronix.com/news/Intel-Perf-Limit-Reasons-Linux) | v2 ☐☑✓ | [LORE v2,0/6](https://lore.kernel.org/all/20240527133400.483634-1-tero.kristo@linux.intel.com) |
| 2024/02/26 | Lai Jiangshan <jiangshanlai@gmail.com> | [KVM: x86/PVM: Introduce a new hypervisor](https://lore.kernel.org/all/20240226143630.33643-1-jiangshanlai@gmail.com) | TODO | v1 ☐☑✓ | [LORE v1,0/73](https://lore.kernel.org/all/20240226143630.33643-1-jiangshanlai@gmail.com) |
| 2020/05/04 | Alexandre Chartre <alexandre.chartre@oracle.com> | [ASI - Part I (ASI Infrastructure and PTI)](https://lore.kernel.org/all/20200504144939.11318-1-alexandre.chartre@oracle.com) | TODO | v4 ☐☑✓ | [LORE v4,0/7](https://lore.kernel.org/all/20200504144939.11318-1-alexandre.chartre@oracle.com) |
| 2024/02/14 | Ard Biesheuvel <ardb+git@google.com> | [arm64: Add support for LPA2 and WXN at stage 1](https://lore.kernel.org/all/20240214122845.2033971-45-ardb+git@google.com) | TODO | v8 ☐☑✓ | [LORE v7,00/50](https://lore.kernel.org/all/20240123145258.1462979-52-ardb+git@google.com)<br>*-*-*-*-*-*-*-*<br>[LORE v8,0/43](https://lore.kernel.org/all/20240214122845.2033971-45-ardb+git@google.com) |
| 2024/07/02 | Huisong Li <lihuisong@huawei.com> | [power: introduce PM QoS interface](https://lore.kernel.org/all/20240702035010.4874-1-lihuisong@huawei.com) | TODO | v5 ☐☑✓ | [LORE v5,0/2](https://lore.kernel.org/all/20240702035010.4874-1-lihuisong@huawei.com) |







[WeWe-RSS](https://help.railway.app/templates/we-we-rss-d75eade8)
[免费部署 wewe-rss 服务](https://stupidjoey.net/posts/deploy_wewe_rss/)
[一键部署属于自己的微信公众号 RSS(wewe-rss)](https://foxistudy.com/posts/wewe-rss/)
[鄙人使用 huggingface 和 railway 搭建的公众号订阅](https://huggingface.co/spaces/gatieme/wewe-rss)


[松鼠快看 - 公众号订阅服务](https://songshukuaikan.com/login)











[eas_overview_and_integration_guide_r1p6.pdf](https://developer.arm.com/-/media/Arm%20Developer%20Community/PDF/Open%20Source/energy-aware-scheduling/eas_overview_and_integration_guide_r1p6.pdf)





[能量感知调度 ¶](https://www.kernel.org/doc/html/latest/translations/zh_CN/scheduler/sched-energy.html)



[Amazon Exploring MM-Local Memory Allocations To Help With Current/Future Speculation Attacks](https://www.phoronix.com/news/Linux-MM-Local-Kernel-Allocs)


update_curr_rt
-=> trace_android_vh_sched_stat_runtime_rt(curr, delta_exec);
	-=> register_trace_android_vh_sched_stat_runtime_rt(fbg_update_rt_util_hook, NULL);
		-=> fbg_update_task_util(tsk, runtime, true);
			-=> update_frame_group_util(tsk, runtime, wallclock, default_part, grp);
				-=> sf_composition_update_cpufreq(tsk);		// sf_composition_group
				-=> default_group_update_cpufreq();		// game_frame_boost_group
				-=> inputmethod_update_cpufreq(tsk);		// default_frame_boost_group



update_curr_{fair|rt}
update_util_before_window_rollover
fbg_android_rvh_cpufreq_transition //REGISTER_TRACE_RVH(trace_android_rvh_cpufreq_transition, cpufreq_transition_handler);

-=> fbg_update_task_util




[AMD's AOMP 19.0-2 Compiler Brings Zero-Copy For CPU-GPU Unified Shared Memory](https://www.phoronix.com/news/AMD-AOMP-19.0-2-Compiler)


HUAWEI P10 Plus, Vicky, Android 7.0, EMUI 5.1

| Model | Description | Open Source |
|:-----:|:-----------:|:-----------:|
| Vicky | HUAWEI P10 Plus, Vicky, Android 7.0, EMUI 5.1 | [LINK](https://download-c1.huawei.com/download/downloadCenter?downloadId=4217C26E864DD3F2F7EF57943CCF4179&version=B7B24D5019E86B029000B1F8D06787F5&siteCode=worldwide) |




[New Power Sequencing Driver Subsystem Merged For Linux 6.11](https://www.phoronix.com/news/Linux-6.11-Power-Sequencing)







[Ghidra Software Reverse Engineering Framework](https://github.com/NationalSecurityAgency/ghidra)



| 2024/08/26 | NeilBrown <neilb@suse.de> | [block: change wait on bd_claiming to use a var_waitqueue, not a bit_waitqueue](https://lore.kernel.org/all/20240826063659.15327-2-neilb@suse.de) | TODO | v1 ☐☑✓ | [LORE v1,0/7](https://lore.kernel.org/all/20240826063659.15327-2-neilb@suse.de) |



https://lore.kernel.org/all/20240830130309.2141697-1-vincent.guittot@linaro.org/





| 编号 | 调度器 | 语言 | 用途 |
|:---:|:-----:|:----:|:---:|
|  1  | scx_lavd | 程序专注于交互性, 具体来说, 就是始终如一地从游戏中获得更高的帧速率. |
|  2  | scx_rustland | RUST | 将调度事件转发到用户空间, 在那里做出决策. 还包含 scx_rusty 用于复杂 CPU 拓扑上的负载平衡, 以及, 一个 scx_layered 分区计划程序. |
|  3  |


| 2024/05/08 | Josh Don <joshdon@google.com> | [sched/fair: prevent unbounded task iteration in load balance](https://lore.kernel.org/all/20240508223456.4189689-1-joshdon@google.com) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20240508223456.4189689-1-joshdon@google.com) |
| 2024/06/18 | Fernand Sieber <sieberf@amazon.com> | [perf: Timehist account sch delay for scheduled out running](https://lore.kernel.org/all/20240618090339.87482-1-sieberf@amazon.com) | TODO | v2 ☐☑✓ | [LORE v1](https://lore.kernel.org/all/20240613185906.31082-1-sieberf@amazon.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2](https://lore.kernel.org/all/20240618090339.87482-1-sieberf@amazon.com) |
| 2024/03/22 | Raghavendra K T <raghavendra.kt@amd.com> | [A Summary of VMA scanning improvements explored](https://lore.kernel.org/all/cover.1710829750.git.raghavendra.kt@amd.com) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/cover.1710829750.git.raghavendra.kt@amd.com) |
| 2024/07/17 | Xavier <xavier_qy@163.com> | [Optimize the RT group scheduling](https://lore.kernel.org/all/20240717030033.309205-1-xavier_qy@163.com) | TODO | v4 ☐☑✓ | [LORE v4,0/2](https://lore.kernel.org/all/20240717030033.309205-1-xavier_qy@163.com) |
| 2024/06/25 | Chen Yu <yu.c.chen@intel.com> | [sched/fair: Enhance sync wakeup for short duration tasks](https://lore.kernel.org/all/cover.1719295669.git.yu.c.chen@intel.com) | TODO | v1 ☐☑✓ | [LORE v1,0/2](https://lore.kernel.org/all/cover.1719295669.git.yu.c.chen@intel.com) |
| 2024/05/16 | Costa Shulyupin <costa.shul@redhat.com> | [sched: Adjust affinity according to change of housekeeping cpumask](https://lore.kernel.org/all/20240516190437.3545310-1-costa.shul@redhat.com) | TODO | v1 ☐☑✓ | [LORE v1,0/7](https://lore.kernel.org/all/20240516190437.3545310-1-costa.shul@redhat.com) |
| 2024/08/09 | K Prateek Nayak <kprateek.nayak@amd.com> | [sched/core: Introduce SM_IDLE and an idle re-entry fast-path in __schedule()](https://lore.kernel.org/all/20240809092240.6921-1-kprateek.nayak@amd.com) | TODO | v2 ☐☑✓ | [LORE](https://lore.kernel.org/all/20240809092240.6921-1-kprateek.nayak@amd.com) |
| 2024/09/04 | K Prateek Nayak <kprateek.nayak@amd.com> | [Idle Load Balance fixes and softirq enhancements](https://lore.kernel.org/all/20240904111223.1035-1-kprateek.nayak@amd.com) | TODO | v2 ☐☑✓ | [LORE v2,0/5](https://lore.kernel.org/all/20240904111223.1035-1-kprateek.nayak@amd.com) |
| 2024/09/25 | NeilBrown <neilb@suse.de> | [`sched: change wake_up_bit() and related function to expect unsigned long *`](https://lore.kernel.org/all/20240925053405.3960701-2-neilb@suse.de) | TODO | v1 ☐☑✓ | [LORE v1,0/7](https://lore.kernel.org/all/20240925053405.3960701-2-neilb@suse.de)|
| 2024/10/10 | Steve Wahl <steve.wahl@hpe.com> | [sched/topology: improve topology_span_sane speed](https://lore.kernel.org/all/20241010155111.230674-1-steve.wahl@hpe.com) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20241010155111.230674-1-steve.wahl@hpe.com) |
| 2024/11/09 | Changwoo Min <multics69@gmail.com> | [sched_ext: Clarify sched_ext_ops table for userland scheduler](https://lore.kernel.org/all/20241109022514.173631-1-changwoo@igalia.com) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20241109022514.173631-1-changwoo@igalia.com) |
| 2024/11/09 | Tejun Heo <tj@kernel.org> | [sched_ext: Rename dispatch and consume kfuncs](https://lore.kernel.org/all/20241109194853.580310-1-tj@kernel.org) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20241109194853.580310-1-tj@kernel.org) |
| 2024/11/19 | K Prateek Nayak <kprateek.nayak@amd.com> | [sched/fair: Idle load balancer fixes for fallouts from IPI optimization to TIF_POLLING CPUs](https://lore.kernel.org/all/20241119054432.6405-1-kprateek.nayak@amd.com) | TODO | v5 ☐☑✓ | [LORE v5,0/4](https://lore.kernel.org/all/20241119054432.6405-1-kprateek.nayak@amd.com) |
| 2024/11/29 | Rafael J. Wysocki <rjw@rjwysocki.net> | [cpufreq: intel_pstate: Enable EAS on hybrid platforms without SMT](https://lore.kernel.org/all/5861970.DvuYhMxLoT@rjwysocki.net) | TODO | v21 ☐☑✓ | [LORE v21,0/9](https://lore.kernel.org/all/5861970.DvuYhMxLoT@rjwysocki.net) |
| 2025/01/09 | Changwoo Min <changwoo@igalia.com> | [sched_ext: Support high-performance monotonically non-decreasing clock](https://lore.kernel.org/all/20250109131456.7055-1-changwoo@igalia.com) | TODO | v8 ☐☑✓ | [LORE v8,0/6](https://lore.kernel.org/all/20250109131456.7055-1-changwoo@igalia.com) |
| 2024/12/02 | Vincent Guittot <vincent.guittot@linaro.org> | [sched/fair: Fix statistics with delayed dequeue](https://lore.kernel.org/all/20241202174606.4074512-1-vincent.guittot@linaro.org) | TODO | v3 ☐☑✓ | [LORE v3,0/11](https://lore.kernel.org/all/20241202174606.4074512-1-vincent.guittot@linaro.org) |
| 2024/12/12 | Vineeth Pillai (Google) <vineeth@bitbyteword.org> | [sched/dlserver: flag to represent active status of dlserver](https://lore.kernel.org/all/20241213032244.877029-1-vineeth@bitbyteword.org) | TODO | v1 ☐☑✓ | [LORE v1,0/2](https://lore.kernel.org/all/20241213032244.877029-1-vineeth@bitbyteword.org) |
| 2024/12/20 | Swapnil Sapkal <swapnil.sapkal@amd.com> | [Fixes and improvements in /proc/schedstat](https://lore.kernel.org/all/20241220063224.17767-1-swapnil.sapkal@amd.com) | TODO | v2 ☐☑✓ | [LORE v2,0/6](https://lore.kernel.org/all/20241220063224.17767-1-swapnil.sapkal@amd.com) |
| 2025/01/13 | Chuyi Zhou <zhouchuyi@bytedance.com> | [Take the scheduling domain into account in numa balancin](https://lore.kernel.org/all/20250113073050.2811925-1-zhouchuyi@bytedance.com) | TODO | v3 ☐☑✓ | [LORE v3,0/3](https://lore.kernel.org/all/20250113073050.2811925-1-zhouchuyi@bytedance.com) |
| 2025/01/06 | wujing <realwujing@qq.com> | [sched/fair: Correct CPU selection from isolated domain](https://lore.kernel.org/all/tencent_160A5B6C838FD9A915A67E67914350EB1806@qq.com) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/tencent_160A5B6C838FD9A915A67E67914350EB1806@qq.com)|
| 2025/01/04 | Andrea Righi <arighi@nvidia.com> | [sched_ext: idle: small CPU iteration refactoring](https://lore.kernel.org/all/20250104090009.331193-1-arighi@nvidia.com) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20250104090009.331193-1-arighi@nvidia.com) |
| 2025/01/08 | Honglei Wang <jameshongleiwang@126.com> | [sched_ext: switch class when preempted by higher priority scheduler](https://lore.kernel.org/all/20250108023328.37675-1-jameshongleiwang@126.com) | TODO | v2 ☐☑✓ | [LORE](https://lore.kernel.org/all/20250108023328.37675-1-jameshongleiwang@126.com) |
| 2024/12/16 | Michal Koutný <mkoutny@suse.com> | [Add kernel cmdline option for rt_group_sched](https://lore.kernel.org/all/20241216201305.19761-1-mkoutny@suse.com) | TODO | v1 ☐☑✓ | [LORE v1,0/9](https://lore.kernel.org/all/20241216201305.19761-1-mkoutny@suse.com)<br>*-*-*-*-*-*-*-* <br>[LORE v1,0/9](https://lore.kernel.org/all/20250210151239.50055-1-mkoutny@suse.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,00/10](https://lore.kernel.org/all/20250310170442.504716-1-mkoutny@suse.com/) |
| 2025/01/14 | Florian Schmaus <flo@geekplace.eu> | [sched: provide sched_set_batch()](https://lore.kernel.org/all/20250114130513.498482-3-flo@geekplace.eu) | TODO | v1 ☐☑✓ | [LORE v1,0/2](https://lore.kernel.org/all/20250114130513.498482-3-flo@geekplace.eu) |
| 2024/12/04 | Tobias Huschle <huschle@linux.ibm.com> | [sched/fair: introduce new scheduler group type group_parked](https://lore.kernel.org/all/20241204112149.25872-1-huschle@linux.ibm.com) | TODO | v1 ☐☑✓ | [LORE v1,0/2](https://lore.kernel.org/all/20241204112149.25872-1-huschle@linux.ibm.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/3](https://lore.kernel.org/all/20250217113252.21796-1-huschle@linux.ibm.com) |
| 2025/01/13 | I Hsin Cheng <richard120310@gmail.com> | [sched/fair: Refactor can_migrate_task() to elimate looping](https://lore.kernel.org/all/20250113041249.6847-1-richard120310@gmail.com) | TODO | v2 ☐☑✓ | [LORE](https://lore.kernel.org/all/20250113041249.6847-1-richard120310@gmail.com) |
| 2025/01/16 | Phil Auld <pauld@redhat.com> | [sched: Mention autogroup disabled behavior](https://lore.kernel.org/all/20250116124654.2365691-1-pauld@redhat.com) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20250116124654.2365691-1-pauld@redhat.com) |
| 2024/11/13 | Juri Lelli <juri.lelli@redhat.com> | [Fix DEADLINE bandwidth accounting in root domain changes and hotplug](https://lore.kernel.org/all/20241113125724.450249-1-juri.lelli@redhat.com) | TODO | v1 ☐☑✓ | [LORE v1,0/2](https://lore.kernel.org/all/20241113125724.450249-1-juri.lelli@redhat.com) |
| 2024/12/23 | Hao Jia <jiahao.kernel@gmail.com> | [sched/core: Prioritize migrating eligible tasks in sched_balance_rq()](https://lore.kernel.org/all/20241223091446.90208-1-jiahao.kernel@gmail.com) | TODO | v2 ☐☑✓ | [LORE](https://lore.kernel.org/all/20241223091446.90208-1-jiahao.kernel@gmail.com) |
| 2025/01/26 | Changwoo Min <changwoo@igalia.com> | [sched_ext: Implement core event counters](https://lore.kernel.org/all/20250126101614.232388-1-changwoo@igalia.com) | TODO | v2 ☐☑✓ | [LORE v2,0/11](https://lore.kernel.org/all/20250126101614.232388-1-changwoo@igalia.com) |
| 2025/01/25 | Andrea Righi <arighi@nvidia.com> | [sched_ext: Move built-in idle CPU selection policy to a separate file](https://lore.kernel.org/all/20250125213911.283318-1-arighi@nvidia.com) | TODO | v2 ☐☑✓ | [LORE](https://lore.kernel.org/all/20250125213911.283318-1-arighi@nvidia.com) |
| 2024/12/19 | Pierre Gondois <pierre.gondois@arm.com> | [sched/fair: Decrease util_est in presence of idle time](https://lore.kernel.org/all/20241219091207.2001051-1-pierre.gondois@arm.com) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20241219091207.2001051-1-pierre.gondois@arm.com) |
| 2025/02/04 | Changwoo Min <changwoo@igalia.com> | [sched_ext: Implement core event counters](https://lore.kernel.org/all/20250204052057.67776-1-changwoo@igalia.com) | TODO | v4 ☐☑✓ | [LORE v4,0/7](https://lore.kernel.org/all/20250204052057.67776-1-changwoo@igalia.com) |
| 2025/01/29 | Christian Loehle <christian.loehle@arm.com> | [sched/debug: Print slice length for fair tasks](https://lore.kernel.org/all/453349b1-1637-42f5-a7b2-2385392b5956@arm.com) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/453349b1-1637-42f5-a7b2-2385392b5956@arm.com) |
| 2025/01/28 | Fernand Sieber <sieberf@amazon.com> | [sched: Add core cookie update tracepoint](https://lore.kernel.org/all/20250128113410.263994-1-sieberf@amazon.com) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20250128113410.263994-1-sieberf@amazon.com) |
| 2025/01/29 | Christian Loehle <christian.loehle@arm.com> | [sched/debug: Print slice length for fair tasks](https://lore.kernel.org/all/453349b1-1637-42f5-a7b2-2385392b5956@arm.com) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/453349b1-1637-42f5-a7b2-2385392b5956@arm.com) |
| 2025/02/07 | Changwoo Min <changwoo@igalia.com> | [sched_ext: Add a core event and update scx schedulers](https://lore.kernel.org/all/20250207031338.393045-1-changwoo@igalia.com) | TODO | v1 ☐☑✓ | [LORE v1,0/2](https://lore.kernel.org/all/20250207031338.393045-1-changwoo@igalia.com) |
| 2025/01/21 | zihan zhou <15645113830zzh@gmail.com> | [sched: Cancel the slice protection of the idle entity](https://lore.kernel.org/all/20250121030628.113497-1-15645113830zzh@gmail.com) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20250121030628.113497-1-15645113830zzh@gmail.com) |
| 2025/02/21 | Abel Wu <wuyun.abel@bytedance.com> | [Fix SCHED_IDLE behavior on wakeup preemption](https://lore.kernel.org/all/20250221111226.64455-1-wuyun.abel@bytedance.com) | TODO | v1 ☐☑✓ | [LORE v1,0/2](https://lore.kernel.org/all/20250221111226.64455-1-wuyun.abel@bytedance.com) |
| 2025/02/07 | Tejun Heo <tj@kernel.org> | [sched_ext: Event counter updates](https://lore.kernel.org/all/20250208084229.1274399-1-tj@kernel.org) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20250208084229.1274399-1-tj@kernel.org) |
| 2025/02/20 | Jemmy Wong <jemmywong512@gmail.com> | [sched: Refine scheduler naming for clarity and specificity](https://lore.kernel.org/all/20250219182020.393006-1-jemmywong512@gmail.com) | TODO | v1 ☐☑✓ | [LORE v1,0/2](https://lore.kernel.org/all/20250219182020.393006-1-jemmywong512@gmail.com) |


Event counter updates


[成大资工 - WIKI](https://wiki.csie.ncku.edu.tw/User/OscarShiang)

[Empower Embedded Computing with VexRiscv and FreeRTOS](https://eventyay.com/e/6b901f56/session/7569)

[Towards PREEMPT_RT for the Full Task Isolation](https://ossna2022.sched.com/event/11NtQ)

[sched: Clarify PREEMPT is full preemption model](https://lore.kernel.org/lkml/cb013939c11991f429773b83a6ff1513221dd678.a11e9ded.5679.4468.8cc8.fe30e9b90f3f@feishu.cn/)




https://www.phoronix.com/news/Schedutil-30p-Speedometer-Boost

[Linux 6.14 Resource Control To Allow Total Memory Bandwidth Monitoring](https://www.phoronix.com/news/Linux-6.14-resctrl-Total-RAM-BW)





成坚 	410881199005133515
王盼盼   410881199101013601
范小香	410827195708153524
成梽  	33010820200707137X

https://www.phoronix.com/news/cpufreq_ext-RFC#google_vignette







用例范围
https://xarjbochz9n.feishu.cn/wiki/I03BwQQMDi5fMSkLRc0cW5iJnAf?open_in_browser=true


整理好的所有164条测试样例。
https://xarjbochz9n.feishu.cn/wiki/WBX8wziSMiVBxFkKNVXcGjEsnqb?from=from_copylink




在 OSPM'25 上, 不少开发者建议使用推送任务机制来进行 idle balance 和 newidle balance. 借鉴了 [sched/fair: Rework EAS to handle more cases](https://lore.kernel.org/all/20240830130309.2141697-1-vincent.guittot@linaro.org) 的思路, 实现了一套统一的 CFS 任务推送框架, 并已针对 !EAS 场景进行了实现.<br>1. 该系列实现了 [Valentin 的想法](https://lore.kernel.org/lkml/xhsmh1putoxbz.mognet@vschneid-thinkpadt14sgen2i.remote.csb), 即在存在可推送任务的情况下, CPU 会将自身设置为每个 LLC 的"过载掩码(overloaded mask)".<br>2. NUMA 间的新空闲平衡机制对此做了优化, 会先遍历本地 LLC 上 overloaded mask 中的 CPU 集合, 然后遍历同一 NUMA 节点中其他 LLC 上 overloaded mask 中的 CPU 集合, 目的是将单个任务拉向自身, 而非执行全面的负载均衡.<br>3. 这实现了 [David Vernet 的 SAHRED_RUNQ 原型](https://lore.kernel.org/lkml/20231212003141.216236-1-void@manifault.com/) 中的一些想法, 不过, 与每个 LLC/每个分片使用一个单独的 SHARED_RUNQ 不同, 这里过载掩码用作指示符, 表明每个 CPU 的 rq 中包含可迁移到即将空闲的 CPU 的可推送任务. 这样做的代价是维护过载的 cpumask, 但避免了为每个 SHARED_RUNQ 设置锁.<br>4. 推送回调函数本身已进行了修改, 会尝试将可推送任务列表中的任务推送到"nohz.idle_cpus_mask"掩码中的某个 CPU 上, 从而减轻空闲平衡的负载.
