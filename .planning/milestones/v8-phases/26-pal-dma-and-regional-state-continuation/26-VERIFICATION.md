---
phase: 26-pal-dma-and-regional-state-continuation
verified: 2026-11-03T10:04:27Z
status: passed
score: 11/11 must-haves verified
candidate_commit: 6224805a8f94dbbb86ace7847f94d6537ef54c83
candidate_tree: cff5127f0935d3fc9ec938ec09a281ceb9ea44f3
covered_files:
  - .planning/phases/26-pal-dma-and-regional-state-continuation/26-01-PLAN.md
  - .planning/phases/26-pal-dma-and-regional-state-continuation/26-01-SUMMARY.md
  - .planning/phases/26-pal-dma-and-regional-state-continuation/26-02-PLAN.md
  - .planning/phases/26-pal-dma-and-regional-state-continuation/26-02-SUMMARY.md
  - README.md
  - docs/api.md
  - include/nesturbator.h
  - src/bus.c
  - src/cpu.c
  - src/internal.h
  - src/state.c
  - tests/CMakeLists.txt
  - tests/cmake/vector_api_policy.cmake
  - tests/core/test_api.c
  - tests/core/test_state.c
  - tests/core/test_state_fork.c
  - tests/core/test_state_reject.c
  - tests/cpu/test_bus.c
  - tests/libretro/libretro_host.c
  - tests/runner/hashes.txt
covered_digest: "v4:sha256:30f4587c17a79adb2275ddbc558fb6988b5ce1e4e3b26db7f86e28d01b635e31"
behavior_unverified: 0
overrides_applied: 0
flagged_prohibitions: 2
flagged_fallback_rows:
  - DMA-01
  - STATE-01
decision_coverage:
  honored: 9
  total: 9
  not_honored: []
human_verification: []
---

# Phase 26: PAL DMA and regional state continuation — Verification Report

**Phase Goal:** A user can run PAL DMC DMA under the documented 2A07 model and resume a machine state only under its matching resolved region.

