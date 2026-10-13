# Roadmap: nesturbator

## Milestones

- ✅ **v1 NROM games with sound** — Phases 1–4.2 (shipped 2026-10-09) — [archive](milestones/v1-ROADMAP.md)
- ✅ **v2 Most of the library plays** — Phases 5–10 (shipped 2026-10-11) — [archive](milestones/v2-ROADMAP.md)
- ✅ **v3 Save states and rewind** — Phases 11–15 (shipped 2026-10-13) — [archive](milestones/v3-ROADMAP.md)

## Phases

<details>
<summary>✅ v1 NROM games with sound (Phases 1–4.2) — SHIPPED 2026-10-09</summary>

- [x] Phase 1: A test frame in RetroArch (12/12 plans) — completed 2026-10-03
- [x] Phase 2: The CPU matches the public vectors (11/11 plans) — completed 2026-10-06
- [x] Phase 3: A real game in RetroArch (15/15 plans) — completed 2026-10-09
- [x] Phase 4: Sound (4/4 plans) — completed 2026-10-09
- [x] Phase 4.1: Close gap: GAME-01 — initialize accepted iNES trainers (1/1 plan, INSERTED) — completed 2026-10-09
- [x] Phase 4.2: Close gap: SND-01 — deliver single-sample libretro audio (1/1 plan, INSERTED) — completed 2026-10-09

</details>

<details>
<summary>✅ v2 Most of the library plays (Phases 5–10) — SHIPPED 2026-10-11</summary>

- [x] Phase 5: Tune-up and v1 debt (8/8 plans) — completed 2026-10-10
- [x] Phase 6: Mapper seam and PPU fetch pipeline (2/2 plans) — completed 2026-10-10
- [x] Phase 7: UxROM, CNROM and AxROM (4/4 plans) — completed 2026-10-10
- [x] Phase 8: MMC1 and battery saves (8/8 plans) — completed 2026-10-10
- [x] Phase 9: MMC3 (12/12 plans) — completed 2026-10-10
- [x] Phase 10: Close-out and boot-to-play (7/7 plans) — completed 2026-10-11

</details>

<details>
<summary>✅ v3 Save states and rewind (Phases 11–15) — SHIPPED 2026-10-13</summary>

- [x] Phase 11: Tune-up and state readiness (4/4 plans) — completed 2026-10-11
- [x] Phase 12: Save states in the library (10/10 plans) — completed 2026-10-12
- [x] Phase 13: Rewind and run-ahead in RetroArch (8/8 plans) — completed 2026-10-12
- [x] Phase 14: Runner process contract (8/8 plans) — completed 2026-10-12
- [x] Phase 15: Speed and close-out (8/8 plans) — completed 2026-10-13

</details>

## Backlog

### Phase 999.2: Bump AccuracyCoin past 673ef550 (BACKLOG)

**Goal:** Re-baseline the AccuracyCoin ROM sha256, scoreboard and hashes at a newer upstream commit; upstream added tests and an OAM fix.
Compare: https://github.com/100thCoin/AccuracyCoin/compare/673ef550...main
