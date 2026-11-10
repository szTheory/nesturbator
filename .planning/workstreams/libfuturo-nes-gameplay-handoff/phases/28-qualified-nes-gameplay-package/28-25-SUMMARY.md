---
phase: 28-qualified-nes-gameplay-package
plan: "25"
subsystem: testing
tags: [cmake, provider-receipt, source-binding, synthetic-controls]
requires: []
provides:
  - Strictly separate historical AccuracyCoin identity from current reviewed input hashes.
  - D-44 receipt controls and a bounded private driver for synthetic checks, one-shot route stages, and saved-evidence inspection.
affects: [28-26 local acceptance, package handoff]
actuals:
  tokens: 41205
  tasks: 2
  commits: 3
commits: 3
plan_head_before: 39c62969886ac92eefff2068ae47651e788cd978
plan_head_after: 441ba454c2be7328df88f7a8b75b724c87fffeca
tech-stack:
  added: []
  patterns:
    - Historical input identity remains separate from independently reviewed current checker hashes.
    - One-shot subprocesses use a fixed root, deadline, owned process-group cleanup, and saved read-only evidence.
key-files:
  created:
    - tests/accuracy/input-binding-v1.json
    - build/phase28-d43/run.py (ignored, private to the prepared worktree)
  modified:
    - tests/cmake/accuracycoin_activation.cmake
    - tests/cmake/accuracycoin_input_fingerprint_controls.cmake
    - tests/cmake/accuracycoin_input_fingerprint.cmake
    - tests/cmake/provider_receipt.cmake
    - README.md
    - docs/testing.md
    - docs/releases.md
key-decisions:
  - "Keep the dirty historical baseline identity and the current checker identity distinct; the reconstructed map does not authenticate the dirty historical tree or hardware behavior."
  - "Keep the route driver ignored and prepared-worktree-only; public guides do not advertise it as a fresh-checkout or shipped tool."
  - "Only the exact two post-freeze evidence overlays are allowed during inspection; the pre-freeze product delta remains exactly eight reviewed paths."
requirements-completed: []
coverage:
  - id: D1
    description: Historical and current AccuracyCoin input identities are checked separately, with mutations rejected before acceptance.
    requirement: PKG-01
    verification:
      - kind: unit
        ref: "build/phase28-d43/plan25-task1-final5: accuracycoin.input_fingerprint and accuracycoin.activation"
        status: pass
    human_judgment: false
  - id: D2
    description: Fresh D-44 receipt scope, discriminator, handoff observation, and strict schema controls reject synthetic mutations.
    requirement: PROF-01
    verification:
      - kind: unit
        ref: "build/phase28-d43/plan25-task1-final5: provider.receipt.controls"
        status: pass
    human_judgment: false
  - id: D3
    description: The private driver exercises bounded synthetic publication and inspect behavior without consuming the actual route ledger.
    verification:
      - kind: other
        ref: "build/phase28-d43/plan25-driver-controls-final5/result.json"
        status: pass
    human_judgment: false
duration: 129m
completed: 2026-11-10
status: complete
---

# Phase 28 Plan 25: Historical and Current Input Binding Summary

**The D-44 route now binds historical measurement inputs separately from current checker bytes, with synthetic receipt controls and a bounded inspectable driver.**

## Performance

- **Duration:** 129 minutes from the first committed RED control to the final driver-control evidence; setup before that commit was not timed.
- **Started:** 2026-11-10 13:13:14 -05:00 (first task commit)
- **Completed:** 2026-11-10 15:22:35 -05:00
- **Tasks:** 2
- **Files modified:** 9 task files, including the ignored driver; the summary is recorded separately.
- **Actuals:** 41,205 tokens, measured as changed tracked-file characters plus the new ignored driver file size, divided by four.

## Accomplishments

