---
phase: 21-interrupt-latency-and-hijack
plan: "01"
subsystem: testing
tags: [C, CTest, CPU, IRQ, AccuracyCoin]

# Dependency graph
requires:
  - phase: 20-nmi-line-and-vblank-flag
    provides: Production bus capture seam and owned interrupt-test patterns
provides:
  - Real APU frame-IRQ guest tracer and old-I expected-red cases
  - Exact CPU branch access table and test-only IRQ poll schedules
affects: [21-02 IRQ poll implementation]

# Actuals (#2632): chars/4 across the realized implementation/test diff; commit count measured from the plan ledger.
actuals:
  tokens: 13070.5
  tasks: 2
  commits: 10
plan_head_before: 3c4a29b79cf92aca52bbe21bc936adbce51e6d7a
plan_head_after: 7dff18a476b89029af8627fbadb3cf7fd56f5c39

# Tech tracking
tech-stack:
  added: []
  patterns:
    - Real APU/bus source proof kept separate from CPU-only injected IRQ schedules
    - Named pre-fix defects use exact-count expected-red assertion wrappers

key-files:
  created:
    - tests/core/test_irq_poll.c
    - tests/interrupt_fixture.h
  modified:
    - tests/CMakeLists.txt
    - tests/accuracy/baseline-metadata.txt
    - tests/cpu/test_cpu_unit.c
    - tests/cpu/vector_bus.c
    - tests/cpu/vector_bus.h

key-decisions:
  - "Use the production APU and bus for source and handler evidence; label flat-bus schedules as sequence-only evidence."
  - "Keep known poll defects as named expected-red checks until their causal fixes land in a later plan."
  - "Preserve the AccuracyCoin result counts and report hash; refresh only the build-input digest required by the added test registration."

patterns-established:
  - "Source proof: observe frame IRQ line, capture, poll, entry stack, vector and guest marker through the shipped bus."
  - "Sequence proof: inject finite IRQ transitions in the flat test bus and assert exact accesses, poll samples and stack bytes."

requirements-completed: []

# Coverage metadata
coverage:
  - id: D1
    description: "Production-bus APU IRQ tracer with CLI, SEI, CLI-to-SEI and both PLP old-I transitions."
    verification:
      - kind: integration
        ref: "ctest --preset ci -R '^(core\\.irq_poll|core\\.apu|core\\.nmi_poll)$' --no-tests=error --output-on-failure"
        status: pass
    human_judgment: false
  - id: D2
    description: "All eight branch opcodes across not-taken, same-page, forward-cross and backward-cross access sequences, plus poll timing and acknowledgment controls."
    verification:
      - kind: unit
        ref: "tests/cpu/test_cpu_unit.c#test_branch_opcode_access_table"
        status: pass
      - kind: unit
        ref: "ctest --preset ci -R '^(cpu\\.unit|core\\.irq_poll|core\\.nmi_poll|cpu\\.vectors\\..*)$' --no-tests=error --output-on-failure"
        status: pass
    human_judgment: false
  - id: D3
    description: "RTI restored-I inhibition, immediate acceptance and unacknowledged retrigger guards."
    verification:
      - kind: unit
        ref: "tests/cpu/test_cpu_unit.c#test_rti_restored_i_and_unacknowledged_retrigger"
        status: pass
    human_judgment: false

# Metrics
duration: 85min
completed: 2026-10-29
status: complete
---

# Phase 21 Plan 01: Interrupt Poll Evidence Summary

**Production APU IRQ tracing and CPU poll schedules now pin old-I, branch, and RTI behavior with pre-fix regression cases.**

## Performance

- **Duration:** 85 min
- **Started:** 2026-10-29T00:27:34.902Z
- **Completed:** 2026-10-29T01:52:05Z
- **Tasks:** 2
- **Files modified:** 11, including the deferred-item note for the preexisting planning-ledger issue

## Accomplishments

