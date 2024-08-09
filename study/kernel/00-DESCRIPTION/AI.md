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



# 1 自动化故障定位、修复和分析
-------

| 论文 | 描述 |
|:---:|:----:|
| [A Unified Debugging Approach via LLM-Based Multi-Agent Synergy](https://arxiv.org/abs/2404.17153) | 大型语言模型 (LLM) 在自动调试方面显示出了巨大潜力. 然而, 我们发现传统和基于 LLM 的调试工具面临着三个挑战: 1) 故障定位的上游不完美会影响下游的修复; 2) 处理复杂逻辑错误的不足; 3) 忽略程序上下文. 作者提出了第一个自动化的、统一的调试框架——FixAgent, 通过 LLM 代理协同作用. FixAgent 可以执行端到端的故障定位、修复和分析. LLM 可以从人类开发人员在调试中认可的通用软件工程原则中受益, 如 rubber duck debugging, 从而更好地理解程序功能和逻辑错误. 因此, 我们创建了三个受 rubber duck debugging 启发的设计来解决这些挑战. 它们是代理专业化和协同作用、关键变量跟踪和程序上下文理解, 这些要求 LLM 提供明确的解释, 并迫使它们关注关键的程序逻辑信息. 在广泛使用的 QuixBugs 数据集上的实验表明, FixAgent 正确修复了 80 个中的 79 个错误, 其中有 9 个以前从未被修复过. 即使没有故障位置信息和少于 0.6% 的采样时间, 它也比 CodeFlaws 上表现最佳的修复工具更可信地修补了 1.9 倍的缺陷. 平均而言, FixAgent 相对于使用不同 LLM 的基础模型平均增加了约 20% 的可信和正确的修复, 显示出我们设计的有效性. 此外, FixAgent 的正确率达到了惊人的 97.26%, 表明 FixAgent 有可能克服现有方法的过度拟合问题. |



# 2 AI-OS
-------

| 论文 | 描述 |
|:---:|:----:|
| [LLM as OS, Agents as Apps: Envisioning AIOS, Agents and the AIOS-Agent Ecosystem](https://arxiv.org/abs/2312.03815) | 本文设想了一个革命性的 AIOS-Agent 生态系统, 其中大型语言模型 (LLM) 充当 (人工) 智能操作系统 (IOS, 或 AIOS)——一个 "有灵魂" 的操作系统. 在此基础上, 开发了各种 LLM 基于 AI 代理的应用程序 (Agents, 或 AAP), 丰富了 AIOS-Agent 生态系统, 标志着传统 OS-APP 生态系统的范式转变. 作者设想 LLM 其影响将不仅限于人工智能应用层面, 相反, 它将彻底改变计算机系统、架构、软件和编程语言的设计和实现, 其特点是几个主要概念: LLM 操作系统 (系统级)、代理即应用程序 (应用程序级)、自然语言作为编程接口 (用户级) 和工具即设备 / 库 (硬件 / 中间件级). 我们首先介绍传统操作系统的架构. 然后, 我们通过 "LLMas OS(LLMOS)" 正式化 AIOS 的概念框架, 将 AIOS 与传统操作系统进行类比: LLM 将上下文窗口比作操作系统内核, 将上下文窗口比作内存, 将外部存储比作文件系统, 将硬件工具比作外围设备, 将软件工具比作编程库, 将用户提示比作用户命令. 随后, 我们引入了新的 AIOS-Agent 生态系统, 用户可以使用自然语言轻松编程 Agent 应用程序 (AAP), 使软件开发民主化, 这与传统的 OS-APP 生态系统不同. 在此之后, 我们将探索代理应用程序的多样化范围. 我们深入研究了单智能体和多智能体系统, 以及人机交互. 最后, 借鉴传统 OS-APP 生态的洞察, 提出了 AIOS-Agent 生态演进的路线图.  该路线图旨在指导未来的研究和开发, 建议 AIOS 及其代理应用程序的系统性进展. |

# 3 AI-IN-OS
-------

[Mooncake: Kimi's KVCache-centric Architecture for LLM Serving](https://arxiv.org/abs/2407.00079)
[如何利用Kimi解读Kimi的KVCache技术细节](https://blog.csdn.net/kunpengtingting/article/details/140202126)

[Attention is All you Need 全文翻译](https://zhuanlan.zhihu.com/p/682007654)

[大模型思维链 (Chain-of-Thought) 技术原理](https://www.zhihu.com/tardis/bd/art/629087587)
[【LLM 系列 - 07】Chain-of-Thought Prompting Elicits Reasoning in Large Language Models](https://zhuanlan.zhihu.com/p/616838483)
[LLM 之 Prompt 工程技能总结](https://zhuanlan.zhihu.com/p/692618896)


[LLMs-Agent 论文: PAL, 2023, Luyu Gao et al., CMU](https://zhuanlan.zhihu.com/p/664408840)


[LLM 自省系列(1)ReAct: Synergizing Reasoning and Acting in Language Model](https://zhuanlan.zhihu.com/p/650686547)
[[论文笔记]REACT: SYNERGIZING REASONING AND ACTING IN LANGUAGE MODELS](https://blog.csdn.net/yjw123456/article/details/139102046)


## 3.1 云侧大模型
-------


## 3.2 端侧大模型
-------

| 编号 | 模型 | 团队 | 详情 | 实例 |
|:----:|:----:|:---:|:---:|:---:|
|  1  | Apple Intelligence | Apple | [Apple Intelligence Foundation Language Models](https://arxiv.org/abs/2407.21075), [苹果Private Cloud Compute: 云端AI隐私技术的新前沿(翻译版)](https://zhuanlan.zhihu.com/p/703584080) |
|  2  | Gemma-2-2B | Google DeepMind | NA | NA | [aistudio.google](https://aistudio.google.com/app/prompts/new_chat?model=gemma-2-2b-it), [HuggingFace](https://huggingface.co/collections/google/gemma-2-2b-release-66a20f3796a2ff2a7c76f98f), [HuggingFace/mlc-ai](https://huggingface.co/spaces/mlc-ai/webllm-simple-chat), [mlc-ai/deploy/android](https://llm.mlc.ai/docs/deploy/android.html)
|  3  | Phi-2 | NA | NA | NA |
|  4  | MiNiCPM-2B | NA | NA | NA |
|  5  | Qwen2-1.5B | NA | NA | NA |


## 3.3 推理框架
-------


[大模型推理框架概述](https://zhuanlan.zhihu.com/p/659792625)

[大模型推理部署：LLM 七种推理服务框架总结](https://blog.csdn.net/m0_59596990/article/details/135311245)

[29 种本地部署大模型和调用的工具平台分类与总结](https://blog.csdn.net/l35633/article/details/138379452)

| 编号 | 推理框架 | 团队 | 介绍 |
|:---:|:-------:|:---:|
| 1 | [vLLM](https://github.com/vllm-project/vllm) | UC Berkeley | vLLM 是一个开源的大模型推理加速框架, 通过 PagedAttention 高效地管理 attention 中缓存的张量, 实现了比 HuggingFace Transformers 高 14-24 倍的吞吐量. PagedAttention 是 vLLM 的核心技术, 它解决了 LLM 服务中内存的瓶颈问题. 传统的注意力算法在自回归解码过程中, 需要将所有输入 Token 的注意力键和值张量存储在 GPU 内存中, 以生成下一个 Token. 这些缓存的键和值张量通常被称为 KV 缓存. [vLLM(二)架构概览](https://zhuanlan.zhihu.com/p/681716326) |
| 2 | [Text Generation Inference(TGI)](https://github.com/huggingface/text-generation-inference) | HuggingFace | Text Generation Inference(TGI)是 HuggingFace 推出的一个项目, 作为支持 HuggingFace Inference API 和 Hugging Chat 上的 LLM 推理的工具, 旨在支持大型语言模型的优化推理.  |
| 3 | [FasterTransformer](https://github.com/NVIDIA/FasterTransformer) | NVIDIA | NVIDIA FasterTransformer (FT) 是一个用于实现基于 Transformer 的神经网络推理的加速引擎. 它包含 Transformer 块的高度优化版本的实现, 其中包含编码器和解码器部分. 使用此模块, 您可以运行编码器 - 解码器架构模型 (如：T5)、仅编码器架构模型(如：BERT) 和仅解码器架构模型 (如: GPT) 的推理. FT 框架是用 C++/CUDA 编写的, 依赖于高度优化的 cuBLAS、cuBLASLt 和 cuSPARSELt 库, 这使您可以在 GPU 上进行快速的 Transformer 推理.  |
| 4 | [DeepSpeed-Mll](https://github.com/microsoft/DeepSpeed-MII) | MicroSoft | DeepSpeed-MII 是 DeepSpeed 的一个新的开源 Python 库, 旨在使模型不仅低延迟和低成本推理, 而且还易于访问.  |
| 5 | [FlexFlow Server](https://github.com/flexflow/FlexFlow) | Leland Stanford Junior University | 一个开源编译器和分布式系统, 用于低延迟、高性能 LLM 服务.  |
| 6 | [LMDeploy](https://github.com/InternLM/lmdeploy) |  [MMDeploy](https://github.com/open-mmlab/mmdeploy) 和 [MMRazor](https://github.com/open-mmlab/mmrazor) 团队联合开发 | 一个 C++ 和 Python 库, 用于使用 Transformer 模型进行高效推理. |
| 7 | [CTranslate2](https://github.com/OpenNMT/CTranslate2) | 一个快速推理引擎, 适用于 Transformer 模型, 提供高效的推理能力和性能优化技术. 在本文中, 我们将探索与 CTranslate2 相关的关键功能、模型类型、安装过程、基准测试以及其他资源. |
| 8 | OpenLLM | OpenLLM | 用于在生产中操作大型语言模型 (LLM) 的开放平台. 为核心模型添加 adapter 并使用 HuggingFace Agents, 尤其是不完全依赖 PyTorch |
| 9 | Ray Serve | NA | 一个可扩展的模型服务库, 用于构建在线推理 API. Serve 与框架无关, 因此你可以使用单个工具包来服务深度学习模型中的所有内容. 稳定的 Pipeline 和灵活的部署, 它最适合更成熟的项目 |
| 10 | MLC LLM | NA | MLC LLM 是一种通用部署解决方案. 可在客户端 (边缘计算), 例如 Android 或 iPhone 平台上, 本地部署 LLM. [MLC LLM：将LLMs部署到消费类硬件的优势、挑战以及解决方案](https://blog.csdn.net/FrenzyTechAI/article/details/132340135) |

# 4 分析工具
-------


| 编号 | 工具 | 团队 | 详情 |
|:---:|:----:|:---:|:---:|
|  1  | [Gemma Scope](https://www.163.com/dy/article/J8GVD23005566WT8.html) | Google DeepMind | [Google DeepMind 发布大模型可视化工具 Gemma Scope](https://www.163.com/dy/article/J8GVD23005566WT8.html) |
|  2  | [Inspectus](https://github.com/labmlai/inspectus) | NA | [探索语言模型的奥秘 - 体验 Inspectus 的强大视觉洞察力](https://github.com/labmlai/inspectus) |
|  3  | [Transformer Explainer](https://github.com/poloclub/transformer-explainer) | NA |  通过互动可视化的方式了解生成式 AI 中 Transformer 的工作原理. 他在浏览器中实时运行一个 GPT-2 模型, 允许用户使用自己的文本进行试验, 并实时观察 Transformer 的内部组件和操作如何协同工作来预测下一个 Token, 在线体验地址 [transformer-explainer/](https://poloclub.github.io/transformer-explainer) |

# 5 基础理论
-------


[Hannibal046/Awesome-LLM](https://github.com/Hannibal046/Awesome-LLM) 一份关于大型语言模型的精选论文列表, 尤其是与 ChatGPT 相关的论文. 它还包含用于 LLM 培训的框架、要部署 LLM 的工具、有关所有公开可用的 LLM 检查点和 API 的 LLM 课程和教程.

[dair-ai/ML-Papers-of-the-Week](https://github.com/dair-ai/ML-Papers-of-the-Week) 每周 AI 热点论文.























<br>

*   本作品 / 博文 ([AderStep - 紫夜阑珊 - 青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚 (gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用 <a rel="license"href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt=" 知识共享许可协议 "style="border-width:0"src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png"/></a><a rel="license"href="http://creativecommons.org/licenses/by-nc-sa/4.0/"> 知识共享署名 - 非商业性使用 - 相同方式共享 4.0 国际许可协议 </a> 进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名 [成坚 gatieme](http://blog.csdn.net/gatieme) (包含链接: http://blog.csdn.net/gatieme), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   ** 转载请务必注明出处, 谢谢, 不胜感激 **
<br>
