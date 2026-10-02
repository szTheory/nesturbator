---
id: SEED-005
status: dormant
planted: 2026-10-02
planted_during: preparation, before milestone 1
trigger_when: the six common mappers and save states have shipped, or a milestone is about Famicom games, expansion audio, the Disk System or extra controllers
scope: large
---

# SEED-005: Famicom hardware — expansion audio, the Disk System, more mappers and peripherals

## Why This Matters

The Famicom library uses hardware the NES never had: extra sound chips on the cartridge, the Disk System, and controllers such as the Zapper, paddles and the microphone. Peripherals are the second largest group of user reports against existing cores, and Disk System side swapping has its own group.

## When to Surface

**Trigger:** the six common mappers and save states have shipped, or a milestone is about Famicom games, expansion audio, the Disk System or extra controllers.

This seed covers requirement HW-01 in `.planning/REQUIREMENTS.md`.

## Scope Estimate

**Large** — more than one milestone. Each of these stands alone: the next mappers by library share (206, 5, 19, 16, 18, 66, 69 and 210); expansion audio; the Disk System with a host-supplied BIOS; light gun and paddle input.

## Breadcrumbs

- `.planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md` section 5: the next tier of mappers by dump count.
- `.planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md` sections 6 and 8: FDS images and BIOS; regions, RGB PPUs, input devices.
- `.planning/preparation/NES-HARDWARE-CPU-APU.md` section 4: where expansion audio is documented.
- `.planning/preparation/LIBRETRO-AND-RUNNER.md` section 2: the libretro disk-control calls.
- `.planning/preparation/NES-ECOSYSTEM.md` section 5: what users report, ranked.

## Notes

The Disk System BIOS is supplied by the player and never shipped.
