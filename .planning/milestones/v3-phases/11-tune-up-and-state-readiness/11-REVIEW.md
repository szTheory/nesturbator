---
phase: 11-tune-up-and-state-readiness
reviewed: 2026-10-11T00:00:00Z
depth: standard
files_reviewed: 19
files_reviewed_list:
  - .github/workflows/ci.yml
  - README.md
  - include/nesturbator.h
  - libretro/libretro.c
  - runner/main.c
  - runner/save.c
  - runner/save.h
  - src/cartridge.c
  - src/instance.c
  - src/internal.h
  - tests/CMakeLists.txt
  - tests/cmake/runner_save.cmake
  - tests/cmake/runner_version.cmake
  - tests/core/test_boundary.c
  - tests/core/test_cartridge.c
  - tests/libretro/libretro_host.c
  - tests/retroarch/run_retroarch.cmake
  - tests/runner/test_save_fsize.c
  - tests/runner/test_save_replace.c
findings:
  critical: 0
  warning: 4
  info: 3
  total: 7
status: issues_found
---

# Phase 11: Code Review Report

**Reviewed:** 2026-10-11
**Depth:** standard
**Files Reviewed:** 19
**Status:** issues_found

## Summary

The atomic replace helper, the shared power-on helper, the single-instance libretro lifecycle, `--version` and the SAVE_SIZE validation are largely correct. I found no data-loss or memory-safety defect. The `.tmp` handling never writes through a symlink or removes a directory. The remaining problems are a documented guarantee that the code does not keep, a permanent-failure mode at the `.tmp` name, a macOS durability gap, and a test that passes vacuously. README.md, tests/libretro/libretro_host.c and tests/core/test_*.c were only skimmed; the findings below come from the source, the CI/CMake diff and the two runner tests.

## Warnings

### WR-01: save.h promises a symlink at the .tmp name is "never removed", but the helper removes it

**File:** `runner/save.h:30-31` (behaviour at `runner/save.c:118-127`)
**Issue:** `stat(tmp)` follows symlinks. When `<path>.tmp` is a symlink to a regular file, `S_ISDIR` is false, so `remove(tmp)` deletes the symlink. The victim is not touched, but the header and the test header comment ("a file the call did not create is never removed") say otherwise. The documented contract is wrong, and the same code treats any file at that name as disposable, including one the user placed there on purpose. The `.tmp` check runs before `fopen("wbx")`. A concurrent second runner on the same `--save-dir` can pass `stat`, remove the first runner's live `.tmp`, and then rename a partial file or fail. The "cleanup of only its own .tmp" claim does not hold under that race.
**Fix:** Either correct the header to say a stale regular file or symlink at `.tmp` is removed, or use `lstat` and refuse anything that is not a regular file. For the race, drop the pre-remove: on `EEXIST` from `wbx`, report failure and let the user clear the file. Alternatively, use a unique temp name (pid or counter) and accept stale files.

### WR-02: A dangling symlink at the .tmp name fails every save permanently

**File:** `runner/save.c:118-128`
**Issue:** `stat` follows the symlink and fails for a dangling one, so the stale-file branch is skipped. `fopen("wbx")` then fails with `EEXIST`, and every flush (interval and exit) returns 1 with a `strerror` message that does not name the `.tmp` path. The helper's own `.tmp` is only removed when it created it, so nothing recovers.
**Fix:** Use `lstat` on POSIX, as in WR-01. Include the temp path in the failure message so the user can see what blocks the save.

### WR-03: "sync" is not a durable flush on macOS, and the CI RetroArch job runs only on macOS

**File:** `runner/save.c:94-100`, `runner/save.h:26-34`
**Issue:** On macOS, `fsync()` pushes data to the drive but does not flush the drive's write cache. `F_FULLFSYNC` is needed for that. The header promises "flush, sync, ... never torn" and lists only the missing directory sync as a gap. On macOS, a power cut after `rename` can still leave a zero-length or torn file despite the claim.
**Fix:** Under `#ifdef __APPLE__`, call `fcntl(fileno(f), F_FULLFSYNC)` and fall back to `fsync` on failure. Otherwise state the macOS limitation in `save.h` and the README.

### WR-04: The chmod sub-case and the symlink sub-case in test_save_replace.c can pass without testing anything

**File:** `tests/runner/test_save_replace.c:176-199`
**Issue:** Case (g) skips the assertions when the process can write to a 0555 directory (root, or a container with `CAP_DAC_OVERRIDE`). It prints a note and counts as a pass, so the failure-cleanup path is not exercised in that environment. Case (h) accepts either the old or the new target and ignores the helper's return value. It would also pass if the helper wrote through the symlink to the victim, as long as the victim matches its original 64 bytes. Because `write_file(other, old_data, 64)` writes 64 bytes and the helper writes 8192 bytes, the victim assertion does detect write-through. The helper's observable outcome is untested, and the WR-01 contract goes unchecked.
**Fix:** In (g), force the failure another way when root, for example by making the directory missing or a file. In (h), assert the return value and `!path_exists(tmp)`, matching whichever behaviour WR-01 settles on.

## Info

### IN-01: nesturbator_load_cartridge duplicates the power-on reset the new helper was created to share

**File:** `src/cartridge.c:339-352`
**Issue:** The load path still carries its own copy of the memset/reset sequence for bus, apu, synth, ppu, cpu, ticks, frame_number, audio_rem and cpu_cycle, with `cpu.s` and `cpu.p` set afterwards. The helper `nesturbator__power_on_no_cartridge` also sets `ppu.scanline = 261`, which the load path does not. The two paths can now drift: the load path starts the PPU at scanline 0 and unload/create at 261. The header describes the unloaded state as "the pre-render line", so a loaded game and the test card start the PPU in different places by design or by accident. The code does not say which.
**Fix:** Call the helper from the load path, then set `cpu.s`/`cpu.p`, or add a comment explaining why the load path must not set `ppu.scanline = 261`.

### IN-02: The libretro adapter's failure path leaves inst NULL, so av info is zeroed

**File:** `libretro/libretro.c:151`, `libretro/libretro.c:89-96`
**Issue:** If `nesturbator_create` fails in `retro_init`, `retro_get_system_av_info` calls `nesturbator_get_info(NULL, ...)`. That returns without writing, so fps and sample_rate are reported as 0. The frontend normally stops at `retro_load_game` returning false, so this is harmless in practice.
**Fix:** Hard-code the NTSC fallback when `inst == NULL`, or document that the values are zero.

### IN-03: Over-long comment line in runner/main.c

**File:** `runner/main.c:26`
**Issue:** The new comment line exceeds the file's wrap width and breaks the surrounding paragraph's formatting. The "A failed write removes its .tmp" sentence is split awkwardly.
**Fix:** Re-wrap the paragraph at the file's column limit.

---

_Reviewed: 2026-10-11_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_
