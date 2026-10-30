---
phase: 20-nmi-line-and-vblank-flag
plan: "03"
subsystem: cpu
tags: [nmi, instruction-poll, vblank, save-state, accuracycoin]
requires:
  - phase: 20-02
    provides: [derived NMI line, captured request history, format-2/revision-8 state contract]
provides:
  - Separate serialized NMI request-capture and CPU-poll history.
  - Source-backed instruction and branch NMI polling with Control 8/9 guest markers.
  - Six measured AccuracyCoin NMI/VBlank target rows passing at default alignment.
affects: [20-04, 20-05, 20-06, NMI-01, NMI-02]
actuals:
  tokens: 32492
  tasks: 3
  commits: 1
plan_head_before: 2c6ba6810823accdb352c9985c81e2408078836a
plan_head_after: ace7abe2f92ac2a911992cb69ac5ea429f76b874
commits: 1
tech-stack:
  added: []
  patterns:
    - Captured interrupt requests remain distinct from the instruction-boundary poll result.
    - Branch polling keeps the not-taken, same-page, and page-cross paths explicit.
key-files:
  created:
    - .planning/phases/20-nmi-line-and-vblank-flag/20-03-SUMMARY.md
  modified:
    - src/cpu.c
    - src/bus.c
    - src/internal.h
    - src/state.c
    - tests/core/test_nmi_poll.c
    - tests/ppu/test_nmi_races.c
    - tests/accuracy/measurements.md
    - tests/accuracy/scoreboard.txt
    - tests/runner/hashes.txt
key-decisions:
  - "Keep cpu.nmi_pending as the captured request and cpu.nmi_polled as its separately serialized poll history; the service consumes the polled request at the next instruction boundary."
  - "Use one source-consistent integer-timestamp policy: bus side effect, detector capture, then tied PPU transition; CPU polling reads the captured request at phi2 open. No per-table phase offset is used, and the exact physical tie remains unproven."
requirements-completed: [NMI-01, NMI-02]
coverage:
  - id: D1
    description: "Control 8/9 and the required instruction/branch classes deliver NMI at their source-backed polling point."
    requirement: NMI-02
    verification:
      - kind: unit
        ref: "ctest --preset ci -R '^(core\\.nmi_poll|core\\.nmi_edge|ppu\\.nmi_races)$'"
        status: pass
    human_judgment: false
  - id: D2
    description: "Captured and polled history is serialized, range-checked, reset, and resumed through a guest NMI marker."
    verification:
      - kind: integration
        ref: "ctest --preset ci -R '^(core\\.state_reject|state\\.format_pin|core\\.nmi_poll)$'"
        status: pass
    human_judgment: false
  - id: D3
    description: "All six pinned AccuracyCoin NMI/VBlank target rows pass at default alignment while all protected PASS keys remain passing."
    requirement: NMI-01
    verification:
      - kind: integration
        ref: "cmake --workflow --preset ci; ctest --preset ci -R '^(accuracycoin\\.(campaign|activation|protected|protected_observation)|accuracy\\.scoreboard(\\.no_lost_pass)?|runner\\.write_hashes(\\.content)?)$'"
        status: pass
    human_judgment: false
duration: 83min
completed: 2026-10-26
status: complete
---

# Phase 20 Plan 03: NMI instruction polling Summary

**Instruction-specific NMI polling now distinguishes captured requests from boundary polls, passes all six target rows, and retains measured state and hash evidence.**

## Performance

- **Duration:** 83 minutes
- **Started:** 2026-10-26T22:38:52Z
- **Completed:** 2026-10-27T00:02:13Z
- **Tasks:** 3
- **Files modified:** 25 implementation, test, and evidence files

## Accomplishments

