---
gsd_state_version: "1.0"
milestone: v4
milestone_name: Measured NTSC accuracy and robustness
current_phase: 17
current_phase_name: Complete pinned conformance baseline
current_plan: 3
status: executing
stopped_at: Completed 17-02-PLAN.md
last_updated: "2026-10-23T23:11:29.501Z"
state_head: dbcc64190d9a6269eb1ac1a437709c51ff37fdab
progress:
  total_phases: 3
  completed_phases: 1
  total_plans: 17
  completed_plans: 15
  percent: 33
last_activity: 2026-10-23
last_activity_desc: Plan 17-02 passed full CI; bounded new-pin campaign measured and retained raw outcomes
---

# Project State

## Project Reference

See [PROJECT.md](PROJECT.md), updated 2026-10-23, and AGENTS.md.
Core value: Games behave as on the console, identically across platforms, from a small MIT C library.
Current focus: Phase 17 — Complete pinned conformance baseline.

## Current Position

Phase: 17 (Complete pinned conformance baseline) — EXECUTING
Current Plan: 3
Total Plans in Phase: 4
Status: Ready to execute
Progress: [███░░░░░░░] 33%

Phase 16 is complete: 13/13 live plans, 22/22 verification truths and all six
requirements (EVID-01–04, DIR-01/02) verified. The normal completion handler
advanced to Phase 17. Phase 17 research and four plans in four sequential waves
are complete. Plan 17-01's comparison fixture, inventory, tracer and old-pin
evidence now pass full CI. Plan 17-02's bounded new-pin campaign also passed
full CI and produced retained outcomes for all 22 pages. Protected-main
regression work and canonical activation remain for Plans 17-03 and 17-04.
The planning step itself ran no implementation tests.

## Phase 17 Planning

- [Research](phases/17-complete-pinned-conformance-baseline/17-RESEARCH.md)
  checks the immutable suite and existing runner/CI seams. New-pin campaign
  outcomes, accepted caps and CI placement remain execution measurements.
- Plans 17-01 through 17-04 cover the real tracer/inventory, bounded campaign,
  protected-main regression enforcement, then atomic canonical activation and
  measured CI placement. Plan 17-01 now passes full CI with the comparison
  fixture admitted; the old canonical strict checks stay active through staging.
- Independent plan checking passed without findings. Four plan structures and
  frontmatters validate; all 10 task verification commands state failure signals;
  all four requirements and 15 tracked decisions are covered.
- The planning step ran no implementation tests. Plan 17-01's production
  tracer, inventory, status and old-page evidence passed full CI; Plan 17-02's
  bounded new-pin campaign also passed full CI across all 22 pages.

## Phase 16 Evidence

- [Independent verification](phases/16-host-evidence-and-roadmap/16-VERIFICATION.md)
  passes with a canonical current-input fingerprint.
- [Release evidence](phases/16-host-evidence-and-roadmap/16-RELEASE-EVIDENCE.md):
  v0.1.11, source e47feb3afc34baf1d611ef44153af199424f41a2, exact successful
  Release run 39993108038; 18 actual ZIP checksums and attestations, six truthful
  MMC3 metadata copies, all checked against the immutable release source.
- [Delivery](phases/16-host-evidence-and-roadmap/16-RELEASE-DELIVERY.md): approved
  exact PR #50 head, protected merge and normal release-please publication.
- [Current review](phases/16-host-evidence-and-roadmap/16-REVIEW.md) is clean.
  WR-07's missing preflight CI registration was fixed in e80af77: full CI
  650/650, no skips, three packages; focused ASan preflight 1/1.
- [Source acceptance](phases/16-host-evidence-and-roadmap/16-56-SOURCE-ACCEPTANCE.md)
  and its review/disposition preserve the actual source-root, lock ownership,
  finalizer and causal RED/GREEN evidence.

## Decisions and Lessons

1. Resume from the canonical live plan index and completed summaries. Retired
   admission/helper/transport/snapshot/clock/slot gates are historical data;
   [16-RECOVERY.md](phases/16-host-evidence-and-roadmap/16-RECOVERY.md) is the
   authority for the completed phase. No Plan 57 is needed.
2. Diagnose concrete failures within execution. The Sigstore verifier needed
   a writable task-local XDG cache; its exact source/signer policy stayed intact.
   Code review caught a real missing CI registration, which was fixed and tested
   in this same step.
3. Source/package success and publication proof are distinct. Keep exact release
   identities and actual bytes; complete the phase only after independent goal
   verification passes. Finite checks do not establish universal reliability or
   future milestone compliance.

## Preserved Debt and History

- Separate Nightly run 39993108154: vectors-full failed; suite-flake passed.
  This is not a required Release gate and remains unresolved debt.
- Completion warned that 16-05-SUMMARY.md lists two command strings as file paths:
  `cmake -DDIR=build/ci -P tests/cmake/check_archives.cmake` and
  `bash scripts/hygiene.sh`. These metadata warnings do not change the verdict.
- Old failed checkpoints, missing originals/UNKNOWNs and Plan 26's historical
  active/pending observation remain unchanged. Superseded plans gain no false
  completion credit. No broad build-tree traversal or historical replay.
- [Loop forensics](forensics/report-20261022-234424.md) records the earlier
  62-document-commit loop. Original state/roadmap are retained at 227bd6a.
- Preexisting `.planning/config.json` edits remain exact and unstaged.

## Next Action

`$gsd-execute-phase 17` — continue with Plan 17-03, protected-main authority,
actual protected measurements and source-backed migration enforcement. Keep the
phase-level CONF requirements pending until the phase is verified.

## Session

Last session: 2026-10-23T23:11:29.339Z
Stopped at: Completed 17-02-PLAN.md
Resume file: None

## Performance Metrics

| Plan | Duration | Tasks | Files |
|------|----------|-------|-------|
| Phase 17 P01 | 54min | 3 tasks | 15 files |
| Phase 17 P02 | 67min | 3 tasks | 12 files |

## Decisions

- [Phase 17]: Keep 673ef550 canonical while admitting 6bee919 as a comparison fixture; do not activate the new pin in Plan 17-01.
- [Phase 17]: Accept nonstandard pass bytes only for exact source-defined decoded keys; classify $02 as a valid failure and DRAW sentinel rows as inventory-only.
- [Phase 17]: Retain old protected measurements and mark all corresponding new-pin observations unmeasured until the later campaign.
- [Phase 17]: The fixed five-Right navigation input is the measured campaign unit: six inputs change guest timing and the active page19 row, so cap-six exploration remains separately labeled.
- [Phase 17]: Keep both raw navigation-sensitive page19 observations; timeout only the source-confirmed active row, retain stale running markers as not-run, and do not fall back when all bytes were observed.
