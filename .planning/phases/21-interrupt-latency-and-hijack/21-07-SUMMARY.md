---
phase: 21-interrupt-latency-and-hijack
plan: "07"
subsystem: testing
tags: [libretro, runahead, interrupt-entry, native-state]
requires:
  - phase: 21-06
    provides: Production-generated BRK/IRQ/NMI entry cutpoints and public native-state replay fixtures
provides:
  - Actual libretro module replay from every BRK/IRQ/NMI entry stage at runahead depths one through three
  - Adapter controls for stale, skipped, omitted-history, and wrong-origin restores
  - Native state-runahead parity for the same entry cutpoint matrix
affects: [INT-02, KEEP-01, libretro, hosted-acceptance]
actuals:
  tokens: 9664
  tasks: 2
  commits: 4
commits: 4
plan_head_before: bbf90bb63213ade54ffa045cb6dd8ae668e2acd2
plan_head_after: ee34b3dd165823b4587c8c4e4f4ad15737cdddae
tech-stack:
  added: []
  patterns:
    - Load production-generated native cutpoints through the dynamically loaded adapter's public unserialize entrypoint
    - Compare committed and speculative frame, audio, state, timing, and guest-marker outputs against native continuation
key-files:
  created: []
  modified:
    - tests/libretro/libretro_host.c
    - tests/runahead_model.h
    - tests/core/test_state_runahead.c
key-decisions:
  - "Keep internal cutpoint origin labels separate from ordinary public frames, which provide no exact mid-entry stop API."
  - "Reuse the already registered libretro.runahead target; tests/CMakeLists.txt and AccuracyCoin build-input metadata did not need changes."
requirements-completed: []
coverage:
  - id: D1
    description: "Actual dynamically loaded libretro restores all 20 BRK/IRQ/NMI entry cutpoints and compares depths one through three under both existing input patterns."
    requirement: INT-02
    verification:
      - kind: integration
        ref: "libretro.runahead: entry_cutpoints=20, entry_depth_cases=60, entry_script_cases=120"
        status: pass
      - kind: integration
        ref: "cmake --workflow --preset ci (693/693 tests)"
        status: pass
      - kind: integration
        ref: "cmake --workflow --preset asan (684/684 tests)"
        status: pass
    human_judgment: true
    rationale: "INT-02 remains pending phase-wide hosted and all-lane acceptance."
  - id: D2
    description: "Stale, skipped-unserialize, omitted-entry-history, and wrong-origin states reach their intended actual-adapter comparison verdicts; native parity and existing boundary, JAM, and multi-board coverage remain passing."
    requirement: KEEP-01
    verification:
      - kind: integration
        ref: "libretro.runahead: four entry controls tripped with expected state/marker mismatch masks"
        status: pass
      - kind: integration
        ref: "core.state_runahead and core.state_fork.*"
        status: pass
      - kind: integration
        ref: "cmake --workflow --preset ci (693/693 tests)"
        status: pass
    human_judgment: true
    rationale: "KEEP-01 remains pending confirmation across hosted CI lanes."
duration: 24m
completed: 2026-10-29
status: complete
---

# Phase 21 Plan 07: Actual libretro entry replay summary

**The loaded libretro module now restores every native BRK, IRQ, and NMI entry cutpoint and proves run-ahead equivalence with four discriminating adapter controls.**

## Performance

- **Duration:** About 24 minutes.
- **Started:** 2026-10-29T13:07:00Z.
- **Completed:** 2026-10-29T13:30:38Z.
- **Tasks:** 2/2.
- **Files modified:** 3 task deliverables.

## Accomplishments

