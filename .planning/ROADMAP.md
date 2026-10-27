# Roadmap: nesturbator

## Milestones

- ✅ **v1 NROM games with sound** — Phases 1–4.2 (shipped 2026-10-09) — [archive](milestones/v1-ROADMAP.md)
- ✅ **v2 Most of the library plays** — Phases 5–10 (shipped 2026-10-11) — [archive](milestones/v2-ROADMAP.md)
- ✅ **v3 Save states and rewind** — Phases 11–15 (shipped 2026-10-13) — [archive](milestones/v3-ROADMAP.md)
- ✅ **v4 Measured NTSC accuracy and robustness** — Phases 16–18 (shipped 2026-10-24) — [archive](milestones/v4-ROADMAP.md)
- 🚧 **v5 NMI, VBlank and sprite-0 timing** — Phases 19–21 (scope approved 2026-10-25)

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

<details>
<summary>✅ v4 Measured NTSC accuracy and robustness (Phases 16–18) — SHIPPED 2026-10-24</summary>

- [x] Phase 16: Host evidence and roadmap (13/13 live plans) — completed 2026-10-23
- [x] Phase 17: Complete pinned conformance baseline (4/4 plans) — completed 2026-10-24
- [x] Phase 18: Execution and lifecycle robustness (4/4 plans) — completed 2026-10-24

</details>

### v5 — NMI, VBlank and sprite-0 timing

A game's frame loop and status-bar splits time out exactly as they do on an NTSC console. Eleven AccuracyCoin rows at pin `6bee919` move to pass; the 101 rows that pass today keep passing. Direction: RM-004 in [PRODUCT-ROADMAP.md](PRODUCT-ROADMAP.md).

- [x] **Phase 19: JAM reporting and sprite-0 hit** - AccuracyCoin names a guest JAM, and sprite-0 hit lands on the console's line (completed 2026-10-25)
- [ ] **Phase 20: NMI line and VBlank flag** - `$2002` reads and `$2000` writes around VBlank give the console's flag and NMIs, and save states carry the new latches
- [ ] **Phase 21: Interrupt latency and hijack** - IRQs start on the console's instruction and an NMI takes over BRK or IRQ as on the console

## Phase Details

Each phase merges by pull request into a green `main` and is released automatically. Behaviour changes land with tests in `cmake --workflow --preset ci`, README and `nesturbator.h` comments. One NMI-line and interrupt-poll model serves every row; no row or title gets a special case, the 1800-frame AccuracyCoin cap stays, and no blargg ROM is committed.

### Phase 19: JAM reporting and sprite-0 hit

**Goal**: A developer reading an AccuracyCoin run sees a guest JAM named as a JAM, and a game's sprite-0 status-bar split lands on the console's line.
**Depends on**: Phase 18 (JAM stop contract, AccuracyCoin baseline at `6bee919`)
**Requirements**: JAM-01, JAM-02, SPR-01, SPR-02
**Success Criteria** (what must be TRUE):
  1. A developer running `cmake --workflow --preset ci` sees an AccuracyCoin run that stops on a guest JAM reported as `guest-jam`, and a control that drops the JAM attribution fails.
  2. `libretro.runahead` shows that run-ahead across a JAM gives the same picture and silence as running without run-ahead.
  3. The `Sprite 0 Hit behavior` and `Arbitrary Sprite zero` rows report pass in `accuracy.scoreboard`, within the 1800-frame cap, and owned `ppu.sprites` tests show the hit on its dot and the first sprite evaluated acting as sprite 0 at any OAM index.
  4. Every row that passes on `main` today still passes in `accuracy.scoreboard.no_lost_pass` on every CI lane; any frame hash the sprite-0 fix moves is re-pinned in the same change with its reason.

**Plans**: 6/6 plans complete
**Wave 1**
- [x] 19-01-PLAN.md — AccuracyCoin names a guest JAM in the execution stage (`guest-jam`, `jam_frame`, `accuracycoin.jam`)

**Wave 2** *(blocked on Wave 1 completion)*
- [x] 19-02-PLAN.md — guest JAM named in earlier stages and strict mode; tracer pin, README, collector and attribution-dropped controls

**Wave 3** *(blocked on Wave 2 completion)*
- [x] 19-03-PLAN.md — run-ahead across a JAM keeps the frozen picture and silence (adapter cache tag with timeline identity, `libretro.runahead` JAM image)

**Wave 4** *(blocked on Wave 3 completion)*
- [x] 19-04-PLAN.md — owned sprite tests first, then the OAMADDR reset with its campaign diff

