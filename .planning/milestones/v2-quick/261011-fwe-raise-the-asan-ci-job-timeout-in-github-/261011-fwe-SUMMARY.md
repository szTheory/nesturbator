---
phase: quick-261011-fwe
plan: 01
subsystem: ci
tags: [ci, asan, timeout]
requires: []
provides: [asan job timeout of 20 minutes]
affects: [.github/workflows/ci.yml]
key-files:
  modified: [.github/workflows/ci.yml]
decisions:
  - "asan timeout-minutes raised 10 to 20, provisional until a complete measured run (ENGINEERING section 5)"
metrics:
  duration: 3m
  completed: 2026-10-11
status: complete
actuals:
  tokens: 300
  tasks: 1
  commits: 1
plan_head_before: e5db548dd8001434c3b93459df339fec0bb38c53
plan_head_after: 1a5eac5
---

# Quick 261011-fwe: Raise asan CI timeout Summary

The asan job in `.github/workflows/ci.yml` now has `timeout-minutes: 20`, with a comment marking it provisional and citing run 38150104151 (cancelled at 477/480 tests, zero failures, after Phase 10 added runner.boot_to_play at about 235 s and libretro.parity mapper 4 runs at about 55 s each).

## Commit

- 1a5eac5: ci: raise asan timeout to 20 minutes

## Verification

YAML parses with jobs.asan.timeout-minutes == 20; grep matches the provisional comment; scripts/hygiene.sh passed.

## Deviations from Plan

None - plan executed exactly as written.

## Known Stubs

None.

## Self-Check: PASSED
