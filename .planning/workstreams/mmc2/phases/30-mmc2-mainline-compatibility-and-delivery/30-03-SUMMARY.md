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

**The reviewed MMC2 source binding and original focused checks passed, but the initial full workflow failed in the build because two C/C++ public-header tests pinned state format 3. Their assertions were corrected and targeted checks now pass. The owner authorized exactly one follow-on full run, conditional on the amended plan/preflight review and corrected-candidate gates; that review is in progress.**

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

The single authorized `cmake --workflow --preset ci` started at 2026-11-19T21:16:02Z with a supervised 1500-second deadline. Configure completed. Build failed after 0.971 seconds with exit 1:

- `tests/header/header_c.c:28`: `_Static_assert(NESTURBATOR_STATE_FORMAT_VERSION == 3, ...)` conflicts with declared format 4.
- `tests/header/header_cxx.cpp:28`: the same obsolete C++ assertion conflicts with format 4.

Evidence is retained in `build/ci/phase30-03-full.log`, `phase30-03-full.json`, and `phase30-03-full-started.json`. The full CTest, campaign and package stages did not run, so no new campaign metadata, campaign report, JUnit acceptance or deliverable archives exist for this attempt. No second full invocation, PR, merge, release or publication occurred; the only post-failure source change is the narrow header assertion correction below.

The focused build omitted `header.c` and `header.cxx`; these two compile-only pins were also absent from the reviewed plan file scopes. This is a verification omission, not an emulation mismatch. The failed attempt remains terminal and preserved.

## Post-failure targeted fix

After recording the terminal full-workflow result, updated only the obsolete state-format assertions in `tests/header/header_c.c` and `tests/header/header_cxx.cpp` from 3 to 4. The targeted command `cmake --build build/ci --target header.c header.cxx` exited 0; the build log is `build/ci/phase30-03-header-build.log`. The exact targeted CTest command `ctest --test-dir build/ci --output-on-failure --no-tests=error -R '^(header[.]c|header[.]cxx)$'` passed both tests (2/2, 0.01 seconds); the log is `build/ci/phase30-03-header-ctest.log`.

These narrow checks verify the corrected public-header smoke tests only. They do not validate the changed candidate against the AccuracyCoin binding/preflight or satisfy MMC2-11. The prior candidate fingerprint and source-bound preflight are stale for any new acceptance attempt.

## Authorized follow-on conditions

On 2026-11-19, the owner explicitly authorized exactly one additional `cmake --workflow --preset ci` with the same 1500-second hard cap, conditional on updating and independently reviewing the plan/preflight and passing all corrected-candidate gates. Plan 30-03 now adds both header files to its file scope, requires their build and CTest, and the preflight validates the exact commands, two test names, source hashes, logs and log digests. Independent review initially found two gaps; the follow-up review passed after both were corrected. The reviewed product-diff SHA-256 is `d2a36bbd4ed05e11fe3d11fdfc0888e9dce14c68d748fed3feff90bfbd157fef`. Before using the authorization, refresh the local reviewed-diff pin, then require the source-bound preflight and focused controls to pass and freeze the candidate. This authorization permits no third full workflow. MMC2-11 and MMC2-12 remain incomplete until the corrected candidate's full acceptance and delivery gates pass.

## Preserved boundaries

Phase 29 attempts and recovery routes remain stopped. Historical AccuracyCoin evidence and report pins were not changed. No hardware, commercial-title, broad mapper-9, MMC4, performance, RM-017 publisher receipt or LibFuturo/Playstead consumer claim is made. Continue in this same Phase 30 checkout; return to the primary project checkout only after delivery and safe reconciliation of its existing dirty work.

## Checkpoint status

The frozen initial candidate, its failed full-workflow logs and targeted header-gate logs are recorded. The two header test files differ from the initial candidate by the state-format assertion correction. One follow-on full workflow is owner-authorized under the amended plan, subject to the conditions above; it has not run yet. This is not plan, phase or delivery completion.
