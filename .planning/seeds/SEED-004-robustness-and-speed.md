---
id: SEED-004
status: dormant
planted: 2026-10-02
planted_during: preparation, before milestone 1
trigger_when: milestone 1 has shipped a playable core that can be measured, or a milestone is about speed, fuzzing or weak devices
scope: medium
audit_acknowledged:
  milestone: v1
  at: 2026-10-09
  status: dormant
---

# SEED-004: The core survives hostile input and gets faster without changing a single hash

## Why This Matters

A core that other programs embed is fed files it did not choose, and a host that rewinds or runs ahead calls it many times per displayed frame. Published accuracy-class emulators reach 250 to 400 frames per second; none publishes more. Speed work is safe here because every frame and audio hash has to stay identical.

## When to Surface

**Trigger:** milestone 1 has shipped a playable core that can be measured, or a milestone is about speed, fuzzing or weak devices.

This seed covers requirements PERF-01, PERF-02 and PERF-03 in `.planning/REQUIREMENTS.md`.

## Scope Estimate

**Medium** — one or two phases: property tests and long fuzz runs over the loader, the state loader and bounded runs of arbitrary ROM bytes; then an instruction-count comparison on every pull request, and a runner mode that prints frames per second and frame-time percentiles.

## Breadcrumbs

- `.planning/preparation/ENGINEERING.md` section 4: fuzz targets and property tests with a printed seed.
- `.planning/preparation/ENGINEERING.md` section 5: the `perf` job, instruction counts under Cachegrind, head against merge-base.
- `.planning/preparation/ARCHITECTURE.md` section 2: eager PPU catch-up first, a lazy schedule only if hashes stay identical.
- `.planning/preparation/NES-ECOSYSTEM.md` section 2: published speeds and what the finest time step costs.

## Notes

A lazy PPU schedule is accepted only if every frame hash stays identical.
