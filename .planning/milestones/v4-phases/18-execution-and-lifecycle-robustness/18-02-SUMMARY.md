---
phase: 18-execution-and-lifecycle-robustness
plan: "02"
subsystem: testing
tags: [C, NEXE, mappers, fuzzing, LLVM-coverage, ASan, UBSan, lifecycle]

requires:
  - phase: 18-execution-and-lifecycle-robustness
    provides: Plan 18-01 bounded NROM execution tracer and matched replay driver
provides:
  - Six in-memory mapper recipes execute register-sensitive guest work and qualify through the public cartridge loader.
  - Actual-verdict controls reject suppressed execution, skipped restore, wrong origin, and shared-instance interference.
  - Six-family lifecycle checks cover reset, refused loads, same-state loads, unload/reload, and battery generation.
  - Matched replay evidence records useful six-family reach, resource use, corpus identities, and limits.
affects: [Phase 18 Plan 03 loop/JAM boundaries, Phase 18 Plan 04 campaign budgets, ROB-01, ROB-02]

actuals:
  tokens: 25372
  tasks: 3
  commits: 3

tech-stack:
  added: []
  patterns:
    - Compact NEXE descriptors generate accepted mapper images in memory; no ROM images are stored.
    - The same continuation verdict is exercised by valid paths and deliberate false-agreement controls.
    - Coverage and resource measurements use one matched Clang/ASan/UBSan/LLVM-coverage replay build.

key-files:
  created:
    - tests/fuzz/corpus_execution/mmc1-deep
    - tests/fuzz/corpus_execution/uxrom-deep
    - tests/fuzz/corpus_execution/cnrom-deep
    - tests/fuzz/corpus_execution/mmc3-deep
    - tests/fuzz/corpus_execution/axrom-deep
    - tests/fuzz/test_execution.c
  modified:
    - tests/fuzz/execution.c
    - tests/fuzz/execution-measurements.md
    - tests/cmake/execution_measure.cmake
    - tests/CMakeLists.txt
    - README.md
    - include/nesturbator.h
    - tests/accuracy/baseline-metadata.txt

key-decisions:
  - "Keep the deep corpus as six 16-byte descriptors and retain arbitrary trailing PRG/CHR byte mutation within each generated profile's capacity."
  - "Interleave a distinct mapper between source and restored continuation calls and hash the exact save-state origin."
  - "Keep ROB-01 and ROB-02 unchecked until final Phase 18 verification; the Plan 18-02 evidence is finite and does not claim hardware accuracy."

patterns-established:
  - "Successful frames compare canonical state, defined frame metadata, native pixels, and only valid audio samples; JAM compares status and canonical state only."
  - "Lifecycle controls alter retained state before reset, compare reload with a fresh machine, and never dereference an expired memory span."

requirements-completed: []

coverage:
  - id: D1
    description: "Six accepted representative boards run non-JAM guest work with measured mapper-specific PRG, CHR, or nametable effects."
    requirement: ROB-01
    verification:
      - kind: integration
        ref: "ctest --preset ci -R '^fuzz.execution.(seeds|regress|controls|oracle_failure)$' --no-tests=error"
        status: pass
      - kind: other
        ref: "cmake -DSOURCE_DIR=. -DOUTPUT_DIR=build/execution-measure -DMODE=replay -P tests/cmake/execution_measure.cmake"
        status: pass
    human_judgment: false
  - id: D2
    description: "The continuation verdict rejects four named false agreements and preserves six-family lifecycle contracts."
    requirement: ROB-02
    verification:
      - kind: integration
        ref: "fuzz.execution.controls: valid and negative scenarios through nt_exec_verdict"
        status: pass
      - kind: integration
        ref: "cmake --workflow --preset ci (667/667 tests and package step)"
        status: pass
    human_judgment: false

commits: 3
plan_head_before: 6b827c46d24308ad808a3b55a915c5d47f4499fb
plan_head_after: 7a72947b71be8164e96f0dc9ed447574c4eb029f
duration: 60min
completed: 2026-10-24
status: complete
---

# Phase 18 Plan 02: Six-family execution and lifecycle qualification Summary

**Six generated mapper recipes now execute bank-sensitive guest programs and pass discriminating continuation, reset, state-load, and instance-isolation checks, reaching 24 core functions beyond the loader corpora.**

## Performance

- **Duration:** 60 minutes
- **Started:** 2026-10-24T17:47:55Z
- **Completed:** 2026-10-24T18:48:02Z
- **Tasks:** 3
- **Files modified:** 13

## Accomplishments

