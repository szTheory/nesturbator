# Phase 21: Interrupt latency and hijack - Discussion Log

> **Audit trail only.** Research, planning and execution use `21-CONTEXT.md`,
> not this log.

**Date:** 2026-10-28
**Phase:** 21-interrupt-latency-and-hijack
**Areas discussed:** IRQ acceptance and branch polls; shared entry/vector
takeover; mid-entry state and host continuation; regression/delivery evidence.

## Delegation and process

The owner invoked `$gsd-discuss-phase 21`, then requested that every relevant
decision be examined across appropriate stakeholder/technical roles, with
primary-source research, tradeoffs, adversarial review and coherent
recommendations followed automatically. That explicitly delegates choices;
no questionnaire or additional routine approval was needed. It does not
authorize chaining into planning or execution. Existing one-step and clean-room
rules remain in force.

Three parallel specialist reviews inspected code, prior context and source
material. Polling covered hardware/C/test concerns, entry covered circuit and
bus/stack semantics, and state/acceptance covered API, serialization security,
host replay, CI and performance. The entry reviewer then challenged the
combined resumable-entry proposal. No implementation or runtime experiment
was performed. No speculative UI or unrelated ecosystem work was added.

The Phase 20 PR was initially described as open in local planning records.
GitHub's live PR metadata showed #59 merged at
`a64c23fd3ca7caed7e3ad102ff1a9beaa29fa562`. Fetching `origin/main` confirmed
the merged revision. Its tree matched the old branch head; Phase 21's branch
was created from merged main, retaining the owner's config modification.

## IRQ acceptance and branch polls

| Decision / option | Pros | Cons / footguns | Selected |
|---|---|---|---|
| Accept eligibility at the legal poll | Preserves I latency and earlier acceptance | Must distinguish sampled/history/live values | Yes |
| Test live I only at dispatch | Small current code | CLI/SEI/PLP timing is wrong | No |
| Store accepted result vs sampled I separately | Accepted result is minimal; separate I aids diagnostics | Redundant history can drift or enlarge state | Minimal representation left to planning |
| Instruction-specific branch history | Captures first and page-cross polls | Must retain earlier acceptance after acknowledgment | Yes |
| Use final bus sample for every branch | Uniform rule | Invents same-page poll; loses earlier result | No |
| Rewrite global capture/alignment | Could address a separately proven defect | Reopens six passing NMI rows and mapper timing | Only with a minimal reproducer |

**Agent-selected under owner delegation:** Source-derived poll acceptance,
RTI as its distinct case, and explicit branch history. Preserve Phase 20's
ordinary-loop capture model and one IRQ-line writer.

**Questions resolved:** Which I value decides acceptance? Which branch polls
exist? Does an earlier acceptance survive acknowledgment? Which existing
capture behavior should remain stable? Answers are D-01 through D-04.

**Adversarial findings:** The CPU-only vector bus does not perform production
sampling. An older branch-test helper conditions an expectation on observed
poll history, but a later helper already has independent fixed tables; reuse
the latter. AccuracyCoin latency tests include DMA/open-bus prerequisites,
so first divergence must establish the cause before expanding work.

## Shared entry and NMI vector takeover

| Decision / option | Pros | Cons / footguns | Selected |
|---|---|---|---|
| Small shared entry with separate origin/vector | One hardware sequence and state mechanism | Must preserve BRK-specific PC/B and count its opcode fetch | Yes |
| Three fixed-vector paths | Locally straightforward | Already lacks takeover; duplicated consumption/stack rules | No |
| Fix vector at entry | Simple | Misses eligible late NMI | No |
| Independently choose each vector byte | Direct reads | Permits NMI/IRQ mixed vectors excluded by source analysis | No |
| Phase-qualified recognition and consumption | Explains takeover, deferral and inhibited pulses | Requires source-to-bus-cycle crosswalk | Yes |
| Retain every raw NMI pulse during entry | Easy blanket invariant | Extrapolates ordinary-loop evidence beyond its domain | No |

**Agent-selected under owner delegation:** D-05 through D-08. Origin controls
stack semantics; source-qualified recognition controls a coherent vector.
Entry does not become an extra ordinary instruction poll.

**Questions resolved:** What is shared? When is a vector allowed to change?
How does hijacked BRK differ from hijacked IRQ? What becomes of late requests?
The exact source-derived cutoff is a research obligation, not a guessed
cycle number locked during discussion.

**Adversarial findings:** Preparation's cycle-4 wording and old research's
vector-low wording use different timing concepts. Primary circuit analysis
describes inhibition of a recognition stage, not blanket disabling of all
input history. A pulse entirely inside an inhibited window and an already
retained request are different cases. Phase 20's oracle already excludes
entry windows; its ordinary-loop results are not contradicted by that limit.

## Mid-entry state and host continuation

