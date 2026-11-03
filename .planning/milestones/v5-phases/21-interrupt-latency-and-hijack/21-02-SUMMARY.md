---
phase: 21-interrupt-latency-and-hijack
plan: "02"
subsystem: cpu-interrupts
tags: [C, IRQ, save-state, AccuracyCoin, CTest]
requires:
  - phase: 21-interrupt-latency-and-hijack
    provides: Source-backed IRQ poll schedules and pre-fix reproducers from Plan 21-01
provides:
  - Per-instance IRQ acceptance at legal polls, including old-I handling and branch poll accumulation
  - Native state format 3 and behaviour revision 9 with accepted IRQ history
  - Full local campaign, protected-pass comparison, hash pins, and generated corpus verification
affects: [21-03, 21-04, 21-05, 21-06, 21-08, 21-09]
actuals:
  tokens: 13808
  tasks: 3
  commits: 1
commits: 1
plan_head_before: d6563967d90ba154b445297c16d6494a55b94486
plan_head_after: f4580618e5dbc768bd3d073c00135a050559da21
tech-stack:
  added: []
  patterns:
    - Keep accepted IRQ history distinct from the live line and raw bus poll sample.
    - Accumulate only the source-defined legal branch polls.
key-files:
  created: []
  modified:
    - src/cpu.c
    - src/internal.h
    - src/state.c
    - include/nesturbator.h
    - README.md
    - tests/core/test_irq_poll.c
    - tests/core/test_nmi_state.c
    - tests/core/test_state_reject.c
    - tests/cpu/test_cpu_unit.c
    - tests/accuracy/measurements.md
    - tests/accuracy/scoreboard.txt
    - tests/accuracy/baseline-metadata.txt
    - tests/runner/hashes.txt
    - tests/cmake/phase20_hosted_evidence.cmake
key-decisions:
  - "Keep the approved format 3 / behaviour revision 9 change with unchanged ABI and refuse native formats 1 and 2."
  - "Record the measured test-E latency divergence; INT-01 remains pending until its AccuracyCoin row passes."
  - "Keep KEEP-01 pending until protected passes are confirmed across every CI lane."
patterns-established:
  - "IRQ acceptance: commit source-qualified eligibility at an instruction's legal poll and consume it after the instruction boundary."
  - "State compatibility: serialize each new historical decision with a bounded field and prove refused-load non-mutation."
requirements-completed: []
coverage:
  - id: D1
    description: Poll-time IRQ acceptance, old-I transitions, branch accumulation, and independent source reproducers
    requirement: INT-01
    verification:
      - kind: unit
        ref: "ctest --test-dir build/ci -R '^(cpu.unit|core.irq_poll|core.nmi_poll|core.nmi_edge|ppu.nmi_races|cpu.vectors..*)$'"
        status: pass
      - kind: integration
        ref: "accuracycoin.campaign: Interrupt flag latency"
        status: fail
    human_judgment: true
    rationale: The full AccuracyCoin row remains fail at 0x03/3; source test E diverges before its BCC C1 poll and C3 acknowledgement sequence.
  - id: D2
    description: Format 3 / behaviour revision 9 state round-trip, old-format refusal, and untouched-instance guarantees
    verification:
      - kind: unit
        ref: "ctest --test-dir build/ci -R '^(core.state_reject|core.state_generation|core.nmi_state|core.boundary)$'"
        status: pass
      - kind: integration
        ref: "cmake --workflow --preset ci"
        status: pass
    human_judgment: false
  - id: D3
    description: Full local campaign preserves the protected 114 PASS keys and refreshes state hashes
    requirement: KEEP-01
    verification:
      - kind: integration
        ref: "accuracy.scoreboard.no_lost_pass and accuracycoin.activation"
        status: pass
      - kind: integration
        ref: "cmake --workflow --preset ci on macOS arm64"
        status: pass
    human_judgment: true
    rationale: KEEP-01 requires confirmation on every CI lane; this plan ran the local workflow only.
