---
phase: 01-a-test-frame-in-retroarch
plan: 07
subsystem: verification-lanes
tags: [asan, ubsan, nofp, abi, nm, frame-06]
status: complete

requires:
  - phase: 01-06
    provides: "install.consumer forwarding CMAKE_C_FLAGS and CMAKE_EXE_LINKER_FLAGS to the consumer build"
provides:
  - "Presets asan (configure, build, test, workflow): Debug, -fsanitize=address,undefined -fno-sanitize-recover=all, warnings fatal, label retroarch excluded, UBSAN_OPTIONS=print_stacktrace=1"
  - "Presets nofp (configure, build, test, workflow): inherits ci, NESTURBATOR_NOFP=ON, label abi only"
  - "Option NESTURBATOR_NOFP: -mgeneral-regs-only on the nesturbator target, FATAL_ERROR under MSVC, adds tests/abi"
  - "CTest label abi: abi.float_scan, abi.float_scan.selftest, abi.undefined_symbols, abi.global_symbols, abi.float_fixture"
  - "EXCLUDE_FROM_ALL target nesturbator_float_fixture"
affects: [01-08, 01-12]

actuals:
  tokens: 4400
  tasks: 2
  commits: 2
plan_head_before: a63f289020c1e8031bdea6ddd42ce0dc99509449
plan_head_after: ca36b26847aa89a8c1f861bdfc2d460765389fbd

tech-stack:
  added: []
  patterns:
    - "ABI checks are cmake -P scripts over nm output, registered as CTest tests under one label"
    - "Each check has a fixture or self-test showing it can fail"

key-files:
  created:
    - tests/abi/CMakeLists.txt
    - tests/abi/float_fixture.c
    - tests/cmake/float_scan.cmake
    - tests/cmake/undefined_symbols.cmake
    - tests/cmake/global_symbols.cmake
    - tests/cmake/float_fixture.cmake
  modified:
    - CMakePresets.json
    - CMakeLists.txt
    - README.md
    - src/internal.h

key-decisions:
  - "abi.undefined_symbols ignores names that another member of the same archive defines: nm -u on a static library lists cross-object references such as nesturbator__palette_ntsc"
  - "bzero is on the undefined-symbol allowlist on Darwin only, with a comment: Apple clang 21 lowers memset(p, 0, n) to bzero at -O2"
  - "abi.float_fixture accepts a failed fixture build only if the build output names float_fixture.c, so an unrelated build error cannot pass it"
  - "Comments in the core state numbers as integers or words (89341 and a half dots), so the float scan needs no comment exception"

patterns-established:
  - "tests/abi holds tests that exist only under NESTURBATOR_NOFP; scripts stay in tests/cmake"

requirements-completed: [FRAME-06]

coverage:
  - id: D1
    deliverable: "asan lane runs every test except label retroarch, including install.consumer, with no sanitizer report"
    human_judgment: false
    verification:
      - kind: command
        ref: "cmake --workflow --preset asan"
        status: pass
      - kind: command
        ref: "ctest --preset asan -N (lists install.consumer, no retroarch test)"
        status: pass
  - id: D2
    deliverable: "nofp lane builds the core with -mgeneral-regs-only and passes the five abi tests"
    human_judgment: false
    verification:
      - kind: command
        ref: "cmake --workflow --preset nofp"
        status: pass
  - id: D3
    deliverable: "Each abi check fails on bad input"
    human_judgment: false
    verification:
      - kind: test
        ref: "abi.float_scan.selftest"
        status: pass
      - kind: test
        ref: "abi.float_fixture (rejected for __muldf3, __floatunsidf, __fixunsdfsi)"
        status: pass
      - kind: command
        ref: "float_scan.cmake on a scratch copy of src/frame.c with a double (reported, exit 1)"
        status: pass
      - kind: command
        ref: "global_symbols.cmake on a scratch library with D, C and d symbols (reported, exit 1)"
        status: pass
      - kind: command
        ref: "undefined_symbols.cmake on a scratch library calling printf (putchar reported, exit 1)"
        status: pass
  - id: D4
    deliverable: "ci lane still green"
    human_judgment: false
    verification:
      - kind: command
        ref: "cmake --workflow --preset ci"
        status: pass

duration: 6 min
completed: 2026-10-02
---

# Phase 01 Plan 07: asan and nofp lanes Summary

**`asan` runs all 25 tests clean under AddressSanitizer and UBSan, including the installed-package consumer. `nofp` builds the core with `-mgeneral-regs-only` and runs five `abi` checks: a float text scan, an `nm` undefined-symbol allowlist, an `nm` writable-data check, and a `double` fixture that the checks reject for `__muldf3`.**

