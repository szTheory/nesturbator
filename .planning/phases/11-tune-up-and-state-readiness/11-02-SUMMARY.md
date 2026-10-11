---
phase: 11-tune-up-and-state-readiness
plan: 02
subsystem: libretro
tags: [lifecycle, unload, power-on]
requires: [11-01]
provides: [nesturbator__power_on_no_cartridge, one libretro instance per retro_init..retro_deinit]
affects: [src/instance.c, src/cartridge.c, libretro/libretro.c, include/nesturbator.h, README.md]
key-files:
  modified: [src/internal.h, src/instance.c, src/cartridge.c, libretro/libretro.c, tests/libretro/libretro_host.c, tests/core/test_cartridge.c, tests/CMakeLists.txt, include/nesturbator.h, README.md]
decisions:
  - "C3: unload always runs the power-on helper; load never does, so no pinned hash moved"
requirements: [TUNE-09]
status: complete
commits: 3
plan_head_before: 65f723e7319ff3cd9ae6ea5f1e8f9450431eadb4
plan_head_after: c4733bc
actuals:
  tokens: 12000
  tasks: 3
  commits: 3
---

# Phase 11 Plan 02: Unload and one libretro instance Summary

The libretro adapter now keeps one instance from `retro_init` to `retro_deinit`, and `nesturbator_unload_cartridge` restores the exact created state through a helper shared with `nesturbator_create`.

## Done
- Task 1 (3e75855): `nesturbator__power_on_no_cartridge` (instance.c); create and unload call it, load does not. Adapter: `retro_init` creates, `retro_load_game` clears the cached span, unloads, then loads (failure leaves the instance, no cartridge), `retro_unload_game` clears the span then unloads. `retro_deinit` already reset every static; left unchanged.
- Task 2 (5325b67): `test_unload_equals_fresh` in `core.cartridge`; `check_lifecycle_reuse`, `check_unload_then_no_content`, `check_unload_edges` in `libretro.host`. asan workflow: 480/480.
- Task 3 (c4733bc): header comment, adapter head comment (task 1), README.
- `cmake --workflow --preset ci`: 485/485. `tests/runner/hashes.txt` unchanged.

## Red run
With unload restored to the old early return (no cartridge) and the helper call commented out:
```
Test #123: core.cartridge ...***Failed
test_cartridge.c:816: CHECK(memcmp(&a->bus, &b->bus, sizeof a->bus) == 0) failed
test_cartridge.c:817: CHECK(memcmp(&a->apu, &b->apu, sizeof a->apu) == 0) failed
test_cartridge.c:818: CHECK(memcmp(&a->synth, ...) failed
test_cartridge.c:819: ... ppu ... failed
test_cartridge.c:820: ... cpu ... failed
test_cartridge.c:822: ... map ... failed
Test #176: libretro.host ...***Failed
33% tests passed, 2 tests failed out of 3
```
`src/cartridge.c` was reverted (`git diff --quiet HEAD` clean) and never committed broken.

## Deviations from Plan
- [Rule 3 - Blocking] `tests/CMakeLists.txt`: `bus.unit` links `cartridge.c` directly, so it needed `instance.c` and `cpu.c` added to resolve the new helper.
- The host's direct-API comparison covers audio hashes per frame (video is compared between the two adapter passes), since the direct core returns native 9-bit video.
- The unload edge test also checks that a rejected image after a good load drops the span.

## Known Stubs
None.

## Self-Check: PASSED
