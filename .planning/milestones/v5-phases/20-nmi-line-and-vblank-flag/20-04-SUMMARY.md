---
phase: 20-nmi-line-and-vblank-flag
plan: "04"
subsystem: testing
tags: [nmi, vblank, save-state, replay, ctest]
requires:
  - phase: 20-03
    provides: [instruction-specific NMI polling, serialized captured/polled history, Control 8/9 guest oracles]
provides:
  - Exact enabled VBlank set/clear and captured-before-poll save/load continuation evidence.
  - A bounded Control 8/9 transition sweep with every tested offset and result recorded.
  - A CRC-correct omitted-history control and execution-backed live-low captured-request replay.
affects: [20-05, 20-06, KEEP-03]
actuals:
  tokens: 9396
  tasks: 3
  commits: 2
plan_head_before: 22a1a9bb71e4e6a568e75fc770b7f02acca497f9
plan_head_after: 354cf393d136bccd9d3236a1e17fe61369976a9a
commits: 2
tech-stack:
  added: []
  patterns:
    - Exact test origins are labeled constructed and endpoints are reached by complete production cpu_step calls.
    - Canonical save/load/save and future guest, video, PCM, tick, and native-state comparisons use real public state APIs.
key-files:
  created:
    - tests/core/test_nmi_state.c
  modified:
    - tests/CMakeLists.txt
    - tests/accuracy/measurements.md
    - tests/accuracy/baseline-metadata.txt
key-decisions:
  - "Keep every cutpoint at a completed instruction boundary; no partial-instruction state or public pause API is introduced."
  - "Keep KEEP-03 pending for the later host replay evidence; this plan establishes local instruction-boundary continuation only."
patterns-established:
  - "State replay: label fixture origin separately from the executed cutpoint and compare the canonical serialization before continuation."
  - "Race oracle: preserve an independent guest counter and test omitted serialized history with a CRC-correct control."
requirements-completed: [NMI-01, NMI-02]
coverage:
  - id: D1
    description: "Enabled VBlank set and clear states save and load canonically with coherent audio and matching future guest/frame output."
    requirement: NMI-01
    verification:
      - kind: unit
        ref: "ctest --preset ci -R '^core\\.nmi_state$' --no-tests=error --output-on-failure"
        status: pass
    human_judgment: false
  - id: D2
    description: "Control 8/9 edge-before-poll states, the clear-live-line request, omitted-history oracle, and interleaved-instance isolation are exercised at complete instruction boundaries."
    requirement: NMI-02
    verification:
      - kind: unit
        ref: "ctest --preset ci -R '^core\\.nmi_state$' --no-tests=error --output-on-failure"
        status: pass
    human_judgment: false
  - id: D3
    description: "CRC-correct malformed NMI histories reject atomically while valid captured/live-line differences load canonically."
    requirement: KEEP-03
    verification:
      - kind: integration
        ref: "ctest --preset ci -R '^(core\\.nmi_state|core\\.state_reject|state\\.format_pin)$' --no-tests=error --output-on-failure"
        status: pass
    human_judgment: true
    rationale: "The local matrix passes, but the later host replay and cross-platform evidence required to close KEEP-03 remains outstanding."
duration: 31min
completed: 2026-10-26
status: complete
---

# Phase 20 Plan 04: NMI race-state replay Summary

**Enabled NMI set, clear, and captured-before-poll boundaries now replay through production save/load with matching guest markers, native state, video, and PCM.**

## Performance

- **Duration:** 31 minutes
- **Started:** 2026-10-27T00:04:00Z (approximate; Plan 20-03 state was recorded at 00:03:42Z)
- **Completed:** 2026-10-27T00:35:00Z
- **Tasks:** 3
- **Files modified:** 4 plan artifacts

## Accomplishments

- Reproduced the disabled-NMI set/clear probe as a limited guard and reached enabled-NMI set `(241,1)` and clear `(261,1)` coordinates through completed JMP instructions from explicitly labeled constructed origins. Both serialized states contain coherent nonzero APU/synth history and replay through canonical save/load/save into equal future frame, tick, video, PCM, and guest-marker results.
- Reached captured-before-poll after `STA $2000` at `(241,2)` and a captured request after VBlank clear at `(261,1)`. The latter preserves `pending=1` with the live status low. The bounded Control 8/9 sweep records all 20 transition/offset candidates from -2 through +2. A CRC-correct omitted-pending state changes the guest marker from 1 to 0; loading it over an actually pending instance clears that stale latch.
- Kept the state-rejection matrix's CRC-correct malformed/valid cases and format precedence intact. Focused state tests and full local CI pass.

