---
phase: 16-host-evidence-and-roadmap
plan: "03"
subsystem: testing
tags: [retroarch, progression, host-evidence, one-shot-commands, deadlines, cmake]

requires:
  - phase: 16-host-evidence-and-roadmap
    provides: Qualified marker images, native state anchors, and the shared reason-coded progression verdict from Plans 01 and 02
provides:
  - Bounded one-shot RetroArch session driver with attributable captures, retained failure evidence, and owned-child cleanup
  - Exact pinned-host progression proof and reason-specific dropped, doubled, and wrong-origin controls
affects: [EVID-01, EVID-02, Phase 16 Plan 04]

actuals:
  tokens: 48743
  tasks: 3
  commits: 27
plan_head_before: dbc78b8b230e70db13e3da1159db4f9a600acf6a
plan_head_after: 46543ad84b62fc866c096882950abb7c8b2fa7b0

tech-stack:
  added: []
  patterns:
    - Retry only strictly parsed read-only readiness requests within live operation and attempt budgets
    - Treat mutations and artifact publication as one-shot operations with attributable postconditions
    - Preserve the first failure and reap only owned child processes

key-files:
  created:
    - .planning/phases/16-host-evidence-and-roadmap/16-03-SUMMARY.md
  modified:
    - .github/workflows/ci.yml
    - .planning/phases/16-host-evidence-and-roadmap/16-03-PLAN.md
    - CMakePresets.json
    - README.md
    - include/nesturbator.h
    - tests/retroarch/CMakeLists.txt
    - tests/retroarch/frame_index.c
    - tests/retroarch/progression.h
    - tests/retroarch/run_retroarch.cmake
    - tests/retroarch/state_session.sh
    - tests/retroarch/test_state_session.sh

key-decisions:
  - "Require two independently validated forward startup captures before the initial pause toggle, then equal captures to prove the pause held."
  - "Keep readiness reads and injected mutations on separate transports; only readiness retries, while mutations remain one-shot."
  - "Keep production operation deadlines and default attempt limit intact; size private test and workflow enclosures from measured runs."
  - "Record later successful job durations against the fixed last-40-run timeout audit instead of turning caps into a moving target."

patterns-established:
  - "A negative host control passes only after valid capture and native-origin prerequisites reach the expected production verdict reason."
  - "A deliberately stalled child must cross the intended fault injection point, record its owned PID, and prove child and host reaping."
  - "The host marker proof covers displayed progression; portable state and audio checks retain their separate guarantees."

requirements-completed: [EVID-01, EVID-02]
coverage:
  - id: D1
    description: "The production progression verdict accepts the intended pinned-host history and rejects dropped, doubled, and valid wrong-origin histories for their exact reasons."
    requirement: EVID-01
    verification:
      - kind: e2e
        ref: "GitHub Actions run 38651381295, exact head 46543ad; retroarch-marker-progression hosted artifact"
        status: pass
      - kind: integration
        ref: "cmake --workflow --preset ci; 643/643 local tests and progression ambiguity/origin controls"
        status: pass
    human_judgment: false
  - id: D2
    description: "Readiness recovery is bounded and mutation, publication, deadline, and cleanup controls preserve one-shot counts and exact failure reasons."
    requirement: EVID-02
    verification:
      - kind: integration
        ref: "ctest --preset ci -R '^retroarch.session.controls$' --output-on-failure; corrected stalled-action attempt-cap case"
        status: pass
      - kind: e2e
        ref: "GitHub Actions run 38651381295, exact head 46543ad; hosted intended and injected-control journal"
        status: pass
    human_judgment: false

duration: 24h 52m
completed: 2026-10-14
status: complete
---

# Phase 16 Plan 03: Bounded host progression Summary

**A bounded one-shot RetroArch driver now proves exact progression and reason-specific host failures through the shared production verdict.**

## Performance

- **Elapsed duration:** 24h 52m across the resumed execution, including hosted CI waits and interruptions.
- **Started:** 2026-10-13T22:35:02Z, the first retained Task 1 RED commit.
- **Completed:** 2026-10-14T23:26:53Z.
- **Tasks:** 3 complete.
- **Files modified:** 11 tracked files, including the final plan evidence record.
- **Actual tokens:** 48,743, rounded up from 194,971 changed implementation/documentation diff characters divided by four.

## Accomplishments

