---
phase: 01-a-test-frame-in-retroarch
plan: 03
subsystem: video
tags: [palette, ntsc, palgen, xrgb8888, d-12, d-13, d-14]
status: complete

requires:
  - phase: 01-02
    provides: "tests/check.h, nesturbator_core_test, the ci test suite"
provides:
  - "tools/palgen: offline generator of the native-to-XRGB8888 table from cited NESdev Wiki facts, sqrt only, contraction off"
  - "src/palette_ntsc.c: generated const uint32_t nesturbator__palette_ntsc[512], integer-only header naming wiki revisions 24244, 24257, 23220"
  - "nesturbator_get_palette(inst, out, count): copies min(count, 512) entries"
  - "palette.regen: regenerates the table and byte-compares it (ignoring line endings)"
  - "core.palette: copy rules and the corrected D-14 invariants"
affects: [01-04, 01-05, 01-06, 01-12]

actuals:
  tokens: 7354
  tasks: 2
  commits: 2
plan_head_before: 175ed5f224b02215090c78611ad5c55db6f43223
plan_head_after: 8219654a5bae45e92ecb0bc024dc66963e6a4dd1

tech-stack:
  added: []
  patterns:
    - "Generated sources are checked in and pinned by a cmake -P test that regenerates into the build dir and compares with compare_files --ignore-eol"
    - "Host tools live under tools/<name>/ with their own CMakeLists.txt, use nesturbator_warnings, and are not installed"

key-files:
  created:
    - tools/palgen/palgen.c
    - tools/palgen/CMakeLists.txt
    - src/palette_ntsc.c
    - src/palette.c
    - tests/cmake/palette_regen.cmake
    - tests/core/test_palette.c
  modified:
    - include/nesturbator.h
    - src/internal.h
    - CMakeLists.txt
    - tests/CMakeLists.txt
    - README.md
    - .planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md
    - .planning/phases/01-a-test-frame-in-retroarch/01-CONTEXT.md

key-decisions:
  - "Phase p is sampled at reference angle 75 + 30p degrees; U uses the sine and V the cosine, which puts hue 8 (colorburst) on -U with V = 0, as palgen's self-check requires"
  - "The generated file's banner uses ASCII '--' instead of an em dash, so every source stays ASCII and MSVC /W4 /WX cannot raise C4819 on a non-UTF-8 code page"
  - "nesturbator_get_palette accepts a NULL inst, since Phase 1 has one table; inst is reserved for per-PPU-revision tables (D-08)"

patterns-established:
  - "tools/<name>/ for host tools, added with add_subdirectory from the top-level CMakeLists.txt"

requirements-completed: [FRAME-03]

coverage:
  - id: D1
    description: "palgen regenerates src/palette_ntsc.c byte for byte; a changed table fails the test"
    requirement: FRAME-03
    verification:
      - kind: integration
        ref: "tests/CMakeLists.txt#palette.regen"
        status: pass
    human_judgment: false
  - id: D2
    description: "nesturbator_get_palette copies min(count, 512) entries and writes nothing past them; NULL out and count 0 do nothing"
    requirement: FRAME-03
    verification:
      - kind: unit
        ref: "tests/core/test_palette.c#main"
        status: pass
    human_judgment: false
  - id: D3
    description: "$20/$30 white, $xE/$xF black under all 8 emphasis values, emphasis raises only its own channel, integer luma non-decreasing down every column"
    requirement: FRAME-03
    verification:
      - kind: unit
        ref: "tests/core/test_palette.c#main"
        status: pass
    human_judgment: false
  - id: D4
    description: "palgen and the generated table use only cited facts, restated in this project's own structure (no wiki example code)"
    requirement: FRAME-03
    human_judgment: true
    rationale: "Clean-room provenance is a judgment no test asserts"

duration: 1 min (measured wall clock between start and end stamps)
completed: 2026-10-02
---

# Phase 1 Plan 03: NTSC palette generator and nesturbator_get_palette Summary

