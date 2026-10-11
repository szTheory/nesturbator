---
phase: 10-close-out-and-boot-to-play
plan: 05
subsystem: tests
tags: [hashes, hash-keys, drift-guard, holymapperel, nes-runner, cmake]
requires: ["10-02", "10-03", "10-04"]
provides:
  - "tests/cmake/hash_keys.cmake: nesturbator_expected_hash_keys, NESTURBATOR_HASH_GAMES, NESTURBATOR_ROM_HASH_EXEMPT"
  - "95-row tests/runner/hashes.txt (nes-runner boot and play, Holy Mapperel audio)"
  - "policy.rom_hash_coverage and policy.rom_hash_coverage.self_test"
affects: [10-06, 10-07]
tech-stack:
  added: []
  patterns: ["one key list read by writer, validator, fixture and drift guard", "SELFTEST mode building scratch trees"]
key-files:
  created: [tests/cmake/hash_keys.cmake, tests/cmake/rom_coverage.cmake]
  modified: [tests/cmake/write_hashes.cmake, tests/cmake/hash_inventory.cmake, tests/runner/hashes.txt, tests/CMakeLists.txt, README.md, .planning/phases/07-uxrom-cnrom-and-axrom/07-REVIEW-DISPOSITION.md]
key-decisions:
  - "Key list lives once in hash_keys.cmake; no literal row count remains in the writer or validator (D-16)"
  - "Holy Mapperel audio rows come from the existing 2N-frame run with --hash-audio added, no extra emulation (D-15)"
  - "AccuracyCoin is the only exempt ROM (D-14)"
requirements-completed: [BOARD-04, PLAY-01]
status: complete
commits: 3
plan_head_before: a0127eaeb191a7d4e8891ebc2e192d30deeca779
plan_head_after: f58b3061598aa59e1c9c706d7cd349c79112c4cf
actuals:
  tokens: 30000
  tasks: 3
  commits: 3
duration: 25 min
completed: 2026-10-11
---

# Phase 10 Plan 05: Hash key list, audio and play rows, drift guard Summary

One key list in `hash_keys.cmake` now drives the hash writer, the CI validator and its fixture, `hashes.txt` grows to 95 rows with nes-runner boot and play rows and Holy Mapperel audio rows, and a drift guard rejects any committed ROM that is neither hashed nor exempt.

REQUIREMENTS.md still shows BOARD-04 and PLAY-01 open on purpose: later plans in this phase still declare them.

## Tasks

| Task | Commit | Result |
|------|--------|--------|
| 1 One key list, nes-runner boot rows (tracer) | 2da3860 | Writer compares generated keys to the function and names missing and extra keys; 57 rows; the 50 earlier rows unchanged |
| 2 HM audio, nes-runner play, WR-02 | d244f38 | 95 rows (28 HM audio, 10 play); per-artifact key comparison; three new self-test cases; WR-02 marked fixed |
| 3 Drift guard | f58b306 | `rom_coverage.cmake`, two CTests, configure-time empty-list stop, README sentence |

Tracer verified end-to-end before expansion. Play frame 300 (`8478ee93`) and frame 600 (`6bf066cf`) match the research prefixes.

## Verification

`cmake --workflow --preset ci` (454 tests) and `cmake --workflow --preset hygiene` pass. `ctest -N -R "^policy\.rom_hash_coverage"` prints 2 tests. Row counts: 95 total, 28 `holymapperel/*/audio/*`, 10 `nesrunner/play/*`, 7 `nesrunner/boot/*`. The only rows removed or changed against `origin/main` are the 24 keys Plan 01 recorded.

## Deviations from Plan

- **[Rule 2 - Missing coverage] Extra self-test cases.** Beyond the two cases the plan names (missing audio row in every artifact, in only one), added `renamed_all` (identical artifacts, same row count and order, one renamed key) because it is the only case that reaches the per-artifact key comparison; the other two are rejected earlier by the row-count and identity checks. The drift guard self-test also covers empty game and Holy Mapperel lists.

**Total deviations:** 1 (additive). **Impact:** none on the plan's outputs.

## Known Stubs

None.

## Threat Flags

None. T-10-07 and T-10-08 are mitigated as planned. `.github/workflows/ci.yml` is unchanged (PROH-10-05-02).

## Self-Check: PASSED
