---
phase: 20-nmi-line-and-vblank-flag
plan: "02"
subsystem: ppu
tags: [nmi, vblank, bus-timing, save-state, accuracycoin]
requires:
  - phase: 20-01
    provides: [owned race probes, pinned test cases, protected campaign baseline]
provides:
  - Split PPUSTATUS bit-7 and later status-bit sampling at the two bus-cycle phases.
  - A derived PPU NMI line, separately sampled edge history, and retained CPU request.
  - Format-2/revision-8 state contract with format-1 refusal and canonical race continuation.
  - Fresh full campaign, source table observations, control evidence, and all changed hash reasons.
affects: [20-03, 20-04, 20-05, 20-06]
actuals:
  tokens: 36962
  tasks: 3
  commits: 1
tech-stack:
  added: []
  patterns:
    - Derived hardware line state stays separate from sampled CPU history.
    - Circuit projections call the production sampler at documented phase endpoints.
key-files:
  created: []
  modified:
    - src/bus.c
    - src/ppu.c
    - src/state.c
    - tests/ppu/test_nmi_races.c
    - tests/accuracy/measurements.md
    - tests/accuracy/scoreboard.txt
    - tests/runner/hashes.txt
key-decisions:
  - "Equal integer timestamps use PPU transition, CPU sample, then bus side effect; exact physical tie precedence remains unclaimed."
  - "The bus captures a rising derived NMI line at cycle close; later line deassertion does not revoke the request."
  - "PPUSTATUS decoding requires an address in $2000-$3fff as well as register bits 010, including for DMC parked reads."
requirements-completed: []
requirements-addressed: [NMI-01, NMI-02, KEEP-02, KEEP-03]
metrics:
  duration: 90min
  completed: 2026-10-26
status: complete
plan_head_before: 53b32c2baf653684264ea899d909c644a939dc89
plan_head_after: 1a48384c3c872f68e1689bf38ee7aec733c1f900
commits: 1
coverage:
  - id: D1
    description: "The production bus samples a distinct NMI line and retains captured requests after status/control changes lower it."
    verification:
      - kind: unit
        ref: "tests/ppu/test_nmi_races.c; ctest --preset ci -R '^ppu\\.nmi_races$'"
        status: pass
    human_judgment: false
  - id: D2
    description: "Format 2 and revision 8 serialize sampled/captured history, refuse format 1 atomically, and preserve canonical race continuation."
    verification:
      - kind: integration
        ref: "ctest --preset ci -R '^(core\\.state_reject|state\\.format_pin|ppu\\.nmi_races)$'"
        status: pass
    human_judgment: false
  - id: D3
    description: "The current AccuracyCoin campaign, protected-pass comparison, six source table observations, and complete hash changes are recorded with provenance."
    verification:
      - kind: integration
        ref: "cmake --workflow --preset ci; tests/accuracy/measurements.md"
        status: pass
    human_judgment: false
---

# Phase 20 Plan 02: NMI line and VBlank flag Summary

**A split PPUSTATUS bus read and sampled NMI capture path with format-2 state, decoded race evidence, and current campaign/hash pins.**

## Performance

- **Duration:** 90 minutes
- **Started:** 2026-10-26T20:54:30Z
- **Completed:** 2026-10-26T22:24:42Z
- **Tasks:** 3
- **Files modified:** 24 implementation, test, and evidence files

## Accomplishments

- Split `$2002` bit-7 sampling at M2 rise from sprite and low-bit composition at M2 fall. The PPU owns the derived `/NMI` line; the bus samples its rising edge and retains the request independently of later line deassertion.
- Added real decoded bus, guest handler, save/load continuation, state-format refusal, six finite-margin circuit endpoint projections, and production-path wrong-model controls. The deterministic tie rule is documented as a model convention, without claiming an analog or paired-chip winner.
- Captured a fresh 22-page AccuracyCoin campaign: 110 pass, 36 fail, 0 skip, 0 not-run, 0 timeout, and 5 DRAW; all 108 protected starting PASS keys survive. VBlank beginning/end pass. NMI Control and Hybrid Addresses are new passes. The remaining NMI timing/suppression/end/disable observations and the `$2002` flag-timing row remain recorded failures for their follow-up plans.
- Re-pinned all 24 changed runner/state hashes with old/new digests and causes. State byte sizes remain unchanged; new state hashes reflect format 2, revision 8, and serialized NMI history.
- Fixed a bus decoder regression that applied PPUSTATUS effects to RAM addresses whose low three bits were `010`, including DMC parked reads. Added `$00ea`, RAM mirror/boundary, `$2002` mirror, and DMC cycle/latch regressions.

