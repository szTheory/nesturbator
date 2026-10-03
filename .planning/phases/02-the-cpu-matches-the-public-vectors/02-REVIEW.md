---
phase: 02-the-cpu-matches-the-public-vectors
reviewed: 2026-10-03T00:00:00Z
depth: standard
files_reviewed: 40
files_reviewed_list:
  - .github/workflows/ci.yml
  - .github/workflows/nightly.yml
  - CMakeLists.txt
  - CMakePresets.json
  - release-please-config.json
  - .gitattributes
  - PROVENANCE.md
  - README.md
  - THIRD-PARTY-NOTICES.md
  - include/nesturbator.h
  - src/bus.c
  - src/cpu.c
  - src/instance.c
  - src/internal.h
  - tests/CMakeLists.txt
  - tests/cmake/fetch_vectors.cmake
  - tests/cmake/manifest_sha256.cmake
  - tests/cmake/pins_check.cmake
  - tests/cmake/release_config.cmake
  - tests/cmake/vecconv_negative.cmake
  - tests/cmake/vectors_fixture.cmake
  - tests/cmake/vectors_full_run.cmake
  - tests/cmake/vectors_regen.cmake
  - tests/cmake/vectors_sample_match.cmake
  - tests/core/test_api.c
  - tests/core/test_profile.c
  - tests/cpu/test_bus.c
  - tests/cpu/test_cpu_unit.c
  - tests/cpu/test_vectors.c
  - tests/cpu/vector_bus.c
  - tests/cpu/vector_bus.h
  - tests/embed/CMakeLists.txt
  - tests/roms/manifest.txt
  - tests/vectors/fixtures/README.md
  - tests/vectors/n65v.c
  - tests/vectors/n65v.h
  - tests/vectors/test_n65v.c
  - tools/vecconv/CMakeLists.txt
  - tools/vecconv/vecconv.c
findings:
  critical: 1
  warning: 3
  info: 6
  total: 10
status: issues_found
---

# Phase 2: Code Review Report

**Reviewed:** 2026-10-03T00:00:00Z
**Depth:** standard
**Files Reviewed:** 40
**Status:** issues_found

## Summary

The review covered the 6502 core (`src/cpu.c`), the library bus, the N65V
reader, the `vecconv` JSON tokenizer, the vector harnesses, the CMake test
scripts and the CI/nightly workflows. It focused on findings the 2.56M upstream
vectors cannot catch.

**src/cpu.c is clean on the points asked about.** All 256 opcodes appear
exactly once in the switch. Every shift is done on a promoted `int` or
`uint32_t` and then narrowed, so no shift can overflow or go negative. No full
expression has two bus calls with an unspecified order. The ALR, ANE, LXA and
LAS lines (1249, 1265, 1268, 1273) mix one `fetch`/`read_at` with reads of
`a`, `x` and `s`, and the bus functions never modify those registers. The core
uses no floating point and no mutable static. `cpu.c` and `bus.c` call nothing
outside the core, so the "C memory functions only" rule holds.

**The N65V reader is sound.** Every read checks `n > len - off`, and that
check cannot underflow because `off <= len` is always true. The `ram_count`
and `cycle_count` fields are `u8`, so the 255-entry arrays cannot overflow.

**The `vecconv` tokenizer is sound.** `parse_uint` stops growing its value at
65536, so it cannot overflow. `parse_string` writes the key's terminator
inside bounds for every length; this was checked at 14, 15, 16 and 17 bytes.
List lengths are checked before the entry is written.

**The real defects are in the test and CI tooling:**
- A user-configurable recursive delete in `fetch_vectors.cmake` whose
  in-source guard can be bypassed (data loss, including the repository's own
  `src/`).
- A nightly path filter that misses most of the nightly's inputs.
- A nightly timeout that leaves no room for network variance.
- A harness cleanup that lets one failing test corrupt later ones.

## Critical Issues

### CR-01: The in-source guard in fetch_vectors.cmake is lexical, so `file(REMOVE_RECURSE)` can delete the repository's `src/` or any `<dir>/src`

**File:** `tests/cmake/fetch_vectors.cmake:40-47, 136-137` (and `tests/CMakeLists.txt:350-351`)
**Issue:** The script deletes `${DIR}/src` recursively whenever the 256 files
do not verify. On a first run nothing verifies, so the delete always runs.
`DIR` comes from the user-settable cache variable `NESTURBATOR_VECTORS_DIR`,
and the README tells users to point it "somewhere that survives a clean
build".

