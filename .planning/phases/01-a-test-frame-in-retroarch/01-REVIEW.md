---
phase: 01-a-test-frame-in-retroarch
reviewed: 2026-10-03T01:17:02Z
depth: standard
files_reviewed: 66
files_reviewed_list:
  - include/nesturbator.h
  - src/frame.c
  - src/instance.c
  - src/internal.h
  - src/palette.c
  - src/testcard.c
  - host/convert.c
  - host/convert.h
  - runner/main.c
  - runner/ppm.c
  - runner/ppm.h
  - runner/sha256.c
  - runner/sha256.h
  - libretro/libretro.c
  - libretro/CMakeLists.txt
  - libretro/nesturbator_libretro.info
  - tools/palgen/palgen.c
  - tools/palgen/CMakeLists.txt
  - CMakeLists.txt
  - CMakePresets.json
  - runner/CMakeLists.txt
  - cmake/packaging.cmake
  - cmake/nesturbatorConfig.cmake.in
  - cmake/nesturbator.pc.in
  - scripts/hygiene.sh
  - .github/workflows/ci.yml
  - .github/workflows/release.yml
  - .github/rulesets/main.json
  - .github/dependabot.yml
  - release-please-config.json
  - tests/CMakeLists.txt
  - tests/check.h
  - tests/core/test_api.c
  - tests/core/test_frame.c
  - tests/core/test_palette.c
  - tests/host/test_convert.c
  - tests/runner/test_sha256.c
  - tests/libretro/libretro_host.c
  - tests/consumer/main.c
  - tests/consumer/CMakeLists.txt
  - tests/header/header_c.c
  - tests/header/header_cxx.cpp
  - tests/abi/CMakeLists.txt
  - tests/abi/float_fixture.c
  - tests/hygiene/CMakeLists.txt
  - tests/retroarch/CMakeLists.txt
  - tests/retroarch/bmp_ppm.c
  - tests/retroarch/bmp_ppm.h
  - tests/retroarch/compare_frame.c
  - tests/retroarch/test_compare_frame.c
  - tests/retroarch/run_retroarch.cmake
  - tests/retroarch/test.cfg.in
  - tests/cmake/action_pins.cmake
  - tests/cmake/check_archives.cmake
  - tests/cmake/check_install_line.cmake
  - tests/cmake/check_ppm.cmake
  - tests/cmake/expect_output.cmake
  - tests/cmake/float_fixture.cmake
  - tests/cmake/float_scan.cmake
  - tests/cmake/format_check.cmake
  - tests/cmake/global_symbols.cmake
  - tests/cmake/palette_regen.cmake
  - tests/cmake/undefined_symbols.cmake
  - tests/cmake/vendored_sha256.cmake
  - tests/cmake/version_consistency.cmake
  - tests/cmake/write_hashes.cmake
findings:
  critical: 2
  warning: 9
  info: 10
  total: 21
status: issues_found
---

# Phase 1: Code Review Report

**Reviewed:** 2026-10-03T01:17:02Z
**Depth:** standard
**Files Reviewed:** 66
**Status:** issues_found

## Narrative Findings (AI reviewer)

## Summary

I reviewed the core (instance, frame, test card, palette), the shared host conversion, the runner (options, SHA-256, PPM), the libretro adapter, palgen, the CMake build, packaging and presets, the hygiene script, the CI and release workflows, and the test programs and CMake test scripts.

The core holds up. The size-tag check, the order of the run_frame checks, the carried audio remainder, the tick and sample arithmetic (714732 ticks per frame and 352 samples per 315000 ticks, both exact) and the test card all trace correctly. The SHA-256 padding and compression match FIPS 180-4, and the BMP and P6 readers check bounds properly.

The serious defects are in `scripts/hygiene.sh`, the public-repository privacy gate (CLAUDE.md rule 3). I reproduced two bypasses in a scratch repository:

- A tracked file with a non-ASCII name is never scanned.
- A text file that is not valid UTF-8 is never scanned under a UTF-8 locale.

In both cases a home path and an email address passed with no finding.

Other issues:
- A runner loop never ends at the largest allowed frame count.
- The runner exits 0 when `--frames` is left out, although the README says the option is required.
- The size-tag forward-compatibility rule depends on padding bytes the caller cannot control.
- Neither the allocator alignment contract nor the zeroing rule for larger structs is written down.
- Configuring the project from another CMake project (add_subdirectory or FetchContent) fails on any OS or CPU other than the six release targets.
- Several checks are weaker than their comments say: the float scan, commit-message hygiene, the `release-as` pin, and CI robustness.

