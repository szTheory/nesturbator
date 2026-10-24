---
phase: 18-execution-and-lifecycle-robustness
plan: "03"
subsystem: testing
tags: [C, NEXE, libretro, JAM, save-states, deterministic-replay]

requires:
  - phase: 18-execution-and-lifecycle-robustness
    provides: Plan 18-02 six-family generated profiles and continuation/lifecycle controls
provides:
  - Generated loop and JAM cases distinguish useful guest progress from native nonprogress and a latched halt.
  - The libretro adapter repeats only its last complete owned XRGB frame, or initialized black, on JAM and emits no audio.
  - Portable host checks cover cache invalidation, state refusal/preservation, recovery, reset, and reload behavior.
affects: [Phase 18 Plan 04 replay and campaign evidence, ROB-02, ROB-04, libretro]

actuals:
  tokens: 8725
  tasks: 2
  commits: 2

tech-stack:
  added: []
  patterns:
    - Progress qualification uses advancing frame and tick counters; repeated PC or pixels may be valid guest looping.
    - Adapter presentation is session-local state, invalidated at successful lifecycle transitions and excluded from native serialization.
    - Generated NEXE recipes stay compact, NUL-free descriptors while guest images remain in memory.

key-files:
  created:
    - tests/fuzz/corpus_execution/loop
    - tests/fuzz/corpus_execution/jam
  modified:
    - tests/fuzz/execution.c
    - tests/fuzz/test_execution.c
    - tests/core/test_frame.c
    - tests/runner/test_movie.c
    - tests/CMakeLists.txt
    - libretro/libretro.c
    - tests/libretro/libretro_host.c
    - README.md
    - include/nesturbator.h
    - .planning/preparation/LIBRETRO-AND-RUNNER.md
    - tests/accuracy/baseline-metadata.txt

key-decisions:
  - "Keep native STOP_JAM latched and preserve its rule that native output buffers are undefined on the stop."
  - "Keep cached presentation outside the native instance and save-state; repeat a complete converted frame or black, never stale audio or partially written native pixels."
  - "Keep ROB-02 and ROB-04 pending for final Phase 18 evidence and verification."

patterns-established:
  - "A regular guest loop is progress when frame and tick counters advance, even when its PC and pixels repeat."
  - "Only a successful native frame refreshes the adapter's XRGB cache; successful state changes invalidate it, while a refused state load preserves it."

requirements-completed: []

coverage:
  - id: D1
    description: "Generated looping and JAM guests prove native frame progress, latched-stop behavior, saved-state restoration, reset, re-entry, and runner diagnostics/save handling."
    requirement: ROB-04
    verification:
      - kind: integration
        ref: "ctest --preset ci -R '^(core.frame|runner.movie|runner.save.jam|fuzz.execution.(seeds|regress|controls))$' --no-tests=error (7/7 passed)"
        status: pass
      - kind: integration
        ref: "cmake --workflow --preset ci (667/667 tests passed)"
        status: pass
    human_judgment: false
  - id: D2
    description: "The libretro adapter polls and sends exactly one complete video callback on each JAM, with correct cache transitions and no audio callbacks."
    requirement: ROB-04
    verification:
      - kind: integration
        ref: "ctest --preset ci -R '^libretro.host$' --output-on-failure --no-tests=error (2/2 including fixture passed)"
        status: pass
      - kind: integration
        ref: "cmake --workflow --preset ci (libretro.host, serialize, parity, and full suite passed)"
        status: pass
    human_judgment: false
  - id: D3
    description: "README, public-header, and preparation text describe the native halt contract and adapter presentation policy; fresh AccuracyCoin provenance preserves the canonical measured report."
    verification:
      - kind: other
        ref: "Fresh AccuracyCoin campaign: 22/22 pages; report SHA-256 6d0a811960f6964e7fc66802c6e4ade0c4ffb8772c40d88d50cf79e1f8463652; canonical counts unchanged."
        status: pass
      - kind: integration
        ref: "cmake --workflow --preset ci (all 3 packages generated)"
        status: pass
    human_judgment: true
    rationale: "The README, public-header, and preparation wording is prose and is not semantically asserted by automated tests."

commits: 2
plan_head_before: 9b9d01e02fd8a89932c77cc84bb97efcbd0a424c
plan_head_after: d1f9d7f0497aef8dddeaedf64729c84fde2c9d06
duration: "20min"
completed: 2026-10-24
status: complete
---

# Phase 18 Plan 03: Native loop, JAM, and libretro presentation Summary

**Generated loop and JAM fixtures now prove native progress and recovery, while libretro repeats only an owned complete XRGB frame on JAM.**

## Performance

- **Duration:** 20 minutes (started 2026-10-24T19:03:30Z; completed 2026-10-24T19:23:49Z)
- **Tasks:** 2
- **Files modified:** 13

## Accomplishments

