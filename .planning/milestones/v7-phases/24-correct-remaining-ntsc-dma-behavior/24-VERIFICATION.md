---
phase: 24-correct-remaining-ntsc-dma-behavior
verified: 2026-11-01T19:58:13Z
status: passed
score: "9/9 must-haves verified"
covered_files:
  - .github/workflows/ci.yml
  - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-01-PLAN.md
  - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-01-SUMMARY.md
  - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-02-PLAN.md
  - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-02-SUMMARY.md
  - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-03-PLAN.md
  - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-03-SUMMARY.md
  - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-04-PLAN.md
  - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-04-SUMMARY.md
  - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-05-PLAN.md
  - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-05-SUMMARY.md
  - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-ACCEPTANCE.md
  - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-AUTOMATION-DISPOSITION.md
  - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-CI-RUN-20261101.md
  - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-DMA-EVIDENCE.md
  - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-HOSTED-EVIDENCE.md
  - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-REVIEW-DISPOSITION.md
  - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-REVIEW.md
  - CMakeLists.txt
  - README.md
  - include/nesturbator.h
  - libretro/CMakeLists.txt
  - src/apu.c
  - src/bus.c
  - src/internal.h
  - src/state.c
  - tests/CMakeLists.txt
  - tests/accuracy/baseline-metadata.txt
  - tests/accuracy/measurements.md
  - tests/accuracy/scoreboard.txt
  - tests/cmake/phase24_dma_gap_evidence.cmake
  - tests/core/test_state_fork.c
  - tests/core/test_state_reject.c
  - tests/cpu/test_bus.c
  - tests/perf/allow.txt
covered_digest: "v4:sha256:05a5ab312ca9db9b2e55f8a063ad2265a02d987aaf6c101941084384afd33ee0"
behavior_unverified: 0
overrides_applied: 0
re_verification:
  previous_status: gaps_found
  previous_score: "8/9"
  gaps_closed:
    - "Protected PASS authority, current-candidate hosted CI, six-platform hash equality, and unchanged performance evidence."
  gaps_remaining: []
  regressions: []
decision_coverage:
  honored: 8
  total: 8
  not_honored: []
advisory: []
human_verification: []
---

# Phase 24: Correct Remaining NTSC DMA Behavior — Verification Report

**Phase Goal:** Hosts using the library, runner or libretro adapter receive source-supported NTSC DMA behavior for the five selected failures, with existing passing behavior and deterministic continuation preserved.
**Verified:** 2026-11-01T19:58:13Z
**Status:** `passed`
**Re-verification:** Yes — the exact-candidate acceptance evidence closed the prior hosted-evidence gap.

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|---|---|---|
| 1 | All five selected rows pass at the unchanged AccuracyCoin pin for the declared target. | ✓ VERIFIED | The candidate-bound report for `d54e21898fbf7bb3984fd2d33aa563932b464581` records individual PASS results for Landing `0x01`, DMC+OAM `0x01`, Bus Conflicts `0x01`, Explicit Abort `0x01`, and the pinned accepted Implicit Abort `0x09` variant at ROM pin `6bee919529289c555803b612b8c9e4f7eaaeeabb`. `src/apu.c`, `src/bus.c`, `src/internal.h`, and `src/state.c` match the tested candidate blobs in this checkout. This establishes the pinned test contract, not silicon behavior. |
| 2 | Each corrected behavior has a separate source-backed cycle explanation and an owned fixture that accepts the correction and rejects a named wrong model. | ✓ VERIFIED | `24-DMA-EVIDENCE.md` keeps five causes distinct and separates hardware references, test expectations, observed events, and wrong-model controls. `bus.unit` passed, including the final `$FFC0` sample GET and resumed `$4000` CPU read at cycle 38,166. The offline evidence controls also passed, including overflow, missing-read, corrupt-data, and symlink-containment rejection cases. |
| 3 | DMC GET collisions preserve OAM bytes, resume position, and source/destination timing through interior and final windows. | ✓ VERIFIED | Production `src/bus.c:oam_dma` steals only the DMC GET source slot while preserving OAM stages. The owned matrix checks 256 writes and interior/final resume positions; the fresh `bus.unit` run passed. |
| 4 | DMC bus conflicts preserve internal register effects and external sample data/latch behavior. | ✓ VERIFIED | `src/bus.c:dmc_sample_get` fetches the mapper sample, updates the external latch, and applies internal register selection using the parked CPU address. The owned fixture checks sample address/data/phase, IRQ acknowledgement, controller shifts, returned value and latch; `bus.unit` passed. |
| 5 | Explicit and implicit aborts remain distinct from write-deferred retries, with the selected early-G model and pinned accepted implicit outcome preserved. | ✓ VERIFIED | The abort countdown path is separate from `dma_pending`; neighboring phase/counter windows, write controls, looping and length controls are covered. README and source comments call early-G ordering an inference and avoid a universal chip claim. The pinned campaign accepts implicit result `0x09`. |
| 6 | Changed in-flight DMA state resumes identically at production-reachable native save boundaries, with format/revision and compatibility effects explicit. | ✓ VERIFIED | `src/state.c` serializes and validates the affected state. The public header records native format 6, behavior revision 14 and ABI 1, and documents exact-format loading with recorded-only behavior revision. Fresh `core.state_fork --dma`, `core.state_reject --short` and the `state.format_pin` / `core.state_fork.entry` CTests passed. |
| 7 | Public documentation explains the behavior and state consequences, and owned regressions are registered for the library, runner and libretro deliverables. | ✓ VERIFIED | README and public-header comments document the behavior and compatibility limits. CMake connects the shared library to the runner and libretro adapter and registers the bus and state tests. The exact-candidate hosted CI passed; the documentation-focused CTests also passed locally. |
| 8 | Protected PASS behavior is independently verified, six-platform frame/PCM/transition inventories match, and the existing performance gate passes unchanged. | ✓ VERIFIED | On the exact candidate and base, protected-main artifacts independently bind main revision `b25146c6efaac41c58f60555f59f0fe79a90ca68`; all 119 protected PASS keys pass in every platform lane and remain PASS in the candidate report. Six Linux/macOS/Windows x64/arm64 hash artifacts are byte-identical (SHA-256 `cd90c2d7227ad306559c30caaaa934b3a6290709741bcf77919767a32c7266fe`). Eighteen raw Cachegrind captures cover three ROMs × base_a/base_b/head × 60/360 frames; the three comparisons pass at +0.18% to +0.25%, within the unchanged +1% allowance. |
| 9 | Acceptance evidence and authority boundaries are recorded without inferring acceptance from local fixtures. | ✓ VERIFIED | PR #63 remains open at candidate `d54e21898fbf7bb3984fd2d33aa563932b464581`. Its exact-head CI run `41336227793` and Nightly run `41336227847` passed; receipt v16 binds the candidate, tested merge, base, runs and immutable artifacts. There was no merge or release. Local synthetic controls are identified as controls, and no test or hash is presented as silicon proof. |