**Verified:** 2026-11-03T10:04:27Z
**Status:** Passed. All roadmap and plan truths have current implementation evidence and a passing behavioral check. Both transparency prohibitions remain explicitly flagged.
**Refresh:** The former report fingerprint was stale after its plan summaries gained machine-readable requirements-completed metadata. This is an initial goal-backward pass under the verifier's mode rule because the prior report had no gaps section.

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---:|---|---|---|
| 1 | A generated PAL CPU/APU/bus guest leaves a real DMC request pending during an operand or data read, then accepts it at an eligible opcode/SYNC read and resumes. | ✓ VERIFIED | src/cpu.c:35-39,658 marks only opcode dispatch as SYNC; interrupt C1 uses the same seam at src/cpu.c:499. src/bus.c:28-30,286-289 gates pending PAL acceptance on the loaded PAL profile and SYNC kind. test_pal_sync_halt in tests/cpu/test_bus.c:2209-2320 uses an APU-configured guest and asserts the ordinary pending read, opcode halt, sample GET, resumed read, and continuation. bus.unit passed in the supplied current-head CI run. |
| 2 | An any-eligible-read wrong-model control changes halt, GET, and resume timing on the same PAL history and fails the selected halt-edge oracle. | ✓ VERIFIED | tests/cpu/test_bus.c:2030-2052,2059-2125 takes the control from the same initial guest and saved first-GET state; it asserts equal request cycle/address/phase and different halt, GET, and resume cycles. The fixture checks both the data-read and operand-read alignments. Exact observed cycles are recorded in 26-01-SUMMARY.md. |
| 3 | The observed and ordinarily linked CPU/bus paths agree with the control disabled, while NTSC arbitration and established output pins remain protected. | ✓ VERIFIED | run_pal_sync_halt compares the full instance after each CPU step (tests/cpu/test_bus.c:2070-2090). Production eligibility remains unconditionally true for NTSC (src/bus.c:28-30), preserving its existing bus path. Existing OAM, abort, and NTSC checks remain in bus.unit. The source-history diff updates only the six boards' frame-0/frame-60 state SHA rows; frame/audio pins, state sizes, and ticks are unchanged. The serializer visitor order and field widths are unchanged for NTSC. |
| 4 | Public DMA documentation separates the reported trace from the SYNC interpretation and leaves physical proof and later no-op-cycle effects unresolved. | ✓ VERIFIED | README.md:201-214, include/nesturbator.h:57-64,462-471, and docs/api.md:55-59 identify the forum report as observation, SYNC as interpretation, and the owned control as a software sensitivity check. They explicitly leave direct 2A07 proof and later no-op-cycle behavior unresolved. |
| 5 | New states retain format 6 and ABI 1, record behavior revision 17, and use existing header byte 23 for the loaded region; NTSC pin changes reflect revision metadata and its checksum. | ✓ VERIFIED | src/state.c:34-43,515-524 retains the 64-byte header and writes nesturbator__is_pal(inst) to byte 23. The public constants are ABI 1, behavior revision 17, format 6 (include/nesturbator.h:54,118,143); tests/core/test_state.c:115-155 checks these values and loads a valid revision-16 format-6 state. Comparing the Phase 25 source to the Phase 26 source shows the body visitor order and widths did not change; byte 23 stays 0 for NTSC. tests/runner/hashes.txt changes only the 12 state hashes (six boards at frames 0 and 60); their sizes and tick pins and all non-state hash rows remain unchanged. The recorded 12-payload comparison in 26-01-SUMMARY.md attributes byte differences to revision 16→17 and the consequent CRC. |
| 6 | A same-region PAL restore reproduces frame output and machine continuation, including real pending DMC work and regional fractional history. | ✓ VERIFIED | pal_region_fork in tests/core/test_state_fork.c:253-363 obtains a frame-boundary cut from a generated guest, requiring real dma_pending, nonzero sample_phase, and audio_rem above the NTSC ceiling. It restores through the public API and compares pixels, PCM/count, ticks, frame number, and canonical state over three frames. A control that removes the pending request must diverge. core.state_fork.pal is registered in tests/CMakeLists.txt:1126-1127 and passed in the full run. |
| 7 | PAL remainder, scanline, and PPU/header-gap limits accept their modeled maxima and reject values beyond them, while NTSC keeps its tighter limits. | ✓ VERIFIED | src/state.c:90-93,295-303,412-423 derives the remainder and frame-gap bounds from the loaded region and applies scanline maxima 261/311. regional_bounds in tests/core/test_state_reject.c:1610-1718 checks denominator-minus-one/denominator, scanline maximum/maximum-plus-one, and one-frame-minus-one/one-frame gaps for both regions; accepted edges advance through a public frame. |
| 8 | Opposite-region and unknown-region states fail with the existing identity error before application and preserve the destination and its future behavior. | ✓ VERIFIED | src/state.c:555-608 compares mapper, submapper, region, and ROM identity before length, CRC, body validation, or APPLY. regional_mismatch in tests/core/test_state_reject.c:1125-1248 tests both directions, unknown region values 2/255, and identity combined with malformed size/checksum/body. Existing refusal helpers snapshot the whole instance, RAM, and save generation; the test compares subsequent outputs and canonical state against an untouched same-region instance. core.state_reject.regional is registered at tests/CMakeLists.txt:1144-1145 and passed in the full run. |
| 9 | Existing NTSC format-6 states remain loadable, behavior revision is informational, and the public API ABI/layout is unchanged. | ✓ VERIFIED | The old-revision load/canonical-save assertion is in tests/core/test_state.c:138-155; header and API tests check revision 17, format 6, and ABI 1. The public compatibility contract in docs/api.md:246-253 and include/nesturbator.h:118-143 states the same. Historical revision-16 NTSC state loading is also recorded in 26-01-SUMMARY.md; no state field or public struct was added. |
| 10 | The real libretro module keeps PAL serialization size fixed through frames, reset, successful and refused loads, and preference changes without reload. | ✓ VERIFIED | check_pal_state_size in tests/libretro/libretro_host.c:4817-4870 calls the built module, checks byte 23, round-trips PAL state, refuses a same-content NTSC state without changing serialization, and confirms the latched PAL region and size after frame/reset/option changes. The function is invoked by the registered libretro.host test (tests/libretro/libretro_host.c:5107, tests/CMakeLists.txt:1920-1935), which passed in the full run. |
| 11 | The combined phase implementation passes unfiltered local CI and produces all three deliverables. | ✓ VERIFIED | The supplied fresh cmake --workflow --preset ci receipt is for current HEAD 6224805a8f94dbbb86ace7847f94d6537ef54c83: all 722 CTest tests passed in 434.77 seconds and library, runner, and libretro packages were created. I verified that current CMake, source, headers, tests, README, and API-guide inputs have no diff from the accepted Phase 27.1 source tree. This report refresh did not rerun CI. |

**Score:** 11/11 truths verified (0 present but behavior-unverified).

### Required Artifacts

Both plan artifact queries passed: 26-01, 3/3; 26-02, 4/4. Each was also checked for substantive code and active use.

