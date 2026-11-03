# Project Retrospective

*A living document updated after each milestone. Lessons feed forward into future planning.*

## Milestone: v1 — NROM games with sound

**Shipped:** 2026-10-09
**Phases:** 6 (two inserted) | **Plans:** 44 | **Tasks:** 73 | **Releases:** v0.1.0 to v0.1.6

### What Was Built
- A C17 library, headless runner and libretro core that build, test, package and release on six platforms behind one required CI check.
- A 6502 that matches all 256 opcodes of the public 65x02 vectors on every bus cycle, with the full set run nightly.
- A bounded mapper-0 loader, a dot-timed PPU with sprites and OAM DMA, two controller ports, movie replay and an AccuracyCoin scoreboard read from emulated RAM.
- An integer APU and band-limited synthesiser whose transition and PCM hashes match on every platform, with libretro batch and single-sample audio paths.

### What Worked
- Shipping the release pipeline and the built-in test frame first, so every later phase ended in a published release by merging.
- Tracer plans (02-01, 02-02, 03-01) that pushed one thin slice through every layer before widening it.
- Byte-exact hash inventories compared across all six platforms. They turned determinism from a claim into a CI gate.
- Committed, licensed samples in CI with the full pinned suite fetched cold at night: fast feedback without giving up coverage.

### What Was Inefficient
- Two gap-closure phases (04.1 trainers, 04.2 single-sample audio) and a late Phase 3 plan (03-15 sprite overflow) came after the main phases were verified. One integrated playability check would have caught some of these sooner.
- Gap fixes landing after phase verification left all six verifications marked stale, forcing an override closeout.
- Events that only happen after a merge (the first scheduled nightly, release publication) needed an owner-authorized substitute until the real runs arrived.
- The local RetroArch tests stopped running on the owner's Mac partway through, leaving only the hosted job as screenshot evidence.

### Patterns Established
- Every machine-checkable rule becomes a CTest gate with a mutation self-test; owner input is reserved for consent and clean-room provenance.
- Gaps found late are closed with inserted decimal phases (`/gsd-phase --insert`), each with its own verification.
- Evidence from GitHub runs is tied to its exact commit and checked by command, never by reading a dashboard.

### Key Lessons
1. Prove a playability claim with one integrated host session (boot, input that changes play, scrolling, non-silent audio). Seam tests alone don't prove it. See SEED-261009-zs2.
2. Close verification gaps in one batch against the current revision, then re-verify. Never edit verdicts to move routing forward.
3. Diagnose a stalled guest at its first divergence with a bounded trace before adding cycles or device activity.
4. Record acceptance blockers apart from engineering dependencies, and keep independent work moving.

### Cost Observations
- Model mix: not tracked this milestone.
- Sessions: not tracked.
- Notable: the longest plan was 04-02 (122 min, APU frame counter, IRQ and DMC DMA). Most plans finished in 3 to 16 minutes.

---

## Milestone: v2 — Most of the library plays

**Shipped:** 2026-10-11
**Phases:** 6 | **Plans:** 41 | **Tasks:** 99 | **Releases:** v0.1.7 to v0.1.9, v0.1.10 pending in release PR #35

### What Was Built
- A tune-up: parallel CTest with a nightly flake job, an exact release-policy gate check, `policy.no-skip`, and a soft reset shared by the library and `retro_reset()`.
- One per-board mapper ops table, and a 2C02 fetch pipeline driven from `v` with A12 on the PPU bus.
- UxROM, CNROM, AxROM, MMC1 and MMC3 boards, with bus conflicts by submapper and the M2-filtered A12 scanline IRQ.
- Battery saves through a library span with a write generation, atomic runner writes and RetroArch `.srm`.
- One 95-key hash list on six platforms, libretro parity with the runner for every ROM in scope, and `runner.boot_to_play` with negative controls and mutants.