duration: 62min
completed: 2026-10-29
status: complete
---

# Phase 21 Plan 02: Poll-time IRQ Acceptance and State Compatibility Summary

**The CPU now records IRQ eligibility at legal instruction polls and saves that history in format 3; all local CI checks pass, while the full latency row remains an open Phase 21 requirement.**

## Performance

- **Duration:** 62 min
- **Started:** 2026-10-29T01:50:30Z
- **Completed:** 2026-10-29T02:52:24Z
- **Tasks:** 3
- **Files modified:** 21

## Accomplishments

- Added per-instance IRQ acceptance separate from the live line and raw bus poll. CLI, SEI, and PLP use the pre-instruction I flag; RTI uses restored I. Branches retain the first legal poll and add the page-cross poll without allowing a later sample to erase earlier acceptance. NMI keeps priority, and reset clears accepted history.
- Serialized accepted IRQ history with a bounded field. State format 3 and behaviour revision 9 leave the public ABI unchanged; CRC-correct formats 1 and 2 are refused before instance or save-generation mutation. README and public-header comments describe the compatibility and polling behavior.
- Completed the 22-page AccuracyCoin campaign at the existing 32/5/1800 caps. The run observed 146 scored rows and reported 114 pass, 30 fail, 2 not-run, 0 skip, 0 timeout, and 5 inventory-only DRAW rows. All 114 protected starting PASS keys remain PASS. Campaign and input-fingerprint hashes are recorded in the measurement metadata.
- Refreshed state hashes and sizes for the serialized byte and format/revision identity. Frame, PCM, and APU-transition hashes did not move. The state corpus generator and verifier matched all 17 listed entries without changing seed bytes; the ROM manifest and ROM assets were unchanged.
- The final local `cmake --workflow --preset ci` run passed 690/690 tests and packaged the library, runner, and libretro adapter.

## Task Commits

The plan's three tasks were committed together as one causal change, as required by its atomicity contract:

1. **Task 1: Accept IRQ using the I flag at legal polls** — `f458061` (`fix`)
2. **Task 2: Persist accepted history under the approved compatibility change** — `f458061` (`fix`)
3. **Task 3: Refresh only measured poll campaign, hashes and owned corpus** — `f458061` (`fix`)

The execution metadata commit is created after this summary and is reported by the executor.

## Files Created/Modified

- `src/cpu.c`, `src/internal.h`, `src/state.c` — Track, consume, reset, serialize, and validate accepted IRQ history.
- `include/nesturbator.h`, `README.md` — Publish format/revision and polling semantics.
- `tests/core/test_irq_poll.c`, `tests/cpu/test_cpu_unit.c`, `tests/core/test_nmi_state.c`, `tests/core/test_state_reject.c` — Promote poll defect reproducers to passing assertions and cover save/load and refusal behavior.
- `tests/accuracy/scoreboard.txt`, `tests/accuracy/baseline-metadata.txt`, `tests/accuracy/measurements.md`, `tests/runner/hashes.txt` — Preserve the measured campaign, protected comparison, provenance, and hash reasons.
- `tests/cmake/accuracycoin_activation.cmake`, `tests/cmake/phase20_hosted_evidence.cmake`, `tests/cmake/phase20_hosted_evidence_controls.cmake`, `tests/cmake/vector_api_policy.cmake` — Update measured fixture expectations and compatibility pins.
- `tests/core/test_api.c`, `tests/fuzz/state_loader.c`, `tests/header/header_c.c`, `tests/header/header_cxx.cpp` — Update revision and serialization-layout pins.

## Decisions Made

