# Product roadmap: nesturbator

Reviewed: 2026-10-25 at v5 opening.
Direction revision: 3.
Active milestone: v5 "NMI, VBlank and sprite-0 timing" (RM-004), approved at new-milestone on 2026-10-25.

This is the rolling product direction. The execution contract remains in `.planning/ROADMAP.md`. Planning milestones v1, v2, v3 and v4 are distinct from package versions such as v0.1.10. No dates or future phase numbers are promised here.

## Horizons and commitment

| Horizon | Meaning | Planning depth |
|---|---|---|
| Immediate | Current approved milestone | Requirements, executable phases and automated acceptance |
| Near | Preferred next outcomes and a clearly identified alternative | Bounded outcome, evidence, prerequisites and promotion trigger |
| Mid | Useful following themes | Why, dependencies, decision trigger and acceptance direction |
| Long | Exploratory hardware and host capabilities | Value, feasibility questions and conditions for scoping |

Only `committed` items are current scope. Other statuses are `proposed`, `shipped`, `parked` and `retired`. Parked items need a return trigger; retired items need a reason; shipped items need milestone, release and acceptance evidence. Stable RM IDs are never recycled. Technical prerequisites and priority choices are recorded separately.

## Immediate

RM-004 is committed for v5, together with the recurring SEED-006 tune-up (AccuracyCoin JAM attribution and a run-ahead-under-JAM test, both carried v4 debt). Requirements are in [REQUIREMENTS.md](REQUIREMENTS.md); phases start at 19. The shipped v4 rows stay below as the record.

