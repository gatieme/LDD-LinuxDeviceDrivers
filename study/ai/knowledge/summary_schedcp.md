# SchedCP: Towards Agentic OS - LLM Agent Framework for Linux Schedulers

## Paper Overview

**Title**: Towards Agentic OS: An LLM Agent Framework for Linux Schedulers
**arXiv ID**: 2509.01245
**GitHub**: https://github.com/eunomia-bpf/schedcp

## Problem Statement

Operating system schedulers suffer from a fundamental **semantic gap**, where kernel policies fail to understand application-specific needs, leading to suboptimal performance. Traditional schedulers use generic policies that cannot adapt to the nuanced requirements of diverse workloads.

## Solution: SchedCP Framework

SchedCP is the first framework that enables fully autonomous LLM agents to safely and efficiently optimize Linux schedulers without human involvement.

### Core Innovation

The key insight is that the challenge is not merely to apply a better LLM, but to **architect a decoupled control plane** that separates:

- **AI's role**: Semantic reasoning ("what to optimize")
- **System's role**: Execution ("how to observe and act")

This separation divides the optimization problem into two stages:
1. **Goal-inference**: Understanding what needs optimization
2. **Policy-synthesis**: Creating the actual scheduling policies

### Architecture Components

Implemented as a Model Context Protocol (MCP) server, SchedCP provides a stable interface with three key services:

1. **Workload Analysis Engine** - Analyzes runtime workload characteristics
2. **Scheduler Policy Repository** - Maintains an evolving collection of scheduling policies
3. **Execution Verifier** - Validates all AI-generated code and configurations before deployment using:
   - Static analysis
   - Dynamic analysis

## Demonstration: sched-agent

The framework is demonstrated with **sched-agent**, a multi-agent system that:

1. Autonomously analyzes workloads
2. Synthesizes custom eBPF scheduling policies
3. Deploys them via Linux's `sched_ext` infrastructure

## Performance Results

- **1.79x** performance improvement
- **13x** cost reduction compared to naive ag
  gentic approaches
- Maintains high success rate

## Significance

By bridging the semantic gap, SchedCP:
- Democratizes expert-level system optimization
- Represents a step toward truly self-optimizing, application-aware operating systems
- Enables autonomous OS-level optimization without human intervention

## Technical Stack

- **LLM Agents**: Multi-agent system for reasoning
- **eBPF**: Extended Berkeley Packet Filter for kernel-level policies
- **sched_ext**: Linux scheduler extension infrastructure
- **MCP**: Model Context Protocol for agent integration
- **Static/Dynamic Analysis**: Safety verification

## Potential Applications for agent-toolkit

This paper's architecture has several relevant insights for the agent-toolkit project:

1. **Decoupled Control Plane**: Separating semantic reasoning from execution is a powerful pattern for agent systems
2. **Verification Layer**: The two-stage verification (static + dynamic) before deployment ensures safety
3. **MCP Integration**: Using Model Context Protocol as the interface layer
4. **Multi-agent Collaboration**: The sched-agent demonstrates effective multi-agent coordination
5. **Repository Pattern**: Maintaining an evolving policy repository enables learning and adaptation

## Key Takeaways

1. **Separation of Concerns**: The "what to optimize" vs. "how to execute" separation is fundamental
2. **Safety First**: All AI-generated code must be verified before deployment
3. **Learning System**: Policies should evolve and be reusable across sessions
4. **Self-Optimizing Systems**: The goal is creating truly autonomous, self-improving systems
