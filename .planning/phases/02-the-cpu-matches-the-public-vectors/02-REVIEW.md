---
phase: 02-the-cpu-matches-the-public-vectors
reviewed: 2026-10-03T00:00:00Z
depth: standard
files_reviewed: 8
files_reviewed_list:
  - .github/workflows/nightly.yml
  - README.md
  - tests/CMakeLists.txt
  - tests/cmake/fetch_guard.cmake
  - tests/cmake/fetch_vectors.cmake
  - tests/cmake/vectors_sample_match.cmake
  - tests/cmake/vectors_stray_write.cmake
  - tests/cpu/test_vectors.c
findings:
  critical: 0
  warning: 1
  info: 9
  total: 10
status: issues_found
---

# Phase 2: Code Review Report

**Reviewed:** 2026-10-03T00:00:00Z
**Depth:** standard
**Files Reviewed:** 8
**Status:** issues_found

## Summary

This is the re-review after the fix pass for CR-01, WR-01, WR-02 and WR-03
(commits 2a4ea01, bae80dc, 040ca2d, f553966, diff base 809e96f). The Info
findings IN-01 to IN-06 from the first review are open in
`02-REVIEW-DISPOSITION.md`. Each was checked again against the current source
and all six still apply, so they are carried forward below with their IDs.
Line numbers were updated where they moved. New findings are numbered from
WR-04 and IN-07.

**The fixes:**

- **CR-01 (fixed, correct).** The delete now targets only `DIR/65x02-src`, and
  only when that directory holds the marker `.nesturbator-vectors`. A
  non-marked `65x02-src` makes the script stop before any delete. A
  `65x02-src` that is itself a symlink is removed as a link, not followed.
  The in-source check runs again on `file(REAL_PATH)` results, so a symlink
  or a different letter case no longer gets past it. The marker is what
  actually stops the data loss: even if the in-source check is bypassed (for
  example on a case-insensitive mount under Linux or WSL, where the
  lower-casing does not run), the worst outcome is fetched files written into
  the tree, never a deleted user directory. No path still refers to the old
  `DIR/src`.
  `vectors.fetch.guard` would fail against the old script, because case 1
  expects a refusal message the old script never printed. Two side issues
  remain: IN-07 and IN-08.
- **WR-01 (fixed, incomplete).** The fix adds `src/internal.h` because
  `cpu.c` includes it, but it leaves out what `internal.h` includes in turn
  (`include/nesturbator.h`). It also leaves out the root `CMakeLists.txt`,
  which defines `nesturbator_cpu` and its compile flags. See WR-04.
- **WR-02 (fixed, correct).** `timeout-minutes: 16` (960 s) covers the ctest
  `TIMEOUT 900` of the fetch plus twice the measured 23 s of everything else.
  The workflow comment and the comment in `tests/CMakeLists.txt` tie the two
  values together.
- **WR-03 (fixed, correct).** `clear_ram(passed)` zeroes all 64 KiB after a
  failed test. That also covers a failure caused by log overflow. On a
  passing test the cycle compare limits writes to the logged cycles, so the
  listed-address clear is enough there. I traced `cpu.vectors.stray-write`
  through the old harness:
  - Test 1 leaves `$10 = 1`.
  - Test 2 then reads 1 and writes 2.
  - The `ram[0x0010]` compare fails, so the run prints `2 of 2`, which does
    not match the required `1 of 2`.
  The test therefore detects a regression.

**Experiments run (scratch directory only, no source modified):**

- `file(REAL_PATH "/USERS/<u>/PROJECTS/NESTURBATOR")` on macOS returns the
  canonical-case path.
- A copy of `fetch_vectors.cmake` with the `TOLOWER` block deleted still
  passes `fetch_guard.cmake` (IN-08).
- `DIR=<symlink-to-source>/sub/deeper` is refused, but `sub/deeper` is left
  created inside the source tree (IN-07).

## Warnings

### WR-04: The nightly's path filter still misses inputs that change the CPU build the lane runs

**File:** `.github/workflows/nightly.yml:13-30`, `README.md:231-236`
**Issue:** The comment says the list is "every file the vectors-full lane
builds or runs". The fix added `src/internal.h` because `src/cpu.c` includes
it. By the same reasoning, two more files belong on the list:

- **`include/nesturbator.h`.** `src/internal.h:6` includes it, and
  `struct nesturbator` (which holds the CPU state the harness drives) is built
  on its types.
- **The root `CMakeLists.txt`.** Lines 44-45 define the `nesturbator_cpu`
  object library that `cpu.vectors` links (`tests/CMakeLists.txt:214`). Lines
  11-13 and 33-42 set its C standard, warning set and `-mgeneral-regs-only`.
  A change there changes the object that runs the 2.56M vectors.

