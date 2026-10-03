---
phase: 02-the-cpu-matches-the-public-vectors
fixed_at: 2026-10-03T00:00:00Z
review_path: .planning/phases/02-the-cpu-matches-the-public-vectors/02-REVIEW.md
iteration: 1
findings_in_scope: 4
fixed: 4
skipped: 0
status: all_fixed
---

# Phase 2: Code Review Fix Report

**Fixed at:** 2026-10-03T00:00:00Z
**Source review:** .planning/phases/02-the-cpu-matches-the-public-vectors/02-REVIEW.md
**Iteration:** 1

**Summary:**
- Findings in scope: 4 (CR-01, WR-01, WR-02, WR-03; the six Info findings are out of scope)
- Fixed: 4
- Skipped: 0

## Fixed Issues

### CR-01: The in-source guard in fetch_vectors.cmake is lexical, so `file(REMOVE_RECURSE)` can delete the repository's `src/` or any `<dir>/src`

**Files modified:** `tests/cmake/fetch_vectors.cmake`, `tests/cmake/fetch_guard.cmake` (new), `tests/cmake/vectors_sample_match.cmake`, `tests/CMakeLists.txt`, `README.md`
**Commit:** 2a4ea01
**Applied fix:**
- The fetch keeps the lexical check, then creates DIR and repeats the check on `file(REAL_PATH)` results, in lower case on macOS and Windows hosts. It creates DIR first because REAL_PATH resolves only existing paths.
- The files now live in `DIR/65x02-src`. The script writes the marker `.nesturbator-vectors` when it creates that directory. It refuses to run, before any delete, if `DIR/65x02-src` exists without the marker.
- The consumers of the path now use the new name: `vectors_full_files` and `vectors_sample_match.cmake`.
- A new `ci` test, `vectors.fetch.guard`, runs offline with a nonexistent git. It covers three refusals:
  - an unmarked `65x02-src`, whose file must survive;
  - a symlink to the source tree (skipped on Windows);
  - the source path in upper case, when that spelling exists on a case-insensitive file system.
- The README documents the test and the new layout.

### WR-01: The nightly's pull_request path filter misses most of the lane's inputs

**Files modified:** `.github/workflows/nightly.yml`, `README.md`
**Commit:** bae80dc
**Applied fix:** The `pull_request.paths` list now has the reviewer's list plus `src/internal.h`, which `cpu.c` includes. The README's description of the nightly trigger lists the same paths. No ci test reads `nightly.yml`; the next pull-request nightly run will show the filter working.

### WR-02: The nightly's 3-minute job timeout leaves no room for network variance and overrides the fetch's own 900 s timeout

**Files modified:** `.github/workflows/nightly.yml`, `tests/CMakeLists.txt`
**Commit:** 040ca2d
**Applied fix:**
- The job log of run 37136095512 (`gh run view`) splits the measured 88 s into 65.04 s for `cpu.vectors-full.fetch` and about 23 s for everything else.
- The new `timeout-minutes` is the fetch's ctest `TIMEOUT 900` plus twice the 23 s: 946 s, rounded up to 16 minutes. A comment in the workflow shows this arithmetic.
- A comment next to `TIMEOUT 900` in `tests/CMakeLists.txt` says to change the two values together.

### WR-03: The cpu.vectors cleanup misses stray writes, so one failing test can corrupt every later test in the run

**Files modified:** `tests/cpu/test_vectors.c`, `tests/cmake/vectors_stray_write.cmake` (new), `tests/CMakeLists.txt`, `README.md`
**Commit:** f553966
**Applied fix:**
- `clear_ram(passed)` zeroes all 64 KiB after a failed test, which also covers a log overflow. After a passed test it still zeroes only the listed addresses.
- This replaces the reviewer's suggested log walk with the simpler fallback the reviewer offered.
- A new `ci` test, `cpu.vectors.stray-write`, builds a two-test e6 (INC $10) JSON at test time, so nothing new is committed and no manifest line is needed. In the first test, $10 is not listed and the expected A is wrong. The test requires `65x02/e6: 1 of 2 vectors failed`.
- Against the pre-fix binary the test fails with `2 of 2`, and the second test reports `ram[0x0010] expected 0x01 got 0x02`.
- The README documents the behaviour and the test.

## Verification

All runs were in the main checkout (`workflow.use_worktrees` is false, so no worktree was used), on macOS arm64:
- `cmake --workflow --preset ci`: exit 0, 100% of 319 tests passed, packages generated. This includes `vectors.fetch.guard` and `cpu.vectors.stray-write`.
- `cmake --workflow --preset vectors-full` (network): exit 0, 258 of 258 passed. `cpu.vectors-full.fetch` fetched into `build/vectors-full/vectors-full/65x02-src` and wrote the marker in 103.65 s; `sample-match` passed.
- The hygiene git hook passed on all four commits.

Note: the files an earlier local run fetched into `build/vectors-full/vectors-full/src` are no longer used. The script does not delete them because it did not mark them. They can be removed by hand.

---

_Fixed: 2026-10-03T00:00:00Z_
_Fixer: Claude (gsd-code-fixer)_
_Iteration: 1_