- Added NUL-free 16-byte `loop` and `jam` NEXE descriptors. Native/core tests prove that a repeating PC and image can still advance ticks and frames; the no-progress control is rejected. JAM checks cover the first partial stop, repeated latched calls, saved halted and pre-halt states, reset, deliberate re-entry, runner failure diagnostics, and existing save behavior.
- Added adapter-local cache validity and invalidation. Each loaded-session `retro_run` polls input; JAM sends one non-NULL complete XRGB frame from the cache or initialized black, and sends neither audio mode. Successful load, reset, unload and state load invalidate the cache; refused state loads retain it. Native state and progress remain unchanged.
- Expanded portable host assertions for exact poll/video cadence, complete-frame contents, first/repeated JAM, a visibly nonblack prior frame, both audio callback modes, successful and refused state loads, reset, recovery, and unload/reload. README, public-header, and preparation documentation now distinguish native output validity from adapter presentation.
- Refreshed AccuracyCoin provenance from a fresh full campaign after the build-input and public-header changes. The measured campaign completed 22/22 pages and 146 scored rows (101 pass, 42 fail, 0 skip, 2 not-run, 1 timeout); the canonical report SHA and counts remained unchanged. The new 56-input fingerprint is `a0f7f3fda8b51460d617277d62f608378b526160acae9968e4ca34595f8029c5`.
- `cmake --workflow --preset ci` passed all 667 tests in 379.30 seconds and generated the library, runner, and libretro packages. `retroarch.session.controls` passed in 276.81 seconds.

## Task Commits

Each task was committed atomically:

1. **Task 1: Distinguish guest-loop progress from latched JAM at native and runner boundaries** — `7c6a274` (`feat`)
2. **Task 2: Present one owned complete frame on JAM and enforce cache lifetime** — `d1f9d7f` (`feat`)

## Files Created/Modified

- `tests/fuzz/corpus_execution/loop`, `tests/fuzz/corpus_execution/jam` — generated ordinary-loop and controlled-halt recipes.
- `tests/fuzz/execution.c`, `tests/fuzz/test_execution.c` — bounded progress qualification, JAM recovery, and a suppressed-progress negative control.
- `tests/core/test_frame.c`, `tests/runner/test_movie.c`, `tests/CMakeLists.txt` — public-core loop, runner loop, and external CTest deadline coverage.
- `libretro/libretro.c`, `tests/libretro/libretro_host.c` — owned presentation cache policy and portable callback/state-transition assertions.
- `README.md`, `include/nesturbator.h`, `.planning/preparation/LIBRETRO-AND-RUNNER.md` — native-versus-adapter halt contract.
- `tests/accuracy/baseline-metadata.txt` — measured campaign identity, build-input fingerprint, elapsed time, and artifact path.

## Decisions Made

- Progress is shown by monotonically advancing native frame/tick counters, not by requiring changing PC or pixels.
- The adapter cache remains out of native state and serialization; JAM does not trigger reset, unload, frontend termination, or audio callbacks.
- Plan evidence does not close phase-level requirements. ROB-02 and ROB-04 remain pending until final Phase 18 verification.

## Deviations from Plan

### Necessary test coverage

**1. [Rule 2 - Missing critical negative control] Added the suppressed-progress control in `tests/fuzz/test_execution.c`.**
- **Found during:** Task 1
- **Issue:** The plan required proving the production verdict rejects a no-progress observation, but its declared task file list omitted the controls source file.
- **Fix:** Added a control that zeroes the observed guest progress and asserts verdict rejection.
- **Files modified:** `tests/fuzz/test_execution.c`
- **Verification:** Focused `fuzz.execution.controls` and full CI passed.
- **Committed in:** `7c6a274`

**2. [Rule 2 - Missing critical enclosure] Added a 30-second CTest timeout to `core.frame`.**
- **Found during:** Task 1
- **Issue:** The new core loop regression needed an external nonprogress deadline, and the existing `core.frame` registration had no explicit timeout.
- **Fix:** Set a CTest-only timeout; no native instruction or wall-clock kill policy was added.
- **Files modified:** `tests/CMakeLists.txt`
- **Verification:** `core.frame` passed in focused checks and full CI.
- **Committed in:** `7c6a274`

### Required measurement maintenance

**3. Refreshed the AccuracyCoin baseline metadata after source/build-input changes.**
- **Found during:** Task 2 closeout
- **Issue:** The public-header and test registration changes changed the authenticated build-input fingerprint; retaining the prior fingerprint would invalidate measured provenance.
- **Fix:** Ran a fresh 22-page campaign and updated source revision, dirty state, fingerprint, elapsed time, artifact path, and reason. Canonical report hash and scored counts were preserved.
- **Files modified:** `tests/accuracy/baseline-metadata.txt`
- **Verification:** Fresh `accuracycoin.campaign` passed; campaign report SHA-256 matched the canonical report; full workflow activation passed.
- **Committed in:** `d1f9d7f`

**Total deviations:** 3 (two missing-test/enclosure corrections and one required measurement refresh).
**Impact on plan:** All additions support the locked acceptance contracts and truthful baseline provenance; no runtime dependency, public API, or native halt behavior changed.

## Issues Encountered

- The first portable-host fixture attempt used a reference image whose reset vectors had been changed by an earlier helper. Capturing the original generated image after vector setup resolved the fixture issue; the focused host test then passed.

## Known Stubs

None found in the files changed by this plan.

## User Setup Required

None — no external service configuration is required.

## Next Phase Readiness

- Plan 18-04 can reuse the generated loop/JAM recipes, the native continuation evidence, and the adapter cache contract.
- ROB-02 and ROB-04 remain unchecked pending final Phase 18 evidence and verification. The preexisting `.planning/config.json` edit remains unstaged.

## Self-Check: PASSED

- Summary and both generated recipe descriptors exist.
- Task commits `7c6a274` and `d1f9d7f` are ancestors of the current HEAD.
- The summary passes `git diff --check`.

---
*Phase: 18-execution-and-lifecycle-robustness*
*Completed: 2026-10-24*
