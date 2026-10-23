# Phase 16 release delivery

## Current handoff — 2026-10-23 (Plan 16-07, Wave 11)

**Status:** Plan 16-07 Task 1 is preparing a comment-only correction for a new
native PR run. EVID-04 remains open. No merge consent, merge, tag or publication
has occurred.

Plan 16-56 accepted source candidate
`2c356d574290aef1d57122566f7b81beaaa10027`, with source commits `2e9928b` and
`2c356d5` and metadata commit `52f6915`. Its [summary](16-56-SUMMARY.md),
[source acceptance](16-56-SOURCE-ACCEPTANCE.md), [independent review](16-56-REVIEW.md)
and [disposition](16-56-REVIEW-DISPOSITION.md) record final-source CR-01
RED/GREEN evidence, focused CI and ASan, 649/649 full CI tests with no skips,
fresh package checks and the configured parent gates. Before this delivery
correction, the final parent manifest matched all 11 source/support paths in
both the accepted commit and worktree. The sole current source delta is one
public-header comment line; the other ten manifest paths remain byte-identical.
`.planning/config.json` remains byte-identical at SHA-256
`0517388f5d422ae97afdc5dc1ae11bc43ebd481a757424f152ee8af61559ff5e` and
unstaged. These are local source acceptance results; they do not establish
native PR CI or released bytes.

