---
phase: quick-261002-uu0
plan: 01
subsystem: public-api
tags: [abi, size-tag, header, review-fix]
status: complete
requires: []
provides:
  - caller-memset rule for size-tagged structs in the header and README
  - core.frame test for a larger zeroed frame and a non-zero tail
affects: [include/nesturbator.h, runner, libretro, tests]
tech-stack:
  added: []
  patterns: ["memset a size-tagged struct, then set size and fields"]
key-files:
  created: []
  modified:
    - include/nesturbator.h
    - README.md
    - runner/main.c
    - libretro/libretro.c
    - tests/core/test_frame.c
    - tests/core/test_api.c
    - tests/core/test_palette.c
    - tests/consumer/main.c
    - .planning/phases/01-a-test-frame-in-retroarch/01-REVIEW-FIX.md
    - .planning/phases/01-a-test-frame-in-retroarch/01-REVIEW-DISPOSITION.md
decisions:
  - "WR-03: callers zero every size-tagged struct with memset; structs are not made padding-free (owner decision)"
  - "NESTURBATOR_CONFIG_INIT stays for source compatibility; its comment says not to use it as the initialiser of a struct handed to the library"
metrics:
  duration: ~15 min
  completed: 2026-10-02
actuals:
  tokens: 9000
  tasks: 3
  commits: 2
plan_head_before: f5585ce748c4cab77b9144c6440783c7d1d5e2e5
plan_head_after: 922a56f4f19d365113e876fed7585c0ba7620ad9
---

# Quick 261002-uu0: Fix WR-03 with the caller-memset rule

Callers must now zero every size-tagged struct with memset. The header and README say so, every in-repo caller does it, and a new core.frame test checks two cases. A larger zeroed `nesturbator_frame` is accepted. A larger frame with a non-zero tail byte is refused and leaves the instance unchanged.

## Tasks

| Task | Name | Commit |
|------|------|--------|
| 1 | Rule in header and README; big_frame test | b6adc77 |
| 2 | Runner, adapter and tests memset every config and frame | b6adc77 |
| 3 | WR-03 recorded as fixed in fix report and disposition ledger | 922a56f |

Tasks 1 and 2 share one commit, as the plan's Task 2 done-criterion directs.

## Verification

- `cmake --workflow --preset ci`: 32/32 passed, including the new `test_big_frame` in core.frame. Task 1 is a tracer task, so this was re-run before Task 2 started, and it passed.
- `cmake --workflow --preset hygiene`: 6/6 passed after formatting with the project's clang-format 18.
- Plan greps: no `= NESTURBATOR_CONFIG_INIT` outside `tests/header`, no by-value `frame_io`, no brace-initialised frame in the consumer.
- Task 3 checks: the ledger frontmatter and table show WR-03 fixed, WR-03 appears under `## Fixed Issues`, and `scripts/hygiene.sh --tree` passed.
- The pre-commit hook ran on both commits and was not bypassed.

## Deviations from Plan

- **Order of changes:** `frame_io` was changed to fill the frame in place during Task 1 instead of Task 2, because the new test calls it that way. Tasks 1 and 2 share one commit, so the result is the same.
- **[Rule 3 - Blocking] Formatting:** my scripted edit gave one `frame_io` call in `test_audio_period` the wrong indent, and `hygiene.format` failed. I fixed it and ran the build tree's clang-format 18 on the changed files.

## Known Stubs

None.

## Self-Check: PASSED

- Commits b6adc77 and 922a56f exist on phase/01-test-frame.
- All modified files exist. `test_big_frame` is in tests/core/test_frame.c.
