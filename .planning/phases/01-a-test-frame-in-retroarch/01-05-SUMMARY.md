---
phase: 01-a-test-frame-in-retroarch
plan: 05
subsystem: libretro
tags: [libretro, adapter, vendored, gitattributes, frame-04, d-05, d-10, d-15]
status: complete

requires:
  - phase: 01-04
    provides: "nesturbator_host_convert and the testframe_ppm fixture (build/ci/testframe/frame1.ppm)"
provides:
  - "libretro/libretro.h vendored at RetroArch v1.22.2, pinned by ctest libretro.vendored"
  - ".gitattributes: LF checkout for every text file on every platform"
  - "nesturbator_libretro MODULE: no lib prefix, .dylib/.so/.dll, exports only the 25 retro_* functions"
  - "libretro/nesturbator_libretro.info with supports_no_game and a release-please-marked display_version"
  - "ctest libretro.host: loads the module like a frontend and compares its frame with the runner's P6 (FRAME-04)"
  - "THIRD-PARTY-NOTICES.md and the PROVENANCE vendored-file row"
affects: [01-09, 01-10]

actuals:
  tokens: 82243
  tasks: 2
  commits: 2
plan_head_before: 6aebf27480e239a0969b50084b6fa4829e1df25f
plan_head_after: 5cd067341d120f506638233bfdf9123170f1d985

tech-stack:
  added: ["libretro.h (vendored, MIT, RetroArch v1.22.2)"]
  patterns:
    - "Run-time loading test: dlopen/LoadLibraryA, symbol address copied into the typed pointer with memcpy"
    - "Vendored-file pin: cmake -P script comparing file(SHA256) with the recorded hash"

key-files:
  created:
    - .gitattributes
    - libretro/libretro.h
    - libretro/libretro.c
    - libretro/nesturbator_libretro.info
    - libretro/CMakeLists.txt
    - THIRD-PARTY-NOTICES.md
    - tests/cmake/vendored_sha256.cmake
    - tests/libretro/libretro_host.c
  modified:
    - CMakeLists.txt
    - PROVENANCE.md
    - tests/CMakeLists.txt
    - tests/cmake/version_consistency.cmake
    - README.md

key-decisions:
  - "libretro.vendored uses a small reusable driver, tests/cmake/vendored_sha256.cmake (FILE, SHA256), rather than inline -P code"
  - "retro_load_game also returns false when an instance already exists, so a second load cannot leak the first instance"
  - "The libretro.host test reads the frame pitch from the callback and copies rows, so a core that pads rows still compares correctly"

patterns-established:
  - "libretro/ holds the adapter, its .info file and the vendored header; the module links the core and nesturbator_host privately"

requirements-completed: [FRAME-04]

coverage:
  - id: D1
    description: "libretro.h is byte-identical to the v1.22.2 pin and checks out LF everywhere"
    requirement: FRAME-04
    verification:
      - kind: integration
        ref: "tests/CMakeLists.txt#libretro.vendored"
        status: pass
      - kind: command
        ref: "git check-attr eol -- libretro/libretro.h -> eol: lf"
        status: pass
    human_judgment: false
  - id: D2
    description: "The module is nesturbator_libretro.dylib on macOS and exports exactly 25 retro_* symbols and no nesturbator_* symbol"
    requirement: FRAME-04
    verification:
      - kind: command
        ref: "nm -gU build/ci/libretro/nesturbator_libretro.dylib | grep -c ' _retro_' == 25; grep -c '_nesturbator_' == 0"
        status: pass
    human_judgment: false
  - id: D3
    description: "Called as RetroArch calls it, the module refuses content, starts without content in XRGB8888, reports 256x240 at 39375000/655171 fps and 48000 Hz, and one retro_run gives one video call and one 798-frame silent batch"
    requirement: FRAME-04
    verification:
      - kind: integration
        ref: "tests/libretro/libretro_host.c#main"
        status: pass
    human_judgment: false
  - id: D4
    description: "The libretro frame equals the runner's P6 pixel for pixel, and four hard-coded D-05 colours hold"
    requirement: FRAME-04
    verification:
      - kind: integration
        ref: "tests/libretro/libretro_host.c#compare_with_ppm"
        status: pass
    human_judgment: false
  - id: D5
    description: "The .info display_version matches version.txt"
    requirement: FRAME-04
    verification:
      - kind: integration
        ref: "tests/CMakeLists.txt#version.consistency"
        status: pass
    human_judgment: false

