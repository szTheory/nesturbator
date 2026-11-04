# Milestones

## v8 PAL region support (Shipped: 2026-11-03)

**Phases completed:** 4 phases (25–27.1), 12 plans, 31 plan tasks.
**Timeline:** 2026-11-01 → 2026-11-03 (three calendar dates).
**Change range:** `5997cae^..34ca8d0`, 116 commits, 116 files, 13,733 insertions and 332 deletions.
**Code:** 56,438 tracked C lines, excluding vendored `libretro.h`; 722 registered CTest tests in `ci`.
**Requirements:** 7/7 satisfied; all four phase verifications passed (57/57 truths); audit `tech_debt`, with 7/7 integration paths and 5/5 end-to-end flows complete.

The archive CLI counted 15 tasks from summary metadata; the twelve plan
contracts contain 31 task blocks. This entry uses the plan-contract total and
records the parser discrepancy without changing phase summaries.

**Key accomplishments:**
- Added shared region selection and exact PAL timing/audio accounting across all six supported mapper families while preserving NTSC output.
- Implemented the bounded, source-backed 2A07 DMC model with a wrong-model control, plus region-specific state identity and atomic cross-region refusal.
- Exposed matching loaded-region AV behavior through the library, runner and libretro adapter, with a named modeled 2C07 palette and explicit color limits.
- Added Phase 27.1 supplemental validation for PAL five-step endpoints and regional frame counters while keeping Phase 26 as STATE-01's sole owner.
- Passed the accepted 722-test local workflow and packaged the library, runner and libretro adapter.

**Closeout:** The artifact audit is clear after reconciling the halted traceability quick task against the later Phase 26 verification refresh and v8 audit. No new items were acknowledged; 16 previously acknowledged items remain carried forward. `Known verification overrides: 0 newly acknowledged, 16 carried forward from a prior close (see STATE.md Deferred Items)`. Closeout type: verified.

The audit retains WR-01 baseline metadata that points to an earlier source revision/capture path, while current-candidate campaign evidence was separately verified. Direct 2A07 proof, later held no-op-cycle behavior, PAL reset-edge duration, 2C07 calibration and physical host pacing remain unverified. The older interrupted full-CI/AccuracyCoin campaign remains stopped with unknown outcome.

**Delivery:** v8 is a planning milestone only; no v8 package release, tag, push or hosted action is claimed. Milestone tags are disabled by configuration. The latest published package is v0.1.12, published through merged release PR #56 on 2026-10-31. The interrupted campaign was not restarted.

**Direction:** RM-005 shipped. No next milestone is committed. RM-015 remains the preferred conditional proposal only after bounded native/source triage confirms a core fault; scoped maintenance is the smaller alternative. RM-008 remains consumer-owned. PAL infrastructure is now proven, but Dendy RM-006 still needs Dendy-specific timing evidence and demand. Mid and Long triggers are unchanged. SEED-003 remains partial; SEED-006 is recurring. Seed files were not modified. Five unowned quick-task folders remain in `.planning/quick/` because their milestone ownership is not established.

**Archive:** [product roadmap](milestones/v8-PRODUCT-ROADMAP.md),
[execution roadmap](milestones/v8-ROADMAP.md),
[requirements](milestones/v8-REQUIREMENTS.md), [audit](milestones/v8-MILESTONE-AUDIT.md),
[closeout](milestones/v8-CLOSEOUT.md) and [phase artifacts](milestones/v8-phases/).

---

## v7 Remaining DMA accuracy failures (Shipped: 2026-11-01)

**Phases completed:** 1 phase (24), 5 plans, 15 tasks.
**Timeline:** 2026-10-31 → 2026-11-01 (two calendar dates).
**Change range:** `b2a46ff^..1b5a6bd`, 46 commits, 86 files, 11,327 insertions and 338 deletions.
**Code:** 56,941 lines of owned C, excluding vendored `libretro.h`; 707 registered CTest tests in `ci`.
**Requirements:** 4/4 satisfied; Phase 24 passed 9/9, with 5/5 traced flows and no blocking gaps; audit `tech_debt`.

