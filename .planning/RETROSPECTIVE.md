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

## Cross-Milestone Trends

### Process Evolution

| Milestone | Sessions | Phases | Key Change |
|-----------|----------|--------|------------|
| v1 | — | 6 | Automation-first verification; zero manual UAT for product behaviour |

### Cumulative Quality

| Milestone | Tests | Coverage | Zero-Dep Additions |
|-----------|-------|----------|-------------------|
| v1 | 357 CTest tests in `ci` | not measured | all (only `libretro.h` vendored) |

### Top Lessons (Verified Across Milestones)

1. (Needs a second milestone to cross-validate.)
