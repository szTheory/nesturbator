---
phase: 10-close-out-and-boot-to-play
reviewed: 2026-10-11T00:00:00Z
depth: standard
files_reviewed: 15
files_reviewed_list:
  - include/nesturbator.h
  - src/ppu.c
  - tests/CMakeLists.txt
  - tests/cmake/hash_inventory.cmake
  - tests/cmake/hash_keys.cmake
  - tests/cmake/rom_coverage.cmake
  - tests/cmake/vector_api_policy.cmake
  - tests/cmake/write_hashes.cmake
  - tests/core/test_api.c
  - tests/core/test_nmi_edge.c
  - tests/core/test_reset.c
  - tests/libretro/libretro_host.c
  - tests/ppu/test_registers.c
  - tests/runner/game_movie.c
  - tests/runner/test_boot_to_play.c
findings:
  critical: 0
  warning: 3
  info: 3
  total: 6
status: issues_found
---

# Phase 10: Code Review Report

**Reviewed:** 2026-10-11
**Depth:** standard (changes since 3de6ec0)
**Files Reviewed:** 15
**Status:** issues_found

## Summary

The core change is small: `nesturbator__ppu_register_write` for `$2000` now raises an NMI only on a 0-to-1 change of bit 7 while the vblank flag is set. It clears a pending NMI when bit 7 is written as 0. The behaviour revision, the API-policy hash and the unit and end-to-end tests are consistent with it. I found no incorrect emulation behaviour and no security issue. This diff adds no new parsing of untrusted input. The remaining findings are robustness gaps in the new test harness code, where a failure can crash or give a false result instead of failing cleanly.

## Warnings

### WR-01: `--rom-parity` carries on after a failed `retro_load_game`

**File:** `tests/libretro/libretro_host.c:1138`
**Issue:** `CHECK(p_load_game(&game))` records the failure but does not stop. The code then calls `p_get_memory_data`, `p_run` for all frames, and `p_unload_game` on a module with no game loaded. A module that fails to load, such as a regression in header parsing, would then be exercised in a state the adapter may not guard. The result is a crash or a hang with a poor diagnostic instead of one clear failed check. The sibling guards (`rom == NULL`, `movie.masks == NULL`) show the intended pattern.
**Fix:**
```c
if (!p_load_game(&game)) {
    CHECK(0);
    p_deinit();
    module_close();
    free(rom);
    nesturbator_movie_free(&movie);
    CHECK_DONE();
}
```

### WR-02: Audio peak-to-peak window can keep stale min/max

**File:** `tests/runner/test_boot_to_play.c:208-215`
**Issue:** `low`/`high` are re-seeded only when `(frame - 1) % WINDOW == 0 && i == 0`, that is, on sample 0 of the first frame of a window. If that frame returns `audio_count == 0`, the loop body never runs and the previous window's extremes carry into the new window. `p2p` then overstates loudness. This can hide a silent window and weaken the P3 floor check, which the "silence" and "click then silence" mutants (edited in memory, never re-run) cannot detect. Today the core emits audio every frame, so this is latent.
**Fix:** Reset at the window start, independent of `audio_count`:
```c
if ((frame - 1u) % WINDOW == 0u) { low = INT32_MAX; high = INT32_MIN; }
/* ... after the loop, when frame % WINDOW == 0: */
t->p2p[checkpoint] = (high >= low) ? high - low : 0;
```

### WR-03: Parity "was written" guard only catches a fully unwritten frame

**File:** `tests/libretro/libretro_host.c:1175-1178`
**Issue:** `CHECK(unwritten < W * H)` passes if even one pixel was written. The comment and the header claim the sentinel proves the module wrote the frame. A module that writes only part of the frame and leaves the rest at `0xA5A5A5A5` is caught only by `compare_with_ppm`. That is probably adequate, but the extra check is misleading as written. The sentinel also masks to 24 bits (`& 0xFFFFFF`) in `frame_equals_ppm`. If `compare_with_ppm` masks the same way, a leftover `0xA5A5A5` pixel would compare against a PPM pixel of that colour, which is unlikely but possible.
**Fix:** Require `unwritten == 0` (a full-frame video callback should overwrite every pixel), or drop the check and state that `compare_with_ppm` is the real assertion.

## Info

### IN-01: Return path of `rom_parity` leaks on early exits

**File:** `tests/libretro/libretro_host.c:1117-1131`
**Issue:** The early `module_close(); CHECK_DONE();` paths are fine for a test process, but the `missing != 0` path does not free the movie or the ROM. Harmless, but ASan leak checking could flag it if `detect_leaks` is on for the `asan` preset (the movie is not yet read on that path, so it is clean today).
**Fix:** None needed now. Keep the single-exit style if more resources are added.

### IN-02: `read_file` duplicated across test programs

**File:** `tests/libretro/libretro_host.c:994`, `tests/runner/test_boot_to_play.c:262`
**Issue:** Two near-identical `read_file` helpers (seek, `ftell`, malloc, fread) now exist in the tests. This follows the existing "small modules" style only loosely. The two share the same unchecked `long` to `size_t` pattern, which is fine here for small ROMs but would truncate on a 32-bit `long` for files over 2 GiB.
**Fix:** Move into a shared test helper (for example `tests/test_file.h`) next to `test_process.h`.

### IN-03: Magic frame constants repeated between the movie writer and the judge

**File:** `tests/runner/game_movie.c:118`, `tests/runner/test_boot_to_play.c:604`
**Issue:** The judge hard-codes `CHECK(start == 61u)` and `K_SCORE`, and the movie writer hard-codes Start at index 60. They are linked only by comments. A change to one without the other fails loudly (by design, per the header comment), so this is a maintenance note rather than a defect.
**Fix:** Optional: share a `NESRUNNER_START_INDEX` constant through a small header included by both.

---

_Reviewed: 2026-10-11_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_