## Critical Issues

### CR-01: hygiene.sh never scans tracked files whose names contain non-ASCII bytes

**File:** `scripts/hygiene.sh:94`, `scripts/hygiene.sh:100`, `scripts/hygiene.sh:41-53`, `scripts/hygiene.sh:86-88`
**Issue:** `git diff --cached --name-only` and `git ls-files` C-quote any path with bytes above 0x7F (`core.quotePath` defaults to true). For example, `café.txt` is listed as `"caf\303\251.txt"`. `check_file` then gets a path that does not exist. `tree_show` and `staged_show` both run `git show ":<quoted>"`, which fails, and every content check sends that failure to `2>/dev/null`. The content is empty, so no rule fires.

Reproduced: a scratch repo with `café.txt` holding a macOS home path and a webmail address. `scripts/hygiene.sh --tree` reported only the ASCII-named control file. Any file with a non-ASCII name can therefore carry personal paths, email addresses or ROM bytes through both the pre-commit hook and the CI `hygiene.tree` test. The script also fails open: a file whose content cannot be read counts as clean.
**Fix:** Turn path quoting off, and fail closed when content cannot be read:
```sh
git() { command git -c core.quotePath=false "$@"; }   # near the top
...
check_file() {
  [ "$1" = "$SELF" ] && return 0
  if ! $2 "$1" >/dev/null 2>&1; then found 'cannot read file' "$1"; return 0; fi
  ...
}
```
Better still, read NUL-separated lists (`git ls-files -z`, `git diff --cached --name-only -z`) so that names containing newlines are handled too.

### CR-02: Under a UTF-8 locale, `grep -I` skips non-UTF-8 text, so the home-path and email scans miss it

**File:** `scripts/hygiene.sh:30`, `scripts/hygiene.sh:52-53`
**Issue:** `leaks()` runs `grep -I -E` in the caller's locale. Under a UTF-8 locale (macOS defaults, and `C.UTF-8` on GitHub's ubuntu runners), both GNU and BSD grep treat bytes that are not valid UTF-8 as binary. `-I` then reports no match. The binary-file rule on line 49 does use `LC_ALL=C`, so the same file counts as text there. Neither rule fires.

Reproduced with `/usr/bin/grep`: `printf 'caf\351 <home path>\n' | LC_ALL=en_US.UTF-8 grep -I -E <home-path pattern>` exits 1, while the same command with `LC_ALL=C` matches. `hygiene.sh --tree` on a Latin-1 file holding a home path reported nothing. Any Latin-1, Windows-1252 or Shift-JIS text file (or a UTF-16 file without NULs) gets past the privacy scan.
**Fix:** Pin the locale for the whole script so that every grep decides text versus binary on NUL bytes only:
```sh
set -eu
LC_ALL=C
export LC_ALL
```

## Warnings

### WR-01: `--frames 4294967295` makes the runner loop forever

**File:** `runner/main.c:213`
**Issue:** `parse_count_n` accepts values up to 4294967295, and the loop is `for (uint32_t f = 1; f <= opt.frames && status == 0; f++)`. When `opt.frames == UINT32_MAX`, `f <= opt.frames` is always true. After the last frame, `f++` wraps to 0 and the loop goes on forever (and later re-hashes and re-dumps frames when `f` wraps back to listed values). The documented contract, run N frames and exit 0, is broken at the top of the accepted range.
**Fix:** Count with a 64-bit variable or check the bound before incrementing:
```c
for (uint64_t f = 1; f <= opt.frames && status == 0; f++) { ... (uint32_t)f ... }
```

### WR-02: The runner exits 0 and does nothing when `--frames` is missing; the README says it is required

**File:** `runner/main.c:3`, `runner/main.c:36`, `runner/main.c:126-172`
**Issue:** `opt.frames` starts at 0, and nothing requires `--frames`. Running `nesturbator-run` with no arguments exits 0 with no output (verified with `build/ci/runner/nesturbator-run`). README.md:213 documents `nesturbator-run --frames N [...]` with `--frames` required and N >= 1. The usage text shows `[--frames N]`. A script that leaves out `--frames` because of a typo gets a silent success instead of the usage exit 2.
**Fix:** Once parsing finishes, add `if (o->frames == 0u) return usage("--frames N is required");` and drop the brackets in the usage text and the file comment.

### WR-03: The size-tag rule rejects larger structs whose padding bytes are not zero, and callers cannot control those bytes

