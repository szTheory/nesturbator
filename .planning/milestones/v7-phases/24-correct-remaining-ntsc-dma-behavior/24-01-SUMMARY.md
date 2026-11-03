---
phase: 24-correct-remaining-ntsc-dma-behavior
plan: "01"
subsystem: apu-dma-state
tags: [ntsc, dmc, dma, native-state, clean-room]
requires:
  - phase: 21
    provides: Write retry and bus phase seam
provides:
  - DMC silent output phase correction with owned landing regression
  - Five independent DMA first-divergence records
  - Format-4 native silence/pending continuation and atomic refusal coverage
affects: [24-02, 24-03, 24-04]
actuals:
  tokens: 22410
  tasks: 3
  commits: 4
tech-stack:
  added: []
  patterns: [compile-time source observation, public-boundary native fork]
key-files:
  created:
    - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-DMA-EVIDENCE.md
    - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-01-REVIEW.md
  modified:
    - src/apu.c
    - src/internal.h
    - src/state.c
    - tests/cpu/test_bus.c
    - tests/core/test_state_fork.c
    - include/nesturbator.h
    - README.md
key-decisions:
  - Owner approved the concrete silence field, continuous bits meaning, format 4 and behavior revision 10 before mutation; no converter.
  - Tasks 1 and 3 land together to keep behavior, serializer, native continuation and docs coherent.
  - Keep historical AccuracyCoin evidence at revision 9; do not relabel it as fresh revision-10 acceptance.
requirements-completed: []
requirements-pending: [DMA-01, DMA-02, DMA-03, DMA-04]
coverage:
  - id: landing-owned
    description: Silent bit clock and source-shaped delayed-load landing continuation
    verification:
      - kind: unit
        ref: bus.unit
        status: pass
    human_judgment: false
  - id: dma-native
    description: Reachable silent and pending native cuts with full output/state equality and refusal controls
    verification:
      - kind: integration
        ref: core.state_fork --dma
        status: pass
    human_judgment: false
  - id: campaign-acceptance
    description: Required full pinned, protected-key, platform-hash and performance acceptance
    verification:
      - kind: other
        ref: 24-04 acceptance gate
        status: unknown
    human_judgment: true
    rationale: Existing runtime stop requires separate owner authorization before execution; no acceptance is inferred.
duration: 94min wall including checkpoint
completed: 2026-10-31
status: complete
---

# Phase 24 Plan 01 — silent DMC phase, independent traces and native continuation

**The output bit counter now advances through silence, so the delayed-load landing fixture fetches on the expected final read; native states preserve that phase.** This completes only this plan's bounded work. Phase and requirement acceptance remain incomplete.

## Task commits

- Initial failing diagnostics / pre-mutation checkpoint: `7348a10`.
- Tasks 1 and 3, coherently combined under the approved native-state decision: `fe735e6` — output correction, format 4 / behavior 10, visitor validation, real native fork/refusal tests, docs and coupled version/hash consumers.
- Task 2 independent evidence: `be4fef5` — five separate hardware/test/core records; no other row is claimed corrected.
- This metadata commit records summary/review; state and roadmap are updated afterward.

## Results and evidence

The [DMA ledger](24-DMA-EVIDENCE.md) records per-row source identity, setup, expected and observed events, first divergence, wrong-model candidate, command and limits. It explicitly withdraws early conclusions from bad synthetic addressing modes, approximate delay, incoherent phase pairing and an out-of-range long counter. The corrected wait has an asserted exact duration. The direct source observer compiles away in production.

The [approved checkpoint](24-01-CHECKPOINT.md) and [ownership amendment](24-01-AMENDMENT.md) record the concrete irreversible choice and necessary version consumers. Serialized DMC size changes 22->23 bytes, adding silence at local offset 15 and changing bits at offset 14 to continuous silent/audible cycle position. C-struct padding is not serialized. Version-3 states are refused, ABI stays 1, and no converter exists. Six native state board sizes grow one byte; their frame ticks and every non-state hash row stay unchanged.

The public native fork reaches frame 1 silent phase and frame 29 pending DMA through owned guest execution. Reference A never loads. B restores, then full pixels, PCM, metadata, RAM effects, field-wise transition cycle/level events and canonical final bytes match. Each selected continuation has 68 DAC transitions. A valid-valued wrong silence history changes PCM; invalid silence and an old-format header are refused atomically. --dma is also invoked from existing --stress; synchronous DMA has no public mid-transfer cut.

## Verification

Read-only generated CTest JSON verified the selected closures contain no campaign fixture. Commands and results:

- Bounded builds: bus.unit, core.apu, core.state_fork, core.api, header.c/cxx and documented runner helper targets — passed.
- bus.unit/core.apu/core.api, C/C++ header tests, state.format_pin, vectors.api_policy and mutation self-test — passed.
- docs.reproduction and mutation self-test — passed under their existing 60-second limits after the missing documented helpers were built. No timeout was raised.
- core.state_fork --dma — passed both affected real frame cuts and negative controls.
- core.state_reject --short on committed dabg/MMC1/MMC3 fixtures — passed, including all old/future format cases and the new silence bound.
- Current silence/landing/remaining trace modes — exit 0; remaining is diagnostic capture, not four corrections passing.
- Current bus executable linked against the retained pre-fix APU source — silence/landing/remaining exit 1. This isolates the counter correction while retaining current test/layout declarations; it is not a full old-build state compatibility claim.
- clang-format 18 checks and diff whitespace checks — passed. Normal git hygiene hooks ran on commits.

[Code review](24-01-REVIEW.md) found missing version consumers; they were corrected, verified and re-reviewed. The remaining stale format sentence was corrected and verified by the doc contract. No active review finding remains.

## Deviations and boundaries

Task 1 and native Task 3 were committed together because the plan explicitly forbids a behavior correction without its coherent serializer/continuation work. The format bump exposed coupled version pins/docs outside the original list; the scoped amendment adds only those necessary consumers and preserves the historical campaign metadata. No requirement was checked complete from the plan frontmatter.

The inherited generic post-wave test command is `cmake --workflow --preset ci`. Existing build/ci explicitly has NESTURBATOR_ACCURACYCOIN_FULL=ON, and hosted Linux x64 enables the same campaign fixture. It has not been run. Plan-specific authorized bounded verification is recorded separately; the full workflow gate remains pending for the phase's explicit acceptance checkpoint. No campaign stop was cleared and no gate is reported passed from bounded results.

## Next work

Plan 02 can use the independent OAM/conflict traces; Plan 03 can use the separate stop traces. They must retain this landing/native guard and stop before any additional saved-field/layout/meaning change not covered by the silence decision. Plan 04 still owns separately authorized acceptance. No ROM result, gameplay qualification, cross-platform equality or performance result is newly claimed here.

## Self-Check: PASSED

All named created artifacts exist; commits 7348a10, fe735e6 and be4fef5 are reachable on the active phase branch; the latest bounded correction/continuation/version/doc checks pass. The phase, requirements and campaign acceptance remain pending explicitly.
