---
phase: 01-a-test-frame-in-retroarch
plan: 08
subsystem: verification-lanes
tags: [hygiene, clang-format, action-pins, manifest, frame-06]
status: complete

requires:
  - phase: 01-07
    provides: "asan and nofp presets; README lane commentary moved into the new Checks section"
provides:
  - "Presets hygiene (configure, build, test, workflow): NESTURBATOR_HYGIENE=ON, label hygiene only"
  - "Option NESTURBATOR_HYGIENE; cache variables NESTURBATOR_SH, NESTURBATOR_GIT, NESTURBATOR_CLANG_FORMAT (major version 18 or configure fails)"
  - "CTest label hygiene: hygiene.tree, hygiene.action_pins, hygiene.action_pins.bad, hygiene.action_pins.good, hygiene.format"
  - "scripts/hygiene.sh rule 'binary file not in manifest' in --staged, --tree and --history modes"
  - "tests/roms/manifest.txt: format header, no entries"
  - ".clang-format (LLVM base, 4 spaces, 100 columns, Linux braces) applied to every tracked C source but libretro.h"
affects: [01-12]

actuals:
  tokens: 10500
  tasks: 2
  commits: 2
plan_head_before: d882a1682b9417d5381f83563862a0c366c708e1
plan_head_after: 48c17a23ef8a635a692a0672a73971221c522e72

tech-stack:
  added: ["clang-format 18 (Homebrew llvm@18, keg-only, local build tool for the hygiene preset)"]
  patterns:
    - "Hygiene checks are CTest tests under one label, each with a fixture showing it can fail"
    - "Generated or layout-sensitive code is fenced with bare /* clang-format off */ and /* clang-format on */ lines; the reason goes in a separate comment above"

key-files:
  created:
    - .clang-format
    - tests/hygiene/CMakeLists.txt
    - tests/hygiene/fixtures/pins_bad.yml
    - tests/hygiene/fixtures/pins_good.yml
    - tests/cmake/action_pins.cmake
    - tests/cmake/format_check.cmake
    - tests/roms/manifest.txt
  modified:
    - scripts/hygiene.sh
    - CMakeLists.txt
    - CMakePresets.json
    - README.md
    - ASSET_POLICY.md
    - tools/palgen/palgen.c
    - src/palette_ntsc.c
    - include/nesturbator.h
    - "14 other C sources (whitespace only, from the formatting pass)"

key-decisions:
  - "The binary rule keys on a NUL byte, as git does: LC_ALL=C grep -I -q '' on non-empty content, so BSD grep, GNU grep and ugrep agree and non-UTF-8 text is not flagged"
  - "History mode finds binaries with git diff-tree --numstat, where a binary shows '-' for both counts"
  - "palgen emits /* clang-format off */ and /* clang-format on */ around the colour table to keep eight entries per line (two lines per palette row); palette.regen still matches byte for byte"
  - "NESTURBATOR_CONFIG_INIT in the public header is fenced from clang-format, which would lay its braced initialiser out as a block"
  - "clang-format 18 ignores a block-comment marker with trailing text (/* clang-format off: why */), so the markers are bare and the reason sits on the line above"
  - "action_pins.cmake reads files with file(READ) and splits on newlines, because file(STRINGS) drops empty lines and would skew the reported line numbers"

patterns-established:
  - "tests/hygiene holds tests registered only under NESTURBATOR_HYGIENE; their scripts stay in tests/cmake"

requirements-completed: [FRAME-06]

coverage:
  - id: D1
    deliverable: "hygiene lane runs five tests under label hygiene and passes on the Mac"
    human_judgment: false
    verification:
      - kind: command
        ref: "cmake --workflow --preset hygiene"
        status: pass
      - kind: command
        ref: "ctest --preset hygiene -N (lists the five hygiene.* tests)"
        status: pass
  - id: D2
    deliverable: "hygiene.sh reports an unlisted binary file and accepts it once the manifest lists it"
    human_judgment: false
    verification:
      - kind: command
        ref: "scratch git repo with a 16-byte binary: --tree and --history exit 1 with 'binary file not in manifest'; exit 0 after listing it"
        status: pass
  - id: D3
    deliverable: "Unpinned actions fail; SHA-pinned and local ones pass"
    human_judgment: false
    verification:
      - kind: test
        ref: "hygiene.action_pins.bad (WILL_FAIL, reports lines 7 and 9)"
        status: pass
      - kind: test
        ref: "hygiene.action_pins.good"
        status: pass
  - id: D4
    deliverable: "Unformatted code fails hygiene.format; a non-18 clang-format fails configure"
    human_judgment: false
    verification:
      - kind: command
        ref: "appending an unformatted line to src/frame.c makes hygiene.format fail (then restored)"
        status: pass
      - kind: command
        ref: "configure with NESTURBATOR_CLANG_FORMAT=/usr/bin/true stops with 'the hygiene preset needs clang-format 18'"
        status: pass
  - id: D5
    deliverable: "Formatting pass breaks nothing; every lane stays green"
    human_judgment: false
    verification:
      - kind: command
        ref: "cmake --workflow --preset ci (25 tests, palette.regen included)"
        status: pass
      - kind: command
        ref: "cmake --workflow --preset asan"
        status: pass
      - kind: command
        ref: "cmake --workflow --preset nofp"
        status: pass

duration: 4 min
completed: 2026-10-02
---

# Phase 01 Plan 08: Hygiene Lane Summary

