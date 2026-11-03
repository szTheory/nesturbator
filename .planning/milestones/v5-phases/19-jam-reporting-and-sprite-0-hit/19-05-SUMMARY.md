---
phase: 19-jam-reporting-and-sprite-0-hit
plan: "05"
subsystem: ppu
tags: [ppu, sprite-evaluation, oamaddr, sprite-0, behaviour-revision, accuracycoin]
requires: ["19-04"]
provides:
  - "sprite evaluation starting at OAMADDR with the dot-66 sprite-zero identity (D-05, D-06)"
  - "behaviour revision 7 (D-18)"
affects: [19-06]
key-files:
  modified:
    - src/ppu.c
    - tests/ppu/test_sprites.c
    - tests/cmake/accuracycoin_driver.cmake
    - include/nesturbator.h
    - tests/core/test_api.c
    - tests/cmake/vector_api_policy.cmake
    - tests/cmake/accuracycoin_activation.cmake
    - tests/runner/hashes.txt
    - README.md
key-decisions:
  - "Driver page-19 case uses a case-local execution cap of 300; campaign caps 32/5/1800 unchanged"
  - "Re-pinned the 12 state/* rows of tests/runner/hashes.txt: only the recorded behaviour-revision byte changed, no frame hash moved"
requirements-completed: [SPR-01, SPR-02]
completed: 2026-10-25
status: complete
commits: 1
plan_head_before: 1623459
plan_head_after: 1cb7976
actuals:
  tokens: 40000
  tasks: 3
  commits: 1
---

# Phase 19 Plan 05: Sprite evaluation from OAMADDR, revision 7 Summary

Sprite evaluation now starts at `OAMADDR` (n = addr>>2, m = addr&3), the sprite found in range on the dot-66 read is sprite zero, and secondary OAM is filled in copy order; page 19 completes in 521 execution frames with `Arbitrary Sprite zero` at `pass 0x01`, behaviour revision is 7.

## D-10 cases after commit 5

All eleven cases are now plain checks (the `expect_red` wrapper and helper were removed): 1, 3, 5, 6, 7 and the `$FD` case green as reproducers; guards 2, 4, 8, 9 and all-Y=$FF stay green. `ppu.sprites` passes under ci and under asan (all six `ppu.` tests, incl. the `$FD` case).

## Page-19 observation (caps 32/5/1800)

`verdict=complete`, 521 execution frames. Sprite overflow pass, Sprite 0 Hit behavior pass 0x01, Suddenly Resize Sprite pass, Misaligned OAM DMA pass, Arbitrary Sprite zero pass 0x01, `$2002 flag timing` fail 0x06, Misaligned OAM behavior fail 0x16, OAM Corruption fail 0x0a. D-08 guard was not triggered.

## Commit-5 campaign raw-byte diff (key / status / raw / frames; scoreboard not edited)

```
$2002 flag timing            fail 0x06 1800 -> fail 0x06 521
$2004 Stress Test            fail 0x0a 503  -> fail 0x0a 569
$2007 Stress Test            fail 0x0a 503  -> fail 0x0a 569
$2007 read w/ rendering      fail 0x06 503  -> pass 0x01 569
Address $2004 behavior       fail 0x0e 503  -> fail 0x0e 569
Arbitrary Sprite zero        timeout 0x03 1800 -> pass 0x01 521
INC $4014                    fail 0x0a 503  -> pass 0x01 569
Misaligned OAM DMA           fail 0x06 1800 -> pass 0x01 521
Misaligned OAM behavior      not-run 0x03 1800 -> fail 0x16 521
OAM Corruption               not-run 0x00 1800 -> fail 0x0a 521
Rendering Flag Behavior      fail 0x0a 503  -> pass 0x01 569
Sprite 0 Hit behavior        fail 0x06 1800 -> pass 0x01 521
Sprite overflow behavior     pass 0x01 1800 -> pass 0x01 521
Suddenly Resize Sprite       fail 0x06 1800 -> pass 0x01 521
t Register Quirks            pass 0x01 503  -> pass 0x01 569
```

Plan 19-06 re-records these. `Misaligned OAM behavior` and `OAM Corruption` still fail (the latter is out of scope, D-09).

## Driver case

Chosen execution cap C = 300 (`--frames 424` = 32+90+2+300). Measured from diagnostics: Arbitrary Sprite zero (row 6, cursor 5) is the running test for execution frames 241-367, so 300 is strictly inside. The case asserts `verdict=guest-timeout`, `stop_cause=execution-cap-active-test`, active key and `active_raw=0x03`, and the timeout report row; last diagnostic regex unchanged. `stale_marker` dropped with a comment pointing at the report fixture in `tests/accuracy/test_conformance.c`. The campaign caps 32/5/1800 are unchanged (`budgets.txt` and `CMakePresets.json` untouched).

## API digest

`PHASE8_DECLARATIONS_SHA256`: `f2f4389ce9215fcc0cd08ccce9c89d9e3211614ecebd853ece5811c6350c14c6` -> `f697e9833c93680a74a2762800927d4f4e942061090d72e917418f3d41236ad6`.

## Commit

`1cb7976` `fix(ppu): start sprite evaluation at OAMADDR (behaviour revision 7)`

```
 README.md                                 |  5 ++++-
 include/nesturbator.h                     |  9 ++++++---
 src/ppu.c                                 | 22 +++++++++++++++++-----
 tests/cmake/accuracycoin_activation.cmake |  2 +-
 tests/cmake/accuracycoin_driver.cmake     | 13 ++++++++-----
 tests/cmake/vector_api_policy.cmake       |  4 ++--
 tests/core/test_api.c                     |  2 +-
 tests/ppu/test_sprites.c                  | 22 ++++------------------
 tests/runner/hashes.txt                   | 24 ++++++++++++------------
 9 files changed, 55 insertions(+), 48 deletions(-)
```

## Verification

`ctest --preset ci` (FULL reset to OFF): 669/670 pass; the one failure is the expected `accuracycoin.activation`. `ctest --preset asan -R '^ppu\.'`: 6/6.

## Deviations from Plan

**1. [Rule 3 - Blocking] `tests/runner/hashes.txt` state rows re-pinned.** `runner.write_hashes.content` failed on 12 `state/*` rows (`state/dabg`, `m1sxrom`, `m2`, `m3`, `m4tkrom`, `m7`, each at ticks 0 and 60) because a saved state records the behaviour revision (header offset 16). No `frame/` row moved, so the plan's "frame hash moved" clause did not apply; the file was added to commit 5 so the suite stays green. Commit `1cb7976`.

**2. [Style] Removed the now-unused `expect_red` helper** (it would fail `-Werror` unused-function). Commit `1cb7976`.

**Total deviations:** 2. **Impact:** none on behaviour.

## Known Stubs

None.

## Threat Flags

None.

## Self-Check: PASSED

Commit `1cb7976` is an ancestor of HEAD; edited files exist.