**File:** `include/nesturbator.h:11-16`, `src/instance.c:15-21`
**Issue:** A struct larger than the library's is accepted only if every byte past the library's `sizeof` is zero. When a later release appends a field, the bytes past the old size include the new struct's tail padding (and any interior padding before the new field). For example, appending one `uint32_t` to `nesturbator_config` on LP64 makes a 40-byte struct: 36 bytes of fields and 4 bytes of padding. In C, an automatic struct initialised with a brace list, as `NESTURBATOR_CONFIG_INIT` is, does not guarantee zero padding. A host built against the newer header and running on the older library can then get a random `NESTURBATOR_ERR_STRUCT_SIZE`, depending on stack contents. The header promises this convention will not change ("fixed; later releases only append"), so the gap needs closing before the first release.
**Fix:** Pick one and document it in the header:
- Require callers to `memset` boundary structs to zero before filling them. Say so in the conventions block and in `NESTURBATOR_CONFIG_INIT`'s comment, and use `memset` in the runner, the adapter and the tests (several already do).
- Or commit to appending only fields that leave no padding, for example in pairs of `uint32_t` or as `uint64_t`, with a `_Static_assert` per struct that `sizeof` equals the sum of its fields.

### WR-04: The allocator contract states no alignment requirement, but arena allocators are explicitly invited

**File:** `include/nesturbator.h:79-88`, `src/instance.c:99-104`
**Issue:** The header says `free` receives the size "so arena allocators work". It never says what alignment `alloc` must return. `struct nesturbator` holds `uint64_t` counters and function pointers. A bump allocator that returns 4-byte-aligned memory, common in arenas, causes undefined behaviour: misaligned 64-bit accesses, which fault on some ARM targets and get reported by UBSan.
**Fix:** Document the requirement, for example: "alloc must return memory aligned as malloc's (suitable for any object type, `_Alignof(max_align_t)`)". Optionally have the core reject a misaligned pointer by freeing it and returning `NESTURBATOR_ERR_NO_MEMORY`.

### WR-05: Configuring the project from another CMake project fails on any OS or CPU outside the six release targets

**File:** `cmake/packaging.cmake:5-31`, `CMakeLists.txt:5`, `CMakeLists.txt:95-99`, `CMakeLists.txt:124`
**Issue:** The project calls itself "a small C library that any host can embed", but its top-level CMakeLists has no `PROJECT_IS_TOP_LEVEL` guards:
- It always enables CXX, so a C-only host toolchain must provide a C++ compiler.
- It always adds the runner, the libretro module, palgen and every test, plus their install rules.
- It always runs `include(CPack)`, which overwrites the parent project's CPack configuration.
- `packaging.cmake` raises `FATAL_ERROR` for any `CMAKE_SYSTEM_NAME` other than Linux, Darwin or Windows, and for any processor other than x64 or arm64.

So `add_subdirectory(nesturbator)` or `FetchContent` fails to configure on FreeBSD, Android, iOS, armv7, i686 and riscv64, and a top-level build on those platforms fails too.
**Fix:** Wrap the parts only the top-level project needs:
```cmake
project(nesturbator VERSION ${NESTURBATOR_VERSION} LANGUAGES C)
if(PROJECT_IS_TOP_LEVEL)
  enable_language(CXX)
  enable_testing()
  add_subdirectory(runner) ... add_subdirectory(tests)
  include(cmake/packaging.cmake)
endif()
```
In `packaging.cmake`, fall back to `${CMAKE_SYSTEM_NAME}`/`${CMAKE_SYSTEM_PROCESSOR}` (lower-cased) with a `message(WARNING ...)` instead of `FATAL_ERROR`.

### WR-06: `--history` checks commit identities but never checks commit message bodies

**File:** `scripts/hygiene.sh:103-121`
**Issue:** Rule 3 forbids personal paths, email addresses and names anywhere in the public repo. History mode scans tree contents, binary files, ROM names and author/committer emails, but not commit messages. Squash merges copy the PR title and body into the commit message on `main`. A pasted log line with a home directory path or a `Co-authored-by:` trailer with a personal address goes out with every push.
**Fix:** Add a message scan inside the history loop:
```sh
if git log -1 --format=%B "$c" | leaks "$HOME_PATH" "$HOME_OK"; then found 'home directory path' "message of $c"; fi
if git log -1 --format=%B "$c" | leaks "$EMAIL" "$EMAIL_OK"; then found 'email address' "message of $c"; fi
```
Add `@anthropic\.com` (or whichever trailer addresses you choose) to `EMAIL_OK` only on purpose.

