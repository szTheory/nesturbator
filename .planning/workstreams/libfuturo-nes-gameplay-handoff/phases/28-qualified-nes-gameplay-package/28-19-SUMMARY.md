---
phase: 28-qualified-nes-gameplay-package
plan: "19"
subsystem: installed-consumer
status: complete
requires: []
provides: [Base-API relocated DABG consumer]
affects: [28-20, 28-22, 28-24]
requirements-completed: []
completed: 2026-11-10
---

# Phase 28 Plan 19: relocated base-API consumer

The fixed DABG recipe passes through the relocated installed public C package on the approved NTSC base. No emulator source, public header, flags, protected fixture or workflow changed.

## Isolation and source

The orchestrator created the single checkout `build/phase28-d42-candidate` on `agent-phase28-d42-package` from `b25146c6efaac41c58f60555f59f0fe79a90ca68` through the installed `worktree.create` adapter. Its manifest and process logs are in ignored `build/phase28-d42-orchestrator` in the orchestration checkout. Initial product status was clean. Execution used explicit cwd binding, serially; no merge-back into the dirty MMC2 checkout occurred. Only this workstream's administrative context was copied unstaged.

Donor commit: `7a004f2a4f8833a23b820fbb834f295911561636`. No donor history was merged. Per-file donor blobs:

- `tests/consumer/main.c`: `206b9cdd4dd43c13965c08a14b0c559be9ce212a`
- `tests/consumer/CMakeLists.txt`: `0660ee8cd65bcdfde60402fcb859de3685fb8412`
- `tests/cmake/qualified_consumer.cmake`: `e4793af60d638aceb4455625f5a40dd292150944`
- `tests/CMakeLists.txt`: `1f2b61c66a052367a9f91831378029739507cd3a`
- `docs/nrom-gameplay.md`: `ec1f59ddcfdf0c67e5f091f3345de7c2e03e60ee`

## Changes and verification

- Task 1 commit `a7b64d5`: ordinary loader and metadata equality replace unavailable region APIs; the runtime report states `ntsc-only-base-api`. Existing moved-prefix import confinement, Windows path controls, malformed-load equality, exact-capacity/input and negative semantic controls remain.
- Task 2 commit `4a57d17`: guide and README record actual reset identity and observed gameplay, preserving source-map evidence and consumer ownership limits.
- `cmake --preset ci` and `cmake --build --preset ci --parallel 4` exited zero. This was focused configuration/build, not the full workflow.
- The initial `install.gameplay` check passed. The combined `install.consumer|install.gameplay` check and its required install fixture passed in under four seconds. Logs are `plan19-{configure,build,gameplay,boundaries}.log` in the ignored orchestrator directory.
- Source version 0.1.12, ABI 1, behavior 9, state format 3. The 110-frame recipe reaches level 1 at frame 59, player X 40→70 across frames 80–95, 238 changed pixels, 12,779 PCM samples (12,647 nonzero, peak 4,926). Thirty-frame refused-load continuation matches its untouched twin and reaches X=100. No-input/Start-only controls complete and reject progress; all positive integrity controls pass.
- Guide/report consistency and `git diff --check` passed after documentation edits. A one-time documentation write failed because the base had no docs directory; creating that directory corrected the edit. It did not run or fail a gameplay gate.

## Limits and next step

The candidate is still in preparation, not frozen or published. The one full local workflow and candidate/main hosted allowances remain unused. PKG-01/PROF-01 remain pending. No LibFuturo consumer, host, hardware or Playstead result is claimed. Continue with 28-20 in this same owned checkout; D-39 and all historical stops remain preserved.

Preparation history was replayed before freeze to satisfy the unchanged base clang-format 18 policy in every outgoing consumer blob. The replay changed only consumer formatting; exact final reviewed product bytes were checked afterward. The original-to-replayed identity map is retained in ignored orchestration evidence. No acceptance stage was consumed.
