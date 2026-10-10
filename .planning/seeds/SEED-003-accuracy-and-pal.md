---
id: SEED-003
status: dormant
planted: 2026-10-02
planted_during: preparation, before milestone 1
trigger_when: the committed scoreboard shows which AccuracyCoin tests still fail, or a milestone is about accuracy or about PAL and Dendy games
scope: large
audit_acknowledged:
  milestone: v1
  at: 2026-10-09
  status: dormant
---

# SEED-003: Every AccuracyCoin test passes, and PAL and Dendy games run at their own timing

## Why This Matters

The open slot this project aims at is a permissive C core in the accuracy class: 136 or more on AccuracyCoin at a pinned commit. Milestone 1 builds the timing model those tests need and records the score; this work closes the remaining failures one test at a time. PAL and Dendy timing opens the European and clone libraries.

## When to Surface

**Trigger:** the committed scoreboard shows which AccuracyCoin tests still fail, or a milestone is about accuracy or about PAL and Dendy games.

This seed covers requirements ACC-01 and ACC-02 in `.planning/REQUIREMENTS.md`.

## Scope Estimate

**Large** — a milestone. Phases follow the scoreboard: the failing CPU and DMA tests, the failing PPU tests, then the PAL and Dendy profiles.

## Breadcrumbs

- `.planning/preparation/CONFORMANCE.md`: AccuracyCoin's result bytes and test names, the classic suites and how each reports, MesenCE as a reference binary, the scoreboard file.
- `.planning/preparation/NES-HARDWARE-CPU-APU.md` sections 3 and 6: the hard cases with the test for each; region tables.
- `.planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md` sections 3 and 8: the PPU hard cases; PAL and Dendy frame timing.
- `.planning/preparation/NES-ECOSYSTEM.md` section 2: how the emulators that score 136 to 140 are built.

## Notes

The public leaderboard takes a results image and both revisions by e-mail; whether and when to submit is the owner's call. AccuracyCoin changes often, so moving its pin is a change of its own.
