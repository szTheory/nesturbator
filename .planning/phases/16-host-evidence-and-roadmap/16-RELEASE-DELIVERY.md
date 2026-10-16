# Phase 16 release delivery

**Recorded:** 2026-10-16

**Status:** EVID-04 remains open. Plan 21's repaired source has passed its current local acceptance, independent review and mandatory parent gates. Plan 07 Task 1 is resuming from that evidence. PR #50 still points at the older pushed head `3de23d4aed5d9696dc4020db3b8ef5e8205300fb`, whose latest native CI failed; the newly accepted source and current delivery preparation have not yet been pushed or checked natively. Plans 07/08 remain incomplete. No merge consent has been requested or granted.

## Accepted implementation and remaining gap

Plans 16-01 through 16-06 remain complete and their reviewed behavior is preserved. The sole Phase 16 acceptance gap is EVID-04: the actual published `v0.1.10` release at `da2bf973813530cb9561a004e7b774011a0a7867` has libretro metadata that omits MMC3. The inspected `nesturbator-0.1.10-libretro-macos-arm64.zip` has SHA-256 `9226ddce3e95c4bb0a2b13d910e34c225f67c0a02e24ce550b315e551235142d`. Current source metadata, documentation, staged install and package checks describe NTSC MMC3 mapper 4 submappers 0/4 and exclude MMC6 and four-screen; those checks do not establish what was published.

## Source, test and review evidence

