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

## Cross-Milestone Trends

### Process Evolution

| Milestone | Sessions | Phases | Key Change |
|-----------|----------|--------|------------|
| v1 | — | 6 | Automation-first verification; zero manual UAT for product behaviour |
| v2 | — | 6 | Milestone audit before close-out; post-audit quick tasks; per-preset CI cost |
| v3 | — | 5 | Measured timeouts with run IDs; claims tied to tests at configure time; per-PR perf gate |

### Cumulative Quality

| Milestone | Tests | Coverage | Zero-Dep Additions |
|-----------|-------|----------|-------------------|
| v1 | 357 CTest tests in `ci` | not measured | all (only `libretro.h` vendored) |
| v2 | 481 CTest tests in `ci` | not measured | all (only `libretro.h` vendored) |
| v3 | 633 CTest tests in `ci` | not measured | all (only `libretro.h` vendored) |

### Top Lessons (Verified Across Milestones)

1. Prove behaviour with one integrated check against the real host or process; every milestone found real bugs this way (v1 gap phases, v2 NMI edge, v3 EINTR in the state read).
2. Fixes landing after phase verification make the close an override; plan for it or re-verify in one batch.
3. Every limit or claim needs its measurement or test beside it (timeouts with run IDs, `.info` levels with CTests, exit statuses with the README).
