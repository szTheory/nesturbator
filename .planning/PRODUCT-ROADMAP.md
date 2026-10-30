# Product roadmap: nesturbator

Reviewed: 2026-10-30 during next-milestone discovery, after v5 closeout.
Direction revision: 6.
Active milestone: none. v5 "NMI, VBlank and sprite-0 timing" (RM-004) is complete; the next scope remains proposed.

This is the rolling product direction. The execution contract remains in `.planning/ROADMAP.md`. Planning milestones v1, v2, v3, v4 and v5 are distinct from package versions such as v0.1.10. No dates or future phase numbers are promised here.

## Horizons and commitment

| Horizon | Meaning | Planning depth |
|---|---|---|
| Immediate | Current approved milestone | Requirements, executable phases and automated acceptance |
| Near | Preferred next outcomes and a clearly identified alternative | Bounded outcome, evidence, prerequisites and promotion trigger |
| Mid | Useful following themes | Why, dependencies, decision trigger and acceptance direction |
| Long | Exploratory hardware and host capabilities | Value, feasibility questions and conditions for scoping |

Only `committed` items are current scope. Other statuses are `proposed`, `shipped`, `parked` and `retired`. Parked items need a return trigger; retired items need a reason; shipped items need milestone, release and acceptance evidence. Stable RM IDs are never recycled. Technical prerequisites and priority choices are recorded separately.

## Immediate

No new outcome is committed. RM-004 and the v5 SEED-006 tune-up shipped through merged PRs #57, #59 and #61. All eleven requirements are satisfied; the final campaign is 119 PASS / 27 FAIL over 146 terminal scored rows, plus five inventory-only DRAW entries. The remaining failures are separate proposed work. Requirements and phase details are archived in [v5 requirements](milestones/v5-REQUIREMENTS.md) and [v5 roadmap](milestones/v5-ROADMAP.md).

The next-milestone discussion now includes the owner's request for ordinary game trials, a usable README and Playstead integration lessons. The preferred proposal is bounded player-ready NES delivery (RM-014), with an upstream Playstead handoff and separate consumer acceptance (RM-008). It has not been approved as execution scope. See the [cross-project synthesis](research/milestone-v6-discovery/SYNTHESIS.md).

