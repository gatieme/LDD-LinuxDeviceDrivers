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
| 2025/01/14 | Florian Schmaus <flo@geekplace.eu> | [sched: provide sched_set_batch()](https://lore.kernel.org/all/20250114130513.498482-3-flo@geekplace.eu) | TODO | v1 ☐☑✓ | [LORE v1,0/2](https://lore.kernel.org/all/20250114130513.498482-3-flo@geekplace.eu) |
| 2024/12/04 | Tobias Huschle <huschle@linux.ibm.com> | [sched/fair: introduce new scheduler group type group_parked](https://lore.kernel.org/all/20241204112149.25872-1-huschle@linux.ibm.com) | TODO | v1 ☐☑✓ | [LORE v1,0/2](https://lore.kernel.org/all/20241204112149.25872-1-huschle@linux.ibm.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/3](https://lore.kernel.org/all/20250217113252.21796-1-huschle@linux.ibm.com)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/4](https://lore.kernel.org/all/20250512115325.30022-1-huschle@linux.ibm.com) |
| 2025/01/13 | I Hsin Cheng <richard120310@gmail.com> | [sched/fair: Refactor can_migrate_task() to elimate looping](https://lore.kernel.org/all/20250113041249.6847-1-richard120310@gmail.com) | TODO | v2 ☐☑✓ | [LORE](https://lore.kernel.org/all/20250113041249.6847-1-richard120310@gmail.com) |
| 2025/01/16 | Phil Auld <pauld@redhat.com> | [sched: Mention autogroup disabled behavior](https://lore.kernel.org/all/20250116124654.2365691-1-pauld@redhat.com) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20250116124654.2365691-1-pauld@redhat.com) |
| 2024/11/13 | Juri Lelli <juri.lelli@redhat.com> | [Fix DEADLINE bandwidth accounting in root domain changes and hotplug](https://lore.kernel.org/all/20241113125724.450249-1-juri.lelli@redhat.com) | TODO | v1 ☐☑✓ | [LORE v1,0/2](https://lore.kernel.org/all/20241113125724.450249-1-juri.lelli@redhat.com) |
| 2024/12/23 | Hao Jia <jiahao.kernel@gmail.com> | [sched/core: Prioritize migrating eligible tasks in sched_balance_rq()](https://lore.kernel.org/all/20241223091446.90208-1-jiahao.kernel@gmail.com) | TODO | v2 ☐☑✓ | [LORE](https://lore.kernel.org/all/20241223091446.90208-1-jiahao.kernel@gmail.com) |
| 2025/01/25 | Andrea Righi <arighi@nvidia.com> | [sched_ext: Move built-in idle CPU selection policy to a separate file](https://lore.kernel.org/all/20250125213911.283318-1-arighi@nvidia.com) | TODO | v2 ☐☑✓ | [LORE](https://lore.kernel.org/all/20250125213911.283318-1-arighi@nvidia.com) |
| 2024/12/19 | Pierre Gondois <pierre.gondois@arm.com> | [sched/fair: Decrease util_est in presence of idle time](https://lore.kernel.org/all/20241219091207.2001051-1-pierre.gondois@arm.com) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20241219091207.2001051-1-pierre.gondois@arm.com) |
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

https://www.github-zh.com/
[A new type of spinlock for the BPF subsystem](https://mp.weixin.qq.com/s/6pEk4PBnf6WXNdj4zmIeBA)
[LWN: GCC BPF 支持的进展！](https://mp.weixin.qq.com/s/hlXtGhkIhIhpgPQTnolVXg)




用例范围
https://xarjbochz9n.feishu.cn/wiki/I03BwQQMDi5fMSkLRc0cW5iJnAf?open_in_browser=true


整理好的所有 164 条测试样例.
https://xarjbochz9n.feishu.cn/wiki/WBX8wziSMiVBxFkKNVXcGjEsnqb?from=from_copylink




https://www.aidenleong.com/lxr/linux-6.14.2/kernel/sched/idle.c
https://ncnz67vv5cuy.feishu.cn/wiki/IN66w8dW8imkkUkKusMcy2jLnBb


[phoronix, 2025/05/28, Linux 6.16 Will Be Able To Exit User Mode Faster: 2~11% Improvement](https://www.phoronix.com/news/Linux-616-Faster-Exit-User-Mode)


| 2025/05/28 | Ingo Molnar <mingo@kernel.org> | [sched: Use the SMP scheduler on UP too](https://lore.kernel.org/all/20250528080924.2273858-1-mingo@kernel.org) | Ingo Molnar 提出一组共 43 个补丁, 旨在将 Linux 调度器统一为 SMP(对称多处理) 版本, 从而大幅减少调度器中 `CONFIG_SMP` 相关的预处理条件编译代码. 当前 `kernel/sched/` 中有 175 处 `CONFIG_SMP` 判断, 导致代码维护困难、易出错, 并影响可读性和可审查性. 该补丁系列将这一数量减少至 28 处, 删除近 1000 行代码, 提升了代码整洁度. <br><br> 此改动使 UP(单处理器) 调度器也使用 SMP 实现, 虽引入少量冗余代码和性能开销 (如上下文切换时间增加约 5%、内核体积增长约 0. 3% 文本段), 但换来更简单的维护模型. 作者强调, 未来若需优化 UP 性能或体积, 应通过合理方式 (如 `IS_ENABLED() `) 进行, 而非回归大量 `#ifdef`. 补丁系列已初步测试验证在 UP 系统上可正常启动和运行, 目前存放于指定 Git 仓库中. 参见 [phoronix, 2025/05/28, Big Linux Patch Series Shakes Up The Scheduler Code For Anyone With Only One CPU Core](https://www.phoronix.com/news/Linux-UP-SMP-Scheduler-2025)  | v1 ☐☑✓ | [2025/05/28, LORE v1, 0/43](https://lore.kernel.org/all/20250528080924.2273858-1-mingo@kernel.org) |
| 2025/06/11 | Cheng-Yang Chou <yphbchou0911@gmail.com> | [sched_ext: Standardize preprocessor comment markers](https://lore.kernel.org/all/20250611135404.13851-1-yphbchou0911@gmail.com) | 旨在对 `sched/ext*. {c, h}` 文件中的 `#if/#else/#endif` 预处理注释标记进行标准化, 以提升代码一致性与可读性. 该工作遵循 Ingo Molnar 近期调度器清理工作的方向, 旨在减少 `CONFIG_SMP` 相关复杂性并删除冗余的 `#ifdef` 代码块. 此次修改不涉及任何功能变更. 补丁 ^ 共修改 4 个文件, 删除 60 行、新增 3 行代码.  | v1 ☐☑✓ | [2025/06/11, LORE v1, 0/4](https://lore.kernel.org/all/20250611135404.13851-1-yphbchou0911@gmail.com) |
| 2025/07/27 | Sasha Levin <sashal@kernel.org> | [Add agent coding assistant configuration to Linux kernel](https://lore.kernel.org/all/20250727195802.2222764-1-sashal@kernel.org) | 邮件提出了一项为 Linux 内核引入 "Agent 编码助手" 统一配置和文档规范的补丁集, 旨在规范 AI 编码工具在内核开发中的使用. 共包含四个补丁: 1) 为多个编码助手 (如 Claude、GitHub Copilot 等) 添加统一配置文件并实现一致性管理; 2) 引入核心开发参考文档, 引导助手遵循内核开发流程; 3) 明确编码风格规范, 如 80 字符行限制、无尾随空格等; 4) 制定法律要求与署名规范, 要求所有助手在提交中通过 Co-developed-by 标识自身参与. 补丁从 RFC 版本起已转为 RST 格式, 拆分为多个文件, 并简化了说明. 此系列为内核引入 AI 辅助开发提供了制度化基础.  | v1 ☐☑✓ | [2025/07/27, LORE v1, 0/4](https://lore.kernel.org/all/20250727195802.2222764-1-sashal@kernel.org) |








[](https://mp.weixin.qq.com/s/QZNb1IZMN8M-X3VW7ZCndQ)

[Tintin: A Unified Hardware Performance Profiling Infrastructure to Uncover and Manage Uncertainty](https://zhuanlan.zhihu.com/p/1925925001460160336)


[BayesPerf: minimizing performance monitoring errors using Bayesian statistics](https://dl.acm.org/doi/10.1145/3445814.3446739)



[HeMem: Scalable Tiered Memory Management for Big Data Applications and Real NVM](https://zhuanlan.zhihu.com/p/461022129)


[Practical guide to fuzzing the Binder kernel driver using the Linux Kernel Library (LKL)](https://androidoffsec.withgoogle.com/posts/binder-fuzzing)


[NeoMem: Hardware/Software Co-Design for CXL-Native Memory Tiering](https://arxiv.org/abs/2403.18702)

















https://support.huaweicloud.com/bestpractice-ecs/zh-cn_topic_0141067581.html


1. 开启 DOCKER

sudo docker run -it --name ubuntu22.04 ubuntu:22.04 bash
sudo docker ps -a
sudo docker start 665e1247f4c8
sudo docker exec -it ubuntu22.04 bash

2. 处理 Qwen2.5-Coder-7B-Instruct-AWQ

sudo docker cp /home/chengjian/Work/Package/Miniconda/py310 ubuntu22.04:/root/Model
sudo docker cp Qwen2.5-Coder-7B-Instruct-AWQ ubuntu22.04:/root/Model/QWEN/QWEN2.5

mkdir -p /root/Model/QWEN/QWEN2.5/Qwen2.5-Coder-7B-Instruct-AWQ-HEADER
python3 convert_awq.py /root/Model/QWEN/QWEN2.5/Qwen2.5-Coder-7B-Instruct-AWQ /root/Model/QWEN/QWEN2.5/Qwen2.5-Coder-7B-Instruct-AWQ-HEADER --group_size 128 --gen_model_header
sudo docker cp ubuntu22.04:/root/Model/QWEN/QWEN2.5/Qwen2.5-Coder-7B-Instruct-AWQ-PARAMS ./Qwen2.5-Coder-7B-Instruct-AWQ-PARAMS


mkdir -p /root/Model/QWEN/QWEN2.5/Qwen2.5-Coder-7B-Instruct-AWQ-HEADER-VERSION

3. 处理 Qwen2.5-Coder-7B-Instruct-AWQ

sudo docker cp Qwen2.5-0.5B-Instruct ubuntu22.04:/root/Model/QWEN/QWEN2.5/0.5B/Qwen2.5-0.5B-Instruct

python3 convert_llama.py ~/Model/QWEN/QWEN2.5/0.5B/Qwen2.5-0.5B-Instruct/Qwen2.5-0.5B-Instruct ~/Model/QWEN/QWEN2.5/0.5B/Qwen2.5-0.5B-Instruct/Qwen2.5-0.5B-Instruct-Q4N0-HEADER --qwen2 --gen_model_header --qtype Q4_0
python3 convert_llama.py ~/Model/QWEN/QWEN2.5/0.5B/Qwen2.5-0.5B-Instruct/Qwen2.5-0.5B-Instruct ~/Model/QWEN/QWEN2.5/0.5B/Qwen2.5-0.5B-Instruct/Qwen2.5-0.5B-Instruct-Q40-HEADER-VERSION --qwen2 --qtype Q4_0 --gen_model_header --model_version "6.0.0.1"
sudo docker cp ubuntu22.04:/root/Model/QWEN/~/Model/QWEN/QWEN2.5/0.5B/Qwen2.5-0.5B-Instruct/Qwen2.5-0.5B-Instruct-Q40-HEADER-VERSION ./Qwen2.5-0.5B-Instruct-Q40-HEADER-VERSION


4. 2026/07/01 转 AWQ 模型 SDLLM-AWQ-ChatExcel_v2-20260701

cd /home/chengjian/Work/GitHub/AI/Model/QWEN/QWEN2.5/7B/Qwen2.5-Coder-7B-Instruct-SDLLMv2/sdllmv2
sudo docker cp SDLLM-AWQ-ChatExcel_v2-20260701 ubuntu22.04:/root/Model/QWEN/QWEN2.5/7B/Qwen2.5-Coder-7B-Instruct-SDLLMv2

mkdir -p ~/Model/QWEN/QWEN2.5/7B/Qwen2.5-Coder-7B-Instruct-SDLLMv2/Qwen2.5-Coder-7B-Instruct-AWQ-SDLLMv2-HEADER-VERSION

cd /root/Model/py310/test
python3 convert_awq.py /root/Model/QWEN/QWEN2.5/7B/Qwen2.5-Coder-7B-Instruct-SDLLMv2/SDLLM-AWQ-ChatExcel_v2-20260701 ~/Model/QWEN/QWEN2.5/7B/Qwen2.5-Coder-7B-Instruct-SDLLMv2/Qwen2.5-Coder-7B-Instruct-AWQ-SDLLMv2-HEADER-VERSION --group_size 128 --gen_model_header --model_version "6.0.0.1"
参见 cat config.json  | grep "transformers_version", tranformers 需要 v4.56.2, 由于标准 transformer 不支持加载 SDLLM 的模型, 模型路径下需要 register_sdllm.py, configuration_sdllm.py, convert_to_hf_sdllm.py, modeling_qwen2_sdllm.py 这几个文件来支撑.

sudo docker cp ubuntu22.04:/root/Model/QWEN/QWEN2.5/7B/Qwen2.5-Coder-7B-Instruct-SDLLMv2/Qwen2.5-Coder-7B-Instruct-AWQ-SDLLMv2-HEADER-VERSION ./


python3 convert_awq.py /root/Model/QWEN/QWEN2.5/7B/Qwen2.5-Coder-7B-Instruct-SDLLMv2/SDLLM-AWQ-ChatExcel_v2-20260701 ~/Model/QWEN/QWEN2.5/7B/Qwen2.5-Coder-7B-Instruct-SDLLMv2/Qwen2.5-Coder-7B-Instruct-AWQ-SDLLMv2-Embed_Token_FP16_HEADER-VERSION --group_size 128 --gen_model_header --model_version "6.0.0.2"
sudo docker cp ubuntu22.04:~/Model/QWEN/QWEN2.5/7B/Qwen2.5-Coder-7B-Instruct-SDLLMv2/Qwen2.5-Coder-7B-Instruct-AWQ-SDLLMv2-Embed_Token_FP16_HEADER-VERSION ./


XAMPP
git fetch --depth=100  # 拉取最近 100 次提交出
git fetch --depth=1000 # 继续拉取更多
git fetch --unshallow



=======推荐插件======
superpowers
oh-my-opencode


=======推荐 SKILL=======
1. patchwork agent, 可以后续调用
2. blog agent, 作为 rss_agent 的后续调用者
3. SKILL 描述改成中文
4. SKILL 调用要显示
https://github.com/ViffyGwaanl/papertok-reader
https://github.com/linshenkx/prompt-optimizer



cartography: Repository understanding and hierarchical codemap generation

代码检视

[Mieluoxxx/code-review](https://github.com/Mieluoxxx/opcd/tree/opencode/.opencode/skill/code-review)
[sanyuan0704/sanyuan-skills](https://github.com/sanyuan0704/sanyuan-skills/tree/main/skills/code-review-expert)


PPTX
npx skills add https://github.com/anthropics/skills --skill pptx


论文阅读


[](https://github.com/code-yeongyu/oh-my-opencode/issues/917)



这个仓库的代码包含了如下功能
1. lkml 目录实现了一个分析指定 LKML 社区补丁的 agent.
2. rss 目录是用 TRAE 生成的代码, 它是用 langgraph 实现的一个 phoronix 和 lwn 解析的 AGENT.
3. patchwork 目录是用 shell 实现的解析 patchwork 工程的脚本.
4. cgit 目录是用分析内核 commit 的 agent
5. kde.py 计划是整个工程的入口.
6. test 目录下是整个项目的测试用例
项目根目录以及每个子目录的 README 中有对相关代码架构以及功能的解析.

请完成如下个功能
请根据已有的 agent 和脚本(主要是 @src/patchwork/get_patchwork_series.sh), 在 patchwork 目录下基于 langgraph 实现一个 patchwork 的 AGENT.
1. 支持获取指定日期, 指定 projects 的 id , 所有的补丁列表, 并通过 lkml agent 对其进行分析.
2. 格式保持原来的输出格式, 并参考 lkml_agent 的输出格式.
3. 在 test 用力补齐对应的测试用例, 要求测试用例全部通过, 功能才算完成.
参见 projects id 和邮件列表的索引参考 @src/patchwork/project/projects_list.md
请使用 superpowers brainstorm skill 对该任务进行 brainstorm, 将 brainstorm 生成到 .task/20260512-patchwork_agent/01-brainstorm.md


/ralph 将 .task/20260512-patchwork_agent/02-plan.md 转换为 .task/20260512-patchwork_agent/03-plan.json

功能一:
请根据当前最新代码和架构更新 REAMDE, 包括整个项目的 README 以及各个 agent 以及子目录的 README; README 需要包括各个项目的项目介绍, 软件架构, 主要功能,
1. 架构图 - 所有模块都添加了清晰的 ASCII art 架构图
2. 工作流程 - 详细说明了每个 Agent 的工作流程和步骤
3. 技术细节 - 添加了状态类定义、API 说明、配置参数等
4. 使用示例 - 包含了完整的使用方法和示例命令
5. 代码规范 - 统一了命名约定和注释风格说明
6. 扩展计划 - 为每个模块添加了未来发展方向

功能二:
整个项目的 README 以及各个 agent 以及子目录的 README 中, 包含了整个项目和各个 AGENT 的软件架构,
diagrams 目录下有 mermaid 和 excalidraw 架构图, 但是并不美观.
请参考 README 中的架构图, 重新生成更美观的 mermaid 和 excalidraw 架构图, 更新到 diagrams 目录下, 并将 excalidraw 架构图更到到 README 中.

1. 请将生成的架构图保存在 diagrams 目录下. 包括 mermaid 代码, ASCII, svg, excalidraw 格式. 我发现之前的架构图中有 <br> 字样, 这些无法正常显示为换行, 请换种方式呈现.
2. 请通过 @beautiful-mermaid  @pretty-mermaid 生成 mermaid 代码, ASCII, svg 格式的架构图(要求保留 mermaid 代码到对应的 mmd 文件中, SVG 配色使用浅色)
3. 请通过 @excalidraw-diagram-generator 和 @excalidraw-diagram, 以及 @excalidraw 几个 SKILLS 生成【手绘风格的架构图+Excalidraw动画】, 保存原始 JSON 格式和 SVG 格式. 要求手绘风格: Excalifont字体、roughness:手绘线条、轻配色; 动画顺序: 标题→XX层→XX层→连接线, 每步时长500ms; 画布0-1200x0-800, 元素间距≥30px. 若为复杂架构图, 请通过 excalidraw 子代理委托规则执行.
4. 将最后生成的 excalidraw 手绘风格 SVG 架构图, 更新到各个 README 中.


/ralph-loop:ralph-loop  "	, 全部完成后输出 <promise>COMPLETE</promise>" --completion-promise "COMPLETE" --max-iterations 10

/ralph-loop:ralph-loop "当前 claude 的 statusline 使用了 claude-hud 插件. 如果我想继续补充 ccstatusline 插件.
让两个插件的信息都显示, 需要怎么做. ccstatusline 可以使用 npx -y ccstatusline@latest 在 command 字段中配置. 请帮我配置" --completion-promise "DONE" --max-iterations 10


整个项目的 README 以及各个 agent 以及子目录的 README 中, 包含了整个项目和各个 AGENT 的软件架构,
diagrams 目录下有 mermaid 和 excalidraw 架构图, 但是并不美观.
请参考 README 中的架构图, 重新生成更美观的 mermaid 和 excalidraw 架构图, 更新到 diagrams 目录下, 并将 excalidraw 架构图更到到 README 中.

当前 diagrams 目录下生成的架构图非常乱, 而且不美观, 都不如生成的 ASCII 图标, 给了你那么多 skills, 你干活这么垃圾还能不能行了, 重新生成下
1. 每个架构图都按照 ASCII 的图标重新生成, ASCII 如果不美观或者不完整的也需要重新生成一次.
2. 保留 mermaid 源代码(保存到 xxx.mmd), 以及生成的对应的 SVG(保存到 xxx.svg).
3. 保留 excalidraw 源代码(保存到 xxx.excalidraw.json), 以及生成的对应的 SVG(保存到 xxx.excalidraw.svg), 且文字要用手写体.
4. 之前多次生成架构图, 每次重新生成的都保留了旧的, 新的被命名为 xxx_new, 不要这样子, 每个架构图的每个格式, 只保留最美观的那个文件.
5. 最后使用 @git-commit 提交这个提交一个 COMMIT, 要求 COMMIT 描述为纯英文

整个项目的 README 以及各个 agent 以及子目录的 README 中, 包含了整个项目和各个 AGENT 的软件架构,
diagrams 下存储了架构图的素材信息, 其中
1. mermaid 源代码以及 SVG 文件,  mermaid 源代码保存在 xxx.mmd, 对应的 SVG 保存在 xxx.svg.
2. excalidraw 源代码以及 SVG 文件,  excalidraw 源代码保存在 xxx.excalidraw.json, 对应的 SVG 保存在 xxx.excalidraw.svg.



整个项目的 README 以及各个 agent 以及子目录的 README 中, 包含了整个项目和各个 AGENT 的软件架构,
diagrams 下存储了架构图的素材信息
1. mermaid 源代码以及 SVG 文件,  mermaid 源代码保存在 xxx.mmd, 对应的 SVG 保存在 xxx.svg.
2. excalidraw 源代码以及 SVG 文件,  excalidraw 源代码保存在 xxx.excalidraw.json, 对应的 SVG 保存在 xxx.excalidraw.svg.

其中当前 diagrams 目录下生成的 excalidraw 架构图非常乱, 而且不美观, 都不如生成的 ASCII 图标, 给了你那么多 skills, 你干活这么垃圾还能不能行了, 重新生成下.

1. 请自行选择 @excalidraw-diagram-generator 和 @excalidraw-diagram, @excalidraw-diagram-skill 以及 @excalidraw 几个 SKILLS 生成【手绘风格的架构图+Excalidraw动画】, 保存原始 JSON 格式和 SVG 格式. 要求手绘风格: Excalifont字体、roughness:手绘线条、轻配色; 动画顺序: 标题→XX层→XX层→连接线, 每步时长500ms; 画布0-1200x0-800, 元素间距≥30px. 若为复杂架构图, 请通过 excalidraw 子代理委托规则执行.
2. 最后使用 @git-commit 提交这个提交一个 COMMIT, 要求 COMMIT 描述为纯英文

我使用 architecture-diagram skill 生成了仓库的架构图

当前仓库还有未提交的代码, 最后使用 @git-commit 提交这个提交一个 COMMIT, 要求 COMMIT 描述为纯英文
1. 注意请将我 git add 的代码提交, 没有 add 的请不要 COMMIT


[AI开发】—— OpenCode双插件协同开发指南](https://blog.csdn.net/Lvyizhuo/article/details/157686197)
[[Question]: How to get the status of background agents? #917](https://github.com/code-yeongyu/oh-my-opencode/issues/917)





[](https://github.com/eunomia-bpf/agentsight)
[](https://github.com/aaupov/ebpf-bolt)
[](https://github.com/solatis/claude-config/tree/main)




1. 介绍下 这个 skills
https://skills.sh/jimliu/baoyu-skills/baoyu-slide-deck

2. 最后按照如下 MARKDOWN 格式输出分析
| 技能名称 | 开源地址 | 简要描述 | 安装方式 |
|:-------:|:------:|:-------:|:-------:|

/plugin uninstall everything-claude-code@everything-claude-code


对比下如下几个仓库的功能, 用途, 优势和缺点
[wshobson/agents](https://github.com/wshobson/agents)
[sangrokjung/claude-forge](https://github.com/sangrokjung/claude-forge)
[ruvnet/ruflo](https://github.com/ruvnet/ruflo) | Ruflo(Claude-Flow)
[Yeachan-Heo/oh-my-claudecode](https://github.com/Yeachan-Heo/oh-my-claudecode)
[langchain-ai/deepagents](https://github.com/langchain-ai/deepagents)



https://x.com/Voxyz_ai/status/2038237755654783107

Cliamp
kanban
everything-claude-code/skills/autonomous-loops/SKILL.md


plugin/README.md 中庸 markdown 表格的格式, 分析和记录了每个仓库的用户, 星级.

1. ## 未补全内容

但是部分内容未补全. 比如如下内容就是未补全的.

| [xyz](https://github.com/xxxx) | 260 |
| [xyz](https://github.com/xxxx) |
| [xyz](https://xxxx) | 260 |
| [xyz](https://xxxx) |
| [](https://github.com/xxxx) | 260 |
| [](https://github.com/xxxx) |
| [](https://xxxx) | 260 |
| [](https://xxxx) |


这些行往往只表格中只有 URL (可能追加了 star 数量)但没有描述的行

2. ## 评分规则

### 阈值评分系统

本工具使用阈值评分算法, 根据仓库的多项指标计算综合评分:

| 指标 | 权重 | 阈值(1星-5星) |
|------|------|-------------------|
| Stars | 60% | ≤5K, ≤10K, ≤50K, ≤100K, >100K |
| Forks | 15% | ≤100, ≤200, ≤1K, ≤5K, >5K |
| Watchers | 10% | ≤100, ≤300, ≤1K, ≤5K, >5K |
| Issues | 15% | ≤100, ≤200, ≤500, ≤1K, >1K |

- ⭐⭐⭐⭐⭐ (5星): 综合得分 ≥ 4.5
- ⭐⭐⭐⭐ (4星): 综合得分 ≥ 3.5
- ⭐⭐⭐ (3星): 综合得分 ≥ 2.5
- ⭐⭐ (2星): 综合得分 ≥ 1.5
- ⭐ (1星): 综合得分 < 1.5

请遍历所有未补全的内容
补全这部分内容, 请访问对应的网站简要分析其仓库, 分析其详细信息, 然后总结出这个仓库的目标, 技术和使用场景.
请注意
1. 一定要访问这个仓库, 然后去分析, 不要瞎编. 分析要尽可能详细
2. 要求只修改未补全的这几行, 不要全量生成整个 README.md
3. 访问 github 可能会失败, 如果访问失败, 请重试几次
4. 输出中文


领导要求每个项目绘制甘特图, 附件为2个合同及一个甘特图模板, 帮我按照模板分别绘制2个合同对应项目的甘特图, 时间未知的可以写时间待定.
1. 请使用 /pdf skills 来解析 PDF 文件
2. 生成 HTML 格式的甘特图, 同时保存成 SVG/PNG 格式



下周计划: <br/>1、天谷项目: 对接天谷下周正式评价准备工作(周一-周二); <br/>2、部门规划: 根据领导要求进一步修改知识产权部26年年度规划(周一-周四); <br/>3、价值评估: 进一步修改价值评估方案, 搜集2-3家专利价值评估机构(周二-周四); <br/>4、对接西子航空29490项目并进行下一步动作(周二-周五); <br/>5、对接杭州市局56005项目拜访工作并进行下一步动作(周二-周四); <br/>6、对接钱塘区产业专利导航工作并进行下一步动作(周二-周四)


要求按照上述计划生成 HTML + CSS 格式的甘特图.



★

歸藏(guizang.ai)
@op7418
·
6小时
最近有两个非常出圈、非常牛逼的短剧:

一个是《Enemy》, 一个是《吉时已到》, 可以看看

https://github.com/dontbesilent2025/dbskill
https://github.com/hamelsmu/evals-skills
https://github.com/sstklen/trump-code


| [DeusData/codebase-memory-mcp](https://github.com/DeusData/codebase-memory-mcp) |
https://github.com/haris-musa/excel-mcp-server

[OpenCode Day11: 5个让OpenCode记住一切的Memory插件](https://mp.weixin.qq.com/s?__biz=MzY4NDAwNDk0Ng==&mid=2247484210&idx=2&sn=9b83f311941fe23c05467f6bb1a4af25&chksm=f28b3ec43e284bf709d8503ff300c5de81a20ec2ee77dab269fc7092e7df5be8431ba7a79c17&mpshare=1&scene=1&srcid=0310TbDbuWzrVO2cHZCS9BzE&sharer_shareinfo=e8294db882b0a67f0d95a923d7d055ef&sharer_shareinfo_first=0b91e4e5cdcfe20d8de52fc7659e4407#rd)

[Claude Code 600倍加速隐藏神器！LSP一键开启, 代码查找50ms精准定位](https://mp.weixin.qq.com/s?__biz=MzcwNjA1ODkxOQ==&mid=2247484867&idx=3&sn=9eabafb46c8dca13d2ffb24eb282183d&chksm=f5f631981b2ed1136c4ef9d0ca16d5af9b365817ea7c9de1fafa888aeab077d1b474336c2ee8&mpshare=1&scene=1&srcid=0310iaugLaelp9RbOr2DMtpi&sharer_shareinfo=61f85b69f7978c63cb73cc7fd7617ffd&sharer_shareinfo_first=2d40e269eba4cdcdd264941dba56df3a#rd)


https://github.com/hesreallyhim/awesome-claude-code

https://github.com/mikewarrior/sdd-orchestrator-skill



| [A-mem](https://github.com/agiresearch/A-mem) | 基于 Zettelkasten 原理的代理式内存系统, 通过 ChromaDB 实现智能索引和链接, 支持动态内存组织、互连知识网络和持续内存演变, 适用于 LLM 代理的内存管理和知识组织 | 多模型支持 | ⭐⭐ |
| [Clawith](https://github.com/dydy-94/clawith) | 开源多智能体协作平台, 为每个 AI 智能体提供持久身份、长期记忆和独立工作空间, 支持智能体间协作和团队级管理. 主要特点包括: 数字员工身份系统、组织知识共享平台(Plaza)、任务监督机制、组织级控制、自进化能力、持久身份与记忆、私人工作空间等. 技术栈: 后端(FastAPI、SQLAlchemy、PostgreSQL/SQLite、Redis)、前端(React 19、TypeScript、Vite)、Docker 部署. 适用于团队协作、企业级应用、多智能体协同工作、知识管理等场景 | 多模型支持 | ⭐⭐ |
| [Forge](https://github.com/antinomyhq/forge) | AI 增强的终端开发环境, 是一个综合编码代理, 将 AI 能力与开发环境集成. 主要功能包括: 代码理解、新功能实现、调试辅助、代码审查、学习新技术、数据库架构设计、重构遗留代码、Git 操作等. 支持多提供商(OpenAI、Anthropic 等), 具有零配置、无缝集成、安全设计(受限 shell 模式)等特点. 技术栈: 命令行工具, 支持多种配置选项和 MCP 协议集成. 适用于开发人员日常编码、问题解决、学习新技术等场景 | 多模型支持 | ⭐⭐⭐⭐⭐
| [graph-memory](https://github.com/adoresever/graph-memory) | OpenClaw 的知识图谱上下文引擎, 解决了三个核心问题: 上下文爆炸(通过结构化知识图谱压缩)、跨会话失忆(通过 FTS5/向量搜索 + 图遍历自动召回相关知识)、技能孤岛(通过连接相关技能). 主要功能包括: 知识图谱构建(3 种节点类型、5 种边类型)、个性化 PageRank 排序、社区检测、向量去重等. 技术栈: SQLite 数据库、TypeScript、OpenAI 兼容 API. 适用于长对话场景、跨会话知识管理、技能学习与应用等场景 | 多模型支持 | ⭐⭐⭐⭐⭐
| [Notchi](https://notchi.app/) | Claude Code 的 macOS 伴侣应用, 居住在设备的 notch 区域, 实时响应每个想法、工具调用和错误. 适用于 macOS Sequoia 系统, 目前处于 beta v1.0.0 版本. 适用于 Claude Code 用户在 macOS 上的实时辅助场景 | Claude Code | ⭐⭐⭐⭐
| [memory-lancedb-pro](https://github.com/CortexReach/memory-lancedb-pro) | OpenClaw 智能体的 AI 记忆助手, 为智能体提供跨会话、跨智能体、跨时间的持久记忆. 主要功能包括: 自动捕获、智能提取(6 类分类)、智能遗忘(Weibull 衰减模型)、混合检索(向量 + BM25 全文搜索 + 交叉编码器重排序)、上下文注入、多范围隔离、多提供商支持、完整工具包等. 技术栈: LanceDB、OpenAI 兼容 API、TypeScript. 适用于智能体记忆管理、跨会话知识保持、个性化偏好学习等场景 | 多模型支持 | ⭐⭐⭐⭐⭐
| [bozhou-skills](https://github.com/bozhouDev/bozhou-skills) | 私有仓库, 无法获取详细信息 | 未知 | ⭐⭐⭐⭐
https://github.com/tickflow-org/tickflow
https://github.com/x1xhlol/system-prompts-and-models-of-ai-tools
https://github.com/IsHexx/system-prompts-and-models-of-ai-tools-chinese
https://github.com/tangshimin/MuJing
https://github.com/JOYCEQL/magic-resume
https://github.com/decksters-lab/fastfetch/tree/main
https://github.com/mayukh4/linux-android
https://github.com/HKUDS/RAG-Anything
https://github.com/Lakr233/vphone-cli
https://github.com/thunderbird/thunderbolt
https://github.com/outcomeops/context-engineering

askanyai.com
droidxai.com
https://github.com/thunderbird/thunderbolt


https://github.com/chatgptprojects/clear-code
https://github.com/aaif-goose/goose
[](https://x.com/bystander520/status/2042873263379091856)

https://github.com/ninehills/blog/issues/97
https://github.com/FradSer/dotfiles/blob/main/dot_claude/executable_statusline.sh
https://github.com/ChrisTitusTech/winutil

===diff 工具===
https://github.com/wong2/diffx
https://github.com/agavra/tuicr
https://madewithvuejs.com/ast-explorer
https://github.com/dannote/reach
https://github.com/CarterMcAlister/linear-code-review
https://github.com/bterwijn/memory_graph
https://tokennav.cc
https://github.com/pierrecomputer/pierre
https://github.com/modem-dev/hunk
https://docu.md
https://github.com/rockorager/comview
https://diffshub.com
https://github.com/nkzw-tech/codiff


===GIT===
https://github.com/affromero/gitpane
https://github.com/kitlangton/ghui
https://github.com/open-gitagent/gitagent
https://github.com/unnecessary-special-projects/ghist
https://github.com/JetpackDuba/Gitnuro
https://github.com/coffeejones/gitvision
https://github.com/Chronos778/git-rewind
https://github.com/abdosorour7/git-commands-cheatsheet
https://github.com/SSShooter/ebook-to-mindmap
https://checkmygit.com
https://github.com/DetachHead/rebased
https://github.com/github/github-mcp-server
https://github.com/AmintaCCCP/GithubStarsManager
[2026/06/08, 陈成 @chenchengpro, AI 写代码比人审代码快太多, 瓶颈早就从「produce diff」挪到了「validate diff」. no-mistakes 这个 Go 工具的思路很巧: 在你的仓库和真实远端之间塞一个本地裸仓库当「闸门」, 你 push 到 no-mistakes 这个 remote 而不是 origin(origin 永不被劫持, 普通 push 照常), 它就在一个一次性 worktree 里跑一条固定九步流水线 intent→rebase→review→test→document→lint→push→pr→ci, 全过了才转发上游并自动开干净 PR. ](https://x.com/chenchengpro/status/2063991395543859443)
https://github.com/521xueweihan/HelloGitHub
https://gh.jiayouvibe.com, 阿喵给你推荐这个工具: GitHub Open Data. 它做了一件很反常识的事: 把 GitHub 变成了一个“项目信息流”. 不再是搜索仓库, 而是直接刷开源项目. 
https://github.com/Younesfdj/gitfut, 有点意思, 只要把 github 改为 gitfut 就可以把你的 GitHub 个人资料统计数据做成 FIFA 球员卡的样子, 谁看了都想点一下. 
https://gitfut.com/gatieme
https://zaowujuzhen.com/maker/radar/github

===REPO 分析===
https://github.com/lirantal/repolyze [2026/04/20, Geek Lite @QingQ77, 在阅读代码之前, 用 git 命令快速摸底一个仓库的健康状况. ](https://x.com/QingQ77/status/2046048707284791322)
https://mintlify.wiki/ghostty-org/ghostty/introduction
https://github.com/rahulnyk/knowledge_graph
https://github.com/potpie-ai/potpie
https://github.com/trsoliu/mini-wiki
https://github.com/robert-mcdermott/ai-knowledge-graph
https://github.com/Done-0/fuck-u-code
https://github.com/Lum1104/Understand-Anything
https://github.com/study8677/awesome-architecture
https://github.com/Sidhant0707/codeautopsy
https://www.gitreverse.com 挖到一个有点逆天的玩意儿, 专门扒任何GitHub仓库背后的prompt提示词.用法简单到离谱, 压根不用装也不用配: 1️⃣ 把网址里的github直接手动换成gitreverse; 2️⃣ 它立马就把复刻这个项目的prompt甩到你脸上; 几秒钟就能逆向工程任何项目, 还是100%免费不用掏钱.

说白了, 以后刷到别人的开源项目, 不光能抄代码, 连


Computer Use
https://github.com/coasty-ai/open-computer-use
https://github.com/browser-use/video-use
https://github.com/remorses/usecomputer
https://github.com/iFurySt/open-codex-computer-use
https://github.com/injaneity/pi-computer-use
https://github.com/onesuper/tui-use
https://github.com/remorses/usecomputer
https://github.com/injaneity/pi-computer-use
https://github.com/trycua/cua
https://github.com/e2b-dev/surf
https://github.com/memohai/Autofish
https://github.com/DearVa/Everywhere
https://github.com/AmrDab/clawdcursor
https://github.com/google/agents-cli/
https://github.com/bytedance/UI-TARS-desktop
https://github.com/Core-Mate/OpenGUI
https://github.com/opencyvis/opencyvis-phone
https://www.kimi.com/features/webbridge
https://github.com/simular-ai/Agent-S
https://github.com/agent-sh/computer-use-linux
Marvis
https://github.com/browser-use/bux
https://github.com/yb2460/harness-anything
[2026/06/03, 车厘子 @0xcherry, 两个 Coding Harness 合并起来, 等于一个 Coding Harness. 这是 Coding 任务的合并律.](https://x.com/0xcherry/status/2062006201928524164)

开盒
https://github.com/sherlock-project/sherlock
https://d2lang.com/
https://github.com/HunxByts/GhostTrack
https://github.com/Z4nzu/hackingtool

===英语学习===
https://github.com/cuixueshe/earthworm
julebu.ai
https://github.com/UlionTse/translators
https://github.com/zyronon/TypeWords, 结合背单词 + 打字练习📘 内置20+词库（CET4/6、雅思、托福…）🧾 自动生成生词本和错词本👨‍💻 程序员专属API词库
🎧 支持英美音发音 + 音标显示
https://github.com/xiaochong/hi-kid
https://engoo.com/app/materials/en
https://github.com/byoungd/English-level-up-tips
https://github.com/nicejade/gpt-wordbook
https://github.com/mengxi-ream/read-frog
https://github.com/Cuimao777/cuimao-translator
https://keybr.com
graker
https://github.com/hehonghui/awesome-english-ebooks
https://github.com/Cookee24/PairTranslate 配对翻译插件
https://julebu.co/
https://github.com/fishjar/kiss-translator
https://en.kyrgk.com
https://github.com/RealKai42/qwerty-learner, 一个练英语的硬核工具：Qwerty Learner。把背单词直接塞进打字练习里，一边敲一边记，效率拉满。
https://github.com/echo-loop/Echo-Loop
https://readto.ai/
@midudev
从零开始 → http://curso-ingles.com
口语 → http://sesame.com
语法 → http://engvid.com
通过电影 → http://playphrase.me
剑桥 → http://cambridgeenglish.org
听力 → http://bbc.co.uk/learningenglish
https://github.com/HoraceLuBFA/en-zh-translation-polish
@tinyfool
https://github.com/forestai123456/hear-me-out, 听我解释
Chrome / Edge 浏览器扩展 — 为网页视频生成实时 AI 字幕, 支持网页翻译和划词翻译. 
https://engoo.com/app/materials/en
https://github.com/tangshimin/MuJing, 幕境是一款通过电影、美剧或文档中的真实语境学习英语单词的应用，让您在原汁原味的情境中记忆词汇，提升学习效率。
https://github.com/byoungd/up, 这篇爆火的英语学习指南，核心只有一句话：你以前学英语的方式，从根本上就错了。传统的死记硬背路线，几乎全踩雷。真正高效的路径完全不同

1」Trancy  官网 http://trancy.org
除了沉浸翻译, 还有 YouTube 双语字幕功能, UI 更好看, 目前搭配DeepSeek 主力在用. 免费, 高级功能付费, 闭源. 
2」FluentRead 流畅阅读 ,免费开源 https://github.com/Bistutu/FluentRead
3」KISS Translator 简约翻译 , 免费开源 https://github.com/fishjar/kiss-translator
4」Read Frog 陪读蛙 免费开源 https://github.com/mengxi-ream/read-frog
http://github.com/tangshimin/MuJing, GitHub 上的开源工具 MuJing(幕境), 把电影、美剧、字幕和文档里的真实表达变成你的词汇课堂: 在情境中理解, 在反复出现中巩固, 再通过练习把记忆“锁死”. 在线地址: https://mujingx.com
[2026/07/01, 墓碑科技 @mubeitech, 学外语最省钱、也最粗暴的办法: 边快走, 边像疯子一样自言自语. 这套方法有个正经的名字, 叫“影子跟读法”(Shadowing). ](https://x.com/mubeitech/status/2072007624770424995)
https://github.com/deusyu/translate-book/tree/main Claude Code Skill, 使用并行 subagent 将整本书(PDF/DOCX/EPUB)翻译成任意语言. 
[2026/07/08, Rey判断位｜英语自由 @ReynoldDai, 英语接入世界是判断位的第1块积木,  打开人生选项集的起点](https://x.com/ReynoldDai/status/2074814728376135913)
[2026/07/08, 董币哥 @NextBullReady, 个人认为目前实现全英交流最快的方式之一, 最快7天见效](https://x.com/NextBullReady/status/2074705283570602292)
[2026/07/07, 孤桜ETH @GYLQ520, 2017年, 有位朋友向他请教“高效学英语的路径”, 他当场梳理了一份实操指南. ](https://x.com/GYLQ520/status/2074487666389991700)
[Huan @Huanusa, 会英语-能用其实只需要这850个单词！](https://x.com/Huanusa/status/2075222778207162464), 我写了一个iOS的应用: 词骨英语850, http://lei-lei.org/850s, 有对应的单词游戏网站, 大家可以自由学习, 😂 巧了我这也有一个对应的网站: https://ogden-basic-english-7s7.pages.dev
ENGSENCE, 韩国人学英语
https://english.modengsir.com, 英语点读
learn.kugua.cn, 这个我做的儿童英语口语学习的，可以交流下，免费的，需要注册，因为需要token
https://github.com/RealKai42/qwerty-learner, 这软件qwerty-learner专治键盘工作者：一边背单词一边练打字，打错必须重来，把正确拼写焊进肌肉记忆。四六级、考研、GRE、程序员词库全有，还有 VSCode 摸鱼插件。
https://waiyuka.cc/zh-CN, 轻松背单词, AI 配图 + 标准发音 + 英语音素级发音分析，让单词学习更高效. 覆盖西班牙语、法语、德语、英语、日语，共 3 万+ 词库.
《床头灯英语》1到5级打包，号称用3000词读遍世界名著，文本、音频、PDF都齐。
https://parroto.app/id#google_vignette, 怎么可能功能这么齐全——有英语短视频、词汇游戏、听写练习、跟读练习，甚至还有 TOEIC 和 IELTS 模拟测试。而且大部分功能都是免费的
https://fluentdictation.com/zh, 发现一个挺好用的免费英语听写在线网站：FluentDictation。
Book Dash, 是一个提供英文绘本的公益网站：词汇量较小、句子结构简单、配图丰富，适合英语入门，也适合家长陪孩子共读。部分绘本还配有真人朗读和视频，能同时练阅读与听力。
我觉得 CapWords 挺好的啊
火遍海淀妈妈圈的清华附小英语动画片，1-6年级覆盖小学全部英语知
免费英语听力网站：ELLLO。
https://www.bbc.co.uk/learningenglish/, BC Learning English
有简单，普通和难度的分级，可以根据自己的水平选择，甚至从基础学起都可以。视频可以看，音频可以听，视频和音频的台本-对话台词-都可以免费下载。非常适合学习。
每天分享一个宝藏网站第三十七期
今天要讲的是http://ameponline.homeaffairs.gov.au
跟着迪士尼电影学习英语
https://simple.wikipedia.org/wiki/Main_Page, 它是维基百科的「简单英语版」，把复杂词汇和长句改写成更易懂的表达。读百科的同时，把英语阅读也练起来。
练习英语口语的app，名字是Unscript （脱稿）， https://x.com/wulianwen1/status/2100065447785935253
开口鸭, https://apps.apple.com/cn/app/%E5%BC%80%E5%8F%A3%E9%B8%AD-%E4%BB%8E%E4%B8%8D%E6%95%A2%E8%AF%B4%E5%88%B0%E4%BC%9A%E8%A1%A8%E8%BE%BE/id6761402209
https://github.com/knowledgefxg/learning-english, 精选优质英语学习资源合集，专注于听说读写等核心技能的提升。包含语法、词汇和媒体资源，助您更好地学习英语。
https://github.com/kkkoaoa/biscuit-studio, 小饼干英语视频工坊：从英语知识点到 30 秒竖屏教学视频的一站式 AI 创作工具。 已上线，网址可见：
https://github.com/gtiipoi/ielts-practice, 备考雅思听力，这个开源项目可以直接上手刷题。IELTS Listening Practice 把雅思听力题搬到网页上：不用装软件，打开就能练。适合用电脑做精听、熟题型：音频、题目、答案一页搞定，做完立刻对照解析，不用再到处找音频和 PDF、来回切换。
https://github.com/lilinji/English,  全网最全英语单词词库 (Anki/Quizlet 等背词工具兼容)
https://www.ncego.com, 发现一个适合系统学英语的网站：夸克英语笔记. 从《新概念英语》第一册到第四册，再到《走遍美国》，按难度整理了完整的教材、课文、语法、词汇和听力口语材料，适合从零基础一路学到中高级。
[2026/10/08, VIVI @VIVISTAYGOLD, 我是如何在2个月内自学考到雅思7.5，口语7分的？](https://x.com/VIVISTAYGOLD/status/2108186854214471888)


===理财===
https://www.wise-hold.com/
https://github.com/komako-workshop/digital-oracle
https://wall-street-skill.com/
https://github.com/XiaomingX/ai-money-maker-handbook
https://github.com/brokermr810/QuantDinger
[2026/04/05, 邦比快跑 @binbinmath, 两天时间, 我把巴菲特70年的股东信变成了知识图谱](https://x.com/binbinmath/status/2040690438122987762)
https://btcdca.me
http://tiatleak.com/sol
https://creao.ai/@Moyu
https://github.com/pingdj/Web3
https://invest-nav.com/tools/investment-handbook/ai-optical-interconnect-chain/
https://github.com/waditu/tushare
https://www.wise-hold.com/
https://github.com/Open-Dev-Society/OpenStock
https://github.com/QuantConnect/Lean
https://www.slickcharts.com, https://x.com/YaelC_03/status/2060686265734066644
https://github.com/ArvinLovegood/go-stock
https://github.com/zgwl/chinese-buy-us-stock-guide
https://github.com/TNT-Likely/PanWatch, https://x.com/eastweb3eth/status/2068157453251080498
https://wise-etf.com
[Finviz](https://x.com/AYi_AInotes/status/2067904860486349263) https://x.com/YaelC_03/status/2067495682093764817
https://aichainmap.com/serenity, 白毛速报: 有人根据白毛股神的推文做了一个产品. 你看这种产品都是 AI slop, 但没人在乎, 能获取到有用的信息就行. 拿别人的内容包装一下, 还可以做成盈利产品, 需要付费才能解锁全部内容. 
https://www.ainvest.com/terminal/真诚推荐一个自用的美股交易工具 AInvest terminal mac 版本. 好用在哪里呢？一句话可以建设你自己的交易界面, 你想看什么, 想怎么调整就怎么调整, 很舒服. 最关键的是, 可以直接通过AI对话来帮助你在实时图表上画K线, 这个在其他的大模型上是很难做到的. 
Machine Learning for Trading
Vibe-Research
[研究美股最应该关注的 10 个网站](https://x.com/spicycandy00/status/2074727556511838292)
[断浪 @waveking1314, 如果你连这 10 个网站都不知道, 真的别急着去炒美股. 美股的信息密度很高, 只靠 X 上几个 KOL 喊票, 很容易被情绪带着跑. ](https://x.com/waveking1314/status/2074512270017896603)
[鸟哥 | 蓝鸟会🕊️ @NFTCPS, 如果你连这10个网站都不知道就不要去炒美股了！](https://x.com/NFTCPS/status/2074380392195399942)
https://github.com/LLMQuant/quant-wiki
https://github.com/xuchonglang/investing-for-beginners  投资入门指南
https://chatgpt.com/apps?q=longbridge, 长桥 Longbridge 已经在 ChatGPT 发布上线了. 超过 140 个 MCP tools, 涵盖: 全面的股票/期权行情、资讯、财报、分红派息、分析师预测、股东信息、机构持仓变动、经营回顾、完整的历史 K 线, 还有你的持仓订单 -- 你需要的都有. 让它帮你分析持仓、发现投资机会. 
https://www.starterstory.com, 发现一个网站, 专门收集 starter idea 并公开收入. 里面有不少 idea 还挺有意思的
https://invest.howlifeusa.com/zh, 美国投资指南, HowLifeUSA推出的《投资百科》
[华尔街观察 Xtrader @cnfinancewatch, 十个短线客喜欢的A股 APP](https://x.com/cnfinancewatch/status/2074645568492904818)
stock-sdk
https://github.com/calesthio/BreakoutAnalysis, 研究股票的朋友多少有这个困扰, 平时不可能一直盯盘, 但又生怕错过突然放量拉升的票. BreakoutAnalysis 干的就是盯盘这个活, 开市期间每 15 分钟扫一遍全美股市场, 揪出放量大涨的股票. 
https://www.quiverquant.com, 它可以追踪：1、美国国会议员股票交易 2、特朗普相关持仓 3、Jim Cramer 股票观点 4、公司高管内幕交易; 还有机构持仓、政府合同、游说数据等。
https://touzid.com, 炒股与投资过程中到底去哪里查看各类数据？
https://www.languagereactor.com, 别再死背单词了. Language Reactor 能把你本来就在看的东西，变成英语输入. 看剧、刷 YouTube、听播客，顺手把英语学了装个 Chrome 插件，Netflix  YouTube 直接出双语字幕. 鼠标点单词：释义、语境、用法都出来，还能存进笔记. 站点上还有 TED、BBC、播客、公版书. 不止英语，日语韩语法语等 40 多种语言都能用. 免费功能已经够用
https://learnenglish.britishcouncil.org, 学语言不用瞎找，这几个网站真该收好！英语
https://github.com/wquguru/dasheng, 英语流利说那一套，已经能够被 Jev + 网易有道的实时 ASR 模型 Confucius4-R2T2 平替了，更关键的是，R2T2 和项目仓库都是开源的！！
https://github.com/wenyuanw/a-share-heatmap, A 股热力图
https://suoha888.github.io/Trader-Archives, Trader-Archives, 推特上 90% 的内容都是噪音，但剩下的 10% 绝对是无价的干货。我用CodeX+X的API+JEV 把二级、宏观、链上的全能高手麦总 @Michael_Liu93  的推文全收录了！现在Trader-Archives 已经收录两位实战交易员: 第一期，套利/LP 高手 CJ 老师 
@gch_enbsbxbs 第二期，全能交易员/麦总 @Michael_Liu93

===可视化===
https://github.com/drasimwagan/mdv
https://github.com/amit9838/brewlens
https://github.com/sqshq/sampler
https://github.com/neneodonkor/asciigraph-rs
https://regex-vis.com/
https://github.com/Domenez-dev/lazy-cron
https://github.com/SurpriseDog/LazyCron
https://github.com/unhappychoice/splashboard
https://docu.md
https://github.com/pranshuparmar/witr
https://github.com/HarleyCoops/Math-To-Manim
https://github.com/HugeCatLab/ChatTutor
https://langsagne.vercel.app/
https://visualgo.net/en
https://seeing-theory.brown.edu/cn.html
https://www.napkin.ai/  可视化图表
https://github.com/asciidraw/asciidraw.github.io
https://github.com/GordenSun/mathVideoMaker
https://github.com/HugeCatLab/ChatTutor
https://github.com/andyhuo520/aetherviz-master
https://github.com/charmbracelet/freeze
https://github.com/Makisuo/maple

===Office===
https://github.com/manpoai/AgentOfficeSuite
https://github.com/iOfficeAI/OfficeCLI
https://github.com/larksuite/cli
https://github.com/officecli/officedex


===安全===
https://github.com/Armur-Ai/Pentest-Swarm-AI

===软件===
https://github.com/stackia/best-windows-apps
https://youmind.com/zh-CN/skills/x6ux3YCCI2qtaT
https://github.com/sindresorhus/awesome
https://github.com/scriptscat/scriptcat
https://github.com/SuperManito/LinuxMirrors
https://github.com/kimlimjustin/xplorer
https://github.com/ricocc/rico-md
https://github.com/bannedbook/fanqiang
https://github.com/appergb/openless
| [](https://github.com/karinushka/paneru) | 1,442 | 窗口管理器
| [](https://github.com/niri-wm/niri) | 24,028 | 窗口管理器
韩国某哥们为了戒网瘾, 把1119个"危险网站"全拉黑, 结果这份名单一上GitHub, 直接变成了全网最热宝藏导航—— https://github.com/wpzzz/blocked-sites-in-south-korea
https://github.com/eduwass/tmux-palette
https://github.com/jaywcjlove/awesome-mac
https://appstoreprice.org/zh/apps
https://github.com/LiuMengxuan04/shushu-internship-tool
https://github.com/tejas-raskar/noted.md
https://github.com/wangrongding/clash-kit
https://github.com/l0ng-ai/papr
https://github.com/plait-board/drawnix
https://appark.ai/en/top-charts/app-store
https://www.paywallpro.app/zh
https://github.com/clefspear/starcommand
https://github.com/Ponphil/LitePan
https://github.com/cccyd2003-qwq/pinkbin
https://app.liulian.ink/
https://github.com/zakirullin/files.md
https://github.com/LOWERTOP/Shadowrocket-First#shadowrocket-%E9%85%8D%E8%89%B2%E6%96%87%E4%BB%B6
https://github.com/superloglabs/superlog
https://github.com/hydropix/TranslateBooksWithLLMs
https://github.com/bannedbook/fanqiang
https://github.com/mikf/gallery-dl
https://github.com/ShadowArcanist/netviz
https://github.com/colinvkim/Radix
https://github.com/beltromatti/get-it
https://github.com/vid4l-07/Klip
https://v.bxkp.org/
https://mineru.net/
https://github.com/stardustai/dataset-viewer
https://github.com/opendataloader-project/opendataloader-pdf
[2026/06/04, 歸藏(guizang.ai) @op7418, 即览: 手机上看 Markdown 和 HTML, 怎么就这么难？](https://x.com/op7418/status/2062361524367401461)
https://github.com/MrsEWE44/musicDownload
https://github.com/code2rich/jpage, 拖入 HTML 或 Markdown 文件即得一个可分享的在线页面, 零配置、无需部署. 即页是个零配置的上传即预览工具. 把 HTML 或 Markdown 文件拖进去, 秒级生成在线页面, 自动给你一个短链接. 支持代码高亮、数学公式、流程图渲染, 还有版本历史、标签分类、多用户权限这些功能.
https://appstoreprice.org/zh/apps
https://fmhy.net [2026/06/19, Amto @XAMTO_AI, 听说了没, 闲鱼上那些卖家好多都跑这儿来进货！](https://x.com/XAMTO_AI/status/2067803303791260086)
https://github.com/rccyx/thyx Linux 登录管理器
https://github.com/Milktang0128/Dob, 在 macOS 上选中文本后快速用 AI 朗读、解释、翻译、审校并留档的菜单栏工具. 一个安静的 macOS 菜单栏工具. 选中一段文字, 它就弹出工具条——朗读、翻译、解释、提炼、审校, 点一下就好. 支持拿当前页面的上下文一起发给 AI, 也能用多个模型比较结果. 看完觉得有用就留档, 存成本地 Markdown. 兼容 DeepSeek、OpenAI 和各种国产模型. 
https://search-sharp.com, 闲鱼搜不到某件东西，很多时候不是没有货，而是搜索词没对上卖家的表达。Search Sharp 可以把“想找的物品”转换成更适合闲鱼检索的关键词。输入目标，它会给出可用的搜索说法，淘二手、小众物件和冷门商品时尤其方便。换一个词，可能就能打开完全不同的结果页。
https://github.com/chess99/tab-out
[2026/06/30, 唐清乐 @tangqingyue, 刷知乎看到知乎开放AI接口了](https://x.com/tangqingyue/status/2071877502189138353)
https://github.com/ricocc/rico-bookmark-manager,  做了一个书签 SKill 来管理我乱糟糟的 3000 多条浏览器书签
🔗 https://github.com/ricocc/rico-bookmark-manager, 支持生成一个内置的书签导航站, 可以在导航站上进行书签的管理和修改, 然后导出书签数据. 
领哥LingGe, @shangdu2005 的个人网站; 卧槽！终于让我搞成了！https://lingge66.pages.dev/
https://github.com/Ebullioscopic/Atoll, 苹果灵动岛
最近开发了一个类似slideshare的 PPT共享网站:  https://ppt.qiaomu.ai 大家可以注册上传 keynote、pdf、pptx、html ppt 成为全屏演示PPT, 以后出去分享可以不用带电脑, 哈哈哈. Demo: 动漫解读哲学是什么  https://ppt.qiaomu.ai/decks/zhexue-shishenme
Typeless
https://github.com/elebumm/RedditVideoMakerBot
https://github.com/NanmiCoder/MediaCrawler,  MediaCrawler - 自媒体平台爬虫 🕷️
[2026/07/08, Ren @Ryrenz, 用好这 6 个工具, 机票、路线、酒店全给你省一大笔](https://x.com/Ryrenz/status/2074705035389456863)
https://github.com/larlarua/AutoCVE
https://github.com/lzh-phd/topic-feasibility-screener 手头有个数据集但不知道该往哪个方向做？它直接帮你发散出多个选题. 然后自动去扒文献, 能搜出直接证据还是只有桥接文献——这种区分很实用. 还会拿你的数据跑个初步回归, 验证实证可行性. 最后给每个候选选题打0-100的分, 包含理论支持、证据缺口、创新价值. 等于把“选题–查文献–试回归”这一套前期试错流程自动走了一遍. 
https://github.com/fmhy/edit GitHub 上的 FMHY 是一份把全网免费资源打包整理好的宝藏合集, 强烈建议直接收藏. 电影、动漫、音乐、书籍、下载、游戏、去广告等常用资源一应俱全, 找资源不再东翻西找. 不止资源, 它还汇总了系统、文件管理、网络、
https://github.com/wickenico/WailBrew, WailBrew 就是为此而来: 一款开源的 Homebrew 可视化工具, 把 Homebrew 变成“App Store 式”操作——浏览、搜索、安装、管理, 一目了然、上手即用. 
https://github.com/kalcaddle/kodbox, 直接把浏览器魔改成了一个完整的电脑桌面系统. GitHub 上挖出的 3.2k 星开源神器 kodbox, 名义上是个 Web 文件管理器, 实际上是个随身的云端 OS + 在线 IDE. 不管你人在哪, 用谁的电脑, 只要打开浏览器, 立马接管你的所有云盘和本地文件, 还能直接写代码建网站, 白嫖党狂喜！
https://sobrief.com, 不再错过任何好书
https://github.com/nilbuild/developer-roadmap, 一个由社区驱动的交互式开发者成长路线平台, 目前在GitHub上已狂揽360k+星标, 全球排名高居第7位. 
https://web.archive.org, 给网页留个永久备份，页面被删、网站倒闭，你手里还攥着底牌。
https://github.com/siknet/FreePEP
https://github.com/ikevss/wechat-ai-memory
https://12306.cjh0613.com/zh/free-refund, 12306免费退票助手; 一个用于避免火车票退票手续费的网站。通过提供晚点列车实时监控信息和详细的免费退票操作指南，帮助用户在列车晚点时顺利退票，从而节省费用。无需输入12306账号密码，所有购票和退票操作仍在官方APP内完成。功能完全免费，无需付费或登录任何账号，只需完成人机验证即可查看晚点列车信息。
https://t.urongda.com, 高考真题大全. 历年全国高考真题与答案解析，一键下载，助力备考。
https://github.com/SteveSuv/remix-words-funny, WordsFunny 是一个简单、免费、开源的英语单词学习网站，围绕单词书、单词详情、学习进度和评论互动组织学习流程。
https://huarun.win, 想查科学上网工具底细，以前得在 GitHub 里翻到手指发酸，现在一个站就给你兜住了，它叫华润赢。
https://zoo.ink/tvbox.html, 有位大佬把 TvBox 全家桶归置妥了，一条龙摆齐，白嫖党看了直呼过瘾
https://bjarneo.github.io/ferroprint, 蓝图绘制工具Bjarne Øverli @iamdothash, 这将取代我的 Excalidraw。Ferroprint。Ferroprint，源自制造首批蓝图的铁氰化铁工艺
https://github.com/KingKongRobotics/jumper, Jumper — an crab robot.
https://github.com/qingjian-team/qingjian, 青简 Qingjian：用 Rust 写的拼音输入法，候选词旁多一条正在学的语言的译词
===网站复刻===
https://github.com/uxKero/anydesign
https://github.com/JCodesMore/ai-website-cloner-template


ourse-marketing.html)

===CLI===
https://github.com/iOfficeAI/OfficeCLI
https://github.com/jackwener/wx-cli
https://down.mptext.top/dashboard/api
https://github.com/teng-lin/notebooklm-py
https://github.com/adithya-s-k/omniparse
https://github.com/can4hou6joeng4/boss-agent-cli
https://github.com/521xueweihan/HelloGitHub
https://github.com/lawrencewzen/imgen
[2026/06/01, yibie @yibie, 把 AI Agent 当普通 CLI 用——pipe in, pipe out.](https://x.com/yibie/status/2061466323306397956)
https://github.com/ntrospect0/glint
https://github.com/microsoft/intelligent-terminal
https://github.com/ruvnet/agent-harness-generator
https://github.com/langchain-ai/openwiki OpenWiki —— 让你的 AI Agent 拥有最新代码文档的工具 OpenWiki 是 LangChain 团队出品的 CLI 工具, 专为 AI Agent 设计, 能自动为代码库生成并持续维护高质量的 agent 文档. 
https://tikhub.io, 面向具身智能的社交推理与情感智能数据集——以及驱动量化交易、政府与国防、营销广告领域 AI 智能体的实时社媒数据 API. 

===WeChat===
https://github.com/wechat-article/wechat-article-exporter
微信公众号文章批量下载导出, 终于有了一个相对完整的开源方案:
wechat-article-exporter, GitHub 1.1 万+ stars.
在线版可以直接用:
https://down.mptext.top
GitHub:
https://github.com/wechat-article/wechat-article-exporter
它能通过公众号关键词定位文章, 批量拉取文章列表, 并导出为 html / excel / md / docx / json / txt 六种格式.
最实用的是 HTML 导出: 图片和样式会一起打包, 离线打开也能尽量保留原文排版. 对于做个人知识库、资料归档、内容研究的人, 这个功能非常直接.
如果配合抓包拿到 credentials, 还可以进一步导出阅读量、评论、评论回复、转发量等数据. 做公众号内容分析的人, 可以少写很多重复脚本.
不想依赖第三方服务, 也支持 Docker 或 Cloudflare 私有化部署. 文档在:
https://docs.mptext.top
最后提醒一句: 文章版权归原作者. 工具适合归档自己读过、需要研究和整理的内容, 不适合拿去商用侵权.
对长期做内容、研究、知识管理的人来说, 这类工具值得放进自己的工作流.
https://github.com/jackwu321/Wechat-Fav-Export
https://aichuhai.dev/
https://github.com/Gloridust/WechatOnCloud
https://github.com/limin112/wechat-publish-template
https://github.com/extrastu/readneo
https://down.mptext.top
https://github.com/zjp1997720/wechat-article-search
https://github.com/wuchubuzai2018/expert-skills-hub
https://github.com/wuchubuzai2018/expert-skills-hub/blob/main/skills/wechat-article-search/SKILL.md
https://www.md2wechat.cn/features 内容表达系统, Agent 公众号创作与发布 CLI, 把 Markdown 排版成公众号草稿. md2wechat 覆盖微信 Markdown 编辑器、公众号排版、主题模块、封面配图、标题建议、API 转换和 md2wechat Skill, 帮助人和 Agent 稳定完成公众号发布流程. 
https://mpbook.geekeditor.com, 公号书 MPBOOK, 把关注的公众号, 装进一座私人书房.
https://studio.yituohub.com, 给各位安利一个自用的公众号排版工具
叭哥说官方网站：https://bug.youdao.com
https://github.com/sennkuwu/wechat-style-extractor, 从公众号文章中统计颜色、字号、行高、间距和内容组件，预览并复制文章排版。样式统计是启发式识别；未识别项不显示，预览使用默认值。内联样式不能保证公众号编辑器完整保留原始排版。
https://github.com/freestylefly/WeChatBridge, 微信聊天记录一键转发到 AI Agent 与 Obsidian 的原生 macOS 工具
https://github.com/yqxie1991/WxCleaner, WxCleaner - 微信重复文件清理工具. WxCleaner 是一款专为清理微信接收文件而设计的轻量级、跨平台桌面工具. 它能够高效扫描指定目录, 通过智能哈希算法快速识别重复文件, 并提供可视化的清理界面, 帮助用户安全释放磁盘空间.


====雷达===
https://x.com/Jason23818126/status/2099110080268615912
https://github.com/Thysrael/Horizon
https://github.com/sansan0/TrendRadar
https://github.com/ourongxing/newsnow
https://sopilot.net/zh/hot-tweets
https://github.com/zarazhangrui/follow-builders
https://rebang.open2hub.com
https://ithome.com
1、今日热榜(类目最全)
常规平台都有, 更狠的是 GitHub Trending、Product Hunt、Hacker News 全给你聚合了——技术圈和产品圈的风向标, 一个顶十个.
https://tophub.today/c/tech
2、AI 今日热榜
只盯 AI 圈, GitHub Trend、Product Hunt、HuggingFace 热门、OpenAI、Anthropic……做 AI 内容的每天必扫一遍.
https://aihot.today, https://github.com/KKKKhazix/AIHOT
https://contexi.shadowlaws.com/ai-products/pricing
3、SoPilot(X 爆帖专用)
只看 X 上真正爆了的推文, 抢前排评论、快速捕捉舆论风向全靠它.
https://sopilot.net/zh/hot-tweets
国外补充: Reddit / Hacker News 热门内容汉化版
https://buzzing.cc
国外多平台聚合, 类似今日热榜的模式, 刷英文信息源用:
https://techurls.com
https://devurls.com
AttentionVC: 同时收录中英文爆火 X 热点和 github 热点, 追上英推的前沿热点
https://attentionvc.ai
AI 论文简报: 不看最新的重量级论文怎能了解 AI 近况, AI 论文简报每天三分钟, 跟上学术圈最新突破！
https://ai-brief.liziran.com/zh/
中国独立开发产品合集: 一个每天更新的 github 仓库, 将国内独立开发产品一网打尽, 看看大家都在做什么, 模仿是通往成功最高效的路径.
https://github.com/1c7/chinese-independent-developer?tab=readme-ov-file
https://github.com/leiting-eric/DailyBrief
https://www.reddtrends.com/
https://github.com/Pls-1q43/Dibao
https://news.mksaas.link
https://mkdollar.com/news
https://github.com/YD4223/aihub
https://aihot.virxact.com/   数字生命卡兹克 @Khazix0918 的作品
https://news.stormzhang.ai/
https://liziran.com/zh/column/ 博客标题鼠标悬浮上去, 有封面, 挺好玩的. 
https://best.xiaohu.ai/ AI 圈最新的东西, 用大白话讲透
https://github.com/JaredYe04/news-bot
https://youtube.qiaomu.ai/
https://hype.replicate.dev, [别再刷推特了，这个网站才是真正的 AI 情报站！1. GitHub, 2. Replicate, 3. HuggingFace, 4. Reddit](https://x.com/0xCheshire/status/2097662818455900245)
https://github.com/ohmyangboy/PaperRss, PaperRss: 现代纸感 RSS 阅读器原生 macOS 应用. 界面简约沉浸，支持双语翻译与克制的 AI 功能, 一切以阅读为先.
https://qingkeai.online/blog, 
https://github.com/fuxiaoai/tidings-rss

===论坛===
https://x.com/wsl8297/status/2061001992752034140
https://x.com/cxjwin/status/2056028025721090538
[2026/06/16, Ren @FakeMaidenMaker, AI 内容创作的地基: 信息源(以 AI、投资、设计三个领域为例)](https://x.com/Ryrenz/status/2066724310929088601)
https://startup-radar-site.vercel.app/ 分享一个创业阅读网站, 是我做来消耗碎片时间的. 本来也没期望自己会常看, 一阵子后我发现自己还挺爱读的, 经常会打开看一看. 挺有助于创业者随机地去了解一下新的或热门的创业方向, 或了解下现在市场上有什么有意思的初创公司. 它选的文章也挺经典的. 

===科研===
https://github.com/dw-dengwei/daily-arXiv-ai-enhanced
https://github.com/JOYCEQL/magic-resume
Palantir
https://github.com/hxdflying/paperbanana
https://totoro-jam.github.io/battle-tested-patterns/zh/
[2026/06/14, paperpaper @paperpaper886, 最近在带入组的本科实习生, 发现怎么读论文其实是科研训练里最容易被忽略的一步. ](https://x.com/paperpaper886/status/2066148829439861016)
https://web.stanford.edu/class/ee384m/Handouts/HowtoReadPaper.pdf
[2026/06/19, 黄小木 @ai_xiaomu, 写论文用这个工具: zgpaper, 省掉80%的废话时间: ](https://x.com/ai_xiaomu/status/2067926365412774011)
arXiv 生态系统里有一批非常好用的第三方工具, 每个解决一个特定痛点. 
Semantic Scholar, 地址: http://semanticscholar.org, Semantic Scholar 对 arXiv 论文做了语义索引, 它的"Related Papers"功能比 arXiv 原生的好用得多. 输入一篇你觉得重要的论文, 它能找到内容真正相关(而不只是关键词匹配)的其他论文. 它还提供影响力指标——"Highly Influential Citations"只统计那些实质性引用了这篇论文的后续工作, 过滤掉了"Related Work 里随手一提"的水引用. 
Connected Papers, 地址: http://connectedpapers.com, 输入一篇论文, 它会生成一张可视化的关联图谱. 节点是论文, 边表示相似度. 你能一眼看出一个研究方向的脉络——哪些是开创性工作, 哪些是最新进展, 哪些是连接不同方向的桥梁论文. 最适合的场景:  你刚进入一个新领域, 需要快速建立全景认知. 
Papers With Code, 地址: http://paperswithcode.com, 这个工具的核心价值是把论文和代码实现对应起来. 看到一篇论文, 直接跳到它的 GitHub 仓库看实现. 它还维护了各个 benchmark 的排行榜(State-of-the-Art), 你能看到每个任务上当前最好的方法是什么. 
ar5iv, 地址: http://ar5iv.labs.arxiv.org, 把 arXiv 论文的 LaTeX 源码渲染成 HTML 页面. 阅读体验比 PDF 好很多, 尤其在手机上. 用法很简单——把论文 URL 里的 http://arxiv.org 换成 http://ar5iv.labs.arxiv.org 就行. 
Hugging Face Daily Papers, 地址: http://huggingface.co/papers, Hugging Face 社区每天投票选出最值得关注的 AI 论文. 如果你不想自己筛选, 这是一个高质量的"人肉过滤器". 


===配色===
https://github.com/nevertoday/zhongguo-traditional-colors
https://colorhunt.co
https://javii.tools, 它的功能很简单: 上传一张图片或一段视频, 输出由ASCII字符组成的画面. 每个像素点被替换成一个字符, 字符的密度和形状模拟原图的明暗和轮廓. 


===求职===
https://github.com/yuanzhongqiao/ai-interview-platform
https://github.com/vasu-devs/JustHireMe
https://github.com/goodseafly/how-to-make-work-pay, 教你把工资、加班费和没休完的年假一分不少地要回来，社保断了怎么接、工伤生育怎么办；被针对、被调岗、被裁时知道怎么走、怎么不留尾巴。576 条建议，每条写明花掉什么、换回什么、证据有多硬，来源只引官方文件原文和研究文献。

===内核===
https://github.com/tamnd/kernel-index
https://github.com/withesse/linux-kernel-analysis


https://www.llm-book.com/
https://github.com/handsOnLLM/Hands-On-Large-Language-Models
https://github.com/Feather-2/Burner-X

[无颜 @WY_mask, 突然注意到还分享了100多个开源的aiAgent项目](https://x.com/WY_mask/status/2053428837015343286)


obsidian
notebook navigator 插件
https://docs.ollama.com/integrations/claude-desktop
https://github.com/danielrosehill/Awesome-Obsidian-AI-Tools
阅迹 Pro
bijitongbu.site

===浏览器插件===
https://x.com/Jolyne_AI/status/2071820285645586718
https://github.com/027xiguapi/code-box 免登录复制代码: 不用扫码, 不用登录, 看到代码直接一键拿走. 强解关注限制: 什么“关注博主阅读全文”, 统统强制解锁, 懂的都懂. 一键文章扒站: 直接把整篇文章下载成 Markdown 或 HTML, 本地囤货爽翻. 流氓弹窗杀手: 恶心的登录弹窗、强制跳转 APP 提示, 全部干碎. 
https://github.com/scriptscat/scriptcat, 在 GitHub 挖到一款比油猴(Tampermonkey)更狠的浏览器脚本插件: ScriptCat. 油猴脚本完全兼容不说, 最大杀手锏是支持后台脚本: 网页关了也照跑. 再加上云同步和脚本订阅, 从安装到管理一条龙, 脚本控直接省心. 
https://github.com/qufei1993/skills-hub, X 赛博清洁工：高置信垃圾账号黄框标注，一键原生拉黑，全端同步消失
===爬虫===
https://github.com/D4Vinci/Scrapling
https://github.com/withoneai/cli
https://github.com/jackwener/wx-cli
https://github.com/machinepulse-ai/world2agent
https://github.com/unclecode/crawl4ai
jina.ai/reader/
markdowndown.vercel.app
https://youmind.com/zh-CN/landing/markdown-to-x-article
https://github.com/NanmiCoder/MediaCrawler 深度优化的爬虫工具  ||  省时省心; 一个功能强大的多平台自媒体数据采集工具, 支持小红书、音、快手、B站、抖音微博、贴吧、知乎等主流平台的公开信息抓取. 
https://github.com/miantiao-me/ssh-ai-chat, 813
https://github.com/raiyanyahya/kit
https://github.com/1broseidon/ketch,  ketch, 一个命令行工具, 集成了网页搜索、代码搜索、文档查询和网页抓取等功能. 
https://feedsieve.win/lists/keywords, 福滤娃把屏蔽关键词已经全部开源公开到官网了 
https://github.com/aizhimou/javranking-extension, 这是真牛逼，一键展示 JAV 封面与排行榜数据的浏览器扩展，省去逐个翻页的麻烦，快速浏览热门排名。



电子书
https://github.com/jbiaojerry/ebook-treasure-chest
https://github.com/bookorbit/bookorbit
https://github.com/LiuMengxuan04/shushu-internship-tool
https://lizabethli.github.io/markdown-to-wechat-converter/
https://md.doocs.org/
https://github.com/freeCodeCamp/freeCodeCamp 我的天神爷, 发现一个全球最硬核的免费编程大学, 新手转行找工作必备……😮
https://github.com/TapXWorld/ChinaTextbook
https://github.com/jbiaojerry/ebook-treasure-chest
https://github.com/BryanHoo/FeedFuse
https://tstrs.me/#google_vignette, 里面包含了200多万本书，种类更是多达上百种. 最舒服的还是不用登录  不用积分等等条件，搜完直接跳资源页。以前找书像在用腿跑论坛，现在基本是输入完就结束。


信用卡
card.lengziyu.cn 的 PC 版来了：http://card-fi.com, 把查卡、选卡、对比和办卡前需要了解的信息，整理到一个更适合电脑使用的网站里。
https://haiwaibiaoju.com/hot


bun update -g opencode-ai
opencode models --refresh
cd ~/.config/opencode && bun update oh-my-openagent






当前 opencode 有如下模型可用
opencode models --refresh
opencode/big-pickle
opencode/gpt-5-nano
opencode/minimax-m2.5-free
opencode/nemotron-3-super-free
ZhiPuGLM-CGS/GLM-5
ZhiPuGLM-CJ/GLM-5

帮我配置下 oh-my-openagent
配置文件在 ~/.config/opencode/oh-my-openagent.jsonc


解析当前仓库的整体架构, 输出"系统架构图 — Claude 官方风格" 输出路径: diagrams/v4/architecture.png 和 diagrams/v4/architecture.svg




阅读 plugin/TODO_PROMPT.md, 完成对应的工作



opencode 有如下模型可用, 帮我配置 oh-my-opencode-slim 插件中 OpenCode-Free preset
配置文件在  ~/.config/opencode/oh-my-opencode-slim.jsonc
OpenCode-Free preset 可用模型如下:

opencode/big-pickle
opencode/deepseek-v4-flash-free
opencode/laguna-s-2.1-free
opencode/ling-3.0-flash-free
opencode/mimo-v2.5-free
opencode/nemotron-3-ultra-free
opencode/north-mini-code-free

1. 配置 oh-my-opencode-slim 的各个 agent 为合适的模型
2. 其中 opencode/big-pickle 模型是大概率不会失效的，将其加入到 fallback 序列

各模型的能力优先级

deepseek-v4-flash-free > laguna-s-2.1-free > ling-3.0-flash-free > big-pickle > mimo-v2.5-free > nemotron-3-ultra-free > north-mini-code-free






生成整个仓库的整体架构图, 以及各个子 AGENT 的架构图. 



1. 你已经实现了好多次, 但是我都不满意, 请参考 diagrams/kde-overview.html 的样式, 这个是我最满意的样式和架构
2. HTML 格式即可. 保存在 diagrams 目录下. 






参考 diagrams/kde-overview.html 生成整个仓库的动态架构图. 输出 GIF、PNG 和 Excalidraw. 放到 diagrams 目录下
我 diagrams 目录下的各个图标有点乱, 请帮我创建子目录, 按照各 agent 划分子目录







《AI 时代的编码新范式》. 全系列共 9 篇, 基于 43 篇行业文献、学术论文与一线实践报告, 探讨 Spec-Driven Development 如何在 AI Agent 时代从边缘实践变为工程的基础设施. 每篇可独立阅读. 
[2026/07/02, SagaSu @sujingshen, #01-Vibe Coding 的尽头是规划先行](https://www.sagasu.art/p/vibe-coding-end-is-planning-first)









1. 详细分析下论文 https://arxiv.org/abs/2601.17063
2. 论文下载到 ./arxiv/2601.17063/paper
2. 将分析结果保存在 ./arxiv/2601.17063/paper-analyzer
3. 按照 storytelling, academic, concise风格分别输出
4. 需要配图, 建议优先使用论文中的图, 并结合配图进行分析讲解



1. 详细分析下论文 https://www.computer.org/csdl/proceedings-article/isca/2026/506500a936/2iG0TBZ0Yh2
2. 论文下载到 ./doi/10.1109_ISCA66397.2026.00075/paper
2. 将分析结果保存在 ./doi/10.1109_ISCA66397.2026.00075/paper-analyzer
3. 按照 storytelling, academic, concise风格分别输出
4. 需要配图, 建议优先使用论文中的图, 并结合配图进行分解


当前仓库 Flash-MoE: Running a 397B Parameter Model on a Laptop 支持在端侧运行 397B 的大模型。
其中有代码实现和相关论文，以及文档。
阅读 docs 下的文档和 paper 下的论文，结合代码详细分析下，Flash-MoE 是怎么搞定大 397B 的大模型入端的

公众号中有很多 [Understanding LLMs from Scratch Using Middle School Math](https://towardsdatascience.com/understanding-llms-from-scratch-using-middle-school-math-e602d27ec876) 的分析和翻译。
1. 查找下所有相关相关的文章和技术分享，将文章保存到 wechat/Understanding-LLMs。标题不限于：初中数学/中学数学 理解 LLM 之类的。
2. 我自己也列了一些文章，这些文档如果没在搜索的列表中的话，帮我加进去

3. 分析下这些文章，看下哪个文章讲的最好。


当前目录下有 omlx 和 mlx 的源代码。
1. 帮忙分析下 omlx 在 KVCache 以及长序列下有没有什么优化
2. 将分析结果保存到 docs 目录下



分别生成 MLX 和 oMLX 格式



[`yizhiyanhua-ai/fireworks-tech-graph`](https://github.com/yizhiyanhua-ai/fireworks-tech-graph)
[`architecture-diagram`](https://skills.sh/cocoon-ai/architecture-diagram-generator/architecture-diagram)

当前目录下有 omlx 和 mlx 的源代码。
1. 整体架构分析
1.1. 分析 mlx 和 oMLX  的源代码，分别输出 mlx 和 oMLX 的设计架构，保存到 docs 目录下
1.2 使用 architecture-diagram skills 分别生成 mlx 和 oMLX 的架构图，保存成 html 格式，保存到 diagrams  目录下即可
2. 再重点分析下两个框架在量化/KVCache/长序列等方向有没有什么优化
2.1 针对两个框架分别输出详细的分析报告，保存到 docs 目录下。
2.2 再分析下两个模块在这些方向上如何协作，结合前面的几份报告和架构图，输出一份整体的分析报告，保存到 docs 目录下


当前目录下 colibri 的源代码仓库
详细分析这个仓库的实现。
分析下当前仓库怎么支撑大模型入端的
分析结果保存到 ~/Work/GitHub/Knowledge/wechat/Flash-MoE/JustVugg_colibri.md


[2025/11/12, 微信公众号--林间有风, 白话Transformer（上）](https://mp.weixin.qq.com/s/vqvr-1YVn9mIW2ZOdvfTDQ)


[2026/01/08, 微信公众号--戴戴向前冲, 如何只用初中数学讲清楚大语言的原理](https://mp.weixin.qq.com/s/o30u71WGGgLvVIOvs-9hIw)
[2024/11/30, 微信公众号--中国指挥与控制学会, CICC科普栏目｜用初中数学理解LLM工作原理](https://mp.weixin.qq.com/s/8BkNM2F0SYDJ4HPyk-ulHA)
[2025/03/20, 微信公众号--数科纵横, 《使用中学数学从头开始理解LLM》](https://mp.weixin.qq.com/s/HjgOPRkbVFVBHJ4YKHzB9A)
[2024/01/01, 微信公众号--AINLPer, 揭秘 Transformer 的数学原理](https://mp.weixin.qq.com/s/qVZqwpD7ngdSX-UzIj68hw)
[2025/01/05, 微信公众号--林锵锵, 使用初中数学从零开始理解 LLMs](https://mp.weixin.qq.com/s/OEfhumiSYgLP5Tlmni-Rfw)
[2025/09/05, 微信公众号--Ai学习的老章, 用初中数学从零理解大模型](https://mp.weixin.qq.com/s/PMJX7w_4-EPAjPUo9h0wHQ)
[2026/05/04, 微信公众号--AI大模型应用实践, 中学生就能看懂：从零开始理解LLM内部原理【十四，大结局】｜理解 Transformer 架构](https://mp.weixin.qq.com/s/ixtL65L50QPmtaxRQ57drA)
[2025/09/24, 微信公众号--IRR 实验室, 如何用中学知识理解大模型技术原理](https://mp.weixin.qq.com/s/QyotH4Fw3Ybl72-eGSBnfA)
[2024/11/30, 微信公众号--数学中国, 用初中数学理解LLM工作原理](https://mp.weixin.qq.com/s/vofXHc5UsM2izj_J9CJ16g)
[2025/01/08, 微信公众号--丁师兄大模型, LLM工作原理，很直观很好懂!](https://mp.weixin.qq.com/s/iIZF58iFHXqU15n8_4nEOg)
[2024/12/20, 微信公众号--数据派THU, 独家 | 用初中数学从零开始理解大语言模型(下)](https://mp.weixin.qq.com/s/Jf9TkeXgc5JpVelmvvNZTg)










[Claude Code橙皮书：AI编程实战](https://leshu8.com/bookDetail/6a553b30256fc04cfd3454ff)
[全解深度学习——九大核心算法](https://leshu8.com/bookDetail/6a0f756ff0265f5ccb2b614d)
[CPU自制入门](https://leshu8.com/bookDetail/67c40697c0fd5e1542814e51)
[x86汇编与逆向工程：软件破解与防护的艺术](https://leshu8.com/bookDetail/67a7b3167048e060f362eea1)
[图解算法](https://leshu8.com/bookDetail/678638f05ddc0d9d8321d1f2)
[机器学习算法与实践](https://leshu8.com/bookDetail/6a3ed5ebd27a8a755d052cb9)
[Happy-LLM：从零开始构建大模型](https://leshu8.com/bookDetail/6a31758ce257020abe55fa35)
[Transformer模型开发从0到1——原理深入与项目实践, 李瑞涛 著](https://leshu8.com/bookDetail/6a0f748af0265f5ccb2b6141)
[百面大模型](https://leshu8.com/bookDetail/69bef6daa5e0aa6794cc6dbc)
[Vibe Coding：AI编程时代的认知重构](https://leshu8.com/bookDetail/69bef5f8a5e0aa6794cc6dae)
[AI知识库：个人与企业的智慧玩法](https://leshu8.com/bookDetail/69b55b4a2e19dd42a86757db)
[ChatGPT的基本原理与核心算法](https://leshu8.com/bookDetail/68448cd18c9df63d5479b75c)
[计算机系统：从应用程序到底层实现](https://leshu8.com/bookDetail/682677e6e5855184fcc82baf)
[深入浅出人工智能：原理、技术与应用](https://leshu8.com/bookDetail/68239b99a0a9db7b3cd2103b)
[神探狗狗 12：AI之乱](https://leshu8.com/bookDetail/6822b24d15733ea2d9b8c94b)

[奇思妙想：C++青少年趣味编程100例（视频教学版）](https://leshu8.com/bookDetail/68140731b7370142234e4225)
[人工智能编程实践（C++编程5级）](https://leshu8.com/bookDetail/67b13053ad79cbc5f79a7c40)

[AI驱动下的量化策略构建（微课视频版）](https://leshu8.com/bookDetail/67c8f7080a61c2c80f1001c2)
[AI助理：用ChatGPT轻松搞定工作](https://leshu8.com/bookDetail/67caa3cfe16c42fc401527a8)
[深度学习的数学](https://leshu8.com/bookDetail/67c5066caea13c95690dedf2)
[大语言模型：基础与前沿](https://leshu8.com/bookDetail/679ff66729ec665fe5b85866)
[零基础学机器学习](https://leshu8.com/bookDetail/679b25bb79f495b8c731944d)
[程序员的三门课：技术精进、架构修炼、管理探秘](https://leshu8.com/bookDetail/67925f1cda9e0d9e1ec464c1)
[漫画图解人工智能](https://leshu8.com/bookDetail/679175b400a38be1f546dfbb)
[大模型入门：技术原理与实战应用](https://leshu8.com/bookDetail/6790cdd61b457309fb4176dc)
[如何高效向GPT提问](https://leshu8.com/bookDetail/676dbcd6794bab40a55b8352)
[大语言模型：原理、应用与优化](https://leshu8.com/bookDetail/6761fe255a0436c4075b59e0)
[GPT图解：大模型是怎样构建的](https://leshu8.com/bookDetail/674e3afa61a867158db271b8)
[通识AI：人工智能基础概念与应用](https://leshu8.com/bookDetail/67455ce1b5ae8b5c3e00c163)
[一本书读懂大模型：技术创新、商业应用与产业变革](https://leshu8.com/bookDetail/673e4a91d46f08b6c2693d38)\
[大型语言模型实战指南：应用实践与场景落地](https://leshu8.com/bookDetail/67350cfce367121fda7fda83)
[CPU眼里的C/C++[Plus版]](https://leshu8.com/bookDetail/6a7251bea16f506259f19b33)
[青少年的AI学习力](https://leshu8.com/bookDetail/69fa2db3f0265f5ccb292a30)
[动手学AI：孩子的AI魔法书](https://leshu8.com/bookDetail/6a22c5c47374f706c6fdad7c)






详细对比下 opencode go 和 command goal 两个套餐
opencode go: https://opencode.ai/docs/zh-cn/go
command goal: https://commandcode.ai/docs/plans/goat





Understanding_Attention_Handbook_01.pdf  Understanding_Attention_Handbook_01_zh-CN.pdf 分别是 Understanding_Attention 的英文版本和中文版本
1. 帮我做一个中英文对照的版本，最好是一段英文下面配套一段中文的版本，最后保存成 epub 和 pdf 格式
2. 对照的时候注意下，完整的大块的公式不需要中英文对照。因为这些是不会翻译成中文的，对照之后，公式会显示两遍。



其中文字中的公式部门没有处理好
Take query row qi and key row kj. Their dot product is
取查询行 qi 和键行 kj，它们的点积为：
The complete matrix multiplication QK
computes every query-key dot product in parallel:
完整的矩阵乘法 QKT 会并行计算所有查询与键的点积

q_i 和 k_j i j 都是下标。然后 QKT T 是上标。这些公式格式不对


我对中英文带公式的时候的处理依旧不满意

The paper gives a specific motivation. Suppose the components of q and k are independent random
variables with mean zero and variance one. Then q⊤k = X dk qrkr
论文给出了一个具体动机。假设 q 和 k 的分量是相互独立、均值为 0、方差为 1 的随机变量，那么：
has mean zero and variance dk. As dk grows, unscaled dot products can therefore grow in
magnitude. Large- magnitude logits can push softmax into a saturated regime with very small
gradients. Dividing by √dk normalizes this illustrative variance from dk to approximately one. [3] A
short derivation under those assumptions is
的均值为 0，方差为 dk。因此，随着 dk 增大，未经缩放的点积可能变得更大。绝对值较大的 logits 可能把 softmax
推入梯度很小的饱和区间。除以
 √dk，可以将这个示例中的方差从
在上述假设下，简短的推导为：


这里还是公式没处理好，这里明明是一句话但是被你因为 q⊤k = X dk qrkr 这个公式，给阶段成了两句话，然后中文中这个公式直接消失了q⊤k = X dk qrkr

类似这种地方还有很多，好好改下，检查下


opencode 配置文件 ~/.config/opencode/opencode.json 中配置 themes 为 tokyonight 为啥没生效。帮我解决下



https://commandcode.ai/docs/plans/goat

https://opencode.ai/docs/zh-cn/go








我想同时安装 workbuddy 和 workbuddy 国际版，分别在如下两个目录下。
D:\Software\WorkBuddy\WorkBuddyCN
D:\Software\WorkBuddy\WorkBuddyAI
但是现在安装完两个软件之后，一打开 workbuddy，他直接开始更新，更新成了 WorkBuddy 国际版，并且感觉上像是在自己安装目录 D:\Software\WorkBuddy\WorkBuddyAI 下安装了  workbuddy 国际版
帮我确认下怎么回事。我当前已经把两个人软件全部卸载，当前只安装了 workbuddy 国际版，没敢继续去安装 workbuddy

两个安装包位于




我想同时安装 workbuddy，到如下目录下
D:\Software\WorkBuddy\WorkBuddyCN
但是现在安装完，一打开 workbuddy，他直接开始更新，更新成了 WorkBuddy 国际版，并且感觉上像是在自己安装目录 D:\Software\WorkBuddy\WorkBuddyCN 下安装了  workbuddy 国际版，两个版本共存，但是只有国际版能正常打开。
帮我确认下怎么回事。按道理 workbuddy 不应该升级成国际版的

分析下这个仓库的整体架构以及实现，特别是 KIMI K3 是怎么做到 2.78T 的参数量，在端侧 8G 内存的设备上跑起来的，分析结果总结到 docs 目录