### What Worked
- Landing the mapper seam first as its own plan with every v1 hash unchanged, so the later PPU re-pin had a clean baseline.
- Building the boards from cheapest to hardest: by MMC3, the loader, seeds, Holy Mapperel reader and hash list were all in place.
- Seed lesson SEED-261009-zs2 paid off: the boot-to-play check found the PPUCTRL NMI edge bug (1086 NMIs in 10 frames), which no seam test had caught.
- The milestone audit before close-out turned review leftovers into small quick tasks (#38, #39) instead of a gap phase.

### What Was Inefficient
- Long hash tests were run again in full under asan, which pushed that job to about 15 minutes; it took two quick tasks (a timeout raise, then a short-run option) to fix.
- Quick tasks after the phases left all six verifications marked stale again, forcing a second override closeout.
- Review leftovers were spread across six disposition files; the audit had to gather them by hand.
- Phase 9 took 12 plans, mostly fuzz seeds split across waves for size.

### Patterns Established
- One key list (`tests/cmake/hash_keys.cmake`) that the writer, validator, fixtures and drift guard all read.
- Sanitizer lanes check the first frames, not full-length runs; full-length hashes stay on the build jobs.
- Nightly oracle lanes for suites that are fetched cold (blargg MMC3), with unsupported rows recorded rather than skipped.
- CI timeouts are set to twice a measured hosted run and cite it.

### Key Lessons
1. Measure a new test's cost under each preset, asan included, before merging it; asan multiplies run time.
2. A play check from boot catches bugs that unit-level tests miss; keep one per milestone.
3. Do post-audit fixes before the last phase verification, or accept that close-out will be an override.
4. Gather review leftovers into one place at each phase transition, not at the audit.

### Cost Observations
- Model mix: planning on opus, execution mostly on sonnet; not measured.
- Sessions: not tracked.
- Notable: two days for six phases and 41 plans; Phase 9 (12 plans) was the longest.

---

## Milestone: v3 — Save states and rewind

**Shipped:** 2026-10-13
**Phases:** 5 | **Plans:** 38 | **Releases:** none yet; v0.1.10 in release PR #35 is blocked by its `hygiene.format` check

### What Was Built
- One atomic-replace helper for battery saves, an MMC3 `.srm` round trip in RetroArch, and callers for every public function.
- A fixed-size, CRC-checked save state on all six boards from one visitor, validated in full before it is applied, with fork tests at every frame and six-platform state pins.
- libretro serialize, rewind and run-ahead under a `deterministic` claim tied to its tests, a RetroArch state e2e and a nightly state-loader fuzz target.
- A signal-safe runner stop and one exit-status table shared by `--help`, the code and the README.
- `--bench` frame-time percentiles and a per-PR Cachegrind instruction gate.

### What Worked
- One visitor for size, write, validate, apply and record: save and load never drifted, and the rejection sweep came almost free.
- Staging the `.info` claim (`serialized`, then `deterministic`) behind a configure-time guard kept every claim behind green evidence.
- Real-signal and real-Ctrl+Break tests found an EINTR abort in the state read that a simulated stop would have missed.
- Instruction counts, not wall time, made the perf gate deterministic; two zero-spread control runs justified arming it.

### What Was Inefficient
- Guessed CI timeouts cancelled the macOS Intel build on every Phase 13 head; it took three measured runs and a rule (`hygiene.timeout_comments`) to stop it.
- Release PR #35 has stayed open since v2 and was failing `hygiene.format` at the milestone close; the release path was not checked at each phase close.
- Plan summaries used `requirements:` or no key instead of `requirements-completed:`, so the audit cross-checked five requirements by hand.
- Phase 12 needed 10 plans; the fork and rejection matrices were split across waves for size.

### Patterns Established
- A save state's field list lives in one visitor; a board without a `state` op does not build.
- Every libretro `.info` capability maps to the CTests that prove it, enforced at configure time.
- Every `timeout-minutes` line carries the ID of the run it doubles.
- Process outcomes come from one X-macro table checked against the README.

### Key Lessons
1. Check that the release PR is green at every phase close; "each phase ends in a release" needs a test, not an assumption.
2. Formatting rules that align comments break on release-tool version bumps; keep machine-edited lines out of aligned blocks.
3. Use the `requirements-completed` key in every summary so the audit's three-source check stays automatic.
4. Measure timeouts before guessing them; a guessed limit fails healthy jobs.

### Cost Observations
- Model mix: planning and execution on opus; not measured in detail.
- Sessions: not tracked.
- Notable: three days for five phases and 38 plans; Phase 12 (10 plans) was the longest.

---

## Milestone: v4 — Measured NTSC accuracy and robustness

**Shipped:** 2026-10-24
**Phases:** 3 | **Plans:** 21 live (Phase 16 had 56 physical plan files) | **Releases:** v0.1.11 (Phase 16); Phases 17–18 ship through PR #55

### What Was Built
- RetroArch progression/state checks with an owned frame counter and one bounded verdict that rejects dropped, doubled and wrong-load sequences; confirmed network commands; exact fixture identity; truthful MMC3 metadata.
- A four-horizon PRODUCT-ROADMAP.md reconciled at both milestone boundaries.
- A complete AccuracyCoin inventory at `6bee919` and a 22-page bounded campaign: 101 pass, 42 fail, 2 not-run, 1 timeout out of 146 scored rows, with fresh PASS protection on every lane.
- An owned execution fuzz/replay target on all six board families, with lifecycle, loop and JAM controls and finite nightly sanitizer campaigns.

### What Worked
- Phases 17 and 18 ran four plans each, in order, with no loop; independent verification passed first time after review fixes.
- Separating "observed" from "passed" let the baseline be honest: a guest timeout is valid evidence, DRAW stays out of the denominator, and missing results never become passes.
- Generating programs from a 16-byte descriptor reached 24 core functions the loader corpora never touched, and a finite campaign found a real libretro state-load presentation bug.

### What Was Inefficient
- Phase 16 looped: 62 consecutive docs commits and 56 physical plans for 13 live ones. Each bookkeeping failure became a new plan that inherited every earlier control ([forensics](forensics/report-20261022-234424.md)). It took an explicit recovery to stop.
- Two Phase 16 debug sessions were left open and were only acknowledged at close.
- Phases 17 and 18 piled up as stacked draft PRs instead of merging at each phase close, so delivery lagged the milestone.

### Patterns Established
- Acceptance binds to product behaviour on the actual tested source, not to meta-evidence machinery.
- Protected scores come from one protected-main revision plus fresh, unique raw evidence per key.
- Fuzz and campaign budgets come from a measured hosted pilot, with separate fuzzer, replay, memory and outer-process limits; a clean campaign is reported as finite evidence.

### Key Lessons
1. When a tool or bookkeeping step fails, diagnose it and fix it inside the step; never turn it into a new prerequisite plan.
2. Merge each phase's PR when its CI is green, before starting the next phase; stacked drafts delay release and cloud the audit.
3. Measure first, then choose: the full baseline is what changed the next-milestone order from PAL to NTSC timing.

### Cost Observations
- Model mix: planning and execution on opus, with some sessions on Codex; not measured in detail.
- Sessions: not tracked.
- Notable: Phase 16 took ten of the twelve days; Phases 17 and 18 took about one day each.

---

## Milestone: v5 — NMI, VBlank and sprite-0 timing

**Shipped:** 2026-10-30 (planning milestone; 0.1.12 publication pending).
**Phases:** 3 | **Plans:** 27 | **Tasks:** 68 | **Timeline:** six calendar dates

### What Was Built
- Eleven timing targets pass through OAMADDR sprite evaluation, split status sampling, NMI capture/poll and shared interrupt entry.
- Guest JAM attribution and actual-adapter frozen-frame/silence behavior survive rewind and run-ahead.
- Format-3 state validation and native/actual-adapter entry-cutpoint continuation with source-bound six-platform acceptance.

### What Worked
- Owned pre-fix reproducers and exact revert/mutant controls separated real hardware-model defects from plausible wrong fixes.
- Native and loaded-adapter replay checked actual guest markers, state, frame and audio history.
- Full-key PASS protection and unchanged performance thresholds kept the improvement attributable; all three phase reports were refreshed before closeout.

### What Was Inefficient
- Hosted collection needed source-root/archive/log repairs and run-specific mode checks; recurring collector defects remain maintenance debt.
- Later review-record changes invalidated cross-phase verification, requiring sequential refreshes.
- Body-only summary parsing undercounted 68 recorded tasks as 49 and missed completed requirement claims; closeout reconciles the evidence without changing historical summaries.

### Patterns Established
- Separate raw assertions, inhibited candidates, recognized requests and legal poll acceptance in serialized history.
- Preserve immutable hosted source identities separately from current local tests and final-head workflow success.
- Refresh direction from current exact-key outcomes: three of the old eight DMA candidates now pass; the remaining cluster is five failures.

### Key Lessons
1. Treat collector identity enforcement as a prerequisite for trustworthy new evidence, not a property supplied permanently by one independent historical check.
2. Batch source and review-disposition updates before final phase verification where possible; never rewrite fingerprints simply to remove a stale route.
3. Keep package publication separate from merged implementation and planning completion; release PR #56 is still open.
4. Repair routine ledger representation from retained source evidence inside closeout without manufacturing a new product phase or historical deviation.

### Cost Observations
- Model mix, token costs and sessions were not measured consistently.
- Phase 20 needed twelve plans including evidence gap closure; Phase 21 needed nine. No new speedup claim is made from this record.

---

## Milestone: v6 — Player-ready NES and reproducible feedback

**Shipped:** 2026-10-31 (planning milestone; no package release claimed)
**Phases:** 2 | **Plans:** 7 | **Tasks:** 15 | **Timeline:** two calendar dates

### What Was Built
- A player-facing README route with canonical API, runner, release, testing and progress guides, each retaining its executable contract checks.
- Build identity and progress guidance that distinguishes battery saves, native snapshots and host containers, with backup and rollback limits.
- A sanitized scene report, bounded repeated cold replay, native checkpoint identity preflight and neutral continuation.
- Layered evidence guidance that routes each observation to bounded checks and separates repeatability from correctness claims.

### What Worked
- Independent negative controls kept documentation contracts meaningful through the README migration.
- The two-phase structure separated player installation/progress guidance from scene replay/reporting while reusing existing runner facilities.
- The milestone audit reconciled ten requirements and three flows without rewriting summaries or invalidating verification fingerprints.

### What Was Inefficient
- Phase 22 summary metadata omitted or left empty the six READY requirement completions, so the audit reconciled them manually.
- The archive CLI counted eight tasks while the seven plan summaries record fifteen; the closeout uses the summary total and records the discrepancy.
- Intermittent document-test timing anomalies remained unexplained. A prior full-CI/campaign run was interrupted with an unknown outcome, and the safety stop prevented a rerun.

### Patterns Established
- Keep one canonical guide per topic and maintain independent checks for relocated claims.
- Treat matching frame/audio hashes as repeatability evidence only; preserve separate adapter, host and hardware claims.
- Check the current release PR status before publication; automatic publication is authorized only after all required checks pass.
- Keep interrupted or incomplete campaign results unknown unless a separately authorized run establishes the outcome.

### Key Lessons
1. Populate requirement-completion and task metadata consistently in each phase summary; the closeout counter must agree with the retained plan evidence.
2. Keep release publication separate from planning completion and merged implementation; a failed or cancelled check means the release remains held.
3. Reuse bounded permitted fixtures and distinguish native checkpoint provenance from general build/content support.
4. Preserve timing, platform, campaign and judgment-tier limits without converting them into unrequested runtime work or risk acceptance.

### Cost Observations
- Model mix and session count were not measured.
- Notable: two phases, seven plans and fifteen tasks completed over two calendar dates; no speedup claim is made.

---

## Milestone: v7 — Remaining DMA accuracy failures

**Shipped:** 2026-11-01 (planning milestone; no package release or tag)
**Phases:** 1 | **Plans:** 5 | **Tasks:** 15 | **Timeline:** two calendar dates

### What Was Built
- Corrected five selected NTSC DMA failures at the unchanged AccuracyCoin pin, each with cycle-level evidence and owned wrong-model controls.
- Preserved all protected PASS keys, deterministic six-platform output, native continuation, and existing performance limits.
- Updated native state to format 6 and behavior revision 14 while retaining ABI 1; old and invalid states have explicit refusal coverage.
- Passed the local 707-test workflow and candidate-bound hosted CI/Nightly, hash-equality and Cachegrind evidence.

### What Worked
- Separating hardware references, pinned test expectations and observed core traces kept the five rows independently justified.
- The exact-candidate receipt, protected-main observations, six-platform hashes and raw performance measurements were validated as separate evidence classes.
- The milestone audit and static integration trace found no unsatisfied requirements or broken flows; historical stop conditions remained explicit.

### What Was Inefficient
- The archive CLI reported three tasks and no accomplishments because summary metadata was incomplete; the closeout reconciled fifteen tasks from the five plan contracts and extracted accomplishments manually.
- The audit found README acceptance wording that still described already-passed exact-candidate evidence as outstanding. This was carried as non-blocking documentation debt.
- Historical branch-wide whitespace diagnostics remain in archived planning evidence; scoped changes passed their own checks, and unrelated old files were not bulk-edited.

### Patterns Established
- Record the tested code candidate separately from later evidence/documentation commits.
- Keep old interrupted campaign results unknown unless a separately authorized run establishes them.
- At closeout, reconcile the product roadmap's Immediate item to shipped, preserve all four horizons and triggers, and keep package publication separate.

### Key Lessons
1. Keep summary task metadata complete; verify archive-tool statistics against plan evidence before writing the historical record.
2. Update public acceptance status when new evidence closes a gap, then verify the README claim against the final report before audit.
3. Preserve independent run identity and existing stop conditions when exact-candidate evidence arrives later in the workflow.

### Cost Observations
- Model mix and session count were not measured.
- Five plans each declared three tasks; no speedup or runtime-budget change is claimed.

---

## Milestone: v8 — PAL region support

**Shipped:** 2026-11-03 (planning milestone; no package release or tag)
**Phases:** 4 | **Plans:** 12 | **Tasks:** 31 | **Timeline:** three calendar dates

### What Was Built
- Region-aware loading and PAL CPU/PPU/APU timing cover all six existing mapper families, with exact integer frame/sample accounting and NTSC output retained.
- PAL DMC arbitration uses a bounded, source-backed 2A07 model; regional state identity and continuation reject cross-region states before applying them.
- The library, runner and libretro adapter expose matching AV profiles and a named modeled 2C07 palette, with color and hardware limits documented.
- Phase 27.1 validates PAL five-step state endpoints and regional frame counters as supplemental evidence for Phase 26's sole STATE-01 ownership.
- All seven requirements, four phase verifications, seven integration paths and five end-to-end flows pass. The accepted local workflow passed 722 tests and packaged all three deliverables.

### What Worked
- One shared loader policy kept region precedence consistent across the library, runner and adapter while preserving the existing Auto API.
- Exact rational timing, per-instance remainders, six-mapper PAL/NTSC controls and owned wrong-model fixtures made the acceptance observable without relying on rounded 50 Hz timing or hardware claims.
- The audit traced all three deliverables and separated the non-blocking WR-01 metadata warning from current-candidate acceptance evidence.
- The older interrupted campaign stop stayed explicit; closeout relied on the current audit and retained phase evidence without restarting it.

### What Was Inefficient
- The first documentary audit pass exposed missing Phase 26 summary attribution and a Phase 27.1 roadmap marker. The repairs made Phase 26 verification stale, so a separate verification refresh and audit rerun were needed before closeout.
- A quick-task handoff remained marked halted after its original stale-verification blocker was cleared by those later steps; closeout reconciled its status to the current evidence.
- The archive tool's counts and accomplishments still need review against plan contracts and phase summaries; the 31-task total is taken from the twelve plan estimates.

### Patterns Established
- Supplemental validation can close an audited acceptance case without creating a second requirement owner.
- A passing audit can retain transparent hardware, host and provenance debt without treating it as an unsatisfied software requirement.
- Closeout records the product-roadmap transition, package boundary and historical campaign stop separately from the phase acceptance result.

### Key Lessons
1. Run the canonical milestone manager after documentary summary or roadmap edits; phase completeness and verification freshness are separate signals.
2. Keep requirement ownership distinct from additional validation evidence, and make the parser-facing summary metadata match the implementing plans.
3. Verify current-candidate evidence independently from stale baseline metadata; preserve the warning when the current receipt remains valid.
4. Keep PAL model evidence, direct hardware qualification, host pacing and the older interrupted campaign as separate claims.

### Cost Observations
- Model mix and session count were not tracked consistently.
- The 722-test local workflow and all package builds were completed during phase work; closeout did not rerun CI or the stopped campaign.
- No new CI-efficiency change or speedup is claimed.

---

## Cross-Milestone Trends

### Process Evolution

| Milestone | Sessions | Phases | Key Change |
|-----------|----------|--------|------------|
| v1 | — | 6 | Automation-first verification; zero manual UAT for product behaviour |
| v2 | — | 6 | Milestone audit before close-out; post-audit quick tasks; per-preset CI cost |
| v3 | — | 5 | Measured timeouts with run IDs; claims tied to tests at configure time; per-PR perf gate |
| v4 | — | 3 | Rolling product roadmap at both boundaries; loop recovery that binds acceptance to product behaviour |
| v5 | — | 3 | Causal timing reproducers, native/actual-adapter cutpoints and source-bound hosted acceptance; current passed reports before closeout |
| v6 | — | 2 | Executable documentation contracts, bounded scene replay and explicit separation of repeatability from correctness; release status checked at closeout |
| v7 | — | 1 | Candidate-bound DMA acceptance, native continuation and evidence provenance; prior interruption stop preserved |
| v8 | — | 4 | Region support across all three deliverables, supplemental state validation and a refreshed audit that carries evidence limits without a closeout override |

### Cumulative Quality

| Milestone | Tests | Coverage | Zero-Dep Additions |
|-----------|-------|----------|-------------------|
| v1 | 357 CTest tests in `ci` | not measured | all (only `libretro.h` vendored) |
| v2 | 481 CTest tests in `ci` | not measured | all (only `libretro.h` vendored) |
| v3 | 633 CTest tests in `ci` | not measured | all (only `libretro.h` vendored) |
| v4 | 669 CTest tests in `ci` | 24 core functions beyond loader corpora (execution fuzz) | all (only `libretro.h` vendored) |
| v5 | 695 CTest tests in `ci` | exact eleven-key target contract plus every-stage entry replay; no percentage claimed | all (only `libretro.h` vendored) |
| v6 | not recounted; full CI not rerun | 10/10 requirements, 2/2 passed phases and 3/3 flows; no runtime code change | all (only `libretro.h` vendored) |
| v7 | 707 CTest tests in `ci` | 4/4 requirements, 9/9 phase truths and 5/5 traced flows | all (only `libretro.h` vendored) |
| v8 | 722 CTest tests in `ci` | 7/7 requirements, 4/4 passed phases, 7/7 integration paths and 5/5 flows | all (only `libretro.h` vendored) |

### Top Lessons (Verified Across Milestones)

1. Prove behaviour with one integrated check against the real host or process; every milestone found real bugs this way (v1 gap phases, v2 NMI edge, v3 EINTR in the state read).
2. Fixes landing after phase verification make the close an override; plan for it or re-verify in one batch (v3 and v4 both closed as overrides for this reason).
3. Every limit or claim needs its measurement or test beside it (timeouts with run IDs, `.info` levels with CTests, exit statuses with the README).
4. Keep process overhead proportional: evidence machinery that grows its own prerequisites stalls delivery (v4 Phase 16).
5. Re-run the canonical manager after documentary edits; implementation completeness and current verification status are separate signals (v8).
