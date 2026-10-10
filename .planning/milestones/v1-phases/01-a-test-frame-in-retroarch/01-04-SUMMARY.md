---
phase: 01-a-test-frame-in-retroarch
plan: 04
subsystem: video
tags: [host, convert, ppm, runner, dump-frame, d-15, d-16, frame-03]
status: complete

requires:
  - phase: 01-03
    provides: "nesturbator_get_palette and the 512-entry NTSC table"
provides:
  - "nesturbator_host: static library (hidden visibility, PIC, not installed) with nesturbator_host_convert, the native-to-XRGB8888 loop shared by runner and adapter"
  - "runner --dump-frame N:FILE: 256x240 binary P6 through the shared loop; hash unchanged over native pixels"
  - "tests/cmake/check_ppm.cmake: optional run-then-check driver for size, P6 header, first pixel and listed pixels"
  - "CTest fixture testframe_ppm: ${CMAKE_BINARY_DIR}/testframe/frame1.ppm, set up by runner.dump"
affects: [01-05, 01-09]

actuals:
  tokens: 6151
  tasks: 2
  commits: 2
plan_head_before: 7da72769529801f783362926db9c400177121ddb
plan_head_after: 05eeece566e1710a8cb6b3dffda61d6b114a8d1e

tech-stack:
  added: []
  patterns:
    - "Host-side helpers shared by runner and adapter live under host/ in target nesturbator_host, with the core's symbol properties"
    - "File-producing runner tests use a cmake -P driver that includes expect_output.cmake, then checks the file"

key-files:
  created:
    - host/convert.c
    - host/convert.h
    - tests/host/test_convert.c
    - runner/ppm.c
    - runner/ppm.h
    - tests/cmake/check_ppm.cmake
  modified:
    - CMakeLists.txt
    - tests/CMakeLists.txt
    - runner/CMakeLists.txt
    - runner/main.c
    - README.md

key-decisions:
  - "--dump-frame splits N:FILE at the first colon, so FILE may itself contain colons (Windows drive letters)"
  - "check_ppm.cmake doubles as the runner.dump driver: with CMD set it removes FILE, runs CMD through expect_output.cmake, then checks the image; no separate driver file"
  - "The runner's option loop steps by two because every option takes one value; this avoids clang's -Wfor-loop-analysis error"

patterns-established:
  - "host/ for code shared by the runner and the libretro adapter but not part of the core"

requirements-completed: [FRAME-03]

coverage:
  - id: D1
    description: "nesturbator_host_convert indexes the palette by the low 9 bits and honours both pitches"
    requirement: FRAME-03
    verification:
      - kind: unit
        ref: "tests/host/test_convert.c#main"
        status: pass
    human_judgment: false
  - id: D2
    description: "nesturbator_host is static, hidden visibility, PIC, not installed"
    requirement: FRAME-03
    verification:
      - kind: command
        ref: "grep -A6 nesturbator_host CMakeLists.txt | grep -c 'C_VISIBILITY_PRESET hidden\\|POSITION_INDEPENDENT_CODE ON' == 2"
        status: pass
    human_judgment: false
  - id: D3
    description: "--frames 1 --dump-frame 1:FILE --hash-frame 1 prints the FRAME-03 hash and writes a 184335-byte P6 with first pixel FFFFFF and (8,232) = 0x552AFF"
    requirement: FRAME-03
    verification:
      - kind: integration
        ref: "tests/CMakeLists.txt#runner.dump"
        status: pass
    human_judgment: false
  - id: D4
    description: "Bad --dump-frame arguments exit 2; an unwritable FILE exits 1"
    requirement: FRAME-03
    verification:
      - kind: integration
        ref: "tests/CMakeLists.txt#runner.usage.dump"
        status: pass
      - kind: integration
        ref: "tests/CMakeLists.txt#runner.dump.unwritable"
        status: pass
    human_judgment: false

duration: 2 min
completed: 2026-10-02
---

# Phase 1 Plan 04: Shared conversion loop and runner --dump-frame Summary

**`nesturbator_host_convert` in a hidden-visibility, PIC static library `nesturbator_host`, and `nesturbator-run --dump-frame N:FILE`, which writes the test card as a 184335-byte binary P6 through that loop while the hash stays over native pixels.**