**Wave 5** *(blocked on Wave 4 completion)*
- [x] 19-05-PLAN.md — evaluation from OAMADDR with the dot-66 identity, driver re-target, behaviour revision 7

**Wave 6** *(blocked on Wave 5 completion)*
- [x] 19-06-PLAN.md — re-record scoreboard and activation baseline, D-11 guard, correct the research record

**Canonical refs**: [CONFORMANCE.md](preparation/CONFORMANCE.md), [NES-HARDWARE-PPU-CARTRIDGE.md](preparation/NES-HARDWARE-PPU-CARTRIDGE.md), [LIBRETRO-AND-RUNNER.md](preparation/LIBRETRO-AND-RUNNER.md), [ENGINEERING.md](preparation/ENGINEERING.md); [research/FEATURES.md](research/FEATURES.md) (G5, JAM), [research/PITFALLS.md](research/PITFALLS.md) (#8, #9, #15, #16), [research/SUMMARY.md](research/SUMMARY.md) (steps 0–2, 7)

Note: the basic hit already worked; the `0x06` sprite rows were guest re-run artifacts of a stale OAMADDR (never cleared during rendering, evaluation started at slot 0), fixed in behaviour revision 7 (D-01/D-02). Sprite-0 lands in its own pull request, before the NMI model changes, so a moved row is attributable.

### Phase 20: NMI line and VBlank flag

**Goal**: A game that reads `$2002` or toggles NMI around the start or end of VBlank gets the console's flag value and exactly the console's NMIs, and its save states hold through that race.
**Depends on**: Phase 19 (sprite-0 settled so NMI regressions are attributable)
**Requirements**: NMI-01, NMI-02, KEEP-02, KEEP-03
**Success Criteria** (what must be TRUE):
  1. The `$2002 flag timing`, `NMI Timing`, `NMI Suppression`, `NMI Control`, `NMI at VBlank end` and `NMI disabled at VBlank` rows report pass in `accuracy.scoreboard`, and owned `tests/ppu` reproducers for the `$2002`/`$2000` dots around VBlank start and end, written before the fix with expected values from NESdev and AccuracyCoin error codes, pass in `ci`, including the extra NMI from a `$00` then `$80` write that Bases Loaded II relies on.
  2. A developer running `cmake --workflow --preset ci` on any of the six platforms gets identical frame and audio hashes; each hash that moved was re-pinned in the change that moved it, with its reason beside it.
  3. A state saved on the VBlank-set dot, the VBlank-clear dot or between an NMI edge and its poll loads back to the same run in `core.state_fork.*`, `runner.state.*` and `libretro.runahead`; `state.format_pin` shows format version 2, and a format 1 state is refused with the existing version error.
  4. Every row that passes on `main` today, including Phase 19's sprite-0 rows, still passes in `accuracy.scoreboard.no_lost_pass` on every CI lane.

**Plans**: 9/10 plans executed — 6 historical plans executed; 4 additive gap-closure plans pending

Plans:
**Wave 1**
- [x] 20-01-PLAN.md — owned absolute-dot, real-bus and guest NMI reproducers with exact pinned table oracles

**Wave 2** *(blocked on Wave 1 completion)*
- [x] 20-02-PLAN.md — measured split status/line capture with coherent state format 2 and behavior revision 8

**Wave 3** *(blocked on Wave 2 completion)*
- [x] 20-03-PLAN.md — instruction-specific NMI polling, complete-table alignment and causal evidence

**Wave 4** *(blocked on Wave 3 completion)*
- [x] 20-04-PLAN.md — exact enabled-NMI set/clear/edge state continuation and rejection controls

**Wave 5** *(blocked on Wave 4 completion)*
- [x] 20-05-PLAN.md — NMI-sensitive core, runner and libretro rewind/run-ahead replay

**Wave 6** *(blocked on Wave 5 completion)*
- [x] 20-06-PLAN.md — six-target gate, final baseline/hash provenance and local/hosted evidence ledger

**Wave 7** *(gap closure; consumes completed Wave 6)*
- [x] 20-07-PLAN.md — changing-sprite split-decoder witness and WR-01/WR-02 contract corrections

**Wave 8** *(gap closure; blocked on Wave 7)*
- [x] 20-08-PLAN.md — equal-tick full-key controls and combined frame-zero/empty-PCM/nonzero-history replay

**Wave 9** *(gap closure; blocked on Wave 8)*
- [x] 20-09-PLAN.md — locally tested hosted collector, artifact retention and delivery preparation with ordinary complete summary

**Wave 10** *(gap closure; blocked on completed Wave 9 and separately authorized delivery)*
- [ ] 20-10-PLAN.md — read-only real hosted evidence collection after a first-task delivery precondition, then immutable-source measurement confirmation

Local gap execution can complete and summarize 20-09 preparation. Actual source-bound hosted results remain an external delivery prerequisite for 20-10, checked before its first task mutates artifacts or commits. Missing results leave 20-10 with zero task commits and KEEP-02/KEEP-03 and Phase 20 incomplete; these plans do not authorize push, PR, workflow dispatch or merge.

**Canonical refs**: [NES-HARDWARE-PPU-CARTRIDGE.md](preparation/NES-HARDWARE-PPU-CARTRIDGE.md), [NES-HARDWARE-CPU-APU.md](preparation/NES-HARDWARE-CPU-APU.md), [ARCHITECTURE.md](preparation/ARCHITECTURE.md), [DECISIONS.md](preparation/DECISIONS.md), [CONFORMANCE.md](preparation/CONFORMANCE.md); [research/ARCHITECTURE.md](research/ARCHITECTURE.md) (section 1, state), [research/FEATURES.md](research/FEATURES.md) (G1, G2), [research/PITFALLS.md](research/PITFALLS.md) (#1, #2, #3, #6, #7, #10, #12), [research/SUMMARY.md](research/SUMMARY.md) (steps 3–4)

Note: the NMI line and its edge latch come before the penultimate-cycle poll, which reads that latch; both may share one pull request if the reproducers check each step. Test whether AccuracyCoin needs a CPU/PPU alignment constant before adding one.

### Phase 21: Interrupt latency and hijack

**Goal**: A game's IRQ starts on the same instruction as on the console, and an NMI arriving during BRK or an IRQ takes over its vector as on the console.
**Depends on**: Phase 20 (NMI edge latch and poll)
**Requirements**: INT-01, INT-02, KEEP-01
**Success Criteria** (what must be TRUE):
  1. The `Interrupt flag latency`, `NMI Overlap BRK` and `NMI Overlap IRQ` rows report pass in `accuracy.scoreboard`, and owned `tests/cpu` reproducers for the CLI, SEI, PLP, RTI and branch polls and for NMI at each cycle of BRK and IRQ, written before the fix with expected values from NESdev and AccuracyCoin error codes, pass in `ci`.
  2. All eleven v5 rows report pass at `6bee919` and every row that passed before v5 still passes in `accuracy.scoreboard.no_lost_pass` on every CI lane.
  3. `cpu.vectors`, the nightly blargg MMC3 lane, Holy Mapperel and `runner.boot_to_play` stay green with the shared NMI, IRQ and BRK entry in place.
  4. A state saved in the middle of a BRK, IRQ or NMI sequence loads back to the same run in `core.state_fork.*` and `libretro.runahead`, and moved hashes stay identical on all six platforms.

**Plans**: TBD
**Canonical refs**: [NES-HARDWARE-CPU-APU.md](preparation/NES-HARDWARE-CPU-APU.md), [ARCHITECTURE.md](preparation/ARCHITECTURE.md), [CONFORMANCE.md](preparation/CONFORMANCE.md), [ENGINEERING.md](preparation/ENGINEERING.md); [research/ARCHITECTURE.md](research/ARCHITECTURE.md) (shared `interrupt_entry`), [research/FEATURES.md](research/FEATURES.md) (G3, G4), [research/PITFALLS.md](research/PITFALLS.md) (#4, #5, #6, #11), [research/SUMMARY.md](research/SUMMARY.md) (steps 5, 6, 8)

## Progress

**Execution order**: 19 → 20 → 21.

| Phase | Milestone | Plans Complete | Status | Completed |
|-------|-----------|----------------|--------|-----------|
| 19. JAM reporting and sprite-0 hit | v5 | 6/6 | Complete    | 2026-10-25 |
| 20. NMI line and VBlank flag | v5 | 9/10 | In Progress | - |
| 21. Interrupt latency and hijack | v5 | 0/TBD | Not started | - |

Future direction and promotion triggers are in [PRODUCT-ROADMAP.md](PRODUCT-ROADMAP.md).

## Backlog disposition

2026-10-13: Backlog 999.2, "Bump AccuracyCoin past 673ef550", is promoted into [Phase 17](milestones/v4-ROADMAP.md#phase-17-complete-pinned-conformance-baseline), CONF-01 through CONF-04 and [RM-001](PRODUCT-ROADMAP.md#rm-001-complete-ntsc-conformance-evidence). Its pin/scoreboard/hash comparison is included there; no duplicate active backlog remains. Other future outcomes remain proposed in PRODUCT-ROADMAP.md.
