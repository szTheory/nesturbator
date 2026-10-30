---
phase: 21-interrupt-latency-and-hijack
plan: "08"
subsystem: testing
tags: [accuracycoin, cmake, hosted-evidence, source-identity]
requires:
  - phase: 21-interrupt-latency-and-hijack
    provides: Native/adapter interrupt replay and current source baseline from Plan 21-07
provides:
  - Exact eleven-row v5 AccuracyCoin gate with fresh 114-key protected-main authority
  - Reused hosted collector with explicit Phase21 acceptance contract and preserved Phase20 defaults
  - Local source identity manifest, final workflow evidence, package inventory, and pending hosted-delivery contract
affects: [phase21-hosted-acceptance, accuracycoin, ci]
actuals:
  tokens: 14478
  tasks: 3
  commits: 11
  plan_head_before: fe1916d9afc6701285852305a2b61d1da686dc5b
  plan_head_after: 43d260a914ba76a130809524fba53ad58aca1298
tech-stack:
  added: []
  patterns: [explicit collector contract with historical defaults, tracked Git mode/blob source identity]
key-files:
  created:
    - .planning/phases/21-interrupt-latency-and-hijack/21-EVIDENCE-PREPARATION.md
    - tests/cmake/phase21_hosted_evidence.cmake
    - tests/cmake/phase21_hosted_evidence_controls.cmake
    - tests/cmake/phase21_source_identity.cmake
    - tests/cmake/v5_targets.cmake
  modified:
    - .planning/phases/21-interrupt-latency-and-hijack/deferred-items.md
    - src/state.c
    - tests/CMakeLists.txt
    - tests/accuracy/baseline-metadata.txt
    - tests/accuracy/measurements.md
    - tests/cmake/accuracycoin_v5_targets_control.cmake
    - tests/cmake/phase20_hosted_evidence.cmake
    - tests/cmake/phase20_hosted_evidence_controls.cmake
key-decisions:
  - "Phase21 selects 114 protected keys, the exact eleven target rows, behaviour revision 9, and the current library version through the existing collector helpers."
  - "Phase20 retains its 108-key, six-target, revision-8, 0.1.11 defaults and artifact names."
  - "Candidate `43d260a` adds post-choice NMI recognition inhibition with the existing serialized byte; shared IRQ arbitration, format 3/revision 9, ABI and state sizes remain unchanged."
  - "Fresh current-main 114-key authority is separately captured and verified with same-invocation candidate campaign/protection controls; CR-01, WR-03 and WR-04 remain deferred."
  - "No delivery.json or hosted acceptance is created without real CI/Nightly and PR facts; Plan 21-09 remains pending."
patterns-established:
  - "Collector variants pass a narrow explicit contract into shared validators instead of copying the collector."
  - "Local fixture controls demonstrate diagnostics only and are never counted as hosted provenance."
requirements-completed: []
requirements-pending: [INT-01, INT-02, KEEP-01]
duration: 84min
completed: 2026-10-29
status: complete
---

# Phase 21 Plan 08: Exact acceptance contract and local evidence summary

**All eleven AccuracyCoin target rows and all 114 protected PASS keys are explicit gates, with a reusable Phase21 collector contract and source-bound local preparation for later hosted acceptance.**

## Performance

- **Duration:** 84 minutes from the first task commit to summary creation; initial read-in preceded that timestamp.
- **Started:** 2026-10-29T09:42:52-04:00 (first atomic plan commit; reliable lower bound)
- **Completed:** 2026-10-29
- **Tasks:** 3
- **Files modified:** 13 plan files; `.planning/config.json` was preserved unstaged.

## Accomplishments