### WR-07: The float scan misses hex floating literals

**File:** `tests/cmake/float_scan.cmake:29-33`
**Issue:** The header comment says the scan exists because "constant folding leaves no trace" of floating literals. It removes `0[xX][0-9a-fA-F]+` before matching, so `0x1.8p1` becomes `.8p1` and `0x1p-3` becomes `p-3`. Neither matches any of the three decimal patterns. A line such as `return (uint32_t)(0x1.8p1 * 2);` folds to an integer, passes `-mgeneral-regs-only`, passes the symbol checks, and passes this scan. That is exactly the case the scan was written to catch.
**Fix:** Before removing hex integers, match hex floats: `0[xX][0-9a-fA-F]*\.?[0-9a-fA-F]*[pP][+-]?[0-9]`. Add a `0x1p3` line to the SELFTEST sample.

### WR-08: The one-time `release-as: "0.1.0"` pin has only a manual todo to remove it, and nothing checks it

**File:** `release-please-config.json:10`
**Issue:** While `release-as` remains, release-please proposes 0.1.0 for every release PR. After v0.1.0 ships, the next releasable merge either opens a 0.1.0 PR again or fails on the existing tag. The only safeguard is a Pending Todo in STATE.md. Per CLAUDE.md rule 6, "Checks are automated; none waits on a person".
**Fix:** Add a CTest that fails when `.release-please-manifest.json` is at or above the pinned version while the pin is still present. In CMake: `string(JSON ...)` reads both files, then compare with `VERSION_GREATER_EQUAL`. The release PR that publishes 0.1.0 then turns CI red until the pin is removed.

### WR-09: CI installs Ninja without `apt-get update`, and its one-minute timeouts sit on the only required check

**File:** `.github/workflows/ci.yml:91`, `.github/workflows/ci.yml:147`, `.github/workflows/ci.yml:33,43,69,122,134,155,196`
**Issue:** `sudo apt-get install -y ninja-build` without `sudo apt-get update` fails with "Unable to locate package" or a 404 whenever the runner image's package lists are stale, which GitHub documents as a known cause of failures. Timeouts are also set to twice one cold run and rounded up, giving 1 minute for hygiene (a full configure and build), nofp, title, hash-equality and CI required, and 2 minutes for the full six-platform build, test and package. Ordinary runner slowness (queue start-up, a slow apt mirror, a cold Windows ARM disk) cancels a job. That fails `CI required`, which blocks every merge and the release publish.
**Fix:** Run `sudo apt-get update && sudo apt-get install -y ninja-build`. Set timeouts with real headroom, for example 5 to 10 times the measured run or at least 10 minutes. Their purpose is to stop a hung job, not to measure speed.

## Info

### IN-01: The test-card comment says every native value has a 16x7 cell; the border trims two columns of cells

**File:** `src/testcard.c:13-16`, `src/testcard.c:25-28`
**Issue:** Rule 2 (the border) runs before rule 4 and overwrites x=0 and x=255 on chart rows. Cells with c=0 and c=15 are therefore 15x7. The comment "Each of the 512 native values appears in its own 16x7 cell" is wrong. The hashes are unaffected.
**Fix:** Change the comment to "16x7 cell (15x7 for columns 0 and 15, whose outer column is border)".

### IN-02: The header suggests `audio` may be NULL, but a NULL buffer always fails

**File:** `include/nesturbator.h:129-131`, `src/frame.c:28-33`
**Issue:** "may be NULL only when audio_capacity is 0" suggests a video-only host can pass no audio buffer. Every frame needs at least 798 samples, so capacity 0 always gives `NESTURBATOR_ERR_BUFFER_TOO_SMALL`. `test_frame.c:86-89` pins this behaviour. The documentation offers an option that cannot succeed.
**Fix:** Write "audio must hold at least 799 samples; there is no video-only mode" or similar, or add a real way to discard audio.

### IN-03: `a.free(...)` breaks if `<stdlib.h>` defines `free` as a function-like macro

**File:** `src/instance.c:115`
**Issue:** C17 7.1.4p1 lets a library function also be defined as a function-like macro, and debug allocators do this (for example MSVC with `_CRTDBG_MAP_ALLOC`). `instance.c` includes `<stdlib.h>`, so `a.free(a.user, inst, sizeof *inst)` then expands as a macro call with three arguments and fails to compile.
**Fix:** `(a.free)(a.user, inst, sizeof *inst);`. Optionally mention the same idiom in the header comment for hosts.