- Added the versioned historical/current binding and updated the real AccuracyCoin activation and fingerprint paths to verify the immutable historical map, exact reviewed input set, and current checker hashes. The baseline metadata, ROM pins, result rows, collector, thresholds, and provenance remain unchanged.
- Extended the D-44 receipt path with an explicit discriminator, strict field validation, exact reviewed source delta, saved handoff-time missing-publication observation, and synthetic producer/consumer controls. D-42's prior base and rejection behavior remain intact.
- Added an ignored, prepared-worktree-only driver with fixed-root admission, bounded process-group cleanup, saved stdout/stderr digests, one-shot route operations, and read-only inspection. Updated public guides to describe the evidence and limits without presenting the private driver as a shipped or fresh-checkout tool.

## Task Commits

1. **Task 1 RED: synthetic D-44 binding and scope controls** — `6ff3c6a` (`test(28-25): add synthetic D-44 binding and scope controls`)
2. **Task 1 GREEN: separate historical and current input identity** — `7ac6cdf` (`fix(28-25): separate historical and current input identity`)
3. **Task 2: bind handoff observations and document evidence route** — `441ba45` (`fix(28-25): bind handoff observations and document evidence route`)

## Verification

- `ctest --test-dir build/phase28-d43/controls --output-on-failure -R '^(accuracycoin\.input_fingerprint|accuracycoin\.activation|provider\.receipt\.controls)$'` passed all three focused controls in 2.230 seconds. CTest discovery added zero fixture dependencies; `NESTURBATOR_ACCURACYCOIN_FULL=OFF`. Evidence, exact command output, timing, and before/after hashes are in [`plan25-task1-final5`](../../../../../build/phase28-d43/plan25-task1-final5/).
- `python3 build/phase28-d43/run.py controls --synthetic` passed in 2.590 seconds under the 180-second cap. Its result binds the ignored driver SHA-256 `ebe30fa9c4fc9f4377a6e7479986557c8b30c552be4245d60fa5fe0159c89b45`; the eight scoped product files and route-ledger paths were unchanged. Evidence is in [`plan25-driver-controls-final5`](../../../../../build/phase28-d43/plan25-driver-controls-final5/).
- `python3 -m py_compile build/phase28-d43/run.py` and `git diff --check` passed.
- A separate read-only reconstruction confirmed the 56 historical input rows and aggregate match the pinned source blobs. The final complete-delta independent read-only agent review found no additional blocker. It verified current checker/binding/control/receipt and ignored-driver hashes, preserved protected inputs, exact release-source binding and saved handoff observation checks. This is source review, not live acceptance; its disposition is in [28-D44-REVIEW.md](28-D44-REVIEW.md).

## Deviations from Plan

No scope deviations. Review-driven corrections stayed within the approved eight-file product allowlist and private ignored driver. They strengthened stage inspection, preserved leading Git status bytes, allowed only the two exact post-freeze evidence overlays, rejected near-prefix paths, and clarified that the ignored driver is unavailable in a fresh checkout.

## Issues Encountered

Synthetic testing exposed and resolved a driver inspection-test scoping error. Review also found that stripping Git output removed porcelain status's leading column, which caused legitimate planning changes to be misread; the driver now preserves leading bytes and tests planning changes and the two exact evidence overlays through the real inspection path. No live identity or acceptance gate was attempted.

## User Setup Required

None.

## Next Phase Readiness

Plan 25 is complete and has not consumed a source freeze, local acceptance workflow, AccuracyCoin campaign, hosted CI run, or publication attempt. The historical map and binding controls are ready for successor work. The final independent complete-delta review is closed with no additional blocker. Plan26 is now the sole next eligible plan; no new requirement completion is claimed here. Full local acceptance remains a later one-shot gate, and a failed actual identity gate remains terminal.

---
*Phase: 28-qualified-nes-gameplay-package*
*Plan: 25*
*Completed: 2026-11-10*

## Self-Check: PASSED

The summary file exists, both evidence directories resolve within the candidate build output, and all three task commits are ancestors of the current HEAD.