- Added a real frame-IRQ path through the production APU source, bus capture, poll, seven entry accesses, stack, vector and a guest handler marker. Added production-bus CLI, SEI, CLI-to-SEI and both PLP old-I transitions.
- Added exact bus access tables for BPL, BMI, BVC, BVS, BCC, BCS, BNE and BEQ covering not-taken, taken same-page, forward page-cross and backward page-cross branches, plus wrapping accesses. Test-only IRQ schedules distinguish first poll, page-cross second poll, late IRQ and `$4015` acknowledgment ordering.
- Filled the branch timing matrix around both legal polls. Flat-bus cases cover source rise before/after C1 and before/on/after C4, plus `$4015` acknowledgment before C1, on C1, between C1 and C4, on C4, and after C4. They assert captured history, current line, both poll samples and entry state; the test bus remains labeled sequence-only.
- Reproduced pinned AccuracyCoin C and D prerequisites through the production PPU/APU/bus: `$2002` write `$44` reads back through `$3FA5`, and a DMC GET of owned byte `$5A` makes a parked `$4000` read return `$5A`. The observed AccuracyCoin DMC-sync startup pretest was also `0x01`.
- Added an owned production-bus E source reproducer: a DMC GET from owned NROM supplies `$90` before branch C1, then the real APU frame IRQ rises and is captured before C1. The branch's real C3 `$4015` read acknowledges the source before C4. At C4 the production bus samples prior captured-high history while the source line and newly captured level are low, and the guest enters IRQ at `$8200` with return `$3FA5`, pushed P `$20`, and its marker. This confirms first-poll source qualification and survival of the real status acknowledgment through guest IRQ entry; the measured C4 poll history is still high, so this production case is not the isolated wrong-model discriminator. The flat-bus expected-red case separately forces captured level low at C4 and isolates lost-first-acceptance behavior. No production timing changed.
- Added bounded wrong-result controls for live-I recheck, delayed RTI polling, extra same-page polling, missing cross-page polling and loss of first acceptance after `$4015`; each intended assertion set rejects its wrong outcome while clean sibling observations pass. These controls compare owned expected observations rather than running separate mutated CPU binaries.
- Added RTI restored-I inhibition, accepted IRQ and immediate unacknowledged retrigger checks. The normal focused suite passes, and the complete CI workflow passes 689/689 tests and packages all three deliverables.
- Preserved the starting AccuracyCoin result of 114 PASS / 32 FAIL, 146 terminal scored rows and five DRAW entries. No campaign improvement is claimed.

## Task Commits

1. **Task 1: Trace an owned APU IRQ from production bus to guest handler** — `a291994` (raw red tests), `fd5af15` (named SEI expected-red wrapper).
2. **Task 2: Expand exact I-flag and branch poll discriminators** — `5e46e56`, `c403184`, `d074ee8` (red evidence, sequence coverage and expected-red wrappers), `1dd904b` (source-window and C/D/E evidence), `7dff18a` (align the production DMC source setup with the pinned C1 sequence).

**Rule 3 CI metadata fix:** `78d561d` refreshed the AccuracyCoin build-input digest after the planned CTest registration changed `tests/CMakeLists.txt`.

**Plan metadata commits:** `00db093` recorded the original execution closeout; `180c752` clarified uncovered poll/C/D/E evidence; `1dd904b` added source-window, C/D prerequisite and E source coverage.

## Files Created/Modified

- `tests/core/test_irq_poll.c` — Production APU frame IRQ, handler, CLI/SEI/PLP source cases, pinned C/D prerequisite checks and the real APU/DMC E source/acknowledgment case.
- `tests/interrupt_fixture.h` — Owned NROM guest and IRQ/NMI marker setup.
- `tests/CMakeLists.txt` — Registers bounded `core.irq_poll` coverage.
- `tests/cpu/test_cpu_unit.c` — Branch access matrix, scheduled poll discriminators, PLP and RTI cases.
- `tests/cpu/vector_bus.c` and `tests/cpu/vector_bus.h` — Optional flat-bus IRQ scheduling and trace data, clearly separate from production sampling.
- `tests/accuracy/baseline-metadata.txt` — Updated only `build_inputs_sha256`; result counts and campaign report hash are unchanged.
- `.planning/phases/21-interrupt-latency-and-hijack/deferred-items.md` — Records the existing `.planning/WINDOWS.md` consistency issue that blocked appending this deviation.