- Added a unique full-key PASS gate for all eleven required v5 rows, plus bounded missing/failed/duplicate controls and a fresh same-invocation protected-main baseline that retains all 114 initial PASS keys.
- Added the Phase21 collector wrapper for 114 keys, all eleven targets, behaviour revision 9, and the current library version. Existing Phase20 collector helpers and artifact names remain in use; historical 108-key/six-target/revision-8/`0.1.11` behavior and controls stay green.
- The post-review causal source candidate is `43d260a914ba76a130809524fba53ad58aca1298`. Its 427-path mode/blob manifest has SHA-256 `20460339ae9f742d3906ebe7ef6d5c4a2fff53c5c4b54cf59bcbb7b99cb9a1b2` and verified against an absolute clean clone at the candidate. Workflows ran on the same source bytes while the working tree was dirty; they are not described as clean-clone builds.
- Final CI passed 695/695 in 421.60s; ASAN 686/686 in 192.01s; vectors-full 258/258 in 17.05s; MMC3 Oracle 13/13 in 2.47s. A separate fresh-authority CTest invocation passed 5/5, including the campaign fixture, same-invocation protected observation/verdict, no-lost-pass, and all-eleven target gate.
- Current AccuracyCoin evidence remains 119 PASS / 27 FAIL, all 146 scored rows terminal, five inventory-only DRAW rows, all 114 fresh protected keys and all eleven targets passing. The final report SHA remains `56ebd01adb97b3916fde36f8739602284d951a1fd00d84285bb4ef441fb463a3`; the 56-input fingerprint is `d96efe4a88bdaa0680398334a3c84a0e04faef405016172e06ef2bb9c4999e57`. All 114 runner hash rows, including state hashes, regenerate byte-for-byte unchanged.
- All three current-source packages are retained under `build/phase21-hosted-evidence/packages/post-review-source/`; their identities and inventories are recorded in `21-EVIDENCE-PREPARATION.md`. The pre-existing `.planning/config.json` edit and historical debt remain preserved.
- Kept source/test/collector and evidence identities separate. The pre-existing `.planning/config.json` edit and historical deferred items remain preserved.

## Task Commits

1. **Task 1: Require all eleven target rows and protected keys** — `096a5c7` (test), `efc217c` (feat)
2. **Task 2: Reuse the hosted collector for the Phase21 contract** — `be5d42d` (test), `29abf80` (feat)
3. **Task 3: Record local source-bound evidence** — `b1de5ba` (fingerprint), `1d27a0f` (no-float citation fix), `4a5281c` (evidence), `0dfcb54` (retained package location)
4. **Post-review source repair and qualification** — `43d260a` (recognition inhibition, owned tests, README/public-header contract, and refreshed AccuracyCoin metadata)

The intervening plan-summary and review commits are `8e13391` and `7d5fbe1`;
the 11-commit total is measured from the recorded plan base through
`43d260a`.

## Files Created/Modified

- `tests/cmake/v5_targets.cmake` and `tests/cmake/accuracycoin_v5_targets_control.cmake` — exact target-key gate and bounded negative controls.
- `tests/cmake/phase21_hosted_evidence.cmake`, `phase21_hosted_evidence_controls.cmake`, and `phase21_source_identity.cmake` — narrow Phase21 contract, control coverage, and run-specific tracked Git identity evidence.
- `tests/cmake/phase20_hosted_evidence.cmake` and `phase20_hosted_evidence_controls.cmake` — shared collector contract inputs while preserving Phase20 defaults and controls.
- `tests/CMakeLists.txt` — registered target and evidence controls without another campaign lane.
- `tests/accuracy/baseline-metadata.txt` and `tests/accuracy/measurements.md` — refreshed input fingerprint and appended local-only final measurements.
- `.planning/phases/21-interrupt-latency-and-hijack/21-EVIDENCE-PREPARATION.md` — candidate/source map, local commands and packages, schema, and Plan09 prerequisites.
- `.planning/phases/21-interrupt-latency-and-hijack/21-REVIEW.md`, `21-REVIEW-DISPOSITION.md`, and `21-STATE-MATRIX.md` — retained original review history, current CR-01/CR-02 dispositions, and state acceptance matrix.
- `.planning/debug/entry-review-inhibition.md` — causal debugger record for the post-choice inhibited-NMI candidate.
- `README.md`, `include/nesturbator.h`, `src/bus.c`, `src/cpu.c`, `src/internal.h`, and `src/state.c` — documented and implemented post-choice NMI recognition inhibition using the existing pending byte; no independent IRQ queue was added.
- `tests/core/test_interrupt_entry.c`, `tests/core/test_interrupt_state.c`, `tests/core/test_state_reject.c`, and `tests/libretro/libretro_host.c` — finite-margin production, state rejection, and actual adapter controls for held/released inhibited candidates.
- `.planning/phases/21-interrupt-latency-and-hijack/deferred-items.md` — preserved the existing Windows-ledger issue and recorded the separate macOS nofp symbol limitation.
- `src/state.c` — expressed the hardware section citation with integer wording so the unchanged no-float scanner accepts it; no runtime behavior changed.

## Decisions Made

