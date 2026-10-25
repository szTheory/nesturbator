---
gsd_state_version: "1.0"
milestone: v4
milestone_name: Measured NTSC accuracy and robustness
current_phase: 18
current_plan: Not started
status: completed
stopped_at: Phase 18 complete — all phases complete
last_updated: "2026-10-25T01:58:53.954Z"
state_head: "0bc644288cadd4cdfebac0b0d6188f6709f66fb0"
progress:
  total_phases: 3
  completed_phases: 3
  total_plans: 21
  completed_plans: 21
  percent: 100
last_activity: 2026-10-24
last_activity_desc: Phase 18 independently verified (4/4 ROB requirements) and marked complete; milestone v4 audit is next
---

# Project State

## Project Reference

See [PROJECT.md](PROJECT.md), updated 2026-10-24, and AGENTS.md.
Core value: Games behave as on the console, identically across platforms, from a small MIT C library.
Current focus: milestone v4 closeout — all three phases complete.

## Current Position

Phase: 18
Current Plan: Not started
Total Plans in Phase: 4
Status: All phases complete
Progress: [██████████] 100% of milestone phases (3/3)
Written plans: 21 total; all 21 plans have summaries; Phase 18 is independently verified.

Phase 17 is complete: four sequential plans, 38/38 independently verified
roadmap/plan truths, all 15 locked decisions honored, and CONF-01 through
CONF-04 Complete. Phase 18 is complete: four plans, independent goal verification
passed 4/4 (ROB-01 through ROB-04 Complete), review findings WR-01..WR-05 fixed.

## Phase 18 Planning

- Research identifies the current execution, lifecycle and CI seams. Local
  sanitizer coverage is available; libFuzzer mutation uses the existing Linux
  toolchain because the local runtime is unavailable.
- Four sequential plans contain ten tasks: execution tracer and initial
  measurement; six-family reach and lifecycle controls; loop/JAM and adapter
  presentation; measured replay/campaign budgets, artifacts and scoped findings.
- Independent plan checking passed after one narrow revision resolved a
  context-budget warning with code/data/docs grouping and task-scoped reading.
  Estimates remain uncalibrated projections, not measured implementation cost.
- Automated structure checks pass for all four plans. Requirement coverage is
  4/4, decision coverage 16/16 and post-planning coverage 20/20. All 12 planned
  verification commands state failure signals. The path probe does not support
  CMake/CTest; independent source inspection checked command feasibility.
- Five classified edge-probe rows have explicit acceptance truths. Two
  unclassified rows remain flagged assumptions; the locked campaign and halt
  acceptance contracts still apply. No kept prohibition was silently dropped.
- No prototype, product test or campaign ran during planning. Plan 18-02 later
  measured useful execution reach across all six families and added 24 core
  functions beyond the loader corpora. Plan 18-03 completed loop/JAM and adapter
  checks; Plan 18-04 completed measured replay/campaign budgets, evidence
  controls and the scoped presentation correction. Independent verification
  ([18-VERIFICATION.md](phases/18-execution-and-lifecycle-robustness/18-VERIFICATION.md))
  passed and ROB-01 through ROB-04 are Complete.

## Phase 17 Evidence

- [Independent verification](phases/17-complete-pinned-conformance-baseline/17-VERIFICATION.md)
  passes through the verification owner with a current covered-input fingerprint.
