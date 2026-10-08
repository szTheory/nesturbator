---
phase: 03-a-real-game-in-retroarch
plan: 05
subsystem: input
tags: [controller, libretro, deterministic-input, c17]
requires:
  - phase: 03-04
    provides: CPU bus scheduling, cartridge-backed frame execution, and the PPU frame path
provides:
  - Size-checked per-frame button masks for two standard controller ports
  - Deterministic $4016/$4017 strobe, latch, serial shift, and post-eight behavior
  - Libretro joypad polling and scripted-versus-host frame parity coverage
affects: [runner-movies, libretro-input, cpu-bus, frame-determinism]
actuals:
  tokens: 5232
  tasks: 1
  commits: 3
tech-stack:
  added: []
  patterns:
    - Input masks are queued then snapshotted at successful frame entry
    - Standard controller state remains per-instance and shifts through CPU bus reads
key-files:
  created: [tests/core/test_controller.c]
  modified: [include/nesturbator.h, src/internal.h, src/bus.c, src/instance.c, src/frame.c, libretro/libretro.c, tests/libretro/libretro_host.c, tests/CMakeLists.txt, README.md, tests/cmake/vector_api_policy.cmake]
key-decisions:
  - "Sample the latest valid input masks once at the start of each successful frame call, preserving one input snapshot when an instruction crosses the frame boundary."
  - "Keep two controller latches and shift registers in instance-owned bus state; expose D0 serial data, D6 high, D5/D7 open bus, and ones after eight reads."
requirements-completed: [GAME-02, GAME-03]
coverage:
  - id: D1
    description: "Two standard controller ports strobe, latch, and shift independent masks in NES button order, including high-strobe A reads and ones after eight bits."
    requirement: GAME-02
    verification:
      - kind: unit
        ref: tests/core/test_controller.c#port independence, order, latch transitions, per-frame changes, and invalid input
        status: pass
    human_judgment: false
  - id: D2
    description: "Synthetic libretro joypad callbacks produce the same rendered frame as the public scripted input API."
    requirement: GAME-03
    verification:
      - kind: integration
        ref: tests/libretro/libretro_host.c#check_input_frame_parity
        status: pass
    human_judgment: false
metrics:
  duration: 7min
  completed: 2026-10-08
  status: complete
  commits: 3
  plan_head_before: 5f6e0439ea703fc0528e32a5b7537f94cfed6dc6
  plan_head_after: 2337f9faaabb605d544cbae707aece919647c1c1
---

# Phase 03 Plan 05: Two-Port Controller Input Summary

**Two independent NES controller ports now accept size-checked scripted masks and map RetroPad callbacks to matching rendered frames.**

## Performance

- **Duration:** 7 min
- **Started:** 2026-10-08T17:54:40Z
- **Completed:** 2026-10-08T18:02:03Z
- **Tasks:** 1
- **Files modified:** 11

## Accomplishments

- Added the public size-tagged `nesturbator_input` API, button constants, and frame-entry sampling semantics; rejected calls leave queued input unchanged.
- Implemented two per-instance controller ports at `$4016/$4017`, with A-through-Right bit order, strobe-high A reads, high-to-low latching, independent shifts, D6 high, and D5/D7 open bus.
- Polled and mapped both standard libretro joypads; the synthetic host test compares its resulting frame pixel-for-pixel with the scripted public API frame.
- Updated README and public header documentation for input mapping, bus bits, and frame-boundary behavior.

## Task Commits

1. **Task 1 RED: add failing tests and input API frame queue** — `f7dec54` (`test(03-05): add failing tests for controller ports`).
2. **Task 1 GREEN: implement two-port controller behavior** — `8de6c74` (`feat(03-05): implement scripted two-port controller input`).
3. **Public API policy follow-up** — `2337f9f` (`fix(03-05): refresh public API declaration guard`).

## Files Created/Modified

- `include/nesturbator.h`, `src/internal.h`, `src/instance.c`, `src/frame.c` — size-checked public input contract, instance-owned queued and sampled masks, and per-frame input snapshot.
- `src/bus.c` — controller strobe, dual latches and shifts, serial reads, and open-bus bits.
- `libretro/libretro.c` — poll both ports and map eight standard RetroPad buttons to public NES masks.
- `tests/core/test_controller.c`, `tests/libretro/libretro_host.c`, `tests/CMakeLists.txt` — core behavior and synthetic callback parity tests.
- `README.md` — user-facing controller and sampling behavior.
- `tests/cmake/vector_api_policy.cmake` — updated temporary public-declaration digest for the planned API addition.

## Decisions Made

- Input API calls queue masks for the next frame. A successful frame call snapshots them before CPU work, so instruction overshoot cannot mix two masks.
- Reads preserve bus D5/D7, drive D6 high, and return serial button state on D0; exhausted low-strobe reads shift in ones.

## TDD Gate Compliance

- **RED:** `ctest --test-dir build/ci -R '^core\.controller$' --output-on-failure` exited 8. The named `core.controller` test ran and failed on its expected `$4016/$4017` bit-sequence and post-eight assertions; API validation checks passed. The unmodified JUnit report and exact command details were recorded in `build/ci/controller-red.json`; `gsd_run check tdd-red-evidence` returned `RED_EVIDENCE_OK`. Semantic assessment: valid behavior RED; the planned controller bus behavior was absent.
- **GREEN:** `core.controller` and `libretro.host` passed. The host test's synthetic controller-reading cartridge produced a frame identical to the public scripted-input frame.
- **REFACTOR:** None needed.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Refreshed the Phase 3 public API declaration guard**
- **Found during:** Task 1 full CI verification.
- **Issue:** `vectors.api_policy` still pinned the Phase 2 declaration stream, so the planned public input API failed the repository's guard and its self-test.
- **Fix:** Updated the expected declaration digest to the new public-header stream.
- **Files modified:** `tests/cmake/vector_api_policy.cmake`.
- **Verification:** `vectors.api_policy` and `vectors.api_policy.self_test` passed.
- **Commit:** `2337f9f`.

**Total deviations:** 1 auto-fixed blocking integration issue.
**Impact on plan:** The policy refresh was necessary for the planned public API change; the added `src/frame.c` sampling hook was also needed to implement the plan's explicit frame-boundary contract.

## Issues Encountered

- `cmake --workflow --preset ci` configured and built successfully. It completed with 337/338 tests passing; only the known local `retroarch.testframe` test failed because RetroArch exited with `Subprocess aborted` and empty stdout/stderr. All controller, libretro host, declaration policy, and other tests passed. This is not a full CI green result.

## User Setup Required

None - controller integration tests use synthetic callbacks and require no physical device.

## Next Phase Readiness

Plan 03-11 can use the public per-frame input API for deterministic movie replay. No controller expansion devices or physical input dependency were introduced.

## Self-Check: PASSED

- The new controller unit test and summary exist.
- RED, GREEN, and guard-refresh commits are descendants of the recorded plan base.
- No placeholder or TODO/FIXME stubs were found in files created or modified by this plan.

---
*Phase: 03-a-real-game-in-retroarch*
*Completed: 2026-10-08*
