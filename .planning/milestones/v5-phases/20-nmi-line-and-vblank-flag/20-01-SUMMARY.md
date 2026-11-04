---
phase: 20-nmi-line-and-vblank-flag
plan: "01"
subsystem: testing
tags: [nmi, vblank, accuracycoin, ctest]
requires: []
provides:
  - Absolute-dot and real-bus PPU status race reproducers
  - Guest-visible NMI polling and instruction-order reproducers
  - Pinned AccuracyCoin baseline diagnostics and table-oracle notes
affects: [20-02, 20-03, 20-04, NMI-01, NMI-02]
actuals:
  tokens: 10624
  tasks: 2
  commits: 2
commits: 2
plan_head_before: 9f200cb4223b63de216019f4436f6e699c0a485e
plan_head_after: 88dead09f0d9a214ba6f1f82155749d012080b0b
tech-stack:
  added: []
  patterns:
    - Test-only NROM fixtures with cycle-aligned whitebox starts
    - Named expected-red assertions that reject setup and unrelated failures
key-files:
  created:
    - tests/nmi_fixture.h
    - tests/core/test_nmi_poll.c
  modified:
    - tests/ppu/test_nmi_races.c
    - tests/CMakeLists.txt
    - tests/accuracy/baseline-metadata.txt
    - tests/accuracy/measurements.md
key-decisions:
  - Keep live PPU, CPU bus, and guest execution evidence separate from source-table comparator self-checks.
  - Leave NMI-01 and NMI-02 pending because this plan adds reproducers without changing production behavior.
patterns-established:
  - "Expected-red guard: mask only one named assertion and require exactly one total failure."
  - "Source-table oracle: accept only complete pinned sequences and label local wrong-model checks as diagnostics, not mutation evidence."
requirements-completed: []
requirements-pending: [NMI-01, NMI-02]
coverage:
  - id: D1
    description: Absolute-dot and ordinary/mirrored CPU-bus status race reproducers with named expected-red cases.
    requirement: NMI-01
    verification:
      - kind: unit
        ref: "ctest --preset ci -R '^ppu\\.nmi_races$'"
        status: pass
    human_judgment: true
    rationale: The reproducer suite passes, but the measured AccuracyCoin behavior remains a failure until later production plans.
  - id: D2
    description: Guest-visible NMI polling, handler entry, and return-order cases for the pinned Control patterns.
    requirement: NMI-02
    verification:
      - kind: unit
        ref: "ctest --preset ci -R '^core\\.nmi_poll$'"
        status: pass
    human_judgment: true
    rationale: Test harness success includes assertion-specific expected-red behavior; it does not mean NMI-02 is implemented.
  - id: D3
    description: Re-recorded, source-pinned AccuracyCoin campaign metadata with unchanged raw report and scoreboard.
    verification:
      - kind: integration
        ref: "ctest --preset ci -R '^accuracycoin\\.campaign$'"
        status: pass
      - kind: unit
        ref: "ctest --preset ci -R '^accuracycoin\\.activation$'"
        status: pass
    human_judgment: false
duration: 32min
completed: 2026-10-26
status: complete
---

# Phase 20 Plan 01: NMI/VBlank diagnostics and reproducers Summary

**Owned PPU-dot, CPU-bus, and guest NMI reproducers now pin the six failing AccuracyCoin rows without changing the production core.**

## Performance

- **Duration:** 32 min
- **Started:** 2026-10-26T18:45:48Z
- **Completed:** 2026-10-26T19:17:51Z
- **Tasks:** 2
- **Files modified:** 6

## Accomplishments

- Added cycle-aligned PPU set/clear boundary checks, ordinary and mirrored `$2002` reads, real `$2000` writes, suppression and captured-request cases, and an exact expected-red helper. Named expected-red cases count as test-harness success only when the one expected assertion is the sole failure.
- Added guest NROM cases for NMI Control 8 and 9, unchanged enable, disable/re-enable, enable after `$2002` clears VBlank, and taken/not-taken/page-crossing branch entry. The tests record guest handler entry and stacked return PCs.
- Recorded the six current AccuracyCoin failures and complete source-derived accepted tables. Re-ran the pinned campaign after the new CMake test registration changed the measured build-input fingerprint; all 146 scored rows and the report hash remained unchanged.

## Verification

- `ppu.nmi_races` and `core.nmi_poll`: 2/2 passed.
- Adjacent `core.nmi_edge`, `ppu.registers`, and `ppu.sprites`: 3/3 passed.
- `accuracycoin.campaign`: passed, 22/22 pages and 146/146 scored rows observed; 108 pass, 38 fail, zero nonterminal results. The campaign report SHA-256 remains `b73c7b255ef4c0fe091c2d50bf94685e5b666760aaead6e28985bc3931ceeb7c`.
- `accuracycoin.activation`: passed with the refreshed input fingerprint in `baseline-metadata.txt`.
- `cmake --workflow --preset ci`: passed all 677 tests and generated the library, runner, and libretro ZIP packages.