| Option | Pros | Cons / footguns | Selected |
|---|---|---|---|
| Narrow production entry stages; internal cutpoint execution; normal calls drain entry | Genuine saved-state continuation; preserves public frame/input behavior | New persistent history and format; distinguish constructed origin from executed cutpoint | Yes |
| Suspend entry at natural frame deadline | Ordinary public calls expose cutpoints | Changes documented instruction completion/input semantics; possible I/O dummy reads | No; reconsider only for a future public-cutpoint requirement |
| Save only before/after entry | Very small change | Cannot satisfy literal mid-entry replay criterion | No |
| Whole-CPU state machine | General resumability | Broad regression/performance cost without a demonstrated need | No |
| Save via callback inside current synchronous code | Superficially exact | C-stack continuation is missing; load repeats or skips work | No |

**Agent-selected under owner delegation:** D-14 through D-19, state format
2 to 3 and behavior revision 8 to 9, ABI unchanged. The roadmap asks for
mid-entry saved-state replay, not an ordinary public API that exposes exact
cycle stops. Internal fixtures execute the same production stages, native
save/load records their complete history, normal execution resumes, and
the actual adapter loads the compatible state for run-ahead evidence.

**Questions resolved:** How can a genuine cutpoint be reached? Which public
semantics remain? What history and compatibility must be preserved? How
does the host receive a mid-entry state? No incomplete C stack or manually
assembled partial push is accepted as replay evidence.

**Cross-review:** A stage denotes the next unperformed action; active entry
resumes before fetch/priority arbitration. Preserve origin, partial vector
and consumption facts, account for BRK cycle 1, and save only after completed
bus actions. Validate reachable history before apply, clear it coherently on
reset, and avoid treating entry bus poll temporaries as normal decisions.
Prove initiation separately from fixture-stage execution. This is an explicit
Phase 21 entry-only extension of Phase 20's narrower architecture limit.

## Regression, attribution and delivery evidence

| Option | Pros | Cons / footguns | Selected |
|---|---|---|---|
| Owned fixed expectations + real-bus guests + complete campaign | Independent timing and integration evidence | More than one proof layer, each with distinct purpose | Yes |
| Three target verdicts only | Cheap aggregate signal | Misses lost PASS, stack errors and shared-core replay bugs | No |
| Protect actual 114 starting PASS keys and authenticated main | Includes Phase 19/20 gains | Requires fresh authority; old hosted 108 count is insufficient | Yes |
| Adopt all keyword-matched todos | Convenient batching | Promotes unrelated framework/signal/CI scope | No |
| Reuse existing deterministic properties and selected wrong-model controls | Small owned checks for known failure classes | Does not claim broad mutation coverage | Yes |
| Expand campaigns, raise caps or loosen performance limits | Could hide symptoms | Weakens acceptance and attribution | No |

**Agent-selected under owner delegation:** D-09 through D-13 and D-20 through
D-26. Keep all eleven v5 rows, pin/cap, six-platform equality and existing
vector/MMC3/Holy Mapperel/play/host/performance gates. Source-changing Phase
21 needs its own hosted acceptance; earlier Phase 20 evidence remains history.

**Questions resolved:** What is the starting protection set? Which evidence
layers establish hardware versus deterministic replay? Which hashes may
move and when? Do nearby testing proposals enter scope? All 146 scored rows
are currently terminal, 114/32; 117/29 is only the arithmetic three-target
expectation, never a verdict to manufacture.

## Sources and limitations

- [Pinned AccuracyCoin assembly](https://raw.githubusercontent.com/100thCoin/AccuracyCoin/6bee919529289c555803b612b8c9e4f7eaaeeabb/AccuracyCoin.asm) was fetched directly and inspected for latency/overlap test intent and paired table alternatives.
- [Visual6502 hijack analysis, revision 19268](https://www.nesdev.org/w/index.php?title=Visual6502wiki/6502_Interrupt_Hijacking&oldid=19268) supports the vector-coherence and recognition-stage distinction. Direct access varied; maintained analysis text was also available in indexed web results. No new transistor experiment or paired-chip physical-tie measurement was run.
- Repository source establishes current implementation limitations. Prior accepted Phase 20 evidence establishes its historical results, not future Phase 21 outcomes. The context identifies the exact local references downstream work must read.
- No GPL/LGPL emulator source was opened. No dependency, ROM or external-service mutation was added. Local branch/artifact/session changes are the only mutations of this step.

## Agent discretion

Exact minimal fields, helper names, fixture organization, bounded sweep/control
selection and diagnostic format. Research establishes the cycle/phase crosswalk
and sufficient resumable representation before planning fixes. The delegated
recommendation is recorded once; no recursive artifact-gap research loop.

## Deferred ideas

All three matched todos remain pending proposals: general property helper,
nightly mutation pilot and runner signal redesign. Reuse relevant existing
ideas without promoting those projects. DMA/PAL/other accuracy clusters,
public stepping, generalized sequencing, added blargg fetch-only lanes and
collector CR-01/WR-03/WR-04 remain separately scoped. Earlier sprite/JAM-cache
advisories and unmeasured physical-tie limits remain visible.

## Next step

`$gsd-plan-phase 21`. No planning/execution command ran during this discussion.
