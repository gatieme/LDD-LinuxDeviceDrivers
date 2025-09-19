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

## 1.1 AI4OS
-------

| 日期 | 概要 | 论文 / 链接 | 团队 | 描述 |
|:---:|:----:|----------:|:----:|:----:|
|  2024/07  | 自动化故障定位、修复和分析 | [A Unified Debugging Approach via LLM-Based Multi-Agent Synergy](https://arxiv.org/abs/2404.17153) | NA | 大型语言模型 (LLM) 在自动调试方面显示出了巨大潜力. 然而, 我们发现传统和基于 LLM 的调试工具面临着三个挑战: 1) 故障定位的上游不完美会影响下游的修复; 2) 处理复杂逻辑错误的不足; 3) 忽略程序上下文. 作者提出了第一个自动化的、统一的调试框架——FixAgent, 通过 LLM 代理协同作用. FixAgent 可以执行端到端的故障定位、修复和分析. LLM 可以从人类开发人员在调试中认可的通用软件工程原则中受益, 如 rubber duck debugging, 从而更好地理解程序功能和逻辑错误. 因此, 我们创建了三个受 rubber duck debugging 启发的设计来解决这些挑战. 它们是代理专业化和协同作用、关键变量跟踪和程序上下文理解, 这些要求 LLM 提供明确的解释, 并迫使它们关注关键的程序逻辑信息. 在广泛使用的 QuixBugs 数据集上的实验表明, FixAgent 正确修复了 80 个中的 79 个错误, 其中有 9 个以前从未被修复过. 即使没有故障位置信息和少于 0.6% 的采样时间, 它也比 CodeFlaws 上表现最佳的修复工具更可信地修补了 1.9 倍的缺陷. 平均而言, FixAgent 相对于使用不同 LLM 的基础模型平均增加了约 20% 的可信和正确的修复, 显示出我们设计的有效性. 此外, FixAgent 的正确率达到了惊人的 97.26%, 表明 FixAgent 有可能克服现有方法的过度拟合问题. |
| 2024/07 | AI 辅助 Linux 补丁测试  | [Testing AI-enhanced reviews for Linux patches](https://lwn.net/Articles/987319) | NA | 在 2024 年 7 月的 Netdev 0x18 大会上的一次演讲中, Brandeburg 概述了一个使用机器学习来审查包含发送到 netdev 邮件列表的补丁的电子邮件的实验. 大型语言模型(LLMs) 不会很快取代人工审阅者, 但它们可能是一个有用的补充, 可以帮助人类专注于更深入的审阅, 而不是简单的规则违规行为. 参见 [AI Enhanced Reviews for Linux Networking](https://netdevconf.info/0x18/docs/netdev-0x18-paper26-talk-slides/netdev_0x18_AI_Reviews.pdf) |
| 2025/09 || AI 辅助调度器 | [Towards Agentic OS: An LLM Agent Framework for Linux Schedulers](https://arxiv.org/abs/2509.01245) | 加州大学圣塔克鲁兹分校、上海科技大学等 | 内核的调度策略无法理解特定于应用程序的需求, 从而导致性能不佳. 实现介绍了 SchedCP 框架, 它使完全自主的 LLM 代理能够在没有人工参与的情况下安全高效地优化 Linux 调度器. 核心思想是, 挑战不仅在于应用更好的 LLM, 还在于构建一个解耦的控制平面, 将 AI 的语义推理角色("优化什么")与系统的执行角色("如何观察和行动")分开. SchedCP 作为模型上下文协议(MCP) 服务器实现, 提供了一个稳定的接口, 其中包含三个关键服务: 工作负载分析引擎、不断发展的调度器策略存储库和执行验证器, 用于验证所有 AI 生成的代码, 并在部署前通过静态和动态分析进行配置. sched-agent 是一个多 AGENT 系统, 可以自主分析工作负载, 合成自定义 eBPF 调度策略, 并通过 `sched_ext` 部署它们. 实现评估, 与朴素的代理方法相比, SchedCP 实现了高达 1.79 倍的性能提升和 13 倍的成本降低, 同时保持了较高的成功率. 通过弥合语义差距, SchedCP 使专家级系统优化民主化, 并代表着朝着创建真正自我优化、应用程序感知的作系统迈出的一步. 开源代码 [eunomia-bpf/schedcp](https://github.com/eunomia-bpf/schedCP). |
| 2025/07 |  AI 辅助调度 Load Balancing  | [LWN 2025/07/01, Improved load balancing with machine learning](https://lwn.net/Articles/1027096 | Free5GC | Ching-Chun("Jim") Huang 展示了其将 (本地) 机器学习应用于在复杂系统上调度器负载均衡的工作成果, [Improve Load Balancing with Machine Learning Techniques based on sched_ext Framework](https://static.sched.com/hosted_files/ossna2025/d2/Improve-Load-Balancing-With-Machine-Learning-Techniques-based-on-sched_ext.pdf). Free5GC 开发人员研究通过机器学习来改进调度器的负载均衡. 在此类系统上进行调度需要考虑许多输入维度; 此外, 调度程序还必须考虑每个任务的优先级、其 CPU 要求、到目前为止的虚拟运行时间以及最近的 CPU 使用模式. 必须考虑每个 CPU 上的负载, 以及 NUMA 距离、缓存共享和工作频率. 当然, 还有特定于工作负载的因素. 基于 scx_rusty 来尝试考虑所有这些参数并决定何时应该将任务从一个 CPU 移动到另一个 CPU. 它最初以数据收集模式运行, 查看迁移决策及其结果; 然后, 这些决策用于训练模型(在用户空间中), 该模型随后存储在 BPF 映射中. 然后, 调度程序可以在内核内使用此模型来做出负载平衡决策. 这些决策的结果会不断被测量并报告回用户空间, 从而随着时间的推移更新模型. 在使用最重要的内核编译基准测试的测试中, 该调度器的编译时间比 EEVDF 调度器缩短了 10%, 任务迁移的数量减少了 77%. Huang 总结了机器学习在这种情况下起作用的原因: 在这种复杂的环境中进行调度是一个模式识别问题, 而神经网络擅长这项任务. 调度程序能够平衡相互竞争的目标, 并自动针对新的架构和工作负载进行自我重新训练. 调度程序能够为每个迁移决策考虑 15 个单独的参数, 并根据结果调整其模型. [2025 Open Source Summit North America](https://events.linuxfoundation.org/open-source-summit-north-america), 参见 LWN 报道 [LWN 2025/07/01, Improved load balancing with machine learning](https://lwn.net/Articles/1027096), 代码 [scx_rusty](https://github.com/vax-r/scx/tree/scx_rusty_MLLB). |
| 2025/03 | AI 辅助 CPU/GPU 调频 | [An Intelligent Scheduling Approach on Mobile OS for Optimizing UI Smoothness and Power](https://dl.acm.org/doi/full/10.1145/3674910) | 提出了 MobiRL 一种基于强化学习的调度器, 用于智能地调整移动系统中的 CPU/GPU 频率, 以准确满足用户需求. MobiRL监测移动系统状态, 并通过执行 CPU/GPU 频率调整操作自主学习以优化用户界面的流畅度和功耗. 在最新交付的智能手机上的实验结果表明, MobiRL 在真实设备上的表现优于广泛使用的商业调度器——分别降低了 4.1% 的掉帧率和 42.8% 的功耗. 此外, 与使用 Q 学习进行 CPU 频率调度的研究相比, MobiRL 实现了最高 2.5% 的掉帧率降低, 并分别减少了 32.6% 的功耗. |
| 2025/03/25 | AI 辅助 CPU/GPU/DDR 调频 | [CRAVE: Analyzing Cross-Resource Interaction to Improve Energy Efficiency in Systems-on-Chip](https://dl.acm.org/doi/10.1145/3689031.3717498) | 提出了 CRAVE, 它利用学习到的设计特性来控制动态电压和频率调节. 在设计阶段, CRAVE 通过在三个主要移动系统组件(CPU内核、GPU和内存)的频率设置多元空间中采样, 为系统级芯片(SoC)确定最优的 DVFS 设置. 在运行时, CRAVE 以类似于当今操作系统内核中内置的现有简单调速器的方式监控资源利用率, 然后应用之前学习到的最优设置. 在两个真实的移动平台上实现了CRAVE: ODROID-XU4 和 NVIDIA Jetson TX2. 与最佳的内置 Linux 调速器相比, CRAVE 在 TX2 上将性能提高了20%, 同时能耗降低了 16%, 在 XU4 上也取得了类似的提升. 此外, 与最先进的应用驱动调速器相比, CRAVE 也表现出了一定的优势, 性能提高了 16%, 能耗节省了10%. |



## 1.2 OS4AI
-------

| 日期 | 概要 | 论文 / 链接 | 团队 | 描述 |
|:---:|:----:|----------:|:----:|:----:|
| 2025/09 | AgentOS | [LLM as OS, Agents as Apps: Envisioning AIOS, Agents and the AIOS-Agent Ecosystem](https://arxiv.org/abs/2312.03815) | NA | 本文设想了一个革命性的 AIOS-Agent 生态系统, 其中大型语言模型 (LLM) 充当 (人工) 智能操作系统 (IOS, 或 AIOS)——一个 "有灵魂" 的操作系统. 在此基础上, 开发了各种 LLM 基于 AI 代理的应用程序 (Agents, 或 AAP), 丰富了 AIOS-Agent 生态系统, 标志着传统 OS-APP 生态系统的范式转变. 作者设想 LLM 其影响将不仅限于人工智能应用层面, 相反, 它将彻底改变计算机系统、架构、软件和编程语言的设计和实现, 其特点是几个主要概念: LLM 操作系统 (系统级)、代理即应用程序 (应用程序级)、自然语言作为编程接口 (用户级) 和工具即设备 / 库 (硬件 / 中间件级). 我们首先介绍传统操作系统的架构. 然后, 我们通过 "LLMas OS(LLMOS)" 正式化 AIOS 的概念框架, 将 AIOS 与传统操作系统进行类比: LLM 将上下文窗口比作操作系统内核, 将上下文窗口比作内存, 将外部存储比作文件系统, 将硬件工具比作外围设备, 将软件工具比作编程库, 将用户提示比作用户命令. 随后, 我们引入了新的 AIOS-Agent 生态系统, 用户可以使用自然语言轻松编程 Agent 应用程序 (AAP), 使软件开发民主化, 这与传统的 OS-APP 生态系统不同. 在此之后, 我们将探索代理应用程序的多样化范围. 我们深入研究了单智能体和多智能体系统, 以及人机交互. 最后, 借鉴传统 OS-APP 生态的洞察, 提出了 AIOS-Agent 生态演进的路线图.  该路线图旨在指导未来的研究和开发, 建议 AIOS 及其代理应用程序的系统性进展. |
| 2025/08 | 用于可扩展的 MoE(混合专家)LLM 推理的高性能框架 | [Expert Kit: A Distributed, Expert-Centric Framework for MoE LLM Inference](https://gitee.com/openeuler/expert-kit) | openEuler | openEuler 提供的 专家工具包 (EK) 是一个用于可扩展的 MoE(混合专家)LLM 推理的高性能框架. EK 的愿景是在商用网络(例如 PCIe、TCP、RDMA) 上的异构硬件 (例如 CPU 和 GPU) 上提供专家并行性 (EP)的高效基础, 从而实现轻松部署和细粒度的专家级扩展.  |
| 2025/09 | decode 阶段自适应选择 CPU 核 | [MNN-AECS: Energy Optimization for LLM Decoding on Mobile Devices via Adaptive Core Selection](https://arxiv.org/abs/2506.19884) | NA | 分析显示, 受内存限制的 LLM 解码阶段在能耗中占主导地位, 然而, 大多数现有工作都集中在加速预填充阶段, 忽视了能效问题. 引入了自适应能效核心选择(AECS), 并将其集成到 MNN 中, 创建了能效版本 MNN-AECS, 这是首个无需 root 权限或操作系统修改即可实现能效 LLM 解码的引擎级系统解决方案. MNN-AECS 旨在通过动态选择低功耗 CPU 核, 在保持解码速度在可接受的减速阈值内的同时, 降低 LLM 解码的能耗. 作者在 5 款安卓设备和 2 款 iOS 设备上, 对 5 种不同规模的流行 LLM 进行了 MNN-AECS 评估. 与原始 MNN 相比, MNN-AECS 在所有 7 款设备和 4 个数据集上的平均能耗降低了 23%, 且速度没有减慢. 与其他引擎(包括 llama.cpp、executorch、mllm 和 MediaPipe)相比, MNN-AECS 平均能节省 39% 至 78% 的能耗, 并实现 12% 至 363% 的速度提升. |



# 2 模型
-------


## 2.1 模型结构
-------


[2025 年大模型与 Transformer 架构：重塑 AI 未来的科技革命](https://blog.csdn.net/lifetragedy/article/details/146948744)

[Mamba 详细介绍和 RNN、Transformer 的架构可视化对比](https://blog.csdn.net/deephub/article/details/136250003)

[机器之心 - 盘一盘，2017 年 Transformer 之后，LLM 领域的重要论文](https://www.jiqizhixin.com/articles/2025-06-29-4)


| 编号 | 结构 | 描述 |
|:---:|:---:|:----:|
|  1  | Transformer | NA |
|  2  | Mamba | 线性复杂度的新星 <br>Mamba 利用结构化空间状态对偶 (SSD/Structured Space-State Duality) 构建了一个稳健的理论框架, 使得原本为 Transformer 开发的算法和系统优化技术能够迁移应用于 SSM. Mamba 架构以其线性增长的低计算开销和硬件感知型算法, 在处理长序列数据方面表现出色, 显著提升了计算速度和性能. 与 Transformer 相比, Mamba 的计算开销随序列长度线性增长, 这使得它能够处理更长的文本序列, 同时大幅降低计算成本.<br> 在 A100 GPU 上, Mamba 使用扫描进行循环计算, 能够将计算速度提升 3 倍. 不过, Mamba 架构也存在一些问题, 如记忆丢失、难以泛化到不同任务、在复杂模式方面的表现不及基于 Transformer 的语言模型等. |
|  3  | RWKV | RNN 变体的新突破 <br>RWKV 是循环神经网络 (RNN) 的一个创新变体. 它的架构由一系列堆叠的残差块组成, 每个残差块包含具有循环结构的时间混合 (time-mixing) 和通道混合 (channel-mixing) 子块. RWKV 采用了动态状态演化(Dynamic State Evolution), 具备恒定的显存占用、恒定的推理生成速度以及 "无限" 的上下文长度, 完全不含自注意力机制.<br> 然而, RWKV 基底模型对提示词(prompt) 的格式非常敏感, 提示词的格式对生成结果有较大影响. 并且由于架构设计的原因, RWKV 模型在需要回顾的任务上表现较弱. |
|  4  | Hyena | 高效低复杂度的全新尝试 <br>Hyena 由两个高效的二次基元递归定义的算子, 交织隐式参数化的长卷积和数据控制的门控组成, 构建了一个高效、灵活且计算复杂度低的注意力替代算法. Hyena 的时间复杂度为 O(n*log(n)), 远低于 Transformer 的 O(n²).<br> 在实际应用中, Hyena 能够显著缩小与注意力机制的差距. 当序列长度为 64K 时, Hyena 算子的速度是高度优化注意力的 100 倍. 不过, Hyena 运算不支持 Mas, ，这使得使用 Hyena 架构进行生成式预训练建模时不够灵活. |
|  5  | Difussion | Difussion Language Model  |


### 2.1.1 Transformer
-------




| 编号 | 日期 | 模型 | 团队 | 详情 |
|:---:|:---:|:----:|:---:|:----:|
|  1  | 2025/08/12 | [Lumina-mGPT 2.0](https://github.com/Alpha-VLLM/Lumina-mGPT-2.0) | 上海人工智能实验室 | [Lumina-mGPT 2.0: Stand-Alone AutoRegressive Image Modeling](https://arxiv.org/pdf/2507.17801), 上海人工智能实验室等团队提出 Lumina-mGPT 2.0, 一款独立的、仅使用解码器的自回归模型, 统一了包括文生图、图像对生成、主体驱动生成、多轮图像编辑、可控生成和密集预测在内的广泛任务. 参见 机器之心报道 [机器之心 - 自回归模型华丽复兴，媲美顶尖扩散模型](https://www.jiqizhixin.com/articles/2025-08-12). |

#### 2.1.1.1 模型汇总
-------

[Mooncake: Kimi's KVCache-centric Architecture for LLM Serving](https://arxiv.org/abs/2407.00079)
[如何利用 Kimi 解读 Kimi 的 KVCache 技术细节](https://blog.csdn.net/kunpengtingting/article/details/140202126)

[Attention is All you Need 全文翻译](https://zhuanlan.zhihu.com/p/682007654)
[Transformer 架构自然语言处理文献综述](https://book.douban.com/review/15817600)
[Transformer models: an introduction and catalog](https://arxiv.org/abs/2302.07730)


[大模型思维链 (Chain-of-Thought) 技术原理](https://www.zhihu.com/tardis/bd/art/629087587)
[【LLM 系列 - 07】Chain-of-Thought Prompting Elicits Reasoning in Large Language Models](https://zhuanlan.zhihu.com/p/616838483)
[LLM 之 Prompt 工程技能总结](https://zhuanlan.zhihu.com/p/692618896)


[LLMs-Agent 论文: PAL, 2023, Luyu Gao et al., CMU](https://zhuanlan.zhihu.com/p/664408840)


[LLM 自省系列 (1)ReAct: Synergizing Reasoning and Acting in Language Model](https://zhuanlan.zhihu.com/p/650686547)
[[论文笔记]REACT: SYNERGIZING REASONING AND ACTING IN LANGUAGE MODELS](https://blog.csdn.net/yjw123456/article/details/139102046)


[大语言模型宇宙 - 大模型架构配置概况](https://github.com/mannaandpoem/AGIDreamFactory)

#### 2.1.1.2 云侧大模型
-------


#### 2.1.1.3 端侧大模型
-------

| 编号 | 模型 | 团队 | 详情 | 实例 |
|:----:|:----:|:---:|:---:|:---:|
|  1  | Apple Intelligence | Apple | [Apple Intelligence Foundation Language Models](https://arxiv.org/abs/2407.21075), [苹果 Private Cloud Compute: 云端 AI 隐私技术的新前沿 (翻译版)](https://zhuanlan.zhihu.com/p/703584080) |
|  2  | Gemma-2-2B | Google DeepMind | NA | NA | [aistudio.google](https://aistudio.google.com/app/prompts/new_chat?model=gemma-2-2b-it), [HuggingFace](https://huggingface.co/collections/google/gemma-2-2b-release-66a20f3796a2ff2a7c76f98f), [HuggingFace/mlc-ai](https://huggingface.co/spaces/mlc-ai/webllm-simple-chat), [mlc-ai/deploy/android](https://llm.mlc.ai/docs/deploy/android.html)
|  3  | Phi-2 | NA | NA | NA |
|  4  | MiNiCPM-2B | NA | NA | NA |
|  5  | Qwen2-1.5B | NA | NA | NA |


### 2.1.2 Mamba
-------


### 2.1.3 RWKV
-------


### 2.1.4 Hyena
-------


### 2.1.5 Difussion
-------


[Diffusion LLMs (dLLMs): Introducing a New Generation of LLMs](https://markovate.com/diffusion-llms)

| 编号 | 日期 | 模型 | 团队 | 详情 |
|:---:|:---:|:----:|:---:|:----:|
|  1  | 2025/02/14 | [LLaDA-8B](https://ml-gsai.github.io/LLaDA-demo) | ml-gsai | 参见论文 [Large Language Diffusion Models](Large Language Diffusion Models](https://arxiv.org/abs/2502.09992), [论文 | 2025 | 论文综述：大型语言扩散模型(LLDM)](https://mp.weixin.qq.com/s/W8lLo6BI1xKkj_1HfiH5pg) |
|  2  | 2025/03/02 | [Mercury](https://www.inceptionlabs.ai/introducing-mercury) | Inception Labs | [扩散模型驱动的下一代 LM 范式：Diffusion LLM - Mercury，“飞一般的生成速度”](https://zhuanlan.zhihu.com/p/26844666590) |
|  3  | 2025/03/06 | GIDD | NA | [Generalized Interpolating Discrete Diffusion](https://arxiv.org/abs/2503.04482), [AI 自我纠错，Diffusion 超越自回归！质量提升 55%，已达理论证据下界](https://mp.weixin.qq.com/s/pu2NmYixfwZq94qFDBZ_YQ) |
|  4  | 2025/03/12 | [BD3-LMs](https://m-arriola.com/bd3lms/) | Cornell Tech | 论文 [Block Diffusion: Interpolating Between Autoregressive and Diffusion Language Models](https://arxiv.org/abs/2503.09573), [爆火 Block Diffusion 引发 LLM 架构变革？自回归 + 扩散模型完美结合 | ICLR 2025](https://zhuanlan.zhihu.com/p/32576344984) |
|  5  | 2025/04/02 | [Dream-7B](https://hkunlp.github.io/blog/2025/dream/) | University of Hong Kong<br>Huawei Noah’s Ark Lab | 参见 GitHub [HKUNLP/Dream](https://github.com/HKUNLP/Dream) |
|  6  | 2025/04/12 | [D1](https://dllm-reasoning.github.io) | UCLA<BR>Meta AI | [dllm-reasoning/d1](https://github.com/dllm-reasoning/d1), 参见论文 [d1: Scaling Reasoning in Diffusion Large Language Models via Reinforcement Learning](https://arxiv.org/abs/2504.12216) |
|  7  | 2024/10/24 | [SMDM](https://github.com/ML-GSAI/SMDM) | ML-GSAI | [Scaling up Masked Diffusion Models on Text](https://arxiv.org/abs/2410.18514) |
|  8  | 2024/10/24 | [Seed Diffusion](https://seed.bytedance.com/seed_diffusion) | ML-GSAI | [技术报告](https://lf3-static.bytednsdoc.com/obj/eden-cn/hyvsmeh7uhobf/sdiff_updated.pdf)<br>[项目地址](https://seed.bytedance.com/seed_diffusion)<br>[体验链接](https://studio.seed.ai/exp/seed_diffusion)<br>[量子位 - 字节 Seed 发布扩散语言模型，推理速度达 2146 tokens/s，比同规模自回归快 5.4 倍](https://qbitai.com/2025/08/316722.html) |
|  9  | 2025/08/08 | [DAEDAL](https://github.com/Li-Jinsong/DAEDAL) |[Beyond Fixed: Variable-Length Denoising for Diffusion Large Language Models](https://arxiv.org/abs/2508.00819) | 当前 DLLM 存在着在推理时必须采用预设固定长度的限制, 对于不同任务都需要专门调整才能达到最优效果.<br> 为了解决这一本质的问题, 香港中文大学 MMLab, 上海 AI 实验室等提出 DAEDAL, 赋予 DLLM 可以根据问题的具体情况自主调整回答长度的能力, 弥补了 DLLM 与自回归 LLM 的关键差距, 为更灵活、高效、强大的扩散大语言模型打下了基石.<br>DAEDAL 作为一种 Training Free 的去噪策略, 从一个统一且很短的初始长度开始, 让模型根据自己的需求在生成中调节长度, 动态扩展, 达到了和现有去噪策略在每个评测基准上精心调整生成长度得到的最佳性能相当的表现, 有时甚至更胜一筹. 参见 [机器之心 -- 扩散 LLM 推理新范式：打破生成长度限制，实现动态自适应调节](https://www.jiqizhixin.com/articles/2025-08-08-5). |
|  10 | 2025/08/14 | [Discrete Diffusion Forcing(D2F)](https://arxiv.org/abs/2508.09192) | 上海交通大学 DENG Lab 联合 UCSD | [D2F：首个推理速度超过自回归的开源扩散语言模型](https://zhuanlan.zhihu.com/p/1939283118306604733) |
|  11 | 2025/09/14 | [LLaDA-MoE-7B](https://huggingface.co/inclusionAI/LLaDA-MoE-7B-A1B-Base) | 蚂蚁&人大 | [扩散语言模型也有 MoE 版本了！蚂蚁&人大从头训练 LLaDA-MoE，即将完全开源 ｜ 机器之心](https://www.bestblogs.dev/article/e6ee1e), LLaDA-MoE 有两个版本: 基础模型版 LLaDA-MoE-7B-A1B-Base 和指令微调版 LLaDA-MoE-7B-A1B-Instruct. |

## 2.2 稠密模型与稀疏模型
-------


### 2.2.1 MoE
--------


MoE(Mixed Expert Models), 即混合专家模型, 首次在 1991 年的论文 [Adaptive Mixture of Local Experts](https://www.cs.toronto.edu/~hinton/absps/jjnh91.pdf) 提出 MoE.

[群魔乱舞：MoE 大模型详解](https://www.zhihu.com/tardis/bd/art/677638939)

[【论文阅读】MOE，《OUTRAGEOUSLY LARGE NEURAL NETWORKS: THE SPARSELY-GATED MIXTURE-OF-EXPERTS LAYER》](https://blog.csdn.net/bylander/article/details/138139345)

[【论文速读】MOD，《Mixture-of-Depths: Dynamically allocating compute in transformer-based language models》](https://blog.csdn.net/bylander/article/details/139536003)

[Mixture of Depths 论文解读](https://zhuanlan.zhihu.com/p/691324301)

[OLMoE](https://github.com/allenai/OLMoE)

[Mixture of Lookup Experts](https://arxiv.org/abs/2503.15798) 由于 MoE 会动态选择 experts, 因此所有 EA 都需要加载到 VRAM 中. 它们的大参数大小仍然限制了部署, 而卸载 (仅在需要时将专家加载到 VRAM) 会显著增加推理延迟. 为了解决这个问题, 我们提出了 Mix of Lookup Experts(MoLE), 这是一种新的 MoE 架构, 在通信和 VRAM 使用方面都非常高效. 在 MoLE 中, 专家在训练期间是前馈网络(FFN), 将嵌入层的输出作为输入. 在推理之前, 这些专家可以重新参数化为查找表(LUT), 该查找表根据输入 ID 检索专家输出, 并卸载到存储设备. 因此, 我们不需要在推理过程中执行专家计算. 相反, 我们根据输入 ID 直接检索 EA 的计算结果并将其加载到 VRAM 中, 因此由此产生的通信开销可以忽略不计. 实验表明, 在相同的 FLOPs 和 VRAM 使用量下, MoLE 实现了与密集模型相当的推理速度, 并且在专家卸载的情况下明显快于 MoE, 同时保持与 MoE 相当的性能.

### 2.2.2 稀疏化
-------

[【ICDE 2022】阿里发布稀疏模型训练框架 HybridBackend, 单位成本下训练吞吐提升至 5 倍](https://blog.csdn.net/weixin_48534929/article/details/124661176)


#### 2.2.2.1 动态剪枝
-------

| 编号 | 技术 | 团队 | 介绍 |
|:---:|:----:|:---:|:---:|
|  1  | [D-LLM: A Token Adaptive Computing Resource Allocation Strategy for Large Language Models](https://blog.csdn.net/paixiaoxin/article/details/145521305) | Huawei | 本文提出了一种名为 D-LLM 的新型动态推理机制, 旨在为大型语言模型 (LLMs) 自适应地分配计算资源. 当前, LLMs 对每个词元的处理是等同的, 但作者认为并非所有词语都同等重要, 某些词语在简单问题中并不需要过多的计算资源. D-LLM 通过为每个 Transformer 层设计动态决策模块, 决定是否执行或跳过该层, 从而提高推理速度. 此外, 本文还提出了一种有效的驱逐策略, 以解决跳过层时 KV 缓存缺失的问题. 实验结果表明, D-LLM 在问答、摘要和数学解题任务中可减少高达 45% 的计算成本和 KV 存储, 在常识推理任务中可减少 50%, 且性能未受影响. |
|  2  | [Mixture-of-Recursions: Learning Dynamic Recursive Depths for Adaptive Token-Level Computation](https://arxiv.org/abs/2507.10524) | KAIST、谷歌 DeepMind 等 | 提出了一种在统一的架构中, MoR 同时实现了三种效率优化 <br>1. 参数共享: 通过共享权重压缩参数量, 减小模型体积, 如同让模型学会 "一法通、万法通";<br>2. 自适应计算: 不对所有 token 一视同仁, 让模型根据任务的难易度动态调整计算量 (推理时递归的深度)，好比 "好钢用在刀刃上", 通过小型路由器, 会为每个 token 的隐藏状态打分, 仅高分 token 的会继续循环, 其余的则提前退出. 简单 Token(比如标点, 常见字" 的 "," 在 "等) 仅需单次递归即可推出, 复杂 Token(比如数学符号, 专业术语等)会递归多层进行计算, 通过动态路由机制, MoR 让模型真正实现了「千人千面」的计算: 每个 Token 都能根据自己的实际需求, 获得恰到好处的「思考」深度, 从而大幅削减了不必要的计算开销.<br>3. 通过智能缓存减少内存开销: KV 缓存是 Transformer 推理时的内存大户, 尤其是在处理长序列和进行大批量推理时. 在动态深度模型中, 由于 Token 可能在不同深度退出, 如何确保缓存的一致性和效率, MoR 设计了两种针对性的 KV 缓存策略, 为不同的部署场景提供了灵活的选择. 无论是追求极致精度与批处理量, 还是追求极致内存节省与预填充速度, MoR 都能提供相应的优化方案.<br>[知乎 - 新智元 - Transformer 终结者！谷歌 DeepMind 全新 MoR 架构问世，新一代魔王来了](https://zhuanlan.zhihu.com/p/1929192855898951941)<br>[知乎 - tomsheep-MoR：共享 + 路由 + 缓存，递归混合模型为 LLM 瘦身](https://zhuanlan.zhihu.com/p/1929159784982086816)<br>[知乎 - 北方的郎 - 混合递归（MoR）：让大模型“量体裁衣”，为每个词元智能分配思考深度](https://zhuanlan.zhihu.com/p/1929175581800506290)<br>[github/mixture_of_recursions](https://github.com/raymin0223/mixture_of_recursions) |
|  3  | [CLONE: Customizing LLMs for Efficient Latency-Aware Inference at the Edge](https://arxiv.org/abs/2506.02847) | 澳门大学 | 边缘设备通常存在存储空间有限、计算能力弱等问题, 导致无法直接运行复杂语言模型. CLONE (Customizing LLMs for Efficient Latency-Aware Inference at the Edge) 是 澳门大学 团队开发的一种算法 - 硬件协同设计系统, 旨在解决在边缘设备上部署大型语言模型 (LLMs) 时面临的存储、计算资源限制问题. 该系统通过优化模型结构和硬件加速器设计, 平衡了延迟、能耗与模型精度, 并已在两种通用边缘平台上进行验证. 技术方案包括:<br>1. 硬件感知的模型剪枝优化: 通过剪枝、量化等技术减少模型体积和计算复杂度, 同时保持模型性能.<br>2. 硬件加速: 采用 28nm 工艺的专用硬件加速器, 进一步提升运算效率.<br>3. 在线延迟感知推理: 在算法层面融入实时优化和能量管理机制, 确保在低延迟场景中稳定运行. 使用基于请求的 MoE 路由器动态配置 Lora, 通过层间 DVFS 有效优化能效.




## 2.3 模型压缩和量化
-------


[一文弄懂模型知识蒸馏、剪枝、压缩、量化](https://blog.csdn.net/RuanJian_GC/article/details/128841124)

| 技术 | 描述 |
|:---:|:---:|
| 量化 | 类似 "量子级别的减肥", 神经网络模型的参数一般都用 float32 的数据表示, 但如果我们将 float32 的数据计算精度变成 int8 的计算精度. 则可以牺牲一点模型精度来换取更快的计算速度. |
| 蒸馏 | 类似 "老师教学生", 使用一个效果好的大模型指导一个小模型训练, 因为大模型可以提供更多的软分类信息量, 所以会训练出一个效果接近大模型的小模型. [详解 4 种模型压缩技术、模型蒸馏算法](https://zhuanlan.zhihu.com/p/638092734) |
| 剪裁 | 类似 "化学结构式的减肥", 将模型结构中对预测结果不重要的网络结构剪裁掉, 使网络结构变得更加 "瘦身". 比如, 在每层网络, 有些神经元节点的权重非常小, 对模型加载信息的影响微乎其微. 如果将这些权重较小的神经元删除, 则既能保证模型精度不受大影响, 又能减小模型大小. 参见 [论文总结 - 模型剪枝](https://xmfbit.github.io/2018/10/03/paper-summary-model-pruning/) |
| 神经网络架构搜索(NAS) | 类似 "化学结构式的重构", 以模型大小和推理速度为约束进行模型结构搜索, 从而获得更高效的网络结构. 参见 [一文详解神经网络结构搜索(NAS)](https://zhuanlan.zhihu.com/p/73785074). |






# 3 推理框架
-------


## 3.1 推理框架汇总
-------

[大模型推理框架概述](https://zhuanlan.zhihu.com/p/659792625)

[大模型推理部署：LLM 七种推理服务框架总结](https://blog.csdn.net/m0_59596990/article/details/135311245)

[29 种本地部署大模型和调用的工具平台分类与总结](https://blog.csdn.net/l35633/article/details/138379452)

[phoronix, 2024/12/07, Llamafile 0.8.17 Brings New Web UI For This Easy-To-Distribute AI LLM Framework](https://www.phoronix.com/news/Llamafile-7-Released)


### 3.1.1 推理引擎框架
-------

| 编号 | 推理框架 | 团队 | 介绍 |
|:---:|:-------:|:---:|:---:|
| 1 | [vLLM](https://github.com/vllm-project/vllm) | UC Berkeley | vLLM 是一个开源的大模型推理加速框架, 通过 PagedAttention 高效地管理 attention 中缓存的张量, 实现了比 HuggingFace Transformers 高 14-24 倍的吞吐量. PagedAttention 是 vLLM 的核心技术, 它解决了 LLM 服务中内存的瓶颈问题. 传统的注意力算法在自回归解码过程中, 需要将所有输入 Token 的注意力键和值张量存储在 GPU 内存中, 以生成下一个 Token. 这些缓存的键和值张量通常被称为 KV 缓存. [vLLM(二) 架构概览](https://zhuanlan.zhihu.com/p/681716326) |
| 2 | [Text Generation Inference(TGI)](https://github.com/huggingface/text-generation-inference) | HuggingFace | Text Generation Inference(TGI) 是 HuggingFace 推出的一个项目, 作为支持 HuggingFace Inference API 和 Hugging Chat 上的 LLM 推理的工具, 旨在支持大型语言模型的优化推理.  |
| 3 | [FasterTransformer](https://github.com/NVIDIA/FasterTransformer) | NVIDIA | NVIDIA FasterTransformer (FT) 是一个用于实现基于 Transformer 的神经网络推理的加速引擎. 它包含 Transformer 块的高度优化版本的实现, 其中包含编码器和解码器部分. 使用此模块, 您可以运行编码器 - 解码器架构模型 (如：T5)、仅编码器架构模型 (如：BERT) 和仅解码器架构模型 (如: GPT) 的推理. FT 框架是用 C++/CUDA 编写的, 依赖于高度优化的 cuBLAS、cuBLASLt 和 cuSPARSELt 库, 这使您可以在 GPU 上进行快速的 Transformer 推理.  |
| 4 | [DeepSpeed-Mll](https://github.com/microsoft/DeepSpeed-MII) | MicroSoft | DeepSpeed-MII 是 DeepSpeed 的一个新的开源 Python 库, 旨在使模型不仅低延迟和低成本推理, 而且还易于访问.  |
| 5 | [FlexFlow Server](https://github.com/flexflow/FlexFlow) | Leland Stanford Junior University | 一个开源编译器和分布式系统, 用于低延迟、高性能 LLM 服务.  |
| 6 | [LMDeploy](https://github.com/InternLM/lmdeploy) |  [MMDeploy](https://github.com/open-mmlab/mmdeploy) 和 [MMRazor](https://github.com/open-mmlab/mmrazor) 团队联合开发 | 一个 C++ 和 Python 库, 用于使用 Transformer 模型进行高效推理. |
| 7 | [CTranslate2](https://github.com/OpenNMT/CTranslate2) | OpenNMT | 一个快速推理引擎, 适用于 Transformer 模型, 提供高效的推理能力和性能优化技术. 在本文中, 我们将探索与 CTranslate2 相关的关键功能、模型类型、安装过程、基准测试以及其他资源. |
| 8 | OpenLLM | OpenLLM | 用于在生产中操作大型语言模型 (LLM) 的开放平台. 为核心模型添加 adapter 并使用 HuggingFace Agents, 尤其是不完全依赖 PyTorch |
| 9 | Ray Serve | NA | 一个可扩展的模型服务库, 用于构建在线推理 API. Serve 与框架无关, 因此你可以使用单个工具包来服务深度学习模型中的所有内容. 稳定的 Pipeline 和灵活的部署, 它最适合更成熟的项目 |
| 10 | MLC LLM | NA | MLC LLM 是一种通用部署解决方案. 可在客户端 (边缘计算), 例如 Android 或 iPhone 平台上, 本地部署 LLM. [MLC LLM：将 LLMs 部署到消费类硬件的优势、挑战以及解决方案](https://blog.csdn.net/FrenzyTechAI/article/details/132340135) |
| 11 | [PaddlePaddle/Anakin](https://github.com/PaddlePaddle/Anakin) | BaiDu | 一个高性能的跨平台推理引擎, 可以在 x86 CPU, ARM, NVIDIA GPU, AMD GPU, 比特大陆以及寒武纪等设备上运行. |
| 12 | [mllm](https://github.com/UbiquitousLearning/mllm) | [UbiquitousLearning](https://ubiquitouslearning.github.io/mllm_website) | 一个快速轻量级的多模态 LLM 推理引擎, 适用于移动和边缘设备, C/C++ 实现, 无任何其他依赖, 并针对多模态比如 fuyu-8B 进行了优化, 支持 ARM NEON 和 x86 AVX2 加速, 以及 4BIT 和 8BIT 整数量化. |
| 13 | [XiaoMi/Mace](https://github.com/XiaoMi/mace) | 小米 | MACE (Mobile AI Compute Engine) 是一个针对移动异构计算平台优化的深度学习推理框架. 它专注于以下目标: 性能、功耗、响应性、内存使用和库体积、模型保护以及平台覆盖. MACE 支持 TensorFlow、Caffe 和 ONNX 等多种模型格式, 并提供了丰富的示例和文档. |
| 14 | [Google-AI-Edge/litert](https://github.com/google-ai-edge/LiteRT) | Google Ai Edge | LiteRT(原名 TensorFlow-Lite) 是 Google 开源的高性能端侧 AI 运行时 |
| 15 | [AliBaBa/MNN](https://github.com/alibaba/MNN) | AliBaBa | MNN 是一个高效轻量级的深度学习框架, 在阿里巴巴的关键业务场景中得到广泛应用. 它支持深度学习模型的推理和训练, 在设备上具有业界领先的性能. MNN 还提供了一系列工具, 包括模型转换、压缩、表达式计算等功能. |
| 16 | [Tencent/TNN](https://github.com/Tencent/TNN) | Tencent | TNN 是由腾讯优图实验室和广影实验室开发的一个跨平台、高性能的深度学习推理框架. 它具有跨平台能力、高性能、模型压缩和代码裁剪等多项优秀特性. TNN 在原有的 ncnn 和 Rapidnet 框架基础上, 进一步加强了对移动设备的支持和性能优化, 同时也借鉴了业界主流开源框架的高性能和良好扩展性特点, 扩展了对 X86 和 NV GPU 的支持. TNN 已经被应用于腾讯移动 QQ、微视、Pitu 等多个应用中. |
| 17 | [Paddle-Lite](https://github.com/PaddlePaddle/Paddle-Lite) | [PaddlePaddle](https://www.paddlepaddle.org.cn/lite) | Paddle Lite 面向端侧场景的轻量化推理引擎 Paddle Lite, 可以实现飞桨模型在 x86/ARM 平台下多种 OS 内的高效部署, 同时支持在 10 种以上的 GPU/NPU 异构后端上进行推理加速和混合调度. 是一个高性能、轻量级、灵活性强且易于扩展的深度学习推理框架, 定位于支持包括移动端、嵌入式以及边缘端在内的多种硬件平台. 它提供了简单易用的部署流程, 支持多种硬件平台和多种编程语言, 并且具有优秀的加速、优化策略及实现. |
| 18 | [uTensor]() | NA | NA |
| 19 | Core ML | Apple | NA |
| 20 | MediaPipe | Google |

### 3.1.2 推理加速库
-------


| 编号 | 加速框架 | 团队 | 介绍 |
|:---:|:-------:|:---:|:---:|
| 1 | [ARM-software/CMSIS-NN](https://github.com/ARM-software/CMSIS-NN) | ARM | CMSIS-NN 是一个高性能的神经网络内核软件库, 旨在最大化 Arm Cortex-M 处理器上神经网络的性能并最小化内存占用. 它遵循 TensorFlow Lite for Microcontrollers 的 INT8 和 INT16 量化规范, 与 TensorFlow Lite 参考内核完全一致. 该库提供了针对不同 Arm 处理器架构的优化实现, 包括纯 C、 DSP 扩展和 MVE 扩展等. |
| 2 | [SNPE](https://www.qualcomm.com/developer?redirect=qdn) | Qualcomm Snapdragon | SNPE 是 Qualcomm Snapdragon Neural Processing Engine 的简称. SNPE 是神经网络在骁龙平台上推理的开发套件, 方便开发者在使用高通芯片的设备上加速 AI 应用. 支持的模型框架: TensorFlow, CAFFE, ONNX, TensorFlowLite. 比如 [SNPE_Tutorial](https://github.com/gesanqiu/SNPE_Tutorial) |
| 3 | [PX4/eigen](https://github.com/PX4/eigen) | PX4 | Eigen 是一个 C++ 模板库, 用于线性代数: 矩阵、向量、数值求解器和相关算法. 它提供了一个高效、灵活和易于使用的接口, 适用于各种应用程序.|
| 4 | [Google/XNNPACK](https://github.com/google/XNNPACK) | Google | XNNPACK 是一个针对 ARM、x86、WebAssembly 和 RISC-V 平台的高度优化的神经网络推理解决方案. 它不是直接面向深度学习从业者和研究人员使用的, 而是为诸如 TensorFlow Lite、TensorFlow.js、PyTorch、ONNX Runtime 和 MediaPipe 等高级机器学习框架提供低级性能原语, 以加速它们的推理性能. |
| 5 | [OpenBLAS](https://github.com/OpenMathLib/OpenBLAS) | OpenBLAS | 开源的 CPU 线性代数库，支持多线程和 SIMD 加速, 广泛应用于科学计算和深度学习框架(如 PyTorch). |
| 6 | [Intel MKL(Math Kernel Library)]() | NA | 针对 Intel CPU 优化的数学计算库, 支持矩阵运算、FFT 等. 在 Intel 平台上性能优于 Eigen. |
| 7 | [Arm Compute Library](https://github.com/ARM-software/ComputeLibrary) | ARM | Arm CPU/GPU 的加速库, 支持图像处理和机器学习算子. 针对 Cortex-A/Cortex-M 优化, 兼容 CMSIS-NN46. |
| 8 | [CuPy](https://github.com/cupy/cupy) | NA | 基于 NVIDIA GPU 的数值计算库，语法兼容 NumPy. 替代部分 Eigen 功能, 适合 GPU 加速场景. |
| 9 | [neon](https://github.com/NervanaSystems/neon) | Intel | neon 是英特尔公司开源的深度学习框架, 致力于在各种硬件上提供最佳性能. 它设计简单易用, 并且具有可扩展性. |
| 10 | [lapack](https://github.com/Reference-LAPACK/lapack) | NA | LAPACK 是一个用于解决常见数值线性代数问题的 Fortran 子程序库. 它是一个免费提供的软件包, 可以包含在商业软件包中. LAPACK 包含了 Fortran 源代码、测试程序、基本线性代数子程序 (BLAS) 的 Fortran 参考实现, 以及 CBLAS 和 LAPACKE 的 C 接口. |
| 11 | [Tencent/ncnn](https://github.com/Tencent/ncnn) | Tencent | ncnn 是一个为手机端极致优化的高性能神经网络前向计算框架. 它从设计之初就深入考虑了手机端的部署和使用. ncnn 无第三方依赖、跨平台, 在手机端 CPU 上的速度快于目前所有已知的开源框架. 开发者可以轻松将深度学习算法移植到手机端高效执行, 开发出人工智能 APP, 将 AI 带到用户的指尖. ncnn 目前已在腾讯多款应用中使用, 如 QQ、Qzone、微信、天天 P 图等. |

## 3.2 推理加速
-------


[知乎 --LLM 推理加速技术简介](https://zhuanlan.zhihu.com/p/691360124)

[bilibili-- 如何将大模型与小模型结合？这 8 种常用策略必看！附 17 篇案例论文和代码](https://www.bilibili.com/opus/887920175625535524)

[知乎 -- 刀刀宁聊大模型推理 -- 笔记：学习推理加速半年之总结与迷思](https://zhuanlan.zhihu.com/p/704938096)

[知乎 - 锦年 - 全面解析 LLM 推理优化：技术、应用与挑战](https://zhuanlan.zhihu.com/p/18736565021)

### 3.2.1 KV Cache 压缩
-------

[SnapKV: LLM 在生成内容之前就知道您在寻找什么](https://blog.csdn.net/qq_36931982/article/details/139118015)

[MiniCache 和 PyramidInfer 等 6 种优化 LLM KV Cache 的最新工作](https://www.51cto.com/aigc/913.html)

[PyramidKV 学习资料汇总 - 动态 KV 缓存压缩技术](https://blog.csdn.net/m0_56734068/article/details/142382328)

[大模型推理加速：KV Cache Sparsity(稀疏化)方法](https://zhuanlan.zhihu.com/p/701580870)

[聊聊大模型推理中的 KVCache 之异构缓存](https://zhuanlan.zhihu.com/p/714288577)

[聊聊大模型推理中的 KVCache 压缩](https://zhuanlan.zhihu.com/p/708946312)


### 3.2.2 稀疏感知推理加速
-------

[论文笔记：DejaVu、LLM in Flash、PowerInfer](https://zhuanlan.zhihu.com/p/675585887)

[苹果极致 LLM 端侧方案：LLM in a flash](https://zhuanlan.zhihu.com/p/673775476)

### 3.2.3 首 Token 时延优化
-------

[[Prefill 优化][万字]🔥原理 & 图解 vLLM Automatic Prefix Cache(RadixAttention): 首 Token 时延优化](https://zhuanlan.zhihu.com/p/693556044)

### 3.2.4 投机解码
-------


#### 3.2.4.1 投机解码算法分析
-------

[论文导读 | 投机解码加速模型推理](https://zhuanlan.zhihu.com/p/698333087)

[大模型推理妙招—投机采样(Speculative Decoding)](https://zhuanlan.zhihu.com/p/651359908)

[最全 LLM 自投机算法汇总](https://zhuanlan.zhihu.com/p/706111755)

[LLM 推理提速 2.8 倍，CMU 清华姚班校友提出「投机式推理」引擎 SpecInfer，小模型撬动大模型高效推理](https://www.jiqizhixin.com/articles/2023-05-30-3)

[知乎 - LLM 推理加速新范式！推测解码（Speculative Decoding）最新综述](https://zhuanlan.zhihu.com/p/678404136)

[知乎 - 投机采样（Speculative Decoding），另一个提高 LLM 推理速度的神器（三）](https://zhuanlan.zhihu.com/p/681401656)

[知乎 - 刀刀宁 - 聊聊大模型推理服务之投机推理](https://zhuanlan.zhihu.com/p/699166575)

[知乎 - hemingkx - 推测解码（Speculative Decoding）哪家强？-- 最新评测基准 Spec-Bench 分享](https://zhuanlan.zhihu.com/p/683995502)

[知乎 - 有没有 speculative decoding 的综述？](https://www.zhihu.com/question/657854511)



| 编号 | 时间 | 文章 | 描述 |
|:---:|:----:|:---:|:----:|
|  1  | 2025/07/04 | [知乎 - Se7en - Speculative Decoding 推测解码方案详解](https://zhuanlan.zhihu.com/p/1920447613800547342) | 本文是 LLM 推理系列的第 4 篇, 介绍 Speculative Decoding 推测解码方案详解, 详细介绍了 EAGLE、Medusa、Lookahead 等主流的 Speculative Decoding 方案. |
|  2  | NA | [从 EAGLE-3 看 Tokenizer 对投机解码的影响](https://zhuanlan.zhihu.com/p/1916965162755285553) | EAGLE3 能带来如此高的加速比，除了它本身优秀的草稿模型和验证机制外，是不是还有其他 “玄机”？不同模型对不同语言的 tokenizer 优化程度不同, 导致它们在多语言场景下的表现差异. Vicuna 的 Tokenizer 在不同语言中普遍呈现出较细的粒度; 从“每步接受多少 Token” 的指标上来看是其他模型的 1.5~2 倍. |
|  3  | 2025/05/24 | [大模型推理 & memory bandwidth bound (4) - Speculative Decoding](https://zhuanlan.zhihu.com/p/1899508608909162422) | 本篇讲解了 Speculative Decoding 的原理, 即以近似模型输出建议 tokens, 目标模型对齐进行并行验证的方式对模型推理进行了加速; 同时, 我们也证明了 Speculative Sampling 的采样方式使得其输出分布与目标模型原有的自回归输出分布保持一致. |
|  4  | 2025/05/05 | [三种投机解码方法对比：Vanilla Speculative Decoding ・ Medusa ・ EAGLE](https://zhuanlan.zhihu.com/p/1902847900742055126) | 作者分析了几个投机算法, 得出总结:<br>1. Vanilla 双模型最易落地, 维护成本在“小模型 + 双 KV Cache”.<br>2. Medusa 把草稿嵌回大模型顶层, 多头 + 树形一次核验, 无需小模型, 但须改写主模型.<br>3. EAGLE 草稿移至特征空间, 接受率最高, 代价是额外 Draft 训练与中间特征. |
|  5  | 2025/07/04 | [知乎 - AI 算法小喵 - 投机解码之 EAGLE：轻量级草稿模型实现 3-4 倍推理加速](https://zhuanlan.zhihu.com/p/1898466485095098162) | 分享了 Eagle, 一种利用目标模型的 feature, 并在 feature 层面结合 token 信息进行自回归的从而保证生成质量实现加速的草稿模型构建方法, 并在结尾对比了与之非常相似的 MTP. |
|  6  | 2025/06/29 | [大模型推理加速之 Speculative Decoding / 投机解码(上)](https://zhuanlan.zhihu.com/p/1922687688307377206) | 作者阅读了《Fast Inference from Transformers via Speculative Decoding》，后简单地分析了 speculative decoding 的处理流程以及其中涉及的一些细节 |
|  7  | [知乎 - - 推测解码 - 从 draft model、Medusa、Recurrent Drafter、EAGLE、 Prompt Lookup 到 Lookahead Decoding](https://zhuanlan.zhihu.com/p/19701798414) |  按照 tensorRT-llm 支持的文档 Speculative Sampling 我们把推测解码分成四种类型:<br>1. draft model: 利用更小的 LLM 先快速生成 token 序列, 使用 LLM 进行一次性验证;<br>2. additional heads: 利用在原 LLM 上新增训练的 transformers 的注意力层来生成 tokens;<br>3. Prompt Lookup: 使用 prompt tokens as draft tokens<br>4. Lookahead Decoding: 用历史数据构成 tokens, 但一个批次的构造[ABC,BCD,CDE], 提高单批次推理速度的骚操作. |
|  8  | 2025/05/25 | [知乎 - 笑渐不闻声渐悄 - Speculative Decoding: 总结、分析、展望](https://zhuanlan.zhihu.com/p/1904881828906668879) | Speculative Decoding: 总结、分析、展望 |
|  9  | 2025/01/25 |[知乎 - CarryPls - 大模型推理加速技术调研 - 投机采样](https://zhuanlan.zhihu.com/p/20233143567) | 现有的投机采样架构可以分为 drafter-scorer-sampler 3 层, 该观念来自 vLLM PR:<nbr>1. drafter 生成 draft tokens: 关注 draft tokens 生成速度, 质量和组织形式.<br>2. scorer 利用 target model（大模型）评估生成的 draft tokens 的概率分布. 关注显存使用和验证速度.<br>3. sampler 根据前两步的结果选择接受的 token. 关注 token 接受率. |
|  10 | 2024/09/11 | [知乎 - Sjrrr 大蛇 - 最全 LLM 自投机算法汇总](https://zhuanlan.zhihu.com/p/706111755) | 对业界领先的多个投机算法进行了分析. |
|  11 | 2025/06/08 | [知乎 - 罗西的思考 - 探秘 Transformer 系列之(30)--- 投机解码](https://zhuanlan.zhihu.com/p/1898466485095098162) | 对 BPD 投机论文的详细分析 |



| 编号 | 时间 | 文章 | 描述 |
|:---:|:----:|:---:|:----:|
|  1  | 2025/04/24 | [OPT-Tree: Speculative Decoding with Adaptive Draft Tree Structure](https://arxiv.org/abs/2406.17276) | [Jikai0Wang/OPT-Tree](https://github.com/Jikai0Wang/OPT-Tree), [微信公众号 - 机器学习算法与自然语言处理 - 一步生成超过 10 个 Tokens!! 无损模型解码加速最新工作](https://mp.weixin.qq.com/s?__biz=MzI4MDYzNzg4Mw==&mid=2247563973&idx=2&sn=a4d1f1a7ee39b11af74bdd020cbdd90c&chksm=ea702fbc79a360767cf3bbb1eb4102b1bb22f8e74f5a258cc3cce656cf88f11eb666f3c97917&scene=27) |
|  2  | 2024/06/25 | [Optimizing Speculative Decoding for Serving Large Language Models Using Goodput](https://arxiv.org/abs/2406.14066) | 对于不同系统负载下的所有工作负载, 没有最佳推测长度工作. 根据观察结果, 作者开发了一个动态框架 SmartSpec. SmartSpec 根据一个名为 goodput 的新指标动态确定每个请求的最佳投机长度(从 0, 即无投机到许多代币)——因此相关的投机执行成本, 该指标表征了整个系统的当前观察负载和投机准确性. |
|  3  | 2025/03/07 | [SpecServe: Efficient and SLO-Aware Large Language Model Serving with Adaptive Speculative Decoding](https://arxiv.org/abs/2503.05096) | 在本文提出了 SpecServe, 可根据实时请求负载和系统配置动态调整推测策略. SpecServe 提出了一个理论模型来理解和预测不同场景中推测解码的效率. 此外, 它还实现了智能绘图和验证算法, 以保证最佳性能, 同时实现高 SLO 实现. 在实际 LLM 跟踪上的实验结果表明, SpecServe 始终满足 SLO 并实现了实质性的性能改进, 与最先进的推测推理系统相比, 速度提高了 1.14. |
|  4  | 2024/05/26 | [Kangaroo: Lossless Self-Speculative Decoding via Double Early Exiting](https://arxiv.org/abs/2404.18911) | [华为诺亚 | 提出自推测解码框架：Kangaroo，降低成本，提升大模型推理效率！](https://cloud.tencent.com/developer/article/2415194)

#### 3.2.4.2 Multi-Token Prediction(多 token 预测)
-------

| 编号 | 时间 | 论文 | 描述 |
|:---:|:----:|:---:|:----:|
|  1  | 2025/07/16 | [Your LLM Knows the Future: Uncovering Its Multi-Token Prediction Potential](https://www.alphaxiv.org/abs/2507.11851) | 实现 MTP 框架, 使预训练的自回归大型语言模型能够执行多 token 预测, 在保持生成质量的同时, 为代码和数学任务提供高达 5.35 倍的推理加速, 以及为一般任务提供约 2.5 倍的推理加速.<br>
研究者们评估了自回归模型在语言模型有监督微调阶段对多 token 预测任务的适应能力. 未来值得探索的一个方向, 是在预训练阶段或下游任务自适应阶段引入该方法, 以进一步检验其适用性与效果. 另一个具有前景的研究方向是将基于扩散的生成方法应用于多 token 预测任务. 研究者们认为, 多 token 预测位于完全自回归生成与完全扩散生成之间, 能够在两者之间取得优势的平衡，兼具效率与质量的潜力. 参见 [机器之心 -- 五倍推理加速，激发自回归潜能，苹果新工作让 LLM 预测未来](https://www.jiqizhixin.com/articles/2025-07-24-9) |
|  2  | 2025/06/13 | [Improving Large Language Models with Concept-Aware Fine-Tuning](https://arxiv.org/abs/2506.07833) | 当前主流 LLM 都依赖 next-token prediction 进行训练,, 但它却让 AI 很难真正理解跨越多 token 的完整概念. 于是南洋理工大学最近提出了一项新技术——概念感知微调 (CAFT), 首次实现将 multi-token prediction(多 token 预测) 引入微调阶段, 让模型能够像人类一样理解和学习完整概念. 原来 LLM 只能碎片化理解每个 token, 现在 CAFT 可以为模型添加额外的辅助头, 在主模型学习下一个词的同时, 帮助学习后续 token, 并通过动态调整权重, 确保模型始终优先优化主要任务的损失. 最终 LLM 可以兼顾多 token 概念学习, 形成更为完整的认知, 在推理和生成能力增强的同时, 既不会影响模型本身, 也不会额外增加多余成本. 参见量子位报道 [知乎 - 量子位 - 突破单 token 预测局限！南洋理工首次将多 token 预测引入微调](https://zhuanlan.zhihu.com/p/1931778341473616685), [项目地址](https://github.com/michaelchen-lab/caft-llm). |


### 3.2.5 并行解码
-------

[Accelerating Transformer Inference for Translation via Parallel Decoding](https://arxiv.org/abs/2305.10427)




### 3.2.6 分布式推理
-------


#### 3.2.6.1 分布式推理
-------

| 编号 | 加速框架 | 团队 | 介绍 |
|:---:|:-------:|:---:|:---:|
| 1 | [b4rtaz/distributed-llama](https://github.com/b4rtaz/distributed-llama) | Bart Tadych(b4rtaz) | Distributed Llama 是一个开源项目, 旨在通过张量并行化技术在多台设备上分布式运行大型语言模型 (LLM). 它可以在普通的 CPU 设备上运行 LLM, 通过分布工作负载来提高推理速度, 并将 RAM 使用量分散到多个节点上, 以加速大型语言模型(LLM) 的推理. 该项目支持 Linux、macOS 和 Windows 操作系统, 并针对 ARM 和 x86_64 AVX2 CPU 进行了优化.<br> 主要功能点:<br>1. 支持多个设备组成集群, 利用张量并行和高速以太网同步, 提高推理性能 <br>2. 支持多种 Llama 模型, 包括 Llama 3.1 405B、Llama 3.3 70B 等 <br>3. 提供简单的命令行工具, 可以快速启动根节点和工作节点 <br>4. 支持 API 服务器, 方便集成到其他应用程序中 |
| 2 | [exo-explore/exo](https://github.com/exo-explore/exo) | exo 实验室 | exo 是一个可以在家中使用普通设备运行自己的 AI 集群的项目 <br> 主要功能点:<br>1. 支持多种模型, 包括 LLaMA、Mistral、LlaVA、Qwen 和 Deepseek 等 <br>2. 动态模型分区, 可根据当前网络拓扑和设备资源自动优化模型分布 <br>3. 自动发现设备, 无需手动配置 <br>4. 提供与 ChatGPT 兼容的 API<br>5. 采用对等连接架构, 设备之间地位平等. |
| 3 | [NVIDIA Dynamo](https://developer.nvidia.cn/dynamo) | NVIDIA | NVIDIA Dynamo 是一个开源、低延迟的模块化推理框架, 用于在分布式环境中服务生成式 AI 模型. 它通过智能资源调度和请求路由、优化的内存管理和无缝的数据传输, 实现跨大型 GPU 集群的推理工作负载无缝扩展. NVIDIA Dynamo 支持所有主要的 AI 推理后端, 并提供专门针对大语言模型 (LLM) 的优化, 例如分解服务. |
| 4 | [prima.cpp](https://github.com/Lizonghang/prima.cpp) | NA | `prima.cpp` 是 `llama.cpp`(一个性能优异的大模型推理框架)的分布式实现, 它允许您在日常设备上运行 70B 级 LLM--💻 笔记本电脑，🖥️ 台式机，📱 手机和平板电脑(GPU 或没有 GPU), 都很好. 参见论文 [PRIMA.CPP: Speeding Up 70B-Scale LLM Inference on Low-Resource Everyday Home Clusters](https://arxiv.org/pdf/2504.08791) |

#### 3.2.6.2 异构推理
-------

[HeteroLLM: Accelerating Large Language Model Inference on Mobile SoCs platform with Heterogeneous AI Accelerators](https://arxiv.org/abs/2501.14794)


| 日期 | 概要 | 论文 / 链接 | 团队 | 描述 |
|:---:|:----:|----------:|:----:|:----:|
| 2024/07 | 异构计算资源混合调度的大模型推理系统 llm.npu | [Fast On-device LLM Inference with NPUs, ASPLOS'2025](https://arxiv.org/abs/2407.05858v2) | 北京大学计算机学院 | 在端设备侧进行高性能的模型推理成为泛在计算环境下一个重要的应用场景. 然而, 即使是专为端侧设备设计的大语言模型(LLM), 如 Gemma-2B 在处理屏幕 UI 理解等任务时, 仍面临预填充阶段的高延迟瓶颈.<br>为了解决这一问题, 团队提出了 llm.npu 系统, llm.npu 是首个基于端侧设备的神经网络处理芯片(NPU) 来进行任务分载, 以降低预填充阶段的延迟/能耗以提升推理任务整体性能的系统, 通过 NPU(整数计算)和 CPU/GPU 上(浮点运算)的内存共享和乱序执行来确保计算精度.<br>1. 在 Prompt 层面, Chunk-sharing graph, 将可变长度的提示词分割为多个固定大小的块, 以保持数据依赖性;<br>2. 在 Tensor 层面, Shadow outlier execution, llm.npu 识别并提取重要的异常值在 CPU/GPU 上处理, 以保证推理的准确性;<br>3. 在 Block 层面, Out-of-order subgraph execution, 根据 Transformer 块的硬件适应性和对精度的敏感性, 将它们灵活调度到 CPU/GPU/NPU上. 实验显示, 在保持精度的同时, 相比 5 个主流的同期主流工作(llama.cpp、TFLite、MNN、MLC-LLM 和 PowerInfer-v2), llm.npu 在可以提升 7.3 到 43.6 倍, 并降低了 1.9 到 59.5 倍的能耗. 该工作为利用端侧异构计算资源来优化 LLM 推理性能探索了全新路径, 也为泛在计算环境下的 LLM 规模化应用提供了有效系统支撑. |

### 3.2.7 注意力机制
-------

[微信公众号 - 地学 AI 实验室 - 可解释 AI，在 Transformer 中可视化注意力（附代码）](https://mp.weixin.qq.com/s/vwJEBXCk6GrwN9-BVucoQA)

[LLM 推理的 Attention 计算和 KV Cache 优化：PagedAttention、vAttention 等](https://www.51cto.com/aigc/1703.html)


| 编号 | 时间 | 论文 | 描述 |
|:---:|:----:|:---:|:----:|
|  1  | 2023/09/12 | [Efficient Memory Management for Large Language Model Serving with PagedAttention](https://arxiv.org/abs/2309.06180) | 大型语言模型的高吞吐量服务需要一次批处理足够多的请求. 然而, 现有系统举步维艰, 因为每个请求的键值缓存 (KV 缓存) 内存非常庞大, 并且会动态增长和缩小. 如果管理效率低下, 该内存可能会因碎片和冗余重复而严重浪费, 从而限制批量大小. 为了解决这个问题, 作者提出了 PagedAttention, 这是一种注意力算法, 其灵感来自作系统中的经典虚拟内存和分页技术. 在此基础上, 构建了 vLLM, 可实现 KV 缓存中近乎零的浪费, 以及在请求内和请求之间灵活共享 KV 缓存, 以进一步减少内存使用. |
|  2  | 2024/05/07 | [vAttention: Dynamic Memory Management for Serving LLMs without PagedAttention](https://arxiv.org/abs/2405.04437) | PagedAttention 支持按需分配 GPU 内存以减轻 KV 缓存碎片, 削弱了先前系统中的批大小(以及因此的吞吐量). 但是, 在尝试在运行时分配物理内存时, PagedAttention 最终会将 KV 缓存的虚拟内存布局从连续更改为非连续. 这样的设计会导致不平凡的编程和性能开销. 作者提出了 vAttention, 在减轻物理内存碎片的同时, 保留虚拟内存中 KV 缓存的连续性. 通过使用 CUDA 虚拟内存管理 API 解耦虚拟内存和物理内存的分配来实现这一点, 还引入了各种特定于 LLM 的优化, 以解决 CUDA 虚拟内存支持的局限性. vAttention 是 PagedAttention 的更简单、更便携且高性能的替代方案: 与使用 FlashAttention 和 FlashInfer 的基于 PagedAttention 的内核相比, 它支持各种开箱即用的注意力内核, 并将 LLM 服务吞吐量提高了 1.23 倍. |
|  3 | 2025/02/16 | [Native Sparse Attention: Hardware-Aligned and Natively Trainable Sparse Attention](https://arxiv.org/abs/2502.11089) | 长上下文建模对于下一代语言模型至关重要, 但标准注意力机制的高计算成本带来了重大的计算挑战. 稀疏注意力为在保持模型能力的同时提高效率提供了一个有希望的方向. DeepSeek 提出了 NSA, 这是一种原生可训练的稀疏注意力机制, 它将算法创新与硬件对齐的优化相结合, 以实现高效的长上下文建模. NSA 采用动态分层稀疏策略, 将粗粒度标记压缩与细粒度标记选择相结合, 以保留全局上下文感知和局部精度. 通过两项关键创新推进稀疏注意力设计: (1)通过算术强度平衡算法设计实现了大幅加速, 并针对现代硬件进行了实现优化. (2)实现端到端训练, 在不牺牲模型性能的情况下减少预训练计算. 实验表明, 使用 NSA 预训练的模型在一般基准、长上下文任务和基于指令的推理中保持或超过全注意力模型. 同时, NSA 在解码、前向传播和向后传播的 64k 长度序列上实现了比全注意力的显着加速, 验证了其在整个模型生命周期中的效率. |



### 3.2.8 低内存
-------


| 日期 | 概要 | 论文 / 链接 | 团队 | 描述 |
|:---:|:----:|----------:|:----:|:----:|
| 2025/03 | 通过灵活的内存管理和异步预加载技术提升设备端推理性能 | [FlexInfer: Breaking Memory Constraint via Flexible and Efficient Offloading for On-Device LLM Inference, EuroMLSys '25](https://arxiv.org/abs/2503.03777) | NA | FlexInfer 通过灵活的内存管理和异步预加载技术提升设备端推理性能. 其核心优化策略包括异步预取、平衡内存锁定和灵活的张量保存机制, 能在限定资源下显著提升吞吐率. 核心优化技术:<br>1. 异步预取: 通过提前加载模型参数和中间数据, 减少推理过程中的 I/O 延迟.<br>2. 平衡内存锁定: 动态调整内存分配策略, 避免单一任务占用过多内存导致性能瓶颈.<br>3. 灵活的张量保存机制: 根据计算需求动态调整张量存储方式, 平衡计算效率与内存占用.<br>性能表现: FlexInfer 在端侧推理任务中, 相比传统方法可提升 2~4 倍的吞吐率, 适用于分类、语义分割等场景. |



## 3.3 算子库
-------


PaddlePaddle/Anakin

Tencent/ncnn

Tencent/FeatherCNN

dmlc/tvm

ARM-software/ComputeLibrary


## 3.4 长上下文
-------

[[LLM 性能优化]聊聊长文本推理性能优化方向](https://zhuanlan.zhihu.com/p/698308542)


# 4 分析工具
-------

| 编号 | 内容 | 详情 |
|:---:|:----:|:---:|
|  1  | [Interactive Tools for machine learning, deep learning, and math](https://github.com/Machine-Learning-Tokyo/Interactive_Tools) | 用于机器学习、深度学习和数学运算的交互式工具. |
|  2  | [Visual Guides to understand the basics of Large Language Models](https://towardsdatascience.com/visual-guides-to-understand-the-basics-of-large-language-models-0715701bdd20) | 一系列工具与文章的汇编, 直观易懂地解读复杂的 AI 概念. 译文 [深入浅出：大语言模型的视觉解析 [译]](https://baoyu.io/translations/llm/visual-guides-to-understand-the-basics-of-large-language-models). |
|  3  | [MLVisuals](https://github.com/dair-ai/ml-visuals) | [科研必备——上手 ML Visuals - 神经网络画图神器](https://blog.csdn.net/weixin_43499292/article/details/127030792) |

## 4.1 Tokenizer
-------

### 4.1.1 Token 计算器
-------

| 编号 | 工具 | 团队 | 详情 |
|:---:|:----:|:---:|:---:|
|  1  | 灵积 Token 计算器 | 阿里 | [阿里 / DashScope 模型服务灵积 / Token 计算器](https://dashscope.console.aliyun.com/tokenizer) |
|  2  | OpenAI/Token 计算器 | OpenAI | [OpenAI/Token 计算器](https://platform.openai.com/tokenizer) |

### 4.1.2 Tokenizer
-------

[大模型分词：sentencepiece vs titoken](https://zhuanlan.zhihu.com/p/691609961)
[tokenizer（一）训练一个 LLM 分词器](https://zhuanlan.zhihu.com/p/688792019)
[Tokenizer 的系统梳理，并手推每个方法的具体实现](https://cloud.tencent.com/developer/article/2327739)
[各种中文分词工具的使用方法](https://blog.csdn.net/PolarisRisingWar/article/details/125388765)
[五款中文分词工具在线 PK: Jieba, SnowNLP, PkuSeg,THULAC, HanLP](https://zhuanlan.zhihu.com/p/64409753)

| 编号 | 分词器 | 团队 | 详情 |
|:---:|:-----:|:----:|:---:|
|  1  | [sentencepiece](https://github.com/google/sentencepiece) | Google | Unsupervised text tokenizer for Neural Network-based text generation.<br>SentencePiece 是谷歌开源的针对 NLP 场景提取词汇表 tokenizer 的开源项目, 它是谷歌推出的子词开源工具包, 其中集成了 BPE、ULM 子词算法. 除此之外, SentencePiece 还能支持字符和词级别的分词. 更进一步, 为了能够处理多语言问题, sentencePiece 将句子视为 Unicode 编码序列, 从而子词算法不用依赖于语言的表示.<br>SentencePiece 提出的目的是在给定词汇表大小的前提下, 最大化词表信息编码 (词频 + 多样性)subword 编码. 比如英语中的 simple 和 simplify 这两个词意思是一样的, 是为了适应语法需求而有的变化, 所以使用独立的 token 对这两个单词编码是有冗余的, 另外一种场景是, 词频不一样, 有常用汉字一说, 也有常用英语单词一说. 出现较少的词使用独立的 token 在训练的时候相比其他高频词由于出现的太少而造成深度学习 (信息压缩) 这一过程容易丢失该信息. |
|  2  | [huggingface/tokenizers](https://github.com/huggingface/tokenizers) | Hugging Face | NA |
|  3  | [openai/tiktoken](https://github.com/openai/tiktoken) | OpenAI | [OpenAI 开源 GPT-2 的子词标记化神器——tiktoken，一个超级快的（Byte Pair Encoder，BPE）字节对编码 Python 库](https://zhuanlan.zhihu.com/p/592399697), [OpenAI 大模型高效 Tokenizer: tiktoken](https://zhuanlan.zhihu.com/p/631840697) |
|  4  | [mlc-ai/tokenizers-cpp](https://github.com/mlc-ai/tokenizers-cpp) | mlc-ai | 一个跨平移台的 C++ 分词器, 包装并 bind 了 HuggingFace 以及 sentencepiece, 并提供了最小的通用 C++ API. |
|  5  | [rust-tokenizers](https://github.com/guillaume-be/rust-tokenizers) | guillaume-be | Rust-tokenizers 是一个高性能的 tokenizer 库, 支持多种现代语言模型, 包括 WordPiece、Byte-Pair Encoding (BPE) 和 Unigram (SentencePiece) 模型. 这些 tokenizer 广泛应用于自然语言处理领域, 特别是在 transformer 架构中. |
|  6  | [OpenNMT/Tokenizer](https://github.com/OpenNMT/Tokenizer) | OpenNMT | 一个快速, 通用, 可定制的文本分词器, 支持 C++/Python, 依赖最小. 提供了多种功能, 包括可逆分词, 子词分词, 高级文本分段, 大小写管理以及保护序列等. |


## 4.2 可视化工具
-------

| 编号 | 工具 | 团队 | 详情 |
|:---:|:----:|:---:|:---:|
|  1  | [Gemma Scope](https://www.163.com/dy/article/J8GVD23005566WT8.html) | Google DeepMind | [Google DeepMind 发布大模型可视化工具 Gemma Scope](https://www.163.com/dy/article/J8GVD23005566WT8.html) |
|  2  | [Inspectus](https://github.com/labmlai/inspectus) | labmlai | [探索语言模型的奥秘 - 体验 Inspectus 的强大视觉洞察力](https://github.com/labmlai/inspectus) |
|  3  | [Transformer Explainer](https://github.com/poloclub/transformer-explainer) | poloclub |  通过互动可视化的方式了解生成式 AI 中 Transformer 的工作原理. 他在浏览器中实时运行一个 GPT-2 模型, 允许用户使用自己的文本进行试验, 并实时观察 Transformer 的内部组件和操作如何协同工作来预测下一个 Token, 在线体验地址 [transformer-explainer/](https://poloclub.github.io/transformer-explainer) |
|  4  | [BertViz](https://github.com/jessevig/bertviz) | Jessevig | 使用交互式的方式, 可视化 Transformer 语言模型 (如 BERT, GPT2) 的注意力机制, 可在 Jupyter 或 Colab 中运行, 通过简单的 Python API 支持大多数 Hugging Face 预训练模型 (如 BERT, GPT2, T5 等), BertViz 扩展了 Llion Jones 的 Tensor2Tensor 可视化工具, 提供了头视图, 模型视图, 神经元视图等多个不同的可视化方式, 每个视图都提供了一个独特的视角来了解注意力机制. 参见 [2019/04/11, Human-Computer Interaction (cs.HC), Visualizing Attention in Transformer-Based Language Representation Models](https://arxiv.org/abs/1904.02679) 和 [可解释 AI，在 Transformer 中可视化注意力（附代码）](https://mp.weixin.qq.com/s/vwJEBXCk6GrwN9-BVucoQA). |
|  5  | [LLM Visualization](https://github.com/bbycroft/llm-viz) | bbycroft | 将 Transformer 原理的详细细节通过交互可视化的方式一步步显示出来, 详细的展示了每一步的数学原理, 模型的网格结构, 参数构造的运行过程, 可以精确到每一步观察大模型运行的运算以及数据的变化. 作者的 [仓库 bbycroft/llm-viz](https://github.com/bbycroft/llm-viz) 以及 [在线演示地址 bbycroft.net/llm](https://bbycroft.net/llm), [@HansChanX](https://x.com/HansChanX) LLM 可视化演的中文翻译版本: [仓库 czhixin/llm-viz-cn](https://github.com/czhixin/llm-viz-cn) 以及 [示](https://llm-viz-cn.iiiai.com/llm). 其他 [Vaaaas/llm-viz-CN](https://github.com/Vaaaas/llm-viz-CN), 知乎报道 [矩阵模拟！Transformer 大模型 3D 可视化，GPT-3、Nano-GPT 每一层清晰可见](https://zhuanlan.zhihu.com/p/670287271) |
|  6  | [Machine-Learning-Tokyo/Interactive_Tools](https://github.com/Machine-Learning-Tokyo/Interactive_Tools) | Machine-Learning-Tokyo | 这个项目收集了各种用于机器学习、深度学习和数学的交互式工具. |
|  7  | [hahnyuan/LLM-Viewer](https://github.com/hahnyuan/LLM-Viewer) | hahnyuan | 一个可视化语言与学习模型 LLMs 并分析在不同硬件平台上性能的工具. 可以进行网络级分析, 考虑峰值内存消耗和总推理时间成本等因素. 使用 LLM-Viewer, 可以获取 LLM 推理和性能优化的宝贵见解. 可以在 Web 浏览器或者命令行(CLI) 工具中使用. 在线体验地址 [LLM-Viewer Web](http://llm-viewer.com). 参见论文 [LLM Inference Unveiled: Survey and Roofline Model Insights](https://arxiv.org/abs/2402.16363). |
|  8  | [A collection of my study notes for learners](https://www.k-a.in/notes.html) | k-a.in | Transformer/MoE Visualized |
|  9  | [CNN Explainer](https://poloclub.github.io/cnn-explainer) | poloclub | CNN Explainer: 卷积神经网络可视化, 可交互有细节, 卷积激活池化一目了然, 该项目用 TensorFlow.js 加载一个 10 层的预训练模型, 相当于在浏览器上跑一个 CNN 模型, 可以了解 CNN 的处理过程. 这个网页工具还可以实现交互,  只要点击其中任何一个格子—CNN 中的 "神经元", 就能显示它的输入、经过的变化, 甚至连每一次卷积运算都能看得清清楚楚. [poloclub/cnn-explainer](https://github.com/poloclub/cnn-explainer), 参见论文 [CNN Explainer: Learning Convolutional Neural Networks with Interactive Visualization](https://arxiv.org/abs/2004.15004) |
|  10 | [PyTorch 可视化工具介绍](https://zhuanlan.zhihu.com/p/658596017) | NA | PyTorch 可视化工具介绍. |
|  11 | [attentionmech/mav](https://github.com/attentionmech/mav) | attentionmech | 一款可视化大模型内部工作原理的工具, 帮助用户更好的理解和分析模型在生成文本时的内部魔偶快, 包括注意力分布, 预测概率等. 参见 [知识图谱 + 知识库 RAG 项目 Yuxi-Know 及大模型推理内部可视化工具 OpenMAV 实现拆解](https://zhuanlan.zhihu.com/p/1893668626810270690) |
|  12 | Logit Lens | NA | [2023/03/14, Eliciting Latent Predictions from Transformers with the Tuned Lens](https://arxiv.org/abs/2303.08112), [AlignmentResearch/tuned-lens](https://github.com/AlignmentResearch/tuned-lens) 和 [2025/02/24, LogitLens4LLMs: Extending Logit Lens Analysis to Modern Large Language Models](https://arxiv.org/abs/2503.11667), [zhenyu-02/LogitLens4LLMs](https://github.com/zhenyu-02/LogitLens4LLMs), 其他 [SullivanCastro/Logit-Lens](https://github.com/SullivanCastro/Logit-Lens), [arnab-api/Logit-Lens-Interpreting-GPT-2](https://github.com/arnab-api/Logit-Lens-Interpreting-GPT-2), [msakarvadia/Attentionlens](https://github.com/msakarvadia/Attentionlens) |
|  13 | [ReasonGraph](https://github.com/ZongqianLi/ReasonGraph) | NA | [ReasonGraph: Visualisation of Reasoning Paths](https://arxiv.org/abs/2503.03979) |
|  14 | [torchvista](https://github.com/sachinhosmani/torchvista) | 可视化交互式工具, 可以直接在 NodeBook 中可视化 PyTorch 模型的前向传播过程. 支持拖拽 / 缩放等交互, 并且可以在出现错误时进行部分可视化, 用户可直接点击节点查看参数和属性信息. |
|  15 | [NN-SVG/](http://alexlenail.me/NN-SVG) | NA | [介绍两款生成神经网络架构示意图的工具：NN-SVG 和 PlotNeuralNet](https://blog.csdn.net/weixin_41896770/article/details/132733991) |
|  16 | [Machine Learning Visualized](https://ml-visualized.com) | NA |
|  17 | [AttentionViz]() |  |
|  18 | [DODRIO](https://poloclub.github.io/dodrio) |[DODRIO: Exploring Transformer Models with Interactive Visualization](http://arxiv-download.xixiaoyao.cn/pdf/2103.14625.pdf), [【NLP】可交互的 Attention 可视化工具！我的 Transformer 可解释性有救了？](https://blog.csdn.net/fengdu78/article/details/116617948) |
|  19 | [AttentionViz]() | NA | [AttentionViz: A Global View of Transformer Attention](https://arxiv.org/abs/2305.03210), [CSDN-AttentionViz: A Global View of Transformer Attention 论文学习，可视化、了解 Transformer 中的注意力机制](https://blog.csdn.net/weixin_48334973/article/details/137968878), [AttentionViz: 一个可视化 Transformer 注意力机制的强大工具](https://www.dongaigc.com/a/attentionviz-visualizing-transformer-attention)<br>AttentionViz 的核心理念是将 Transformer 模型中用于计算注意力的查询 (query) 和键 (key) 向量进行联合嵌入可视化. 与以往的注意力可视化技术不同, AttentionViz 能够分析多个输入序列的全局模式, 为研究人员提供了一个前所未有的视角来理解模型的内部运作. |
|  20 | [nndeploy](https://github.com/nndeploy/nndeploy) | NNDeploy | 基于工作流的多平台 AI 部署工具, 简化 AI 模型的部署流程. [nndeploy: 易用、高性能、支持多端的 AI 推理部署框架](https://zhuanlan.zhihu.com/p/1913542783903438653). |


## 4.3 评测平台
-------

| 编号 | 工具 | 团队 | 详情 |
|:---:|:----:|:---:|:---:|
|  1  | [EleutherAI/lm-evaluation-harness](https://github.com/EleutherAI/lm-evaluation-harness) | EleutherAI | 统一的测试框架, 用于测试生成式语言模型在各种不同的评估任务上的表现. 它提供了超过 60 个标准的学术基准测试, 支持多种模型类型和推理服务器, 并且具有灵活的配置和扩展性. 该项目被广泛用于论文研究和工业应用. |



# 5 基础理论
-------


## 5.1 基础理论汇总
-------


| 项目 | 描述 |
|:---:|:---:|
| [Hannibal046/Awesome-LLM](https://github.com/Hannibal046/Awesome-LLM) | 一份关于大型语言模型的精选论文列表, 尤其是与 ChatGPT 相关的论文. 它还包含用于 LLM 培训的框架、要部署 LLM 的工具、有关所有公开可用的 LLM 检查点和 API 的 LLM 课程和教程. |
| [dair-ai/ML-Papers-of-the-Week](https://github.com/dair-ai/ML-Papers-of-the-Week) | 每周 AI 热点论文. |
| [Lightning-AI/litgpt](https://github.com/Lightning-AI/litgpt) | LitGPT 是一个强大的工具, 使开发人员能够利用 LLM 的最新进展. 其全面的功能、用户友好性以及不断发展的社区使其成为构建和部署 LLM 应用程序的理想选择. |
| [aishwaryanr/awesome-generative-ai-guide](https://github.com/aishwaryanr/awesome-generative-ai-guide) | 关于人工智能的开源综合站点, 汇总了相关的论文以及课程, 以及业界前沿资讯. |
| [LuckyyySTA/Awesome-LLM-hallucination](https://github.com/LuckyyySTA/Awesome-LLM-hallucination) | 这项研究调查了与大型语言模型幻觉相关的论文. 包括相关的调查或分析论文、幻觉原因、幻觉检测和基准、幻觉缓解, 以及该领域的挑战和开放性问题. |
| [Datawhale](https://github.com/datawhalechina) | Datawhale 是一个专注于数据科学与 AI 领域的开源组织, 汇集了众多领域院校和知名企业的优秀学习者, 聚合了一群有开源精神和探索精神的团队成员. Datawhale 以 "for the learner, 和学习者一起成长" 为愿景, 鼓励真实地展现自我、开放包容、互信互助、敢于试错和勇于担当. 同时 Datawhale 用开源的理念去探索开源内容、开源学习和开源方案, 赋能人才培养, 助力人才成长, 建立 起人与人, 人与知识, 人与企业和人与未来的联结. |
| [zjunlp/LLMAgentPapers](https://github.com/zjunlp/LLMAgentPapers) | 关于大型语言模型代理的必读论文. |
| [mli/paper-reading](https://github.com/mli/paper-reading) | 深度学习论文精读. |
| [mlabonne/llm-course](https://github.com/mlabonne/llm-course) | 大型语言模型课程. |
| [AmadeusChan/Awesome-LLM-System-Papers](https://github.com/AmadeusChan/Awesome-LLM-System-Papers) | 收集了大语言模型系统论文, 涵盖了算法 - 系统协同设计, 推理系统相关的内容. |
| [tensorchord/Awesome-LLMOps](https://github.com/tensorchord/Awesome-LLMOps) | 精选了 LLMOps 工具, 收集和整理了大量优秀的 LLMOps 工具, 涵盖了模型, 服务, 安全, 可观测性等多个领域. |
| [HqWu-HITCS/Awesome-Chinese-LLM](https://github.com/HqWu-HITCS/Awesome-Chinese-LLM) | 整理开源的中文大语言模型, 主要关注规模较小, 可私有化部署, 训练成本较低的模型. 包括底座模型, 垂直领域微调以及应用, 数据集合教程等内容 |
| [km1994/nlp_paper_study](https://github.com/km1994/nlp_paper_study) | 该仓库主要记录 NLP 算法工程师相关的顶会论文研读笔记. |
| [NexaAI/Awesome-LLMs-on-device](https://github.com/NexaAI/Awesome-LLMs-on-device) | 汇总了端侧 AI 的相关架构和优化技术, 包括前言的论文研究. |
| [wdndev/llm_interview_note](https://github.com/wdndev/llm_interview_note) | 主要记录大语言大模型 (LLMs) 算法(应用) 工程师相关的知识及面试题. |
| [冬于的博客 - Transformer/BERT / 实战](https://ifwind.github.io/2021/08/31/Transformer-BERT - 实战) | 通过大量图讲述 Transformer 架构 |
| [浅显易懂地介绍 llm.c [译]](https://baoyu.io/translations/llm/explaining-llm-c-in-layman-terms) | [Explainable Language Models: Existing and Novel Approaches](https://twitter.com/karpathy/status/1778153659106533806) 的译文, 参见 [karpathy/llm.c](https://github.com/karpathy/llm.c). |
| [DefTruth/Awesome-LLM-Inference](https://github.com/DefTruth/Awesome-LLM-Inference) | 收集了大量 LLM 推理相关的论文和仓库, 涵盖了并行计算, 量化压缩, 注意力机制优化, 上下文管理等. |
| [SylphAI-Inc/llm-engineer-handbook](https://github.com/SylphAI-Inc/llm-engineer-handbook) | NA |


## 5.2 Survey
-------


[AIWIKI.AI, the AI and ML Wiki](https://aiwiki.ai/wiki/AI_ML_Wiki)

[2025 年 AI 领域最值得关注的博主和一手信息源盘点 - 强化学徒的文章 - 知乎](https://github.com/kaixindelele/2025-Awesome-AI-Bloggers)


| 时间 | 分类 | Survey | 作者 | GitHub | 描述 |
|:---:|:----:|:------:|:---:|:------:|:----:|
| 2024/03/01 | 综述 | [NiuTrans/ABigSurveyOfLLMs](https://github.com/NiuTrans/ABigSurveyOfLLMs) | [NiuTrans](https://github.com/NiuTrans/ABigSurveyOfLLMs) | [NiuTrans](https://github.com/NiuTrans/ABigSurveyOfLLMs) | 一个关于大语言模型的综合性调研集合, 包含 150 多篇关于 LLM 的调研论文. 这些调研涵盖了 LLM 的各个方面, 包含通用调研, Transformer, 对齐, 提示学习, 上下文学习, 推理链, 提示工程, 数据, 评估, 社会问题, 安全性, 幻觉, 属性, 高效 LLM, 学习方法, 多模态 LLM, 基于知识的 LLM, 检索增强型 LLM, 知识编辑, LLM 扩展, LLM 与工具, LLM 与交互, 长序列 LLM, 以及 LLM 在教育, 法律, 医疗, 游戏, NLP 任务, 软件工程, 推荐系统, 图谱等领域的应用. |
| 2024/01/16 | 多模态 | [A Survey of Resource-efficient LLM and Multimodal Foundation Models](https://arxiv.org/abs/2401.08092) | Mengwei Xu | [UbiquitousLearning](https://github.com/UbiquitousLearning/Efficient_Foundation_Model_Survey) | 一篇关于资源高效的大模型和多模态基础模型的综述论文. 论文涵盖了算法和系统两个方面的创新, 包括了高校的模型架构, 训练算法, 推理算法和模型压缩等内容. |
| 2024/04/18 | 效率提升 | [The Efficiency Spectrum of Large Language Models: An Algorithmic Survey](https://arxiv.org/abs/2312.00678) | Tianyu Ding | [tding1](https://github.com/tding1/Efficient-LLM-Survey) | 一篇关于提供大语言模型效率的综合性调查论文, 全面回顾了旨在提高 LLM 效率的算法, 涵盖了扩展定律, 数据利用, 架构创新, 训练和调优策略以及推理计划等. [知乎 - 无影寺 -【LLM / 大模型】大语言模型效率谱：算法综述(](https://zhuanlan.zhihu.com/p/671376104) |
| 2024/04/22 | 效率提升 | [A Survey on Efficient Inference for Large Language Models](https://arxiv.org/abs/2404.14294) | Zixuan Zhou | NA | 1. [如何加速大模型推理？万字综述全面解析大语言模型高效推理技术](https://www.sohu.com/a/790365299_121119001)<br>2. [知乎 -- 罗清雨 -- 大语言模型高效推理综述](https://zhuanlan.zhihu.com/p/707685591)<br>3. [LLM 推理加速调研](https://zhuanlan.zhihu.com/p/699776257) |
| 2024/05/23 | 效率提升 | [Efficient Large Language Models: A Survey](https://arxiv.org/abs/2312.03863) | Zhongwei Wan | [AIoT-MLSys-Lab](https://github.com/AIoT-MLSys-Lab/Efficient-LLMs-Survey) | 本文对高效 LLMs 研究的发展进行了系统而全面的回顾, 并将文献整理成由三个主要类别组成的分类法, 从模型中心、数据中心和框架中心的角度涵盖了不同但相互关联的高效 LLMs 主题, 并且从以模型为中心和以数据为中心的角度, 回顾了 LLMs 的算法层面和系统层面的高效技术. 详细介绍了每个分类下的具体技术, 如: 量化, 剪枝, 知识蒸馏, 数据选择, 提示工程等 <br>1. [知乎 -- 黄浴 -- 高效大语言模型：综述](https://zhuanlan.zhihu.com/p/671710012)<br>2. [知乎 -- 磐石 -- 大模型高效推理 I 推理技术框架总结](https://zhuanlan.zhihu.com/p/696850285)<br>3. [知乎 -- 享享学 AI-- 大模型 LLM 微调技术方法汇总！](https://zhuanlan.zhihu.com/p/673675939)<br>4. [CSDN-rommel rain-Efficient Large Language Models: A Survey](https://blog.csdn.net/qq_52024723/article/details/143415741) |
| 2024/05/17 | 效率提升 <br> 多模态 | [Efficient Multimodal Large Language Models: A Survey](https://arxiv.org/abs/2405.10739), [CSDN - 星夜 Zn-Efficient Multimodal Large Language Models: A Survey (高效多模态大型语言模型综述 - 全文翻译)](https://blog.csdn.net/qq_29868553/article/details/144163118), [知乎 - 吕阿华 -【MLLM 研究综述】《Efficient Multimodal Large Language Models: A Survey》——腾讯最新多模态大模型综述](https://zhuanlan.zhihu.com/p/701495021) |
| 2023/06/23 | 多模态 | [A Survey on Multimodal Large Language Models](https://arxiv.org/abs/2306.13549) | Shukang Yin | [BradyFU](https://github.com/BradyFU/Awesome-Multimodal-Large-Language-Models) | 本综述中主要介绍了多模态幻觉、多模态上下文学习 (Multimodal InContext Learning，M-ICL)、多模态思维链(Multimodal Chain of Thought，M-CoT) 和 LLM 辅助的视觉推理 (LLM-Aided Visual Reasoning，LAVR) 等. |
| 2024/07/26 | 模型压缩 | [Comprehensive Study on Performance Evaluation and Optimization of Model Compression: Bridging Traditional Deep Learning and Large Language Models](https://arxiv.org/abs/2407.15904) | Aayush Saxena | [Comprehensive](https://arxiv.org/abs/2407.15904) | 近年来, 深度学习模型在大多数行业都取得了巨大成功. 这些模型的发展还导致模型大小和能源需求增加, 使其难以在低计算设备上的生产环境中进行部署. 全球互联设备数量的增加保证了压缩模型可以轻松部署在本地设备上, 但计算容量和电源可访问性较低. 不同的研究人员提出了广泛的解决方案来减小此类模型的大小和复杂性, 其中突出的是权重量化、参数修剪、网络修剪、低秩表示、权重共享、神经架构搜索、知识蒸馏等. 在这项研究工作中, 我们调查了使用量化和修剪技术进行压缩的各种训练有素的深度学习模型的性能影响. 我们在图像分类、对象检测、语言模型和基于生成模型的问题陈述中使用的常用深度学习模型上实施了量化和剪枝压缩技术. 我们还探讨了各种大型语言模型在量化和低秩适应后的性能. 我们对所有相关问题陈述使用了标准评估指标(模型的大小、准确性和推理时间), 并通过讨论挑战和未来的工作来总结本文. |
| 2024/06/04 | 投机 | [Unlocking Efficiency in Large Language Model Inference:A Comprehensive Survey of Speculative Decoding](https://arxiv.org/abs/2401.07851) | Heming Xia | [hemingkx/SpeculativeDecodingPapers](https://github.com/hemingkx/SpeculativeDecodingPapers) | [COLING 2025 Tutorial:Speculative Decoding for Efficient LLM Inference](https://speculative-decoding.github.io), [知乎 - LLM 推理加速新范式！推测解码（Speculative Decoding）最新综述](https://zhuanlan.zhihu.com/p/678404136) |
| 2025/06/16 | 离散扩散 (Discrete Diffusion) | [Discrete Diffusion in Large Language and Multimodal Models: A Survey](https://arxiv.org/pdf/2506.13759) | xML 团队 | [LiQiiiii/DLLM-Survey](https://github.com/LiQiiiii/DLLM-Survey) | 本文全面综述了基于离散扩散范式的大语言与多模态模型, 揭示其通过并行解码和去噪策略实现加速推理与精细控制的核心机制, 构建了涵盖理论框架、实现技术与应用场景的完整技术体系. 本文系统梳理了基于离散扩散的大语言模型(dLLMs) 和多模态语言模型 (dMLLMs) 的技术发展脉络. 与传统的自回归模型相比, 这类模型通过并行解码机制和去噪生成策略, 实现了高达 10 倍的推理加速, 同时在细粒度输出控制和动态感知响应方面展现出独特优势. 研究揭示了该领域发展的两大驱动力: 一是自回归模型积累的海量数据和基础设施, 二是吸收状态扩散、转移矩阵优化等数学模型的突破. 论文从历史沿革、数学框架、模型分类三个维度构建技术体系, 特别阐述了全注意力机制与多标记预测的协同优化方法, 以及蛋白质序列生成等跨领域应用的实现路径. 实验分析表明, 当前领先的 d(M)LLMs 在保持同等生成质量的前提下, 通过并行解码实现了 3-10 倍的推理加速. 特别是工业级闭源模型与开源学术模型的双轨发展, 验证了该范式的实际部署价值. 研究最后指出硬件适配优化和高效训练策略将成为未来突破的关键方向. |
| 2025/08/13 | 效率提升 | [Speed Always Wins: A Survey on Efficient Architectures for Large Language Models](https://arxiv.org/abs/2508.09834) | 上海 AI Lab | [Awesome-Efficient-Arch](https://github.com/weigao266/Awesome-Efficient-Arch) | [唯快不破：上海 AI Lab 82 页综述带你感受 LLM 高效架构的魅力](https://www.jiqizhixin.com/articles/2025-08-25-12) |
| 2025/04/12 | 推理系统 | [A Survey of Frontiers in LLM Reasoning: Inference Scaling, Learning to Reason, and Agentic Systems](https://arxiv.org/abs/2504.09037) | 新加坡研究机构与高校 | 这篇综述的核心观点是, LLM 的推理研究正经历两大转变:<br> 一是从 "推理时扩展"(Inference Scaling)向 "学习推理"(Learning to Reason)的范式转变, 即从依赖提示工程和复杂解码策略, 转向通过专门的训练来内化模型的推理能力;<br> 二是从 "单一模型"(Standalone LLMs)向 "代理系统"(Agentic Systems)的架构演进, 即从单个 LLM 独立解决问题, 演变为利用外部工具或多个智能体协作来完成复杂任务. 论文通过其独特的二维分类法, 为理解这一快速发展的领域提供了一个全面的分析框架. 参见 [新加坡研究机构与高校发布最新 Reasoning 综述，从推理扩展、学习推理到 Agent 系统](https://blog.csdn.net/qq_27590277/article/details/147262739) |


[Mobile Edge Intelligence for Large Language Models: A Contemporary Survey](https://arxiv.org/abs/2407.18921)
[Edge Intelligence: Architectures, Challenges, and Applications](https://arxiv.org/abs/2003.12172)
[A Survey on Model Compression for Large Language Models](https://arxiv.org/abs/2308.07633)
| NA | NA | [Towards Efficient Generative Large Language Model Serving: ASurvey from Algorithms to Systems](https://arxiv.org/abs/2312.15234) | NA | NA | [知乎 -- 路漫漫独求索 --LLM 推理加速技术简介](https://zhuanlan.zhihu.com/p/691360124)
[A Survey of Techniques for Maximizing LLM Performance]()
[Large Language Models: A Survey](https://arxiv.org/abs/2402.06196)
[A Survey of Large Language Models](https://arxiv.org/abs/2303.18223)
[Beyond Efficiency: A Systematic Survey of Resource-Efficient Large Language Models](https://arxiv.org/abs/2401.00625)
[The Efficiency Spectrum of Large Language Models: An Algorithmic Survey](https://arxiv.org/abs/2312.00678)



[A Survey on Mixture of Experts](https://arxiv.org/abs/2407.06204)
[MoE-Infinity: Offloading-Efficient MoE Model Serving](https://arxiv.org/abs/2401.14361)
[OLMoE: Open Mixture-of-Experts Language Models](https://arxiv.org/abs/2409.02060)
[Deja Vu: Contextual Sparsity for Efficient LLMs at Inference Time](https://arxiv.org/abs/2310.17157)


## 5.3 paper plaza
-------


| 时间 | 分类 | Survey | 作者 | GitHub | 描述 |
|:---:|:----:|:------:|:---:|:------:|:----:|
| 2024/09/23 | 日常论文精选 | [metame-ai/awesome-llm-plaza](https://github.com/metame-ai/awesome-llm-plaza) | [metame-ai](https://github.com/metame-ai/awesome-llm-plaza) | [awesome-llm-plaza](https://github.com/metame-ai/awesome-llm-plaza) | 日常论文精选 |
| 2024/10/25 | 日常论文精选 | [xianshang33/llm-paper-daily](https://github.com/xianshang33/llm-paper-daily) | [xianshang33](https://github.com/xianshang33/llm-paper-daily) | [xianshang33/llm-paper-daily](https://github.com/xianshang33/llm-paper-daily) | 日常论文精选 |
| 2024/11/25 | 日常论文速递 | NA | NA | [叶子的技术碎碎念 - 每周 AI 论文速递](http://leafw.cn) | NA |











<br>

*   本作品 / 博文 ([AderStep - 紫夜阑珊 - 青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚 (gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用 <a rel="license"href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt=" 知识共享许可协议 "style="border-width:0"src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png"/></a><a rel="license"href="http://creativecommons.org/licenses/by-nc-sa/4.0/"> 知识共享署名 - 非商业性使用 - 相同方式共享 4.0 国际许可协议 </a> 进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名 [成坚 gatieme](http://blog.csdn.net/gatieme) (包含链接: http://blog.csdn.net/gatieme), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   ** 转载请务必注明出处, 谢谢, 不胜感激 **
<br>
