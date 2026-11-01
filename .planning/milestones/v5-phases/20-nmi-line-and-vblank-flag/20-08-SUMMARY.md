---
phase: 20-nmi-line-and-vblank-flag
plan: "08"
subsystem: testing
tags: [nmi, savestates, audio, accuracycoin, hash-inventory]
requires:
  - phase: 20-nmi-line-and-vblank-flag
    provides: [NMI/VBlank state model, canonical state API, production hash inventory validator]
provides:
  - Equal-tick full-key inventory fixtures and duplicate/missing-key controls
  - Frame-zero empty-PCM canonical state and continuation witness
  - Refreshed local campaign provenance and bounded evidence ledger
affects: [Phase 20 verification, accuracycoin, state serialization]
actuals:
  tokens: 9046
  tasks: 3
  commits: 3
tech-stack:
  added: []
  patterns: [six-artifact validator fixtures, internal snapshot continuation checks]
key-files:
  created: []
  modified:
    - tests/cmake/hash_inventory.cmake
    - tests/core/test_nmi_state.c
    - README.md
    - include/nesturbator.h
    - tests/accuracy/baseline-metadata.txt
    - tests/accuracy/measurements.md
    - .planning/phases/20-nmi-line-and-vblank-flag/20-EVIDENCE.md
key-decisions:
  - "Keep full artifact keys as identity when frame and state observations share ticks; leave schema and native hash pins unchanged."
  - "Serialize APU sample phase and synthesizer/filter history independently of transient PCM staging."
  - "Treat synthetic inventory and internal frame-zero results as local evidence; keep hosted requirements pending."
patterns-established:
  - "Generate all six platform artifacts from one fixture and run valid plus invalid variants through the production inventory validator."
  - "Prove empty output by checking the ring, sentinel, canonical state bytes, and later nonempty continuation."
requirements-completed: []
requirements-pending: [KEEP-02, KEEP-03]
coverage:
  - id: D1
    description: "Equal-tick frame, state, PCM, and transition identities remain distinct and invalid keys are rejected by the production validator."
    requirement: KEEP-02
    verification:
      - kind: integration
        ref: "hash.inventory.self_test"
        status: pass
    human_judgment: false
  - id: D2
    description: "A frame-zero snapshot with empty PCM preserves nonzero saved audio history through canonical load and future execution."
    requirement: KEEP-02
    verification:
      - kind: unit
        ref: "core.nmi_state#test_frame_zero_empty_pcm_preserves_history"
        status: pass
    human_judgment: false
  - id: D3
    description: "The measured local AccuracyCoin campaign retains the six target passes and current input fingerprint."
    verification:
      - kind: integration
        ref: "ctest --preset ci -R '^accuracycoin\\.campaign$' --no-tests=error --output-on-failure"
        status: pass
    human_judgment: false
duration: 20min
completed: 2026-10-27
status: complete
commits: 3
plan_head_before: ccad71338b87e89a0a96524bffb596f06f6ff1d5
plan_head_after: f20c46f0a6a1b74baa89250af1575a9b5478e855
---

# Phase 20 Plan 08: Equal-tick identity and empty-output history witnesses Summary

**Production inventory controls now retain distinct lane keys at equal ticks, and a frame-zero state roundtrips with nonzero audio history while emitting no PCM.**

## Performance

- **Duration:** 20 minutes
- **Started:** 2026-10-27T03:51:31Z
- **Completed:** 2026-10-27T04:11:38Z
- **Tasks:** 3
- **Files modified:** 7

## Accomplishments

- Expanded `hash.inventory.self_test` with a valid equal-tick fixture for six named platform artifacts and controls for all-six duplicate/missing keys and a single-platform missing key.
- Added `test_frame_zero_empty_pcm_preserves_history`, including canonical load/save over a poisoned destination, empty-drain sentinel checks, matching continuation, and a bounded lost-history control.
- Updated the README and public comments to distinguish saved APU/synth history from transient PCM output, and refreshed the local campaign fingerprint and evidence without changing format, behavior revision, or native hash pins.

## Task Commits

Each task was committed atomically:

1. **Task 1: Trace equal-time lane identities through the actual artifact validator** - `8835027` (test)
2. **Task 2: Replay one frame-zero empty-output snapshot with saved nonzero audio history** - `df10aaf` (test)
3. **Task 3: Publish the two explicit witnesses and rerun the local behavior gate** - `f20c46f` (docs)

## Files Created/Modified

- `tests/cmake/hash_inventory.cmake` - Distinct equal-tick keys and full-key duplicate/missing controls in the existing six-artifact validator self-test.
- `tests/core/test_nmi_state.c` - Frame-zero audio-history serialization, empty-drain, continuation, and lost-history control.
- `README.md`, `include/nesturbator.h` - Document saved APU/synth history separately from output buffers and staged PCM.
- `tests/accuracy/baseline-metadata.txt`, `tests/accuracy/measurements.md` - Current campaign provenance and measured local witness results.
- `.planning/phases/20-nmi-line-and-vblank-flag/20-EVIDENCE.md` - Current local evidence and explicit remaining hosted limits.