## Decisions Made

- Production source evidence uses the real frame IRQ writer and bus. Flat-bus schedules prove access and sequence expectations only.
- Named pre-fix defects remain as expected-red wrappers with exact failure counts; production timing code is unchanged in this plan.
- `INT-01` and `KEEP-01` remain pending at phase scope. The requirements query found 0/2 ready to mark complete because the planned behavior fixes and all-lane evidence are still outstanding.

## TDD Evidence

- **Task 1 RED:** `core.irq_poll` JUnit evidence returned `RED_EVIDENCE_OK`. The real frame IRQ source and captured poll were present; the five intended post-SEI assertions failed because current dispatch lost acceptance after I became set. The independent ordinary IRQ and CLI guards passed.
- **Task 2 RED:** `cpu.unit` and `core.irq_poll` JUnit records returned `RED_EVIDENCE_OK`. The original semantic checks confirmed same-page late acceptance, loss of earlier acceptance after a flat-bus `$4015` acknowledgment, both PLP old-I errors, and the CLI-to-SEI live-I error. The production APU/DMC case now follows E's order: DMC supplies `$90` before C1, the real frame IRQ is captured before C1, and real `$4015` at C3 clears the source before C4. Since C4 samples prior captured-high history, that production case demonstrates the real source path and guest outcome but is not claimed as the isolated lost-first-acceptance expected-red evidence; that is provided by the flat-bus C4-low wrapper.
- The separate CLI-to-SEI raw record also returned `RED_EVIDENCE_OK`; its five intended assertions failed after SEI while source and capture remained present.
- **GREEN:** The task is test evidence only, so production behavior intentionally remains pre-fix. Each named defect now requires its intended assertion count and restores the check counter; all ordinary guards pass. Final focused validation passed 262/262. Final `cmake --workflow --preset ci` passed 689/689 in 434.49 seconds and generated library, runner and libretro packages.
- **REFACTOR:** No production or timing refactor was in scope.

## Wrong-Model Control Assertions

- **Live-I recheck after SEI:** the production APU SEI and CLI-to-SEI cases each require five dispatch/stack/marker assertions to fail under the current lost-acceptance behavior. Their expected pushed status keeps I set (`$24`) even though eligibility used old I=0.
- **PLP old-I eligibility:** both old-I/new-I transitions are present in the flat-bus and production-source tests. The production cases require two failures for old I=1/new I=0 and four for old I=0/new I=1; they check the next instruction or immediate IRQ entry and pushed status.
- **Extra same-page poll:** a line rising just after the first poll must not interrupt before the branch-target instruction and following INX; the named expected-red wrapper checks exactly two failures.
- **Missing or late page-cross poll:** normal passing guards require a rise after the first poll to be accepted at the page-cross second poll, and a rise after that second poll to wait until the following instruction's poll. These are exact PC, sample and stack assertions, not expected-red wrappers.
- **Lost earlier acceptance after `$4015`:** the flat-bus wrapper requires four dispatch/return-PC/status failures after C1 acceptance is cleared before C4, explicitly making the current source line and C4 captured level low while the first poll's acceptance should persist. A clean sibling records the expected IRQ vector, return `$3FA5` and pushed `$20`. The production APU/DMC E source case separately performs the real C3 `$4015` acknowledgment, but C4's bus close samples prior captured-high history; it therefore does not replace the discriminating flat-bus control.
- **Delayed RTI poll:** normal guards require restored I=1 to return to the next instruction without IRQ, and restored I=0 to enter IRQ on the next dispatch and retrigger after an unacknowledged RTI.
- All eight branch opcodes have exact not-taken, same-page, forward-cross and backward-cross bus access tables. Dynamic poll schedules use BPL as the sequence representative; flat-bus schedules are not production source-sampling evidence.

