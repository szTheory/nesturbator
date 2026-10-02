---
phase: 01-a-test-frame-in-retroarch
plan: 09
subsystem: retroarch-verification
tags: [retroarch, frame-05, bmp, ppm, sips, isolation, ctest-skip]
status: complete

requires:
  - phase: 01-06
    provides: "nesturbator-run --dump-frame N:FILE (P6, 256x240)"
  - phase: 01-05
    provides: "nesturbator_libretro module and nesturbator_libretro.info (supports_no_game)"
  - phase: 01-07
    provides: "asan preset excluding label retroarch"
provides:
  - "tests/retroarch/bmp_ppm.{c,h}: nesturbator_test_read_ppm, nesturbator_test_read_bmp, nesturbator_test_compare, nesturbator_test_compare_files"
  - "Executables compare_frame (CLI, exit 0/1/2) and test_compare_frame"
  - "CTest retroarch.compare (fixture cmp_files), retroarch.compare.cli, retroarch.testframe (label retroarch)"
  - "tests/retroarch/run_retroarch.cmake driver and tests/retroarch/test.cfg.in isolated config"
  - "Environment override NESTURBATOR_RETROARCH"
  - "README section 'Try it in RetroArch (from a build)'"
affects: [01-11, 01-12]

actuals:
  tokens: 9300
  tasks: 2
  commits: 2
plan_head_before: a14a87e20a7395bf8c1f5778633bfff0935224c3
plan_head_after: b023eb0114c5456c845eaab5c7f375d0a5c10f36

tech-stack:
  added: []
  patterns:
    - "Test-only file readers load the file into a buffer of its exact size, so asan sees any read past the end"
    - "A cmake -P driver reports a skip with a 'nesturbator-skip:' line plus exit 77 (CMake >= 3.29) and SKIP_REGULAR_EXPRESSION for 3.25-3.28"

key-files:
  created:
    - tests/retroarch/bmp_ppm.h
    - tests/retroarch/bmp_ppm.c
    - tests/retroarch/compare_frame.c
    - tests/retroarch/test_compare_frame.c
    - tests/retroarch/CMakeLists.txt
    - tests/retroarch/run_retroarch.cmake
    - tests/retroarch/test.cfg.in
  modified:
    - tests/CMakeLists.txt
    - README.md

key-decisions:
  - "nesturbator_test_compare_files holds the whole compare_frame check and message, so retroarch.compare tests the exact CLI messages (size, first difference) through the library"
  - "nesturbator_test_read_bmp returns 1 with the width and height set when the pixels do not fit the caller's buffer, so a 257x240 shot is reported as a size mismatch (exit 1) rather than a parse error"
  - "BI_BITFIELDS masks must each select one whole byte; any other mask is a parse error (exit 2)"
  - "test.cfg.in sets all 34 *_directory/*_path/*_dir keys of the pinned v1.22.2 template under @RA_DIR@, plus 4 the template does not name; libretro_path is set too, though -L overrides it"
  - "The driver takes the after-snapshot of the user's RetroArch directory before checking RetroArch's exit status, so a failed run is still checked for writes"
  - "Snapshot entries record size and modification time to the microsecond (file(TIMESTAMP ... %f UTC)); symlinks record their target, so the snapshot never follows one"

requirements-completed: [FRAME-05]