**Key accomplishments:**
- Corrected the five selected NTSC DMA failures with separate source-backed traces, owned wrong-model fixtures and the original pin; the three passing DMA rows and all protected PASS keys remain guarded.
- Preserved production-reachable native continuation with state format 6, behavior revision 14 and ABI 1, including refusal and compatibility controls.
- Passed the local 707-test workflow and packaged the library, runner and libretro adapter; the five selected campaign rows passed.
- Validated candidate-bound hosted CI/Nightly, six-platform frame/audio/state equality, and the unchanged +1% Cachegrind gate (raw results +0.18% to +0.25%).
- Repaired retained DMA trace completeness and kept hardware claims, pinned expectations, observed core behavior and evidence provenance separate.

The archive CLI reported 3 tasks and no accomplishments. Each of the five plans
declares 3 tasks, so this entry reconciles the task total to 15 and extracts the
accomplishments from the plan summaries and final verification.

**Closeout:** The open-artifact audit found no new items; 16 previously
acknowledged items remain carried forward. The v7 audit records two
non-blocking items: stale README acceptance wording and inherited branch-wide
whitespace diagnostics. The older interrupted full-CI/AccuracyCoin outcome
remains unknown and its restart stop remains active. See the
[v7 audit](milestones/v7-MILESTONE-AUDIT.md).

**Delivery:** v7 is a planning milestone; no v7 package release or tag was
created. The latest published package remains v0.1.12. No push, merge or release
occurred during closeout; the historical campaign stop remains unchanged.

**Direction:** RM-016 shipped. No next milestone is committed. PAL timing
(RM-005) is the preferred proposal; RM-015 remains conditional on a confirmed
core fault, and scoped maintenance is the smaller alternative. Mid and Long
triggers remain unchanged. SEED-003 remains partial, SEED-006 remains recurring,
and the seed files were not modified. Four unowned quick-task folders remain in
`.planning/quick/` rather than being attributed to v7.

**Archive:** [product roadmap](milestones/v7-PRODUCT-ROADMAP.md),
[execution roadmap](milestones/v7-ROADMAP.md),
[requirements](milestones/v7-REQUIREMENTS.md), [audit](milestones/v7-MILESTONE-AUDIT.md),
and [phase artifacts](milestones/v7-phases/).

---

## v6 Player-ready NES and reproducible feedback (Shipped: 2026-10-31; planning milestone)

**Phases completed:** 2 phases (22–23), 7 plans, 15 tasks.
**Timeline:** 2026-10-30 → 2026-10-31.
**Change range:** `5519102..f5f9e27`, 43 commits, 55 files, 10,446 insertions and 2,651 deletions.
**Requirements:** 10/10 satisfied; Phase 22 passed 7/7, Phase 23 passed 9/9; audit `tech_debt`, with 3/3 end-to-end flows and no blockers.

The archive CLI reported 8 tasks. The seven plan summaries record 2, 2, 2, 3,
2, 2 and 2 tasks (15 total); this entry uses the reconciled summary count and
leaves the source summaries unchanged.

**Key accomplishments:**
- Reorganized the README around the player path and added canonical API, runner, release, testing and progress guides with contract checks.
- Documented loaded-build identity, backup/rollback policy and distinctions among battery saves, native snapshots and host containers.
- Added a sanitized scene report and bounded two-run cold replay comparing complete frame and audio records.
- Added native checkpoint preflight for runner/ROM/state identities and kept neutral continuation separate from movie replay.
- Added evidence guidance that separates repeatability from adapter, host and hardware correctness claims.

**Closeout:** The open-artifact audit was clear; no new records were acknowledged,
and 16 earlier acknowledged items remain carried forward. The owner approved
carrying the v6 audit's documented non-blocking debt; no phase or requirement
was waived. See the [closeout record](milestones/v6-CLOSEOUT.md) and
[milestone audit](milestones/v6-MILESTONE-AUDIT.md).

**Delivery:** No v6 package release is claimed. Release PR #56 (0.1.12) remains
open with failed and cancelled checks; v0.1.11 remains the latest published
package. Automatic publication is authorized only after all required release
checks are green, so PR #56 was not published. `git.create_tag` is disabled in
project configuration; no v6 tag was created.

**Direction:** RM-014 shipped. No next milestone is committed. The default next
candidate is the five remaining DMA failures; RM-015 may move ahead only if
bounded triage confirms a core fault. PAL is the regional alternative and
scoped maintenance the smaller option. Mid and Long triggers remain unchanged.
The two pre-v6 quick tasks remain in `.planning/quick/` because they have no
milestone provenance.