**Own offline generator `tools/palgen` decodes the NESdev Wiki NTSC levels, phases and emphasis waves into a checked-in 512-entry XRGB8888 table, exposed by `nesturbator_get_palette` and pinned by a byte-for-byte regeneration test and the corrected D-14 invariants.**

## Performance

- Tasks: 2
- Files: 6 created, 7 modified
- Commits: c076313, 8219654

## Accomplishments

- `tools/palgen/palgen.c` builds each entry from the cited facts: four signal levels (normal and attenuated), the `(Y + p) mod 12 < 6` phase rule, hue 0 high only, hues 13-15 low only, `$xE/$xF` at 312 mV, emphasis waves 12/4/8 for native bits 6/7/8. It demodulates with chroma gain 2 at 15 + 30k degrees using square-root forms only, applies the 299/587/114 and 492111/877283 matrix, clips and rounds. A colorburst self-check (hue 8 to -U, V = 0) exits 1 on failure.
- `tools/palgen/CMakeLists.txt`: `-ffp-contract=off` (GCC/Clang), `/fp:precise` (MSVC), links `m` off Windows, not installed.
- `src/palette_ntsc.c` checked in. Its header lists every parameter as an integer, and the file contains no decimal point.
- The output matches all research prototype values: `$00` 626262, `$16` C23400, `$56` C52700, `$2A` 36F632, `$20`/`$30` FFFFFF, `$0D` 000000.
- `palette.regen` fails when the checked-in table differs (checked by hand against an altered copy).
- `nesturbator_get_palette` is in the public header with its comment; `src/palette.c` implements it.
- `core.palette` enforces the copy rules and invariants. D-14 in 01-CONTEXT.md is corrected in place.
- NES-HARDWARE-PPU-CARTRIDGE section 4 now names tools/palgen. The README describes the table, the API and both tests.

## Task Commits

1. **Task 1: palgen, generated table, regeneration test** - `c076313` (feat)
2. **Task 2: nesturbator_get_palette and invariant tests** - `8219654` (feat)

## Verification

`cmake --workflow --preset ci`: 15/15 tests passed, including `palette.regen` and `core.palette`.

Acceptance checks: banner count 1; no `[0-9].[0-9]` in the table; no `cos(`/`sin(`/`pow(` in palgen.c; `ffp-contract=off` count 1; all three revision IDs present; `tools/palgen` appears in the prep file; `nesturbator_get_palette` is in the header; the corrected D-14 phrase count is 1.

## Deviations from Plan

**1. [Rule 3 - Blocking] ASCII banner instead of an em dash**
- **Found during:** Task 1
- **Issue:** The plan's banner text has an em dash. No source file in the repository has a non-ASCII byte, and MSVC under `/W4 /WX` can reject non-UTF-8 bytes (C4819) when the code page is not UTF-8.
- **Fix:** The banner reads "generated by tools/palgen -- do not edit". The acceptance grep ("generated by tools/palgen") still matches.
- **Commit:** c076313

**2. [Rule 2 - Missing critical] README updated**
- **Found during:** Tasks 1 and 2
- **Issue:** CLAUDE.md rule 6 requires the README to change in the same commit as the behaviour. The plan did not list README.md.
- **Fix:** Added a "The colour table" section and descriptions of `palette.regen` and `core.palette`.
- **Commits:** c076313, 8219654

**3. [Rule 1 - Bug] Comment tripped the no-cos/sin grep**
- **Found during:** Task 1 acceptance check
- **Fix:** Reworded a comment that read like a `cos(` call. The code was unchanged.
- **Commit:** c076313

**Total deviations:** 3 auto-fixed (1 blocking, 1 missing critical, 1 bug). **Impact:** none on behaviour or the table's contents.

## Issues Encountered

None.

## Next Phase Readiness

Ready for 01-04. D-05 can take its hard-coded pixel values from the checked-in table, which matches the research prototype. The cross-platform byte equality of `palette.regen` is not shown until the six-runner CI in plan 12 runs it (flagged assumption A1).

## Self-Check: PASSED

- All six created files exist on disk.
- Commits c076313 and 8219654 are on phase/01-test-frame.
