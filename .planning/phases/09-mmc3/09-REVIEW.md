---
phase: 09-mmc3
reviewed: 2026-10-10T00:00:00Z
depth: standard
files_reviewed: 8
files_reviewed_list:
  - src/mapper_mmc3.c
  - src/cartridge.c
  - src/mapper.h
  - src/ppu.c
  - runner/main.c
  - include/nesturbator.h
  - src/internal.h
  - src/instance.c
findings:
  critical: 0
  warning: 1
  info: 2
  total: 3
status: issues_found
---

# Phase 9: Code Review Report

**Reviewed:** 2026-10-10
**Depth:** standard
**Files Reviewed:** 8 (shipped core and runner code, with the helpers they call). The test, CMake and CI files in the scope list were not read.

## Summary

I traced the MMC3 board against the shared helpers: the modulo bank wrap, PRG RAM enable and protect, CPU write decode, the A12 filter, reset behaviour and the cartridge validation profile. I found no out-of-bounds access, no crash path and no memory-safety problem.

- PRG and CHR bank numbers wrap by true modulo.
- `second_last` cannot underflow, because validation requires at least 16 KiB of PRG.
- The `cpu_cycle` and `low_cycle` arithmetic is safe, because `nesturbator_reset` keeps `cpu_cycle`.
- The runner frees `bytes` on every path.

The remaining issues are robustness and maintainability.

## Warnings

### WR-01: iNES 1 header mirroring bits and four-screen are silently ignored for MMC3 until the first rebuild

**File:** `src/mapper_mmc3.c:82`
**Issue:** `mmc3_rebuild` always derives nametable mirroring from the `$A000` register. That register is zero at power-on, so the board starts as vertical mirroring regardless of header flags 6 bit 0. This matches the documented power-on state, and hardware has no header. A game that never writes `$A000` therefore gets vertical mirroring even when its header says horizontal. That is plausible for test ROMs and is untested at the loader level. The code does not say that the header bit is intentionally unused for mapper 4.
**Fix:** State in the file header comment and in `nesturbator.h` that mapper 4 ignores the header mirroring bit. Add a test that loads a horizontal-header MMC3 image and asserts vertical mirroring until `$A000` is written.

## Info

### IN-01: `map_chr_1k` computes the same pointer twice

**File:** `src/mapper_mmc3.c:29-30`
**Issue:** `nesturbator__map_chr` runs twice per page, and the modulo is computed twice. `mmc3_rebuild` runs this on every register write.
**Fix:**
```c
uint8_t *p = nesturbator__map_chr(nes, bank_1k);
nes->map.chr_r[page] = p;
nes->map.chr_w[page] = nes->cart.chr_is_ram != 0u ? p : NULL;
```

### IN-02: `describe_rejection` duplicates the loader's header decoding

**File:** `runner/main.c:622-635`
**Issue:** The runner re-implements the NES 2.0 mapper and submapper bit decoding and keeps its own supported-mapper list in three strings. The list will drift the next time a mapper is added; this phase already had to edit three messages. The `mapper != 0 && ...` chain is also long.
**Fix:** Keep one static array of supported mapper ids and format the message from it. Alternatively, have the library expose a rejection reason.

---

_Reviewed: 2026-10-10_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_