| Artifact | Expected | Status | Details |
|---|---|---|---|
| src/bus.c | Region-aware SYNC DMC arbitration | ✓ VERIFIED | Non-stub production logic; called through the private read wrapper from src/cpu.c. |
| tests/cpu/test_bus.c | Generated PAL guest and counterfactual | ✓ VERIFIED | Active bus.unit target; checks linked and observed paths, same-history counterfactual, phases, and continuation. |
| include/nesturbator.h | Revision 17 and bounded DMA/state contract | ✓ VERIFIED | Public version constants and limits agree with implementation and README.md/docs/api.md. |
| src/state.c | Format-6 regional writer, identity, and validation | ✓ VERIFIED | Real public save/load path; region identity precedes body validation and application. |
| tests/core/test_state_fork.c | Same-region PAL continuation | ✓ VERIFIED | pal_region_fork is registered and compares real output and state through public APIs. |
| tests/core/test_state_reject.c | Atomic regional refusal and bounds | ✓ VERIFIED | case_regional_state is registered; rejection and edge cases are behaviorally asserted. |
| tests/libretro/libretro_host.c | Fixed-size PAL module serialization | ✓ VERIFIED | Uses the actual built libretro module through libretro.host. |

### Key Link Verification

Both plan link queries passed (3/3 and 4/4); source and test registration checks confirm the connections.

| From | To | Via | Status | Details |
|---|---|---|---|---|
| src/cpu.c | src/bus.c | Opcode dispatch and interrupt C1 call nesturbator__bus_read_sync. | WIRED | The symbol is declared, defined, and used at both intended CPU read sites; operands remain generic reads. |
| src/bus.c | src/internal.h | Effective region controls ordinary pending-DMC eligibility. | WIRED | Production predicate uses nesturbator__is_pal; no runtime option or serialized read-kind state is introduced. |
| tests/cpu/test_bus.c | src/bus.c | Observed source copy and linked production copy run the same PAL history. | WIRED | The source-inclusion aliases call the same production logic; full instance equality is asserted per CPU step. |
| src/state.c | src/internal.h | Regional sample and frame helpers supply validation bounds. | WIRED | nesturbator__sample_ticks_denominator and nesturbator__frame_ticks select the loaded profile's limits. |
| src/state.c | src/state.c | Region identity precedes length, checksum, body validation, and APPLY. | WIRED | The identity branch returns the cartridge error before those checks and before the first state assignment. |
| tests/CMakeLists.txt | tests/core/test_state_fork.c | core.state_fork.pal runs --pal. | WIRED | Registered with a 30-second test cap and included in the unfiltered workflow. |
| tests/CMakeLists.txt | tests/core/test_state_reject.c | core.state_reject.regional runs --regional. | WIRED | Registered with a 30-second test cap and included in the unfiltered workflow. |

### Data-Flow Trace (Level 4)

| Artifact | Data | Source and flow | Status |
|---|---|---|---|
| PAL DMA arbitration | Pending DMC request and read kind | Guest writes and executes → real APU timer/request history → CPU SYNC or ordinary bus read → regional arbitration and sample GET. | ✓ FLOWING |
| Native state writer | Header region and machine state | Loaded instance region and state fields → existing format-6 writer/body visitor → serialized bytes. | ✓ FLOWING |
| Native state loader | Saved region and state fields | Caller bytes → identity check → validate-only visitor and bounds → APPLY only after success. | ✓ FLOWING |
| Libretro state | Region and serialized size | Host loads content/options → built module/core → serialize/unserialize calls → fixed loaded-content size and refusal behavior. | ✓ FLOWING |

This is core-library behavior and has no rendered UI or database-backed data.

### Behavioral Spot-Checks

| Behavior | Command/evidence | Result | Status |
|---|---|---|---|
| PAL DMA, same-region continuation, regional refusal, revision compatibility, and real adapter lifecycle | bus.unit, core.state_fork.pal, core.state_reject.regional, core.state, and libretro.host, included in the supplied current-head workflow | All named regressions are part of the 722/722 passing CTest run; no test was rerun during this report refresh. | ✓ PASS |
| Required combined workflow and package creation | Fresh cmake --workflow --preset ci receipt for current HEAD | 722/722 tests passed; CTest 434.77 seconds; library, runner, and libretro packages created. | ✓ PASS |

The Phase 27.1 acceptance record identifies retroarch.session.controls as the longest check at 262.59 seconds and found no safe reduction that preserves its unique coverage. The current receipt still fits the project's 30-minute workflow limit. No CI change or repeat run is warranted for this report refresh.

### Probe Execution

Not applicable. This is a core-behavior phase; neither plan declares a probe, and no phase probe was found.

### Requirements Coverage

