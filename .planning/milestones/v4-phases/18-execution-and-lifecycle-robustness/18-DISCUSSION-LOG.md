# Phase 18: Execution and lifecycle robustness - Discussion Log

> **Audit trail only.** Do not use as input to planning, research or execution.
> Decisions are captured in 18-CONTEXT.md; this log preserves alternatives.

**Date:** 2026-10-24
**Phase:** 18-execution-and-lifecycle-robustness
**Areas discussed:** Accepted execution reach; lifecycle continuation and
isolation; guest loops/JAM; replay and finite campaigns; findings and scope.

## Delegation and process

The user requested fan-out across relevant roles, primary-source research,
pros/cons and adversarial synthesis, then automatic adoption of coherent
recommendations. They reiterated the preference for small, flat dependency
trees and owned code over unnecessary dependencies. This superseded the
default question-by-question selection flow for this invocation. No question
was silently defaulted; the agent selected all areas under that delegation.
No auto-chain or global configuration setting was enabled.

Three typed advisor tasks reviewed execution reach, lifecycle/halt and
campaign evidence using the project-resolved model/effort. Execution and
campaign advisors then cross-reviewed the combined recommendations. The
orchestrator read project state, preparation references, current source and
the latest three prior phase contexts. No pending todo matched; no Phase 18
SPEC, prior context, plan or interrupted discussion existed.

## Accepted-program execution reach

| Option | Benefit and tradeoff | Selected |
|---|---|---|
| Whole-image mutation alone | End-to-end parser coverage; header/size rejection and early JAM can dominate | |
| Fully structured instruction grammar | Strong directed reach; generator restrictions can hide relevant bytes and add machinery | |
| Accepted-profile recipes, mutable payloads and qualified deep seeds | Six-family sustained execution with bounded generation; needs measurement against existing state fuzz and guards against generator bias | Yes |

**Selection:** Agent recommendation adopted under user delegation (D-01–04).
Existing loader fuzz never executes; state fuzz already executes two frames
after accepted state mutations. Core-only reach, completed operations and
matched measurement conditions determine added value. Each target uses its
own compatible corpus; identical input files are not assumed meaningful across
different formats. No arbitrary numeric budget or measured gain was claimed.

## Lifecycle continuation and independence

| Option | Benefit and tradeoff | Selected |
|---|---|---|
| Fresh/interleaved instances and metamorphic continuations | Tests public determinism and lifecycle contracts with owned code; cannot independently prove hardware correctness | Yes |
| External emulator as required oracle | Independent comparison possibilities; licensing, tool and disagreement scope without a demonstrated need | |
| Hash all buffers/structs after every call | Simple-looking oracle; includes padding, partial JAM output and stale data | |

**Selection:** D-05–07. Cross-review emphasized successful-output-only
comparison, state/latch comparison on JAM, reset retention after state has
actually changed, and the separate lifetime semantics of battery generation.
No new state-loader rejection sweep or public API was selected.

## Guest loops and JAM

| Option | Benefit and tradeoff | Selected |
|---|---|---|
| Return without video on JAM | Current adapter behavior; conflicts with documented once-per-run video cadence | |
| Cached complete XRGB frame or initialized black, input poll, no audio | Keeps native halt and avoids partial outputs; frozen display and possible host audio pacing concern | Yes |
| Schedule adapter silence | Could help audio pacing; introduces adapter timing/state-restore questions without measured need | |

**Selection:** D-08–10. Ordinary software loops remain normal frame execution.
The cache is invalidated on successful content/reset/unload/state transitions;
refused state load preserves it. Native framebuffers are not serialized, so a
restored JAM state uses black rather than unrelated pre-load imagery. Existing
no-stale-audio tests remain binding. A pinned-host pacing finding can justify
a tested silence-policy amendment; no such finding was measured here.

## Fixed replay and finite campaigns

| Option | Benefit and tradeoff | Selected |
|---|---|---|
| Six-platform fixed replay plus one Linux nightly mutator | Portable deterministic regression coverage with controlled mutation cost | Yes |
| Linux-only replay | Cheapest; loses direct corpus replay on other supported compilers/architectures | |
| Mutation in every platform lane | More discovery environments; substantial toolchain and cost expansion without evidence | |

**Selection:** D-11–14. Mutate a writable copy, retain original findings and
exact replay identities, then minimize without changing failure class or
losing intended accepted execution. Review distinguished target work bounds,
replay input bounds, engine iteration/time/timeout limits, sampled RSS and
outer workflow deadlines. Resource-failure input capture must be demonstrated
for the selected mode; missing bytes remain an explicit limitation. Outer
cancellation is incomplete evidence, never a clean campaign or guest timeout.

## Finding disposition and scope

| Option | Benefit and tradeoff | Selected |
|---|---|---|
| Minimized regression and correction, or explicit scope amendment | Honest bounded closure; serious discoveries may require a scope decision | Yes |
| Quiet waiver or weakening accepted-profile coverage | Keeps apparent progress; does not satisfy ROB acceptance | |
| Open-ended architecture/accuracy repair | May address more debt; exceeds approved scope and has no bounded completion | |

**Selection:** D-15–16. No promised bug count or universal safety claim. An
unproductive prototype cannot complete ROB-01. Preserve existing conformance,
state, parity, perf and release contracts. Planning, execution, merging and
milestone direction upkeep remain separate steps.

## Agent discretion

Target/module names, small recipe encoding, representative board sizes,
operation schedule, corpus membership and numeric resource/retention budgets
remain implementation choices selected from actual measurements. Three
initial reviews plus two cross-reviews found no remaining architectural
contradiction; the meaningful remaining questions require runtime evidence.
No runtime experiment or product test ran during this discussion.

## Deferred ideas

New hardware/regions, extra host platforms, a new dependency or fuzz engine,
new host/tick APIs, broad accuracy repairs, generic evidence infrastructure
and timed JAM silence without measured need remain outside this choice.
Nightly vectors-full debt is retained. Phase 17 PR delivery is still pending.

## Sources

The canonical local and external reference list is in 18-CONTEXT.md.
Primary online checks used [LLVM libFuzzer](https://llvm.org/docs/LibFuzzer.html),
[SanitizerCoverage](https://clang.llvm.org/docs/SanitizerCoverage.html) and
[libretro's core contract](https://docs.libretro.com/development/cores/developing-cores/#retro_run).
No reference-emulator implementation source was consulted.