A PR that changes either file can still merge green and break only the next
scheduled run. That is the gap WR-01 was meant to close. The README repeats
the same incomplete list.
**Fix:** Add the two paths to both places:
```yaml
      - CMakeLists.txt
      - include/nesturbator.h
```
Also change the comment so it no longer says "every file the lane builds".
The lane builds the whole `ci` tree. The list is the files that can change
what the `vectors-full` tests run.

## Info

### IN-01: A new instance's CPU state is all zeros, and "only reset clears it" has no reset to point to

**File:** `src/instance.c:103`, `src/internal.h:33-39`, `src/cpu.c:444-460, 488-491`
**Issue:**
- `nesturbator_create` zeroes the instance, so the CPU starts with
  `P = 0x00`, `S = 0x00` and `PC = 0x0000`. That is not a power-up state:
  bit 5 is clear, I is clear and there is no reset vector fetch.
- `jammed` is documented as "only reset clears it", but nothing in the
  library performs a reset.
- `halted_in_read`, `nmi_prev`, `nmi_pending`, `irq_line` and `poll_latch`
  are unused.

None of this is observable yet, because the CPU does not run during frames.
It becomes a bug as soon as `cpu_step` is wired into `nesturbator_run_frame`.
**Fix:** In the phase that drives the CPU, add the reset sequence and
initialise `S`, `P` and `PC` from it. Until then, say in `internal.h` that the
power-up state is not set.

### IN-02: `ticks` will be advanced twice once the CPU runs in frames

**File:** `src/bus.c:17-23`, `src/frame.c:48`
**Issue:** `cycle()` adds 24 to `nes->ticks` on every bus access, and
`run_frame` adds `NESTURBATOR_TICKS_PER_FRAME` to the same field. Both
paths are dormant today because no frame calls the bus. When the CPU is
driven from the frame loop, keeping both double-counts time and breaks the
frame-rate and audio-sample arithmetic.
**Fix:** When the CPU is wired in, make the frame loop run CPU steps until
`ticks` reaches the frame's end, rather than adding a fixed amount. Add a
note at `frame.c:48` now.

### IN-03: Build helpers leak into an embedding project's namespace

**File:** `CMakeLists.txt:16-45`, `tests/embed/CMakeLists.txt:17`, `README.md:293-295`
**Issue:** The following are created unconditionally, so a project that uses
`add_subdirectory` or FetchContent gets them in its own namespace:
- the target `nesturbator_cpu`,
- the global CMake functions `nesturbator_core_flags` and
  `nesturbator_warnings`,
- the option `NESTURBATOR_NOFP`.

The `nesturbator_` prefix makes clashes unlikely. However,
`tests/embed/CMakeLists.txt:17` checks only for runner, libretro, palgen,
vecconv and host targets. The README says an embedded build "adds only the
library", which is not quite true. The extra target is neither documented
nor tested.
**Fix:** Add `nesturbator_cpu` to the embed test's expected targets, or
mention it in the README's embedding paragraph.

### IN-04: Script-mode CMake files set policies inconsistently

**File:** `tests/cmake/action_pins.cmake`, `check_archives.cmake`,
`check_install_line.cmake`, `check_ppm.cmake`, `expect_output.cmake`,
`float_fixture.cmake`, `float_scan.cmake`, `format_check.cmake`,
`global_symbols.cmake`, `manifest_sha256.cmake`, `palette_regen.cmake`,
`pins_check.cmake`, `release_config.cmake`, `release_markers.cmake`,
`vecconv_negative.cmake`, `vectors_fixture.cmake`, `vendored_sha256.cmake`,
`version_consistency.cmake`
**Issue:** Commit 1d9c997 added `cmake_minimum_required(VERSION 3.25)` to the
vectors-full scripts after CMake 3.31 read `IN_LIST` with its pre-3.3
meaning. The two new scripts (`fetch_guard.cmake` and
`vectors_stray_write.cmake`) also set it. The 18 `-P` scripts listed above
still set no policy. Most of them run in `ci`. None of them uses `IN_LIST`
today (only `fetch_vectors.cmake` and `undefined_symbols.cmake` do, and both
set the minimum), so nothing breaks yet. The next edit that adds one will
break only on runners whose CMake defaults differ.
**Fix:** Add `cmake_minimum_required(VERSION 3.25)` at the top of every `-P`
script.

### IN-05: vecconv accepts leading zeros, which JSON forbids