**Archive:** [product roadmap](milestones/v6-PRODUCT-ROADMAP.md),
[execution roadmap](milestones/v6-ROADMAP.md),
[requirements](milestones/v6-REQUIREMENTS.md),
[phase artifacts](milestones/v6-phases/).

---

## v5 NMI, VBlank and sprite-0 timing (Shipped: 2026-10-30)

**Delivered:** Games and hosts get tested sprite-0, VBlank/NMI and interrupt poll/takeover behavior across the library, runner and libretro adapter, with deterministic state continuation and explicit JAM presentation.

**Phases completed:** 3 phases (19–21), 27 plans, 68 tasks.
**Timeline:** 2026-10-25 → 2026-10-30 (six calendar dates).
**Requirements:** 11/11 satisfied; three current passed verifications, eight integration groups and eleven end-to-end flows ([audit](milestones/v5-MILESTONE-AUDIT.md), `tech_debt`, no blocking gaps).

**Key accomplishments:**
- AccuracyCoin names guest JAM in every stage; actual adapter run-ahead preserves the frozen picture and silence without reusing a diverged timeline.
- Sprite evaluation follows OAMADDR and its rendering reset; both sprite-0 target rows pass with owned dot-level evidence.
- Split PPUSTATUS sampling and separate NMI line/capture/poll histories satisfy all six NMI/VBlank target rows.
- Legal instruction polls retain IRQ eligibility; shared resumable BRK/IRQ/NMI entry selects a coherent vector, including held/released inhibited NMI candidates.
- State format 3 / behavior revision 9 / ABI 1 supports validated native and actual-adapter replay at every entry stage, preserving causal six-platform frame/audio/state hashes.
- All eleven selected targets and all 114 Phase 21 starting PASS keys pass on six lanes; final baseline is 119 PASS / 27 FAIL, 146 terminal scored rows plus five unscored DRAW entries. Original conformance/host/performance gates pass in the accepted source-bound bundle.

**Closeout:** override closeout for explicitly acknowledged open records, not missing phase verification or waived product requirements.

Known verification overrides: 6 newly acknowledged, 10 carried forward from a prior close (see STATE.md Deferred Items).

**Delivery:** PRs #57, #59 and #61 merged. [Release PR #56](https://github.com/szTheory/nesturbator/pull/56) (0.1.12) remains open at closeout; no new package publication is claimed. The planning archive is committed locally on a closeout branch. Milestone tags are disabled by configuration.

**Debt and evidence:** [closeout record](milestones/v5-CLOSEOUT.md), [integration audit](milestones/v5-INTEGRATION-AUDIT.md), [product direction snapshot](milestones/v5-PRODUCT-ROADMAP.md), [roadmap](milestones/v5-ROADMAP.md), [requirements](milestones/v5-REQUIREMENTS.md). Source-bound hosted acceptance remains historical; local checks and final-head workflow success do not replace collected bundles. The ledger representation is reconciled; other audit debt and physical-tie/provenance limits remain.

**What's next:** select fresh scope with `$gsd-new-milestone`. Preferred remaining DMA, PAL as main alternative, or smaller maintenance-only cleanup; none approved yet.

---

## v4 Measured NTSC accuracy and robustness (Shipped: 2026-10-24)

**Delivered:** Host builders can trust the RetroArch progression and state checks, read a complete, honest AccuracyCoin baseline at a pinned upstream commit, and rely on bounded evidence that accepted programs run on all six board families through reset, save/load, unload/reload, guest loops and JAM. The baseline is a measurement, not an all-pass claim: 101 of 146 scored rows pass.

**Phases completed:** 3 phases (16 to 18), 21 plans, 42 tasks. Phase 16 merged as PR #50 and released as v0.1.11; Phases 17 and 18 ride PR #55 (PR #54 is folded into it)
**Timeline:** 2026-10-13 → 2026-10-24 (12 days)
**Code:** 39,760 lines of C (8,979 in the core library and public header), excluding the vendored `libretro.h`; 669 CTest tests in `ci`
**Requirements:** 14/14 v4 requirements satisfied ([audit](milestones/v4-MILESTONE-AUDIT.md), status tech_debt)