The only protection is `cmake_path(IS_PREFIX ...)`. That command is purely
lexical: it does not resolve symlinks, and it compares case-sensitively. Two
consequences:

- **The guard can be bypassed.** On macOS (APFS, case-insensitive by default)
  or Windows, `-DNESTURBATOR_VECTORS_DIR=/Users/<u>/projects/Nesturbator`
  (capital N) or `c:/...` instead of `C:/...` passes the guard. The script
  then runs `file(REMOVE_RECURSE "<repo>/src")`, which deletes the core's
  source tree. A symlinked path to the checkout does the same.
- **Outside the tree there is no check at all.** A user who sets the variable
  to `$HOME` or to an existing workspace loses `$HOME/src` or
  `<workspace>/src` with no prompt.

`WRITE_PINS` mode runs the same delete.

**Fix:** Delete only a directory this script created and marked, and resolve
paths before comparing them:
```cmake
file(REAL_PATH "${DIR}" dir_real)           # resolves symlinks
file(REAL_PATH "${SOURCE_DIR}" src_real)
if(WIN32 OR APPLE)                          # case-insensitive filesystems
  string(TOLOWER "${dir_real}" dir_cmp)
  string(TOLOWER "${src_real}" src_cmp)
else()
  set(dir_cmp "${dir_real}")
  set(src_cmp "${src_real}")
endif()
# ... IS_PREFIX on dir_cmp/src_cmp as now ...
set(src "${dir_real}/65x02-src")            # a name no user directory has
set(marker "${src}/.nesturbator-vectors")
if(EXISTS "${src}" AND NOT EXISTS "${marker}")
  message(FATAL_ERROR "fetch_vectors: ${src} exists and was not created by this script; refusing to delete it")
endif()
file(REMOVE_RECURSE "${src}")
file(MAKE_DIRECTORY "${src}")
file(TOUCH "${marker}")
```
Update `vectors_full_files` in `tests/CMakeLists.txt` and the `DIR/src/...`
path in `vectors_sample_match.cmake` to match.

## Warnings

### WR-01: The nightly's pull_request path filter misses most of the lane's inputs

**File:** `.github/workflows/nightly.yml:14-18`
**Issue:** The workflow runs on a pull request only when the PR touches
`nightly.yml`, `tests/vectors/**` or `tests/cmake/fetch_vectors.cmake`. The
`vectors-full` lane also depends on:
- `tests/cmake/vectors_full_run.cmake`
- `tests/cmake/vectors_sample_match.cmake`
- `tools/vecconv/**`
- `tests/cpu/**`
- the `NESTURBATOR_VECTORS_FULL` block of `tests/CMakeLists.txt`
- the `vectors-full` presets in `CMakePresets.json`
- `src/cpu.c`, whose 10000-test behaviour only this lane exercises

This branch already shows the gap. Commit 1d9c997 ("set policies in the
vectors-full scripts for CMake 3.31") fixed a nightly failure (run
37135715964) in scripts that this filter does not cover, so the PR that broke
them could not have run the nightly. A regression in any of these files
reaches `main` green and is reported only by the next scheduled run.

**Fix:** List the real inputs:
```yaml
  pull_request:
    paths:
      - .github/workflows/nightly.yml
      - CMakePresets.json
      - tests/CMakeLists.txt
      - tests/vectors/**
      - tests/cpu/**
      - tests/cmake/fetch_vectors.cmake
      - tests/cmake/vectors_full_run.cmake
      - tests/cmake/vectors_sample_match.cmake
      - tools/vecconv/**
      - src/cpu.c
```

### WR-02: The nightly's 3-minute job timeout leaves no room for network variance and overrides the fetch's own 900 s timeout

**File:** `.github/workflows/nightly.yml:25`, `tests/CMakeLists.txt:362`, `tests/cmake/fetch_vectors.cmake:117-119`
**Issue:** `timeout-minutes: 3` was set from one cold run of 88 s. That run
covers, in one job:
- the toolchain step,
- a full `ci` configure and build (runner, libretro, C++ tests, packaging
  targets),
- a roughly 190 MB git fetch from GitHub,
- 2.56M vectors.

Doubling the measured time is a reasonable rule for CPU-bound jobs. The
largest piece of this job is a network transfer, though, and its duration
varies widely. The settings also contradict each other: the fetch test sets
`TIMEOUT 900`, and git's low-speed abort allows 60 s of stall, but the job
cancels at 180 s.

