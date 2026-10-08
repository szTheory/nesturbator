---
phase: 03-a-real-game-in-retroarch
plan: 08
subsystem: ci
tags: [GitHub Actions, CMake, RetroArch, deterministic hashes, Windows]
requires:
  - phase: 03-06
    provides: Pinned game ROMs and frame-hash conventions.
  - phase: 03-11
    provides: Deterministic movie input scripts and replay hashes.
  - phase: 03-12
    provides: Stable display output for runner and RetroArch pixel comparison.
  - phase: 03-13
    provides: Protected-main scoreboard baseline in hosted CI.
provides:
  - Required six-platform, 30-row game/movie hash equality gate.
  - Required checksum-pinned RetroArch screenshot-to-runner comparison.
  - Exact-commit CI and nightly evidence for Phase 03.
affects: [GAME-02, GAME-03, GAME-05, GAME-06, future-phase-verification]
tech-stack:
  added: []
  patterns:
    - Launch test subprocesses directly so Windows verification does not depend on cmd.exe quoting.
    - Bind hosted evidence to the exact tested commit and retain the frame artifact.
key-files:
  created:
    - tests/test_process.h
    - .planning/phases/03-a-real-game-in-retroarch/03-08-SUMMARY.md
  modified:
    - .github/workflows/ci.yml
    - tests/cmake/write_hashes.cmake
    - tests/retroarch/run_retroarch.cmake
    - tests/CMakeLists.txt
    - README.md
    - .planning/ROADMAP.md
    - .planning/STATE.md
decisions:
  - Require the hosted RetroArch pixel proof and exact six-platform hashes through `CI required`.
  - Use machine-verifiable evidence as the default completion path; owner UAT is unnecessary when the automated boundaries pass.
actuals:
  tasks: 2
  plan_head_after: 0ba7b8127f13bd06a6d1d35f7ff3dee6aa7c95d2
metrics:
  completed: 2026-10-08
  status: complete
requirements-completed: [GAME-02, GAME-03, GAME-05, GAME-06]
coverage:
  - id: D1
    description: Every pinned game and scripted-input frame has one sorted row in each platform artifact, with exact key and digest equality.
    requirement: GAME-06
    verification:
      - kind: integration
        ref: "CI run 37860982545, `hash-equality` and `CI required`, exact head 0ba7b8127f13bd06a6d1d35f7ff3dee6aa7c95d2"
        status: pass
    human_judgment: false
  - id: D2
    description: A checksum-verified RetroArch 1.22.2 release loads Nesteroids, captures frame 60, and matches the runner's frame.
    requirement: GAME-02
    verification:
      - kind: e2e
        ref: "CI run 37860982545; artifact retroarch-e2e-frames (ID 11585683202)"
        status: pass
    human_judgment: false
  - id: D3
    description: Movie replay and libretro frame-parity subprocess checks pass on Windows and other supported hosts.
    requirement: GAME-03
    verification:
      - kind: integration
        ref: "Six hosted `ci`/`ci-msvc` legs in run 37860982545; local ci workflow 346/346"
        status: pass
    human_judgment: false
  - id: D4
    description: Loader fuzzing and full CPU vector checks remain recurring nightly gates.
    requirement: GAME-05
    verification:
      - kind: nightly
        ref: "Nightly run 37860982588 on exact head 0ba7b8127f13bd06a6d1d35f7ff3dee6aa7c95d2"
        status: pass
    human_judgment: false
---

# Phase 03 Plan 08: Automated CI Gates Summary

**Phase 03's game, movie, cross-platform hash, and released-RetroArch seams now have recurring machine-verifiable CI evidence.**

## Accomplishments

- The required six-platform matrix emits an identical, sorted 30-row inventory for game boot frames and DABG scripted-input frames. `hash-equality` verifies the expected keys, row schema, and byte-equal contents.
- `ci-required` now depends on the hash gate and `retroarch-e2e`. The RetroArch job downloads the pinned official release, checks its SHA-256 and version, isolates its user state, loads the manifest-listed game, and compares frame 60 with the runner.
- `retroarch-e2e-frames` preserves the asset record, runner image, and screenshot for failures and audit.
- The README explains the required hosted gates and the retained artifact. The project planning policy records automation-first verification and the zero-human-UAT default when machine checks can cover the behavior.
- Windows test launches no longer use a shell. The test-only helper uses `CreateProcess` on Windows and `fork`/`exec` on POSIX, including captured output for movie rejection checks.

## Verification

Local workflows on the final test code passed:

- `cmake --workflow --preset ci`: 346/346 tests; two optional local RetroArch launches skipped after the GUI session aborted.
- `cmake --workflow --preset asan`: 344/344 tests.
- `cmake --workflow --preset nofp`: 5/5 tests.
- `cmake --workflow --preset hygiene`: 8/8 tests.

Hosted verification is tied to implementation commit `0ba7b8127f13bd06a6d1d35f7ff3dee6aa7c95d2`:

- CI run `37860982545` passed all six platform builds/tests, ASan, no-float, hygiene, title validation, all-six hash equality, `retroarch-e2e`, and the final `CI required` aggregator.
- Nightly run `37860982588` passed `vectors-full` and `rom-loader-fuzz` on that same commit.
- RetroArch reported version 1.22.2. The downloaded DMG SHA-256 matched `81b79121ba26d539064ae13b4d0419a120c3d165afbe656cf5f5412b15fdb434`; its 1503-byte screenshot compared equal to the runner's frame 60.
- Artifact `retroarch-e2e-frames` (ID `11585683202`, SHA-256 `3865b17acb4f79e8d058ad36d74e7f90a95cf6144f41ec85945ff0187225e6d3`) is retained.

## Deviations and Fixes

- Hosted Windows CTest exposed command-shell quoting failures in the runner movie and libretro host tests. The direct test process helper fixed both while retaining output capture.
- The first required hash aggregation found its `awk` schema check expected seven fields; emitted rows have six. After correcting the selected fields, a rerun verified all six 30-row inventories byte for byte.
- Hygiene caught formatting issues in the new helper; clang-format 18 fixed them before the passing final run.

## UAT and Continuation

All product-visible acceptance in this plan is covered by CI, including real RetroArch loading and image comparison. No owner gameplay or screenshot UAT remains. Clean-room source provenance is the separate irreducible judgment documented in Phase 02.

After PR #18 is squash-merged, the next specific command is `$gsd-discuss-phase 04`.
