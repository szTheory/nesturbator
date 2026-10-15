# Phase 16 release delivery

**Recorded:** 2026-10-15

**Status:** EVID-04 remains open. Plan 16-07 Task 1 is resuming after the completed Plan 09 repair. PR #50 remains at its pre-repair head until the prepared phase branch is pushed. No merge consent has been requested or granted.

## Accepted implementation and remaining gap

Plans 16-01 through 16-06 remain complete and their reviewed behavior is preserved. The sole Phase 16 acceptance gap is EVID-04: the actual published `v0.1.10` release at `da2bf973813530cb9561a004e7b774011a0a7867` has libretro metadata that omits MMC3. The inspected `nesturbator-0.1.10-libretro-macos-arm64.zip` has SHA-256 `9226ddce3e95c4bb0a2b13d910e34c225f67c0a02e24ce550b315e551235142d`. Current source metadata, documentation, staged install and package checks describe NTSC MMC3 mapper 4 submappers 0/4 and exclude MMC6 and four-screen; those checks do not establish what was published.

## Source, test and review evidence

[Required source CI run 38676328559](https://github.com/szTheory/nesturbator/actions/runs/38676328559) passed on `841434b46b202dd04ffbdeb0ac4c4bcbd0b07159`. It includes the six-platform build, sanitizer, no-float, hosted RetroArch, hygiene and `CI required` results. It was a `workflow_dispatch`, so its pull-request-only `perf` job was skipped. This is prior implementation evidence, not evidence for the candidate PR head or a release commit.

Plan 09 completed its bounded readiness repair at source commit `e2bed6aeb85b773e44597e8f5d27de94d2dc383f`. Its [timing record](16-TIMING-REPAIR.md) distinguishes the native failure from local measurements: the focused readiness test passed both delayed and stalled cases, the full session-control matrix passed, and `cmake --workflow --preset ci` passed all 649 tests before producing the library, runner and libretro archives. The run did not establish an Intel-specific cause for the prior native failure.

The current [deep source review](16-REVIEW.md) is clean at that exact repair source commit. It covers the six changed source and documentation paths listed in the report and found zero actionable issues; the [disposition record](16-REVIEW-DISPOSITION.md) preserves WR-01 through WR-06 as fixed and reports `open: 0`. No executable, source or workflow files changed after `e2bed6a`; subsequent commits only record the repair, review and plan state. The previous broader review and source CI remain historical evidence for the original implementation.

## Existing PR and prior native failure

PR [#50](https://github.com/szTheory/nesturbator/pull/50) is the existing open, non-draft pull request for `gsd/phase-16-host-evidence-and-roadmap` into `main`. Its title is `fix: make host evidence discriminating and describe supported MMC3`. It is the only open PR for that exact branch and base. The already-pushed preparation commit `30a009838b741a429863e7cb5143526fbaad8ed7` records the initial delivery boundary; safe resume retains it and reuses this PR instead of recreating either.

The prior native [`pull_request` CI run 38740783575](https://github.com/szTheory/nesturbator/actions/runs/38740783575) was on PR head `30a009838b741a429863e7cb5143526fbaad8ed7`, with base `9f9386c661f6fb8f6063bf756655713aedadae77` and preview merge SHA `0bc78b76c3412068e99c050bebe9b49276c7ce83`. It failed: the macOS Intel `readiness-response-stall` full prefix took 26,632 ms against the unchanged strict `<25000 ms` limit. Its exact response, operation interval and child/host cleanup assertions passed; the preceding delayed-response case passed at 20,728 ms. Other platform builds, `perf` and pinned RetroArch passed; `hash-equality` was consequently skipped and `CI required` failed. The source and native job log correct the inherited diagnostic JSON's mislabeled `readiness-response-delay` case. These measurements do not establish a native scheduling or platform root cause. The failed run was not rerun.

The local phase branch now contains the bounded Plan 09 repair, its focused and full normal CI evidence, and the fresh source-bound review. After all intended local preparation commits, Plan 16-07 pushes that existing branch with normal hooks and captures a new full head SHA. The new native PR run must be for that final head and include successful `CI required`, `perf` and pinned RetroArch checks. Its complete PR, run, merge-ref and check identities are retained in ignored `build/phase16-release/` state so recording the result does not change the checked candidate.

## PR body and owner consent boundary

The updated structured PR body is prepared at ignored `build/phase16-release/pr-body.md`. It states the implemented behavior, AI involvement, automated evidence, the sole outstanding publication criterion and the owner-gated release path. No phase-PR auto-merge is enabled.

Only after reversible preparation and fresh native CI pass will the owner be asked to approve the normal squash merge of the presented PR at its exact 40-character checked head. Consent must identify that PR and SHA and include the exact title, base `main`, clean internal review, and links to the successful required, performance and pinned RetroArch checks. Earlier push authorization, automated evidence, silence or a changed head is not merge consent. A changed head requires fresh checks and renewed consent. The approval also authorizes the existing release-please consequence; no package version is selected in advance.

## Existing release handoff

After an approved phase merge reaches protected `main`, the existing [`release.yml`](../../../.github/workflows/release.yml) flow runs release-please. Its release PR auto-merges after native required CI passes. The resulting release-commit run calls the full CI workflow on the exact tagged commit, and publication runs only after that CI succeeds. This plan does not change workflows or versions, create tags, dispatch the release workflow, upload assets or publish manually.

EVID-04 remains pending until the normal flow publishes the candidate and Plan 16-08 verifies the downloaded release metadata and its release identities. The previous `v0.1.10` publication, the prior source CI, and the failed pre-repair PR run cannot satisfy that criterion.