duration: 3 min
completed: 2026-10-02
---

# Phase 1 Plan 05: libretro adapter and host test Summary

**libretro.h vendored at RetroArch v1.22.2 and pinned by SHA-256, a 25-function libretro adapter built as `nesturbator_libretro.dylib/.so/.dll` exporting only `retro_*`, and a host test that loads it like RetroArch and gets the runner's exact frame (FRAME-04).**

## Performance

- **Duration:** about 3 min
- **Started:** 2026-10-02T21:06Z
- **Completed:** 2026-10-02T21:10Z
- **Tasks:** 2
- **Files:** 13 (8 created, 5 modified)

## Accomplishments

- `.gitattributes` (`* text=auto eol=lf`); `git add --renormalize .` changed no tracked file.
- `libretro/libretro.h` fetched from the pinned commit; SHA-256 `bd3398d2…947acb` matches; ctest `libretro.vendored` checks it.
- `libretro/libretro.c`: all 25 entry points. Set-environment declares no-game support; load_game(NULL) sets XRGB8888, creates the instance and fetches the palette once; load_game with content returns false (D-10). retro_run polls input, runs a frame, converts through `nesturbator_host_convert` (D-15), and makes one video call (pitch 1024 bytes) and one stereo batch call. Deinit resets every global.
- `nesturbator_libretro` MODULE target: `PREFIX ""`, hidden visibility, `.dylib` on Apple. `nm -gU` shows 25 `_retro_` symbols and no `_nesturbator_` symbol.
- `nesturbator_libretro.info` with the LIBRETRO-AND-RUNNER §2 keys, and `display_version` between release-please markers. `version.consistency` now also checks it.
- `libretro.host` test: resolves all 25 symbols and checks each step of the RetroArch call order. The frame matches `frame1.ppm` pixel for pixel, and the four D-05 pixels hold. It was checked against a corrupted copy of the PPM: it fails, naming the first differing pixel.
- PROVENANCE row, THIRD-PARTY-NOTICES.md (MIT text from header L10-27), README "The libretro core" section.

## Task Commits

1. **Task 1: Vendor libretro.h, LF checkout, adapter module and .info** - `cff21df` (feat)
2. **Task 2: libretro host test against the runner's frame** - `5cd0673` (test)

## D-05 cross-check

The values in `src/palette_ntsc.c` match the research candidates exactly. Entry 0x30 is FFFFFF, 0x16 is C23400, 0x56 is C52700 and 0x0D is 000000. The test card puts natives $30, $16, $56 and $0D at (0,0), (100,18), (100,46) and (213,10).

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] check.h include path**
- **Found during:** Task 2
- **Issue:** `#include "check.h"` did not resolve. The other tests include it as `"../check.h"`.
- **Fix:** Used `"../check.h"`, as the other tests do.
- **Files modified:** tests/libretro/libretro_host.c
- **Commit:** 5cd0673

**2. [Rule 2 - Missing critical] retro_load_game refuses a second load**
- **Found during:** Task 1
- **Issue:** As written, a second load_game(NULL) without an unload would overwrite and leak the instance.
- **Fix:** load_game returns false when an instance already exists.
- **Files modified:** libretro/libretro.c
- **Commit:** cff21df

Minor: the vendored-hash check is a small driver file, `tests/cmake/vendored_sha256.cmake`, not in the plan's file list. The plan asked for "`cmake -P` with `file(SHA256 ...)`", and this is that script.

**Total deviations:** 2 auto-fixed (1 blocking, 1 missing critical). **Impact:** none on scope.

## Issues Encountered

None.

## Next Phase Readiness

The module that plan 09's `retroarch.testframe` loads exists at `build/ci/libretro/nesturbator_libretro.dylib`, and its `.info` file is at `libretro/nesturbator_libretro.info`. The flagged assumption stands: whether real RetroArch makes the same calls is shown only by plan 09.

## Self-Check: PASSED

- All 8 created files exist on disk.
- Commits cff21df and 5cd0673 are in `git log`.
- `cmake --workflow --preset ci`: 23 of 23 tests pass, including libretro.vendored and libretro.host.