### IN-04: A failed `--dump-frame` stops the run, drops later hash lines and leaves a partial file

**File:** `runner/main.c:234-243`, `runner/ppm.c:14-35`
**Issue:** On a write failure, `status = 1` ends the loop, so a `--hash-frame` for a later frame is never printed (verified: `--frames 2 --dump-frame 1:<missing dir>/x.ppm --hash-frame 2` prints no hash). If the failure happens after `fopen`, the truncated PPM is left on disk. The exit status is 1, which is correct, but the partial outputs are surprising.
**Fix:** `remove(path)` on failure in `nesturbator_run_write_ppm`. Either keep running the remaining frames and still exit 1, or document that the first error stops the run.

### IN-05: With `argc == 0` the runner reports "out of memory"

**File:** `runner/main.c:128-135`
**Issue:** `malloc(0)` may return NULL, and POSIX `execve` allows an empty argv. The runner then prints "out of memory" and exits 1 instead of 2 (usage).
**Fix:** Allocate `n_args + 1` entries, or return `usage(...)` when `argc < 1`.

### IN-06: palgen's MSVC flag does not forbid contraction on older toolsets

**File:** `tools/palgen/CMakeLists.txt:7`
**Issue:** `/fp:precise` stopped emitting contractions by default only in VS 2022 17.0. Earlier toolsets may fuse a*b+c, so a CI image update or an older local MSVC would fail `palette.regen` for a reason that is hard to see. The regen test catches it, so this is robustness only.
**Fix:** Use `/fp:strict` (no contraction on any version), or add `/fp:contract-` where it is supported.

### IN-07: The BMP reader reads BI_BITFIELDS masks at offset 54 even when pixel data starts there

**File:** `tests/retroarch/bmp_ppm.c:143-151`, `tests/retroarch/bmp_ppm.c:131`
**Issue:** With `hsize == 40` and `comp == 3`, the masks must follow the header (bytes 54-65), so `offset` must be at least 66. The check only requires `offset >= 54`. A malformed file can then have pixel bytes read as masks. These are in-bounds reads in test code, but the file is accepted when it should get code 2.
**Fix:** `if (hsize == 40u && offset < 66u) return 2;` inside the BI_BITFIELDS branch.

### IN-08: `--history` splits binary and ROM path names on whitespace

**File:** `scripts/hygiene.sh:113-120`
**Issue:** `for b in $(git diff-tree ... )` and `for n in $(git log --name-only ...)` split on spaces. A manifest-listed binary whose path contains a space is reported as fragments that are not in the manifest: a false failure with confusing output.
**Fix:** Read with `while IFS= read -r b; do ...; done <<EOF` (or pipe to `while read`), as `check_files` already does with `IFS=newline`.

### IN-09: `nesturbator_get_info` needs an instance only to return constants

**File:** `src/instance.c:49-53`, `libretro/libretro.c:122-143`
**Issue:** The output depends on nothing in the instance, yet a NULL `inst` writes nothing. The adapter then reports fps 0 and sample rate 0 if a frontend asks for AV info before `retro_load_game` or after unload. RetroArch does not, but other hosts might.
**Fix:** Document this as intended (per-instance region later), or in the adapter fill timing from a temporary query on a fresh instance or from constants when `inst == NULL`.

### IN-10: The no-global-state and allowed-symbol checks run only on Linux and miss some nm symbol types

**File:** `tests/abi/CMakeLists.txt:1-39`, `tests/cmake/global_symbols.cmake:23`, `.github/workflows/ci.yml:132-148`
**Issue:** `abi.*` tests are registered only with `NESTURBATOR_NOFP`, which CI runs only on the two ubuntu runners. The macOS and MSVC builds of the core are never checked for writable globals or unexpected imports (for example MSVC's `__security_cookie` or `_fltused`). `global_symbols.cmake` also matches only `[BbDdC]`. It misses `V`/`v` (weak objects), `G`/`g` (small data on some ELF targets), `u` (GNU unique) and Mach-O `S`/`s` for writable data in other sections.
**Fix:** Run `undefined_symbols` and `global_symbols` (without `-mgeneral-regs-only`) in the `ci` preset on every platform, using `dumpbin /symbols` on MSVC. Widen the type class to `[BbDdCGgSsVvu]` and filter read-only sections explicitly.

---

_Reviewed: 2026-10-03T01:17:02Z_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_
