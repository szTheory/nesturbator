---
phase: 01-a-test-frame-in-retroarch
plan: 01
subsystem: core, runner, build
tags: [c17, cmake, presets, ctest, sha256, test-card, public-api]

requires: []
provides:
  - "CMake project with dev, ci and ci-msvc presets (configure, build, test, workflow); test presets set noTestsAction error"
  - "include/nesturbator.h: D-07 conventions, version/ABI macros, status enum, structs, five Phase 1 functions"
  - "Core library nesturbator: size-tag helper, create/destroy/get_version/get_info, run_frame with D-02 test card and D-09 silence"
  - "nesturbator-run with --frames and --hash-frame; owned FIPS 180-4 SHA-256"
  - "tests/cmake/expect_output.cmake and runner.* contract tests"
affects: [01-02, 01-03, 01-04, 01-05, 01-06, 01-07]

actuals:
  tokens: 9396
  tasks: 2
  commits: 2
plan_head_before: addf3a101d597e960c5a0292c534eca5654111d1
plan_head_after: 7b9efc48a7cc0b46814acce87ba00199e1ef2665

tech-stack:
  added: [CMake presets schema 6, Ninja, CTest]
  patterns:
    - "Size-tag check through one helper nesturbator__check_size_in before any field is read"
    - "Output structs built as a full local copy, then min(size, sizeof) bytes copied out"
    - "Runner tests run through cmake -P expect_output.cmake with exact stdout and exit status"
    - "Internal core symbols use the nesturbator__ prefix; runner symbols nesturbator_run_"

key-files:
  created:
    - CMakeLists.txt
    - CMakePresets.json
    - version.txt
    - include/nesturbator.h
    - src/internal.h
    - src/instance.c
    - src/frame.c
    - src/testcard.c
    - runner/CMakeLists.txt
    - runner/main.c
    - runner/sha256.c
    - runner/sha256.h
    - tests/CMakeLists.txt
    - tests/cmake/expect_output.cmake
  modified:
    - README.md

key-decisions:
  - "expect_output.cmake takes EXPECT as a list of lines, each ending in a newline; an empty EXPECT means empty stdout. Quoted add_test arguments keep their semicolons, so no escaping is needed."
  - "The runner prints one line per requested frame even if --hash-frame repeats the same N; lines come out in frame order."
  - "Runner numbers accept decimal digits only, 1 to 4294967295; anything else is a usage error (exit 2)."
  - "An allocator with alloc and free NULL but user set is treated as partly NULL (ERR_ARGUMENT)."

patterns-established:
  - "nesturbator_warnings(target) applies the ENGINEERING section 1 warning set to every C target"
  - "nesturbator_runner_test(NAME ARGS EXIT [IGNORE_STDOUT] [EXPECT lines]) registers a runner test"

requirements-completed: [FRAME-01, FRAME-03]

coverage:
  - id: D1
    description: "cmake --workflow --preset ci configures, builds the library and runner with warnings as errors, and runs every test with zero failures"
    requirement: FRAME-01
    verification:
      - kind: integration
        ref: "cmake --workflow --preset ci (7/7 passed)"
        status: pass
    human_judgment: false
  - id: D2
    description: "nesturbator-run --frames 1 --hash-frame 1 prints the FRAME-03 test-card hash equal to the independent model"
    requirement: FRAME-03
    verification:
      - kind: integration
        ref: "tests/CMakeLists.txt#runner.hash"
        status: pass
    human_judgment: false
  - id: D3
    description: "Frame advance shown by metadata (D-04), empty output without --hash-frame, usage errors exit 2"
    requirement: FRAME-03
    verification:
      - kind: integration
        ref: "tests/CMakeLists.txt#runner.advance"
        status: pass
      - kind: integration
        ref: "tests/CMakeLists.txt#runner.nohash"
        status: pass
      - kind: integration
        ref: "tests/CMakeLists.txt#runner.usage.beyond, runner.usage.zero, runner.usage.unknown, runner.usage.positional"
        status: pass
    human_judgment: false
  - id: D4
    description: "Public header carries the D-07 conventions and contract comments for the Phase 1 functions"
    verification:
      - kind: other
        ref: "grep -c x-release-please- include/nesturbator.h = 3; status enum grep = 6 lines"
        status: pass
    human_judgment: true
    rationale: "Comment quality and convention wording are judged by a reader; header-alone compile tests arrive in plan 02"

