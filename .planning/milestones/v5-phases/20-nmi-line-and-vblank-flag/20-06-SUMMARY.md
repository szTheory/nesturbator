---
phase: 20-nmi-line-and-vblank-flag
plan: "06"
subsystem: testing
tags: [nmi, vblank, accuracycoin, save-state, ctest]
requires:
  - phase: 20-05
    provides: [public core/runner/libretro NMI frame-boundary replay]
provides:
  - Exact-six AccuracyCoin target-key gate and final measured campaign provenance.
  - Local protected-PASS, hash-inventory, CI, and ASan evidence with limits recorded.
  - Evidence ledger separating local results from outstanding hosted obligations.
affects: [NMI-01, NMI-02, KEEP-02, KEEP-03]
actuals:
  tokens: 7886
  tasks: 3
  commits: 5
tech-stack:
  added: []
  patterns:
    - Campaign acceptance checks named full keys independently of aggregate totals.
    - Source arrays, internal cutpoints, public host replay, and hosted claims remain separate evidence layers.
key-files:
  created:
    - tests/cmake/nmi_targets.cmake
    - tests/cmake/accuracycoin_nmi_targets_control.cmake
    - .planning/phases/20-nmi-line-and-vblank-flag/20-EVIDENCE.md
  modified:
    - tests/CMakeLists.txt
    - tests/accuracy/baseline-metadata.txt
    - tests/accuracy/measurements.md
    - README.md
key-decisions:
  - "Keep the 108-key cold protected-main PASS set separate from the six newly passing NMI targets."
  - "Retain the globally applied bus-effect, detector-capture, tied-PPU-transition model without claiming physical exact-tie precedence."
  - "Keep KEEP-02 and KEEP-03 pending until hosted exact-head evidence exists; leave the adjacency and combined frame-0/empty-PCM backstops unconfirmed."
requirements-completed: [NMI-01, NMI-02]
requirements-pending: [KEEP-02, KEEP-03]
coverage:
  - id: D1
    description: "The final campaign has exactly one passing row for each of the six required full AccuracyCoin keys."
    requirement: NMI-01
    verification:
      - kind: integration
        ref: "accuracycoin.campaign + accuracycoin.nmi_targets; report SHA 29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac"
        status: pass
    human_judgment: false
  - id: D2
    description: "The local run preserves all 108 starting PASS keys and the generated native hash inventory remains unchanged."
    requirement: KEEP-02
    verification:
      - kind: integration
        ref: "cmake --workflow --preset ci; 683/683 tests passed"
        status: pass
    human_judgment: true
    rationale: "Six-platform exact-head hash equality, protected PASS evidence, and hosted performance remain pending."
  - id: D3
    description: "Core, runner, and libretro NMI replay evidence is recorded at public frame boundaries and exact internal cutpoints."
    requirement: KEEP-03
    verification:
      - kind: integration
        ref: "cmake --workflow --preset ci and cmake --workflow --preset asan; local results passed"
        status: pass
    human_judgment: true
    rationale: "The required hosted evidence and full phase gate have not run."
duration: 26min
completed: 2026-10-26
status: complete
plan_head_before: 75642f71e81a8132af4b6d36d5aca3af3c5fbcb7
plan_head_after: 5ad07e9997e7858fdb18f1272e2c800cc387e5f0
commits: 5
---

# Phase 20 Plan 06: final NMI evidence and local gates

**The six exact NMI target keys now gate the measured 114/32 AccuracyCoin campaign, with protected-main checks and hosted limits recorded.**

## Performance

- **Duration:** 26 minutes
- **Started:** 2026-10-27T01:14:37Z
- **Completed:** 2026-10-27T01:40:18Z
- **Tasks:** 3
- **Files modified:** 7 plan implementation/evidence files, excluding the summary and sequential state artifacts

## Accomplishments

