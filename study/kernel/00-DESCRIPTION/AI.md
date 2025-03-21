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

| 概要 | 论文/链接 | 描述 |
|:---:|:--------:|:----:|
| 自动化故障定位、修复和分析 | [A Unified Debugging Approach via LLM-Based Multi-Agent Synergy](https://arxiv.org/abs/2404.17153) | 大型语言模型 (LLM) 在自动调试方面显示出了巨大潜力. 然而, 我们发现传统和基于 LLM 的调试工具面临着三个挑战: 1) 故障定位的上游不完美会影响下游的修复; 2) 处理复杂逻辑错误的不足; 3) 忽略程序上下文. 作者提出了第一个自动化的、统一的调试框架——FixAgent, 通过 LLM 代理协同作用. FixAgent 可以执行端到端的故障定位、修复和分析. LLM 可以从人类开发人员在调试中认可的通用软件工程原则中受益, 如 rubber duck debugging, 从而更好地理解程序功能和逻辑错误. 因此, 我们创建了三个受 rubber duck debugging 启发的设计来解决这些挑战. 它们是代理专业化和协同作用、关键变量跟踪和程序上下文理解, 这些要求 LLM 提供明确的解释, 并迫使它们关注关键的程序逻辑信息. 在广泛使用的 QuixBugs 数据集上的实验表明, FixAgent 正确修复了 80 个中的 79 个错误, 其中有 9 个以前从未被修复过. 即使没有故障位置信息和少于 0.6% 的采样时间, 它也比 CodeFlaws 上表现最佳的修复工具更可信地修补了 1.9 倍的缺陷. 平均而言, FixAgent 相对于使用不同 LLM 的基础模型平均增加了约 20% 的可信和正确的修复, 显示出我们设计的有效性. 此外, FixAgent 的正确率达到了惊人的 97.26%, 表明 FixAgent 有可能克服现有方法的过度拟合问题. |
| AI 辅助 Linux 补丁测试  | [Testing AI-enhanced reviews for Linux patches](https://lwn.net/Articles/987319) | 在 2024 年 7 月的 Netdev 0x18 大会上的一次演讲中, Brandeburg 概述了一个使用机器学习来审查包含发送到 netdev 邮件列表的补丁的电子邮件的实验. 大型语言模型(LLMs) 不会很快取代人工审阅者, 但它们可能是一个有用的补充, 可以帮助人类专注于更深入的审阅, 而不是简单的规则违规行为. 参见 [AI Enhanced Reviews for Linux Networking](https://netdevconf.info/0x18/docs/netdev-0x18-paper26-talk-slides/netdev_0x18_AI_Reviews.pdf) |


## 1.2 OS4AI
-------

| 论文 | 描述 |
|:---:|:----:|
| [LLM as OS, Agents as Apps: Envisioning AIOS, Agents and the AIOS-Agent Ecosystem](https://arxiv.org/abs/2312.03815) | 本文设想了一个革命性的 AIOS-Agent 生态系统, 其中大型语言模型 (LLM) 充当 (人工) 智能操作系统 (IOS, 或 AIOS)——一个 "有灵魂" 的操作系统. 在此基础上, 开发了各种 LLM 基于 AI 代理的应用程序 (Agents, 或 AAP), 丰富了 AIOS-Agent 生态系统, 标志着传统 OS-APP 生态系统的范式转变. 作者设想 LLM 其影响将不仅限于人工智能应用层面, 相反, 它将彻底改变计算机系统、架构、软件和编程语言的设计和实现, 其特点是几个主要概念: LLM 操作系统 (系统级)、代理即应用程序 (应用程序级)、自然语言作为编程接口 (用户级) 和工具即设备 / 库 (硬件 / 中间件级). 我们首先介绍传统操作系统的架构. 然后, 我们通过 "LLMas OS(LLMOS)" 正式化 AIOS 的概念框架, 将 AIOS 与传统操作系统进行类比: LLM 将上下文窗口比作操作系统内核, 将上下文窗口比作内存, 将外部存储比作文件系统, 将硬件工具比作外围设备, 将软件工具比作编程库, 将用户提示比作用户命令. 随后, 我们引入了新的 AIOS-Agent 生态系统, 用户可以使用自然语言轻松编程 Agent 应用程序 (AAP), 使软件开发民主化, 这与传统的 OS-APP 生态系统不同. 在此之后, 我们将探索代理应用程序的多样化范围. 我们深入研究了单智能体和多智能体系统, 以及人机交互. 最后, 借鉴传统 OS-APP 生态的洞察, 提出了 AIOS-Agent 生态演进的路线图.  该路线图旨在指导未来的研究和开发, 建议 AIOS 及其代理应用程序的系统性进展. |



# 2 模型
-------


### 2.1 大模型汇总
-------

### 2.1.1 大模型汇总
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


### 2.1.2 云侧大模型
 -------


### 2.1.3 端侧大模型
-------

| 编号 | 模型 | 团队 | 详情 | 实例 |
|:----:|:----:|:---:|:---:|:---:|
|  1  | Apple Intelligence | Apple | [Apple Intelligence Foundation Language Models](https://arxiv.org/abs/2407.21075), [苹果 Private Cloud Compute: 云端 AI 隐私技术的新前沿 (翻译版)](https://zhuanlan.zhihu.com/p/703584080) |
|  2  | Gemma-2-2B | Google DeepMind | NA | NA | [aistudio.google](https://aistudio.google.com/app/prompts/new_chat?model=gemma-2-2b-it), [HuggingFace](https://huggingface.co/collections/google/gemma-2-2b-release-66a20f3796a2ff2a7c76f98f), [HuggingFace/mlc-ai](https://huggingface.co/spaces/mlc-ai/webllm-simple-chat), [mlc-ai/deploy/android](https://llm.mlc.ai/docs/deploy/android.html)
|  3  | Phi-2 | NA | NA | NA |
|  4  | MiNiCPM-2B | NA | NA | NA |
|  5  | Qwen2-1.5B | NA | NA | NA |



## 2.2 稠密模型与稀疏模型
-------


### 2.2.1 MoE
--------


MoE(Mixed Expert Models), 即混合专家模型, 首次在 1991 年的论文 [Adaptive Mixture of Local Experts](https://www.cs.toronto.edu/~hinton/absps/jjnh91.pdf) 提出 MoE.

[群魔乱舞：MoE大模型详解](https://www.zhihu.com/tardis/bd/art/677638939)

[【论文阅读】MOE，《OUTRAGEOUSLY LARGE NEURAL NETWORKS: THE SPARSELY-GATED MIXTURE-OF-EXPERTS LAYER》](https://blog.csdn.net/bylander/article/details/138139345)

[【论文速读】MOD，《Mixture-of-Depths: Dynamically allocating compute in transformer-based language models》](https://blog.csdn.net/bylander/article/details/139536003)

[Mixture of Depths论文解读](https://zhuanlan.zhihu.com/p/691324301)

[OLMoE](https://github.com/allenai/OLMoE)

### 2.2.2 稀疏化训练
-------

[【ICDE 2022】阿里发布稀疏模型训练框架HybridBackend，单位成本下训练吞吐提升至5倍](https://blog.csdn.net/weixin_48534929/article/details/124661176)


## 2.3 模型压缩和量化
-------


[一文弄懂模型知识蒸馏、剪枝、压缩、量化](https://blog.csdn.net/RuanJian_GC/article/details/128841124)

| 技术 | 描述 |
|:---:|:---:|
| 量化 | 类似"量子级别的减肥", 神经网络模型的参数一般都用 float32 的数据表示, 但如果我们将 float32 的数据计算精度变成 int8 的计算精度. 则可以牺牲一点模型精度来换取更快的计算速度. |
| 蒸馏 | 类似"老师教学生", 使用一个效果好的大模型指导一个小模型训练, 因为大模型可以提供更多的软分类信息量, 所以会训练出一个效果接近大模型的小模型. [详解4种模型压缩技术、模型蒸馏算法](https://zhuanlan.zhihu.com/p/638092734) |
| 剪裁 | 类似"化学结构式的减肥", 将模型结构中对预测结果不重要的网络结构剪裁掉, 使网络结构变得更加"瘦身". 比如, 在每层网络, 有些神经元节点的权重非常小, 对模型加载信息的影响微乎其微. 如果将这些权重较小的神经元删除, 则既能保证模型精度不受大影响, 又能减小模型大小. 参见 [论文总结-模型剪枝](https://xmfbit.github.io/2018/10/03/paper-summary-model-pruning/) |
| 神经网络架构搜索(NAS) | 类似"化学结构式的重构", 以模型大小和推理速度为约束进行模型结构搜索, 从而获得更高效的网络结构. 参见 [一文详解神经网络结构搜索(NAS)](https://zhuanlan.zhihu.com/p/73785074). |



## 2.4 模型结构优化
-------



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
| 13 | [XiaoMi/Mace](https://github.com/XiaoMi/mace) | 小米 | MACE (Mobile AI Compute Engine) 是一个针对移动异构计算平台优化的深度学习推理框架. 它专注于以下目标: 性能、功耗、响应性、内存使用和库体积、模型保护以及平台覆盖. MACE 支持 TensorFlow、Caffe和ONNX等多种模型格式, 并提供了丰富的示例和文档. |
| 14 | [Google-AI-Edge/litert](https://github.com/google-ai-edge/LiteRT) | Google Ai Edge | LiteRT(原名 TensorFlow-Lite) 是 Google 开源的高性能端侧 AI 运行时 |
| 15 | [AliBaBa/MNN](https://github.com/alibaba/MNN) | AliBaBa | MNN 是一个高效轻量级的深度学习框架, 在阿里巴巴的关键业务场景中得到广泛应用. 它支持深度学习模型的推理和训练, 在设备上具有业界领先的性能. MNN 还提供了一系列工具, 包括模型转换、压缩、表达式计算等功能. |
| 16 | [Tencent/TNN](https://github.com/Tencent/TNN) | Tencent | TNN 是由腾讯优图实验室和广影实验室开发的一个跨平台、高性能的深度学习推理框架. 它具有跨平台能力、高性能、模型压缩和代码裁剪等多项优秀特性. TNN 在原有的 ncnn 和 Rapidnet 框架基础上, 进一步加强了对移动设备的支持和性能优化, 同时也借鉴了业界主流开源框架的高性能和良好扩展性特点, 扩展了对X86和NV GPU的支持. TNN已经被应用于腾讯移动QQ、微视、Pitu等多个应用中. |
| 17 | [Paddle-Lite](https://github.com/PaddlePaddle/Paddle-Lite) | [PaddlePaddle](https://www.paddlepaddle.org.cn/lite) | Paddle Lite 面向端侧场景的轻量化推理引擎 Paddle Lite, 可以实现飞桨模型在 x86/ARM 平台下多种 OS 内的高效部署, 同时支持在 10 种以上的 GPU/NPU 异构后端上进行推理加速和混合调度. 是一个高性能、轻量级、灵活性强且易于扩展的深度学习推理框架, 定位于支持包括移动端、嵌入式以及边缘端在内的多种硬件平台. 它提供了简单易用的部署流程,支持多种硬件平台和多种编程语言,并且具有优秀的加速、优化策略及实现. |
| 18 | [uTensor]() | NA | NA |
| 19 | Core ML | Apple | NA |
| 20 | MediaPipe | Google |

### 3.1.2 推理加速库
-------


| 编号 | 加速框架 | 团队 | 介绍 |
|:---:|:-------:|:---:|:---:|
| 1 | [ARM-software/CMSIS-NN](https://github.com/ARM-software/CMSIS-NN) | ARM | CMSIS-NN 是一个高性能的神经网络内核软件库, 旨在最大化 Arm Cortex-M 处理器上神经网络的性能并最小化内存占用. 它遵循 TensorFlow Lite for Microcontrollers 的 INT8 和 INT16 量化规范, 与 TensorFlow Lite 参考内核完全一致. 该库提供了针对不同 Arm 处理器架构的优化实现, 包括纯 C、 DSP 扩展和 MVE 扩展等. |
| 2 | [SNPE](https://www.qualcomm.com/developer?redirect=qdn) | Qualcomm Snapdragon | SNPE 是 Qualcomm Snapdragon Neural Processing Engine 的简称. SNPE 是神经网络在骁龙平台上推理的开发套件, 方便开发者在使用高通芯片的设备上加速AI应用. 支持的模型框架: TensorFlow, CAFFE, ONNX, TensorFlowLite. 比如 [SNPE_Tutorial](https://github.com/gesanqiu/SNPE_Tutorial) |
| 3 | [PX4/eigen](https://github.com/PX4/eigen) | PX4 | Eigen 是一个 C++ 模板库, 用于线性代数: 矩阵、向量、数值求解器和相关算法. 它提供了一个高效、灵活和易于使用的接口,适用于各种应用程序.|
| 4 | [Google/XNNPACK](https://github.com/google/XNNPACK) | Google | XNNPACK 是一个针对 ARM、x86、WebAssembly 和 RISC-V 平台的高度优化的神经网络推理解决方案. 它不是直接面向深度学习从业者和研究人员使用的, 而是为诸如 TensorFlow Lite、TensorFlow.js、PyTorch、ONNX Runtime 和 MediaPipe 等高级机器学习框架提供低级性能原语,以加速它们的推理性能. |
| 5 | [OpenBLAS](https://github.com/OpenMathLib/OpenBLAS) | OpenBLAS | 开源的 CPU 线性代数库，支持多线程和 SIMD 加速, 广泛应用于科学计算和深度学习框架(如 PyTorch). |
| 6 | [Intel MKL(Math Kernel Library)]() | NA | 针对 Intel CPU 优化的数学计算库, 支持矩阵运算、FFT 等. 在 Intel 平台上性能优于 Eigen. |
| 7 | [Arm Compute Library](https://github.com/ARM-software/ComputeLibrary) | ARM | Arm CPU/GPU 的加速库, 支持图像处理和机器学习算子. 针对 Cortex-A/Cortex-M 优化, 兼容 CMSIS-NN46. |
| 8 | [CuPy](https://github.com/cupy/cupy) | NA | 基于 NVIDIA GPU 的数值计算库，语法兼容 NumPy. 替代部分 Eigen 功能, 适合 GPU 加速场景. |
| 9 | [neon](https://github.com/NervanaSystems/neon) | Intel | neon 是英特尔公司开源的深度学习框架, 致力于在各种硬件上提供最佳性能. 它设计简单易用, 并且具有可扩展性. |
| 10 | [lapack](https://github.com/Reference-LAPACK/lapack) | NA | LAPACK 是一个用于解决常见数值线性代数问题的 Fortran 子程序库. 它是一个免费提供的软件包, 可以包含在商业软件包中. LAPACK 包含了 Fortran 源代码、测试程序、基本线性代数子程序(BLAS)的 Fortran 参考实现, 以及 CBLAS 和 LAPACKE 的 C 接口. |
| 11 | [Tencent/ncnn](https://github.com/Tencent/ncnn) | Tencent | ncnn 是一个为手机端极致优化的高性能神经网络前向计算框架. 它从设计之初就深入考虑了手机端的部署和使用. ncnn 无第三方依赖、跨平台, 在手机端 CPU 上的速度快于目前所有已知的开源框架. 开发者可以轻松将深度学习算法移植到手机端高效执行, 开发出人工智能 APP, 将 AI 带到用户的指尖. ncnn 目前已在腾讯多款应用中使用, 如 QQ、Qzone、微信、天天 P 图等. |

## 3.2 推理加速
-------


[知乎--LLM推理加速技术简介](https://zhuanlan.zhihu.com/p/691360124)

[bilibili--如何将大模型与小模型结合？这8种常用策略必看！附17篇案例论文和代码](https://www.bilibili.com/opus/887920175625535524)

[知乎--刀刀宁聊大模型推理--笔记：学习推理加速半年之总结与迷思](https://zhuanlan.zhihu.com/p/704938096)

[知乎-锦年-全面解析 LLM 推理优化：技术、应用与挑战](https://zhuanlan.zhihu.com/p/18736565021)

### 3.2.1 KV Cache 压缩
-------

[SnapKV: LLM在生成内容之前就知道您在寻找什么](https://blog.csdn.net/qq_36931982/article/details/139118015)

[MiniCache 和 PyramidInfer 等 6 种优化 LLM KV Cache 的最新工作](https://www.51cto.com/aigc/913.html)

[PyramidKV学习资料汇总 - 动态KV缓存压缩技术](https://blog.csdn.net/m0_56734068/article/details/142382328)

[大模型推理加速：KV Cache Sparsity(稀疏化)方法](https://zhuanlan.zhihu.com/p/701580870)

[聊聊大模型推理中的 KVCache 之异构缓存](https://zhuanlan.zhihu.com/p/714288577)

[聊聊大模型推理中的 KVCache 压缩](https://zhuanlan.zhihu.com/p/708946312)


### 3.2.2 稀疏感知推理加速
-------

[论文笔记：DejaVu、LLM in Flash、PowerInfer](https://zhuanlan.zhihu.com/p/675585887)

[苹果极致LLM端侧方案：LLM in a flash](https://zhuanlan.zhihu.com/p/673775476)

### 3.2.3 首 Token 时延优化
-------

[[Prefill优化][万字]🔥原理&图解vLLM Automatic Prefix Cache(RadixAttention): 首Token时延优化](https://zhuanlan.zhihu.com/p/693556044)

### 3.2.4 投机执行
-------

[论文导读 | 投机解码加速模型推理](https://zhuanlan.zhihu.com/p/698333087)

[大模型推理妙招—投机采样(Speculative Decoding)](https://zhuanlan.zhihu.com/p/651359908)

[最全LLM自投机算法汇总](https://zhuanlan.zhihu.com/p/706111755)

[LLM推理提速2.8倍，CMU清华姚班校友提出「投机式推理」引擎SpecInfer，小模型撬动大模型高效推理](https://www.jiqizhixin.com/articles/2023-05-30-3)

[知乎-LLM推理加速新范式！推测解码（Speculative Decoding）最新综述](https://zhuanlan.zhihu.com/p/678404136)

[知乎-投机采样（Speculative Decoding），另一个提高LLM推理速度的神器（三）](https://zhuanlan.zhihu.com/p/681401656)

[知乎-刀刀宁-聊聊大模型推理服务之投机推理](https://zhuanlan.zhihu.com/p/699166575)

[知乎-hemingkx-推测解码（Speculative Decoding）哪家强？-- 最新评测基准Spec-Bench分享](https://zhuanlan.zhihu.com/p/683995502)

| 编号 | 算法 | 描述 |
|:---:|:---:|:----:|
| NA | NA | NA |


### 3.2.5 分布式推理
-------




## 3.3 算子库
-------


PaddlePaddle/Anakin

Tencent/ncnn

Tencent/FeatherCNN

dmlc/tvm

ARM-software/ComputeLibrary

# 4 分析工具
-------

| 编号 | 内容 | 详情 |
|:---:|:----:|:---:|
|  1  | [Interactive Tools for machine learning, deep learning, and math](https://github.com/Machine-Learning-Tokyo/Interactive_Tools) | 用于机器学习、深度学习和数学运算的交互式工具. |
|  2  | [Visual Guides to understand the basics of Large Language Models](https://towardsdatascience.com/visual-guides-to-understand-the-basics-of-large-language-models-0715701bdd20) | 一系列工具与文章的汇编, 直观易懂地解读复杂的 AI 概念. 译文 [深入浅出：大语言模型的视觉解析 [译]](https://baoyu.io/translations/llm/visual-guides-to-understand-the-basics-of-large-language-models). |

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
[Tokenizer的系统梳理，并手推每个方法的具体实现](https://cloud.tencent.com/developer/article/2327739)
[各种中文分词工具的使用方法](https://blog.csdn.net/PolarisRisingWar/article/details/125388765)
[五款中文分词工具在线PK: Jieba, SnowNLP, PkuSeg,THULAC, HanLP](https://zhuanlan.zhihu.com/p/64409753)

| 编号 | 分词器 | 团队 | 详情 |
|:---:|:-----:|:----:|:---:|
|  1  | [sentencepiece](https://github.com/google/sentencepiece) | Google | Unsupervised text tokenizer for Neural Network-based text generation.<br>SentencePiece 是谷歌开源的针对 NLP 场景提取词汇表 tokenizer 的开源项目, 它是谷歌推出的子词开源工具包, 其中集成了 BPE、ULM 子词算法. 除此之外, SentencePiece还能支持字符和词级别的分词. 更进一步, 为了能够处理多语言问题, sentencePiece 将句子视为 Unicode 编码序列, 从而子词算法不用依赖于语言的表示.<br>SentencePiece 提出的目的是在给定词汇表大小的前提下, 最大化词表信息编码 (词频 + 多样性)subword 编码. 比如英语中的 simple 和 simplify 这两个词意思是一样的, 是为了适应语法需求而有的变化, 所以使用独立的 token 对这两个单词编码是有冗余的, 另外一种场景是, 词频不一样, 有常用汉字一说, 也有常用英语单词一说. 出现较少的词使用独立的 token 在训练的时候相比其他高频词由于出现的太少而造成深度学习(信息压缩)这一过程容易丢失该信息. |
|  2  | [huggingface/tokenizers](https://github.com/huggingface/tokenizers) | Hugging Face | NA |
|  3  | [openai/tiktoken](https://github.com/openai/tiktoken) | OpenAI | [OpenAI开源GPT-2的子词标记化神器——tiktoken，一个超级快的（Byte Pair Encoder，BPE）字节对编码Python库](https://zhuanlan.zhihu.com/p/592399697), [OpenAI 大模型高效Tokenizer: tiktoken](https://zhuanlan.zhihu.com/p/631840697) |
|  4  | [mlc-ai/tokenizers-cpp](https://github.com/mlc-ai/tokenizers-cpp) | 一个跨平移台的 C++ 分词器, 包装并 bind 了 HuggingFace 以及 sentencepiece, 并提供了最小的通用 C++ API. |
|  5  | [rust-tokenizers](https://github.com/guillaume-be/rust-tokenizers) | | Rust-tokenizers 是一个高性能的 tokenizer 库, 支持多种现代语言模型, 包括 WordPiece、Byte-Pair Encoding (BPE) 和 Unigram (SentencePiece) 模型. 这些 tokenizer 广泛应用于自然语言处理领域, 特别是在 transformer 架构中. |
|  6  | [OpenNMT/Tokenizer](https://github.com/OpenNMT/Tokenizer) | 一个快速, 通用, 可定制的文本分词器, 支持 C++/Python, 依赖最小. 提供了多种功能, 包括可逆分词, 子词分词, 高级文本分段, 大小写管理以及保护序列等. |


## 4.2 Transformer
-------

| 编号 | 工具 | 团队 | 详情 |
|:---:|:----:|:---:|:---:|
|  1  | [Gemma Scope](https://www.163.com/dy/article/J8GVD23005566WT8.html) | Google DeepMind | [Google DeepMind 发布大模型可视化工具 Gemma Scope](https://www.163.com/dy/article/J8GVD23005566WT8.html) |
|  2  | [Inspectus](https://github.com/labmlai/inspectus) | labmlai | [探索语言模型的奥秘 - 体验 Inspectus 的强大视觉洞察力](https://github.com/labmlai/inspectus) |
|  3  | [Transformer Explainer](https://github.com/poloclub/transformer-explainer) | poloclub |  通过互动可视化的方式了解生成式 AI 中 Transformer 的工作原理. 他在浏览器中实时运行一个 GPT-2 模型, 允许用户使用自己的文本进行试验, 并实时观察 Transformer 的内部组件和操作如何协同工作来预测下一个 Token, 在线体验地址 [transformer-explainer/](https://poloclub.github.io/transformer-explainer) |
|  4  | [BertViz](https://github.com/jessevig/bertviz) | Jessevig | 使用交互式的方式, 可视化 Transformer 语言模型 (如 BERT, GPT2) 的注意力机制, 可在 Jupyter 或 Colab 中运行, 通过简单的 Python API 支持大多数 Hugging Face 预训练模型 (如 BERT, GPT2, T5 等), BertViz 扩展了 Llion Jones 的 Tensor2Tensor 可视化工具, 提供了头视图, 模型视图, 神经元视图等多个不同的可视化方式, 每个视图都提供了一个独特的视角来了解注意力机制. 参见 [2019/04/11, Human-Computer Interaction (cs.HC), Visualizing Attention in Transformer-Based Language Representation Models](https://arxiv.org/abs/1904.02679). |
|  5  | [LLM Visualization](https://github.com/bbycroft/llm-viz | bbycroft | 将 Transformer 原理的详细细节通过交互可视化的方式一步步显示出来, 详细的展示了每一步的数学原理, 模型的网格结构, 参数构造的运行过程, 可以精确到每一步观察大模型运行的运算以及数据的变化. 作者的 [仓库 bbycroft/llm-viz](https://github.com/bbycroft/llm-viz) 以及 [在线演示地址 bbycroft.net/llm](https://bbycroft.net/llm), [@HansChanX](https://x.com/HansChanX) LLM 可视化演的中文翻译版本: [仓库 czhixin/llm-viz-cn](https://github.com/czhixin/llm-viz-cn) 以及 [示](https://llm-viz-cn.iiiai.com/llm). 其他 [Vaaaas/llm-viz-CN](https://github.com/Vaaaas/llm-viz-CN), 知乎报道 [矩阵模拟！Transformer 大模型 3D 可视化，GPT-3、Nano-GPT 每一层清晰可见](https://zhuanlan.zhihu.com/p/670287271) |
|  6  | [Machine-Learning-Tokyo/Interactive_Tools](https://github.com/Machine-Learning-Tokyo/Interactive_Tools) | 这个项目收集了各种用于机器学习、深度学习和数学的交互式工具. |
|  7  | [hahnyuan/LLM-Viewer](https://github.com/hahnyuan/LLM-Viewer) | 一个可视化语言与学习模型 LLMs 并分析在不同硬件平台上性能的工具. 可以进行网络级分析, 考虑峰值内存消耗和总推理时间成本等因素. 使用 LLM-Viewer, 可以获取 LLM 推理和性能优化的宝贵见解. 可以在 Web 浏览器或者命令行(CLI) 工具中使用. 在线体验地址 [LLM-Viewer Web](http://llm-viewer.com). 参见论文 [LLM Inference Unveiled: Survey and Roofline Model Insights](https://arxiv.org/abs/2402.16363). |


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
| [Lightning-AI/litgpt](https://github.com/Lightning-AI/litgpt) | LitGPT 是一个强大的工具, 使开发人员能够利用LLM的最新进展. 其全面的功能、用户友好性以及不断发展的社区使其成为构建和部署LLM应用程序的理想选择. |
| [aishwaryanr/awesome-generative-ai-guide](https://github.com/aishwaryanr/awesome-generative-ai-guide) | 关于人工智能的开源综合站点, 汇总了相关的论文以及课程, 以及业界前沿资讯. |
| [LuckyyySTA/Awesome-LLM-hallucination](https://github.com/LuckyyySTA/Awesome-LLM-hallucination) | 这项研究调查了与大型语言模型幻觉相关的论文. 包括相关的调查或分析论文、幻觉原因、幻觉检测和基准、幻觉缓解, 以及该领域的挑战和开放性问题. |
| [Datawhale](https://github.com/datawhalechina) | Datawhale 是一个专注于数据科学与 AI 领域的开源组织, 汇集了众多领域院校和知名企业的优秀学习者, 聚合了一群有开源精神和探索精神的团队成员. Datawhale 以 "for the learner, 和学习者一起成长"为愿景, 鼓励真实地展现自我、开放包容、互信互助、敢于试错和勇于担当. 同时 Datawhale 用开源的理念去探索开源内容、开源学习和开源方案, 赋能人才培养, 助力人才成长, 建立 起人与人, 人与知识, 人与企业和人与未来的联结. |
| [zjunlp/LLMAgentPapers](https://github.com/zjunlp/LLMAgentPapers) | 关于大型语言模型代理的必读论文. |
| [mli/paper-reading](https://github.com/mli/paper-reading) | 深度学习论文精读. |
| [mlabonne/llm-course](https://github.com/mlabonne/llm-course) | 大型语言模型课程. |
| [AmadeusChan/Awesome-LLM-System-Papers](https://github.com/AmadeusChan/Awesome-LLM-System-Papers) | 收集了大语言模型系统论文, 涵盖了算法-系统协同设计, 推理系统相关的内容. |
| [tensorchord/Awesome-LLMOps](https://github.com/tensorchord/Awesome-LLMOps) | 精选了 LLMOps 工具, 收集和整理了大量优秀的 LLMOps 工具, 涵盖了模型, 服务, 安全, 可观测性等多个领域. |
| [HqWu-HITCS/Awesome-Chinese-LLM](https://github.com/HqWu-HITCS/Awesome-Chinese-LLM) | 整理开源的中文大语言模型, 主要关注规模较小, 可私有化部署, 训练成本较低的模型. 包括底座模型, 垂直领域微调以及应用, 数据集合教程等内容 |
| [km1994/nlp_paper_study](https://github.com/km1994/nlp_paper_study) | 该仓库主要记录 NLP 算法工程师相关的顶会论文研读笔记. |
| [NexaAI/Awesome-LLMs-on-device](https://github.com/NexaAI/Awesome-LLMs-on-device) | 汇总了端侧 AI 的相关架构和优化技术, 包括前言的论文研究. |
| [wdndev/llm_interview_note](https://github.com/wdndev/llm_interview_note) | 主要记录大语言大模型(LLMs) 算法(应用)工程师相关的知识及面试题. |
| [冬于的博客-Transformer/BERT/实战](https://ifwind.github.io/2021/08/31/Transformer-BERT-实战) | 通过大量图讲述 Transformer 架构 |
| [浅显易懂地介绍 llm.c [译]](https://baoyu.io/translations/llm/explaining-llm-c-in-layman-terms) | [Explainable Language Models: Existing and Novel Approaches](https://twitter.com/karpathy/status/1778153659106533806) 的译文, 参见 [karpathy/llm.c](https://github.com/karpathy/llm.c). |
| [DefTruth/Awesome-LLM-Inference](https://github.com/DefTruth/Awesome-LLM-Inference) | 收集了大量 LLM 推理相关的论文和仓库, 涵盖了并行计算, 量化压缩, 注意力机制优化, 上下文管理等. |
| [SylphAI-Inc/llm-engineer-handbook](https://github.com/SylphAI-Inc/llm-engineer-handbook) | NA |


## 5.2 Survey
-------


[AIWIKI.AI, the AI and ML Wiki](https://aiwiki.ai/wiki/AI_ML_Wiki)


| 时间 | 分类 | Survey | 作者 | GitHub | 描述 |
|:---:|:----:|:------:|:---:|:------:|:----:|
| 2024/03/01 | 综述 | [NiuTrans/ABigSurveyOfLLMs](https://github.com/NiuTrans/ABigSurveyOfLLMs) | [NiuTrans](https://github.com/NiuTrans/ABigSurveyOfLLMs) | [NiuTrans](https://github.com/NiuTrans/ABigSurveyOfLLMs) | 一个关于大语言模型的综合性调研集合, 包含 150 多篇关于 LLM 的调研论文. 这些调研涵盖了 LLM 的各个方面, 包含通用调研, Transformer, 对齐, 提示学习, 上下文学习, 推理链, 提示工程, 数据, 评估, 社会问题, 安全性, 幻觉, 属性, 高效 LLM, 学习方法, 多模态 LLM, 基于知识的 LLM, 检索增强型 LLM, 知识编辑, LLM 扩展, LLM 与工具, LLM 与交互, 长序列 LLM, 以及 LLM 在教育, 法律, 医疗, 游戏, NLP 任务, 软件工程, 推荐系统, 图谱等领域的应用. |
| 2024/01/16 | 多模态 | [A Survey of Resource-efficient LLM and Multimodal Foundation Models](https://arxiv.org/abs/2401.08092) | Mengwei Xu | [UbiquitousLearning](https://github.com/UbiquitousLearning/Efficient_Foundation_Model_Survey) | 一篇关于资源高效的大模型和多模态基础模型的综述论文. 论文涵盖了算法和系统两个方面的创新, 包括了高校的模型架构, 训练算法, 推理算法和模型压缩等内容. |
| 2024/04/18 | 效率提升 | [The Efficiency Spectrum of Large Language Models: An Algorithmic Survey](https://arxiv.org/abs/2312.00678) | Tianyu Ding | [tding1](https://github.com/tding1/Efficient-LLM-Survey) | 一篇关于提供大语言模型效率的综合性调查论文, 全面回顾了旨在提高 LLM 效率的算法, 涵盖了扩展定律, 数据利用, 架构创新, 训练和调优策略以及推理计划等. |
| 2024/05/23 | LLMs | [Efficient Large Language Models: A Survey](https://arxiv.org/abs/2312.03863) | Zhongwei Wan | [AIoT-MLSys-Lab](https://github.com/AIoT-MLSys-Lab/Efficient-LLMs-Survey) | 本文对高效 LLMs 研究的发展进行了系统而全面的回顾, 并将文献整理成由三个主要类别组成的分类法, 从模型中心、数据中心和框架中心的角度涵盖了不同但相互关联的高效 LLMs 主题, 并且从以模型为中心和以数据为中心的角度, 回顾了 LLMs 的算法层面和系统层面的高效技术. 详细介绍了每个分类下的具体技术, 如: 量化, 剪枝, 知识蒸馏, 数据选择, 提示工程等<br>1. [知乎--黄浴--高效大语言模型：综述](https://zhuanlan.zhihu.com/p/671710012)<br>2. [知乎--磐石--大模型高效推理 I 推理技术框架总结](https://zhuanlan.zhihu.com/p/696850285)<br>3. [知乎--享享学AI--大模型LLM微调技术方法汇总！](https://zhuanlan.zhihu.com/p/673675939) |
| 2024/04/22 | 综述 | [A Survey on Efficient Inference for Large Language Models](https://arxiv.org/abs/2404.14294) | Zixuan Zhou | 1. [如何加速大模型推理？万字综述全面解析大语言模型高效推理技术 ](https://www.sohu.com/a/790365299_121119001)<br>2. [知乎--罗清雨--大语言模型高效推理综述](https://zhuanlan.zhihu.com/p/707685591) |
| 2023/06/23 | 多模态 | [A Survey on Multimodal Large Language Models](https://arxiv.org/abs/2306.13549) | Shukang Yin | [BradyFU](https://github.com/BradyFU/Awesome-Multimodal-Large-Language-Models) | 本综述中主要介绍了多模态幻觉、多模态上下文学习(Multimodal InContext Learning，M-ICL)、多模态思维链(Multimodal Chain of Thought，M-CoT)和 LLM 辅助的视觉推理(LLM-Aided Visual Reasoning，LAVR)等. |
| 2024/07/26 | 模型压缩 | [Comprehensive Study on Performance Evaluation and Optimization of Model Compression: Bridging Traditional Deep Learning and Large Language Models](https://arxiv.org/abs/2407.15904) | Aayush Saxena | 近年来, 深度学习模型在大多数行业都取得了巨大成功. 这些模型的发展还导致模型大小和能源需求增加, 使其难以在低计算设备上的生产环境中进行部署. 全球互联设备数量的增加保证了压缩模型可以轻松部署在本地设备上, 但计算容量和电源可访问性较低. 不同的研究人员提出了广泛的解决方案来减小此类模型的大小和复杂性, 其中突出的是权重量化、参数修剪、网络修剪、低秩表示、权重共享、神经架构搜索、知识蒸馏等. 在这项研究工作中, 我们调查了使用量化和修剪技术进行压缩的各种训练有素的深度学习模型的性能影响. 我们在图像分类、对象检测、语言模型和基于生成模型的问题陈述中使用的常用深度学习模型上实施了量化和剪枝压缩技术. 我们还探讨了各种大型语言模型在量化和低秩适应后的性能. 我们对所有相关问题陈述使用了标准评估指标(模型的大小、准确性和推理时间), 并通过讨论挑战和未来的工作来总结本文. |
| 2024/06/04 | 投机 | [Unlocking Efficiency in Large Language Model Inference:A Comprehensive Survey of Speculative Decoding](https://arxiv.org/abs/2401.07851) | Heming Xia | [hemingkx/SpeculativeDecodingPapers](https://github.com/hemingkx/SpeculativeDecodingPapers) | [COLING 2025 Tutorial:Speculative Decoding for Efficient LLM Inference](https://speculative-decoding.github.io), [知乎-LLM推理加速新范式！推测解码（Speculative Decoding）最新综述](https://zhuanlan.zhihu.com/p/678404136) |


[Mobile Edge Intelligence for Large Language Models: A Contemporary Survey](https://arxiv.org/abs/2407.18921)
[Edge Intelligence: Architectures, Challenges, and Applications](https://arxiv.org/abs/2003.12172)
[A Survey on Model Compression for Large Language Models](https://arxiv.org/abs/2308.07633)
| NA | NA | [Towards Efficient Generative Large Language Model Serving: ASurvey from Algorithms to Systems](https://arxiv.org/abs/2312.15234) | NA | NA | [知乎--路漫漫独求索--LLM推理加速技术简介](https://zhuanlan.zhihu.com/p/691360124)
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
| 2024/11/25 | 日常论文速递 | NA | NA | [叶子的技术碎碎念-每周AI论文速递](http://leafw.cn) | NA |











<br>

*   本作品 / 博文 ([AderStep - 紫夜阑珊 - 青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚 (gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用 <a rel="license"href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt=" 知识共享许可协议 "style="border-width:0"src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png"/></a><a rel="license"href="http://creativecommons.org/licenses/by-nc-sa/4.0/"> 知识共享署名 - 非商业性使用 - 相同方式共享 4.0 国际许可协议 </a> 进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名 [成坚 gatieme](http://blog.csdn.net/gatieme) (包含链接: http://blog.csdn.net/gatieme), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   ** 转载请务必注明出处, 谢谢, 不胜感激 **
<br>
