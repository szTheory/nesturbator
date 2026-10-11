---
phase: 08-mmc1-and-battery-saves
reviewed: 2026-10-10T00:00:00Z
depth: standard
files_reviewed: 42
files_reviewed_list:
  - .github/workflows/ci.yml
  - CMakeLists.txt
  - README.md
  - THIRD-PARTY-NOTICES.md
  - include/nesturbator.h
  - libretro/libretro.c
  - libretro/nesturbator_libretro.info
  - runner/CMakeLists.txt
  - runner/main.c
  - runner/save.c
  - runner/save.h
  - src/bus.c
  - src/cartridge.c
  - src/instance.c
  - src/internal.h
  - src/mapper.h
  - src/mapper_mmc1.c
  - tests/CMakeLists.txt
  - tests/cmake/check_sav.cmake
  - tests/cmake/clean_dir.cmake
  - tests/cmake/hash_inventory.cmake
  - tests/cmake/runner_save.cmake
  - tests/cmake/vector_api_policy.cmake
  - tests/cmake/write_hashes.cmake
  - tests/core/test_api.c
  - tests/core/test_cartridge.c
  - tests/core/test_mapper.c
  - tests/core/test_mapper_mmc1.c
  - tests/core/test_save.c
  - tests/header/header_c.c
  - tests/header/header_cxx.cpp
  - tests/holymapperel/decode.c
  - tests/holymapperel/hm_decode.h
  - tests/holymapperel/roms.cmake
  - tests/holymapperel/test_decode.c
  - tests/ines.h
  - tests/libretro/libretro_host.c
  - tests/retroarch/run_retroarch.cmake
  - tests/retroarch/test.cfg.in
  - tests/roms/manifest.txt
  - tests/runner/save_rom.c
  - tests/runner/test_save.c
findings:
  critical: 0
  warning: 3
  info: 2
  total: 5
status: issues_found
---

# Phase 8: Code Review Report

**Reviewed:** 2026-10-10
**Depth:** standard
**Files Reviewed:** 42
**Status:** issues_found

## Summary

The C sources (MMC1 board, loader, bus save counter, runner save module, libretro memory span) were read in full and cross-checked against the diff. The MMC1 serial port, bank math, variant RAM selection and the loader's single-allocation layout are sound. No security or crash defects were found. The CMake, workflow, test and documentation files were scanned for consistency only, not line by line. The findings below concern a lost-save path in the runner and a board-variant gap.

## Warnings

### WR-01: A failed interval flush is never retried, so the exit flush can silently skip the save

**File:** `runner/main.c:261-271`
**Issue:** `save_if_changed` stores `sv->generation = gen` before calling `nesturbator_run_save_flush`. If that flush fails (disk full, directory removed), the generation is already consumed and `shadow` still holds the old bytes. The run then sets `status = 1` and breaks out of the loop. The exit flush calls `save_if_changed` again and sees `gen == sv->generation`, so it returns 0 without trying. The battery data is lost although the bytes differ from the shadow, and the user gets no second attempt even if the fault was transient.
**Fix:** Advance the generation only on success:
```c
    if (memcmp(sv->span, sv->shadow, sv->size) == 0) { sv->generation = gen; return 0; }
    if (nesturbator_run_save_flush(sv->path, sv->span, sv->size, sv->shadow) != 0)
        return 1;
    sv->generation = gen;
    return 0;
```

### WR-02: NES 2.0 MMC1 submapper 5 (fixed 32 KiB PRG) is accepted but emulated as ordinary MMC1

**File:** `src/cartridge.c:108`, `src/mapper_mmc1.c:44-76`
**Issue:** The profile admits submapper 5 only for 32 KiB PRG, which marks SEROM/SHROM/SH1ROM boards. On these boards the PRG bank register is not connected. `mmc1_rebuild` never reads `nes->mapper.submapper`, so a program that writes PRG register value 1 in mode 3 maps bank 1 into both windows, while the hardware keeps bank 0 at $8000 and bank 1 at $C000. `grep submapper src/mapper_mmc1.c` returns nothing, so the submapper is validated but never used. This is from my reading of the NESdev "MMC1" and "NES 2.0 submappers" pages, which I did not re-check against the wiki text; treat the hardware claim as likely rather than proven.
**Fix:** In `mmc1_rebuild`, when `nes->mapper.submapper == 5u`, force `lo = 0u; hi = 1u`. Add a test that writes a nonzero PRG value and expects the fixed layout. If the intent is to treat submapper 5 as plain MMC1, remove it from the profile or document the choice.

### WR-03: A directory (or other unreadable path) at the .sav location reports a size mismatch

**File:** `runner/save.c:46-66`
**Issue:** On POSIX, `fopen(dir, "rb")` succeeds on a directory. `ftell` then returns a garbage or huge value, or -1, and the user gets "is N bytes; the cartridge's battery RAM is 8192 bytes" with exit status 4 instead of a read error. Exit 4 is documented as the .sav size mismatch status, so scripts that act on it are misled. A `LONG_MAX` result is also compared through `unsigned long`.
**Fix:** After `fopen`, treat a failed first `fread` or `fstat`/`S_ISREG` check as `NESTURBATOR_RUN_SAVE_ERROR`. Alternatively, read at most `size + 1` bytes and compare the count read, which avoids `ftell` altogether.

## Info

### IN-01: Interval test depends on a wall-clock timeout

**File:** `tests/cmake/runner_save.cmake:130-138`
**Issue:** The `interval` case kills a run of 4294967295 frames after 4 seconds and expects the file to exist. On a slow or loaded CI runner, frame 1 plus a flush within 4 seconds is almost certain, but the case still relies on timing, and the timeout path leaves a `.tmp` file if the kill lands during a flush (the case does not check for it).
**Fix:** Keep the case, but assert `${sav}.tmp` is either absent or ignored, and consider a larger timeout margin for hosted runners.

### IN-02: Failed flush leaves a stale `.tmp` file with no cleanup

**File:** `runner/save.c:99-111`
**Issue:** When the write or the rename fails, `<path>.tmp` is left behind. The rename-failure message tells the user where the new save is, which is deliberate. A write failure leaves a partial file, though, and the message gives no such guidance.
**Fix:** On the `!ok` write path, `remove(tmp)` before returning.

---

_Reviewed: 2026-10-10_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_