| ID | Status | Outcome | Milestone, release and acceptance evidence |
|---|---|---|---|
| RM-004 | shipped | NMI/VBlank, interrupt acceptance/takeover and sprite-0 timing satisfy the eleven-row contract | v5 Phases 19–21; 11/11 requirements, three current passed verifications, all eleven exact targets and 114 starting PASS keys on six lanes; ABI 1, state format 3, behavior revision 9. PR #61 merged. Package publication remains pending in release PR #56 (0.1.12); see [v5 closeout](milestones/v5-CLOSEOUT.md). |
| RM-001 | shipped | Complete pinned NTSC conformance inventory and bounded baseline | v4 Phase 17; CONF-01–04 ([v4 requirements](milestones/v4-REQUIREMENTS.md)); 38/38 verification; 146 scored + 5 DRAW at `6bee919`: 101 pass, 42 fail, 2 not-run, 1 timeout. PR #55 merged; package publication remains pending in release PR #56 (0.1.12), checked at v5 closeout. |
| RM-002 | shipped | Discriminating existing RetroArch progression/state evidence | v4 Phase 16; EVID-01–04; 22/22 verification; released in [v0.1.11](https://github.com/szTheory/nesturbator/releases/tag/v0.1.11) with 18 archives, checksums, attestations and six metadata copies checked. |
| RM-003 | shipped | Bounded accepted-program execution and lifecycle robustness | v4 Phase 18; ROB-01–04; 4/4 verification; six board families reach execution; Nightly 40150888217 green. PR #55 merged; package publication remains pending in release PR #56 (0.1.12), checked at v5 closeout. |

Closeout record: [v4 audit](milestones/v4-MILESTONE-AUDIT.md) (tech_debt, 14/14 requirements). RM-001 and RM-003 remain milestone-complete with package publication pending. PR #56 is still open at v5 closeout; no v0.1.12 publication is claimed.

## Near

Player-ready delivery is now the preferred next milestone proposal: make the existing NES core easy to install and play, collect voluntary title-specific feedback, and prepare its actual launcher boundary. This changes the priority because of new owner demand, not because conformance work lost value. Actual Playstead integration has its own consumer-owned acceptance and must not be credited from a handoff alone. The remaining DMA cluster stays the preferred following accuracy scope, with a bounded collector tune-up before relying on new collected evidence. At the unchanged pin, five of the formerly proposed eight DMA rows still fail; the other three already pass and must be protected. An observed title blocker can change ordering. PAL (RM-005) remains the regional alternative; maintenance-only cleanup is the smaller alternative. No perfect NTSC score technically blocks regional or consumer work.

The owner now prefers the Steam Deck/ES-DE/RetroArch daily-play loop as a fast source of feedback before deeper Playstead work. Two Mac smoke reports, a visible commercial Deck game with user-confirmed Start in the direct launch, and repaired ES-DE startup provide partial local signal, not full game/performance/save qualification. The real collection launch exposed another controller-routing gap under Steam Input; after the launch-context correction, the owner relaunched Mario 3 from that collection and confirmed the controller works. This establishes that session's frontend-to-game input path, while persistence, reconnect/fallback behavior and sustained performance remain unqualified. The normal library is reused through a reference-only collection and switchable per-game core choices. Daily use also calls for an explicit future state-format/behavior compatibility and upgrade-rollback policy, without assuming package 1.0 creates one automatically. [Playtesting lessons](playtesting/LESSONS.md) preserve this intent without private device details. The owner then explicitly authorized an expanded mapper/edge-case queue: 26 normal-library references are prepared, with feedback on 22 titles and four without a Deck report; see the [ordered queue](playtesting/DECK-QUEUE.md). The owner now prioritizes sourced difficult scenes, clear symptoms and efficient access; the [scene playlist](playtesting/SCENE-PLAYLIST.md) captures that next test direction and current cheat/checkpoint limits. This supplies setup and directional evidence, not universal mapper compatibility. New named visual findings now justify bounded native/reference reproduction before selecting a broad accuracy cluster. Private checkpoint/replay/capture prototypes are operational; small owned independent-oracle regressions and original-game reference cards are the preferred quality loop. Duplicate controller-port routing was corrected, while after-fix MM3 slowdown and visual findings remain unresolved. A Shatterhand line drifting downward and wrapping to the top prompted a backed-up per-core adaptive-VSync-off comparison; its activation and result are pending, so host presentation must be excluded before classifying those seams as core failures. This is evidence for prioritization, not approval of native fixes or another campaign. The README and formal next-milestone scope remain pending; this device setup does not itself complete RM-014 or RM-008.

| ID | Status | Outcome | Why and promotion trigger |
|---|---|---|---|
| RM-014 | proposed | A player can install the identified NES core, load supported content, understand controls/progress/limits and give useful feedback | New owner request after v5; local Mac/Deck smoke and ES-DE setup supply partial signal. Promote on scope approval for short current player docs, preserved executable doc/release contracts, and demonstrated daily-play/host gaps. Prefer existing-library feedback before deeper Playstead work; preserve switchable cores and normal progress storage. No universal compatibility or mandatory owner sign-off. |
| RM-008 | proposed | A concrete Playstead NES workflow has sufficient capability reporting and integration evidence | Moved from Mid on named consumer demand. First supply the upstream contract using released app/core identity; actual support requires a Playstead-owned adapter/readiness/input/exit/battery path and paired production acceptance. A report, runner result or RetroArch-only test cannot close this row. Native hosting/managed Continue need measured external-route friction or a supported state-control requirement. |
| RM-005 | proposed | PAL games run at their intended timing and pitch | Main alternative to remaining DMA work. Promote on approved regional demand and a bounded CPU/PPU/APU/DMA, AV metadata and state-identity test plan with permitted regional fixtures; NTSC score is insufficient. |
| RM-006 | proposed | Dendy games run with a distinct, correct timing profile | Keep proposed. Scope independently after regional infrastructure is proven and Dendy-specific delayed-VBlank/clock evidence and demand justify it; do not inherit PAL acceptance. |

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

### Remaining accuracy clusters, RM-005 and RM-006: Conditional next choice

After the full baseline, prioritize named game-relevant failures before obscure score points. The current scoreboard keeps these remaining clusters proposed in this order; each needs its own approval:
- DMA: five rows still fail — `DMA Landing on Write`, `DMC DMA + OAM DMA`, `DMC DMA Bus Conflicts`, `Explicit DMA Abort`, `Implicit DMA Abort`. Three former candidates now pass — `DMA + $4015 Read`, `Misaligned OAM DMA`, `INC $4014` — and become regression guards. The bounded Phase 21 DMC retry/open-bus prerequisite did not complete this cluster. Promote only after fresh measurement, at least one affected-title connection where available, owned bus-cycle oracles, state continuation and regression/performance budgets are scoped. Its game relevance keeps it ahead of PAL; the eight-row list from v4 is no longer eight missing outcomes.
- Sprite evaluation/OAM: `Sprites On Scanline 0`, `Stale Sprite Shift Regs`, `Frozen OAM2 Increment`, `Misaligned OAM2 Address`, the `$2004` rows, `Misaligned OAM behavior` and `OAM Corruption`. Both previously not-run rows now terminate as FAIL; `Suddenly Resize Sprite` already passes.
- PPU rendering reads: `$2007 Stress Test`, `Stale BG Shift Registers`, `BG Serial In`, `ALE + Read`. Other `$2007` rows and `Rendering Flag Behavior` already pass.
- CPU bus: open bus, internal data bus, `Hybrid Addresses`, `APU Register Activation`, `Controller Strobing`, and the five unstable `SH*` stores (last, since games rarely depend on them).

Each cluster should name at least one affected title, where one exists, before it is committed. Hardware fixes need owned reproducers, documented hardware behavior, external test evidence and save-state coverage if instance fields change. Intentional frame/audio hash changes are documented and re-pinned with their reason.

PAL requires CPU/PPU/APU/DMA timing, audio scheduling, video/emphasis, state identity, ROM admission and host AV metadata to agree. Dendy needs its own delayed-vblank and clock evidence. Source material exists in the regional preparation sections, but acceptance needs region-specific owned or permitted fixtures. The NTSC AccuracyCoin score cannot validate regional behavior.

## Mid

| ID | Status | Outcome | Promotion trigger and evidence |
|---|---|---|---|
| RM-007 | proposed | One additional cartridge board family works through all three deliverables | Still proposed: no new title/host demand appeared in v5. Promote on a named title/host need plus licensed validation. Consider 66, 206 or 9/10 individually; choose by useful coverage and test readiness, not stale dump percentages. Include state, persistence when applicable and host parity. |
| RM-009 | proposed | A measured target device meets frame/run-ahead budgets faster | Still proposed: v5 met the existing +1% gate and established no new target-device budget. Promote when a real performance budget and profile identify a bottleneck. Preserve correctness and determinism; benchmark target-device improvement alongside existing instruction-count gates. |

These are priority choices, not technical dependencies on perfect NTSC accuracy. Concrete demand may promote a mapper earlier. RM-008's actual Playstead path is now Near, while a new native host, managed exact Continue, achievements and netplay remain conditional later work requiring a real consumer, supported control boundary and acceptance. No new target-device budget or mapper demand appeared in discovery.

## Long

| ID | Status | Direction | What must be established before promotion |
|---|---|---|---|
| RM-010 | proposed | Selected Famicom expansion-audio boards | No v5 evidence promotes this item. Choose one board; pair banking/IRQ behavior with owned integer audio, mixing/state continuity and permitted tests. Silent partial support is not completion. |
| RM-011 | proposed | MMC5 | No v5 evidence promotes this item. Independent scope for PPU snooping, tile/attribute banking, ExRAM, IRQ, audio and states; do not model it as renamed MMC3. |
| RM-012 | proposed | Famicom Disk System | No v5 evidence promotes this item. Host-supplied BIOS, lawful fixtures, disk-control contract, media state and persistence. No bundled BIOS. |
| RM-013 | proposed | Selected additional peripherals | No new demand surfaced in v5. Concrete player/host demand, input representation and timing tests. Device/window code stays in the host. |

## Recurring policy and historical seed disposition

SEED-006 is recurring: every milestone starts with only the measured CI, flaky-test or documentation repairs needed for that milestone. No promised speedup without a bottleneck and no sweeping cosmetic cleanup. The current player-docs request demonstrates a navigation/truth problem, not a need for a documentation framework. The playability seed is relevant to the proposed actual-game trial; existing boot-to-play/host/state acceptance remains credited rather than re-scoped as missing.

| Source | Direction disposition |
|---|---|
| SEED-001 | Common board families and battery saves delivered in v2; use archived milestone evidence. |
| SEED-002 | Save states, rewind/run-ahead and runner process contract delivered in v3. Actual netplay remains uncommitted. |
| SEED-003 | Partial: the measured baseline shipped in v4 (RM-001). RM-004 shipped in v5; 27 failing rows remain in separate follow-on clusters. Regions remain proposed RM-005/RM-006. |
| SEED-004 | Partial: benchmarks/perf gates, state fuzz (v3) and execution/lifecycle robustness (v4, RM-003) shipped. RM-009 covers measured future optimization. |
| SEED-005 | Split future direction across cartridge, expansion-audio, MMC5, FDS and peripheral outcomes. |
| SEED-006 | Recurring; v5 delivered JAM attribution and run-ahead-under-JAM. Collector/documentation cleanup and three testing proposals remain candidates; never globally marked complete. |
| Playability seed | Existing boot-to-play evidence is credited. Promote an additional integrated host-play outcome only when a new claim requires it. |

Seed files remain unchanged during new-milestone. This disposition ledger prevents stale dormant frontmatter from re-scoping delivered work. Backlog 999.2 is promoted into RM-001 / Phase 17 (CONF-01 through CONF-04); the execution roadmap records its disposition rather than leaving duplicate active backlog work.

## Unresolved debt and delivered tune-up

Authorities: [v5 audit](milestones/v5-MILESTONE-AUDIT.md), [v5 closeout](milestones/v5-CLOSEOUT.md), phase review dispositions and STATE.md Deferred Items. Debt is preserved without implying a new approved maintenance scope.

- Delivered: JAM attribution and explicit run-ahead-under-JAM coverage close the two v4 tune-up gaps. Later accepted vectors pass; the older failed vector run remains history.
- Collector debt: Phase 20 CR-01 omits Git modes, WR-03 scans ignored fixtures/fails relative SOURCE_REF, WR-04 duplicates a validator; Phase 21 WR-01 omits actual filesystem modes (the same defect as Phase 20 WR-05). Independent all-input checks mitigate only the accepted historical runs. Repair recurring enforcement and collect fresh source-bound evidence before treating a new run as qualified.
- Phase 21 historical measurement attribution and README revision-8/revision-9 advisory remain open. Phase 19 per-frame full-state cost, sprite comparator citation and early-stage test-coverage limit remain recorded.
- Local macOS nofp Apple stack-protector references remain platform debt; hosted Linux acceptance is separate. Real-campaign minimization remains proven only by controls. Historical parser/summary-frontmatter metadata warnings remain disclosed.
- Two older Phase 16 debug sessions and the Phase 21 inhibition debug record remain acknowledged without changing their own verdict/status; three proposed testing todos are deferred.
- WINDOWS entry 18's fixed disposition was reconciled to retained source/verification, its missing JSON restored and the unsupported table kind normalized. Historical refusal of a Phase 21 append remains recorded; no missing deviation was invented.
- Physical exact-timestamp ordering and judgment-tier provenance remain explicit limits. The 27 remaining AccuracyCoin FAIL rows and five unscored DRAW entries are not waived or newly committed.
- Package publication of v4 RM-001/RM-003 and v5 remains pending in PR #56; completed planning milestones are distinct from downloadable releases.
- Next-milestone discovery found a player-facing README still opening with v3/behavior revision 8 while current source pins revision 9. A short entry point and topic-doc migration are proposed in RM-014; preserve existing runner/API/release consumers when relocating prose. The two recent CI-overhead quick tasks are already delivered and must not be re-promised.

## Refresh procedure

At each new-milestone and complete-milestone step, the agent reads this file, project state, the last audit, seeds and release evidence. It reconciles shipped/partial/parked items, refreshes each promotion trigger, records changed ordering and presents a preferred next milestone with alternatives. This is part of the invoked step, with no extra lifecycle command or manual reminder.

After scope approval, promote selected IDs to Immediate/committed and link requirements/phases. At closeout, mark only evidenced outcomes shipped, keep real debt visible, refresh the remaining horizons and archive the agreed direction with that milestone. A newly reconstructed boundary snapshot must say when it was reconstructed. Phase discoveries change the ledger only when they affect direction, prerequisites or acceptance.

Keep the file small. Use existing Markdown and git; no roadmap service or parser dependency. A structural validator is optional if omissions recur; it cannot prove that priorities were reconsidered. Small owned helpers and justified local duplication are preferable to extra dependency trees, within the repository's ownership and clean-room rules.

## Review log

- 2026-10-13, v4 opening: promoted RM-001/RM-002/RM-003 from the approved proposal to Immediate/committed. Complete measurement and stronger execution/host evidence precede unnamed accuracy repairs. Retained PAL as the preferred compatibility alternative, with Dendy separately scoped. Deferred larger hardware and host work until evidence or demand justifies it. Historical delivered seed outcomes remain credited; no seed file was changed.
- 2026-10-24, v4 closeout: marked RM-001, RM-002 and RM-003 shipped with evidence. Publication of RM-001 and RM-003 waits on PR #55 and the release after it. The measured baseline met RM-004's trigger, so the order changed: RM-004 is now the preferred next milestone, narrowed to the 11-row NMI/VBlank and sprite-0 cluster. DMA is the strongest alternative cluster. RM-005 (PAL) remains the main alternative milestone and moves behind RM-004, because the baseline shows broad NTSC timing gaps that would carry into PAL timing. Mid and Long items are unchanged; no new demand or performance budget appeared. Carried v4 debt is listed above. No seed file was changed.
- 2026-10-25, v5 opening: promoted RM-004 to Immediate/committed with the SEED-006 tune-up (JAM attribution, run-ahead-under-JAM). The owner chose it over adding the DMA cluster, doing DMA first, or PAL. Ordering otherwise unchanged: DMA is the preferred next cluster, then PAL (RM-005); Mid and Long items are unchanged. Release PR #56 (0.1.12), which publishes RM-001 and RM-003, was still open at the opening. No seed file was changed.

- 2026-10-30, v5 closeout: RM-004 and the JAM tune-up shipped with all eleven requirements and three passed phases; package release PR #56 remains open. Re-measurement of the committed scoreboard narrows the old eight-row DMA candidate set to five failures and three protected passes, so remaining DMA retains first position without re-promising delivered work. PAL remains the main alternative, maintenance-only cleanup the smaller alternative; no new scope is approved. Mid priorities remain demand/profile-triggered and Long capabilities remain exploratory because v5 supplied neither new consumer demand nor regional/hardware acceptance evidence. Recurring collector defects and all evidence limits remain visible; the planning-ledger representation is repaired from retained evidence. The four horizons and triggers were reconsidered in this closeout; seeds remain unchanged.
- 2026-10-30, next-milestone discovery: new owner direction asks for real-game feedback, Playstead readiness, concise public docs and lessons from active sibling cores. Independent Muhtari/peer passes and direct Playstead inspection support RM-014 as the preferred next proposal and move RM-008 from Mid to Near on concrete consumer demand. Immediate remains uncommitted. DMA remains the next accuracy cluster; PAL stays the regional alternative. Mid board/performance work and Long hardware remain conditional because no new title demand, target budget or hardware acceptance evidence promotes them. Retain Dendy's independent regional trigger, all audit debt, published-versus-merged distinctions and existing stops. Do not copy sibling emulation code, import a shared framework or count a handoff as consumer integration. [Synthesis and source reports](research/milestone-v6-discovery/SYNTHESIS.md) preserve rationale, tradeoffs and proposed slices; seeds remain unchanged.
- 2026-10-30, owner playtesting: two Mac smoke sessions were sufficient initial signal despite unattributed sluggishness/audio tearing. The owner requested a Steam Deck daily-play route through existing ES-DE/RetroArch, with reference-only collections, normal progress storage and per-game core choices; this can precede deeper Playstead integration. Local installation/controller/display/launcher work supplies partial RM-014 evidence and no universal compatibility claim. Immediate remains uncommitted; DMA follows player readiness, PAL remains the regional alternative, and Mid/Long retain their demand/evidence triggers. Future emulator projects should consult the PII-free [lessons](playtesting/LESSONS.md). No new phase, requirement, dependency or historical safety waiver was selected.

## Why this shape

A separate product ledger preserves direction while GSD rewrites and archives execution phases. Seeds alone lack current ordering; mixing speculative horizons into execution phase headings invites accidental promotion. GitHub distinguishes exploratory direction from work selected for delivery, and Rust separates broad roadmaps from concrete goals. These practices inform this project's smaller readiness horizons rather than importing dates or governance machinery.

Primary sources checked 2026-10-13: [AccuracyCoin at the proposed pin](https://github.com/100thCoin/AccuracyCoin/blob/6bee919529289c555803b612b8c9e4f7eaaeeabb/README.md), [GitHub roadmap](https://github.com/github/roadmap), [Rust roadmaps](https://goals.rust-lang.org/2026/roadmaps.html), [Rust goal format](https://goals.rust-lang.org/about/goal-format.html), [RetroArch network interface](https://docs.libretro.com/development/retroarch/network-control-interface/), [LLVM libFuzzer](https://llvm.org/docs/LibFuzzer.html). Hardware details in the preparation files preserve their source tiers; NESdev is a curated community reference, not official console documentation.
