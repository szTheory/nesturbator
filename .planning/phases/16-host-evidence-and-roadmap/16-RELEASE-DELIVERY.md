# Phase 16 release delivery

**Recorded:** 2026-10-15
**Status:** EVID-04 remains open; PR #50 is prepared but native required CI failed. Plan 16-07 Task 1 remains incomplete; no merge consent was requested.

## Accepted implementation and remaining gap

Plans 16-01 through 16-06 are complete and their reviewed behavior is preserved. The sole Phase 16 acceptance gap is EVID-04: the actual published `v0.1.10` release at `da2bf973813530cb9561a004e7b774011a0a7867` has libretro metadata that omits MMC3. The inspected `nesturbator-0.1.10-libretro-macos-arm64.zip` has SHA-256 `9226ddce3e95c4bb0a2b13d910e34c225f67c0a02e24ce550b315e551235142d`. Current source metadata, documentation, staged install, and package checks describe NTSC MMC3 mapper 4 submappers 0/4 and exclude MMC6 and four-screen; those checks do not establish what was published.

The clean source review is recorded in [16-REVIEW.md](16-REVIEW.md): zero open findings across the 16 reviewed code, test, metadata, and workflow files. It was completed at `2026-10-15T03:27:54Z`, after the latest source change.

## Prior implementation evidence

[Required source CI run 38676328559](https://github.com/szTheory/nesturbator/actions/runs/38676328559) passed on source commit `841434b46b202dd04ffbdeb0ac4c4bcbd0b07159`. It includes the six-platform build, sanitizer, no-float, hosted RetroArch, hygiene, and `CI required` results. The run was a workflow dispatch, so its pull-request-only `perf` job was skipped. This is evidence for the reviewed implementation only; it is not evidence for the candidate PR head or an exact release commit.

## PR and consent boundary

The phase branch is prepared for one normal, unmerged pull request to `main`, titled `fix: make host evidence discriminating and describe supported MMC3`. Before any merge, the owner must explicitly approve that PR's presented full head SHA. Prior authorization to push and open a PR, successful automated evidence, silence, or a later changed head does not supply that consent. A changed head requires fresh successful required pull-request CI and renewed consent for its new SHA.

Approval authorizes only a squash merge of the identified PR at the identified checked head, under repository protections. No phase-PR auto-merge is enabled. The exact PR, head, run, and check identities are retained in ignored `build/phase16-release/` evidence so recording consent does not change the approved head.

## Native PR CI failure and recovery

[PR #50](https://github.com/szTheory/nesturbator/pull/50) is open and non-draft, titled `fix: make host evidence discriminating and describe supported MMC3`, against `main`. The normal push and PR creation succeeded; phase auto-merge is disabled. Its checked identities are:

| Identity | Observed value |
|---|---|
| PR head | `30a009838b741a429863e7cb5143526fbaad8ed7` |
| Base commit | `9f9386c661f6fb8f6063bf756655713aedadae77` |
| `refs/pull/50/merge` | `0bc78b76c3412068e99c050bebe9b49276c7ce83` |
| Native `pull_request` CI | [38740783575](https://github.com/szTheory/nesturbator/actions/runs/38740783575), concluded failure |

Fresh `gh pr view 50 --repo szTheory/nesturbator` and `gh run view 38740783575 --repo szTheory/nesturbator` queries exited 0 and confirmed the open PR, unchanged head, no merge, and terminal CI failure. Five platform builds, ASan, both no-float legs, hygiene, title, [perf](https://github.com/szTheory/nesturbator/actions/runs/38740783575/job/116441617301), and [pinned RetroArch](https://github.com/szTheory/nesturbator/actions/runs/38740783575/job/116441617155) passed. The macOS Intel build failed, `hash-equality` was consequently skipped, and `CI required` failed.

The [macOS Intel log](https://github.com/szTheory/nesturbator/actions/runs/38740783575/job/116441617204) reports `99% tests passed, 1 tests failed out of 648`; CMake exited 8. The sole failing CTest is `retroarch.session.controls`. Its `readiness-response-stall` control reached `tests/retroarch/test_state_session.sh:748`: the exact response, one-attempt journal, operation supervision time, and child/host cleanup assertions had passed, but the complete prefix-driver elapsed time was 26,632 ms against a strict less-than-25,000 ms assertion. The preceding delayed-reply control passed at 20,728 ms. These observations identify the failing outer elapsed-time assertion; they do not yet establish an adequate replacement bound or a platform-specific root cause.

Raw PR/run/job JSON, exact refs, `failure-diagnosis.json`, and `macos-intel-failed.log` remain in ignored `build/phase16-release/`. The inherited diagnostic JSON mislabeled the control as `readiness-response-delay`; the source assertion and log establish `readiness-response-stall` as the failing control. No failed workflow was rerun. No consent, merge, tag, or publication was initiated. Plan 16-08 remains unstarted, and the existing verification verdict/fingerprint and requirements are unchanged.

Plan 16-07 requires stopping on terminal failed native CI. The next work is a bounded amendment to investigate and repair the elapsed-time control while preserving its exact-response, operation-deadline, send-count, and cleanup assertions, with focused regression, the normal CI preset, review, and fresh native PR CI. Reuse PR #50 and inspect actual state before any mutation. Do not repeat the successful preparation commit or manufacture a completed 16-07 summary: the plan is incomplete, and the safe-resume gate must reconcile its existing commit. These failure/state documentation updates stay local on the phase branch; the failed remote PR head remains the identity above until an authorized repair is pushed.

## Existing release handoff

After an approved phase merge reaches protected `main`, the existing `release.yml` flow runs release-please. Its release PR enables automatic squash merge after native required CI passes. A release-commit run then calls the full CI workflow on the exact tagged commit and publishes only after that CI succeeds. This plan does not select a version, edit a tag, dispatch the release workflow, upload assets, or publish manually.

EVID-04 remains pending until the normal flow publishes the candidate and Plan 16-08 verifies the downloaded release metadata and its release identities. The previous `v0.1.10` publication and the prior source CI run cannot satisfy that criterion.
