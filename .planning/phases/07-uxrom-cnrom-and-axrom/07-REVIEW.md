---
phase: 07-uxrom-cnrom-and-axrom
reviewed: 2026-10-10T00:00:00Z
depth: standard
files_reviewed: 25
files_reviewed_list:
  - CMakeLists.txt
  - README.md
  - THIRD-PARTY-NOTICES.md
  - include/nesturbator.h
  - libretro/libretro.c
  - runner/main.c
  - src/cartridge.c
  - src/internal.h
  - src/mapper.h
  - src/mapper_axrom.c
  - src/mapper_cnrom.c
  - src/mapper_nrom.c
  - src/mapper_uxrom.c
  - tests/CMakeLists.txt
  - tests/cmake/hash_inventory.cmake
  - tests/cmake/write_hashes.cmake
  - tests/core/test_cartridge.c
  - tests/core/test_mapper_discrete.c
  - tests/holymapperel/decode.c
  - tests/holymapperel/hm_decode.h
  - tests/holymapperel/roms.cmake
  - tests/holymapperel/test_decode.c
  - tests/ines.h
  - tests/roms/manifest.txt
  - tests/runner/hashes.txt
findings:
  critical: 0
  warning: 2
  info: 3
  total: 5
status: issues_found
---

# Phase 7: Code Review Report

**Reviewed:** 2026-10-10
**Depth:** standard
**Files Reviewed:** 25
**Status:** issues_found

## Summary

I traced the three new boards (UxROM, CNROM, AxROM), the per-board validation profile, the bus-conflict path in `src/bus.c`, the page-table helpers in `src/internal.h`, the Holy Mapperel decoder, and the CMake hash inventory changes.

I found no memory-safety or correctness bugs in the shipped library code.
- Every bank index goes through a true modulo against the validated size.
- UxROM's `last_1k` is safe for 16 KiB PRG.
- `board_profile_ok` rules out zero-size CHR before any divide.
- The bus-conflict AND uses the pre-write mapping, as the comments say.
- The validator probes the board switch before allocating.

The findings below are about behaviour that is documented more strongly than it is implemented or tested, plus stale documentation.

## Warnings

### WR-01: AxROM "reset vector comes from bank 0" is true only at power-on, not after a soft reset

**File:** `src/mapper_axrom.c:8-10`, `include/nesturbator.h` (load_cartridge comment), `README.md` (runner section)
**Issue:** The header and README say flatly that "the reset vector is read from bank 0". `nesturbator__cpu_reset` (`src/cpu.c:1375`) reads `$FFFC/$FFFD` over the bus, and `nesturbator_reset` (`src/instance.c:158`) leaves `mapper.reg` alone. After a game selects bank N, a soft reset (RetroArch's Reset button) therefore fetches the vector from bank N. That matches hardware, because the AxROM register is not reset. But the documented contract is wrong for soft reset. No test covers `nesturbator_reset` on a board with a non-zero bank register, so the behaviour is unpinned.
**Fix:** Qualify the header and README text ("at power-on; a soft reset keeps the bank register"). Add a test in `tests/core/test_mapper_discrete.c`. Write bank 3 to an AxROM image whose bank 3 has a distinct vector, call `nesturbator_reset`, and check `cpu.pc`. Do the same for UxROM, where the fixed bank makes it safe.

### WR-02: Hash-inventory validator compares only the last platform's keys

**File:** `tests/cmake/hash_inventory.cmake:87-110`
**Issue:** `set(actual_keys)` runs inside the per-directory loop, so after the loop `actual_keys` holds only the final directory's keys. The earlier directories are checked for byte equality with the first file, so the gate holds in practice. Still, the "expected 39-key inventory" check reads as if it covered every artifact. This code was extended in this phase to add the Holy Mapperel keys, and a refactor that drops the byte-equality check would silently weaken it.
**Fix:** Move `set(actual_keys)` and the key comparison inside the loop, or add a comment saying the equality check is what covers the other directories.

## Info

### IN-01: README status paragraph is stale and garbled

**File:** `README.md:6-12`
**Issue:** It says "milestone v2 adds UxROM (mapper 2)" although CNROM and AxROM also ship in this phase. The geometry sentence is run together: "mapper 2 (UxROM), with 8 KiB CHR ROM or declared CHR RAM, mapper 3 (CNROM, 8 to 32 KiB CHR ROM) and mapper 7 …". It reads as if the 8 KiB CHR limit applies to the whole list, and the trainer sentence now lists boards by name. CLAUDE.md rule 6 requires the README to be accurate in the same change.
**Fix:** Rewrite as a short per-board list (NROM, UxROM, CNROM, AxROM with PRG and CHR shapes) and say that milestone v2 adds mappers 2, 3 and 7.

### IN-02: Bus-conflict AND is computed for every write in `$4020-$7FFF` on conflict boards

**File:** `src/bus.c:171-175`
**Issue:** With `NESTURBATOR_WATCH_BUS_CONFLICT` set, the core calls `nesturbator__map_cpu_read` for any write at or above `$4020`, including `$6000-$7FFF` PRG-RAM writes and open-bus pages. The three boards ignore `addr < 0x8000`, so nothing goes wrong today. A future board that watches both bits and accepts `$6000` writes would get an AND against PRG-RAM or open bus.
**Fix:** Apply the AND only when `addr >= 0x8000u`, or document in `mapper.h` that the conflict applies to the whole `$4020-$FFFF` write hook.

### IN-03: Magic board-size limits repeated in `board_profile_ok`

**File:** `src/cartridge.c:68-78`
**Issue:** `16384u`, `32768u`, `8192u` and `4194304u` appear as bare literals across the profile rows. The same numbers also appear in the tests and in `nesturbator.h`. This is minor and consistent with the existing style.
**Fix:** Optional: name them (`PRG_16K`, `PRG_32K`, `CHR_8K`, `UXROM_MAX_PRG`) in `cartridge.c`.

---

_Reviewed: 2026-10-10_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_