- Added `accuracycoin.nmi_targets` for exactly the six full target keys, including changed, missing, duplicate, and mistyped-row controls. It runs in the existing full-campaign lane and checks the committed scoreboard on other lanes without starting duplicate full campaigns.
- Ran the pinned campaign once at default alignment: 22/22 pages, 146 scored rows, 114 pass, 32 fail, no skip/not-run/timeout, and 5 DRAW. All six required targets pass, while the cold protected-main observation preserves all 108 original PASS keys.
- Confirmed the generated 114-row native hash inventory matches the checked-in file byte for byte. No frame, PCM, transition, or state hash moved in this plan.
- Recorded the final source sequences, state cutpoints, finite-margin waveform projections, public host replay, local limitations, and pending hosted evidence in [20-EVIDENCE.md](./20-EVIDENCE.md). README now states format 2/revision 8 and distinguishes internal cutpoint fixtures from public frame-boundary replay; the public header already carries the matching contract.
- Full CI passed 683/683 and packaged the library, runner, and libretro outputs. Full ASan passed 675/675.

## Task Commits

1. **Task 1: Gate the measured campaign's six exact target keys** — `7577663` RED controls, `92db8e2` exact-key gate, `8a68afe` measured campaign provenance.
2. **Task 2: Reconcile final pins and protected-baseline controls** — `e749749`.
3. **Task 3: Run local gates and record hosted obligations honestly** — `5ad07e9`.

The execution-metadata commit follows the sequential state update and is not counted in the five commits measured by the plan ledger.

## Verification

- Final campaign: report SHA-256 `29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac`; input fingerprint `3465f9abcc455f5698f7c444417b496e617af9021a8c648ed5945346754ee3ed` over 56 files; source revision `92db8e2a40a0`, dirty flag 1, guest elapsed `7758848000` ns.
- Exact target gate, activation, cold protected observation, protected verdict, no-lost-pass check, hash writer/content check, format pin, hash inventory self-test, baseline-fetch fixture, and regression controls passed. The full workflow includes the NMI race and host/state replay suites.
- `tests/runner/hashes.txt` and `build/ci/tests/hashes.txt` each contain 114 rows and have SHA-256 `b429a38b026750c6b0c9270bc3ce8b1a73d2bbb100ead0608ff5339dd51bc390`.
- CI was run with `NESTURBATOR_ACCURACYCOIN_FULL=OFF`; the full campaign was not duplicated by CI or ASan.

## TDD Notes

Task 1 recorded a semantic RED result: the changed-target control was accepted by the temporary gate and the CTest control failed with the intended assertion. `gsd_run check tdd-red-evidence` returned `RED_EVIDENCE_OK`; replacing the temporary gate with the exact-key checker made the same control pass by rejecting the changed full key. No artificial CPU regression was introduced because the gate protects the already measured implementation.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking issue] Made final campaign provenance parseable by the protected-main fetcher**

- **Found during:** Task 2
- **Issue:** The new `source_dirty_reason` value contained an extra `=`, which the protected-main metadata parser rejects.
- **Fix:** Rephrased the provenance value without an embedded equals sign while preserving the dirty-local explanation.
- **Files modified:** `tests/accuracy/baseline-metadata.txt`
- **Verification:** `accuracycoin.baseline_fetch` passed on rerun; the full CI workflow also passed.
- **Committed in:** `e749749`.

**Total deviations:** 1 auto-fixed (Rule 3 - blocking issue)
**Impact on plan:** The metadata-only correction restores the existing control; campaign rows, protected-main authority, and build-input fingerprint remain unchanged.

## Known Stubs

None found in files created or modified by this plan. No tests were skipped and no required local verification was left unrun.

## Hosted Evidence Still Pending

No hosted run/job IDs or artifact identities were obtained in this plan. Six-platform exact-head hash equality and protected PASS checks, the existing per-ROM +1% performance gate after zero-spread controls, and configured hosted vector/MMC3/Holy Mapperel/boot-to-play/real-host checks remain pending. STATE records three prior Nightly `vectors-full` fetch failures on 2026-10-26; no successful confirming artifact is recorded. The local campaign remains dirty, so no clean campaign claim is made. KEEP-02/03 remain unchecked and Phase 20 remains in progress for the parent gates.

## Self-Check: PASSED

- The evidence ledger and task summary exist.
- Task commits `7577663`, `92db8e2`, `8a68afe`, `e749749`, and `5ad07e9` are ancestors of the plan head.
- The current plan-commit ledger records five commits from `plan_head_before` to `plan_head_after`.

---
*Phase: 20-nmi-line-and-vblank-flag*
*Completed: 2026-10-26*