| ID | Status | Outcome | Milestone, release and acceptance evidence |
|---|---|---|---|
| RM-004 | committed | NMI/VBlank and sprite-0 timing match the console | v5. The 11 rows: `$2002 flag timing`, `NMI Control`, `NMI Timing`, `NMI Suppression`, `NMI at VBlank end`, `NMI disabled at VBlank`, `NMI Overlap BRK`, `NMI Overlap IRQ`, `Interrupt flag latency`, `Sprite 0 Hit behavior`, `Arbitrary Sprite zero` (10 fail, 1 timeout at `6bee919`, rechecked 2026-10-25). Exit: they pass at `6bee919` with no lost PASS, documented hash re-pins and state coverage for any new field. |
| RM-001 | shipped | Complete pinned NTSC conformance inventory and bounded baseline | v4 Phase 17; CONF-01–04 ([v4 requirements](milestones/v4-REQUIREMENTS.md)); 38/38 verification; 146 scored + 5 DRAW at `6bee919`: 101 pass, 42 fail, 2 not-run, 1 timeout. Release: lands with PR #55 in the release after v0.1.11. |
| RM-002 | shipped | Discriminating existing RetroArch progression/state evidence | v4 Phase 16; EVID-01–04; 22/22 verification; released in [v0.1.11](https://github.com/szTheory/nesturbator/releases/tag/v0.1.11) with 18 archives, checksums, attestations and six metadata copies checked. |
| RM-003 | shipped | Bounded accepted-program execution and lifecycle robustness | v4 Phase 18; ROB-01–04; 4/4 verification; six board families reach execution; Nightly 40150888217 green. Release: lands with PR #55 in the release after v0.1.11. |

Closeout record: [v4 audit](milestones/v4-MILESTONE-AUDIT.md) (tech_debt, 14/14 requirements). RM-001 and RM-003 were marked shipped at milestone close with publication pending; MILESTONES.md records the publication when it is observed.

## Near

RM-004 moved to Immediate at the v5 opening. After it, the DMA cluster (listed with RM-004's follow-ons below) is the preferred next accuracy scope, because DMC games corrupt controller reads when DMA steals cycles; RM-005 (PAL) stays the main alternative milestone behind it. This is priority ordering, not a claim that every NTSC edge case technically blocks PAL or another board.

| ID | Status | Outcome | Why and promotion trigger |
|---|---|---|---|
| RM-005 | proposed | PAL games run at their intended timing and pitch | Preferred next compatibility milestone when no broad NTSC correctness blocker remains; requires regional test evidence. |
| RM-006 | proposed | Dendy games run with a distinct, correct timing profile | Scope independently after regional infrastructure is proven and demand/evidence justify it. |

### RM-001: Complete NTSC conformance evidence

Sources: SEED-003 (baseline portion), Backlog 999.2, `tests/accuracy/scoreboard.txt`, `runner/main.c`, preparation CONFORMANCE.

Acceptance direction: preserve an old-pin comparison; migrate to a reviewed immutable upstream pin and tested name decoder; inventory every scored test and separate DRAW entries; retain raw outcomes, deadlines and pass/fail/skip/not-run/timeout classifications. Missing results cannot become passes. Explain every migration of a previously protected key. Gate regressions within the same pin; never compare scores with different denominators as if they were equal.

Delivered in v4: the canonical inventory is 146 scored rows plus five DRAW entries at `6bee919`, with 101 pass, 42 fail, 0 skip, 2 not-run and 1 timeout. All 17 old protected keys still pass, and every CI lane freshly protects the PASS set taken from protected main. The opening observation, kept for history, was 17 protected passing keys across pages 2, 14 and 17. Upstream `6bee919529289c555803b612b8c9e4f7eaaeeabb` documents 146 scored tests plus five DRAW entries and compresses test names. The new pin requires harness changes, not just a ROM replacement.

Would change priority if: a reproduced player/host defect or serious safety failure needs immediate repair. Fixing every accuracy failure is a separate scoped outcome.

### RM-002: Existing host evidence is discriminating

Sources: v3 audit and Phase 13 WR-01, WR-02, WR-03; existing `tests/retroarch/` and portable libretro host tests.

Acceptance direction: wrong, dropped and doubled frame sequences fail checks using absolute state anchors or a fixture that distinguishes the checked frames. Identical pictures cannot prove exact progress. Retry idempotent requests only with bounded, attributable completion. FRAMEADVANCE and PAUSE_TOGGLE require confirmation or fail/restart, never blind replay. Select fixture basenames rather than substrings of full paths. Correct the MMC3 description. Retain real-host artifacts and measure job budgets.

Excludes: broad old-debt cleanup, new host features and a six-platform real-RetroArch rollout. Portable six-platform parity remains credited.

### RM-003: Accepted-program execution and lifecycle robustness

Sources: SEED-004 (remaining robustness portion), existing loader/state fuzz targets and owned C test helpers.

Acceptance direction: a small owned target constructs bounded accepted board profiles and programs, exercises execution, input, reset, save/load, unload/reload, and compares deterministic continuations. Fixed corpus replays run in CI; finite sanitizer campaigns retain minimized crash/timeout reproducers and measured budgets. All supported board families reach execution. Ordinary guest loops and JAM have documented outcomes. Zero findings is finite evidence, not proof of universal safety.

Discovery gate: demonstrate additional execution paths and useful throughput compared with existing targets before committing a large campaign. If the target duplicates state fuzzing or requires a new architecture, split or park it explicitly. A confirmed reproducible safety failure blocks a clean safety claim and receives a repair or explicit scope decision; never silently defer it.

### RM-004 follow-ons, RM-005 and RM-006: Conditional next choice

After the full baseline, prioritize named game-relevant failures before obscure score points. The v4 baseline's other failing clusters stay as RM-004 follow-ons in this order, and each needs its own approval:
- DMA (8 rows): `DMA + $4015 Read`, `DMA Landing on Write`, `DMC DMA + OAM DMA`, `DMC DMA Bus Conflicts`, `Explicit DMA Abort`, `Implicit DMA Abort`, `Misaligned OAM DMA`, `INC $4014`. This is the strongest alternative to the preferred cluster, because DMC games corrupt controller reads when DMA steals cycles.
- Sprite evaluation/OAM: `Sprites On Scanline 0`, `Suddenly Resize Sprite`, `Stale Sprite Shift Regs`, `Frozen OAM2 Increment`, `Misaligned OAM2 Address`, the `$2004` rows, plus the two not-run rows `Misaligned OAM behavior` and `OAM Corruption`.
- PPU rendering reads: the `$2007` rows, `Rendering Flag Behavior`, `Stale BG Shift Registers`, `BG Serial In`, `ALE + Read`.
- CPU bus: open bus, internal data bus, implied dummy reads, `Hybrid Addresses`, `APU Register Activation`, `Controller Strobing`, and the five unstable `SH*` stores (last, since games rarely depend on them).

Each cluster should name at least one affected title, where one exists, before it is committed. Hardware fixes need owned reproducers, documented hardware behavior, external test evidence and save-state coverage if instance fields change. Intentional frame/audio hash changes are documented and re-pinned with their reason.

PAL requires CPU/PPU/APU/DMA timing, audio scheduling, video/emphasis, state identity, ROM admission and host AV metadata to agree. Dendy needs its own delayed-vblank and clock evidence. Source material exists in the regional preparation sections, but acceptance needs region-specific owned or permitted fixtures. The NTSC AccuracyCoin score cannot validate regional behavior.

## Mid

| ID | Status | Outcome | Promotion trigger and evidence |
|---|---|---|---|
| RM-007 | proposed | One additional cartridge board family works through all three deliverables | A named title/host need plus licensed validation. Consider 66, 206 or 9/10 individually; choose by useful coverage and test readiness, not stale dump percentages. Include state, persistence when applicable and host parity. |
| RM-008 | proposed | A concrete host workflow has sufficient capability reporting and integration evidence | A launcher/distribution requirement or reproducible platform defect. Start with one bounded preflight or Linux real-host pilot; avoid a speculative shared ABI and broad GUI matrix. |
| RM-009 | proposed | A measured target device meets frame/run-ahead budgets faster | A real performance budget and profile identify a bottleneck. Preserve correctness and determinism; benchmark target-device improvement alongside existing instruction-count gates. |

These are priority choices, not technical dependencies on perfect NTSC accuracy. Concrete demand may promote a mapper or host outcome earlier. Achievements or netplay need an actual consumer and acceptance session before joining RM-008's committed scope.

## Long

| ID | Status | Direction | What must be established before promotion |
|---|---|---|---|
| RM-010 | proposed | Selected Famicom expansion-audio boards | Choose one board; pair banking/IRQ behavior with owned integer audio, mixing/state continuity and permitted tests. Silent partial support is not completion. |
| RM-011 | proposed | MMC5 | Independent scope for PPU snooping, tile/attribute banking, ExRAM, IRQ, audio and states; do not model it as renamed MMC3. |
| RM-012 | proposed | Famicom Disk System | Host-supplied BIOS, lawful fixtures, disk-control contract, media state and persistence. No bundled BIOS. |
| RM-013 | proposed | Selected additional peripherals | Concrete player/host demand, input representation and timing tests. Device/window code stays in the host. |

## Recurring policy and historical seed disposition

SEED-006 is recurring: every milestone starts with only the measured CI, flaky-test or documentation repairs needed for that milestone. No promised speedup without a bottleneck and no sweeping cosmetic cleanup.

| Source | Direction disposition |
|---|---|
| SEED-001 | Common board families and battery saves delivered in v2; use archived milestone evidence. |
| SEED-002 | Save states, rewind/run-ahead and runner process contract delivered in v3. Actual netplay remains uncommitted. |
| SEED-003 | Partial: the measured baseline shipped in v4 (RM-001). The remaining failures go to RM-004 and its follow-on clusters; regions go to RM-005/RM-006. |
| SEED-004 | Partial: benchmarks/perf gates, state fuzz (v3) and execution/lifecycle robustness (v4, RM-003) shipped. RM-009 covers measured future optimization. |
| SEED-005 | Split future direction across cartridge, expansion-audio, MMC5, FDS and peripheral outcomes. |
| SEED-006 | Recurring; never globally marked complete. |
| Playability seed | Existing boot-to-play evidence is credited. Promote an additional integrated host-play outcome only when a new claim requires it. |

Seed files remain unchanged during new-milestone. This disposition ledger prevents stale dormant frontmatter from re-scoping delivered work. Backlog 999.2 is promoted into RM-001 / Phase 17 (CONF-01 through CONF-04); the execution roadmap records its disposition rather than leaving duplicate active backlog work.

## Unresolved debt carried from v4

Debt stays visible here until a milestone closes it. Item details are in the [v4 audit](milestones/v4-MILESTONE-AUDIT.md) and STATE.md Deferred Items.

- AccuracyCoin observe records a JAM only as a generic frame failure, and no AccuracyCoin control covers JAM. Fold this into the next accuracy milestone's tune-up.
- No explicit run-ahead-under-JAM test exists in `libretro.runahead`.
- Real-campaign minimization is proven only by controls; no real campaign finding has been reduced yet.
- Nightly `vectors-full` failed in run 39993108154. It is not a Release gate.
- Local macOS `nofp` reports `stack_chk_fail`/`stack_chk_guard` from the Apple toolchain. The Linux no-float lanes are the acceptance environment.
- Two Phase 16 debug sessions (`phase16-cleanup-fixture`, `phase16-interruption`) were acknowledged at close, not resolved.

## Refresh procedure

At each new-milestone and complete-milestone step, the agent reads this file, project state, the last audit, seeds and release evidence. It reconciles shipped/partial/parked items, refreshes each promotion trigger, records changed ordering and presents a preferred next milestone with alternatives. This is part of the invoked step, with no extra lifecycle command or manual reminder.

After scope approval, promote selected IDs to Immediate/committed and link requirements/phases. At closeout, mark only evidenced outcomes shipped, keep real debt visible, refresh the remaining horizons and archive the agreed direction with that milestone. A newly reconstructed boundary snapshot must say when it was reconstructed. Phase discoveries change the ledger only when they affect direction, prerequisites or acceptance.

Keep the file small. Use existing Markdown and git; no roadmap service or parser dependency. A structural validator is optional if omissions recur; it cannot prove that priorities were reconsidered. Small owned helpers and justified local duplication are preferable to extra dependency trees, within the repository's ownership and clean-room rules.

## Review log

- 2026-10-13, v4 opening: promoted RM-001/RM-002/RM-003 from the approved proposal to Immediate/committed. Complete measurement and stronger execution/host evidence precede unnamed accuracy repairs. Retained PAL as the preferred compatibility alternative, with Dendy separately scoped. Deferred larger hardware and host work until evidence or demand justifies it. Historical delivered seed outcomes remain credited; no seed file was changed.
- 2026-10-24, v4 closeout: marked RM-001, RM-002 and RM-003 shipped with evidence. Publication of RM-001 and RM-003 waits on PR #55 and the release after it. The measured baseline met RM-004's trigger, so the order changed: RM-004 is now the preferred next milestone, narrowed to the 11-row NMI/VBlank and sprite-0 cluster. DMA is the strongest alternative cluster. RM-005 (PAL) remains the main alternative milestone and moves behind RM-004, because the baseline shows broad NTSC timing gaps that would carry into PAL timing. Mid and Long items are unchanged; no new demand or performance budget appeared. Carried v4 debt is listed above. No seed file was changed.
- 2026-10-25, v5 opening: promoted RM-004 to Immediate/committed with the SEED-006 tune-up (JAM attribution, run-ahead-under-JAM). The owner chose it over adding the DMA cluster, doing DMA first, or PAL. Ordering otherwise unchanged: DMA is the preferred next cluster, then PAL (RM-005); Mid and Long items are unchanged. Release PR #56 (0.1.12), which publishes RM-001 and RM-003, was still open at the opening. No seed file was changed.

## Why this shape

A separate product ledger preserves direction while GSD rewrites and archives execution phases. Seeds alone lack current ordering; mixing speculative horizons into execution phase headings invites accidental promotion. GitHub distinguishes exploratory direction from work selected for delivery, and Rust separates broad roadmaps from concrete goals. These practices inform this project's smaller readiness horizons rather than importing dates or governance machinery.

Primary sources checked 2026-10-13: [AccuracyCoin at the proposed pin](https://github.com/100thCoin/AccuracyCoin/blob/6bee919529289c555803b612b8c9e4f7eaaeeabb/README.md), [GitHub roadmap](https://github.com/github/roadmap), [Rust roadmaps](https://goals.rust-lang.org/2026/roadmaps.html), [Rust goal format](https://goals.rust-lang.org/about/goal-format.html), [RetroArch network interface](https://docs.libretro.com/development/retroarch/network-control-interface/), [LLVM libFuzzer](https://llvm.org/docs/LibFuzzer.html). Hardware details in the preparation files preserve their source tiers; NESdev is a curated community reference, not official console documentation.