duration: 4min
completed: 2026-10-02
status: complete
---

# Phase 1 Plan 01: Header, core test card and runner hash Summary

**C17 core library with the D-07 public header, a coordinate-computed D-02 test card, and `nesturbator-run` whose owned SHA-256 prints the independently modelled hash `b49e9be4…0453` under `cmake --workflow --preset ci`**

## Performance

- **Duration:** about 4 min
- **Started:** 2026-10-02T20:47:46Z
- **Completed:** 2026-10-02T20:51:36Z
- **Tasks:** 2
- **Files modified:** 15 (14 created, 1 modified)

## Accomplishments

- One command (`cmake --workflow --preset ci`) configures, builds with warnings as errors and runs 7 tests, all passing; the `dev` lane passes too.
- The C test card hashes to the value of the independent Python model on the first run. The C code and the model read D-02 the same way.
- The public header fixes the conventions every host copies: prefix, size tags, ABI number 1, status values 0 to 5, allocator signature, `NESTURBATOR_CONFIG_INIT` with every member listed.
- `run_frame` runs every check before it changes state, and fills 798 or 799 silent samples with the remainder carried over.
- The core release library has no mutable data symbols (`nm` shows no B/b/D/d/C).
- The runner contract (output line, frame advance, usage exits, empty output) is pinned by tests and written up in the README.

## Task Commits

1. **Task 1: Runner prints the test-card hash end to end through header, core, runner, CTest and the ci preset** - `dfeb96e` (feat)
2. **Task 2: Runner contract tests and README build and runner sections** - `7b9efc4` (test)

## Files Created/Modified

- `CMakeLists.txt`: project from `version.txt`, C17 without extensions, `nesturbator_warnings()`, static core target with hidden visibility, PIC, VERSION and SOVERSION 1
- `CMakePresets.json`: `dev`, `ci`, `ci-msvc` configure, build, test and workflow presets
- `version.txt`: `0.0.0`
- `include/nesturbator.h`: the public header
- `src/internal.h`: instance struct, tick and audio constants, first-released sizes, internal prototypes
- `src/instance.c`: size-tag helper, version and info, create with default malloc/free pair, destroy
- `src/frame.c`: `nesturbator_run_frame`
- `src/testcard.c`: `nesturbator__test_pixel` per D-02
- `runner/main.c`, `runner/CMakeLists.txt`: `nesturbator-run`
- `runner/sha256.c`, `runner/sha256.h`: FIPS 180-4 SHA-256 with a hex helper
- `tests/CMakeLists.txt`: `nesturbator_runner_test()` and the 7 `runner.*` tests
- `tests/cmake/expect_output.cmake`: exact stdout and exit-status check, `IGNORE_STDOUT` option
- `README.md`: status, building, runner options, output line, hash definition, exit statuses

## Decisions Made

- `EXPECT` is a list of output lines in `expect_output.cmake`. A quoted `add_test` argument keeps its semicolons, measured: the first attempt escaped them and the backslashes reached the script.
- Repeated `--hash-frame N` prints frame N once. Lines come out in frame order.
- Runner numbers are decimal digits only, 1 to 4294967295.
- An allocator with only `user` set counts as partly NULL (`ERR_ARGUMENT`); the header comment says so.

## Deviations from Plan

None. The plan ran as written. The Task 1 `add_test` escaping was a first-attempt mistake, fixed before the commit, so it is not a deviation.

## Issues Encountered

None.

## User Setup Required

None. No external service configuration required.

## Next Phase Readiness

- Ready for plan 02: D-11 core tests, header-alone C and C++ compiles, SHA-256 known-answer test.
- GCC-only warnings (`-Wduplicated-cond -Wlogical-op`) and the `ci-msvc` lane have not run yet. There is no GCC or MSVC on this Mac; they first run in CI (plan 10 onward).

## Self-Check: PASSED

- All 14 created files and README.md are on disk.
- Commits `dfeb96e` and `7b9efc4` are on `phase/01-test-frame`.
- `cmake --workflow --preset ci`: 100% tests passed out of 7. The runner hash line matches exactly.

---
*Phase: 01-a-test-frame-in-retroarch*
*Completed: 2026-10-02*
