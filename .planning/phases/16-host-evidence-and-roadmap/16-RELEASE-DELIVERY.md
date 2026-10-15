# Phase 16 release delivery

**Recorded:** 2026-10-15
**Status:** EVID-04 remains open; the implementation PR is being prepared.

## Accepted implementation and remaining gap

Plans 16-01 through 16-06 are complete and their reviewed behavior is preserved. The sole Phase 16 acceptance gap is EVID-04: the actual published `v0.1.10` release at `da2bf973813530cb9561a004e7b774011a0a7867` has libretro metadata that omits MMC3. The inspected `nesturbator-0.1.10-libretro-macos-arm64.zip` has SHA-256 `9226ddce3e95c4bb0a2b13d910e34c225f67c0a02e24ce550b315e551235142d`. Current source metadata, documentation, staged install, and package checks describe NTSC MMC3 mapper 4 submappers 0/4 and exclude MMC6 and four-screen; those checks do not establish what was published.

The clean source review is recorded in [16-REVIEW.md](16-REVIEW.md): zero open findings across the 16 reviewed code, test, metadata, and workflow files. It was completed at `2026-10-15T03:27:54Z`, after the latest source change.

## Prior implementation evidence

[Required source CI run 38676328559](https://github.com/szTheory/nesturbator/actions/runs/38676328559) passed on source commit `841434b46b202dd04ffbdeb0ac4c4bcbd0b07159`. It includes the six-platform build, sanitizer, no-float, hosted RetroArch, hygiene, and `CI required` results. The run was a workflow dispatch, so its pull-request-only `perf` job was skipped. This is evidence for the reviewed implementation only; it is not evidence for the candidate PR head or an exact release commit.

## PR and consent boundary

The phase branch is prepared for one normal, unmerged pull request to `main`, titled `fix: make host evidence discriminating and describe supported MMC3`. Before any merge, the owner must explicitly approve that PR's presented full head SHA. Prior authorization to push and open a PR, successful automated evidence, silence, or a later changed head does not supply that consent. A changed head requires fresh successful required pull-request CI and renewed consent for its new SHA.

Approval authorizes only a squash merge of the identified PR at the identified checked head, under repository protections. No phase-PR auto-merge is enabled. The exact PR, head, run, and check identities are retained in ignored `build/phase16-release/` evidence so recording consent does not change the approved head.

## Existing release handoff

After an approved phase merge reaches protected `main`, the existing `release.yml` flow runs release-please. Its release PR enables automatic squash merge after native required CI passes. A release-commit run then calls the full CI workflow on the exact tagged commit and publishes only after that CI succeeds. This plan does not select a version, edit a tag, dispatch the release workflow, upload assets, or publish manually.

EVID-04 remains pending until the normal flow publishes the candidate and Plan 16-08 verifies the downloaded release metadata and its release identities. The previous `v0.1.10` publication and the prior source CI run cannot satisfy that criterion.