## AccuracyCoin C/D/E Evidence and Attribution

- **C:** The pinned `$2002`/`$3FA5` PPU I/O readback check passes with `$44` through production PPU and CPU bus paths.
- **D:** The pinned DMC data-bus prerequisite is represented by a production DMC GET of owned `$5A` followed by a parked `$4000` read returning `$5A`; AccuracyCoin observation metadata reports its DMC sync startup pretest as `0x01`. The test seeds the existing pending-DMA state to isolate that transfer and does not claim to rerun the ROM's full 50-cycle synchronization helper.
- **E:** The pinned source requires the first branch poll to retain IRQ acceptance through its C3 `$4015` dummy-read acknowledgment, then enter with return `$3FA5`, pushed P `$20`, and result marker `$5A`. The owned real-source branch fixture uses a DMC GET of `$90` before C1, then raises/captures the real frame IRQ and performs the real acknowledgment at C3. C4 samples prior captured-high history although the IRQ line and newly captured level are low; the production CPU reaches the expected guest entry and stack, and the owned handler counter increments. This proves handler entry rather than the ROM's literal `$5A` result byte. Because the C4 poll history remains high, the production case does not independently reject a lost-first-acceptance model. The flat-bus C4-low expected-red control supplies the isolated first-acceptance divergence and requires four intended failures while its clean sibling passes.
- The latest retained production page-12 observation uses AccuracyCoin ROM commit `6bee919` and source revision `53b32c2baf65`; it records `Interrupt flag latency: fail, 0x0a` at 141 execution frames, with DMC-sync startup pretest `0x01`. The tracked scoreboard baseline is `0x0a` at frame 151. Owned production C and D bus checks pass. The retained `raw.tsv` reports only aggregate result byte `$0A` at `$0461`; the observer has no original-ROM C/D/E subtest PC or marker, so the campaign artifact cannot identify which assertion first diverged. No full new campaign or post-fix improvement is claimed.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Refreshed the AccuracyCoin build-input digest after test registration**
- **Found during:** Overall verification
- **Issue:** The new `core.irq_poll` registration changes the fingerprinted `tests/CMakeLists.txt`; `accuracycoin.activation` rejected the stale `build_inputs_sha256`.
- **Fix:** Updated only the metadata digest to the current input-set fingerprint. The 114/32 counts, terminal-row count, DRAW count and campaign report hash remain unchanged.
- **Files modified:** `tests/accuracy/baseline-metadata.txt`
- **Verification:** `accuracycoin.activation` passed; the subsequent complete workflow passed 689/689 and packaging.
- **Committed in:** `78d561d`

**Total deviations:** 1 auto-fixed (Rule 3)
**Impact on plan:** Required to keep the existing input-identity check valid after adding a test target; campaign observations and source behavior were not changed.

## Issues Encountered

- The optional `gsd_run windows append` could not append the deviation because `.planning/WINDOWS.md` has inconsistent counts: its frontmatter says 18 entries, while its authoritative JSON and table have 17 records. The existing ledger issue is recorded in `deferred-items.md`; the ledger was not otherwise edited.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- Plan 02 can remove each expected-red wrapper as its corresponding causal poll fix is implemented and verified.
- Owned production C and D prerequisite checks pass, and the E source schedule reaches the real APU/DMC/bus guest path with its pinned expected entry state. The exact in-ROM first failing subtest PC/marker remains unavailable from the pinned page observer, which reports only aggregate row failure `0x0a` at frame 151. The owned flat-bus C4-low case localizes first-acceptance loss in its modeled schedule; it is not claimed as an exact ROM subtest attribution.
- `INT-01` and `KEEP-01` remain pending until phase-wide behavior and all-lane requirements are met.

---
*Phase: 21-interrupt-latency-and-hijack*
*Completed: 2026-10-29*

## Self-Check: PASSED

The summary exists, and all ten measured plan-ledger commits from `plan_head_before` through `plan_head_after` are ancestors of the current branch head. Focused validation passed 262/262, the full workflow passed 689/689, and state and roadmap artifacts are present.