The measured target rows remain `$2002 flag timing` fail `$06` (521 frames), `NMI Control` fail `$22` (1463), `NMI Suppression` fail `$06` (1463), `NMI Timing` fail `$06` (1463), `NMI at VBlank end` fail `$06` (1463), and `NMI disabled at VBlank` fail `$06` (1463). These are campaign measurements. The source-table comparator self-checks do not run the AccuracyCoin ROM and are not execution evidence.

The pinned assembly's table oracles are copied as whole alternatives: `$2002` clear vectors `[E0,E0,80,00]` or `[E0,80,80,00]`; masked set samples `$53=00`, `$54=00|40`, `$55=60`; NMI Timing vectors `[03,02,02,02,02,02,02,01,01,01]` and `[02,02,02,02,02,02,01,01,01,01]`; suppression vector `[FF,02,02,FF,00,01,FF,03,03,03]` with source checks beginning at x=1 and skipping x=3 and x=6; and both NMI-at-VBlank-end instruction sequences expecting `[01,01,01,00,00,00,00]`. The second end sequence is exactly `LDA #$80; STA $2000; NOP; NOP; LDA #$00; STA $2000` before storing the sample. The disabled-at-VBlank vector is `[00,00,00,FF,01,01,01]`, skipping x=3. Source locations and the explicit distinction between live cases, comparator checks, and future production mutation controls are recorded in `tests/accuracy/measurements.md`.

## Task Commits

1. **Task 1: Trace one CPU-visible status race to a named expected assertion** — `58607db` (`test`)
2. **Task 2: Expand the absolute race, control and complete-table reproducers** — `88dead0` (`test`)

Plan state and summary are committed after the self-check.

## Files Created/Modified

- `tests/nmi_fixture.h` — cycle-aligned owned NROM fixture and strict named expected-red bookkeeping.
- `tests/ppu/test_nmi_races.c` — absolute-dot status checks, real bus reads/writes, suppression cases, complete table comparators, and named wrong-model diagnostics.
- `tests/core/test_nmi_poll.c` — guest-visible NMI Control polling and handler-order cases.
- `tests/CMakeLists.txt` — registers both Phase 20 test targets.
- `tests/accuracy/baseline-metadata.txt` — refreshed actual campaign source revision, input fingerprint, and elapsed time.
- `tests/accuracy/measurements.md` — records the six measured rows and the source-vector interpretation and evidence boundaries.

## Decisions Made

- Use owned NROM and bus execution for live behavior checks; treat pinned assembly vectors as comparator inputs only.
- Keep the phase requirements pending. The expected-red tests make current defects reproducible; they do not make the still-failing AccuracyCoin requirements complete.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Corrected the set-race M2 witness's expected scanline at frame wrap**
- **Found during:** Task 1 (Trace one CPU-visible status race to a named expected assertion)
- **Issue:** The test expected the preceding scanline after the witness had crossed the VBlank-set boundary.
- **Fix:** Asserted the event scanline and retained the independent late-sample dot check.
- **Files modified:** `tests/ppu/test_nmi_races.c`
- **Verification:** `ppu.nmi_races`, `core.nmi_edge`, and `ppu.registers` passed.
- **Committed in:** `58607db` (part of Task 1).

**2. [Rule 3 - Blocking] Refreshed the AccuracyCoin input fingerprint after test registration**
- **Found during:** Task 2 full-workflow verification
- **Issue:** The initial full workflow's `accuracycoin.activation` found the committed measurement fingerprint stale because `tests/CMakeLists.txt` had changed; expected `e252004f…`, recorded `3211e94f…`.
- **Fix:** Ran the repository's registered 22-page campaign, confirmed the full report and scoreboard bytes were unchanged, and refreshed measured provenance and explanatory notes.
- **Files modified:** `tests/accuracy/baseline-metadata.txt`, `tests/accuracy/measurements.md`
- **Verification:** `accuracycoin.campaign`, `accuracycoin.activation`, and the subsequent full 677-test workflow passed.
- **Committed in:** `88dead0` (part of Task 2).

---

**Total deviations:** 2 auto-fixed (1 Rule 1 bug, 1 Rule 3 blocking issue)
**Impact on plan:** Both changes were needed for trustworthy tests and baseline metadata. No production files changed.

## Issues Encountered

The first full workflow stopped at the stale AccuracyCoin fingerprint. The measured campaign refresh resolved it; the final full workflow passed. No other issues remain.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

Plan 20-01's reproducers, exact source tables, and measured baseline are ready for Plan 20-02's split status/line capture and coherent state work. NMI-01 and NMI-02 remain pending until later plans implement and measure the required behavior.

## Self-Check: PASSED

- Summary file and all six plan artifacts exist.
- Task commits `58607db` and `88dead0` are ancestors of `HEAD`.
- Full CI reported 677/677 tests passed.

---
*Phase: 20-nmi-line-and-vblank-flag*
*Completed: 2026-10-26*
