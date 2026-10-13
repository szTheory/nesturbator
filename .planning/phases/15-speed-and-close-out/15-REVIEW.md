---
phase: 15-speed-and-close-out
reviewed: 2026-10-13T00:00:00Z
depth: standard
files_reviewed: 28
files_reviewed_list:
  - .github/workflows/ci.yml
  - .github/workflows/nightly.yml
  - .github/workflows/release.yml
  - CMakeLists.txt
  - README.md
  - runner/CMakeLists.txt
  - runner/bench.c
  - runner/bench.h
  - runner/clock.c
  - runner/clock.h
  - runner/main.c
  - runner/stop.c
  - tests/CMakeLists.txt
  - tests/cmake/ci_required_rollup.cmake
  - tests/cmake/perf_compare.cmake
  - tests/cmake/perf_gate_selftest.cmake
  - tests/cmake/perf_roms_render.cmake
  - tests/cmake/runner_bench.cmake
  - tests/cmake/runner_options.cmake
  - tests/cmake/runner_stop.cmake
  - tests/cmake/timeout_comments.cmake
  - tests/hygiene/CMakeLists.txt
  - tests/hygiene/fixtures/timeouts_bad.yml
  - tests/hygiene/fixtures/timeouts_good.yml
  - tests/perf/allow.txt
  - tests/perf/fixtures/cachegrind.out.in
  - tests/runner/test_bench.c
  - tests/runner/test_signal.c
findings:
  critical: 0
  warning: 4
  info: 3
  total: 7
status: issues_found
---

# Phase 15: Code Review Report

**Reviewed:** 2026-10-13
**Depth:** standard
**Files Reviewed:** 28
**Status:** issues_found

## Summary

The `--bench` code (bench.c, clock.c, the main.c loop) is correct on the paths I traced. I checked the nearest-rank arithmetic, the unsigned overflow bounds in the fps and Windows tick conversions, the sample-array indexing under `--bench-state`, and the stop/report ordering. I found no crash or data-loss defect. The defects are in the CI gate logic, the allow-file parser, the documentation of the new signal behaviour, and timeout margins.

## Warnings

### WR-01: Perf gate passes silently when the head measurement is missing and the base is also missing

**File:** `tests/cmake/perf_compare.cmake:221-229`
**Issue:** The `elseif(base_a STREQUAL "" OR base_b STREQUAL "")` branch (status n/a) is evaluated before the `elseif(head STREQUAL "")` branch. When the base build or run failed, a head runner that fails under valgrind also yields n/a and exit 0 in gate mode. The header comment (line 16), the ci.yml comment ("a missing head ... does [fail]") and the README all promise that a missing head fails the gate. The "Measure under Cachegrind" step only emits `::warning::` when valgrind fails, so nothing else catches it. A PR that breaks the runner under Cachegrind passes `CI required` whenever the base is unavailable. No selftest covers "missing head and missing base".
**Fix:** Test the head first in gate mode:
```cmake
if(head STREQUAL "" AND MODE STREQUAL "gate")
  set(status "FAIL")
  list(APPEND errors "perf gate: missing head for ${rom}")
elseif(base_a ... UNSTABLE ...)
...
```
Add a selftest case, `MISSING head:runner base:runner`, that expects exit 1.

### WR-02: `file(STRINGS)` on the allow file mangles reasons that contain `;` or `[` `]`

**File:** `tests/cmake/perf_compare.cmake:152-154`
**Issue:** `file(STRINGS "${ALLOW}" allow_lines)` returns a CMake list. A `;` in a line, such as the plausible reason "PPU fix; see #12", splits it into two list elements. The tail then fails the line regex and is reported as `malformed allow line: N`, and the numbering drifts for every later line. Unbalanced `[` or `]` can swallow following lines. This turns a valid exception into a hard failure of every PR job with a misleading message. `timeout_comments.cmake` already works around the same CMake behaviour (`REGEX REPLACE "[][;]" "_"`), so the pattern is known in this repo. The selftests never put these characters in a reason.
**Fix:** Read the file with `file(READ)`, replace `[`, `]` and `;` with a placeholder, then split on `\n`. Reasons are never printed, so the placeholder is harmless. Add selftest cases for a reason containing `;` and one containing `[`.