**File:** `tools/vecconv/vecconv.c:205-232`
**Issue:** `parse_uint` accepts `007` and `00`. The header comment says the
tokenizer accepts only the vector schema's "unsigned decimal integers", but a
corrupted or hand-edited input with leading zeros converts silently instead
of failing with an offset.
**Fix:** Reject a leading `0` that is followed by another digit:
```c
if (c == '0' && p->pos + 1u < p->len && p->buf[p->pos + 1u] >= '0' && p->buf[p->pos + 1u] <= '9')
    return fail(start, "number with a leading zero");
```

### IN-06: Helpers are duplicated across the vector tools

**File:** `tests/cpu/test_vectors.c:31-43, 64-95`, `tests/vectors/test_n65v.c:30-61`, `tools/vecconv/vecconv.c:104-124, 565-574`
**Issue:**
- `read_file` exists three times, in two different implementations.
- `hex_digit` exists twice.

A fix to one copy, such as a size cap or error handling, will not reach the
others.
**Fix:** Move `read_file` and `hex_digit` into `tests/vectors/n65v.c` (or a
small `vecio.c`) and share them, as the reader already is.

### IN-07: The fetch creates DIR before its resolved-path check, so a refused DIR still leaves directories in the source tree

**File:** `tests/cmake/fetch_vectors.cmake:52-60`
**Issue:** The lexical check (line 52) passes for a path that reaches the
source tree through a symlink or a different letter case. Line 53 then runs
`file(MAKE_DIRECTORY "${dir_abs}")` before the resolved check at line 60
refuses. I reproduced this with `DIR=<link-to-source>/sub/deeper`: the script
fails as intended, but `sub/deeper` now exists inside the source tree. A
refused run should not write to the tree it refuses to touch. Git does not
track empty directories, so no hygiene check catches it.
**Fix:** Resolve the deepest existing ancestor instead of creating DIR first,
and create DIR only after both checks pass:
```cmake
set(probe "${dir_abs}")
set(tail "")
while(NOT EXISTS "${probe}")
  cmake_path(GET probe FILENAME leaf)
  set(tail "${leaf}/${tail}")
  cmake_path(GET probe PARENT_PATH probe)
endwhile()
file(REAL_PATH "${probe}" dir_real)
cmake_path(APPEND dir_real "${tail}" NORMALIZE OUTPUT_VARIABLE dir_real)
# ... TOLOWER and check_in_source as now ...
file(MAKE_DIRECTORY "${dir_abs}")
```

### IN-08: The lower-casing step is redundant, and `vectors.fetch.guard` cannot detect its removal

**File:** `tests/cmake/fetch_vectors.cmake:37-41, 56-59`, `tests/cmake/fetch_guard.cmake:13-14, 63-66`, `README.md:193-197`
**Issue:** `file(REAL_PATH)` already returns the on-disk case. On this macOS
host, `REAL_PATH` of the upper-cased checkout path returns the
canonical-case path. Windows' realpath (`GetFinalPathNameByHandle`) also
returns the canonical case. So the `string(TOLOWER)` block never changes the
result of the comparison. I deleted lines 56-59 in a scratch copy and
`fetch_guard.cmake` still printed "every guard refused". The comments and
the README say case 3 tests the lower-case comparison. It actually tests
`REAL_PATH`, so a reader may later "simplify" the wrong half.
**Fix:** Change the comments to say that `REAL_PATH` resolves symlinks and
canonical case, and either:
- drop the `TOLOWER` block, or
- keep it and label it as a fallback for file systems whose realpath does
  not canonicalise case.

### IN-09: Fetched files in the old `DIR/src` layout are orphaned with no note for users

**File:** `tests/cmake/fetch_vectors.cmake:62-68`, `README.md:246-254`
**Issue:** Before this fix, the lane stored about 1 GB in
`NESTURBATOR_VECTORS_DIR/src`. The script now uses `65x02-src` and, correctly,
never deletes the unmarked `src`. Anyone who ran the lane locally, in the
default `build/vectors-full/vectors-full` or in a persistent
`NESTURBATOR_VECTORS_DIR`, keeps the old copy indefinitely and downloads a
second one. Only `02-REVIEW-FIX.md` mentions this, and that file is not
shipped. CI is not affected, because the nightly has no cache.
**Fix:** Add a sentence to the README's vectors-full paragraph: "Files
fetched before the move to `65x02-src` remain in `<dir>/src`; delete that
directory by hand." Alternatively, have the script print a STATUS line when
`${dir_abs}/src/nes6502/v1` exists.

---

_Reviewed: 2026-10-03T00:00:00Z_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_