## Measured Results

The synthetic validator fixture retained `dabg/p0/frame 60`, `dabg/p1/frame 60`, and `state/dabg 60` at 42,886,728 ticks, plus separate PCM and transition full keys. Its all-six duplicate control preserved row count and byte-identical platform files but was rejected for an unexpected full-key inventory; all-six and one-platform missing controls were rejected for their inventory and artifact reasons.

The combined audio witness starts at a constructed `(240,333)` origin and reaches `(241,1)` after one complete production JMP. At frame 0 and tick 72 it records sample phase 25,344, mixed level 11,540, impulse head 0, integrator 0, no staged PCM, and a null output pointer. Loading over a destination first advanced by a public frame yielded canonical bytes. Draining source and restored empty rings returned zero and preserved sentinel, ring, and state. Their later continuation matched guest count, state, frame/ticks, video, and nonzero 798-sample PCM. Resetting history in a third valid loaded instance was detected and caused a PCM difference at frame 1 sample 0.

One full-enabled local AccuracyCoin campaign completed 22/22 pages and 146 scored rows (114 pass, 32 fail, 0 skip, 0 not-run, 0 timeout, 5 DRAW). All six target keys passed. The 56-input fingerprint is `1d324c83a1aac5a69939eecc149cf4e0e7b86b7175f1da26064969fa7cd26c0c`, report SHA-256 is `29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac`, source revision is `df10aafd2099`, and `host_elapsed_guest_ns=8570616000`. `source_dirty=1` records the uncommitted README/header comments and the preserved pre-existing `.planning/config.json` edit. Native hash pins, campaign caps, state format 2, and behavior revision 8 were unchanged.

## Verification

- Task 1 focused inventory/hash checks: 4/4 passed.
- Task 2 focused state checks: 3/3 passed.
- Final focused gate: 10/10 passed, including the inventory self-test, NMI state witness, AccuracyCoin target/protected controls, and native hash comparison.
- `cmake --workflow --preset ci`: 683/683 passed; library, runner, and libretro packages generated.

The full campaign option was disabled before the focused checks and final CI workflow, so the measured campaign ran once. The synthetic fixture proves local validator behavior, not hosted six-platform artifact equality.

## Decisions Made

- Equal tick values do not merge distinct frame, state, PCM, or transition keys; expected full keys remain the validator oracle.
- Audio generation history is serialized machine state; PCM staging and caller output are transient.
- KEEP-02/KEEP-03 and all Phase 20 requirements stay pending for independent phase verification and hosted exact-head evidence. The evidence does not establish physical exact-tie precedence or clean-source results.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Regenerated the duplicate-key fixture before mutation**
- **Found during:** Task 1 (equal-time inventory validator)
- **Issue:** The initial duplicate-control experiment reused a fixture already changed by an earlier single-platform control, so it could fail for the wrong reason.
- **Fix:** Recreate the valid six-artifact fixture before applying each duplicate or missing-key mutation and assert equal counts and bytes for the all-six duplicate case.
- **Files modified:** `tests/cmake/hash_inventory.cmake`
- **Verification:** Focused inventory/hash checks passed 4/4; the duplicate was rejected by expected full-key inventory validation.
- **Committed in:** `8835027`

**2. [Rule 1 - Bug] Assert audio history at the executed frame-zero endpoint**
- **Found during:** Task 2 (combined frame-zero state witness)
- **Issue:** The constructed origin had zero sample phase before the production JMP, and the NMI guest marker advanced during the subsequent public frame.
- **Fix:** Check nonzero history at the executed `(241,1)` endpoint, require the marker before frame output, then compare matching post-frame markers and bounded lost-history divergence.
- **Files modified:** `tests/core/test_nmi_state.c`
- **Verification:** Focused state checks passed 3/3; the lost-history control diverged at frame 1, sample 0.
- **Committed in:** `df10aaf`

**Total deviations:** 2 auto-fixed (2 Rule 1 test-oracle corrections)
**Impact on plan:** Both corrections made the planned controls test the intended failure reasons; no scope or production behavior changed.

## Issues Encountered

The serial `retroarch.session.controls` CI matrix took 310.20 seconds. It passed, followed by the source-root and readiness controls; the complete workflow passed 683/683.

## User Setup Required

None - no external service configuration is required for this local plan.

## Next Phase Readiness

Plan 20-09 can proceed with local collector and retention preparation. Plan 20-10 still requires separately delivered CI/Nightly evidence before any task mutation or commit. Hosted exact-head six-platform equality, hosted performance, Nightly `vectors-full`, and physical exact-tie evidence remain outstanding. Keep Phase 20 and its requirements open.

## Self-Check: PASSED

The summary file and all three task commits are checked after writing; task commits are ancestors of the plan head recorded above.

---
*Phase: 20-nmi-line-and-vblank-flag*
*Completed: 2026-10-27*
