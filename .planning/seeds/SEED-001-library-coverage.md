---
id: SEED-001
status: dormant
planted: 2026-10-02
planted_during: preparation, before milestone 1
trigger_when: milestone 1 has shipped (NROM games play with picture, input and sound) and the next milestone is about which games run
scope: large
---

# SEED-001: Most of the licensed library plays (MMC1, MMC3, UxROM, CNROM, AxROM, battery saves)

## Why This Matters

After milestone 1 only NROM games run, which is 4.8% of the North American licensed library. Five more board families bring that to 96.2%, and battery saves make the long games in that set playable across sessions. Mapper coverage is also the largest group of user reports against existing cores.

## When to Surface

**Trigger:** milestone 1 has shipped and the next milestone is about which games run.

This seed covers requirements LIB-01 and LIB-02 in `.planning/REQUIREMENTS.md`.

## Scope Estimate

**Large** — a milestone. One phase per result a player can see works well: MMC1 with battery saves, then MMC3 with its scanline counter, then the discrete boards (UxROM, CNROM, AxROM).

## Breadcrumbs

- `.planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md` section 5: library share per mapper, board variants, bus conflicts, battery counts.
- `.planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md` section 2: what mappers observe on the PPU bus; the MMC3 counter.
- `.planning/preparation/ARCHITECTURE.md` section 6: page tables and the events a mapper watches.
- `.planning/preparation/CONFORMANCE.md`: Holy Mapperel, the MMC3 test ROMs, and an open-licence MMC1 game.
- `.planning/preparation/LIBRETRO-AND-RUNNER.md` sections 1 and 5: how battery RAM reaches RetroArch and the runner.

## Notes

With these boards working the core is worth offering to the libretro buildbot, so players can install it from RetroArch's own downloader (`LIBRETRO-AND-RUNNER.md` section 2).
