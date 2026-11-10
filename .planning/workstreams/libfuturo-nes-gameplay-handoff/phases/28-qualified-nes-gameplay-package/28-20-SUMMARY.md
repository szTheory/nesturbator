---
phase: 28-qualified-nes-gameplay-package
plan: "20"
subsystem: provider-profile
status: complete
requires: [28-19]
provides: [Checked NTSC base-API provider profile]
affects: [28-21, 28-22, 28-24]
requirements-completed: []
completed: 2026-11-10
---

# Phase 28 Plan 20: checked base-API profile

The NTSC-only profile agrees with source and a real relocated-package report. It remains candidate-only until live publication evidence exists.

## Changes

Task 1 commit `1d263d9` ports the committed donor profile/checker and registers the existing profile checks. The profile records ABI 1, behavior 9 and state format 3, ordinary `nesturbator_load_cartridge`, no runtime region query/preference, NES2 timing byte 12 zero and iNES reserved bytes 8–15 zero. Source checks read unchanged base loader/arithmetic/header; they do not alter it. The guide digest binds plan 19's fresh observations.

Task 2 adds named negative runtime-report controls for route, behavior/state identity, rates, sample counts and removed progress, plus scoped API/testing/reproduction documentation. Existing strict types, recipe arrays/masks, predicate and qualification-limit controls remain. Borrowed regional fields and donor compatibility 18/7 are rejected. Synthetic reports test contract rejection only, never game or publication success.

## Verification and evidence

- Configure, provider.profile, provider.profile.controls and actual install.gameplay passed. The three checks took under four seconds.
- MODE=contract accepts the fresh real `gameplay.json` from the latest qualified moved-prefix scratch directory. Its report has the actual 0.1.12/ABI1/behavior9/format3 identity and successful fixed DABG predicates.
- Additional runtime controls first exposed variable shadowing in the self-test's recipe loop: it overwrote the function's input profile filename. Preserving that filename separately corrected the port-control implementation defect. The bounded affected profile checks then passed in under one second; this was pre-freeze development, not a gameplay/full-workflow/CI acceptance failure.
- Fresh source/runtime contract validation and base-to-current `git diff --check` passed. A trailing documentation-registration blank line was removed. No emulator/header/build-flag/workflow/protected fixture change occurred.
- Detailed logs: ignored `build/phase28-d42-orchestrator/plan20-{configure,profile,controls,controls-corrected}.log`. Donor file blobs are recorded below; new scoped documentation is owned here.

- `docs/provider-ntsc.json`: `bdc97a487e2af8e33dcb76aca84230eb6812fc9e`
- `docs/provider-ntsc.md`: `8cef93ecf23a37b71b7723b46f80b08bcbbf6975`
- `tests/cmake/provider_profile.cmake`: `db60e072da1fd48d76a88dd09db02db338f2be3a`
- `tests/CMakeLists.txt`: `1f2b61c66a052367a9f91831378029739507cd3a`

## Next step and limits

Continue 28-21 rights/receipt controls in the same single owned checkout. No source freeze, full local workflow, candidate/main hosted run or publication has started. All three acceptance allowances remain unused. No hardware, LibFuturo installed consumer/host/gameplay or Playstead acceptance is claimed; D-39 remains terminal.
