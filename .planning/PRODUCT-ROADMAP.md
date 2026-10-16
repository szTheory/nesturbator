# Product roadmap: nesturbator

Reviewed: 2026-10-13 at v4 opening; scope approved by the user.
Direction revision: 1.
Active milestone: v4, Measured NTSC accuracy and robustness.

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

Approved v4 scope: Measured NTSC accuracy and robustness. No implementation is complete yet.

| ID | Status | Outcome | Execution links |
|---|---|---|---|
| RM-001 | committed | Complete pinned NTSC conformance inventory and bounded baseline | Phase 17; CONF-01 through CONF-04 in REQUIREMENTS.md |
| RM-002 | committed | Discriminating existing RetroArch progression/state evidence | Phase 16; EVID-01 through EVID-04 in REQUIREMENTS.md |
| RM-003 | committed | Bounded accepted-program execution and lifecycle robustness | Phase 18; ROB-01 through ROB-04 in REQUIREMENTS.md |

## Near

Preferred next scope depends on v4's measured NTSC baseline. Material gameplay/timing gaps promote RM-004; otherwise RM-005 is the preferred next compatibility milestone. This is priority ordering, not a claim that every NTSC edge case technically blocks PAL or another board.

| ID | Status | Outcome | Why and promotion trigger |
|---|---|---|---|
| RM-004 | proposed | A named, gameplay-relevant NTSC accuracy correction cluster | Select exact DMA, interrupt or PPU failures and exit tests from the full baseline before committing. |
| RM-005 | proposed | PAL games run at their intended timing and pitch | Preferred next compatibility milestone when no broad NTSC correctness blocker remains; requires regional test evidence. |
| RM-006 | proposed | Dendy games run with a distinct, correct timing profile | Scope independently after regional infrastructure is proven and demand/evidence justify it. |

### RM-001: Complete NTSC conformance evidence

Sources: SEED-003 (baseline portion), Backlog 999.2, `tests/accuracy/scoreboard.txt`, `runner/main.c`, preparation CONFORMANCE.

Acceptance direction: preserve an old-pin comparison; migrate to a reviewed immutable upstream pin and tested name decoder; inventory every scored test and separate DRAW entries; retain raw outcomes, deadlines and pass/fail/skip/not-run/timeout classifications. Missing results cannot become passes. Explain every migration of a previously protected key. Gate regressions within the same pin; never compare scores with different denominators as if they were equal.

Current observation: 17 protected passing keys across pages 2, 14 and 17. This is neither a full-suite score nor proof that unmeasured tests fail. Upstream `6bee919529289c555803b612b8c9e4f7eaaeeabb` documents 146 scored tests plus five DRAW entries and compresses test names. The new pin requires harness changes, not just a ROM replacement.

Would change priority if: a reproduced player/host defect or serious safety failure needs immediate repair. Fixing every accuracy failure is a separate scoped outcome.

### RM-002: Existing host evidence is discriminating

Sources: v3 audit and Phase 13 WR-01, WR-02, WR-03; existing `tests/retroarch/` and portable libretro host tests.

Acceptance direction: wrong, dropped and doubled frame sequences fail checks using absolute state anchors or a fixture that distinguishes the checked frames. Identical pictures cannot prove exact progress. Retry idempotent requests only with bounded, attributable completion. FRAMEADVANCE and PAUSE_TOGGLE require confirmation or fail/restart, never blind replay. Select fixture basenames rather than substrings of full paths. Correct the MMC3 description. Retain real-host artifacts and measure job budgets.

Excludes: broad old-debt cleanup, new host features and a six-platform real-RetroArch rollout. Portable six-platform parity remains credited.

### RM-003: Accepted-program execution and lifecycle robustness

Sources: SEED-004 (remaining robustness portion), existing loader/state fuzz targets and owned C test helpers.

Acceptance direction: a small owned target constructs bounded accepted board profiles and programs, exercises execution, input, reset, save/load, unload/reload, and compares deterministic continuations. Fixed corpus replays run in CI; finite sanitizer campaigns retain minimized crash/timeout reproducers and measured budgets. All supported board families reach execution. Ordinary guest loops and JAM have documented outcomes. Zero findings is finite evidence, not proof of universal safety.