## Task Commits

1. **Task 1: Replay an exact VBlank-set cutpoint with enabled NMI and audio history** — `904f739` (`test(20-04): add exact NMI set replay tracer`)
2. **Task 2: Measure exact clear and edge-before-poll continuation with controls** — `354cf39` (`test(20-04): measure NMI replay cutpoints`)
3. **Task 3: Reject malformed histories without rejecting valid race states** — no source changes needed; existing `core.state_reject` coverage passed with the new execution-backed history evidence.

The summary and sequential state/roadmap close-out are committed separately.

## Files Created/Modified

- `tests/core/test_nmi_state.c` — exact transition replay, bounded candidate traces, stale/omitted-history control, frame continuation, and instance isolation.
- `tests/CMakeLists.txt` — registers `core.nmi_state`.
- `tests/accuracy/measurements.md` — records executed/constructed coordinates, all 20 sweep outcomes, audio history, and the remaining physical tie limit.
- `tests/accuracy/baseline-metadata.txt` — refreshes the measured AccuracyCoin build-input fingerprint after the CTest registration changed `tests/CMakeLists.txt`.

## Decisions Made

- Test fixtures start from declared constructed origins, but acceptance endpoints are reached only through production instructions. No synthetic in-flight CPU state, production pause callback, or public CPU state API was added.
- The model's existing global event ordering remains unchanged. These tests establish deterministic software continuation at modeled instruction boundaries; they do not claim the physical paired-chip tie precedence is settled.
- KEEP-03 remains pending because Plan 20-05/20-06 host replay and cross-platform evidence still remain.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Refreshed the AccuracyCoin input fingerprint for the added CTest target**
- **Found during:** Task 1 full CI verification.
- **Issue:** `accuracycoin.activation` rejected the old `build_inputs_sha256` because `tests/CMakeLists.txt` is an explicit input to the pinned campaign fingerprint.
- **Fix:** Updated only the measured `build_inputs_sha256` to `feb244fbf51273709ecfe8639772652325f3d993a1825f93b7d2d68f0381fe43`; the campaign, scoreboard, and ROM pins were unchanged.
- **Files modified:** `tests/accuracy/baseline-metadata.txt`
- **Verification:** `accuracycoin.activation` passed with its campaign fixture; the subsequent full workflow passed all 679 tests and packaged all three deliverables.
- **Committed in:** `354cf39`.

**Total deviations:** 1 auto-fixed (Rule 3 - blocking)
**Impact on plan:** The metadata fix restores the existing campaign guard after adding the planned test target; no emulation behavior or acceptance threshold changed.

## Verification

- `cmake --build --preset ci` — passed.
- `ctest --preset ci -R '^(core\.nmi_state|core\.state_reject|state\.format_pin)$' --no-tests=error --output-on-failure` — 3/3 passed.
- `ctest --preset ci -R '^accuracycoin\.activation$' --no-tests=error --output-on-failure` — campaign fixture and activation passed.
- `cmake --workflow --preset ci` — all 679/679 tests passed; package step generated library, runner, and libretro archives.

## TDD Notes

The plan adds regression/evidence tests for state and NMI behavior implemented in Plan 20-03; it does not change runtime behavior. The tests were run against that existing implementation. No artificial production regression was introduced to manufacture failing RED evidence.

## Issues Encountered

The first full workflow run caught the stale AccuracyCoin input fingerprint described above. After updating the measured fingerprint, activation and the full workflow passed. The exact physical CPU/PPU A/C tie remains a source-evidence limit already recorded by Plan 20-03.

## User Setup Required

None — no external service configuration required.

## Next Phase Readiness

Plan 20-04's local instruction-boundary cutpoint and state-rejection evidence is complete. Plan 20-05 can proceed with host replay; KEEP-03 remains pending until that later host/cross-platform evidence is available. The exact physical tie remains unclaimed.

---
*Phase: 20-nmi-line-and-vblank-flag*
*Completed: 2026-10-26*

## Self-Check: PASSED

- Summary and key evidence files exist.
- Task commits `904f739` and `354cf39` are ancestors of HEAD.