- Exercised all 20 entry origin/stage classes through the dynamically loaded production module's actual `retro_unserialize` and `retro_run` path. Each class ran at depths 1–3 under both existing input patterns, for 60 class/depth cases and 120 pattern/depth cases.
- Compared compatible committed and speculative native states, ticks, frame metadata, converted XRGB frames, valid stereo PCM, and guest RAM markers against native continuation. The source histories retained nonzero synthesis history.
- Added four compatible-prerequisite controls that reached the adapter comparison verdict: stale state (`0x0E`), skipped unserialize (`0x1E`), omitted entry history (`0x1C`), and wrong origin (`0x0E`). The wrong-origin case shares the selected vector while differing in origin, stack, next action, and consumed NMI edge.
- Added matching native `core.state_runahead` parity. Existing ordinary boundary rewind/run-ahead, JAM image/silence, and multi-board state-fork cases remain in the passing suite.
- Kept constructed origin/stage labels distinct from ordinary public frame observations. No runtime callback, state format, public API, or CMake registration change was needed.

## Verification

- Final focused set (`libretro.runahead`, `core.interrupt_state`, `core.state_runahead`, `core.state_fork.*`): **14/14 passed**.
- Verbose `libretro.runahead` confirmed all four entry controls reached their intended state/marker verdict masks and reported 20 cutpoints, 60 depth cases, and 120 script cases.
- `accuracycoin.activation` and its campaign fixture: **2/2 passed**. The AccuracyCoin build-input fingerprint remained valid; report metadata and campaign counts were not changed.
- `cmake --workflow --preset asan`: **684/684 tests passed**.
- `cmake --workflow --preset ci`: **693/693 tests passed**; library, runner, and libretro packages were generated.
- `git diff --check` passed. No timeout was changed.

## Task Commits

Task 1 used a RED/GREEN pair: `92195a7` (`test(21-07): require adapter entry cutpoint coverage`) and `9e9512d` (`feat(21-07): replay interrupt entry cuts through libretro`).

Task 2 used a RED/GREEN pair: `6a22bea` (`test(21-07): require adapter control verdicts`) and `ee34b3d` (`feat(21-07): reject incomplete adapter entry restores`).

## Files Created/Modified

- `tests/libretro/libretro_host.c` — actual module replay matrix, complete output comparisons, and compatible adapter controls.
- `tests/runahead_model.h` — owned cutpoint ROM/state helpers and native committed/speculative comparison support.
- `tests/core/test_state_runahead.c` — native continuation parity for the 20 entry cutpoint classes.

## Decisions Made

- Kept constructed cutpoint labels separate from public frame calls, which cannot stop at a precise internal entry action.
- Reused the existing `libretro.runahead` registration, so neither `tests/CMakeLists.txt` nor its AccuracyCoin build-input fingerprint changed.
- Kept INT-02 and KEEP-01 pending for hosted and every-lane phase acceptance.

## Deviations from Plan

None. The plan's existing host target already provided the required dynamic-module test path, so no CMake edit or metadata refresh was needed.

## TDD Gate Compliance

- **Task 1 RED:** `libretro.runahead` failed the planned coverage assertions: expected 20 entry classes and 60 class/depth cases, observed zero for both. The failure was in the intended matrix assertions; the existing host controls continued to run. GREEN added the actual module replay matrix and passed the target.
- **Task 2 RED:** `libretro.runahead` failed the planned control assertion: expected four entry controls to trip, observed zero. The positive entry matrix and existing controls passed. GREEN added the four compatible-state controls and passed with the intended mismatch masks.
- Both RED commits precede their corresponding GREEN commits. No refactor was needed.

## User Setup Required

None.

## Next Phase Readiness

Plan 21-07 is locally complete. The actual host and native replay evidence is ready for the remaining Phase 21 acceptance work. Hosted evidence and all three phase requirements remain pending; no remote workflow or later lifecycle step was started.

## Self-Check: PASSED

- Summary file exists at the expected plan path.
- All four task commits are ancestors of the current plan head.
- Only the pre-existing `.planning/config.json` edit remains outside this plan's summary artifact.

---
*Phase: 21-interrupt-latency-and-hijack*
*Completed: 2026-10-29*