- Reworked the shell session driver around bounded, attributable operations. Only an exactly parsed read-only readiness request retries. Pause, advance, rewind, save, load, screenshot, run-ahead toggle, and quit actions are one-shot. Each receive, decode, poll, and child process consumes the live operation and attempt budgets; interruption cleanup preserves the first failure and reaps only owned processes.
- Added startup progress proof before the initial pause toggle, independent boundary captures, native-valid retained origins, production-verdict prefix checks, and a complete final verdict. The complete intended sequence retains 79 sends, 18 screenshots, 40 rewind sends, three pause sends, and all 16 canonical boundary relations.
- Completed the required pinned-host proof in workflow run 38651381295 on exact head 46543ad84b62fc866c096882950abb7c8b2fa7b0. The intended session returned VERDICT_OK at boundary 16 in 235.697 seconds (235.515-second attempt). The dropped and doubled advance controls each reached production PROGRESSION / ADVANCE_NOT_EXACTLY_ONE at boundary 3 after valid startup and native-origin prerequisites. The double shim forwarded exactly twice with a 425 ms gap. The valid wrong-origin control reached production ORIGIN at boundary 13 after 70 sends with valid original and rewound anchors.
- Retained and qualified the hosted artifact retroarch-marker-progression, including the sanitized command journal, startup captures, original and rewound native anchors, and fault-injection evidence. The two startup screenshots independently advanced from frame 290 to 363 before pause qualification.
- Kept the portable machine-state and audio claims separate from the displayed-frame host proof. Exact-head hosted ASan, hygiene, Linux x64/ARM64, macOS Intel/ARM64, Windows x64/ARM64, no-FP, hash equality, and the required rollup all passed.

## Task Commits

The persisted plan ledger measures 27 commits from dbc78b8b230e70db13e3da1159db4f9a600acf6a through task head 46543ad84b62fc866c096882950abb7c8b2fa7b0.

1. **Task 1 RED/GREEN:** 6ae5b69, 3b86f61.
2. **Task 2 RED/GREEN:** ed3eeb9, a763421.
3. **Task 3 and measured corrections:** d11d21d, 693fb82, 7cee27d, 7a5862d, 9f7730d, aedd4b0, 2201c86, 96d96e2, 8308084, 40b9313, 4cc5c35, ff8f0fa, 43c8a45, 3e933c9, d81d5d6, c9170ac, ba18bea, f8831c2, e2397db, 9d2eb0b, 39521b9, 13ad538, 46543ad.

## Verification

- Focused corrected action-attempt control: ctest --preset ci -R '^retroarch.session.controls$' --output-on-failure passed 1/1 in 280.19 seconds. The stalled action actually launched after valid startup and pause evidence; its advance-1 FRAMEADVANCE journal row recorded ATTEMPT_DEADLINE at 29,040 ms with 953 ms remaining, and both the child and host were reaped.
- Full local cmake --workflow --preset ci passed 643/643 tests and generated all three packages. The session-control case passed in 282.67 seconds in that run.
- Local cmake --workflow --preset hygiene passed 13/13. A local completed ASan control run took 555.02 seconds; the exact-head hosted ASan job also passed.
- GitHub Actions run 38651381295 completed successfully on the exact pushed head. The pinned RetroArch job took 1,258 seconds; the Intel build job took 1,028 seconds. The successful exact-head run includes the production host verdict and retained proof artifact.
- The fixed last-40 completed-run audit set the shared build cap to 39 minutes and the host cap to 40 minutes. This later run is recorded as an observation against those fixed caps, not used to recalculate them.

## Failure and Measurement Record

- Run 38648117487 on 13ad538 failed the Intel stalled-child-attempt-cap control before sending its intended blocked FRAMEADVANCE. Its terminal reason was DELAY_PAUSE_SETTLE_DEADLINE, so it had no stall PID or attempt-cap evidence and was not semantic proof. The test-only command-gap correction made the intended action launch; the later focused control proved the exact attempt deadline and reaping.
- An earlier focused assertion let the cleanup QUIT row overwrite the expected advance journal match. That assertion implementation failed and was corrected to select the advance-1 row; it is not recorded as a pass.
- A local RetroArch GUI launch aborted before readiness and sent zero actions. It remains a failed prerequisite, not a semantic negative or host evidence.
- The 1,258-second exact-head host job and 1,028-second Intel build are successful whole-job samples after the fixed timeout audit window. They are observations only; no cap change was made from them.

## Deviations from Plan

### Approved evidence-driven amendments and automatic fixes