- [Exact-source CI](https://github.com/szTheory/nesturbator/actions/runs/40075421001)
  passed all 15 jobs on source `c23d0ac845ed405147c7006e87400abeda94b8cb`:
  primary Linux 663/663 tests, all six platform lanes, ASan, both no-FP lanes,
  real base-to-PR perf, 114-key hash equality, RetroArch E2E and required rollup.
- Canonical pin `6bee919529289c555803b612b8c9e4f7eaaeeabb` has 146 scored
  entries plus five DRAW entries. The bounded campaign observed all 146 rows:
  101 pass, 42 fail, zero skip, two not-run and one timeout; 143 are terminal.
  All 22 independent page attempts and causes are retained. This is a baseline,
  not an all-pass or universal gameplay claim.
- All 17 old protected passes remain measured and mapped. Same-pin protection
  derives its fresh page set from the immutable protected-main PASS inventory;
  the production next-main control proves all 101 new passes. Canonical full
  scoreboard bytes are authenticated before PASS projection.
- [Review](phases/17-complete-pinned-conformance-baseline/17-REVIEW.md) is clean.
  CR-01's duplicate-final-raw acceptance was fixed with per-key coverage and
  production-path RED/GREEN controls; its
  [disposition](phases/17-complete-pinned-conformance-baseline/17-REVIEW-DISPOSITION.md)
  and [fix evidence](phases/17-complete-pinned-conformance-baseline/17-REVIEW-FIX.md)
  remain recorded.
- [Regression gate](phases/17-complete-pinned-conformance-baseline/17-REGRESSION.md)
  binds the current source to hosted command results and the downloaded artifact
  hash. Local full-run timeouts during logged macOS maintenance sleeps remain
  failures; the successful hosted run supplies the final full gate.
- [Draft PR #54](https://github.com/szTheory/nesturbator/pull/54) remains open,
  mergeable and unmerged. Tested production source is pushed; subsequent
  completion/review records are committed locally to avoid repeating unchanged
  source verification solely for a tracking commit.

## Phase 16 Evidence

- [Independent verification](phases/16-host-evidence-and-roadmap/16-VERIFICATION.md)
  passed at closeout with its source-bound fingerprint; Phase 17 has its own verification.
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
4. Treat inventory, attempts, observed bytes, terminal status and infrastructure
   outcomes separately. Guest-frame bounds stay deterministic; host deadlines
   are measured enclosures, and a guest timeout can be trustworthy observation.
5. Protect each accepted PASS using fresh same-invocation raw evidence selected
   from one protected-main revision. Require unique per-key final coverage;
   neither aggregate counts nor a candidate's PASS assertions prove protection.

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
- Phase 17 completion reported three historical metadata references: the staging
  `accuracycoin-6bee.nes` intentionally removed by atomic activation; shorthand
  `.planning/preparation/{CONFORMANCE,LIBRETRO-AND-RUNNER}.md` for two existing
  documents; and the absent optional `scripts/ci_monitor.cjs`, for which GitHub
  Actions API/CLI evidence was used. These are not implementation or CI gaps.
- [Phase 18] Real-campaign minimization is proven only by controls; no real
  campaign finding has been reduced. Exact-head Nightly 40150888217 was still
  running at phase completion (its campaign jobs and vectors-full had passed).

## Next Action

Milestone v4 audit done: tech_debt, 14/14 requirements, no blockers
([v4-MILESTONE-AUDIT.md](v4-MILESTONE-AUDIT.md)). Complete the milestone next
(`/gsd-complete-milestone v4`). Deliver Phase 18 through its PR after exact-head Nightly
40150888217 finishes.
Review/deliver Phase 17 through draft PR #54 using the normal PR workflow.
Do not re-execute completed plans.

## Session

Last session: 2026-10-25T02:05:00Z
Stopped at: Phase 18 complete — all phases complete
Resume file: None

[16-RECOVERY.md](phases/16-host-evidence-and-roadmap/16-RECOVERY.md) remains the
historical authority for completed Phase 16. Its retired helper/admission/
transport/snapshot/clock/slot gates are not prerequisites for new work.

## Performance Metrics

| Plan | Duration | Tasks | Files |
|------|----------|-------|-------|
| Phase 17 P01 | 54min | 3 tasks | 15 files |
| Phase 17 P02 | 67min | 3 tasks | 12 files |
| Phase 17 P03 | 42min | 2 tasks | 15 files |
| Phase 17 P04 | 7h22m | 2 tasks | 38 files |
| Phase 18 P01 | 59min | 2 tasks | 12 files |
| Phase 18 P02 | 60min | 3 tasks | 13 files |
| Phase 18 P03 | 20min | 2 tasks | 13 files |
| Phase 18 P4 | 3h20m | 3 tasks | 20 files |

## Recorded Phase 17 decisions

These entries preserve their execution-stage context. The new pin has since
been activated, all new measurements recorded, and CONF-01 through CONF-04
completed after independent verification.

- [Phase 17]: Keep 673ef550 canonical while admitting 6bee919 as a comparison fixture; do not activate the new pin in Plan 17-01.
- [Phase 17]: Accept nonstandard pass bytes only for exact source-defined decoded keys; classify $02 as a valid failure and DRAW sentinel rows as inventory-only.
- [Phase 17]: Retain old protected measurements and mark all corresponding new-pin observations unmeasured until the later campaign.
- [Phase 17]: The fixed five-Right navigation input is the measured campaign unit: six inputs change guest timing and the active page19 row, so cap-six exploration remains separately labeled.
- [Phase 17]: Keep both raw navigation-sensitive page19 observations; timeout only the source-confirmed active row, retain stale running markers as not-run, and do not fall back when all bytes were observed.
- [Phase 17]: Protected scoreboard and identity must come from the same immutable protected-main revision; legacy fallback reads the manifest from that revision.
- [Phase 17]: Every protected mapping requires fresh old/new raw evidence; only four exact source-reviewed keys can admit measured cross-pin terminal failure.
- [Phase 17]: Keep CONF-01 through CONF-04 pending until Phase 17 verification; canonical activation belongs to Plan 17-04.
- [Phase 17]: Authenticate the canonical protected-main scoreboard from its exact commit and project only measured PASS rows.
- [Phase 17]: Derive same-pin protected pages from the current protected baseline and require fresh PASS/raw evidence on the current invocation.
- [Phase 17]: Run the full campaign once on primary Linux x64 PR CI; keep measured page bounds and existing workflow timeouts.

## Decisions

- [Phase 18]: Keep loader and state-loader corpus identities separate; execution receives its own NEXE recipe identity.
- [Phase 18]: Use one finite matched Clang/ASan/UBSan/LLVM-coverage replay pass and report the measured 20-function delta without selecting final campaign budgets.
- [Phase 18]: Use test-side getrusage for peak RSS after macOS /usr/bin/time -l attempted a sandbox-denied sysctl query.
- [Phase 18]: Keep six compact in-memory mapper recipes and preserve unrestricted raw PRG/CHR mutation within each selected profile's capacity.
- [Phase 18]: Interleave a different-board instance between source and restored frames; this checks deterministic isolation, not simultaneous calls or thread safety.
- [Phase 18]: Keep ROB-01 and ROB-02 pending until remaining Phase 18 evidence and final phase verification.
- [Phase 18]: Use monotonically advancing frame and tick counters as evidence of loop progress; repeated PC or pixels alone do not imply nonprogress.
- [Phase 18]: Keep STOP_JAM native and latched; libretro repeats its owned complete XRGB frame or initialized black and sends no audio.
- [Phase 18]: Keep presentation cache outside native state/save states and keep ROB-02/ROB-04 pending for final Phase 18 verification.
- [Phase 18]: Use the existing Linux Clang workflow for mutation and budget selection because the local Apple Clang runtime lacks libFuzzer.
- [Phase 18]: Keep target input and work bounds separate from replay, fuzzer, sampled RSS, and outer process limits; finite clean runs remain finite evidence.
- [Phase 18]: Successful state restore invalidates frame validity while preserving XRGB storage already visible to the frontend.
- [Phase 18]: Use 15-minute perf and 50-minute randomized-suite workflow caps after exact-host runs demonstrated setup and repeat timing needs.