Discovery gate: demonstrate additional execution paths and useful throughput compared with existing targets before committing a large campaign. If the target duplicates state fuzzing or requires a new architecture, split or park it explicitly. A confirmed reproducible safety failure blocks a clean safety claim and receives a repair or explicit scope decision; never silently defer it.

### RM-004, RM-005 and RM-006: Conditional next choice

After the full baseline, prioritize named game-relevant failures before obscure score points. Hardware fixes need owned reproducers, documented hardware behavior, external test evidence and save-state coverage if instance fields change. Intentional frame/audio hash changes are documented and re-pinned with their reason.

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
| SEED-003 | Partial direction: RM-001 and RM-004 cover measured accuracy; RM-005/RM-006 cover regions. |
| SEED-004 | Partial direction: benchmarks/perf gates and state fuzz shipped; RM-003 covers remaining execution robustness and RM-009 measured future optimization. |
| SEED-005 | Split future direction across cartridge, expansion-audio, MMC5, FDS and peripheral outcomes. |
| SEED-006 | Recurring; never globally marked complete. |
| Playability seed | Existing boot-to-play evidence is credited. Promote an additional integrated host-play outcome only when a new claim requires it. |

Seed files remain unchanged during new-milestone. This disposition ledger prevents stale dormant frontmatter from re-scoping delivered work. Backlog 999.2 is promoted into RM-001 / Phase 17 (CONF-01 through CONF-04); the execution roadmap records its disposition rather than leaving duplicate active backlog work.

## Refresh procedure

At each new-milestone and complete-milestone step, the agent reads this file, project state, the last audit, seeds and release evidence. It reconciles shipped/partial/parked items, refreshes each promotion trigger, records changed ordering and presents a preferred next milestone with alternatives. This is part of the invoked step, with no extra lifecycle command or manual reminder.

After scope approval, promote selected IDs to Immediate/committed and link requirements/phases. At closeout, mark only evidenced outcomes shipped, keep real debt visible, refresh the remaining horizons and archive the agreed direction with that milestone. A newly reconstructed boundary snapshot must say when it was reconstructed. Phase discoveries change the ledger only when they affect direction, prerequisites or acceptance.

Keep the file small. Use existing Markdown and git; no roadmap service or parser dependency. A structural validator is optional if omissions recur; it cannot prove that priorities were reconsidered. Small owned helpers and justified local duplication are preferable to extra dependency trees, within the repository's ownership and clean-room rules.

## Review log

- 2026-10-13, v4 opening: promoted RM-001/RM-002/RM-003 from the approved proposal to Immediate/committed. Complete measurement and stronger execution/host evidence precede unnamed accuracy repairs. Retained PAL as the preferred compatibility alternative, with Dendy separately scoped. Deferred larger hardware and host work until evidence or demand justifies it. Historical delivered seed outcomes remain credited; no seed file was changed.

## Why this shape

A separate product ledger preserves direction while GSD rewrites and archives execution phases. Seeds alone lack current ordering; mixing speculative horizons into execution phase headings invites accidental promotion. GitHub distinguishes exploratory direction from work selected for delivery, and Rust separates broad roadmaps from concrete goals. These practices inform this project's smaller readiness horizons rather than importing dates or governance machinery.

Primary sources checked 2026-10-13: [AccuracyCoin at the proposed pin](https://github.com/100thCoin/AccuracyCoin/blob/6bee919529289c555803b612b8c9e4f7eaaeeabb/README.md), [GitHub roadmap](https://github.com/github/roadmap), [Rust roadmaps](https://goals.rust-lang.org/2026/roadmaps.html), [Rust goal format](https://goals.rust-lang.org/about/goal-format.html), [RetroArch network interface](https://docs.libretro.com/development/retroarch/network-control-interface/), [LLVM libFuzzer](https://llvm.org/docs/LibFuzzer.html). Hardware details in the preparation files preserve their source tiers; NESdev is a curated community reference, not official console documentation.