- Added MMC1, UxROM, CNROM, MMC3, and AxROM compact seeds alongside NROM. All six representative accepted profiles complete four `NESTURBATOR_OK` frames per deep replay, record 55 public API operations, and demonstrate their required PRG, CHR, or AxROM single-screen effect. Arbitrary raw PRG/CHR mutation remains available within the active profile's generated image capacity.
- Added a different-board interleaved frame between the saved source origin and continuation. Repeated A/B/A replay preserves identical generated content, operation order, output observations, and canonical state. The origin state receives a reproducible hash.
- Added real-verdict controls for suppressed execution, skipped restore, wrong saved origin, and shared-instance interference. Valid continuation passes; each named false agreement is rejected. The existing deliberate oracle-failure test remains active.
- Qualified reset retention/current-bank vector timing, refused cartridge and state-load atomicity, same-state load idempotency, fresh unload/reload state, memory-span lifetime, and per-instance battery generation across all six families. Guarded video/audio buffers and JAM comparisons avoid partial outputs.
- The matched Release Clang 21 arm64 ASan/UBSan/LLVM-coverage replay accepted 6/6 execution descriptors and reached 24 `src/` functions not hit by either existing corpus. Aggregate execution measured 24 successful frames, 330 API operations, 17,154,528 guest ticks, 626,054 bytes peak tracked allocation, and 21,905,408 bytes peak RSS. Per-family and corpus data are in [execution-measurements.md](../../../tests/fuzz/execution-measurements.md).
- The matched replay identities are overlay SHA-256 `4a99c0f01b472192b23ccfa4bcb8bf788d6e6e59f6ea4cab0922dfbb5433a0cf`, full-corpus manifest `1a76c9de3aae9d5f999deeab93db8d8506d65cd9cb9cb7647f524f8496e15dc2`, and six-seed execution corpus `bef47c764bef3be36726ffb95f0c4b8f5b3c06a6c0f269e23b6ca2aa7cdc0f88`. Per-export SHA-256 values are in the measurement record.
- A fresh full AccuracyCoin campaign completed 22/22 pages and 146 scored rows (101 pass, 42 fail, 2 not-run, 1 timeout). The canonical report SHA-256 remained `6d0a811960f6964e7fc66802c6e4ade0c4ffb8772c40d88d50cf79e1f8463652`; the actual 56-input build fingerprint is `6868b35ca52a1d5456cf638251f1c8df480881035096988891b8571b55f058d7`.
- The final `cmake --workflow --preset ci` run passed all 667 tests and generated the library, runner, and libretro ZIP packages.

## Task Commits

Each task was committed atomically:

1. **Task 1: Execute MMC1, UxROM and CNROM register-sensitive programs** - `9864d72` (`feat`)
2. **Task 2: Execute MMC3 and AxROM and measure all six families together** - `1d7a010` (`feat`)
3. **Task 3: Reject broken continuation and instance-isolation oracles through their actual verdict** - `7a72947` (`feat`)

## Files Created/Modified

- `tests/fuzz/corpus_execution/{mmc1,uxrom,cnrom,mmc3,axrom}-deep` - compact descriptors; generated images remain in memory.
- `tests/fuzz/execution.c` - mapper programs, measured interleaving, saved-origin identity, initialized outputs, and operation/frame caps.
- `tests/fuzz/test_execution.c` - actual-verdict negative controls and lifecycle qualification for each supported family.
- `tests/CMakeLists.txt` - controls executable and CTest registration.
- `tests/cmake/execution_measure.cmake` - expected per-family frame count updated for the interleaved frame.
- `tests/fuzz/execution-measurements.md` - final six-family data, core function deltas, replay and artifact hashes.
- `README.md`, `include/nesturbator.h` - recipe family, lifecycle comparison limits, and explicit non-accuracy claim.
- `tests/accuracy/baseline-metadata.txt` - current build fingerprint and measured campaign provenance; report identity and scored counts are unchanged.

## Decisions Made

- Use representative accepted profiles to show mapper paths without claiming every admitted subprofile or maximum board size.
- Keep all ROM data generated in memory and keep the committed corpus as NUL-free 16-byte descriptors, consistent with the repository hygiene rule.
- Use the same `nt_exec_verdict` implementation in fuzz replay and deliberately broken control scenarios; do not add production API or thread-safety promises.
- Leave ROB-01 and ROB-02 unchecked in project requirements until the remaining Phase 18 plans and final phase verification provide the required evidence.

## Deviations from Plan

### Necessary measurement update

**1. Updated the matched driver for the new interleaved frame.**
- **Found during:** Task 3 verification
- **Issue:** The prior measurement assertion expected three completed frames per seed; the required different-board interleave makes the truthful total four.
- **Fix:** Updated the driver assertion and reran the full matched measurement after the final harness change.
- **Files modified:** `tests/cmake/execution_measure.cmake`, `tests/fuzz/execution-measurements.md`
- **Verification:** The final measurement passed for all six family rows and retained 24 newly reached core functions.
- **Committed in:** `7a72947`

**Total deviations:** 1 necessary measurement assertion update.
**Impact on plan:** It keeps measured frame accounting aligned with the required interleaving; no mapper family, locked criterion, or product API was removed.

## Issues Encountered

- The first full CI attempt had a transient `retroarch.session.controls` failure with `LOCK_OWNER_UNAVAILABLE` / `READINESS_LOST`. The required final workflow rerun passed that long-running control and all 667 tests; no unrelated source changes were made.
- The local matched comparison is one finite sanitizer/coverage replay over fixed corpora. It does not establish mutation-campaign throughput, cover every supported mapper subprofile, or assert hardware accuracy. Final campaign budgets and phase-level requirement completion remain for later Phase 18 work.

## User Setup Required

None - no external service configuration is required.

## Next Phase Readiness

Plan 18-03 can build on the six-family lifecycle harness to qualify ordinary guest loops, JAM boundaries, and libretro presentation behavior. Plan 18-04 still owns final replay and campaign budgets, artifacts, and scoped findings. ROB-01 and ROB-02 remain pending phase-level verification.

---
*Phase: 18-execution-and-lifecycle-robustness*
*Completed: 2026-10-24*

## Self-Check: PASSED

- All 13 plan implementation, test, measurement, and documentation files exist.
- Task commits `9864d72`, `1d7a010`, and `7a72947` are ancestors of the plan head.
- The final full CI workflow passed 667/667 tests and generated all three packages.
