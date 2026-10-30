---
phase: 21-interrupt-latency-and-hijack
plan: "04"
subsystem: cpu
tags: [interrupts, dmc, open-bus, state, accuracycoin]
requires:
  - phase: 20
    provides: NMI edge history and poll-time capture
  - phase: 21-03
    provides: Source-qualified interrupt cutoff and paired overlap schedules
provides:
  - Shared, instance-owned BRK/IRQ/NMI entry continuation with coherent vector selection
  - Reachable native-state serialization and validation for partial interrupt entry
  - Measured local campaign and protected-main evidence for the final candidate
affects: [phase-21-follow-up, native-state, cpu-interrupts, hosted-acceptance]
actuals:
  tokens: 29956
  tasks: 3
  commits: 1
plan_head_before: a23394036f7197a7f4a2f265c8f2de53a6c11fa3
plan_head_after: db15c9b92ea08cc63bf8d88db9360ef4a5989727
tech-stack:
  added: []
  patterns:
    - Instance-owned interrupt entry stages serialized through the existing state visitor
key-files:
  created:
    - .planning/debug/dmc-jsr-openbus.md
    - .planning/phases/21-interrupt-latency-and-hijack/21-DMC-PHASE-AUDIT.md
  modified:
    - src/cpu.c
    - src/bus.c
    - src/state.c
    - tests/core/test_interrupt_entry.c
    - tests/accuracy/measurements.md
    - tests/accuracy/scoreboard.txt
    - tests/cmake/accuracycoin_regression_controls.cmake
    - README.md
    - include/nesturbator.h
key-decisions:
  - "Keep the approved state format 3 / behavior revision 9 contract and public ABI unchanged."
  - "Use one coherent causal commit for all three tasks after the bounded prerequisite repair and all gates passed."
  - "Keep hosted all-lane acceptance and INT-01, INT-02, KEEP-01 open for later Phase 21 work."
patterns-established:
  - "Persist only reachable interrupt-entry continuation state through the canonical visitor and pre-APPLY validation."
  - "Retry an eligible DMC halt blocked by a CPU write on the following CPU cycle, preserving existing timer and GET ordering."
requirements-completed: []
coverage:
  - id: D1
    description: Shared BRK, IRQ and NMI entry preserves origin semantics, coherent vector selection, and resumable state.
    verification:
      - kind: unit
        ref: "core.interrupt_entry, core.irq_poll, core.apu, core.controller, core.state_reject"
        status: pass
      - kind: integration
        ref: "cmake --workflow --preset ci (691/691 tests)"
        status: pass
    human_judgment: false
  - id: D2
    description: Final source-bound local campaign retains protected results and passes all eleven v5 target rows.
    verification:
      - kind: integration
        ref: "AccuracyCoin full campaign; 119 PASS / 27 FAIL, 146/146 terminal, 114/114 protected PASS retained"
        status: pass
      - kind: other
        ref: "Fresh origin/main authority a64c23fd3ca7caed7e3ad102ff1a9beaa29fa562; all eleven v5 target rows pass"
        status: pass
    human_judgment: false
metrics:
  duration: 21m
  completed: 2026-10-29
  status: complete
---

# Phase 21 Plan 04: Shared interrupt entry and coherent vector takeover

**BRK, IRQ and NMI now share a resumable entry path with coherent vector selection, and the bounded DMC/open-bus prerequisite repair restores the full local target campaign.**

## Performance

- **Duration:** About 21 minutes for this continuation; the exact start timestamp was not persisted.
- **Started:** Approximately 2026-10-29 11:41 UTC.
- **Completed:** 2026-10-29.
- **Tasks:** 3/3.
- **Files modified:** 24 in the causal source commit; planning evidence is committed separately.

## Accomplishments

- Routed BRK, IRQ and NMI through a shared begin/action/drain mechanism. Entry origin, next action, selected vector and partial byte live in the CPU instance and serialize through the existing visitor; normal public execution remains synchronous.
- Added pre-APPLY validation for reachable entry histories and documented the format 3 / behavior revision 9 state contract. Public ABI and format revision remain unchanged; native state hashes and sizes reflect the four-byte continuation addition while frame, PCM and APU-transition hashes remain unchanged.
- Repaired the approved prerequisite: a write-blocked eligible DMC halt retries on the next CPU cycle, and controller D5-D7 retain their open-bus value. Independent revert checks show each correction is required. APU timer order, global phase, GET retention and mapped-read overwrite behavior were preserved.
- Refreshed scoreboard and campaign fixtures to 119 PASS / 27 FAIL, updated six measured page-14 migration links to frame 122, and regenerated all 17 state corpus entries. The seed bytes and ROM manifest rows were unchanged.
- Re-fetched protected-main authority at `a64c23fd3ca7caed7e3ad102ff1a9beaa29fa562`; all 114 starting PASS keys remain PASS and all eleven v5 targets pass. The campaign completed 22/22 pages and all 146 scored rows, with five inventory-only DRAW rows and 29 status/raw/frame row changes.