## Performance

- Tasks: 2
- Files: 6 created, 5 modified
- Commits: 25bcfbc, 05eeece

## Accomplishments

- `host/convert.{c,h}`: `out = palette[video & 0x1FF]` for 256x240, using both pitches. The target `nesturbator_host` has `C_VISIBILITY_PRESET hidden` and `POSITION_INDEPENDENT_CODE ON`, is not installed, and the core does not link it.
- `host.convert` checks the masking (0x000, 0x1FF, 0x040, 0xFE3F to 0x03F), that row 239 reads from `video[239*300]`, and that output columns 256-259 keep a sentinel on every row.
- `runner/ppm.{c,h}`: `nesturbator_run_write_ppm` opens with "wb", writes the header and RGB rows, and checks every fwrite and the fclose.
- `runner/main.c`: repeatable `--dump-frame N:FILE` with the same N rules as `--hash-frame`. It fetches the palette once and converts with `nesturbator_host_convert`. A write failure prints to stderr and exits 1. Option parsing moved into `parse_options`.
- Tests: `runner.dump` (fixture `testframe_ppm`, checks the hash line, size, header, first pixel and pixel (8,232) = `$12` = 0x552AFF), `runner.usage.dump` (beyond --frames), `runner.usage.dump.colon` (no colon), `runner.usage.dump.file` (empty FILE), `runner.dump.unwritable`.
- README: documents `--dump-frame`, states that the hash is over native pixels, and describes the new tests.

## Task Commits

1. **Task 1: nesturbator_host and host.convert** - `25bcfbc` (feat)
2. **Task 2: runner --dump-frame as P6** - `05eeece` (feat)

## Verification

- `cmake --workflow --preset ci`: 21/21 tests passed.
- `build/ci/runner/nesturbator-run --frames 1 --dump-frame 1:build/ci/manual.ppm --hash-frame 1`: exit 0, prints the `b49e9be4...0453` line. The file is 184335 bytes and `od -c` shows `P 6 \n 2 5 6   2 4 0 \n 2 5 5 \n`.
- Negative checks: check_ppm fails on a wrong pixel colour (552AFE) and on a wrong SIZE.
- Acceptance greps: visibility/PIC count 2; `nesturbator_host_convert` in runner/main.c 1; `dump-frame` in README 3.

## Deviations from Plan

**1. [Rule 2 - Missing critical] README updated in Task 1**
- **Found during:** Task 1
- **Issue:** CLAUDE.md rule 6 requires the README to change together with each behaviour. Task 1 did not list README.md.
- **Fix:** Added a description of `host.convert`.
- **Commit:** 25bcfbc

**2. [Rule 3 - Blocking] Option loop steps by two**
- **Found during:** Task 2
- **Issue:** Clang's `-Wfor-loop-analysis` (an error under `-Werror`) rejected the extra `i++` in the restructured loop body.
- **Fix:** The loop advances `i += 2`, because every option takes exactly one value.
- **Commit:** 05eeece

**3. [Rule 2 - Missing critical] Extra usage tests and pixel check**
- **Found during:** Task 2
- **Issue:** The plan names one `runner.usage.dump` test but lists two exit-2 cases. The behaviour also includes the (8,232) pixel, which check_ppm did not cover.
- **Fix:** Added `runner.usage.dump.colon` and `runner.usage.dump.file` (an empty FILE), and an optional `PIXELS` list in check_ppm.cmake. `runner.dump` uses it for (8,232).
- **Commit:** 05eeece

**Total deviations:** 3 auto-fixed (2 missing critical, 1 blocking). **Impact:** none on scope. Every planned test name exists.

## Issues Encountered

No `asan` preset exists yet (only dev, ci, ci-msvc), so the sanitizer run was not possible. That preset is not in this plan's scope.

## Next Phase Readiness

Ready for 01-05. The adapter links `nesturbator_host` and calls `nesturbator_host_convert`. Plans 05 and 09 can consume `${CMAKE_BINARY_DIR}/testframe/frame1.ppm` via `FIXTURES_REQUIRED testframe_ppm`.

## Self-Check: PASSED

- All six created files exist on disk.
- Commits 25bcfbc and 05eeece are on phase/01-test-frame.
