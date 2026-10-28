---
created: 2026-10-26T17:32:52.434Z
title: Add reproducible owned property tests
area: testing
severity: minor
scope: proposed
files:
  - tests/check.h:1
  - tests/state_test.h:1
  - tests/core/test_state_reject.c:1
  - tests/core/test_state_fork.c:1
  - tests/core/test_cartridge.c:324
  - tests/runner/state_mutate.c:1
  - tests/fuzz/state_loader.c:14
---

## Problem

The 2026-10-26 CI/testing findings proposed a small owned helper for reproducible
property tests: fixed CI seeds, the seed printed on failure, and simple shrinking.
Candidate properties are save/load/save byte equality, seeded determinism and
iNES/NES 2.0 header-field limits.

Coverage already exists: `core.state_reject` sweeps every truncation and byte
flip, checks field bounds with a recomputed CRC, and verifies failed loads leave
the instance and save generation unchanged. Its short-run mode samples RAM
flips. `runner.state_mutate` supplies four CLI rejection cases. The state-loader
fuzzer already generates fixed-seed images. Measure the additional value of a
shared helper against these tests before adding another layer.

## Solution

Proposed scope for future prioritization, linked to SEED-004. Reuse the existing
owned helpers first; if needed, start with a small `tests/prop.h` using integer
SplitMix64, a fixed and replayable seed, failure diagnostics and minimal shrinking.
Exercise the candidate properties across the supported board families. Evaluate
exhaustive corruption coverage under ASan and whether any runner mutation cases
can share it while retaining CLI exit-status coverage and rejection oracles.
Preserve documented error precedence and instance-unchanged checks. Measure
runtime and demonstrate additional defect detection before replacing coverage.

Source: `ci-efficiency-findings-2026-10-26` project memory and this capture's
inspection of the existing tests. Severity minor confirmed in the capture session.
This proposal does not add requirements to Phase 20 or the committed v5 scope.