The plans declare DMA-01 (26-01) and STATE-01 plus DMA-01 (26-02). The machine-readable summary metadata now records requirements-completed: [DMA-01] in 26-01 and requirements-completed: [STATE-01] in 26-02. Both plan IDs therefore have a completed summary record, and no requirement was dropped from the coverage check.

| Requirement | Source plan(s) | Summary metadata | Description | Status | Evidence |
|---|---|---|---|---|---|
| DMA-01 | 26-01, 26-02 | 26-01 marks complete | Best source-backed PAL 2A07 model with a distinguishable control and explicit hardware/no-op evidence limits. | ✓ SATISFIED | Truths 1-4; production-path guest/control, documentation, and passing bus.unit. The unresolved transparency prohibition and descriptor-less fallback row remain flagged. |
| STATE-01 | 26-02 | 26-02 marks complete | Same-region continuation, identity-first refusal, and accurate format/revision/ABI contract. | ✓ SATISFIED | Truths 5-10; native state, bounds, refusal, compatibility, and real libretro checks pass. The universal-compatibility prohibition and descriptor-less fallback row remain flagged. |

.planning/REQUIREMENTS.md maps no additional requirement to Phase 26. There are no orphaned requirement IDs.

### Decision Coverage

The decision-coverage query reports: “All trackable CONTEXT.md decisions are honored by shipped artifacts.” It found 9/9 decisions and no unhonored items. This is a non-blocking coverage check.

### Test Quality Audit

| Test file | Linked requirement | Active | Skipped | Circular | Assertion level | Verdict |
|---|---|---:|---:|---:|---|---|
| tests/cpu/test_bus.c | DMA-01 | Yes | No | No | Behavioral: same-history control and cycle/address assertions | PASS |
| tests/core/test_state_fork.c | STATE-01 | Yes | No | No | Behavioral: public restore and multi-frame output/state equality | PASS |
| tests/core/test_state_reject.c | STATE-01 | Yes | No | No | Behavioral: bounds, atomic rejection, future output/state equality | PASS |
| tests/core/test_state.c | STATE-01 | Yes | No | No | Value: revision 16 load and canonical revision-17 save | PASS |
| tests/libretro/libretro_host.c | STATE-01 | Yes | No | No | Behavioral: real module round-trip, refusal, and fixed-size lifecycle | PASS |

Disabled-test scanning produced only false-positive text matches: the C exit(2) argument-error path and the emulator's ordinary .pending state field. Neither is a skipped test. Fixture writes generate owned guest inputs; the expected DMA edge and state continuation are asserted separately and are not generated by capturing the tested output as its own oracle. No circular expected-value generator was found.

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|---|---:|---|---|---|
| None | — | No unreferenced TBD, FIXME, or XXX; no placeholders, empty implementations, or disabled requirement-linked tests. Grep matches were normal null/error paths and DMA pending-state fields. | — | No blocking anti-pattern. |

### Retained Limits and UAT Disposition

The implementation truths pass; the following planning prohibitions and fallback rows remain unverified and are not counted as satisfied evidence:

| Requirement | Retained prohibition | Disposition |
|---|---|---|
| DMA-01 | Do not present the trace-derived rule or owned fixture as direct 2A07 hardware proof or as a settled later no-op-cycle contract. | Flagged/unresolved. Direct hardware proof and later no-op-cycle effects remain unknown. |
| STATE-01 | Do not claim identical historical behavior or universal cross-region/cross-build compatibility. | Flagged/unresolved. Claims stay bounded to tested format, region, content, and build evidence. |

The two former administrative prompts have completed bounded plan/source/documentation review in 26-AUTOMATION-DISPOSITION.md and 26-UAT.md. That disposition preserves both flags; it does not resolve them or claim physical/historical proof. Per the project verification policy, no gameplay or hardware UAT is required. No user action remains.

### Human Verification Required

N/A — core-library phase with no user-facing UI or external service. All behavioral criteria have passing automated checks, and the former administrative acknowledgments have a recorded policy-compliant disposition. No behavior-dependent truth remains untested.

## Gaps Summary

No roadmap success criterion, plan truth, artifact, key link, or requirement failed. The PAL DMA path is wired through production CPU/APU/bus code and distinguishes the wrong model on the same PAL history. Regional state continuation and pre-apply cross-region refusal are implemented and covered through native and libretro tests. The fresh current-head workflow passed all tests and produced all three packages.

DMA-01 and STATE-01 transparency prohibitions and their descriptor-less fallback rows remain explicitly flagged. That preserves the unresolved hardware, no-op-cycle, and universal historical/cross-build boundaries without turning them into a request for gameplay or hardware UAT.

---

_Verified: 2026-11-03T10:04:27Z_
_Verifier: the agent (gsd-verifier)_
