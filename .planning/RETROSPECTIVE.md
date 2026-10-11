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

## Cross-Milestone Trends

### Process Evolution

| Milestone | Sessions | Phases | Key Change |
|-----------|----------|--------|------------|
| v1 | — | 6 | Automation-first verification; zero manual UAT for product behaviour |
| v2 | — | 6 | Milestone audit before close-out; post-audit quick tasks; per-preset CI cost |

### Cumulative Quality

| Milestone | Tests | Coverage | Zero-Dep Additions |
|-----------|-------|----------|-------------------|
| v1 | 357 CTest tests in `ci` | not measured | all (only `libretro.h` vendored) |
| v2 | 481 CTest tests in `ci` | not measured | all (only `libretro.h` vendored) |

### Top Lessons (Verified Across Milestones)

1. Prove playability with one integrated check from boot; both milestones found real bugs this way (v1 gap phases, v2 NMI edge).
2. Fixes landing after phase verification make the close an override; plan for it or re-verify in one batch.
