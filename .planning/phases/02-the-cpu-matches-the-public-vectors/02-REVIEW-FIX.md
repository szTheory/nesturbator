---
phase: 02-the-cpu-matches-the-public-vectors
fixed_at: 2026-10-03T00:00:00Z
review_path: .planning/phases/02-the-cpu-matches-the-public-vectors/02-REVIEW.md
iteration: 1
findings_in_scope: 10
fixed: 10
skipped: 0
status: all_fixed
---

# Phase 2: Code Review Fix Report

**Fixed at:** 2026-10-03T00:00:00Z
**Source review:** .planning/phases/02-the-cpu-matches-the-public-vectors/02-REVIEW.md
**Iteration:** 1 (re-review pass; the earlier pass covered CR-01 and WR-01 to WR-03)

**Summary:**
- Findings in scope: 10 (WR-04, IN-01 to IN-09)
- Fixed: 10
- Skipped: 0

**Where verification ran:** in the main checkout on `phase/02-cpu-vectors`
(`workflow.use_worktrees` is `false`, so no worktree was made). Before each
commit `cmake --workflow --preset ci` passed (319, then 320 and 321 tests as
tests were added). `hygiene` also ran after the IN-04, IN-05, IN-06 and
IN-07 changes, `vectors-full` (258 tests) after IN-05, IN-06, IN-07 and IN-08,
and `asan` after IN-06. All passed.

## Fixed Issues

### WR-04: The nightly's path filter still misses inputs that change the CPU build the lane runs

**Files modified:** `.github/workflows/nightly.yml`, `README.md`
**Commit:** 93167b5
**Applied fix:** Added `CMakeLists.txt` and `include/nesturbator.h` to the
nightly's `pull_request.paths`. The comment no longer says "every file the
lane builds". It now says the lane builds the whole `ci` tree and the list
holds the files that can change what its tests run. The README's list matches.

### IN-01: A new instance's CPU state is all zeros, and "only reset clears it" has no reset to point to

**Files modified:** `src/internal.h`
**Commit:** d7f6c32
**Applied fix:** Comment only. `internal.h` now says the power-up state is
not set yet: a new CPU has P, S and PC equal to 0, no reset vector is fetched,
and the phase that runs the CPU in frames adds the reset sequence. It also
says `halted_in_read` is unused, and the `jammed` comment now says "nothing
clears it yet". The reset sequence itself waits for the phase that runs the
CPU in frames.

### IN-02: `ticks` will be advanced twice once the CPU runs in frames

**Files modified:** `src/frame.c`
**Commit:** 3aa7726
**Applied fix:** Comment only, at the `ticks += NESTURBATOR_TICKS_PER_FRAME`
line. Once the CPU runs in frames, the frame must step it until `ticks`
reaches the frame's end, not add this constant, or time is counted twice.

### IN-03: Build helpers leak into an embedding project's namespace

**Files modified:** `tests/embed/CMakeLists.txt`, `README.md`, `CMakeLists.txt`, `tests/CMakeLists.txt`
**Commit:** 0b7292c
**Applied fix:** The README's embedding paragraph now lists the four helpers
an embedding build gets: `nesturbator_cpu`, `nesturbator_core_flags`,
`nesturbator_warnings` and `NESTURBATOR_NOFP`. `embed.subdirectory` now
requires the embedded directory's targets to be exactly
`nesturbator_cpu;nesturbator`, and requires both functions and the option to
exist. A new target therefore fails the test until the list and the README
are updated. Mutation check: with the expected list changed to `nesturbator`,
the test failed and printed the actual targets.

### IN-04: Script-mode CMake files set policies inconsistently

**Files modified:** 18 scripts in `tests/cmake/` (the ones the review lists), new `tests/cmake/script_policy.cmake`, `tests/CMakeLists.txt`, `README.md`
**Commit:** c453ebf
**Applied fix:** Added `cmake_minimum_required(VERSION 3.25)` after the
header comment of each of the 18 scripts. A new `ci` test,
`cmake.script_policy`, requires that line in every `tests/cmake/*.cmake`, so
a new script cannot leave it out. Mutation check: run on a copy holding the
old `check_ppm.cmake`, the test failed and named that file.
`cmake/packaging.cmake` is not covered: it is loaded with `include()`, not
run with `-P`.