**1. Startup-state discrimination.**
- **Found during:** Task 3 host-artifact diagnosis.
- **Issue:** Existing evidence did not distinguish an initially paused host from a lost or reset initial pause toggle.
- **Fix:** Require two independently published, validated startup captures with strict forward frame progress before the one-shot pause toggle, then require the existing equal pair to prove the pause held. Preserve fail-closed STARTUP_NOT_ADVANCING and PAUSE_DID_NOT_HOLD reasons, all 16 relations, 40 rewinds, and the exact action counts.
- **Files modified:** tests/retroarch/state_session.sh, tests/retroarch/test_state_session.sh, tests/retroarch/CMakeLists.txt, tests/retroarch/run_retroarch.cmake.
- **Verification:** Positive intended host run and exact dropped/doubled/wrong-origin reasons in run 38651381295.

**2. Readiness and action child supervision.**
- **Found during:** Task 3 hosted deadline and mutation-fault diagnosis.
- **Issue:** Read-only readiness fault injection and nested nc supervision could obscure whether a valid reply arrived within the real operation budget, while an interrupted mutation wrapper could retain an owned child.
- **Fix:** Keep fault wrappers on prescribed one-shot mutations and use the real read transport for readiness; nest the one-second nc receive allowance inside the bounded child-supervision slice and terminate/reap the exact owned child on interruption.
- **Files modified:** tests/retroarch/state_session.sh, tests/retroarch/test_state_session.sh, README.md, include/nesturbator.h.
- **Verification:** Delayed exact-read, silent/stalled child, interruption, send-count, exact-reason, and reap controls passed; exact-head host proof passed.

**3. Measured test and hosted enclosures.**
- **Found during:** Task 3 local and hosted verification.
- **Issue:** Several finite test enclosures expired before reaching their intended fault or before the remaining prescribed complete-mode operations, although the underlying operation limits were valid.
- **Fix:** Adjust only evidenced test-mode attempt and enclosing CTest/job allowances. Preserve the 120-second production default, per-operation limits, exact reasons, action counts, and cleanup reserve. The fixed audit set supports 39-minute build and 40-minute host caps; later successful samples remain observations.
- **Files modified:** CMakePresets.json, tests/retroarch/CMakeLists.txt, tests/retroarch/run_retroarch.cmake, .github/workflows/ci.yml.
- **Verification:** Local CI, ASan, hygiene, and exact-head hosted matrix passed; the final host job completed in 1,258 seconds.

**4. Stalled-action fixture setup.**
- **Found during:** Task 3 hosted Intel control failure.
- **Issue:** Test-only command-gap pacing expired during pause settle, before the intended blocked action.
- **Fix:** Set command gap to zero only for the stalled-write fixture. Keep pause and capture prerequisites, operation/attempt deadlines, owned PID evidence, and cleanup requirements.
- **Files modified:** tests/retroarch/test_state_session.sh.
- **Verification:** The targeted test reached the blocked FRAMEADVANCE, returned the exact ATTEMPT_DEADLINE row, and reaped both child and host; full local CI/hygiene and exact-head CI passed.

**Total deviations:** 4 approved evidence-driven amendments or automatic corrections. **Impact:** Kept the host proof fail-closed and bounded; no public API, dependency, state format, emulator behavior, or universal UDP-delivery claim was added.

## Decisions Made

Two forward startup captures now establish that the host is advancing before the initial pause action; equal captures after that action establish that the pause held. Each negative must reach its production verdict after valid prerequisites. Production operation budgets remain unchanged. The latest completed-run audit is fixed, so future runs are observations rather than an automatic moving timeout target.

## Issues Encountered

Automatic approval review initially rejected publication of the existing phase branch until the user directly authorized the exact repository push and required CI. The root agent then used the normal reviewed push path; no workaround was used. The final exact-head run succeeded. No authentication credential gate or package installation occurred.

## User Setup Required

None.

## Next Phase Readiness

EVID-01 and EVID-02 are complete with exact-head host evidence and portable controls. Plan 16-03 is ready to hand off; Plans 16-04 and 16-05 remain for the already planned phase execution step.

## Self-Check: PASSED

- The summary path and all 27 task commits were verified on the current branch.
- The plan evaluation scope resolved and the changed implementation diff contains no TODO, FIXME, placeholder, or coming-soon stubs.
- The latest local CI/hygiene results and exact-head hosted run pass the Plan 16-03 verification gates.

---
*Phase: 16-host-evidence-and-roadmap*
*Plan: 03*
*Completed: 2026-10-14*