- Added `cpu.nmi_polled` as bounded historical state distinct from `cpu.nmi_pending`. NMI polling follows instruction-specific bus seams, keeps branch cases explicit, preserves captured requests when the live line falls, and consumes the polled request at service.
- Kept both real RMW writes. The guest Control 8 marker records `LDX #$10` before NMI entry, handler entry X=`$10`, return `$800b`, and post-handler X=`$11`. Control 9 records the six-cycle `INC $2000`, final control/I/O values `$81`, and handler entry before the following LDX with return `$8008`.
- Serialized and validated poll history under format 2 / behavior revision 8, added valid capture-before-poll save/load continuation and invalid-field rejection, and refreshed the complete native hash inventory with per-row causes.
- Re-ran the pinned 22-page AccuracyCoin campaign: 146 scored rows, 114 pass, 32 fail, no skip/not-run/timeout, and 5 DRAW. All six required target rows pass at default alignment. The cold protected-main observation and the 108-key no-lost-pass check pass.

## Task Commits

Tasks 1–3 share the one causal green commit required by D-20: `ace7abe` (`feat(20-03): implement instruction-level NMI polling`). The summary and sequential state/roadmap close-out follow separately.

1. **Task 1: Trace Control 8 and Control 9 to distinct guest handler order** — `ace7abe` (`feat`)
2. **Task 2: Validate poll history and explain the observed hash movement** — `ace7abe` (`feat`)
3. **Task 3: Measure the poll campaign and close all six target diagnostics** — `ace7abe` (`feat`)

## Files Created/Modified

- `src/cpu.c`, `src/bus.c`, `src/internal.h`, `src/state.c` — poll history, instruction polling, reset, serialization, and validation.
- `include/nesturbator.h`, `README.md` — documented NMI poll behavior and the frame-boundary state contract.
- `tests/core/test_nmi_poll.c`, `tests/core/test_state_reject.c`, `tests/nmi_fixture.h`, `tests/ppu/test_nmi_races.c`, `tests/ppu/test_sprites.c`, `tests/libretro/libretro_host.c`, `tests/state_test.h`, `tests/fuzz/state_loader.c`, `tests/retroarch/frame_index.c` — guest ordering, race, state, observer, and compatibility checks.
- `tests/accuracy/measurements.md`, `tests/accuracy/scoreboard.txt`, `tests/accuracy/baseline-metadata.txt`, `tests/accuracy/migration.tsv`, `tests/runner/hashes.txt`, and four AccuracyCoin CMake controls — measured campaign and compatibility evidence.

## Decisions Made

- Poll history is independent of live line and captured request state. The bus samples the derived line at cycle close; the CPU polls the captured request at phi2 open. A late capture is retained through the current instruction and can be serialized before the next poll.
- Keep one global integer-timestamp model: addressed bus side effect, NMI detector capture, then the tied PPU transition. The CPU poll uses the captured request. Finite-margin evidence supports the modeled endpoints; exact physical tie precedence remains unknown. No per-test offset was applied and VBlank set/clear dots were not moved.
- Mark NMI-01 and NMI-02 complete because their six required rows now pass. KEEP-02 remains pending six-platform evidence, and KEEP-03 remains pending the broader race/interrupt continuation criteria in later Phase 20 plans.

## Verification

- `cmake --build --preset ci` — passed.
- Focused rerun for affected fixtures — 11/11 passed, including `core.state_fork.stress`, `libretro.host`, both fuzz jam-oracle controls, AccuracyCoin regression/fetch/driver controls, and marker controls/fixtures.
- `cmake --workflow --preset ci` — 678/678 tests passed; package step generated library, runner, and libretro ZIPs.
- Final plan Acceptance CTest set — 8/8 passed, including campaign, activation, protected observation, protected comparison, no-lost-pass, and hash writer/content checks.
- The six scoreboard rows are: `$2002 flag timing` pass `0x01` / 537 frames; NMI Control, NMI Timing, NMI Suppression, NMI at VBlank end, and NMI disabled at VBlank each pass `0x01` / 1418 frames.
- Campaign report SHA-256: `29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac`. The 56-input fingerprint is `6b6bc9ecca830db8ca8114a9c70b7d488882c0bb9ac16b5956872249c600bd0a`; scoreboard SHA-256 is `38b08c8c93ca2e3b7b3a52f28284d089ef2240d10dcaa1a1bf902c02cf92f4bf`. Provenance remains `source_revision=2c6ba6810823`, `source_dirty=1`.
- `tests/accuracy/measurements.md` records all six stored source sequences and Control 8/9 guest markers. It also accounts for 83 moved native hash rows and their causes. The diagnostic negative controls reject the omitted-write (5-cycle) and early-service marker observations; production assertions verify both writes, the six-cycle instruction, and actual handler order.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking issue] Refreshed fixtures tied to the measured campaign and serialized layout**
- **Found during:** Full workflow verification
- **Issue:** The measured run moved protected migration frame counts, increased the local candidate PASS set to 114, added one serialized byte, and made the marker state payload end exactly on an eight-byte boundary. Existing fixtures still expected 110 PASS rows, older frame counts, the previous jammed-state offset, and at least one padding byte.
- **Fix:** Updated only affected candidate fixtures and state offset, retained the unchanged fetched-main authority, and used a corrupted footer byte to keep the marker end-container control when no padding exists.
- **Files modified:** `tests/cmake/accuracycoin_baseline_fetch.cmake`, `tests/cmake/accuracycoin_regression_controls.cmake`, `tests/cmake/accuracycoin_driver.cmake`, `tests/fuzz/state_loader.c`, `tests/retroarch/frame_index.c`, `tests/accuracy/baseline-metadata.txt`
- **Verification:** Focused rerun and full `cmake --workflow --preset ci` passed.
- **Committed in:** `ace7abe`