coverage:
  - id: D1
    deliverable: "compare_frame reads every BMP layout sips can produce and requires exact equality at 256x240"
    human_judgment: false
    verification:
      - kind: test
        ref: "tests/retroarch/test_compare_frame.c#retroarch.compare"
        status: pass
      - kind: test
        ref: "retroarch.compare.cli"
        status: pass
      - kind: command
        ref: "cmake --workflow --preset asan (retroarch.compare under ASan and UBSan, no reports)"
        status: pass
  - id: D2
    deliverable: "retroarch.testframe reports Skipped without RetroArch and names the missing path"
    human_judgment: false
    verification:
      - kind: command
        ref: "ctest --preset ci -L retroarch (Skipped)"
        status: pass
      - kind: command
        ref: "NESTURBATOR_RETROARCH=/nonexistent ctest --preset ci -L retroarch (prints 'nesturbator-skip: RetroArch not found at /nonexistent')"
        status: pass
  - id: D3
    deliverable: "retroarch.testframe passes against real RetroArch on the Apple Silicon Mac"
    human_judgment: true
    rationale: "RetroArch is not installed yet; plan 11 Task 2 runs it after the owner installs RetroArch. Assumption A8 (raw 256x240 screenshot under Metal) is settled there"
  - id: D4
    deliverable: "test.cfg.in redirects every path key of the pinned template under the build directory"
    human_judgment: false
    verification:
      - kind: command
        ref: "comm -23 of the template's path keys against test.cfg.in's (prints nothing)"
        status: pass

duration: 7 min
completed: 2026-10-02
---

# Phase 01 Plan 09: RetroArch frame comparison Summary

**An exact, bounds-checked P6-against-BMP comparator, tested on every platform with synthesised images, and a macOS `cmake -P` driver. The driver runs RetroArch on an isolated config, checks that the user's RetroArch directory is unchanged, and requires the `sips`-converted screenshot to equal the runner's frame. It reports Skipped where RetroArch is absent.**

## Performance

- **Duration:** 7 min
- **Started:** 2026-10-02T21:28:46Z
- **Completed:** 2026-10-02T21:36Z
- **Tasks:** 2
- **Files modified:** 9 (7 created, 2 modified)

## Accomplishments

- `compare_frame RUNNER.ppm SHOT.bmp` exits 0 only on exact RGB equality at 256x240. On a wrong size or a differing pixel it exits 1 and prints `size W x H, expected 256 x 240` or `first difference at X,Y: expected RRGGBB got RRGGBB`. An unreadable or malformed file, or a wrong argument count, gives exit 2.
- `retroarch.compare` builds one pattern as a P6 file and as six BMP layouts:
  - a 40-byte header at 24 bpp, top-down and bottom-up;
  - a 108-byte V4 header at 32 bpp with BI_BITFIELDS, bottom-up;
  - a 124-byte V5 header at 32 bpp with BI_BITFIELDS, top-down;
  - a V4 header with RGBA-order masks;
  - a 40-byte header at 32 bpp with BI_RGB.

  Every layout reads back equal. The test also covers these failures:
  - 257x240 and 256x239 images exit 1;
  - one channel off by one at (17,200) gives exactly `first difference at 17,200: expected 11C8D9 got 11C9D9`;
  - 16 kinds of truncated or malformed BMP and 8 malformed PPMs exit 2.

  The test passes under asan.
- Outside the committed tests, a BMP that real `sips` produced from the runner's frame (40-byte header, top-down, 24 bpp) compared equal. This was a scratch check.
- `retroarch.testframe` (label `retroarch`, SKIP_RETURN_CODE 77, SKIP_REGULAR_EXPRESSION, TIMEOUT 60, RUN_SERIAL) reports Skipped on this Mac without RetroArch and with `NESTURBATOR_RETROARCH=/nonexistent`.
- The driver was also run in the scratchpad with a stub RetroArch and a fake HOME:
  - a matching shot passed;
  - a mismatching shot failed with `first difference at 0,0`;
  - a change to a file in the fake user directory failed and listed `- retroarch.cfg ...` and `+ retroarch.cfg ...`;
  - a new file there failed and listed `+ cores/new.txt ...`.

  The user's real RetroArch directory was not involved.

## Pinned template key list (RetroArch v1.22.2, commit 69a4f0ea, retroarch.cfg)

The pinned template names 34 `*_directory`, `*_path` and `*_dir` keys. `test.cfg.in` sets each one under `@RA_DIR@`:

assets_directory, audio_filter_dir, cache_directory, cheat_database_path, content_database_path, content_directory, content_history_dir, content_history_path, content_image_history_path, content_music_history_path, content_video_history_path, core_assets_directory, core_options_path, dynamic_wallpapers_directory, input_remapping_directory, input_remapping_path, joypad_autoconfig_dir, libretro_directory, libretro_info_path, libretro_path, overlay_directory, playlist_directory, recording_config_directory, recording_output_directory, rgui_browser_directory, rgui_config_directory, savefile_directory, savestate_directory, screenshot_directory, system_directory, thumbnails_directory, video_filter_dir, video_font_path, video_shader_dir

`test.cfg.in` also sets these keys, which the template does not name (A7 says RetroArch ignores unknown keys): content_favorites_path, cursor_directory, log_dir, runtime_log_directory, and the non-path key menu_show_start_screen.

The template was fetched into the session scratchpad for its key names only and was not committed.

## Task Commits

1. **Task 1: Bounds-checked P6/BMP comparator and its synthesised-image tests**: `bea22be` (feat)
2. **Task 2: Unattended RetroArch driver, isolated config, snapshot check, skip semantics, README**: `b023eb0` (feat)

## Files Created/Modified

- `tests/retroarch/bmp_ppm.h`, `bmp_ppm.c`: the readers, the comparison and `compare_files`
- `tests/retroarch/compare_frame.c`: the CLI
- `tests/retroarch/test_compare_frame.c`: tests on synthesised images
- `tests/retroarch/CMakeLists.txt`: the two executables and three ctests
- `tests/retroarch/run_retroarch.cmake`: the driver (steps 1 to 9 of D-19 and D-20, with the user-directory snapshot)
- `tests/retroarch/test.cfg.in`: the isolated RetroArch configuration
- `tests/CMakeLists.txt`: `add_subdirectory(retroarch)`
- `README.md`: a Checks paragraph on `retroarch.compare` and the section "Try it in RetroArch (from a build)"

## Decisions Made

See `key-decisions` in the frontmatter.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 2 - Missing critical, CLAUDE.md rule 6] README documents the comparator tests in Task 1**
- **Found during:** Task 1
- **Issue:** The plan touches the README only in Task 2. CLAUDE.md requires each behaviour change to land with its documentation.
- **Fix:** A Checks paragraph on `retroarch.compare` and `retroarch.compare.cli` landed in the Task 1 commit.
- **Commit:** bea22be

**2. [Acceptance criterion wording] `grep -c 'assets_directory = "@RA_DIR@'` prints 2, not 1**
- **Found during:** Task 2 acceptance check
- **Issue:** The pattern is unanchored, so it also matches `core_assets_directory`. That key must stay: the coverage criterion (`comm -23` against the pinned template) requires it, and its value must start with `"@RA_DIR@`.
- **Fix:** None needed. The criterion's intent holds: the anchored `grep -c '^assets_directory = "@RA_DIR@'` prints 1.
- **Commit:** b023eb0

**Total deviations:** 2 (one documentation addition, one criterion wording issue). **Impact:** none on behaviour.

## Issues Encountered

None.

## User Setup Required

None for this plan. RetroArch is installed by the owner in plan 11.

## Verification

- `cmake --workflow --preset ci`: 100% of 28 tests passed; `retroarch.testframe` was Skipped (RetroArch not installed).
- `cmake --workflow --preset asan`: 100% of 27 passed, with no AddressSanitizer or runtime error output. This includes `retroarch.compare` and `retroarch.compare.cli`; the `retroarch` label is excluded.
- `cmake --workflow --preset nofp`: 5/5 passed.
- `cmake --workflow --preset hygiene`: 5/5 passed.
- All acceptance greps of both tasks pass, except the wording issue in deviation 2.

## Next Phase Readiness

- Plan 11 Task 2 requires `retroarch.testframe` to report Passed after the owner installs RetroArch. That run settles A8: with `video_gpu_screenshot = "false"`, the screenshot should be the raw 256x240 frame under Metal. If the frame differs, the driver names the size or the first differing pixel. If RetroArch writes anywhere in the user's directory, the driver lists the paths so the missing key can be added.
- Ready for 01-10.

## Self-Check: PASSED
