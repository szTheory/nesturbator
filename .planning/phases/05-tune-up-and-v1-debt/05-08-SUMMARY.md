---
phase: 05-tune-up-and-v1-debt
plan: 08
subsystem: build-ci
tags: [gcc-14, wconversion, nightly, gap-closure]
requires: []
provides:
  - gcc-14 clean compile of phase 5 sources under -Werror (ci, nofp)
  - suite-flake timeout at the WR-02 30-minute ceiling
affects: [PR #25 Linux legs, nightly suite-flake, vectors-full]
tech-stack:
  added: []
  patterns: [explicit fixed-width cast on ternary assigned to uint8_t]
key-files:
  created: []
  modified:
    - tests/core/test_reset.c
    - .github/workflows/nightly.yml
key-decisions:
  - "Suite-flake timeout 30 minutes until a cold run is measured"
requirements-completed: [TUNE-01, TUNE-02, TUNE-06]
status: complete
duration: 6 min
completed: 2026-10-10
commits: 2
plan_head_before: 3c6030672bcd3a47e14a4de802b1c536e7b3d557
plan_head_after: 1a8fc558ad817f460ba6b106a4cf1d70a97603b8
actuals:
  tokens: 300
  tasks: 2
  commits: 2
---

# Phase 5 Plan 08: gcc-14 build fix and suite-flake timeout Summary

Cast the chr_8k ternary in tests/core/test_reset.c to uint8_t so gcc-14 with -Werror=conversion compiles, and raised the nightly suite-flake timeout from 10 to 30 minutes.

## Accomplishments

- G-05-2: `spec.chr_8k = (uint8_t)(chr_ram ? 0u : 1u);` (commit 9ed3041). Verified in a real `gcc:14` Docker container: `cmake --workflow --preset ci` and `--preset nofp` both built with zero diagnostics and all tests passed (nofp 5 of 5; ci passed in the container too). The sweep found no other gcc-only narrowing, so no other file changed.
- G-05-3 / WR-02: `.github/workflows/nightly.yml` suite-flake `timeout-minutes: 30`, comment reworded to say 30 is the WR-02 ceiling until a cold run is measured (commit 1a8fc55). One line changed.
- Local Apple clang `cmake --workflow --preset ci`: 100% tests passed out of 358.

## Method

Container path used (not the manual-sweep fallback). No environmental test skips occurred in the container. Hosted PR #25 legs remain the final proof after the verify step pushes.

## Deviations from Plan

None - plan executed exactly as written.

## Known Stubs

None.

## Self-Check: PASSED

Commits 9ed3041 and 1a8fc55 exist on phase/05-tune-up-and-v1-debt; both modified files verified.
