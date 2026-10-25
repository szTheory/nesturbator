---
phase: 18-execution-and-lifecycle-robustness
plan: "01"
subsystem: testing
tags: [C, NROM, fuzzing, LLVM-coverage, ASan, UBSan]

requires:
  - phase: 17-complete-pinned-conformance-baseline
    provides: deterministic public core APIs and the pinned AccuracyCoin baseline
provides:
  - A bounded generated-NROM execution recipe with real frame input and state continuation replay.
  - A matched initial coverage, work, stop-status, timing, and RSS comparison against both existing fuzz targets.
affects: [18-02 six-family execution qualification, 18-04 measured fuzz budgets]

actuals:
  tokens: 19453
  tasks: 2
  commits: 2

tech-stack:
  added: []
  patterns:
    - Versioned compact descriptors generate synthetic cartridge images in memory.
    - Optional replay-only counters keep normal CI and libFuzzer harness paths unchanged.

key-files:
  created:
    - tests/fuzz/execution.c
    - tests/fuzz/corpus_execution/nrom-deep
    - tests/cmake/execution_measure.cmake
    - tests/fuzz/execution-measurements.md
  modified:
    - tests/CMakeLists.txt
    - tests/cmake/fuzz_registration.cmake
    - tests/fuzz/rom_loader.c
    - tests/fuzz/state_loader.c
    - tests/fuzz/replay_main.h
    - README.md
    - include/nesturbator.h
    - tests/accuracy/baseline-metadata.txt

key-decisions:
  - "Keep loader and state-loader corpus identities separate; execution receives its own NEXE recipe identity."
  - "Use one finite matched Clang/ASan/UBSan/LLVM-coverage replay pass and report the measured 20-function delta without selecting final campaign budgets."
  - "Use test-side getrusage for peak RSS after macOS /usr/bin/time -l attempted a sandbox-denied sysctl query."

requirements-completed: []

coverage:
  - id: D1
    description: "A generated NROM recipe runs real frames, controller input, save/load continuation, and portable replay under target-local caps."
    verification:
      - kind: integration
        ref: "ctest --test-dir build/ci -R '^fuzz[.]'"
        status: pass
      - kind: integration
        ref: "cmake --workflow --preset ci (667/667 tests and package step)"
        status: pass
    human_judgment: false
  - id: D2
    description: "The initial execution slice has measured added core reach and matched work/resource data."
    verification:
      - kind: other
        ref: "cmake -DSOURCE_DIR=. -DOUTPUT_DIR=build/execution-measure -DMODE=replay -P tests/cmake/execution_measure.cmake"
        status: pass
    human_judgment: false

plan_head_before: 1f6870d094db1aa986d3be8c8cf82c8efc075385
plan_head_after: 22b890647bf43ac1bc49f998a023932d853b7bcb
duration: 59min
completed: 2026-10-24
status: complete
---

# Phase 18 Plan 01: Owned execution tracer and initial reach measurement Summary

**A 16-byte NEXE descriptor now generates an in-memory NROM program that runs controller-driven frames, restores a saved continuation, and adds 20 measured core functions beyond the loader and state-loader corpora.**

## Performance

- **Duration:** 59 minutes
- **Started:** 2026-10-24T16:45:42Z
- **Completed:** 2026-10-24T17:44:45Z
- **Tasks:** 2
- **Files modified:** 12

## Accomplishments

- Added the `fuzz.execution` replay/libFuzzer callback, a descriptor-only NROM seed, ordinary CTest regression/controls, and README/public-header documentation. The target caps input at 16 bytes, generated image at 24,592 bytes, serialized state at 256 KiB, frames at six calls, API operations at 34, and tracked allocations at 2 MiB per input.
- The valid seed completed three non-JAM frames and compared the saved continuation in two compatible instances. JAM control compares stop status and canonical state without reading partial frame output. Empty, truncated, unknown-version, cap-boundary, missing-seed, and intentional oracle-failure paths are covered.
- Ran one matched Clang 21 / ASan / UBSan / LLVM-coverage replay pass over each target's own corpus. The generated recipe added 20 `src/` function sites, chiefly PPU and APU paths. Full corpus hashes, toolchain, timing, accepted input counts, frame statuses, guest ticks, allocations, and peak RSS are recorded in [execution-measurements.md](../../../tests/fuzz/execution-measurements.md).
- Kept ROB-01, ROB-02, and ROB-03 pending. This is initial NROM reach evidence only; six-family qualification and final mutation budgets remain for later plans.

## Task Commits

1. **Task 1: Run one generated NROM recipe through real execution, continuation and portable replay** - `1d5c9db` (`feat`)
2. **Task 2: Measure the initial execution slice against both existing fuzz targets** - `22b8906` (`test`)

## Files Created/Modified

- `tests/fuzz/execution.c` - bounded recipe decode/generation, replay, output/state oracle, boundary tests, and measured work counters.
- `tests/fuzz/corpus_execution/nrom-deep` - compact versioned recipe; no ROM image is stored.
- `tests/cmake/execution_measure.cmake` - matched instrumented replay, corpus/source hashes, RSS/timing, LLVM exports, and core-function delta.
- `tests/fuzz/execution-measurements.md` - actual counts, toolchain, corpus identities, added functions, artifact hashes, and limitations.
- `tests/fuzz/rom_loader.c`, `tests/fuzz/state_loader.c`, `tests/fuzz/replay_main.h` - optional test-only counters for accepted inputs, API calls, frames, stop statuses, and guest ticks.
- `tests/CMakeLists.txt`, `tests/cmake/fuzz_registration.cmake` - target, CTest, corpus, and optional measurement registration.
- `README.md`, `include/nesturbator.h` - bounded target and JAM output-validity contracts.
- `tests/accuracy/baseline-metadata.txt` - build-input fingerprint refreshed from fresh campaign output; the canonical 146-row report SHA-256 remained unchanged.