## Verification

- Full `cmake --workflow --preset ci`: **691/691 tests passed** in 358.64 seconds; library, runner and libretro packages were generated.
- Final AccuracyCoin campaign: 119 PASS, 27 FAIL, 146/146 terminal, zero skip/not-run/timeout, five DRAW, 114/114 protected PASS keys retained, and 11/11 v5 target rows passing. All 12 source/binary/report identity entries matched the final artifacts.
- Final campaign report SHA-256: `56ebd01adb97b3916fde36f8739602284d951a1fd00d84285bb4ef441fb463a3`.
- Final 56-input build fingerprint: `c1aeeddca9b0b5a2641d66ffa9309998dfb67bb620cb1cbb94d6c59534865283`.
- `runner.write_hashes` and its content comparison passed byte-for-byte. State corpus regeneration and `--check-seeds` matched all 17 entries; the ROM manifest did not change.
- `scripts/hygiene.sh` and `git diff --check` passed.

## Task Commit

All three tasks share the single green causal commit required by the plan:

- `db15c9b92ea08cc63bf8d88db9360ef4a5989727` — `fix(21-04): complete shared interrupt entry and DMC retry`.

The planning metadata commit is recorded in the execution completion report.

## Files Created and Modified

- Created `21-DMC-PHASE-AUDIT.md` and `.planning/debug/dmc-jsr-openbus.md` with final source-bound root-cause, independent-revert and identity evidence.
- Updated production CPU, bus and state modules; interrupt, DMC, controller, poll and state tests; public comments and README; campaign scoreboard, migration observations, fixtures and metadata.
- Updated plan path metadata to reflect the files actually changed. Generated state corpus entries and `tests/roms/manifest.txt` were verified unchanged and are not listed as modified.

## Decisions Made

- Keep Plan 21-04 as one causal source commit after Tasks 1–3, compatibility checks, campaign gates and full CI all passed.
- Keep format 3 / behavior revision 9 and the public ABI. Partial interrupt-entry state is validated before APPLY through the existing state visitor.
- Preserve the exact physical paired-chip tie as unmeasured. The tested finite-margin timing policy remains unchanged.
- Keep INT-01, INT-02 and KEEP-01 pending phase-level hosted and every-lane acceptance.

## Deviations from Plan

### Owner-approved prerequisite repair

The owner approved a bounded DMC/open-bus investigation after the pre-repair `$4013` fetch returned `$40` instead of the source-expected `$90`. The repair is limited to next-cycle retry after a write-blocked DMC halt and retaining controller D5-D7. Owned regressions and independent one-fix-at-a-time reverts establish the causal need for both changes. No DMA cluster, timer-order change, pin/cap/table change, or remote delivery was added.

### Measurement-driven fixture refresh

The final campaign-derived page-14 regression fixture and six migration links were refreshed to the measured frame-122 rows. This corrected stale observations without weakening assertions. The final report, fingerprint and source/binary identity checks were rerun after the test-fixture changes.

## Issues Encountered

An initial local closeout still contained candidate-derived page-14 references at frame 121. The final measured report places those observations at frame 122; fixtures and migration references were corrected, focused regression controls passed, and the subsequent full workflow passed.

## User Setup Required

None.

## Next Phase Readiness

Plan 21-04 is complete and Plans 21-05 onward are unblocked by this plan. Hosted cross-platform evidence and all-lane protected acceptance remain assigned to later Phase 21 work. Phase requirements are intentionally not marked complete here.

## Self-Check: PASSED

- Summary, checkpoint, phase audit and debug evidence files exist.
- The measured causal commit is an ancestor of HEAD; the persisted plan ledger reports one task/source commit (`a23394036f7197a7f4a2f265c8f2de53a6c11fa3` → `db15c9b92ea08cc63bf8d88db9360ef4a5989727`).
- The GSD evaluation-scope check resolved Plan 21-04 with no missing-on-disk paths or unreachable commits.
- `git diff --check` passes.

---
*Phase: 21-interrupt-latency-and-hijack*
*Completed: 2026-10-29*