- Used the already approved native format 3 / behaviour revision 9 contract, with ABI unchanged and no format converter.
- Kept test E's source-defined order explicit in the measurement: its first BCC poll sees IRQ, then acceptance must survive the C3 `$4015` acknowledgment.
- Left `INT-01` and `KEEP-01` pending. The first remains pending because the full `Interrupt flag latency` row is fail; the second remains pending because only the local CI lane was run.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Updated the hosted-evidence validator for measured `not-run` rows**
- **Found during:** Task 3 full CI run
- **Issue:** The completed campaign correctly classified two rows as `not-run`, but the Phase 20 validator accepted only terminal `pass`/`fail` test rows and hard-coded all 146 rows as terminal.
- **Fix:** Accept the existing hyphenated scoreboard status, count not-run rows from current metadata, and validate terminal count against the campaign metadata. Historical Phase 20 synthetic fixture metadata remains revision 8 with its original 108/six-target controls.
- **Files modified:** `tests/cmake/phase20_hosted_evidence.cmake`, `tests/cmake/phase20_hosted_evidence_controls.cmake`
- **Verification:** Hosted-evidence controls and adapter passed; final workflow passed 690/690.
- **Committed in:** `f458061`

**2. [Rule 3 - Blocking] Refreshed tests coupled to state-format and behavior-revision constants**
- **Found during:** Full workflow compatibility run
- **Issue:** Direct API, public declaration digest, and JAM-oracle checks still pinned revision 8, format 2, or the old serialized jammed-byte offset.
- **Fix:** Pin revision 9 / format 3, update the declaration digest, and move the JAM fixture mutation to offset 93 after the new serialized acceptance byte.
- **Files modified:** `tests/core/test_api.c`, `tests/cmake/vector_api_policy.cmake`, `tests/fuzz/state_loader.c`
- **Verification:** All five previously failing direct CTests passed; final workflow passed 690/690.
- **Committed in:** `f458061`

**Total deviations:** 2 auto-fixed blocking compatibility issues.
**Impact on plan:** Both fixes were required to preserve existing validator and test coverage under the measured campaign status and approved format change.

## TDD Gate Compliance

Task 1 and Task 2 captured semantic RED evidence in `build/ci/21-02-cpu-red-evidence.json`, `build/ci/21-02-core-red-evidence.json`, `build/ci/21-02-state-reject-red.json`, and `build/ci/21-02-nmi-state-red.json`. The named targets failed on the planned behavior assertions, and each record contains a semantic assessment. `workflow.tdd_mode` was false and the plan's `change_atomicity` requires one causal GREEN commit after all tests, state compatibility, pins, and corpus checks; the plan therefore has one `f458061` commit rather than separate RED/GREEN commits.

## Issues Encountered

The measured campaign did not pass the full interrupt-latency row. Tests 1 through D and their DMC/open-bus prerequisites completed. In test E, the pinned source expects IRQ on the first BCC poll at C1, with that acceptance surviving the later C3 `$4015` dummy-read acknowledgment. The candidate trace already had accepted history after the preceding `JSR $4013` step and before the target instruction: PC `$4013`, `X=$5A`, `P=$20`, and IRQ line, captured level, poll sample, and accepted history asserted. This is the first recorded divergence from the source schedule. The later handler record showed return PC `$10A1`, followed by guest JAM at `$10A6` (`jam_frame=82`, raw `$03`). The two overlap rows were therefore unstarted and remain assigned to Plan 21-04. This local campaign is not evidence that `INT-01` is complete.

The best-effort broken-windows append for the validator deviation was rejected
before writing because the existing ledger's frontmatter/table counts disagree
with its fenced JSON entries (the JSON is missing row 18). The unrelated ledger
was left unchanged.

No stubs or unrun plan verification remain. The pre-existing `.planning/config.json` edit was preserved and excluded from the commit.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

Plan 21-03 can consume the source-backed poll implementation and campaign evidence. Phase 21's target rows and the every-lane protected-pass requirement remain open; no hosted or remote workflow was run.

## Self-Check: PASSED

- Summary file exists at the planned path.
- Commit `f458061` is an ancestor of the current branch head.
- `git diff --check` passed.

---
*Phase: 21-interrupt-latency-and-hijack*
*Completed: 2026-10-29*