## Verification

- `cmake -DSOURCE_DIR=. -DOUTPUT_DIR=build/execution-measure -DMODE=replay -P tests/cmake/execution_measure.cmake` - passed. It measured 60/11 accepted ROM-loader inputs, 17/12 accepted state-loader inputs, and 1/1 accepted execution inputs; execution reached 20 `src/` functions not reached by either existing corpus.
- `ctest --test-dir build/ci -R '^fuzz[.]' --output-on-failure` - 8/8 passed.
- `ctest --test-dir build/ci -R '^accuracycoin[.]activation$' --output-on-failure` - the fresh campaign and activation checks passed; its report retained all 146 scored rows and the canonical report hash.
- `cmake --workflow --preset ci` - all 667 tests passed and the library, runner, and libretro packages were generated (393 seconds).
- `git diff --check` and the normal hygiene commit hook passed.

## Decisions Made

- Kept the ROM-loader, state-loader, and execution corpora independently identified and replayed only compatible seeds for each target.
- Counted only source functions under `src/` for added reach; harness and loader functions are excluded. The measured 20-function gain is PPU/APU plus RAM-marker and synthesis work.
- Used one finite replay pass as exploratory evidence. Its timing and throughput do not establish final fuzz campaign budgets.

## Deviations from Plan

### Auto-fixed Issues and Measurement Adaptations

**1. [Rule 3 - Blocking] Refreshed AccuracyCoin input provenance after planned build-input changes**
- **Found during:** Task 1 verification, repeated after Task 2 CMake changes
- **Issue:** `accuracycoin.activation` correctly rejected the old selected-input fingerprint after the public header and test CMake inputs changed.
- **Fix:** Re-ran the full campaign and copied its current source revision and 56-input fingerprint into the metadata. The actual report SHA-256 and all canonical scores stayed unchanged.
- **Files modified:** `tests/accuracy/baseline-metadata.txt`
- **Verification:** `accuracycoin.campaign` plus `accuracycoin.activation`, then the full CI workflow.
- **Committed in:** `1d5c9db` and `22b8906`.

**2. [Rule 3 - Blocking] Adjusted descriptor field encoding for repository hygiene**
- **Found during:** Task 1 commit hook
- **Issue:** The hygiene hook rejects NUL-containing files outside the test-ROM manifest, and the initial binary descriptor seed contained NULs.
- **Fix:** Encoded the program variant and 16-bit frame counts with nonzero fixed-width biases. The recipe remains 16 bytes and decodes the same little-endian values; the seed is still a descriptor and contains no ROM image.
- **Files modified:** `tests/fuzz/execution.c`, `tests/fuzz/corpus_execution/nrom-deep`
- **Verification:** All four `fuzz.execution.*` tests, hygiene hook, and full CI.
- **Committed in:** `1d5c9db`.

**3. Added optional test-only counters to existing fuzz replay harnesses**
- **Found during:** Task 2 implementation
- **Issue:** Coverage exports alone do not report accepted inputs, lifecycle calls, per-frame stop causes, or guest ticks for the existing targets.
- **Fix:** Added counters behind `NESTURBATOR_FUZZ_METRICS=ON`; normal CI and libFuzzer target paths do not enable them. No public API or core behavior changed.
- **Files modified:** `tests/fuzz/rom_loader.c`, `tests/fuzz/state_loader.c`, `tests/fuzz/replay_main.h`, `tests/fuzz/execution.c`, `tests/CMakeLists.txt`.
- **Verification:** The matched measurement driver and ordinary fuzz CTests.
- **Committed in:** `22b8906`.

**4. Replaced `/usr/bin/time -l` peak-memory collection after sandbox denial**
- **Found during:** Task 2 measurement
- **Issue:** macOS `/usr/bin/time -l` failed its `sysctl kern.clockrate` query with `Operation not permitted`.
- **Fix:** Used `/usr/bin/time -p` for elapsed wall time and test-side `getrusage(RUSAGE_SELF).ru_maxrss` for peak RSS, with macOS/Linux byte normalization.
- **Files modified:** `tests/cmake/execution_measure.cmake`, `tests/fuzz/execution.c`, `tests/fuzz/rom_loader.c`, `tests/fuzz/state_loader.c`.
- **Verification:** The final measurement driver completed and reported nonzero peak RSS for all three processes.
- **Committed in:** `22b8906`.

**Total deviations:** 4 documented adjustments (two blocking fixes and two measurement adaptations).
**Impact on plan:** Each adjustment was needed to pass repository checks or produce the plan's required real measurement evidence. No production API, core emulator behavior, or campaign scope changed.

## Issues Encountered

- The sandbox initially denied Git index writes. The required per-task commits then completed with write escalation, GitHub noreply identity, and normal hooks.
- The local macOS toolchain had LLVM coverage tools through `xcrun` but no libFuzzer runtime. No runtime was installed; mutation measurement remains assigned to the existing Linux toolchain in later plans.

## User Setup Required

None - no external service configuration is required.

## Next Phase Readiness

Plan 18-02 can use the measured prototype and corpus identities to assess the six required execution families and lifecycle controls. ROB-01/02/03 remain pending until their planned evidence and independent phase verification are complete.

---
*Phase: 18-execution-and-lifecycle-robustness*
*Completed: 2026-10-24*

## Self-Check: PASSED

- All implementation, measurement, documentation, and summary files listed above exist.
- Task commits `1d5c9db` and `22b8906` are ancestors of the current plan head.
- `git diff --check` passed.