**`hygiene` preset with five CTest checks: the forbidden-content scan (now rejecting unlisted binary files), a SHA-pin check on every GitHub `uses:` key, and a clang-format 18 dry run over all tracked C sources, which were formatted once.**

## Performance

- **Duration:** 4 min
- **Started:** 2026-10-02T21:22Z
- **Completed:** 2026-10-02T21:26:36Z
- **Tasks:** 2
- **Files modified:** 27

## Accomplishments

- `scripts/hygiene.sh` reports `binary file not in manifest` for any non-empty file holding a NUL byte that `tests/roms/manifest.txt` does not list, in staged, tree and history modes.
- `tests/roms/manifest.txt` documents the `path<TAB>source<TAB>pin<TAB>licence<TAB>sha256` format and points to ASSET_POLICY.md; it has no entries in Phase 1.
- `hygiene.action_pins` checks every `uses:` line (any indentation, with or without `- `) under `.github` for a full 40-hex SHA or a `./` path, and prints file:line for each offender. There is no `.github` yet, so it prints "no workflow files" and passes. The bad and good fixtures show it fails and passes as expected.
- `hygiene.format` lists sources with `git ls-files`, drops the vendored `libretro/libretro.h`, and runs `clang-format --dry-run --Werror`. Configure stops unless the clang-format it finds is version 18.
- The README has a new "Checks" section with a table of the five lanes (`ci`, `ci-msvc`, `asan`, `nofp`, `hygiene`), the abi and hygiene test descriptions, and the format command. ASSET_POLICY.md describes the binary rule.

## Task Commits

1. **Task 1: hygiene preset, binary-file rule, action-pin check** - `65b96e7` (feat)
2. **Task 2: clang-format 18 check, formatting pass, README Checks** - `48c17a2` (feat)

## Files Created/Modified

- `scripts/hygiene.sh` - binary-file rule in `check_file` and in history mode
- `tests/roms/manifest.txt` - manifest format header
- `tests/cmake/action_pins.cmake` - SHA-pin check
- `tests/cmake/format_check.cmake` - clang-format dry run over `git ls-files`
- `tests/hygiene/CMakeLists.txt`, `tests/hygiene/fixtures/pins_{bad,good}.yml` - label `hygiene` tests and fixtures
- `CMakeLists.txt`, `CMakePresets.json` - `NESTURBATOR_HYGIENE`, tool lookup, version guard, `hygiene` presets
- `.clang-format` - style rules
- `tools/palgen/palgen.c`, `src/palette_ntsc.c` - format-off fence around the generated table
- `include/nesturbator.h` - format-off fence around `NESTURBATOR_CONFIG_INIT`; trailing-comment realignment
- `README.md`, `ASSET_POLICY.md` - Checks section; binary rule
- 14 other C sources - whitespace changes only

## Decisions Made

See `key-decisions` in the frontmatter. The main ones:

- Binary detection follows git's rule (a NUL byte) and runs grep in the C locale, so results match across platforms.
- The palette table and the config initialiser macro are fenced from clang-format rather than reflowed.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Line numbers in action_pins.cmake**
- **Found during:** Task 1
- **Issue:** `file(STRINGS)` drops empty lines, so the file:line output would have pointed at the wrong lines.
- **Fix:** Read with `file(READ)`, replace brackets and semicolons (which would break list splitting), then split on newlines.
- **Files modified:** tests/cmake/action_pins.cmake
- **Commit:** 65b96e7

**2. [Rule 3 - Blocking] clang-format would reflow the palette table and the config macro**
- **Found during:** Task 2
- **Issue:** clang-format packs the 512-entry table nine to a line and explodes `NESTURBATOR_CONFIG_INIT` into a seven-line block. A marker with trailing text (`/* clang-format off: why */`) is ignored by version 18.
- **Fix:** palgen now writes a reason comment, then bare `/* clang-format off */` before the table and `/* clang-format on */` after it, and the table was regenerated. The header macro is fenced the same way, and its doc comment gives the reason.
- **Files modified:** tools/palgen/palgen.c, src/palette_ntsc.c, include/nesturbator.h
- **Commit:** 48c17a2

**3. [Rule 2 - Docs] ASSET_POLICY.md "How this is checked"**
- **Found during:** Task 1
- **Issue:** CLAUDE.md rule 6 says docs change in the same commit as the behaviour. The policy did not mention the new binary rule.
- **Fix:** The section now covers the binary rule and names `hygiene.tree`.
- **Commit:** 65b96e7

**Total deviations:** 3 auto-fixed (1 bug, 1 blocking, 1 docs). **Impact:** No change in scope. The palette table keeps its layout and `palette.regen` passes.

## Issues Encountered

- During Task 2 a stray `git checkout -- include/nesturbator.h` threw away the first header edit and its formatting. Both were redone before the commit, and nothing was lost from history.

## User Setup Required

None. `llvm@18` was installed with Homebrew, which the plan allows for command-line build tools. It is keg-only, so the system clang is unchanged.

## Next Phase Readiness

- Ready for 01-09. Plan 12's CI workflow will be the first file `hygiene.action_pins` checks for real. Every `uses:` in it must be pinned to a full SHA.
- When test ROMs arrive in a later phase, they need lines in `tests/roms/manifest.txt`. Otherwise `hygiene.tree` and the pre-commit hook reject them.

## Self-Check: PASSED

- All 7 created files exist on disk.
- Commits 65b96e7 and 48c17a2 are on phase/01-test-frame.
- `cmake --workflow --preset` ci, asan, nofp and hygiene all exit 0.
- `grep -c "asan\|nofp\|hygiene" README.md` = 10.