## Task Commits

Tasks 1–3 share one causal commit, as required by D-20: all production, state, tests, documentation, campaign evidence, and hash pins landed together after the full CI workflow passed.

1. **Task 1: Deliver circuit-constrained capture through a real bus race and production state** — `1a48384` (`feat`)
2. **Task 2: Publish the approved state contract and retain direct-register guards** — `1a48384` (`feat`)
3. **Task 3: Record measured capture changes and make the causal change green** — `1a48384` (`feat`)

## Files Created/Modified

- `src/bus.c`, `src/ppu.c`, `src/internal.h`, `src/state.c` — separate line, sample, capture, and status-read phases; preserve bus-cycle accounting and serialize historical state.
- `include/nesturbator.h`, `README.md` — publish behavior revision 8, format 2, split status sampling, request retention, and tie-policy limits.
- `tests/ppu/test_nmi_races.c`, `tests/ppu/test_registers.c`, `tests/libretro/libretro_host.c`, `tests/core/test_nmi_poll.c`, `tests/core/test_state.c`, `tests/core/test_state_reject.c`, `tests/core/test_api.c`, `tests/header/header_c.c`, `tests/header/header_cxx.cpp` — exercise production races, direct PPU state, guest delivery, versioning, and updated API expectations.
- `tests/accuracy/measurements.md`, `tests/accuracy/scoreboard.txt`, `tests/accuracy/baseline-metadata.txt`, `tests/accuracy/migration.tsv`, `tests/runner/hashes.txt` — retain current measurements and compatibility evidence.
- `tests/cmake/accuracycoin_activation.cmake`, `tests/cmake/accuracycoin_baseline_fetch.cmake`, `tests/cmake/accuracycoin_regression_controls.cmake`, `tests/cmake/vector_api_policy.cmake` — align fixture counts and public version fingerprints with the measured candidate.

## Decisions Made

- Equal timestamps use one deterministic ordering everywhere: PPU transition, CPU sample, then bus side effect. The finite-margin oracle constrains phase behavior; the endpoint projection does not model internal NMIP transitions, output propagation, analog setup/hold, or exact physical tie precedence.
- `cpu.nmi_prev` stores the last sampled asserted line and `cpu.nmi_pending` stores the captured request. Status reads, PPUCTRL disable, and pre-render clear can lower the live line without clearing captured history; CPU service consumes the request and reset clears both history fields.
- The shared status-address predicate requires `0x2000 <= address < 0x4000` and `(address & 7) == 2`, so low-bit matches in RAM or other regions cannot execute PPU register side effects.
- Version 2 and behavior revision 8 ship together. Format-1 states return the existing state-format error before content checks or mutation.
- This plan records contribution evidence for NMI-01, NMI-02, KEEP-02, and KEEP-03; those phase-level requirements remain unchecked until their remaining polling, platform, and continuation criteria are completed.

## Verification

