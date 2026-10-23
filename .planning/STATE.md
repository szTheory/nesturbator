---
gsd_state_version: "1.0"
milestone: v4
milestone_name: Measured NTSC accuracy and robustness
current_phase: 16
current_phase_name: Host evidence and roadmap
current_plan: 13
status: ready_to_execute
stopped_at: Completed 16-07-PLAN.md; Plan 16-08 is ready for release-byte validation
last_updated: "2026-10-23T17:58:19.888Z"
state_head: 788cc12123b372570080e5c8bde36861968ec1bf
progress:
  total_phases: 3
  completed_phases: 0
  total_plans: 13
  completed_plans: 12
  percent: 92
last_activity: 2026-10-23
last_activity_desc: Plan 16-07 published v0.1.11 on the exact successful release commit; Plan 16-08 is ready to verify downloaded bytes
---

# Project State

## Project Reference

Core value: Games behave as on the console, identically across platforms, from a small MIT C library.
See PROJECT.md and AGENTS.md. Current phase:16—Host evidence and roadmap.

## Current Position

Current Plan: 13
Total Plans in Phase: 13
Phase: 16 (Host evidence and roadmap) — EXECUTING
Plan 16-56 complete (3/3); Plan 16-07 complete (3/3) with exact-head consent and published v0.1.11 candidate. Plan 16-08 is ready to verify downloaded release bytes; EVID-04 remains open.
13 live / 56 physical / 12 completed plans. Phase 16 remains incomplete.

Read [16-RECOVERY.md](phases/16-host-evidence-and-roadmap/16-RECOVERY.md) first.
Its direct development contract remains active: no retired admission/transport/
snapshot/clock/slot/helper preparation prerequisites and no Plan57. Ordinary
implementation/tool failures are diagnosed and corrected within execution.
Do not route a diagnosed local failure back to planning by default.

## Completed Current-Source Acceptance

Source commits2e9928b and2c356d5, metadata52f6915. Final tested source candidate:
2c356d574290aef1d57122566f7b81beaaa10027.
[56SUMMARY](phases/16-host-evidence-and-roadmap/16-56-SUMMARY.md),
[source acceptance](phases/16-host-evidence-and-roadmap/16-56-SOURCE-ACCEPTANCE.md),
[independent review](phases/16-host-evidence-and-roadmap/16-56-REVIEW.md) and
[disposition](phases/16-host-evidence-and-roadmap/16-56-REVIEW-DISPOSITION.md).

- Harness identity derives from its own invocation; copied driver/include
  refused before sourcing. All3cwd/12finalizer cases passed.
- Final-source same-oracle CR-01 RED/GREEN distinguishes old foreign-object
  damage from current preservation; owned reap and exact-port reentry passed.
- Independent review found a valid alternate-port socket bypass. Corrected
  production reentry to require55355, added real-socket negative control and
  reran affected/current acceptance. Final exact-source standard review clean.
- Focused CI and ASan/UBSan both2/2. Full CI and independent parent build/fullCI
  passed:649/649, no skips, four CTest jobs, configure/build/test/package and
  three fresh package/source matches. Final parent controls320.45s, total324.18s.
- Existing candidate defects found by focused tests were corrected locally:
  absent lock-root creation, stale exclusive observation in retry fixture,
  unset interruption fixture variable. First failures retained in build logs.
- Tested/committed11-path source manifest matches. Paired five source/test/doc
  paths committed with noreply hooks. Config remains preexisting dirty input,
  exact/unstaged (SHA2560517388f5d422ae97afdc5dc1ae11bc43ebd481a757424f152ee8af61559ff5e).

## Remaining Gates

Plan 16-08 must independently download and validate the actual v0.1.11 release
archives, checksums, attestations, six metadata copies and exact-release CI.
Plan 16-07's exact-head PR consent, protected merge, and normal publication
flow are recorded in 16-RELEASE-DELIVERY.md. EVID-04 and Phase 16 stay open
until Plan 16-08 and phase verification are complete.

All six requirements and D-01–D-17remain. EVID-02retains its existing complete
status; other provisional phase requirement marks are unchanged. Finite test
success proves no universal reliability or future milestone compliance.

## Preserved History and Lessons

Completed01–06,09,15,21,30retain historical credit;51–54remain superseded
unexecuted history. Old failed checkpoints/build trees are DATA, not replay
prerequisites.26active/terminal=false/pending and missing originals/UNKNOWNs
stay unchanged; no historical failure is relabeled or lost original invented.

[Loop forensics](forensics/report-20261022-234424.md) explains62consecutive docs
commits and expanding administrative gates. Full earlier STATE/ROADMAP and old56
remain exact at227bd6a. Keep current state short and read history only for a
concrete question. This execution demonstrated the recovery: fix the actual
source, retain first failures, add a discriminating test, correct within the
same step, rerun affected acceptance and obtain independent review.

## Next Action

Plan 16-08 is the next ready plan: verify the actual published v0.1.11
archives and six metadata copies against the recorded release candidate. Do not
mark EVID-04 or Phase 16 complete until that byte validation and phase-level
verification pass.

## Session

**Last session:** 2026-10-23T17:58:19.863Z
**Stopped at:** Completed 16-07-PLAN.md; Plan 16-08 is ready for release-byte validation
**Resume file:** None

## Performance Metrics

| Plan | Duration | Tasks | Files |
|------|----------|-------|-------|
| Phase 16 P07 | 101m | 3 tasks | 10 files |

## Decisions

- [Phase 16]: Use the existing protected-main release flow without selecting a version; the release PR generated v0.1.11.
- [Phase 16]: Keep EVID-04 open until Plan 16-08 validates the actual downloaded release bytes.
