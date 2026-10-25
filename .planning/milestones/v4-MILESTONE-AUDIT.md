---
milestone: v4
audited: 2026-10-25T02:40:00Z
status: tech_debt
scores:
  requirements: 14/14
  phases: 3/3
  integration: 5/5
  flows: 5/5
gaps:
  requirements: []
  integration: []
  flows: []
tech_debt:
  - phase: delivery
    items:
      - "Phases 17 and 18 are not on main or released: PR #54 (Phase 17) and PR #55 (Phase 18, stacked on #54) are both draft and unmerged; main is at release 0.1.11 (#52)"
      - "PR #55 title is plan-level (`test(18-04): ...`); it needs a phase-level Conventional Commit title before squash merge"
      - "Exact-head Nightly 40150888217 still in progress at audit (campaign jobs and vectors-full passed; suite-flake running)"
      - "Phase 18 completion commits 0bc6442, 0246a90, 714d44c and this audit are not pushed"
  - phase: 16-host-evidence-and-roadmap
    items:
      - "Separate Nightly 39993108154: vectors-full failed; not a Release gate; preserved debt"
      - "16-05-SUMMARY.md lists two command strings as file paths (metadata only)"
  - phase: 17-complete-pinned-conformance-baseline
    items:
      - "Measured baseline is 101/146 scored rows passing (42 fail, 2 not-run, 1 timeout); this is the honest bound, not a gap, and feeds the RM-004 vs RM-005 choice"
      - "accuracycoin.campaign (full 22-page run) runs only on the primary Linux x64 CI lane by design"
      - "Three historical metadata references recorded at completion (removed staging ROM, shorthand doc paths, absent optional ci_monitor script)"
  - phase: 18-execution-and-lifecycle-robustness
    items:
      - "Real-campaign minimization is proven only by controls; no real campaign finding has been reduced"
      - "Libretro JAM sends no audio by policy; a frontend may underrun (documented, not a defect)"
  - phase: integration
    items:
      - "AccuracyCoin observe records a JAM only as a generic boot/execution frame failure (runner/accuracycoin.c:1054-1197); triage cannot tell JAM from other frame failures, and no AccuracyCoin control covers JAM (ROB-04, CONF-03)"
      - "No explicit run-ahead-under-JAM test found in libretro.runahead (ROB-04)"
      - "PRODUCT-ROADMAP.md:21 says 'No implementation is complete yet' and RM-001..003 still read committed; reconcile at complete-milestone under Rule 8 / DIR-02"
      - "ROADMAP.md:8 milestone line still says 'Phase 16 complete'; complete-milestone rewrites it"
      - "SUMMARY requirements-completed keys are empty for Phases 17 and 18 by policy (phase verifier owns requirement status); coverage confirmed from VERIFICATION evidence instead"
---

# Milestone v4 audit: Measured NTSC accuracy and robustness

**Verdict: tech_debt.** All 14 requirements are satisfied and verified, all three phases passed independent verification, and every cross-phase connection traced is wired. Nothing blocks completion. What remains is delivery (two unmerged draft PRs), one in-progress nightly and a short list of non-blocking debt.

Definition of done (ROADMAP v4): trustworthy current-host evidence, a complete pinned NTSC conformance inventory, and bounded execution robustness evidence, without promising an all-pass score. Met.

## Phases

| Phase | Verification | Score | Review findings | Human items |
|---|---|---|---|---|
| 16 Host evidence and roadmap | passed (re-verified 2026-10-23) | 22/22 | all fixed, 0 open | none |
| 17 Complete pinned conformance baseline | passed 2026-10-24 | 38/38 | all fixed, 0 open | none |
| 18 Execution and lifecycle robustness | passed 2026-10-25 | 4/4 | WR-01..05 fixed, 0 open | none |

## Requirements (three-source cross-reference)

