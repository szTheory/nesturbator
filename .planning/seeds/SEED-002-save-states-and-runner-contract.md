---
id: SEED-002
status: dormant
planted: 2026-10-02
planted_during: preparation, before milestone 1
trigger_when: a milestone includes save states, rewind, run-ahead or netplay, or Playstead is ready to launch this core
scope: large
---

# SEED-002: Save states, rewind and run-ahead in RetroArch, and the runner contract Playstead needs

## Why This Matters

RetroArch enables rewind only for a core that declares serialized save states, and run-ahead and netplay only for one that declares deterministic states. States that change size with an option or break between versions are the complaint users feel most in existing cores. Playstead needs an emulator process that replays input exactly, flushes saves on a signal and reports what happened through its exit status.

## When to Surface

**Trigger:** a milestone includes save states, rewind, run-ahead or netplay, or Playstead is ready to launch this core.

This seed covers requirements STATE-01 and STATE-02 in `.planning/REQUIREMENTS.md`.

## Scope Estimate

**Large** — a milestone, or two phases inside one: save states with rewind and run-ahead in RetroArch first, then the runner's save directory, signal handling and exit statuses.

## Breadcrumbs

- `.planning/preparation/ARCHITECTURE.md` section 7: fixed-size canonical state, checked in full before it is applied.
- `.planning/preparation/LIBRETRO-AND-RUNNER.md` section 2: serialization rules, the `.info` save-state levels, memory descriptors for achievements.
- `.planning/preparation/LIBRETRO-AND-RUNNER.md` section 5: the runner's process contract and what Playstead does today.
- `.planning/preparation/LIBRETRO-AND-RUNNER.md` section 7: the C ABI a future shared host would load.
- `.planning/preparation/NES-ECOSYSTEM.md` sections 4 and 5: what went wrong with states in other cores.

## Notes

The state format gets its own version number from the first release that has one, separate from the library version.