### WR-03: An inherited SIG_IGN is honoured but not documented, and it can hang the stop tests

**File:** `runner/stop.c:29-35`, `README.md` ("Stopping the runner", line ~1294), `tests/cmake/runner_stop.cmake` (`frame_exact.int`, `frame_exact.term`)
**Issue:** The behaviour change is correct and deliberate (14-REVIEW WR-01), but rule 6 of CLAUDE.md requires the README and header comments to change with behaviour. Neither `stop.h` nor the README says that a runner started with SIGINT or SIGTERM ignored (a shell background job, `nohup`) will not stop on that signal. A launcher author reading the README would not expect that. In the same environment the tests misbehave: `runner.stop.frame_exact.int` and `runner.signal.int` run with `--frames 4294967295` and rely on the signal. If ctest is started from a background job with SIGINT ignored, the runner ignores it, runs until the 30 s TIMEOUT, and the failure is confusing. `inherited_ignore` shows the author knows the case.
**Fix:** Add one sentence to the README "Stopping the runner" paragraph and to `stop.h` stating that a signal the process inherited as ignored stays ignored. For the tests, either reset the disposition in the test harness (`sh -c 'trap - INT TERM; exec ...'`) or have `runner_stop.cmake` fail fast with a clear message.

### WR-04: One-minute job timeouts sit at about 2 to 3 times a single sample

**File:** `.github/workflows/release.yml:25`, `.github/workflows/ci.yml:41,54,..` (hygiene, title, hash-equality, ci-required)
**Issue:** `release-please` is set to `timeout-minutes: 1` from one 22 s run. `hygiene` is 1 minute from one 21 s run, and several jobs have a 2 to 5 s sample. The jobs include checkout, network calls (`gh`, the release-please API) and runner start-up, all of which have heavy tails on hosted runners. A timeout kill of `release-please` mid-run can leave a release half created. A kill of a required CI job blocks merging until a re-run. The ENGINEERING rule "twice a measured run" is applied literally, but the nightly jobs now use the slowest of three runs while the CI and release jobs use a single run. The `hygiene.timeout_comments` lint checks only that some run ID is present, not how many samples back it.
**Fix:** Raise the network-bound and release jobs (`release-please`, `title`, `hygiene`, `ci-required`) to a floor of 5 minutes. Keep the measured values for the long jobs.

## Info

### IN-01: `timeout_comments.cmake` accepts any 9-digit number

**File:** `tests/cmake/timeout_comments.cmake:30,53`
**Issue:** The pattern `runs? [0-9]{9}` is satisfied by `run 123456789`, so the lint proves a comment exists, not that a run was measured. The README and comments present it as enforcing "measured".
**Fix:** Say in the file header that the check is presence-only, or verify the ID with `gh run view` in a non-gating job.

### IN-02: `bench.h` helpers have undefined behaviour for n == 0

**File:** `runner/bench.c:11-14,44`, `runner/bench.h`
**Issue:** `nesturbator_run_bench_rank(0, p)` underflows to `UINT32_MAX`, and `series_line` reads `s[n - 1u]`. The header documents `n >= 1` and `bench_report` guards `count == 0`, so there is no live bug. The public helpers are still easy to misuse.
**Fix:** Return early from `series_line` when `n == 0`, or add `assert(n >= 1u)`.

### IN-03: Fixed 512-byte report buffer truncates silently

**File:** `runner/main.c` (bench report block), `runner/bench.c:60-72`
**Issue:** `append_line` clamps on overflow with no signal. Typical output is about 400 bytes of the 512, and a ten-digit microsecond value on all four series would exceed it. A later line would then be cut without notice.
**Fix:** Make `report` 1024 bytes, or have `bench_report` return an error on truncation.

---

_Reviewed: 2026-10-13_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_