| ID | Phase | VERIFICATION | SUMMARY requirements-completed | REQUIREMENTS.md | Final |
|---|---|---|---|---|---|
| EVID-01 | 16 | SATISFIED | 16-01, 16-03 | [x] Complete | satisfied |
| EVID-02 | 16 | SATISFIED | 16-03, 16-09, 16-56 | [x] Complete | satisfied |
| EVID-03 | 16 | SATISFIED | 16-04 | [x] Complete | satisfied |
| EVID-04 | 16 | SATISFIED | none (16-08 "provides" release-byte evidence) | [x] Complete | satisfied (manual check) |
| DIR-01 | 16 | SATISFIED | 16-06 | [x] Complete | satisfied |
| DIR-02 | 16 | SATISFIED | 16-06 | [x] Complete | satisfied |
| CONF-01 | 17 | SATISFIED | none (by policy) | [x] Complete | satisfied (manual check) |
| CONF-02 | 17 | SATISFIED | none (by policy) | [x] Complete | satisfied (manual check) |
| CONF-03 | 17 | SATISFIED | none (by policy) | [x] Complete | satisfied (manual check) |
| CONF-04 | 17 | SATISFIED | none (by policy) | [x] Complete | satisfied (manual check) |
| ROB-01 | 18 | SATISFIED | none (by policy) | [x] Complete | satisfied (manual check) |
| ROB-02 | 18 | SATISFIED | none (by policy) | [x] Complete | satisfied (manual check) |
| ROB-03 | 18 | SATISFIED | none (by policy) | [x] Complete | satisfied (manual check) |
| ROB-04 | 18 | SATISFIED | none (by policy) | [x] Complete | satisfied (manual check) |

The matrix rates a passed verification with no SUMMARY claim as "partial, verify manually". For Phases 17 and 18 the plan summaries deliberately leave `requirements-completed` empty and defer requirement status to the phase verifier (17-04-SUMMARY.md:179, 18-04-SUMMARY.md:181). The manual check is each VERIFICATION's requirement table, which cites command evidence for every ID: the 146-row campaign report and migration dispositions for CONF, and the six-family seeds, lifecycle controls, nightly campaigns and JAM regressions for ROB. EVID-04 is backed by 16-RELEASE-EVIDENCE.md (18 archives, checksums, attestations, six metadata copies of v0.1.11). No orphaned requirements.

## Cross-phase integration

The integration checker ran read-only against the Phase 18 branch, which contains Phase 17.

| Check | Status | Notes |
|---|---|---|
| Phase 18 JAM policy vs Phase 16 host checks and Phase 17 runner | wired | Host frame_index treats non-OK as failure; runner exits 1 on JAM; AccuracyCoin treats non-OK as failure. JAM cause is generic in AccuracyCoin (debt). |
| Phase 17/18 tests in `ci` preset and nightly targets | wired | 669 tests listed; no duplicate names; nightly `execution-fuzz` targets exist; report job depends on it. |
| README and public header agree with code (Rule 6) | wired | Runner usage and JAM presentation policy agree across header, README and adapter. |
| Hygiene and manifest | wired | AccuracyCoin ROMs pinned in manifest. The checker suspected the `corpus_execution` descriptors would be flagged as binary; not reproduced: `bash scripts/hygiene.sh` exits 0 locally (C locale, no NUL bytes). |
| Product roadmap links RM-001..003 to requirement IDs | wired | Links correct; status text stale (debt, reconciled at closeout). |

## End-to-end flows

| Flow | Status |
|---|---|
| Runner AccuracyCoin observe: CLI → run → classify → evidence → CI upload | complete |
| Libretro JAM: retro_run → presentation → invalidate → host test → docs | complete |
| Execution fuzz: target → ctest ci → nightly driver → artifacts → report | complete |
| Host evidence: .info MMC3 claim → runner rejects MMC6 and four-screen | complete |
| Product roadmap → project instructions and preparation handoff | complete |

## Nyquist

Skipped: no active validate-phase hook at verify:post.

## Before complete-milestone

1. Wait for Nightly 40150888217 to finish.
2. Push the branch, retitle PR #55 at phase level, then deliver #54 and #55 through the normal PR workflow so v4's code reaches main and a release.
3. In complete-milestone, refresh PRODUCT-ROADMAP.md (Rule 8): mark RM-001..003 shipped with evidence, carry the debt above, and choose between RM-004 (named NTSC repair, informed by the 42 failing rows) and RM-005 (PAL).
