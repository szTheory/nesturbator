---
phase: 30-mmc2-mainline-compatibility-and-delivery
plan: "02"
subsystem: clients-and-compatibility
tags: [MMC2, runner, libretro, save-state, provider]

requires: ["30-01"]
provides:
  - Cold and frame-3 restored MMC2 execution parity through the public library, headless runner and dynamically loaded libretro module.
  - Current provider/runtime metadata and state pins updated to ABI 1, behavior revision 10 and state format 4.
affects: [30-03, release]

actuals:
  tokens: 10100
  tasks: 2
  commits: 2

key-files:
  created: []
  modified: [tests/CMakeLists.txt, tests/libretro/libretro_host.c, runner/main.c, tests/runner/hashes.txt, include/nesturbator.h, README.md, docs/provider-ntsc.json, tests/cmake/provider_profile.cmake]

requirements-completed: [MMC2-07, MMC2-08, MMC2-09, MMC2-10]
coverage:
  - id: D1
    description: "Cold and resumed marker output, PCM, native state and ticks agree across all three clients."
    requirement: MMC2-07
    verification:
      - kind: focused
        ref: "build/ci/phase30-02-parity-focused.log"
        status: pass
    human_judgment: false
  - id: D2
    description: "Provider compatibility, runtime fixture and saved-state format pins match the current public contract."
    requirement: MMC2-10
    verification:
      - kind: focused
        ref: "build/ci/phase30-02-final-focused.log"
        status: pass
    human_judgment: false
  - id: D3
    description: "Existing-board state hashes change only for version/revision/CRC header bytes; payload, size and ticks stay identical."
    requirement: MMC2-08
    verification:
      - kind: transform
        ref: "build/ci/phase30-02-state-transform.log"
        status: pass
      - kind: producer
        ref: "build/ci/phase30-02-hash-producer-final.log"
        status: pass
    human_judgment: false
  - id: D4
    description: "Frame/audio output pins and protected AccuracyCoin inputs remain unchanged."
    requirement: MMC2-09
    verification:
      - kind: inventory
        ref: "build/ci/phase30-02-final-focused.log"
        status: pass
    human_judgment: false

duration: 10min
completed: 2026-11-19
status: complete
---

# Phase 30 Plan 02 Summary

**The owned MMC2 image now passes cold and restored parity through the runner and dynamically loaded libretro module, and the current provider/state metadata is pinned to rev10/state4.**

## Accomplishments

- Compared video, PCM, native serialized state, frame count and tick continuation through the public library, runner and actual libretro module using the same owned MMC2 image.
- Updated runner mapper support text and registered `libretro.mmc2.parity` with its 30-second cap.
- Updated the live provider profile and synthetic runtime fixture to ABI 1, behavior revision 10, state format 4. Preserved the historical installed-consumer report at its recorded rev9/state3 identity.
- Regenerated the state inventory; exactly 12 existing-board hashes and the format row changed. All other generated frame, audio, tick and size rows were byte-for-byte unchanged, and the protected AccuracyCoin inputs were not read or changed.
- Captured base and current state files for all six pinned boards at frames 0 and 60. Updating only header bytes 8–9 (format), 16–19 (behavior revision) and 28–31 (CRC) made each base file byte-identical to the current file; body bytes, size and ticks matched.

## Verification

- Configure with `NESTURBATOR_ACCURACYCOIN_FULL=ON` passed: `build/ci/phase30-02-configure.log`.
- The parity host, runner and libretro module built with warnings as errors: `build/ci/phase30-02-final-build.log`.
- CTest inventory confirmed one `libretro.mmc2.parity` test with timeout 30 and no campaign fixture: `build/ci/phase30-02-parity-inventory.json`; the test passed.
- State/provider inventory contained only the writer, format pin and two provider checks. The writer generated to the build tree and its ROM list excludes AccuracyCoin: `build/ci/phase30-02-state-provider-inventory.json` and `build/ci/phase30-02-hash-producer-final.log`.
- State transform across six boards and two frames passed: `build/ci/phase30-02-state-transform.log`.
- Parity, provider contract/control and state-format pin all passed together: `build/ci/phase30-02-final-focused.log`.
- `clang-format` 18.1.8, `git diff --check`, and staged hygiene passed. No full workflow was run; plan 30-03 still owns the single Phase 30 full workflow.

## Task Commits

1. **Runner/libretro cold and resumed parity** — `471249c` (`test(mmc2): verify runner and libretro parity`)
2. **Provider and state pins** — `35a3604` (`docs(mmc2): update provider and state pins`)

## Issues Encountered

The hash producer first stopped because its declared `runner.game_movie` and derived Holy Mapperel ROM build dependencies were absent. Both were built, then the same producer completed successfully. No output or evidence allowance was renewed.

## Next Plan Readiness

Plan 30-03 is the only remaining plan. It owns frozen-source preflight, exact evidence binding, the single full workflow and delivery gates. Recheck live main before source work; the recorded local baseline remains `8923b66cca16739c5b402643798515cf5537ae22`.

---
*Phase: 30-mmc2-mainline-compatibility-and-delivery*
*Plan: 02*
*Completed: 2026-11-19*