**2. [Rule 3 - Blocking issue] Preserved state-fork DMA boundary coverage under the new poll cadence**
- **Found during:** First full workflow verification
- **Issue:** The MMC3 stress image no longer observed OAM DMA pending at a frame boundary with the previous local NMI-handler gap.
- **Fix:** Adjusted the synthetic stress image's single shared `ST_STRESS_G` delay from 5 to 6 counted pairs; the OAM-pending assertion remains enabled for both boards.
- **Files modified:** `tests/state_test.h`
- **Verification:** `core.state_fork.stress` and the full workflow passed, with both mapper observations retained.
- **Committed in:** `ace7abe`

**3. [Rule 1 - Test expectation] Updated the libretro capture/poll probe for the two-stage history**
- **Found during:** First focused rerun
- **Issue:** The second frame-edge scenario inherited the prior probe's handler PC, so it did not begin at the expected two-byte opcode.
- **Fix:** Reset the probe PC before the second capture case and assert first-instruction execution before service.
- **Files modified:** `tests/libretro/libretro_host.c`
- **Verification:** `libretro.host` passed in focused and full CI runs.
- **Committed in:** `ace7abe`

**4. [Rule 3 - Blocking issue] Made local campaign provenance parse as one metadata value**
- **Found during:** Baseline-fetch fixture verification
- **Issue:** Semicolons and an additional equals sign in `source_dirty_reason` were split by CMake's list-based metadata reader.
- **Fix:** Reworded the provenance value without list separators while retaining its source revision, dirty status, and hosted-evidence limitation.
- **Files modified:** `tests/accuracy/baseline-metadata.txt`
- **Verification:** `accuracycoin.baseline_fetch`, final acceptance gates, and full CI passed.
- **Committed in:** `ace7abe`

**Total deviations:** 4 automatic corrections (2 fixture/observer bugs, 1 stale test expectation, and 1 metadata parsing blocker).
**Impact on plan:** All corrections were required by the measured NMI/state change or its acceptance gates; no target caps, per-case offsets, or protected-main rows were relaxed.

## Issues Encountered

The first full workflow exposed stale fixture expectations and the MMC3 stress-image boundary. Those failures were corrected, the affected focused set passed 11/11, and the next complete workflow passed all 678 tests. The full candidate remains a local dirty-source observation; it does not replace hosted six-platform hash evidence or the unresolved Nightly vector-fetch evidence.

## User Setup Required

None.

## Next Phase Readiness

Plan 20-04 can continue with enabled-NMI and edge-before-poll state continuation. Phase 20 remains in progress. KEEP-02 still needs six-platform evidence, and KEEP-03 still needs its broader race/interrupt continuation acceptance. The exact physical tie remains a documented model limit.

## Self-Check: PASSED

- Summary file exists at the required plan path.
- Causal commit `ace7abe` is an ancestor of the current branch HEAD.

---
*Phase: 20-nmi-line-and-vblank-flag*
*Completed: 2026-10-26*