### IN-05: vecconv accepts leading zeros, which JSON forbids

**Files modified:** `tools/vecconv/vecconv.c`, `tests/cmake/vecconv_negative.cmake`, `tests/CMakeLists.txt`, `README.md`
**Commit:** e5f7041
**Applied fix:** `parse_uint` rejects a `0` followed by another digit with
"number with a leading zero" and the number's offset (RFC 8259 section 6).
New case `vecconv.reject.leading_zero` (`"y": 9` becomes `"y": 09`) passes
on exactly that error at offset 84. The header comment and the README's
reject list (now fourteen tests) are updated. `vectors-full` still converts
all 256 upstream files.

### IN-06: Helpers are duplicated across the vector tools

**Files modified:** `tests/vectors/n65v.c`, `tests/vectors/n65v.h`, `tests/cpu/test_vectors.c`, `tests/vectors/test_n65v.c`, `tools/vecconv/vecconv.c`
**Commit:** 2713836
**Applied fix:** `n65v_read_file` (the malloc-and-grow version) and
`n65v_hex_digit` now live in `n65v.c`, which all three programs already
link. The three `read_file` copies and the two `hex_digit` copies were
removed. vecconv's two read sites now use the shared reader. Net change: 57
fewer lines. `tests/retroarch/bmp_ppm.c` has its own `read_file` too, but it
belongs to the RetroArch comparison tool, not the vector tools, so it is
left alone.

### IN-07: The fetch creates DIR before its resolved-path check, so a refused DIR still leaves directories in the source tree

**Files modified:** `tests/cmake/fetch_vectors.cmake`, `tests/cmake/fetch_guard.cmake`, `README.md`
**Commit:** e968d1f
**Applied fix:** The script now walks up to DIR's deepest existing ancestor,
runs `REAL_PATH` on it and appends the part that does not exist yet. It
creates DIR only after both in-source checks pass. `vectors.fetch.guard`
gained a case: `DIR=<link-to-source>/fetch-guard-probe/deeper` must be
refused, and `fetch-guard-probe` must not exist in the source tree afterwards
(the test removes it if it does, then fails). Against the old script the new
case fails. A new DIR outside the tree is still created (checked by hand with
a three-level path in the scratch directory).

### IN-08: The lower-casing step is redundant, and `vectors.fetch.guard` cannot detect its removal

**Files modified:** `tests/cmake/fetch_vectors.cmake`, `tests/cmake/fetch_guard.cmake`, `README.md`
**Commit:** a02d147
**Applied fix:** Kept the `TOLOWER` block and labelled it as a fallback. The
comments now say `REAL_PATH`, which resolves symlinks and on-disk case, is
what refuses a symlinked or differently cased DIR. The guard's case 3 comment
says it passes without the lower-casing, and the README says the comparison
uses the resolved path and on-disk case. The block was kept, not dropped,
because after IN-07 it has a real use: the part of DIR that does not exist
yet cannot be canonicalised by `REAL_PATH`. Example: `<src>/BUILD/x`, when
`build/` does not exist yet.

### IN-09: Fetched files in the old `DIR/src` layout are orphaned with no note for users

**Files modified:** `README.md`
**Commit:** a27a5d8
**Applied fix:** The README's local vectors-full paragraph now says that
files fetched before the move to `65x02-src` remain in `<dir>/src`, take
about 1 GB, are never deleted by the fetch, and should be deleted by hand.
The fix adds no code that deletes unmarked directories.

## Notes for the reviewer

- IN-01, IN-02, IN-08 and IN-09 change only comments or documentation. IN-01
  and IN-02 are design notes for the phase that runs the CPU in frames, and
  the comments are the smallest change that records the hazard where the
  code is.
- IN-07 changes path-resolution logic. It is tested by the new guard case,
  but someone should read the ancestor walk to confirm it is right on
  Windows drive roots (`PARENT_PATH` of `C:/` is `C:/`, which exists, so the
  loop ends).

---

_Fixed: 2026-10-03T00:00:00Z_
_Fixer: Claude (gsd-code-fixer)_
_Iteration: 1_