**Key accomplishments:**
- An owned frame counter and one bounded verdict make the real RetroArch progression check reject dropped, doubled and wrong-load sequences. Network commands are confirmed rather than blindly retried, fixtures are chosen by exact identity, and v0.1.11's metadata describes MMC3 truthfully.
- PRODUCT-ROADMAP.md keeps four horizons with stable RM IDs, and both milestone boundaries reconcile it within the invoked step.
- AccuracyCoin is pinned at `6bee919` with a tested compressed-name decoder. A 22-page bounded campaign classifies all 146 scored rows (101 pass, 42 fail, 2 not-run, 1 timeout), keeps the five DRAW entries out of the score, and retains the old `673ef55` comparison.
- Every CI lane freshly re-measures the PASS set taken from one protected-main revision. All 17 old protected keys still pass.
- A 16-byte NEXE descriptor generates bank-sensitive programs on mappers 0, 1, 2, 3, 4 and 7. They pass continuation, reset, state-load and instance-isolation checks, and reach 24 core functions beyond the loader corpora.
- Guest loops and JAM have tested native outcomes; on JAM the libretro adapter repeats the last complete frame and sends no audio. Finite nightly sanitizer campaigns run under measured budgets, and they found and fixed a real libretro state-load presentation bug.

**Closeout:** override closeout. All three phase verifications passed (22/22, 38/38, 4/4), but GSD marks Phases 16 and 17 stale because later phases changed files they cover. The milestone audit ran after all three phases and found 14/14 requirements, 5/5 integration and 5/5 flows.

Known verification overrides: 3 newly acknowledged, 7 carried forward from a prior close (see STATE.md Deferred Items)

**Delivery at close:** Phases 17 and 18 had not merged to main. Exact-head PR #55 CI run 40150921041 passed every check, and Nightly 40150888217 succeeded. Publication is recorded below once observed.

**Delivery reconciliation — 2026-10-30:** PR #55 merged; package release PR #56
(0.1.12) remains open and includes v4 baseline/robustness plus v5. No package
publication is claimed. v5 delivered the two JAM tune-up gaps below; later
accepted vector evidence passes. Other carried debt remains in the product ledger.

**Deferred tech debt:**
- AccuracyCoin observe records a JAM only as a generic frame failure, and no AccuracyCoin control covers JAM.
- No explicit run-ahead-under-JAM test exists in `libretro.runahead`.
- Real-campaign minimization is proven only by controls.
- Nightly `vectors-full` failed in run 39993108154 (not a Release gate).
- Local macOS `nofp` flags `stack_chk_fail`/`stack_chk_guard` from the Apple toolchain.
- Debug sessions `phase16-cleanup-fixture` and `phase16-interruption` were acknowledged, not resolved.

---

## v3 Save states and rewind (Shipped: 2026-10-13)

**Delivered:** Hosts save and load the whole machine through the library, players save and load states, rewind and use run-ahead in RetroArch, and a launcher can stop nes-runner at any time without losing a save and read every outcome from one documented exit-status table. No frame or audio hash moved, and speed is measured on every pull request.

**Phases completed:** 5 phases (11 to 15), 38 plans, merged as PRs #42 to #46
**Timeline:** 2026-10-11 → 2026-10-13 (3 days); v0.1.10 published on 2026-10-13 after release PR #35 merged (dated closure below)
**Code:** 32,949 lines of C (8,804 in the core library and public header), excluding the vendored `libretro.h`; 633 CTest tests in `ci`
**Requirements:** 18/18 v3 requirements satisfied ([audit](milestones/v3-MILESTONE-AUDIT.md), status tech_debt)

**Key accomplishments:**
- Battery saves go through one atomic-replace helper that leaves no stale `.tmp`, `retroarch-e2e` round-trips an MMC3 `.srm` beside MMC1, and `nesturbator_get_version` and `nesturbator_unload_cartridge` have callers and README text.
- A fixed-size save state (64-byte header, CRC-32, one visitor for size, write, validate, apply and record) covers all six boards; every field is checked before anything is applied, and forks at every frame of eight ROMs continue byte for byte, pinned on six platforms through `--state-at-frame`, `--load-state` and `--hash-state`.
- RetroArch saves and loads states, rewinds and runs ahead with the core under a `deterministic` `.info` claim that a configure-time guard ties to its proving tests, and the state loader is fuzzed nightly.
- SIGINT, SIGTERM, Ctrl+C, Ctrl+Break and console close finish the frame, flush the save and exit 0, proven with real signals on six platforms; every exit status is one table that `--help` prints and a test holds to the README.
- `--bench` prints frames per second and frame-time percentiles, a Cachegrind `perf` job gates instructions per frame at +1% per ROM against the merge base, and every workflow timeout is twice a measured run whose ID sits beside it.

