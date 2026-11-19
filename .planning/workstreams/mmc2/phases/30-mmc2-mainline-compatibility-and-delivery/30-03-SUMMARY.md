---
phase: 30-mmc2-mainline-compatibility-and-delivery
plan: "03"
subsystem: acceptance-and-delivery
tags: [MMC2, AccuracyCoin, source-binding, blocked]
status: executing
requires: [30-01, 30-02]
provides:
  - Exact current MMC2 source admission with immutable historical evidence.
  - Fresh campaign metadata rejection controls and independent source review.
affects: [acceptance, delivery]
actuals:
  tokens: 9735
  tasks: 0
  commits: 1
plan_head_before: 3a648a524a21dfbbec8387b2ea393e4e67d00327
plan_head_after: 7d4f860d90312277fe1bd395bf3593f8a361e22e
key-files:
  modified:
    - tests/cmake/accuracycoin_input_fingerprint.cmake
    - tests/cmake/accuracycoin_input_fingerprint_controls.cmake
    - tests/cmake/accuracycoin_activation.cmake
    - tests/accuracy/input-binding-v1.json
    - tests/libretro/libretro_host.c
    - README.md
    - include/nesturbator.h
    - runner/main.c
completed: 2026-11-19
---

# Phase 30 Plan 03: Acceptance and delivery checkpoint

**The initial full workflow failed during build. The owner-authorized follow-on completed all 702 CTest entries in 388.8 seconds but failed seven compatibility checks; the campaign test passed and packaging did not start. The stale compatibility checks are now fixed, and the expanded 24-test focused set passes. The owner has authorized exactly one third full workflow after independent review of this final plan/preflight amendment and passing source-bound gates; no retry is authorized.**

## Completed work

- Preserved the original 56-path historical manifest, metadata, scoreboard, report digest and provenance fields. The exact current closure has 57 paths, with seven explicitly admitted changed product inputs and external checker/registration hashes.
- Added same-run campaign metadata validation against recomputed current input hash/count and actual report digest. Standalone owned fixtures reject stale, missing, malformed, duplicate and changed-source/report cases without running a campaign.
- Added rejection of escaped JSON object member names before parsing, including escaped duplicate top-level and nested path keys.
- Independent review fixed restored PCM capture bounds and stale support lists in the README, public header and runner. All support statements retain the bounded NES 2.0 mapper-9/submapper-0 profile. See `30-REVIEW.md`.
- Committed the clean measured candidate as `7d4f860d90312277fe1bd395bf3593f8a361e22e` (`fix(30-03): bind reviewed MMC2 acceptance inputs`).

## Source identity and focused evidence

- Live main and merge base remained `8923b66cca16739c5b402643798515cf5537ae22`.
- Frozen and post-run AccuracyCoin input fingerprint: `d45734c098d790da3aad63eb25d1cba37e388e62b7e7f5d00aea5ec7525bbb01`, count 57. Product bytes and HEAD were unchanged after failure.
- Reviewed complete planned product diff digest: `8b6b2aabcdabbf632c182a515c35d1933fd92c537cf479b073452761b99ce85b`.
- Standalone binding/freshness controls passed: `build/ci/phase30-03-controls.log`.
- Focused targets rebuilt, then the exact mapper/state/provider/parity CTest set passed: `build/ci/phase30-03-final-focused-build.log`, `build/ci/phase30-03-final-runner-build.log`, and `build/ci/phase30-03-focused-final2.log` (6.62 seconds).
- Full-base whitespace check and clang-format 18 checks passed. Planning-only trailing whitespace was trimmed to satisfy the complete-base check.
- Explicit FULL=ON configure and complete CTest inventory passed, with 702 tests, campaign fixture linkage and original caps: `build/ci/phase30-03-freeze-configure.log` and `build/ci/phase30-03-full-inventory.log`.
- Throwaway preflight passed after freeze, including negative guards for source/base, paths/hunks, historical bytes, compatibility fields, source root, inventory, fixtures and caps: `build/ci/phase30-03-preflight.json` and `.log`. It verifies declared version fields but did not compile the separate header contract targets.

## Full workflow failure and stop

The initial `cmake --workflow --preset ci` started at 2026-11-19T21:16:02Z with a supervised 1500-second deadline. Configure completed. Build failed after 0.971 seconds with exit 1:

- `tests/header/header_c.c:28`: `_Static_assert(NESTURBATOR_STATE_FORMAT_VERSION == 3, ...)` conflicts with declared format 4.
- `tests/header/header_cxx.cpp:28`: the same obsolete C++ assertion conflicts with format 4.

Evidence is retained in `build/ci/phase30-03-full.log`, `phase30-03-full.json`, and `phase30-03-full-started.json`. The full CTest, campaign and package stages did not run on this first attempt. The later owner-authorized follow-on is recorded separately below.

The focused build omitted `header.c` and `header.cxx`; these two compile-only pins were also absent from the reviewed plan file scopes. This is a verification omission, not an emulation mismatch. The failed attempt remains terminal and preserved.

## Post-failure compatibility fixes

After the first failure, changed only the obsolete state-format assertions in `tests/header/header_c.c` and `tests/header/header_cxx.cpp` from 3 to 4. After a target-specific clean, `cmake --build build/ci --target header.c header.cxx` compiled both source files, and CTest passed both header smoke tests. Evidence is in `build/ci/phase30-03-header-gate.json`, `phase30-03-header-build.log`, and `phase30-03-header-ctest.log`.

The follow-on workflow then exposed three compatibility-check issues: the Phase 21 test still asserted behavior revision 9; the temporary public-declaration digest still described revision 9/state format 3; and the info-claims guard treated the new MMC2 clause as a duplicate MMC3 clause. The follow-on also showed that the info-claims unsupported-board self-test expected the old diagnostic. Corrected those pins and made the info-claims guard count only `Supported NTSC MMC3:` clauses, preserving duplicate-MMC3 rejection while allowing the separate MMC2 fact. These test-only changes do not alter the AccuracyCoin measured-source closure.

The eight directly affected tests subsequently passed 8/8. The expanded focused CTest set, including all compatibility/header/mapper/state/provider/parity/libretro checks, passed 24/24 in 43.8 seconds; log and command record are `build/ci/phase30-03-followon-focused-ctest.log` and `.json`. The prior source-bound fingerprint remains `d45734c098d790da3aad63eb25d1cba37e388e62b7e7f5d00aea5ec7525bbb01` with 57 inputs; rerun the fingerprint and preflight after the final edits, before the newly authorized full workflow.

## Authorized follow-on conditions

On 2026-11-19, the owner authorized one follow-on `cmake --workflow --preset ci` under a 1500-second cap. It started from frozen candidate `3cf84ceaf1aa9e8451d47b3a100598ccf917e5ac`, completed configure/build and 702 CTest entries in 388.8 seconds, and exited 8 because seven tests failed: `phase21.hosted_evidence.controls`, `vectors.api_policy`, `vectors.api_policy.self_test`, `libretro.info_claims.self_test`, `libretro.info_support`, `libretro.info_staged`, and `libretro.info_archives.self_test`. `accuracycoin.campaign` passed. Workflow steps 1–3 completed; package step 4 did not start, so no release archives were produced. The full-run record and log are `build/ci/phase30-03-followon-full-started.json`, `phase30-03-followon-full.json`, and `phase30-03-followon-full.log`. The post-run source fingerprint matched the frozen run fingerprint, count 57. That authorization is spent. The failed test contracts have since been corrected and their expanded focused set passes. The owner has now authorized one additional full workflow (third overall), conditional on independent review of the final plan/preflight amendment and passing source-bound gates. This third invocation is terminal; no retry is authorized. MMC2-11 and MMC2-12 remain incomplete until current acceptance/delivery evidence exists.

## Preserved boundaries

Phase 29 attempts and recovery routes remain stopped. Historical AccuracyCoin evidence and report pins were not changed. No hardware, commercial-title, broad mapper-9, MMC4, performance, RM-017 publisher receipt or LibFuturo/Playstead consumer claim is made. Continue in this same Phase 30 checkout; return to the primary project checkout only after delivery and safe reconciliation of its existing dirty work.

## Checkpoint status

Both prior full-workflow attempts, their logs and source identities are recorded. The second run's campaign test passed, but seven compatibility checks failed before packaging. All seven directly affected checks now pass within the expanded focused set. The product tree has compatibility-test fixes and a corresponding plan/preflight amendment awaiting independent review. The owner has authorized exactly one third full workflow after review and passing source-bound gates; no retry is authorized by that decision. No PR, merge or release has started; this is not plan, phase or delivery completion.