- The Phase21 wrapper selects the stricter contract through explicit inputs to the existing collection and validation helpers. No collector duplication or gratuitous artifact renaming was needed.
- CR-01's independent IRQ-queue claim is rejected by shared request-flop/level-sensitive IRQ evidence. CR-02 is fixed by encoding `none=0`, `recognized=1`, and `inhibited=2` in the existing byte; released candidates cancel and held candidates promote before the first handler poll.
- Run-specific Git mode/blob rows plus clean-equivalent checkout/current-checkout revalidation supplement collector debt. They do not enforce host filesystem mode or close CR-01, WR-03, or WR-04.
- Local fixture controls and local native packages are not hosted evidence. `delivery.json` remains absent until actual run, attempt, PR, candidate, and tested-merge facts are supplied.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking issue] Reworded the state-validation source citation for the required no-float scan**
- **Found during:** Task 3 (local final gates)
- **Issue:** The final-source `nofp` probe showed that the scanner intentionally checks comments and rejected the decimal `2.3` in the hardware section citation at `src/state.c:98`.
- **Fix:** Changed the comment to spell the reference as section 2, subsection 3; refreshed the 56-input AccuracyCoin fingerprint. No scanner gate or runtime logic changed.
- **Files modified:** `src/state.c`, `tests/accuracy/baseline-metadata.txt`
- **Verification:** The focused scan passed on 23 files, and final CI passed 695/695. The separate macOS `nofp` run passed the scan, its self-test, global-symbol check, and float fixture; its Apple undefined-symbol result is documented below.
- **Committed in:** `1d27a0f` (source citation and fingerprint update)

**2. [Rule 1 - Metadata parsing issue] Removed an equals sign from refreshed source-dirty prose**
- **Found during:** Post-review full CI refresh
- **Issue:** The strict baseline metadata parser treats equals signs as delimiters and rejected the explanatory `source_dirty_reason` value.
- **Fix:** Reworded the value without an embedded equals sign; no gate or runtime behavior changed.
- **Verification:** The five affected Phase20/Phase21 collector and baseline-fetch controls passed, followed by full CI at 695/695.
- **Committed in:** `43d260a` with the causal source/doc/test change.

**Total deviations:** Two narrowly scoped corrections after actual scanner/parser failures.
**Impact on plan:** The no-float wording and metadata-value corrections preserve existing hard gates and change no acceptance threshold, AccuracyCoin row, ROM pin, cap, performance bound, package ABI, or collector default.

## Issues Encountered

- The first wrapper around CI lost CMake's exit status because it assigned zsh's reserved variable `status`. A direct final-source workflow rerun exited 0; its full result is recorded in the preparation document and `build/phase21-hosted-evidence/ci-review-final.log`.
- The first post-review CI attempt exposed five strict metadata-parser failures after the local explanation field contained an equals sign. The value was corrected, the five affected controls passed, and the subsequent full CI rerun passed 695/695.
- The optional local macOS arm64 `nofp` workflow still fails `abi.undefined_symbols` on `stack_chk_fail` and `stack_chk_guard`; the other four ABI checks pass. The configured nofp CI matrix runs Linux x64 and arm64, so those actual results remain external and pending. The hard gate was not weakened.
- The previous `.planning/WINDOWS.md` count/record mismatch remains in `deferred-items.md`; the phase's pre-existing ledger history was retained.

## User Setup Required

None. Plan 21-09 requires separately delivered completed CI/Nightly runs and actual PR facts; no local setup or manual sign-off substitutes for them.

## Next Phase Readiness

Plan 21-08 local preparation is complete. Plan 21-09 must remain pending until completed current-candidate CI and Nightly run/attempt identities, the actual PR and tested merge, and their immutable artifact metadata/content are available. No hosted acceptance or phase requirements are claimed complete. INT-01, INT-02, and KEEP-01 remain pending that work.

## Self-Check: PASSED

- All Plan08 source, control, measurement, preparation, and deferred-item files exist.
- Task commits `096a5c7`, `efc217c`, `be5d42d`, `29abf80`, `b1de5ba`, `1d27a0f`, `4a5281c`, `0dfcb54`, and `43d260a` are in the current branch history.
- The measured plan range also includes summary/review metadata commits `8e13391` and `7d5fbe1`; the final documentation/state commit follows this summary.
- Candidate source manifest verified all 427 paths on the current candidate and an absolute clean clone; post-review run logs and all package files exist.
- Plan 21-09, INT-01, INT-02, and KEEP-01 remain pending external hosted evidence.

---
*Phase: 21-interrupt-latency-and-hijack*
*Completed: 2026-10-29*
