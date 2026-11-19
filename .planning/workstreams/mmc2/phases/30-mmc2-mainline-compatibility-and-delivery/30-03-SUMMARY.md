---
phase: 30-mmc2-mainline-compatibility-and-delivery
plan: "03"
subsystem: acceptance-and-delivery
tags: [MMC2, AccuracyCoin, source-binding, delivery]
status: executing
requires: [30-01, 30-02]
provides:
  - Exact current MMC2 source admission with immutable historical evidence.
  - Fresh campaign metadata rejection controls and independent source review.
affects: [acceptance, delivery]
actuals:
  tokens: 9735
  tasks: 1
  commits: 1
plan_head_before: 3a648a524a21dfbbec8387b2ea393e4e67d00327
plan_head_after: 48f6f054ed363ea50b5da0bfe96f960230f673b7
key-files:
  modified:
    - tests/cmake/accuracycoin_input_fingerprint.cmake
    - tests/cmake/accuracycoin_input_fingerprint_controls.cmake
    - tests/cmake/accuracycoin_activation.cmake
    - tests/accuracy/input-binding-v1.json
    - tests/libretro/libretro_host.c
    - tests/header/header_c.c
    - tests/header/header_cxx.cpp
    - tests/cmake/phase21_hosted_evidence_controls.cmake
    - tests/cmake/vector_api_policy.cmake
    - tests/cmake/info_claims.cmake
    - README.md
    - include/nesturbator.h
    - runner/main.c
completed: 2026-11-19
---

# Phase 30 Plan 03: Acceptance and delivery checkpoint

**The initial full workflow failed on stale header state-format pins. The owner-authorized follow-on passed the campaign but failed seven compatibility checks before packaging; all causes were fixed. The owner then authorized one third full workflow. Its final preflight and independent amendment review passed, all 702 CTest entries and the AccuracyCoin campaign/activation passed, and CPack produced all three local packages. The post-run source fingerprint matched the frozen candidate. A standard-depth review of the 27-file product scope found no blockers or warnings. The one-run authorization is spent; no retry is authorized. PR, merge and release remain pending.**

## Completed work

- Preserved the original 56-path historical manifest, metadata, scoreboard, report digest and provenance fields. The exact current closure has 57 paths, with seven explicitly admitted changed product inputs and external checker/registration hashes.
- Added same-run campaign metadata validation against recomputed current input hash/count and actual report digest. Standalone owned fixtures reject stale, missing, malformed, duplicate and changed-source/report cases without running a campaign.
- Added rejection of escaped JSON object member names before parsing, including escaped duplicate top-level and nested path keys.
- Independent review fixed restored PCM capture bounds and stale support lists in the README, public header and runner. All support statements retain the bounded NES 2.0 mapper-9/submapper-0 profile. See `30-REVIEW.md`.
- Committed the clean measured candidate as `7d4f860d90312277fe1bd395bf3593f8a361e22e` (`fix(30-03): bind reviewed MMC2 acceptance inputs`).

## Source identity and focused evidence

- Live main and merge base remained `8923b66cca16739c5b402643798515cf5537ae22`.
- Frozen and post-run AccuracyCoin input fingerprint: `d45734c098d790da3aad63eb25d1cba37e388e62b7e7f5d00aea5ec7525bbb01`, count 57. The successful third workflow ran on candidate commit `48f6f054ed363ea50b5da0bfe96f960230f673b7`; the fingerprint matched before and after.
- Independently reviewed product diff digest for the union of all three plan scopes: `76ff77dda059d326cdae01ffd5d8f2fde4e9465e31dc9527d410c172436077b4`.
- Standalone binding/freshness controls passed: `build/ci/phase30-03-controls.log`.
- Focused targets rebuilt, then the exact mapper/state/provider/parity CTest set passed: `build/ci/phase30-03-final-focused-build.log`, `build/ci/phase30-03-final-runner-build.log`, and `build/ci/phase30-03-focused-final2.log` (6.62 seconds).
- Full-base whitespace check and clang-format 18 checks passed. Planning-only trailing whitespace was trimmed to satisfy the complete-base check.
- Explicit FULL=ON configure and complete CTest inventory passed, with 702 tests, campaign fixture linkage and original caps: `build/ci/phase30-03-freeze-configure.log` and `build/ci/phase30-03-full-inventory.log`.
- Final throwaway preflight passed on the clean candidate after the amended plan explicitly added it as a verification command. The supervisor record/log are `build/ci/phase30-03-final-preflight.json` and `phase30-03-final-preflight.log`; the report is `build/ci/phase30-03-preflight.json`. It checked the reviewed diff, pinned base, 702-test inventory, all 19 negative guards, historical bytes, compatibility pins, header gate, focused evidence, campaign fixture and resource caps.

## Full workflow failure and stop

The initial `cmake --workflow --preset ci` started at 2026-11-19T21:16:02Z with a supervised 1500-second deadline. Configure completed. Build failed after 0.971 seconds with exit 1:

- `tests/header/header_c.c:28`: `_Static_assert(NESTURBATOR_STATE_FORMAT_VERSION == 3, ...)` conflicts with declared format 4.
- `tests/header/header_cxx.cpp:28`: the same obsolete C++ assertion conflicts with format 4.

Evidence is retained in `build/ci/phase30-03-full.log`, `phase30-03-full.json`, and `phase30-03-full-started.json`. The full CTest, campaign and package stages did not run on this first attempt. The later owner-authorized follow-on is recorded separately below.

The focused build omitted `header.c` and `header.cxx`; these two compile-only pins were also absent from the reviewed plan file scopes. This is a verification omission, not an emulation mismatch. The failed attempt remains terminal and preserved.

