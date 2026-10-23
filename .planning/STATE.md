---
gsd_state_version: "1.0"
milestone: v4
milestone_name: Measured NTSC accuracy and robustness
current_phase: 17
current_phase_name: Complete pinned conformance baseline
current_plan: Not started
status: executing
stopped_at: "Phase 17 planned and checked; next: gsd-execute-phase 17"
last_updated: "2026-10-23T20:14:51.058Z"
state_head: baf802ec42e6f36671dc63137bca8717b5e78cbe
progress:
  total_phases: 3
  completed_phases: 1
  total_plans: 17
  completed_plans: 13
  percent: 33
last_activity: 2026-10-23
last_activity_desc: Phase 17 research and four sequential plans complete; independent plan verification passed
---

# Project State

## Project Reference

See [PROJECT.md](PROJECT.md), updated 2026-10-23, and AGENTS.md.
Core value: Games behave as on the console, identically across platforms, from a small MIT C library.
Current focus: Phase 17 — Complete pinned conformance baseline.

## Current Position

Phase: 17 (Complete pinned conformance baseline) — READY TO EXECUTE
Current Plan: Not started
Status: Ready to execute

Phase 16 is complete: 13/13 live plans, 22/22 verification truths and all six
requirements (EVID-01–04, DIR-01/02) verified. The normal completion handler
advanced to Phase 17. Phase 17 research and four plans in four sequential waves
are complete; implementation has not started.

## Phase 17 Planning

- [Research](phases/17-complete-pinned-conformance-baseline/17-RESEARCH.md)
  checks the immutable suite and existing runner/CI seams. New-pin campaign
  outcomes, accepted caps and CI placement remain execution measurements.
- Plans 17-01 through 17-04 cover the real tracer/inventory, bounded campaign,
  protected-main regression enforcement, then atomic canonical activation and
  measured CI placement. The old canonical strict checks stay active during
  comparison staging.
- Independent plan checking passed without findings. Four plan structures and
  frontmatters validate; all 10 task verification commands state failure signals;
  all four requirements and 15 tracked decisions are covered.
- No implementation tests or full conformance campaign ran in this planning step.

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

`$gsd-execute-phase 17` — execute the four verified plans in order.
Do not re-run Phase 16 gap planning or execution for completed work.

## Session

Last session: 2026-10-23T20:14:51.058Z
Stopped at: Phase 17 planned and checked; next: gsd-execute-phase 17
Resume file: .planning/phases/17-complete-pinned-conformance-baseline/17-01-PLAN.md