The live [PR #50](https://github.com/szTheory/nesturbator/pull/50) is open,
non-draft and unmerged, titled `fix: make host evidence discriminating and
describe supported MMC3`, from `gsd/phase-16-host-evidence-and-roadmap` to
`main`. Its current remote head is
`450ab3b6b30bbd2be2ab6e69524b32d15275553a`, with preview merge
`d8bc658ea8bbd9b2153d1c161ce776f00e80400f`. Its body now describes the accepted
Plan 16-56 candidate and pending publication. The active `main` ruleset requires
the `CI required` check and permits squash merges. The PR has no auto-merge
request or submitted reviews.

The latest earlier native PR result on #50 is historical run
[38976400182](https://github.com/szTheory/nesturbator/actions/runs/38976400182)
on the old remote head. `CI required` failed after hygiene formatting, Linux
owner-sidecar assertions and ASan/readiness failures; the Intel leg separately
reported a cleanup-matrix budget failure and readiness timeout. The `perf` and
pinned RetroArch jobs passed. Nightly run 38976400185 failed separately. These
failures remain data and are not acceptance for the current source. The
initially pushed candidate contained the accepted 56 source commits plus
documentation only; all 11 tested source/support blobs matched exactly.

The first refreshed native `pull_request` run for head
`450ab3b6b30bbd2be2ab6e69524b32d15275553a` is
[39885096932](https://github.com/szTheory/nesturbator/actions/runs/39885096932).
Its two Linux `nofp` legs failed in `tests/cmake/float_scan.cmake:81` because
the public-header comment's IPv4 address `127.0.0.1:55355` was scanned as a
floating-point token. The exact run is retained as historical evidence; its
failure is in the comment, not emulator behavior. A local one-line comment
correction replaces the address literal with “loopback TCP port 55355”; the
focused local `abi.float_scan` and `abi.float_scan.selftest` passed. Its local
`cmake --workflow --preset ci` run completed all 649 tests with no skips in
332.14 seconds and generated the library, runner and libretro ZIPs. A separate
local macOS no-float workflow reported `abi.undefined_symbols` requiring
`stack_chk_fail;stack_chk_guard`; that Apple-toolchain result is not Linux
acceptance.

Run 39885096932 completed with failure. Both Linux `nofp` legs failed on the
same comment scan. Windows x64/ARM, Linux x64/ARM, hygiene, title and `perf`
passed. macOS ARM, macOS Intel, ASan and the hosted RetroArch job failed
`retroarch.session.controls` with `cleanup matrix exceeded measured wall
budget`; the ASan job also failed `retroarch.session.readiness`, which reported
`readiness-delayed` driver+cleanup at 26,065 ms against its strict 25,000 ms
bound. The hosted RetroArch artifact errors followed the failed test suite and
missing generated evidence. `CI required` failed, and hash equality was
skipped. The exact native logs/excerpts and job identities are retained under
ignored `build/phase16-release/`; none of these timing criteria were changed.

The header correction changes only the explanatory comment; no executable
code or test expectation changed. The accepted Plan 16-56 independent review
remains applicable to executable source, and a bounded review of this exact
one-line comment delta found no behavior change. The 450ab3b results cannot
satisfy native acceptance. After this record and comment are committed, push
normally for a fresh `pull_request` run on the corrected full head.

Before recording the final pushed head, update the PR body and commit the
delivery preparation with normal hooks, leaving the config preference unstaged.
Push the existing phase branch normally and require fresh native `pull_request`
checks on its actual head and merge ref: `CI required`, `perf` and pinned
RetroArch must all pass. Preserve every old failure. The current run, check and
merge-ref identities are kept in ignored `build/phase16-release/context.json`
so later polling cannot change the checked head. Only then present the exact
PR/head and release consequence for the blocking owner-consent checkpoint.

The existing [release workflow](../../../.github/workflows/release.yml) remains
the publication path after consent: protected `main` triggers release-please,
its PR auto-merges only after native required CI, and the resulting exact-tag
CI must pass before publication. The published `v0.1.10` still omits MMC3;
EVID-04 stays open until Plan 16-08 verifies the actual downloaded release
bytes. Wave 11 ends at the consent checkpoint. Plan 16-08 and phase-level
verification remain separate later work; do not alter the verification status
or trigger another planning step.

---

## Historical delivery record — 2026-10-16

**Recorded:** 2026-10-16

**Status:** EVID-04 remains open. Plan 07 Task 1 pushed the accepted source and preparation at `65c3a7fb0b320bfe7a51ac486bf922c5ad5872d9`, but fresh native PR CI run 38976400182 failed. Plans 07/08 remain incomplete (9/11 live plans complete). Performance and pinned RetroArch passed. No merge consent was requested or granted; no merge or publication occurred.

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

## Historical accepted source and Plan 07 resume — 2026-10-16

Plan 21 is complete on source candidate `38e14fbc0104038a7eb9ad17d16499c7924c2b9b`. Its accepted actual-helper RED and eight-case GREEN are bound to the same frozen harness; the repair handles a failed cleanup QUIT write while preserving the first failure/status and continuing owned-host teardown and endpoint-lock release. The normal `cmake --workflow --preset ci` passed 649/649 tests with no skips, four jobs and all four workflow stages in 293.375720 seconds, and its three new local library, runner and libretro ZIPs passed the existing package checks. A separate parent build and full-workflow repeat also passed; that repeat passed 649/649 with no skips and four jobs in 298.625100 seconds.

The fresh independent source review is clean with zero findings on the source-equivalent snapshot `b071e63ee0e11c8b944e7f0d8b426867f227dda3`. Its six reviewed blobs match the accepted candidate, and the digest-bound disposition reports `open: 0` with report SHA-256 `2eb2369ab458597ee7aea6875121a27145463faa528861c61e44ca0e270c935d`. The parent-gate result binds the same ten source blobs and protected config hash and records `PASS` at snapshot `11fc2bf8cec6ba864240e76d3bd7be85378ef760`.

At resume, local HEAD was `e8ade4cd77d67664c34107c06975e9fb60d2d70a`. All ten paths in Plan 21's candidate register still match their accepted Git blobs; the difference from the source candidate is planning and state documentation only. Plans 16-01 through 16-06, 16-09, 16-15 and 16-21 are complete; only Plans 07 and 08 remain incomplete, so the live count stays 9/11. The protected config remains byte-identical at SHA-256 `0517388f5d422ae97afdc5dc1ae11bc43ebd481a757424f152ee8af61559ff5e`; its user preference diff is unstaged and excluded. The phase verifier remains `gaps_found`, so the installed `$gsd-ship` preflight would block; this plan follows its expressly authorized delivery path inside execute-phase and does not invoke that workflow.

## Historical pre-push preparation snapshot

At the pre-push snapshot, the live PR was [#50](https://github.com/szTheory/nesturbator/pull/50), open and non-draft, titled `fix: make host evidence discriminating and describe supported MMC3`, from `gsd/phase-16-host-evidence-and-roadmap` to `main`. Its remote head is still `3de23d4aed5d9696dc4020db3b8ef5e8205300fb`; its base is `9f9386c661f6fb8f6063bf756655713aedadae77`. No PR auto-merge is configured.

That older head's latest native run [38764197994](https://github.com/szTheory/nesturbator/actions/runs/38764197994) remains failed. The Intel `readiness-response-stall` case took 25,946 ms and ASan's `readiness-delayed` case took 26,269 ms against the strict `<25000 ms` limit; `CI required` failed, while `perf` and pinned RetroArch passed. Neither the source nor platform cause is established. Run 38764179312 was automatically superseded after the previous body edit and was not manually rerun. These old results do not cover the newly accepted Plan 21 source.

At that snapshot, the remaining Task 1 preparation was to commit the current delivery record and state pointer, update the structured PR body before pushing, push the existing branch with normal hooks and no force, and observe fresh native `pull_request` checks on the exact final head and merge ref. The final check set must include `CI required`, `perf` and pinned RetroArch. Only a fresh successful set plus the clean current source review permits the exact-head owner-consent checkpoint. No consent, merge, tag, release-please action or publication has occurred. Plan 08 remains responsible for verifying the actual published assets; EVID-04 stays open until that work and independent re-verification pass.


## Historical native PR failure — 2026-10-16

Preparation commit `65c3a7fb0b320bfe7a51ac486bf922c5ad5872d9` updated the delivery record and state pointer. The structured PR body was updated before one normal, non-force branch push; normal hooks passed. The actual open, non-draft PR remains [#50](https://github.com/szTheory/nesturbator/pull/50), titled `fix: make host evidence discriminating and describe supported MMC3`, targeting `main` at `9f9386c661f6fb8f6063bf756655713aedadae77`. Its checked head is that preparation commit, and the preview `refs/pull/50/merge` SHA is `aabd1d940d8deb2dd3c5369eddf074158517b92e`. Auto-merge is off. No source/config change followed local Plan 21 acceptance.

Fresh native [`pull_request` CI 38976400182](https://github.com/szTheory/nesturbator/actions/runs/38976400182) completed with failure at 2026-10-16T22:50:05Z. Its [CI required](https://github.com/szTheory/nesturbator/actions/runs/38976400182/job/117232139742) rollup failed; hash-equality was skipped. [Performance](https://github.com/szTheory/nesturbator/actions/runs/38976400182/job/117227153464), [pinned RetroArch](https://github.com/szTheory/nesturbator/actions/runs/38976400182/job/117227153003), macOS ARM, both Windows builds, both no-float legs and title passed. The pinned job passed its local suite, progression qualification, hosted frame, both battery-save paths and deterministic auto-state lanes. Those successes do not discharge the failed required check.

| Native job | Observed failure | Test outcome |
|---|---|---|
| [Hygiene](https://github.com/szTheory/nesturbator/actions/runs/38976400182/job/117227153307) | `hygiene.format`: clang-format violations in `tests/retroarch/frame_index.c` lines 1276–1283 | 12/13 passed |
| [Linux x64](https://github.com/szTheory/nesturbator/actions/runs/38976400182/job/117227153269) | `retroarch.session.controls`: `owner-sidecar collision: actual-driver-noclobber-refusal-not-proven` | 648/649 passed |
| [Linux ARM](https://github.com/szTheory/nesturbator/actions/runs/38976400182/job/117227153527) | Same owner-sidecar assertion | 648/649 passed |
| [ASan](https://github.com/szTheory/nesturbator/actions/runs/38976400182/job/117227153348) | Same owner-sidecar assertion; `retroarch.session.readiness` timeout at 60.03 seconds | 642/644 passed |
| [Intel macOS](https://github.com/szTheory/nesturbator/actions/runs/38976400182/job/117227153526) | `retroarch.session.controls`: `cleanup matrix exceeded measured wall budget`; readiness timeout at 60.26 seconds | 647/649 passed |

The Intel log contains no owner-sidecar refusal diagnostic. Earlier interim attribution of that assertion to Intel was incorrect; the raw log establishes the distinct matrix-budget failure above. The session-control/readiness causes remain unproven, and no bound or implementation was changed to reinterpret these results. The known cleanup-write correction and its accepted local evidence remain valid within their recorded scope; they do not establish native acceptance.

The PR body edit triggered run 38976329721 on old head `3de23d4`; it was automatically cancelled and is excluded. Ancillary [Nightly 38976400185](https://github.com/szTheory/nesturbator/actions/runs/38976400185) failed `suite-flake` on the current head; its other jobs passed. It is retained separately from the required CI result. No failed run was manually rerun.

Ignored `build/phase16-release/` retains the terminal run/jobs/PR API snapshots, bounded polling state (21/120 snapshots), `failure-summary-38976400182.json`, raw failed-job logs and their SHA-256 digests. Prior native/development evidence is preserved. The protected config stays byte-identical and unstaged. This terminal handoff commit is local documentation only, not a new native candidate; the checked remote head above remains immutable evidence.

Plan 07 Task 1 stops incomplete under its terminal-failure rule; there is no success SUMMARY or completion credit. Plan 08 cannot start. Historical next-command recommendation (superseded by the current handoff above): `$gsd-plan-phase 16 --gaps` for a bounded amendment covering formatting, the actual-driver owner-sidecar assertion, and measured readiness/cleanup timing. Preserve the existing timing/ownership contracts, establish causes before behavioural corrections, refresh affected acceptance/review gates after source changes, then obtain fresh native PR CI on the final exact head. Only successful native required/performance/pinned-host checks permit the exact-head owner-consent checkpoint. EVID-04 and the phase verifier remain open pending actual publication and released-byte verification.