- `cmake --workflow --preset ci` — passed, 678/678 tests; package step produced library, runner, and libretro archives.
- Focused correction rerun — passed, 7/7 (`core.api`, `core.nmi_poll`, `core.state`, vector API policy and self-test, `libretro.host`, plus the runner fixture pulled by the test selection).
- AccuracyCoin campaign — 22/22 pages launched, attributed, and completed; 146/146 rows observed and terminal; 110 pass / 36 fail / 5 DRAW; no skip, not-run, or timeout.
- Protected-pass comparison — 108/108 starting PASS keys retained. Candidate provenance remains `source_dirty=1` at source revision `53b32c2baf65`; report SHA-256 `15f69db8a3c0c7037219bd27f930ae2607fba99b0f5013ef0b99c303a6d72dc7`, fingerprint `d3898ef935b97427518b9cdc1501074f44fee3f4a6c7caa7ff23a7baff16115b` over 56 inputs.
- `runner.write_hashes` and its content comparison passed as part of the full workflow. The local dirty campaign is not hosted authority; Nightly vectors-fetch evidence remains unresolved.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Prevent RAM reads from selecting PPUSTATUS effects**
- **Found during:** Task 3 protected regression investigation
- **Issue:** Both the normal bus path and DMC parked-read path recognized PPUSTATUS by low address bits without first requiring the PPU register address range. A plain RAM read such as `$00ea` could clear VBlank and its latches before the RAM decoder returned the byte.
- **Fix:** Added one shared range-and-register predicate and tests for `$0002`, `$00ea`, RAM mirrors/boundaries, and DMC parked reads with retained PPU latches and five 24-tick cycles.
- **Files modified:** `src/bus.c`, `tests/ppu/test_nmi_races.c`
- **Verification:** The regression failed before the predicate fix; focused `ppu.nmi_races` and the final 678-test CI workflow passed after it.
- **Committed in:** `1a48384`

**2. [Rule 1 - Blocking issue] Refresh version and line-ownership expectations exposed by the approved contract**
- **Found during:** Full CI verification
- **Issue:** Existing tests still expected behavior revision 7, treated format 2 as an invalid state, pinned the old public declaration digest, or assumed direct PPU advancement immediately writes the CPU capture latch.
- **Fix:** Updated those checks for revision 8/format 2, retained format-1 refusal, refreshed the intentional public-header digest, and tested the derived line separately from bus-sampled capture.
- **Files modified:** `tests/core/test_api.c`, `tests/core/test_state.c`, `tests/core/test_state_reject.c`, `tests/libretro/libretro_host.c`, `tests/cmake/vector_api_policy.cmake`
- **Verification:** Focused rerun passed 7/7; full CI passed 678/678.
- **Committed in:** `1a48384`

**3. [Rule 1 - Test expectation] Promote the now-passing Control-8 markers to ordinary guards**
- **Found during:** Full CI verification
- **Issue:** The expected-red wrappers failed because the source-shaped following-LDX marker and handler-entry assertions now passed under the sampled capture path.
- **Fix:** Made those two case-8 assertions ordinary checks. The full campaign independently records NMI Control as a new pass (`0x22` to `0x01`); remaining NMI Timing, Suppression, VBlank-end and disabled-at-VBlank rows stay explicit measured failures for later plans.
- **Files modified:** `tests/core/test_nmi_poll.c`, `tests/accuracy/measurements.md`, `tests/accuracy/scoreboard.txt`
- **Verification:** `core.nmi_poll` passed and full CI passed 678/678.
- **Committed in:** `1a48384`

**Total deviations:** 3 automatic corrections (2 correctness fixes and 1 stale test expectation).
**Impact on plan:** All were required for correctness or to reflect behavior produced by the approved causal change; no unrelated behavior was changed.

## Issues Encountered

The first full CI run exposed six stale assertions and pins after the version and NMI ownership changes. They were corrected, all six focused reruns passed, and the second complete workflow passed. The initial run's failures are not waived or counted as green.

## User Setup Required

None.

## Next Phase Readiness

Plan 20-03 can proceed with the source-table poll defects and poll-dependent expected-red work. The six captured tables, default alignment, endpoint projections, global model tie rule, and exact first divergences are in `tests/accuracy/measurements.md`; later plans must revalidate them after polling changes. This plan does not complete Phase 20 or claim the remaining NMI-01/NMI-02 acceptance rows pass.

---
*Phase: 20-nmi-line-and-vblank-flag*
*Completed: 2026-10-26*

## Self-Check: PASSED

- Summary file exists at the required plan path.
- Causal commit `1a48384` is an ancestor of the current branch HEAD.
