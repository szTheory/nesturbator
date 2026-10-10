---
phase: 06-mapper-seam-and-ppu-fetch-pipeline
reviewed: 2026-10-10T00:00:00Z
depth: standard
files_reviewed: 22
files_reviewed_list:
  - CMakeLists.txt
  - README.md
  - include/nesturbator.h
  - src/apu.c
  - src/bus.c
  - src/cartridge.c
  - src/internal.h
  - src/mapper.h
  - src/mapper_nrom.c
  - src/ppu.c
  - tests/CMakeLists.txt
  - tests/cmake/vector_api_policy.cmake
  - tests/core/test_api.c
  - tests/core/test_mapper.c
  - tests/mapper_test.h
  - tests/ppu/ppu_fixture.h
  - tests/ppu/test_fetch.c
  - tests/ppu/test_registers.c
  - tests/ppu/test_render.c
  - tests/ppu/test_split_scroll.c
  - tests/ppu/test_sprites.c
  - tests/runner/hashes.txt
findings:
  critical: 0
  warning: 3
  info: 2
  total: 5
status: issues_found
---

# Phase 6: Code Review Report

**Reviewed:** 2026-10-10
**Depth:** standard
**Files Reviewed:** 22
**Status:** issues_found

## Summary

Reviewed the mapper seam (mapper.h, mapper_nrom.c, cartridge.c, bus.c, apu.c IRQ OR) and the rewritten PPU fetch pipeline (ppu.c). I traced the shifter shift/reload dot schedule (shifts at 2-257 and 322-337, reloads at 9..257, 329, 337), the v increments and copies, the ALE/read-dot address rebuild, sprite slot fetch addressing (8x8, 8x16, flips, empty slot tile $FF), the odd-frame skip, and the NROM page tables (trainer RAM at $6000-$7FFF, 16 KiB mirror, CHR RAM writability). I found no memory-safety or determinism defects and no build-breaking problems. The test files (test_fetch.c, test_mapper.c, fixtures) have real assertions and bounded loops; I found no reliability problems in them. The findings below are an off-by-one against the documented contract, one register edge case, and two robustness items.

## Warnings

### WR-01: Mapper write stamp is one higher than the documented cycle index

**File:** `src/bus.c:30-31` (also `src/mapper.h:32`, `src/bus.c:175`)
**Issue:** `cycle()` increments `nes->cpu_cycle` before `nesturbator__bus_write` calls the hook with `nes->cpu_cycle`. `mapper.h` documents the stamp as "the index of the write's cycle", and the comment in `cycle()` says "the index of the cycle that just ran". With a zero-based count since load, the first write gets stamp 1 while its index is 0. The stamp is really "cycles completed, including this one". Boards such as the MMC1 compare stamps of consecutive writes, so differences still work. But any board that compares a stamp with an absolute index, or an MMC3 or IRQ timer that anchors to `cpu_cycle`, will be off by one, and the contract text is wrong either way. `check_write_stamps` only asserts relative differences and equality with the post-write `cpu_cycle`, so it cannot catch this.
**Fix:** Either stamp the pre-increment index or fix the wording. For the code:
```c
/* in nesturbator__bus_write: cycle(nes) has already incremented, so the write's own index is: */
nes->map.ops.cpu_write(nes, addr, v, nes->cpu_cycle - 1u);
```
Otherwise change both comments to "the count of CPU cycles completed including the write's". Add a test that asserts the first write after load has the documented stamp.

### WR-02: `$2007` treats v >= $4000 as non-palette

**File:** `src/ppu.c:570` (and `src/ppu.c:577`)
**Issue:** `v` is a 15-bit register. `$2005` second write (`value & 7) << 12`) and the pre-render vertical copy (mask `0x7be0`) can set bit 14. `nesturbator__ppu_read` masks the address to 14 bits, but the `v < 0x3f00u` test does not. With v = $7F00 (or any value in $4000-$7FFF whose low 14 bits are $3F00-$3FFF), the palette is read through the buffered path and the palette-mirror branch is skipped. On hardware the access uses the 14-bit bus address, so the palette read returns immediately. The hit is rare (rendering is turned off after a scroll with fine Y 4-7 and `$2007` is read without a `$2006` write), but it is a deterministic wrong result.
**Fix:**
```c
uint16_t addr = (uint16_t)(nes->ppu.v & 0x3fffu);
uint8_t memory_value = nesturbator__ppu_read(nes, addr);
if (addr < 0x3f00u) { ... } else { ... nesturbator__ppu_read(nes, (uint16_t)(addr - 0x1000u)); ... nesturbator__ppu_read(nes, addr); ... }
```
Add a `test_registers.c` case with v = $7F00.

### WR-03: `nesturbator__map_cpu_read` indexes `cpu_r` with no range guard

**File:** `src/internal.h:221-225`
**Issue:** `(addr - 0x4000u) >> 10` is computed in unsigned int. For addr < $4000 the result is about 4 million, an out-of-bounds read of `cpu_r[48]`. All current callers are safe (`bus_read_data` guards >= $4020, DMC addresses are always >= $8000, the bus-conflict path is in the >= $4020 branch). But this is a shared static inline whose contract says "$4020-$FFFF" only in prose, and the next board or caller that passes a lower address gets undefined behaviour that the sanitizers only catch if a test happens to hit it. The same pattern is in `bus_write` (`cpu_w[(addr - 0x4000u) >> 10]`), which is guarded.
**Fix:** Mask or assert in the helper, for example `if (addr < 0x4000u) return nes->bus.open_bus;`, or document it as a precondition and add a `CHECK` in the mapper test for the lowest and highest accepted addresses.

## Info

### IN-01: Unknown mapper id loads successfully as an empty cartridge

**File:** `src/cartridge.c:148-154`
**Issue:** `nesturbator__mapper_load` returns silently from the `default` case, leaving a zeroed map (all pages NULL, no ops), and `nesturbator_load_cartridge` still returns `NESTURBATOR_OK`. Today the validator rejects every non-zero mapper, so this is unreachable. If the validator is relaxed before a board is added, the game would boot with every PRG page open bus and no error.
**Fix:** Make `nesturbator__mapper_load` return a status and have the loader return `NESTURBATOR_ERR_CARTRIDGE` (after freeing the copy) for an unsupported id.

### IN-02: Comments contradict each other on NROM stamp and a possibly stale `cart.size` comment

**File:** `src/internal.h:198-199`, `src/bus.c:30`
**Issue:** The `cpu_cycle` field comment ("CPU bus cycles since load") and the bus comment ("index of the cycle that just ran") describe two different quantities (see WR-01). Resolving WR-01 should also reconcile the comments.
**Fix:** Pick one definition and use it in `internal.h`, `bus.c` and `mapper.h`.

---

_Reviewed: 2026-10-10_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_