## Post-failure compatibility fixes

After the first failure, changed only the obsolete state-format assertions in `tests/header/header_c.c` and `tests/header/header_cxx.cpp` from 3 to 4. After a target-specific clean, `cmake --build build/ci --target header.c header.cxx` compiled both source files, and CTest passed both header smoke tests. Evidence is in `build/ci/phase30-03-header-gate.json`, `phase30-03-header-build.log`, and `phase30-03-header-ctest.log`.

The follow-on workflow then exposed three compatibility-check issues: the Phase 21 test still asserted behavior revision 9; the temporary public-declaration digest still described revision 9/state format 3; and the info-claims guard treated the new MMC2 clause as a duplicate MMC3 clause. The follow-on also showed that the info-claims unsupported-board self-test expected the old diagnostic. Corrected those pins and made the info-claims guard count only `Supported NTSC MMC3:` clauses, preserving duplicate-MMC3 rejection while allowing the separate MMC2 fact. These test-only changes do not alter the AccuracyCoin measured-source closure.

The eight directly affected tests subsequently passed 8/8. The expanded focused CTest set, including all compatibility/header/mapper/state/provider/parity/libretro checks, passed 24/24 in 43.8 seconds; log and command record are `build/ci/phase30-03-followon-focused-ctest.log` and `.json`. The source-bound fingerprint remained `d45734c098d790da3aad63eb25d1cba37e388e62b7e7f5d00aea5ec7525bbb01` with 57 inputs. It was refreshed before and after the authorized third workflow and matched both times.

## Authorized follow-on conditions

On 2026-11-19, the owner authorized one follow-on `cmake --workflow --preset ci` under a 1500-second cap. It started from frozen candidate `3cf84ceaf1aa9e8451d47b3a100598ccf917e5ac`, completed configure/build and 702 CTest entries in 388.8 seconds, and exited 8 because seven tests failed: `phase21.hosted_evidence.controls`, `vectors.api_policy`, `vectors.api_policy.self_test`, `libretro.info_claims.self_test`, `libretro.info_support`, `libretro.info_staged`, and `libretro.info_archives.self_test`. `accuracycoin.campaign` passed. Workflow steps 1–3 completed; package step 4 did not start, so no release archives were produced. The full-run record and log are `build/ci/phase30-03-followon-full-started.json`, `phase30-03-followon-full.json`, and `phase30-03-followon-full.log`. The post-run source fingerprint matched the frozen run fingerprint, count 57. That authorization is spent. The failed test contracts have since been corrected and their expanded focused set passes. The owner has now authorized one additional full workflow (third overall), conditional on independent review of the final plan/preflight amendment and passing source-bound gates. This third invocation is terminal; no retry is authorized. MMC2-11 and MMC2-12 remain incomplete until current acceptance/delivery evidence exists.

## Third authorized full workflow: local acceptance passed

After the final plan/preflight amendment passed independent review and the source-bound preflight passed, the owner-authorized third workflow ran once on candidate `48f6f054ed363ea50b5da0bfe96f960230f673b7`. It began at `2026-11-19T22:26:22Z`, completed in 460.358 seconds under the 1500-second cap, and exited 0. Evidence is in `build/ci/phase30-03-third-full.json` and `phase30-03-third-full.log`.

All four workflow steps completed: configure, build, CTest and package. CTest passed 702/702 entries; `accuracycoin.campaign` and `accuracycoin.activation` passed, with the campaign report pin unchanged. The final CTest summary is in the full workflow log. The source fingerprint before and after was `d45734c098d790da3aad63eb25d1cba37e388e62b7e7f5d00aea5ec7525bbb01` with 57 inputs.

CPack generated the library, runner and libretro archives. The library package contains the public header, static library, CMake config and pkg-config file; the runner archive contains `nesturbator-run`; the libretro archive contains the module and `.info` file. The local archive manifest and SHA-256 values are in `build/ci/phase30-03-third-package-manifest.json`. No PR, merge or release has started. The single new-run authorization is spent; do not retry local full CI without another explicit owner decision and independent plan/preflight review.

The final standard-depth review covered all 27 product files in the union of Plans 30-01/02/03 at candidate `48f6f054ed363ea50b5da0bfe96f960230f673b7`. It reported zero blockers, warnings or info findings; see [30-REVIEW.md](30-REVIEW.md). No tests or workflows were run by the reviewer.

Read-only GitHub triage found no open MMC2 or mapper-9 PR. PR [#68](https://github.com/szTheory/nesturbator/pull/68) remains the separate provider qualification route; it is open and marked unmergeable, and was not modified. Live `main` still pointed to the pinned base at the latest read. Revalidate it immediately before opening the MMC2 PR.

## Preserved boundaries

Phase 29 attempts and recovery routes remain stopped. Historical AccuracyCoin evidence and report pins were not changed. No hardware, commercial-title, broad mapper-9, MMC4, performance, RM-017 publisher receipt or LibFuturo/Playstead consumer claim is made. Continue in this same Phase 30 checkout; return to the primary project checkout only after delivery and safe reconciliation of its existing dirty work.

## Checkpoint status

All three full-workflow outcomes and their source identities are recorded: two terminal failures followed by one successful, terminal local acceptance. The third run passed all 702 CTest entries and package generation on the same frozen fingerprint. Final code review is clean. No PR, merge or release has started; MMC2-11 local acceptance is complete, but MMC2-12 and Phase 30 delivery remain incomplete. No further full workflow is authorized by the one-run decision.