## Performance

- **Duration:** 6 min
- **Started:** 2026-10-02T21:16:11Z
- **Completed:** 2026-10-02
- **Tasks:** 2
- **Files modified:** 10

## Accomplishments

- `cmake --workflow --preset asan`: 25 of 25 tests pass with no sanitizer report. `install.consumer` links the sanitizer runtime through the flags plan 06 forwards, so it needed no change.
- `cmake --workflow --preset nofp`: `abi.float_scan`, `abi.float_scan.selftest`, `abi.undefined_symbols`, `abi.global_symbols` and `abi.float_fixture` pass. The release library needs only `free malloc memcpy bzero` plus symbols its own members define, and it defines no writable data.
- Every check was shown to fail on bad input: a scratch `double` in a copy of `src/frame.c`, a scratch library with `D`, `C` and `d` symbols, a scratch library calling `printf`, and the fixture itself.
- README lists the `asan` and `nofp` lanes and what the `abi` tests check.

## Task Commits

1. **Task 1: asan lane** - `830071f` (feat)
2. **Task 2: nofp lane with the abi checks** - `ca36b26` (feat)

## Files Created/Modified

- `CMakePresets.json` - asan and nofp configure, build, test and workflow presets
- `CMakeLists.txt` - option NESTURBATOR_NOFP, -mgeneral-regs-only on the core, tests/abi
- `tests/abi/CMakeLists.txt` - the five abi tests and the fixture target
- `tests/abi/float_fixture.c` - deliberate double multiply
- `tests/cmake/float_scan.cmake` - text scan with SELFTEST mode
- `tests/cmake/undefined_symbols.cmake` - nm -u allowlist check
- `tests/cmake/global_symbols.cmake` - writable-data check, with the no-.data.rel.ro rule in its header
- `tests/cmake/float_fixture.cmake` - passes only if the compiler or the symbol check rejects the fixture
- `src/internal.h` - frame-length comment without a decimal point
- `README.md` - asan and nofp lanes

## Decisions Made

See `key-decisions` in the frontmatter.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] The undefined-symbol check would fail on the library's own internal references**
- **Found during:** Task 2
- **Issue:** `nm -u` on a static archive lists each member's undefined symbols, including `nesturbator__check_size_in`, `nesturbator__test_pixel` and `nesturbator__palette_ntsc`, which other members define.
- **Fix:** The script also reads `nm` without `-u` and ignores names the library defines.
- **Files modified:** tests/cmake/undefined_symbols.cmake
- **Commit:** ca36b26

**2. [Rule 3 - Blocking] Apple clang emits `bzero`**
- **Found during:** Task 2
- **Issue:** The release library on the Mac references `_bzero`, because Apple clang lowers `memset(p, 0, n)` to it. The must_haves allowlist does not name it.
- **Fix:** `bzero` is added to the allowlist when APPLE is true, with a comment naming the platform and the measurement. This follows the plan's own rule for toolchain helpers: one named symbol per platform, never a pattern. ELF keeps the exact must_haves list.
- **Files modified:** tests/cmake/undefined_symbols.cmake, README.md
- **Commit:** ca36b26

**3. [Rule 3 - Blocking] A decimal number in a core comment**
- **Found during:** Task 2
- **Issue:** `src/internal.h` said "89341.5 dots", which the float scan correctly reports.
- **Fix:** The comment now says "89341 and a half dots".
- **Files modified:** src/internal.h
- **Commit:** ca36b26

**4. [Rule 2 - Documentation] README updated with both lanes**
- **Found during:** Task 2
- **Issue:** CLAUDE.md rule 6 requires the README to change with behaviour. The plan's file list left it out.
- **Fix:** README lists `asan` and `nofp` and what the abi tests check. Task 1's asan line is in the Task 2 commit.
- **Files modified:** README.md
- **Commit:** ca36b26

**Total deviations:** 4 auto-fixed (1 bug, 2 blocking, 1 documentation). **Impact:** The checks keep their meaning. Only `bzero` on Darwin widens the allowlist, and only by one named symbol.

## Issues Encountered

None.

## Next Phase Readiness

The ELF allowlist is still unmeasured locally (flagged assumption A2). The first Linux `nofp` CI run (plan 12) confirms it. Ready for 01-08 (hygiene lane).

## Self-Check: PASSED

- All six created files exist on disk.
- Commits 830071f and ca36b26 are in `git log`.
- `cmake --workflow --preset ci`, `--preset asan` and `--preset nofp` exit 0 from clean build directories.