**Closed after the audit:** none; the audit ran on the merged head.

**Closeout:** override closeout. All five phase verifications passed, but GSD marked Phases 11 to 14 stale because later phases changed files they cover. CI, Nightly and Release passed on every job at `b0ec7fb`, and the milestone audit covers the phases.

Known verification overrides: 0 newly acknowledged, 9 carried forward from the v1 close (see STATE.md Deferred Items)

**Release blocker found at close:** release PR #35 (v0.1.10) has not merged since v2 because its `hygiene.format` check fails on `include/nesturbator.h:43-44`. The bumped patch number shifts the aligned `x-release-please` version comments, so no release has gone out since v0.1.9 and none of v3 has been published.

**Release-blocker closure — 2026-10-13:** The paragraph above retains the
milestone-close observation. [PR #48](https://github.com/szTheory/nesturbator/pull/48)
fixed version-comment formatting and merged as `b8e2824`.
[Release PR #35](https://github.com/szTheory/nesturbator/pull/35) then merged at
2026-10-13T16:38:38Z as `da2bf973813530cb9561a004e7b774011a0a7867` after
15 successful CI checks. Publication was pending independently of that merge.
Read-only evidence at 2026-10-13T16:53:46Z now confirms
[run 38432194730](https://github.com/szTheory/nesturbator/actions/runs/38432194730)
completed successfully on the exact release SHA, including `publish`.
[v0.1.10](https://github.com/szTheory/nesturbator/releases/tag/v0.1.10) is non-draft,
published at 2026-10-13T16:53:40Z, with exactly the 18 expected library, runner
and libretro ZIPs across Linux/macOS/Windows x64/arm64 plus `SHA256SUMS`, all
with nonzero sizes. Publication is complete; the deferred technical debt below
is unchanged.

**Deferred tech debt:**
- Phase 13 WR-01: the state-net frame-chain check cannot detect a dropped or doubled FRAMEADVANCE.
- Phase 13 WR-02: `state_session.sh` sends most commands once, though RetroArch can miss one.
- Phase 13 WR-03: special-case ROM selection matches a substring of the full path.
- The RetroArch state, rewind and run-ahead e2e runs only on macOS; other platforms run `libretro.serialize` and `libretro.runahead`.
- The libretro adapter does not read `nesturbator_save_generation`; libretro has no such signal and the host writes the `.srm`.
- `runner/save.h:36` says state writes will use the atomic helper "later"; they already do.
- Open info findings in Phases 11, 13, 14 and 15 (listed in the audit), including the `.info` description that omits MMC3.
- Summaries for TUNE-07, TUNE-09, PERF-02, PERF-03 and TUNE-10 lack a `requirements-completed` key; the audit checked each by hand.

---

## v2 Most of the library plays (Shipped: 2026-10-11)

**Delivered:** Games on the six common board families (mappers 0, 1, 2, 3, 4 and 7, about 91% of the North American licensed library) play through the runner and RetroArch with battery saves, with frame and audio hashes identical on Linux, macOS and Windows on x64 and arm64, and one game proven to play from boot by an automated check.

**Phases completed:** 6 phases (05 to 10), 41 plans, 99 tasks
**Timeline:** 2026-10-10 → 2026-10-11 (2 days), releases v0.1.7 to v0.1.9, with v0.1.10 in release PR #35
**Code:** 22,419 lines of C (7,801 in the core library and public header), excluding the vendored `libretro.h`; 481 CTest tests in `ci`
**Requirements:** 19/19 v2 requirements satisfied ([audit](milestones/v2-MILESTONE-AUDIT.md), status tech_debt)

**Key accomplishments:**
- CI runs CTest in parallel with a nightly flake job, the v1 debt is closed (exact release-policy gate, trainer image through every host path, no self-skipping tests), and `retro_reset()` performs the console's soft reset.
- Every cartridge loads through one per-board mapper ops table, and the PPU runs the 2C02 fetch pipeline from `v` with A12 on the bus, so mid-frame scroll splits render as on the console.
- UxROM, CNROM, AxROM, MMC1 and MMC3 boards run, with bus conflicts by submapper and the MMC3 scanline IRQ clocked by M2-filtered A12 rises; Holy Mapperel reports `0000` on every board, and 10 of 12 blargg MMC3 ROMs pass nightly (the two NEC-revision ROMs are recorded unsupported).
- Battery saves persist as raw `.sav` bytes through a library span with a write generation, the runner's atomic `--save-dir` writes, and RetroArch's `.srm`.
- One 95-key hash list pins every board on six platforms, the libretro adapter matches the runner's frames and saves for every ROM in scope, and `runner.boot_to_play` proves nes-runner plays from boot against four negative controls and eight mutants.

**Closed after the audit:** G1 reset-retention tests (PR #38), 07 WR-01 AxROM reset-vector wording (PR #38), 08 WR-01 interval-flush retry at exit (PR #39), and the asan timeout, now 10 minutes against a measured 4m52s run (PR #39).

**Closeout:** override closeout. All six phase verifications passed, but GSD marked them stale because later phases and quick tasks #38 and #39 changed files they cover. CI passed on every job at `493b995`, and the milestone audit covers the phases.

Known verification overrides: 0 newly acknowledged, 9 carried forward from the v1 close (see STATE.md Deferred Items)

**Deferred tech debt:**
- G2: retroarch-e2e round-trips a `.srm` for MMC1 only; MMC3 saves are covered by the simulated host.
- G3: the MMC3 IRQ is checked against the blargg ROMs only in the nightly mmc3-oracle lane.
- G4: AccuracyCoin has no libretro parity test, by design (D-14).
- G6: `nesturbator_get_version` and `nesturbator_unload_cartridge` have no runner or libretro consumer and no README mention.
- Backlog 999.1: the nightly suite-flake timeout is a 30-minute ceiling, not twice a measured run.
- 08 IN-02: a failed save write can leave a stale `.tmp` file.
- Open review items in phases 05 to 10 (listed in the audit), most of them in test code.
- The PPU fetch pipeline cost about 19 percent frame rate; measured, not gated.

---

## v1 NROM games with sound (Shipped: 2026-10-09)

**Delivered:** NROM games render, take controller input and play with sound in RetroArch, with frame and audio hashes identical on Linux, macOS and Windows on x64 and arm64.

**Phases completed:** 6 phases (01, 02, 03, 04, 04.1, 04.2), 44 plans, 73 tasks
**Timeline:** 2026-10-02 → 2026-10-10 (8 days), releases v0.1.0 through v0.1.6
**Code:** 13,642 lines of C (5,964 in the core library), excluding the vendored `libretro.h`
**Requirements:** 19/19 v1 requirements satisfied ([audit](milestones/v1-MILESTONE-AUDIT.md))

**Key accomplishments:**
- A C17 library, headless runner and libretro core build, test and release on six platforms behind one required CI check; merging a behaviour change publishes 18 attested archives and `SHA256SUMS` with no manual step.
- The 6502 matches the public 65x02 vectors on all 256 opcodes on every bus cycle: a committed sample runs in CI, and the full 2,560,000-test set runs nightly at its pinned commit.
- A bounded mapper-0 iNES/NES 2.0 loader, a dot-timed PPU with sprites and OAM DMA, and two controller ports run three open-licence games whose frame hashes match byte for byte on all six platforms and in a pinned RetroArch screenshot check.
- Deterministic movie replay, AccuracyCoin results read from emulated RAM against a protected scoreboard, and loader fuzzing (corpus replay in CI, libFuzzer nightly).
- An integer APU and band-limited synthesiser produce mono PCM whose transition and sample hashes match on every platform; five reference tones stay below -80 dB of non-harmonic content, and libretro delivers equal stereo through both the batch and single-sample callbacks.

**Closeout:** override closeout. All six phase verifications passed, but GSD marked them stale because 03-15, 04.1, 04.2 and release version bumps later changed covered files. Those changes carry their own passing verifications, `main` CI passed at `5c082c1`, and the milestone audit covers them.

Known verification overrides: 9 newly acknowledged, 0 carried forward from a prior close (see STATE.md Deferred Items)

**Deferred tech debt:**
- `tests/cmake/release_policy.cmake` matches the publish gate by substring; an appended `|| always()` could evade it.
- No trainer-bearing fixture runs the full runner or libretro host path.
- The local `retroarch.testframe` and `retroarch.game` tests self-skip on the owner's Mac because RetroArch does not start there; only the hosted `retroarch-e2e` job gives live screenshot evidence.
- `retro_reset()` does nothing for a loaded game (outside v1 requirements).

---