A slow fetch therefore turns the run `cancelled`. The report job then opens
or updates the `nightly` issue with "no test recorded". That is a false-red
alarm, which teaches people to ignore the issue.

**Fix:** Give the network part its own allowance, for example
`timeout-minutes: 15`, or measured build/test time × 2 plus the fetch's
ctest `TIMEOUT`. Alternatively, run the fetch as its own step with a separate
`timeout-minutes`. Either way, make the ctest and job timeouts agree.

### WR-03: The cpu.vectors cleanup misses stray writes, so one failing test can corrupt every later test in the run

**File:** `tests/cpu/test_vectors.c:187-197, 236`
**Issue:** `clear_ram()` zeroes only the addresses named in the current
test's `initial` and `final` RAM lists. The comment says "The cycle compare
shows that no other address was written", but that holds only when the test
passed.

When a CPU bug writes to an address the test does not list, two things
happen:
1. `run_test` reports that test as failed, which is correct.
2. The stray byte stays in `machine.ram`.

Any later test that reads that address without listing it then sees a
non-zero value. The run reports inflated "N of 10000 vectors failed" counts
and blames the wrong tests, which is exactly when a correct report is needed
for debugging. The 10000-test nightly runs are the most exposed.

**Fix:** Also clear every address the CPU actually wrote, from the log:
```c
static void clear_ram(void)
{
    for (uint32_t i = 0u; i < test.initial.ram_count; i++) {
        machine.ram[test.initial.ram_addr[i]] = 0u;
    }
    for (uint32_t i = 0u; i < test.final.ram_count; i++) {
        machine.ram[test.final.ram_addr[i]] = 0u;
    }
    for (uint32_t i = 0u; i < machine.log_count; i++) {
        if (machine.log[i].kind == N65V_KIND_WRITE) {
            machine.ram[machine.log[i].addr] = 0u;
        }
    }
}
```
If the log overflowed, `memset(machine.ram, 0, sizeof machine.ram)` after the
failure is the simple fallback.

## Info

### IN-01: A new instance's CPU state is all zeros, and "only reset clears it" has no reset to point to

**File:** `src/instance.c:103`, `src/internal.h:30-38`, `src/cpu.c:444-460, 488-491`
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

**File:** `CMakeLists.txt:24-48`
**Issue:** The following are created unconditionally, so a project that uses
`add_subdirectory` or FetchContent gets them in its own namespace:
- the target `nesturbator_cpu`,
- the global CMake functions `nesturbator_core_flags` and
  `nesturbator_warnings`,
- the option `NESTURBATOR_NOFP`.

The `nesturbator_` prefix makes clashes unlikely. However,
`tests/embed/CMakeLists.txt:17` checks only for runner, libretro, palgen,
vecconv and host targets, so the extra target is not documented or tested.
**Fix:** Add `nesturbator_cpu` to the embed test's expected targets, or
mention it in the README's embedding paragraph.

### IN-04: Script-mode CMake files set policies inconsistently

**File:** `tests/cmake/pins_check.cmake`, `manifest_sha256.cmake`,
`release_config.cmake`, `vecconv_negative.cmake`, `vectors_fixture.cmake`
**Issue:** Commit 1d9c997 added `cmake_minimum_required(VERSION 3.25)` to the
vectors-full scripts after CMake 3.31 read `IN_LIST` with its pre-3.3 meaning.
The scripts listed above, which run in `ci`, still have no policy settings.
None of them uses `IN_LIST` today, so nothing breaks yet. The next edit that
adds one will break only on runners whose CMake defaults differ.
**Fix:** Add `cmake_minimum_required(VERSION 3.25)` at the top of every `-P`
script.

### IN-05: vecconv accepts leading zeros, which JSON forbids

**File:** `tools/vecconv/vecconv.c:206-232`
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

**File:** `tests/cpu/test_vectors.c:31-95`, `tests/vectors/test_n65v.c:31-61`, `tools/vecconv/vecconv.c:106-124, 565-574`
**Issue:**
- `read_file` exists three times, in two different implementations.
- `hex_digit` exists twice.

A fix to one copy, such as a size cap or error handling, will not reach the
others.
**Fix:** Move `read_file` and `hex_digit` into `tests/vectors/n65v.c` (or a
small `vecio.c`) and share them, as the reader already is.

---

_Reviewed: 2026-10-03T00:00:00Z_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_