**Score:** 9/9 observable truths verified; 0 behavior-unverified.

The result is tied to the existing AccuracyCoin pin, declared target, candidate and recorded evidence. The implicit-abort `0x09` outcome is a pinned accepted variant; early-G ordering remains an inference. Nothing here asserts universal silicon behavior. The older interrupted run remains unknown; this verification did not restart it, dispatch another campaign, or retry a workflow.

## Required Artifacts and Key Links

| Artifact or link | Status | Evidence |
|---|---|---|
| `src/apu.c`, `src/bus.c`, `src/internal.h`, `src/state.c` | ✓ VERIFIED | Core implementation is linked into the library and matches the tested candidate blobs. |
| `include/nesturbator.h`, `README.md` | ✓ VERIFIED | Behavior, state format/revision and compatibility policy are documented. |
| `tests/cpu/test_bus.c`, `tests/core/test_state_fork.c`, `tests/core/test_state_reject.c` | ✓ VERIFIED | Concrete bus event, continuation and refusal assertions are registered and exercised. |
| `tests/cmake/phase24_dma_gap_evidence.cmake` | ✓ VERIFIED | Offline receipt checks and synthetic rejection controls pass; synthetic data is not acceptance evidence. |
| `24-DMA-EVIDENCE.md`, `24-ACCEPTANCE.md`, `24-HOSTED-EVIDENCE.md` | ✓ VERIFIED | Source causes, local limits, exact-candidate receipt and protected/hash/performance evidence are linked. |
| `.github/workflows/ci.yml` → CMake/CTest and hosted artifacts | ✓ WIRED | Exact candidate runs completed successfully, with required evidence retained in immutable run artifacts. |
| CMake library → runner and libretro adapter | ✓ WIRED | Both host deliverables consume the same core library. |

## Verification Runs

Focused checks freshly run in this verification step:

| Check | Result |
|---|---|
| `ctest --test-dir build/ci -R '^(bus\.unit|docs\.reproduction|docs\.reproduction\.self_test|state\.format_pin|core\.state_fork\.entry)$' --no-tests=error --output-on-failure` | 5/5 passed. |
| `build/ci/tests/core.state_reject --short tests/roms/dabg.nes tests/roms/runner.nes tests/roms/hm/M4_P128K_CR8K.nes` | Passed old/future-format refusal controls. |
| `build/ci/tests/core.state_fork --dma` | Passed DMA continuation comparisons and negative controls. |
| `cmake -DMODE=controls -DSOURCE_REF=. -DROOT=build/phase24-audit-controls-20261101 -DBUS=build/ci/tests/bus.unit -P tests/cmake/phase24_dma_gap_evidence.cmake` | Passed synthetic rejection controls. |
| `build/ci/tests/bus.unit --dma-trace landing` | Passed; retained the final `$FFC0` GET and resumed `$4000` read. |

The candidate-bound receipt also records full CI and Nightly success. This step did not rerun full local CI or the AccuracyCoin campaign. It reused the exact-head hosted artifacts, freshly downloaded and checked the artifact contents and candidate identities, then ran the focused local checks above. The previous project stop on restarting the older interrupted workflow/campaign remains in force.

## Requirement Coverage

| Requirement | Status | Evidence |
|---|---|---|
| DMA-01 | ✓ SATISFIED | Five individual pinned selected rows pass, including the accepted implicit-abort variant. |
| DMA-02 | ✓ SATISFIED | Distinct source-backed causes, owned fixtures and wrong-model controls are present and pass. |
| DMA-03 | ✓ SATISFIED | Protected keys, six-platform output equality and unchanged performance allowance pass on the exact candidate evidence. |
| DMA-04 | ✓ SATISFIED | Public native-state continuation, refusal controls, format/revision disposition and documentation pass. |

All four requirements map to Phase 24, with no orphaned phase requirement. The branch-wide inherited whitespace diagnostics remain recorded in the acceptance history; scoped changes and required evidence controls pass. This historical hygiene warning does not invalidate the behavioral, compatibility or candidate-bound acceptance evidence.

## Human Verification

None remains. This core-library phase has no visual or subjective acceptance step, and each observable acceptance condition has automated evidence. The former Plan 24-04 authority checkpoint is an operational stop: the exact-candidate runs already have their separately recorded authorization and results. The automation disposition documents the agent review of the three prohibition/claim labels. No human UAT pass, retroactive approval or risk acceptance is claimed.

---

_Verified: 2026-11-01T19:58:13Z_
_Verifier: Codex phase verification_