[Required source CI run 38676328559](https://github.com/szTheory/nesturbator/actions/runs/38676328559) passed on `841434b46b202dd04ffbdeb0ac4c4bcbd0b07159`. It includes the six-platform build, sanitizer, no-float, hosted RetroArch, hygiene and `CI required` results. It was a `workflow_dispatch`, so its pull-request-only `perf` job was skipped. This is prior implementation evidence, not evidence for the candidate PR head or a release commit.

Plan 09 completed its bounded readiness repair at source commit `e2bed6aeb85b773e44597e8f5d27de94d2dc383f`. Its [timing record](16-TIMING-REPAIR.md) distinguishes the native failure from local measurements: the focused readiness test passed both delayed and stalled cases, the full session-control matrix passed, and `cmake --workflow --preset ci` passed all 649 tests before producing the library, runner and libretro archives. The run did not establish an Intel-specific cause for the prior native failure.

The current [deep source review](16-REVIEW.md) is clean at that exact repair source commit. It covers the six changed source and documentation paths listed in the report and found zero actionable issues; the [disposition record](16-REVIEW-DISPOSITION.md) preserves WR-01 through WR-06 as fixed and reports `open: 0`. No executable, source or workflow files changed after `e2bed6a`; subsequent commits only record the repair, review and plan state. The previous broader review and source CI remain historical evidence for the original implementation.

## Existing PR and prior native failure

PR [#50](https://github.com/szTheory/nesturbator/pull/50) is the existing open, non-draft pull request for `gsd/phase-16-host-evidence-and-roadmap` into `main`. Its title is `fix: make host evidence discriminating and describe supported MMC3`. It is the only open PR for that exact branch and base. The already-pushed preparation commit `30a009838b741a429863e7cb5143526fbaad8ed7` records the initial delivery boundary; safe resume retains it and reuses this PR instead of recreating either.

The prior native [`pull_request` CI run 38740783575](https://github.com/szTheory/nesturbator/actions/runs/38740783575) was on PR head `30a009838b741a429863e7cb5143526fbaad8ed7`, with base `9f9386c661f6fb8f6063bf756655713aedadae77` and preview merge SHA `0bc78b76c3412068e99c050bebe9b49276c7ce83`. It failed: the macOS Intel `readiness-response-stall` full prefix took 26,632 ms against the unchanged strict `<25000 ms` limit. Its exact response, operation interval and child/host cleanup assertions passed; the preceding delayed-response case passed at 20,728 ms. Other platform builds, `perf` and pinned RetroArch passed; `hash-equality` was consequently skipped and `CI required` failed. The source and native job log correct the inherited diagnostic JSON's mislabeled `readiness-response-delay` case. These measurements do not establish a native scheduling or platform root cause. The failed run was not rerun.

The bounded Plan 09 repair, local CI evidence and fresh source-bound review were pushed with preparation commit `3de23d4aed5d9696dc4020db3b8ef5e8205300fb` using normal hooks. PR #50 was updated through its structured body file. Complete PR, run, merge-ref and check identities are retained in ignored `build/phase16-release/` state. The resulting native failure is recorded below; local success did not establish native timing acceptance.

## PR body and owner consent boundary

The updated structured PR body is prepared at ignored `build/phase16-release/pr-body.md`. It states the implemented behavior, AI involvement, automated evidence, the sole outstanding publication criterion and the owner-gated release path. No phase-PR auto-merge is enabled.

Only after reversible preparation and fresh native CI pass will the owner be asked to approve the normal squash merge of the presented PR at its exact 40-character checked head. Consent must identify that PR and SHA and include the exact title, base `main`, clean internal review, and links to the successful required, performance and pinned RetroArch checks. Earlier push authorization, automated evidence, silence or a changed head is not merge consent. A changed head requires fresh checks and renewed consent. The approval also authorizes the existing release-please consequence; no package version is selected in advance.

## Existing release handoff

After an approved phase merge reaches protected `main`, the existing [`release.yml`](../../../.github/workflows/release.yml) flow runs release-please. Its release PR auto-merges after native required CI passes. The resulting release-commit run calls the full CI workflow on the exact tagged commit, and publication runs only after that CI succeeds. This plan does not change workflows or versions, create tags, dispatch the release workflow, upload assets or publish manually.

EVID-04 remains pending until the normal flow publishes the candidate and Plan 16-08 verifies the downloaded release metadata and its release identities. The previous `v0.1.10` publication, the prior source CI, and the failed pre-repair PR run cannot satisfy that criterion.

## Repaired candidate native CI failure — 2026-10-15

PR #50 remains open and non-draft at head `3de23d4aed5d9696dc4020db3b8ef5e8205300fb`, base `main` at `9f9386c661f6fb8f6063bf756655713aedadae77`, and preview merge `81c0a2132250892b324ec4c9be2a26fe8ea91056`. Its title remains `fix: make host evidence discriminating and describe supported MMC3`.

The authorized push and body update triggered native pull-request runs 38764179312 and [38764197994](https://github.com/szTheory/nesturbator/actions/runs/38764197994) for this same head. GitHub automatically superseded the first run; the later run completed with failure. Neither event was a manual rerun.

| Failed job | Focused control | Complete driver plus cleanup | Required bound | Test result |
|---|---|---|---|---|
| [macOS Intel](https://github.com/szTheory/nesturbator/actions/runs/38764197994/job/116523895865) | readiness-response-stall | 25,946 ms | strictly less than 25,000 ms | 648/649 passed |
| [ASan](https://github.com/szTheory/nesturbator/actions/runs/38764197994/job/116523896064) | readiness-delayed | 26,269 ms | strictly less than 25,000 ms | 643/644 passed |

Both failures were in `retroarch.session.readiness`. The ASan full `retroarch.session.controls` matrix passed in 548.70 seconds. [Performance](https://github.com/szTheory/nesturbator/actions/runs/38764197994/job/116523895800), [pinned RetroArch](https://github.com/szTheory/nesturbator/actions/runs/38764197994/job/116523896168), all other platform builds, hygiene/title and both no-float legs passed. [CI required](https://github.com/szTheory/nesturbator/actions/runs/38764197994/job/116534283543) failed and hash equality was skipped. The native platform or source cause remains unproven; the local process reduction did not satisfy every native timing control.

Ignored `build/phase16-release/` retains `failure-summary-38764197994.json`, `poll-state.json`, `run-jobs-38764197994.tsv`, `macos-intel-repaired-failed.log`, `asan-job.log` and `ci-required-failed.log`. No bound, sleep, parallelism, source or workflow was changed after this failure; CI was not rerun. No owner consent, merge, tag or publication occurred. The unrelated config preference edit remains unstaged and byte-identical to its execution baseline.

## Current accepted source and Plan 07 resume — 2026-10-16

Plan 21 is complete on source candidate `38e14fbc0104038a7eb9ad17d16499c7924c2b9b`. Its accepted actual-helper RED and eight-case GREEN are bound to the same frozen harness; the repair handles a failed cleanup QUIT write while preserving the first failure/status and continuing owned-host teardown and endpoint-lock release. The normal `cmake --workflow --preset ci` passed 649/649 tests with no skips, four jobs and all four workflow stages in 293.375720 seconds, and its three new local library, runner and libretro ZIPs passed the existing package checks. A separate parent build and full-workflow repeat also passed; that repeat passed 649/649 with no skips and four jobs in 298.625100 seconds.

The fresh independent source review is clean with zero findings on the source-equivalent snapshot `b071e63ee0e11c8b944e7f0d8b426867f227dda3`. Its six reviewed blobs match the accepted candidate, and the digest-bound disposition reports `open: 0` with report SHA-256 `2eb2369ab458597ee7aea6875121a27145463faa528861c61e44ca0e270c935d`. The parent-gate result binds the same ten source blobs and protected config hash and records `PASS` at snapshot `11fc2bf8cec6ba864240e76d3bd7be85378ef760`.

At resume, local HEAD was `e8ade4cd77d67664c34107c06975e9fb60d2d70a`. All ten paths in Plan 21's candidate register still match their accepted Git blobs; the difference from the source candidate is planning and state documentation only. Plans 16-01 through 16-06, 16-09, 16-15 and 16-21 are complete; only Plans 07 and 08 remain incomplete, so the live count stays 9/11. The protected config remains byte-identical at SHA-256 `0517388f5d422ae97afdc5dc1ae11bc43ebd481a757424f152ee8af61559ff5e`; its user preference diff is unstaged and excluded. The phase verifier remains `gaps_found`, so the installed `$gsd-ship` preflight would block; this plan follows its expressly authorized delivery path inside execute-phase and does not invoke that workflow.

## Current PR and next native check

The live PR is still [#50](https://github.com/szTheory/nesturbator/pull/50), open and non-draft, titled `fix: make host evidence discriminating and describe supported MMC3`, from `gsd/phase-16-host-evidence-and-roadmap` to `main`. Its remote head is still `3de23d4aed5d9696dc4020db3b8ef5e8205300fb`; its base is `9f9386c661f6fb8f6063bf756655713aedadae77`. No PR auto-merge is configured.

That older head's latest native run [38764197994](https://github.com/szTheory/nesturbator/actions/runs/38764197994) remains failed. The Intel `readiness-response-stall` case took 25,946 ms and ASan's `readiness-delayed` case took 26,269 ms against the strict `<25000 ms` limit; `CI required` failed, while `perf` and pinned RetroArch passed. Neither the source nor platform cause is established. Run 38764179312 was automatically superseded after the previous body edit and was not manually rerun. These old results do not cover the newly accepted Plan 21 source.

The remaining Task 1 preparation is to commit the current delivery record and state pointer, update the structured PR body before pushing, push the existing branch with normal hooks and no force, and observe fresh native `pull_request` checks on the exact final head and merge ref. The final check set must include `CI required`, `perf` and pinned RetroArch. Only a fresh successful set plus the clean current source review permits the exact-head owner-consent checkpoint. No consent, merge, tag, release-please action or publication has occurred. Plan 08 remains responsible for verifying the actual published assets; EVID-04 stays open until that work and independent re-verification pass.
