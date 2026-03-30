---

title: AI
date: 2021-02-15 00:32
author: gatieme
tags:
    - linux
    - tools
categories:
        - 技术积累
thumbnail:
blogexcerpt: 虚拟化 & KVM 子系统

---

<br>

本作品采用 <a rel="license"href="http://creativecommons.org/licenses/by-nc-sa/4.0/"> 知识共享署名 - 非商业性使用 - 相同方式共享 4.0 国际许可协议 </a> 进行许可, 转载请注明出处, 谢谢合作

<a rel="license"href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt=" 知识共享许可协议 "style="border-width:0"src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png"/></a>

因本人技术水平和知识面有限, 内容如有纰漏或者需要修正的地方, 欢迎大家指正, 鄙人在此谢谢啦

** 转载请务必注明出处, 谢谢, 不胜感激 **

<br>

| 日期 | 作者 | GitHub| CSDN | BLOG |
| ------- |:-------:|:-------:|:-------:|:-------:|
| 2021-02-15 | [成坚 - gatieme](https://kernel.blog.csdn.net) | [`AderXCoding/system/tools/fzf`](https://github.com/gatieme/AderXCoding/tree/master/system/tools/fzf) | [使用模糊搜索神器 FZF 来提升办公体验](https://blog.csdn.net/gatieme/article/details/113828826) | [Using FZF to Improve Productivit](https://oskernellab.com/2021/02/15/2021/0215-0001-Using_FZF_to_Improve_Productivity)|


<br>

2   ** AI OS **
=====================




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*　重要功能和时间点　-*-*-*-*-*-*-*-*-*-*-*-*-*-*-***





下文将按此目录分析 Linux 内核中 MM 的重要功能和引入版本:




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* 正文 -*-*-*-*-*-*-*-*-*-*-*-*-*-*-***


# 1 场景
-------

华擎的 AI QuickSet WSL 旨在通过 WSL 下的自动 ROCm 设置以及安装/配置流行的 AI 软件包以在 WSL+ROCm 下加速执行, 从而轻松"在 Windows 上运行 Linux AI 应用程序". 参见 [phoronix, 2025/09/15, ASRock AI Quickset WSL Aims To Make It Easier Running ROCm + AI Linux Apps On Windows](https://www.phoronix.com/news/ASRock-AI-QuickSet-WSL)


[知乎--机器之心--跟不上、读不完？上万篇顶会论文，这个工具一键分析](https://zhuanlan.zhihu.com/p/1968619277394346770)

## 1.1 AI4OS
-------


[Learning-directed operating system (LDOS)](https://ldos.utexas.edu) 是德克萨斯大学奥斯汀分校的一个研究项目, 专注于开发基于机器学习的下一代作系统, 以提高效率和性能. 该项目旨在通过将机器学习驱动的方法集成到核心系统组件中, 通过"从头开始的范式"彻底改变作系统设计. 这包括优化资源分配、调度和拥塞控制机制，以动态适应不同的工作负载.
2025 年, LDOS 团队发表了两篇论文: "Canopy: Property-Driven Learning for Congestion Control" 探索了使用机器学习根据网络属性动态调整拥塞控制策略, "Large Language Models as Realistic Microservice Trace Generators" 应用 LLM 生成用于测试微服务的真实流量跟踪, 提高模拟模型的准确性.


| 日期 | 概要 | 论文 / 链接 | 团队 | 描述 |
|:---:|:----:|----------:|:----:|:----:|
|  2024/07  | 自动化故障定位、修复和分析 | [A Unified Debugging Approach via LLM-Based Multi-Agent Synergy](https://arxiv.org/abs/2404.17153) | NA | 大型语言模型 (LLM) 在自动调试方面显示出了巨大潜力. 然而, 我们发现传统和基于 LLM 的调试工具面临着三个挑战: 1) 故障定位的上游不完美会影响下游的修复; 2) 处理复杂逻辑错误的不足; 3) 忽略程序上下文. 作者提出了第一个自动化的、统一的调试框架——FixAgent, 通过 LLM 代理协同作用. FixAgent 可以执行端到端的故障定位、修复和分析. LLM 可以从人类开发人员在调试中认可的通用软件工程原则中受益, 如 rubber duck debugging, 从而更好地理解程序功能和逻辑错误. 因此, 我们创建了三个受 rubber duck debugging 启发的设计来解决这些挑战. 它们是代理专业化和协同作用、关键变量跟踪和程序上下文理解, 这些要求 LLM 提供明确的解释, 并迫使它们关注关键的程序逻辑信息. 在广泛使用的 QuixBugs 数据集上的实验表明, FixAgent 正确修复了 80 个中的 79 个错误, 其中有 9 个以前从未被修复过. 即使没有故障位置信息和少于 0.6% 的采样时间, 它也比 CodeFlaws 上表现最佳的修复工具更可信地修补了 1.9 倍的缺陷. 平均而言, FixAgent 相对于使用不同 LLM 的基础模型平均增加了约 20% 的可信和正确的修复, 显示出我们设计的有效性. 此外, FixAgent 的正确率达到了惊人的 97.26%, 表明 FixAgent 有可能克服现有方法的过度拟合问题. |
| 2024/07 | AI 辅助 Linux 补丁测试  | [Testing AI-enhanced reviews for Linux patches](https://lwn.net/Articles/987319) | NA | 在 2024 年 7 月的 Netdev 0x18 大会上的一次演讲中, Brandeburg 概述了一个使用机器学习来审查包含发送到 netdev 邮件列表的补丁的电子邮件的实验. 大型语言模型(LLMs) 不会很快取代人工审阅者, 但它们可能是一个有用的补充, 可以帮助人类专注于更深入的审阅, 而不是简单的规则违规行为. 参见 [AI Enhanced Reviews for Linux Networking](https://netdevconf.info/0x18/docs/netdev-0x18-paper26-talk-slides/netdev_0x18_AI_Reviews.pdf) |
| 2025/09 || AI 辅助调度器 | [Towards Agentic OS: An LLM Agent Framework for Linux Schedulers](https://arxiv.org/abs/2509.01245) | 加州大学圣塔克鲁兹分校、上海科技大学等 | 内核的调度策略无法理解特定于应用程序的需求, 从而导致性能不佳. 实现介绍了 SchedCP 框架, 它使完全自主的 LLM 代理能够在没有人工参与的情况下安全高效地优化 Linux 调度器. 核心思想是, 挑战不仅在于应用更好的 LLM, 还在于构建一个解耦的控制平面, 将 AI 的语义推理角色("优化什么")与系统的执行角色("如何观察和行动")分开. SchedCP 作为模型上下文协议(MCP) 服务器实现, 提供了一个稳定的接口, 其中包含三个关键服务: 工作负载分析引擎、不断发展的调度器策略存储库和执行验证器, 用于验证所有 AI 生成的代码, 并在部署前通过静态和动态分析进行配置. sched-agent 是一个多 AGENT 系统, 可以自主分析工作负载, 合成自定义 eBPF 调度策略, 并通过 `sched_ext` 部署它们. 实现评估, 与朴素的代理方法相比, SchedCP 实现了高达 1.79 倍的性能提升和 13 倍的成本降低, 同时保持了较高的成功率. 通过弥合语义差距, SchedCP 使专家级系统优化民主化, 并代表着朝着创建真正自我优化、应用程序感知的作系统迈出的一步. 开源代码 [eunomia-bpf/schedcp](https://github.com/eunomia-bpf/schedCP). |
| 2025/07 |  AI 辅助调度 Load Balancing  | [LWN 2025/07/01, Improved load balancing with machine learning](https://lwn.net/Articles/1027096 | Free5GC | Ching-Chun("Jim") Huang 展示了其将 (本地) 机器学习应用于在复杂系统上调度器负载均衡的工作成果, [Improve Load Balancing with Machine Learning Techniques based on sched_ext Framework](https://static.sched.com/hosted_files/ossna2025/d2/Improve-Load-Balancing-With-Machine-Learning-Techniques-based-on-sched_ext.pdf). Free5GC 开发人员研究通过机器学习来改进调度器的负载均衡. 在此类系统上进行调度需要考虑许多输入维度; 此外, 调度程序还必须考虑每个任务的优先级、其 CPU 要求、到目前为止的虚拟运行时间以及最近的 CPU 使用模式. 必须考虑每个 CPU 上的负载, 以及 NUMA 距离、缓存共享和工作频率. 当然, 还有特定于工作负载的因素. 基于 scx_rusty 来尝试考虑所有这些参数并决定何时应该将任务从一个 CPU 移动到另一个 CPU. 它最初以数据收集模式运行, 查看迁移决策及其结果; 然后, 这些决策用于训练模型(在用户空间中), 该模型随后存储在 BPF 映射中. 然后, 调度程序可以在内核内使用此模型来做出负载平衡决策. 这些决策的结果会不断被测量并报告回用户空间, 从而随着时间的推移更新模型. 在使用最重要的内核编译基准测试的测试中, 该调度器的编译时间比 EEVDF 调度器缩短了 10%, 任务迁移的数量减少了 77%. Huang 总结了机器学习在这种情况下起作用的原因: 在这种复杂的环境中进行调度是一个模式识别问题, 而神经网络擅长这项任务. 调度程序能够平衡相互竞争的目标, 并自动针对新的架构和工作负载进行自我重新训练. 调度程序能够为每个迁移决策考虑 15 个单独的参数, 并根据结果调整其模型. [2025 Open Source Summit North America](https://events.linuxfoundation.org/open-source-summit-north-america), 参见 LWN 报道 [LWN 2025/07/01, Improved load balancing with machine learning](https://lwn.net/Articles/1027096), 代码 [scx_rusty](https://github.com/vax-r/scx/tree/scx_rusty_MLLB). |
| 2025/03 | AI 辅助 CPU/GPU 调频 | [An Intelligent Scheduling Approach on Mobile OS for Optimizing UI Smoothness and Power](https://dl.acm.org/doi/full/10.1145/3674910) | 提出了 MobiRL 一种基于强化学习的调度器, 用于智能地调整移动系统中的 CPU/GPU 频率, 以准确满足用户需求. MobiRL监测移动系统状态, 并通过执行 CPU/GPU 频率调整操作自主学习以优化用户界面的流畅度和功耗. 在最新交付的智能手机上的实验结果表明, MobiRL 在真实设备上的表现优于广泛使用的商业调度器——分别降低了 4.1% 的掉帧率和 42.8% 的功耗. 此外, 与使用 Q 学习进行 CPU 频率调度的研究相比, MobiRL 实现了最高 2.5% 的掉帧率降低, 并分别减少了 32.6% 的功耗. |
| 2025/03/25 | AI 辅助 CPU/GPU/DDR 调频 | [CRAVE: Analyzing Cross-Resource Interaction to Improve Energy Efficiency in Systems-on-Chip](https://dl.acm.org/doi/10.1145/3689031.3717498) | 提出了 CRAVE, 它利用学习到的设计特性来控制动态电压和频率调节. 在设计阶段, CRAVE 通过在三个主要移动系统组件(CPU内核、GPU和内存)的频率设置多元空间中采样, 为系统级芯片(SoC)确定最优的 DVFS 设置. 在运行时, CRAVE 以类似于当今操作系统内核中内置的现有简单调速器的方式监控资源利用率, 然后应用之前学习到的最优设置. 在两个真实的移动平台上实现了CRAVE: ODROID-XU4 和 NVIDIA Jetson TX2. 与最佳的内置 Linux 调速器相比, CRAVE 在 TX2 上将性能提高了20%, 同时能耗降低了 16%, 在 XU4 上也取得了类似的提升. 此外, 与最先进的应用驱动调速器相比, CRAVE 也表现出了一定的优势, 性能提高了 16%, 能耗节省了10%. |
| 2025/07/11 | AI 生成操作系统交互界面 | | [NeuralOS: Towards Simulating Operating Systems via Neural Generative Models](https://arxiv.org/abs/2507.08800) | 滑铁卢大学 | 名为 NeuralOS 的创新性神经框架. 其核心目标是利用深度生成模型, 完全模拟一个操作系统的图形用户界面(GUI). 不同于传统依赖预编程内核和应用程序的操作系统, NeuralOS 通过一个深度神经网络, 直接根据用户的输入(如鼠标移动、点击和键盘事件)来预测并生成屏幕的下一帧图像.<br>核心贡献可以概括为以下几点:<br>1. 提出新范式: 首次尝试将整个操作系统 GUI 交互过程建模为一个端到端的生成问题, 为实现完全自适应、个性化的未来人机交互界面提供了概念验证(Proof-of-Concept).<br>2. 设计创新架构:  提出了一种受传统操作系统启发的模块化架构, 该架构由一个负责追踪系统状态的循环神经网络(RNN)"内核"和一个负责生成屏幕图像的扩散模型"渲染器"组成, 有效处理了动态交互中的长期依赖和高保真视觉生成.<br>3. 开发有效训练策略:  设计了一套复杂的多阶段训练流程, 包括 RNN 预训练、联合训练、计划采样和课程学习等, 成功解决了直接训练生成式交互模型时遇到的梯度消失、渲染器忽略控制信号和误差累积等关键挑战.<br>4. 构建大规模数据集: 通过结合 AI Agent 的智能探索和随机探索, 建立了一个大规模、多样化的 Ubuntu XFCE 桌面交互数据集, 为训练此类复杂的交互模型奠定了基础. 参见 [知乎--周舒畅--远程桌面蒸馏成RNN+扩散模型：NeuralOS: Towards Simulating Operating Systems via Neural Generative Models](https://zhuanlan.zhihu.com/p/1928592949131863847) |
| 2025/10/21 | AI 实现补丁管理，实现补丁管理效率倍级提升| openEuler | [基于 openEuler Intelligence打造补丁管理智能体，实现补丁管理效率倍级提升](https://mp.weixin.qq.com/s/SwRTuH6iDISfD3QucNeOzw) | openEuler Intelligence 服务引擎, 实现操作系统级的 Agent 智能体与 MCP 工具管理, 提供全局、高效、低噪的智能服务框架. |
| 2026/01/11 | 利用大语言模型(LLM) 辅助解决 Linux 内核中的 Git 合并冲突 | [LLMinus: LLM-Assisted Merge Conflict Resolution](https://lore.kernel.org/all/20260111212915.195056-1-sashal@kernel.org) | Sasha Levin <sashal@kernel.org> | 引入名为 **LLMinus** 的工具, 利用大语言模型( LLM) 辅助解决 Linux 内核中的 Git 合并冲突. LLMinus 通过构建历史冲突解决案例的数据库, 使用语义嵌入( 基于 BGE-small 模型) 查找相似冲突, 并为 LLM 构建包含上下文的提示以辅助解决当前冲突.<br>工具支持 `learn`、`vectorize`、`find`、`resolve`、`pull` 等命令, 可与任意支持标准输入的 LLM 配合使用. 新版本改进包括: 支持自适应 RAG 缩减的 token 限制、通过构建测试检测语义冲突, 以及更新 HTTP 客户端.<br>LLMinus 正在通过子系统集成分支进行实际测试, 相关仓库可供参考.该工具旨在辅助而非替代现有合并流程.  | v2 ☐☑✓ | [2026/01/11, LORE v2, 0/7](https://lore.kernel.org/all/20260111212915.195056-1-sashal@kernel.org) |
| 2025/02/25 | 通过 AI 生成 eBPF 程序 | [Kgent: Kernel Extensions Large Language Model Agent](https://dl.acm.org/doi/10.1145/3672197.3673434) | NA  | Kgent 简化了传统上复杂的 eBPF 程序编写过程. 通过将用户提示语翻译成 eBPF 代码, 消除了对深厚作系统内核知识的需求. 该工具结合了程序理解、符号执行和反馈循环, 确保合成程序准确且符合用户意ds图. 参见 [Simplifying Kernel Programming: The LLM-Powered eBPF Tool](https://eunomia.dev/blog/2024/07/11/simplifying-kernel-programming-the-llm-powered-ebpf-tool) |
| [](https://github.com/xlang-ai/OSWorld-G) |


## 1.2 OS4AI
-------

| 日期 | 概要 | 论文 / 链接 | 团队 | 描述 |
|:---:|:----:|----------:|:----:|:----:|
| 2025/09 | AgentOS | [LLM as OS, Agents as Apps: Envisioning AIOS, Agents and the AIOS-Agent Ecosystem](https://arxiv.org/abs/2312.03815) | NA | 本文设想了一个革命性的 AIOS-Agent 生态系统, 其中大型语言模型 (LLM) 充当 (人工) 智能操作系统 (IOS, 或 AIOS)——一个 "有灵魂" 的操作系统. 在此基础上, 开发了各种 LLM 基于 AI 代理的应用程序 (Agents, 或 AAP), 丰富了 AIOS-Agent 生态系统, 标志着传统 OS-APP 生态系统的范式转变. 作者设想 LLM 其影响将不仅限于人工智能应用层面, 相反, 它将彻底改变计算机系统、架构、软件和编程语言的设计和实现, 其特点是几个主要概念: LLM 操作系统 (系统级)、代理即应用程序 (应用程序级)、自然语言作为编程接口 (用户级) 和工具即设备 / 库 (硬件 / 中间件级). 我们首先介绍传统操作系统的架构. 然后, 我们通过 "LLMas OS(LLMOS)" 正式化 AIOS 的概念框架, 将 AIOS 与传统操作系统进行类比: LLM 将上下文窗口比作操作系统内核, 将上下文窗口比作内存, 将外部存储比作文件系统, 将硬件工具比作外围设备, 将软件工具比作编程库, 将用户提示比作用户命令. 随后, 我们引入了新的 AIOS-Agent 生态系统, 用户可以使用自然语言轻松编程 Agent 应用程序 (AAP), 使软件开发民主化, 这与传统的 OS-APP 生态系统不同. 在此之后, 我们将探索代理应用程序的多样化范围. 我们深入研究了单智能体和多智能体系统, 以及人机交互. 最后, 借鉴传统 OS-APP 生态的洞察, 提出了 AIOS-Agent 生态演进的路线图.  该路线图旨在指导未来的研究和开发, 建议 AIOS 及其代理应用程序的系统性进展. |
| 2025/08 | 用于可扩展的 MoE(混合专家)LLM 推理的高性能框架 | [Expert Kit: A Distributed, Expert-Centric Framework for MoE LLM Inference](https://gitee.com/openeuler/expert-kit) | openEuler | openEuler 提供的 专家工具包 (EK) 是一个用于可扩展的 MoE(混合专家)LLM 推理的高性能框架. EK 的愿景是在商用网络(例如 PCIe、TCP、RDMA) 上的异构硬件 (例如 CPU 和 GPU) 上提供专家并行性 (EP)的高效基础, 从而实现轻松部署和细粒度的专家级扩展.  |
| 2025/09 | decode 阶段自适应选择 CPU 核 | [MNN-AECS: Energy Optimization for LLM Decoding on Mobile Devices via Adaptive Core Selection](https://arxiv.org/abs/2506.19884) | NA | 分析显示, 受内存限制的 LLM 解码阶段在能耗中占主导地位, 然而, 大多数现有工作都集中在加速预填充阶段, 忽视了能效问题. 引入了自适应能效核心选择(AECS), 并将其集成到 MNN 中, 创建了能效版本 MNN-AECS, 这是首个无需 root 权限或操作系统修改即可实现能效 LLM 解码的引擎级系统解决方案. MNN-AECS 旨在通过动态选择低功耗 CPU 核, 在保持解码速度在可接受的减速阈值内的同时, 降低 LLM 解码的能耗. 作者在 5 款安卓设备和 2 款 iOS 设备上, 对 5 种不同规模的流行 LLM 进行了 MNN-AECS 评估. 与原始 MNN 相比, MNN-AECS 在所有 7 款设备和 4 个数据集上的平均能耗降低了 23%, 且速度没有减慢. 与其他引擎(包括 llama.cpp、executorch、mllm 和 MediaPipe)相比, MNN-AECS 平均能节省 39% 至 78% 的能耗, 并实现 12% 至 363% 的速度提升. |
| 2025/10 | Xsched 异构调度 | [支持 NPU 算力切分](https://gitee.com/openeuler/kernel/issues/IC5EHB) | openEuler | 基于 ARM64 + 910B 实现的 NPU 卡支持算力时分抢占特性, 支持 NPU share、带宽管控等特性.<br>算力时分抢占: 通过 AI 任务的算力抽象, 构建异构时分调度机制, 实现异构多任务毫秒级抢占, 多推理单卡调度抢占小于 10 毫秒;<br>
算力带宽管控: 基于算力带宽标准化语义, 构建任务级的算力管控分配机制, 实现算力隔离与共享, 提升吞吐. |
| 2025/10 | GPU eBPF profiling | NA | NA | |[知乎--云微--GPU可观测性差距：为什么我们需要GPU上的eBPF](https://zhuanlan.zhihu.com/p/1962141761364301211), [迈向可编程观测：在GPU Kernel中构建类eBPF风格的性能探针](https://developer.aliyun.com/article/1681315), [Neutrino: Fine-grained GPU Kernel Profiling via Programmable Probing, OSDI '25](https://github.com/open-neutrino/neutrino) |


## 1.3 AI Tools
-------

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2026/01/11 | Sasha Levin <sashal@kernel.org> | [LLMinus: LLM-Assisted Merge Conflict Resolution](https://lore.kernel.org/all/20260111212915.195056-1-sashal@kernel.org) | 提出了一项名为 **LLMinus** 的新工具, 旨在利用大语言模型(LLM) 辅助解决 Linux 内核中的 Git 合并冲突. LLMinus 通过分析 Git 历史中的冲突解决案例, 构建可搜索的语义数据库, 并在当前冲突发生时生成带有上下文的提示, 引导 LLM 进行冲突解决.<br>主要功能包括: 冲突学习、语义向量化、相似冲突查找、LLM 辅助解决及自动拉取请求合并. 新版本改进包括: 支持最大 token 限制以避免提示溢出、通过构建测试检测语义冲突、使用新版 HTTP 客户端.<br>该工具已在多个内核子系统集成分支中进行实际测试, 但目前仍为实验性质, 不替代现有流程. 代码已以补丁形式提交,供社区审阅. [phoronix, 2026/01/12, LLMinus Working On AI/LLM-Powered Merge Conflict Resolution For The Linux Kernel](https://www.phoronix.com/news/LLMinus-RFC-v2) | v2 ☐☑✓ | [2026/01/11, LORE v2, 0/7](https://lore.kernel.org/all/20260111212915.195056-1-sashal@kernel.org) |
| 2021 | SBU-FSL | [kernel-ml](https://github.com/sbu-fsl/kernel-ml) | 由纽约州立大学石溪分校(SBU)文件系统与存储实验室(FSL)开发的机器学习框架, 用于优化操作系统和存储系统的性能, 该项目通过 ML 技术自适应地调整存储系统的参数(如预读值), 识别工作负载模式, 使系统能够动态适应复杂多变的应用场景. 相比较手动调优, KML 内存占用小, 开销低. |


<br>

*   本作品 / 博文 ([AderStep - 紫夜阑珊 - 青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚 (gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用 <a rel="license"href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt=" 知识共享许可协议 "style="border-width:0"src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png"/></a><a rel="license"href="http://creativecommons.org/licenses/by-nc-sa/4.0/"> 知识共享署名 - 非商业性使用 - 相同方式共享 4.0 国际许可协议 </a> 进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名 [成坚 gatieme](http://blog.csdn.net/gatieme) (包含链接: http://blog.csdn.net/gatieme), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   ** 转载请务必注明出处, 谢谢, 不胜感激 **
<br>
