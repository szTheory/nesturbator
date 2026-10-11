---
phase: quick-261011-inm
plan: 01
status: complete
commits: 5
plan_head_before: 9259e714d249e67f4d255d85bc264aff80497db0
plan_head_after: 7ae29150960e2f61f4845fbe941cde801eea4954
requirements: [08-WR-01, AUDIT-ASAN-TIMEOUT]
actuals:
  tasks: 3
  commits: 5
---

# Quick 261011-inm: interval save retry and shorter asan lane

A failed `--save-interval` flush is now tried again at exit (08 WR-01), and the new `NESTURBATOR_SHORT_RUNS` option, set only by the asan preset, cuts the asan lane's tests from 231 s to 78 s locally.

## Commits

- 40a861d test(runner): a failed interval save flush is tried again at exit (RED: reported "1 failed attempts, not 2")
- d7afb4b fix(runner): retry a failed interval save flush at exit (GREEN)
- 09318ed test: shorten the long tests under the asan preset
- style(runner): wrap the header comment to the format limit (hygiene.format failed on my header sentence)
- 7ae2915 ci: provisional asan timeout after shortening the long tests

## What changed

- `save_if_changed` (runner/main.c) records the generation only after a successful flush. `runner.save.interval_retry` counts two failed attempts on stderr. README and the runner header comment state the retry.
- `NESTURBATOR_SHORT_RUNS` (default OFF, set in the asan configure preset): not registered when ON are `runner.write_hashes`, `.content`, `runner.boot_to_play`, and both `vectors.registration_policy` tests. `core.reset` runs with `--short` (reset point step 20). Non-save Holy Mapperel parity (m4tnrom, m4txrom, m4w8k) compares frame 100 via an extra `-f100.ppm` dump. The helper `nesturbator_core_test` forwards `${ARGN}`.
- ci.yml asan `timeout-minutes: 15`, provisional, with a comment.
- 08-REVIEW-DISPOSITION.md: WR-01 set to `fixed`, `open:` 5 to 4. This file is modified but NOT committed (docs artifact for the orchestrator's docs commit).

## Asan measurements (local, macOS arm64, Apple clang, 4 jobs)

- `cmake --workflow --preset asan` after: 78.4 s wall, 476 tests, ctest 77.5 s.
- Before (same flags, `NESTURBATOR_SHORT_RUNS=OFF`, scratch build): ctest 231 s, 480ish tests. `vectors.registration_policy` failed there only because the scratch build dir lacks the sibling `vectors-full` tree; it is not registered in the real asan lane.
- Before top: runner.write_hashes 231 s, core.reset 104 s, runner.boot_to_play 61 s, runner.dump.nesrunner.play 17 s, libretro.parity.nesrunner.play 16 s.
- After, ten slowest: runner.dump.nesrunner.play 18.2 s, libretro.parity.nesrunner.play 14.7 s, holymapperel.m4txrom.dump 13.3 s, holymapperel.m4tnrom.dump 13.0 s, libretro.parity.m4tkrom.run1 12.8 s, libretro.parity.m4tkrom.run2 12.8 s, holymapperel.m4w8k.dump 12.7 s, holymapperel.m4tkrom.run2.dump 12.7 s, holymapperel.m4tkrom.run1.dump 12.2 s, core.reset 8.8 s.
- The hosted run was about 15 min before (write_hashes 830 s, core.reset 422 s, boot_to_play 262 s, registration_policy 131 s). The orchestrator sets the final asan timeout to twice the measured hosted run from this PR's CI; 15 is provisional.

## Deviations

- The ci `--show-only=json-v1` list is identical to before except the `backtrace` `line` numbers inside tests/CMakeLists.txt (they shifted by the added comment/guard lines); with those lines filtered, `diff` is empty. A literal `cmp` is therefore not silent.
- [Rule 3] Added the style commit: hygiene.format rejects a header line longer than 100 columns, and clang-format is not installed, so I wrapped by hand.
- The asan `vectors-full` fixture was not exercised (not needed).

## Not changed

08 IN-02 (`runner/save.c` stale `.tmp`) stays open.

## Self-Check: PASSED

`cmake --workflow --preset dev`, `ci`, `asan` and `hygiene` pass.
