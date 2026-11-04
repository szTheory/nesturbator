# Product roadmap: nesturbator

Reviewed: 2026-10-31 during v6 closeout, after phase verification, milestone audit and release-status review.
Direction revision: 10.
Active milestone: none. v6 "Player-ready NES and reproducible feedback" (RM-014) is complete with ten satisfied requirements across Phases 22–23. v5 (RM-004) remains complete.

Package status at closeout: release PR #56 (0.1.12) remains open; its required CI has failures and cancelled checks. The latest published release remains v0.1.11. The owner authorized automatic publication after required release checks are green; this PR is not eligible. Planning milestone completion is separate from package publication.

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

No next milestone is committed. The next scope selection belongs to the next milestone-opening step; all other horizons below remain proposals.

## Delivered outcomes

RM-014 shipped in v6. The player documentation, identified-install, progress-safety and bounded report/replay outcome passed both phase verifications and the milestone audit. The audit found no requirement or integration blocker and retains its non-blocking traceability, timing, campaign, platform and evidence limits. Existing daily-play setup and 22 title reports are credited as partial evidence rather than assigned again. Overscan remains deferred.

RM-004 and the v5 SEED-006 tune-up shipped through merged PRs #57, #59 and #61. All eleven requirements are satisfied; the final campaign is 119 PASS / 27 FAIL over 146 terminal scored rows, plus five inventory-only DRAW entries. Requirements and phase details remain archived in [v5 requirements](v5-REQUIREMENTS.md) and [v5 roadmap](v5-ROADMAP.md). v6 reuses the [postmortem](../playtesting/POSTMORTEM.md) and [cross-project synthesis](../research/milestone-v6-discovery/SYNTHESIS.md); it does not diagnose or promise to fix the reported scenes.

| ID | Status | Outcome | Milestone, release and acceptance evidence |
|---|---|---|---|
| RM-014 | shipped | A player can install the identified NES core, preserve daily progress and supply a reproducible scene report | v6, Phases 22–23; 10/10 requirements, phase verification 7/7 and 9/9, audit 10/10 requirements and 3/3 flows. README/topic guides, install identity, progress protection and bounded cold/checkpoint reporting are documented with checks. No runtime, ABI, state-format or hash change. Existing Deck/ES-DE setup and 22 reports remain scoped smoke evidence. No package release, broad sweep, overscan or actual Playstead qualification is claimed. |
| RM-004 | shipped | NMI/VBlank, interrupt acceptance/takeover and sprite-0 timing satisfy the eleven-row contract | v5 Phases 19–21; 11/11 requirements, three current passed verifications, all eleven exact targets and 114 starting PASS keys on six lanes; ABI 1, state format 3, behavior revision 9. PR #61 merged. Package publication remains pending in release PR #56 (0.1.12); see [v5 closeout](v5-CLOSEOUT.md). |
| RM-001 | shipped | Complete pinned NTSC conformance inventory and bounded baseline | v4 Phase 17; CONF-01–04 ([v4 requirements](v4-REQUIREMENTS.md)); 38/38 verification; 146 scored + 5 DRAW at `6bee919`: 101 pass, 42 fail, 2 not-run, 1 timeout. PR #55 merged; package publication remains pending in release PR #56 (0.1.12), checked at v5 closeout. |
| RM-002 | shipped | Discriminating existing RetroArch progression/state evidence | v4 Phase 16; EVID-01–04; 22/22 verification; released in [v0.1.11](https://github.com/szTheory/nesturbator/releases/tag/v0.1.11) with 18 archives, checksums, attestations and six metadata copies checked. |
| RM-003 | shipped | Bounded accepted-program execution and lifecycle robustness | v4 Phase 18; ROB-01–04; 4/4 verification; six board families reach execution; Nightly 40150888217 green. PR #55 merged; package publication remains pending in release PR #56 (0.1.12), checked at v5 closeout. |

Closeout record: [v4 audit](v4-MILESTONE-AUDIT.md) (tech_debt, 14/14 requirements). RM-001 and RM-003 remain milestone-complete with package publication pending. PR #56 is still open at v5 closeout; no v0.1.12 publication is claimed.

## Near

Phase 23 supplied a repeatable reporting route but did not establish a concrete core fault. Therefore the five remaining DMA failures are the default next measured accuracy candidate. RM-015 stays proposed and can move ahead only after bounded native/source-based triage identifies a specific core fault. No further broad owner play, general frontend, autonomous player or recurring manual release gate is proposed.

The newly named visual scenes justify RM-015 as a conditional following accuracy outcome. First separate native pixels, adapter conversion, host presentation and original game behavior. A confirmed core fault can promote a small owned regression/fix ahead of DMA. If findings are host-only, original quirks or still unlocalized after bounded triage, record that result and retain DMA as the next measured accuracy candidate; do not invent a scene-fix milestone. Five of the former eight DMA candidates still fail; the other three must remain protected. New qualified collection still needs its bounded collector tune-up. Actual Playstead integration requires consumer-owned acceptance; PAL remains the regional alternative, and maintenance-only work the smaller alternative. Perfect NTSC conformance is not a prerequisite for either consumer or regional work.

The Deck/ES-DE/RetroArch route is already useful for daily play. Its 26 normal-library references preserve per-game core choices and normal battery storage, with core-specific snapshots. Feedback covers 22 titles across all six supported mapper IDs; the four remaining games are optional. Actual collection input, same-core automatic state loading and private replay/capture supply partial local evidence. Reconnect, handheld fallback, controlled manual progress roundtrips and version compatibility remain separate claims.

Private recipes repeat current RGB/PCM, while native checkpoint containers and general scene navigation remain limited. Corrected duplicate P1/P2 routing did not remove MM3's reported slowdown. Shatterhand's adaptive-VSync-off relaunch was verified, but disturbances remain and its exact drifting-line comparison is inconclusive. These findings justify investigation, not a core diagnosis or universal compatibility claim. The [postmortem](../playtesting/POSTMORTEM.md), [scene findings](../playtesting/SCENE-PLAYLIST.md) and [automation direction](../playtesting/AUTOMATION.md) preserve the evidence and limits. This setup alone does not complete RM-014 or actual Playstead integration.

| ID | Status | Outcome | Why and promotion trigger |
|---|---|---|---|
| RM-015 | proposed | A confirmed player-visible core fault has an independently justified owned regression and verified fix | New reported OCEAN/split/seam scenes supply investigation candidates, not diagnoses. Promote only after native-vs-host localization and a bounded, source-based reproduction; prefer the CPU→MMC3 IRQ→scroll→pixel boundary. Keep content/checkpoints private and use original fixture bytes. If no core fault is established, record the limit and skip standalone scene-accuracy scope. Do not gate DMA forever on speculative visuals. |
| RM-008 | proposed | A concrete Playstead NES workflow has sufficient capability reporting and integration evidence | Moved from Mid on named consumer demand. First supply the upstream contract using released app/core identity; actual support requires a Playstead-owned adapter/readiness/input/exit/battery path and paired production acceptance. A report, runner result or RetroArch-only test cannot close this row. Native hosting/managed Continue need measured external-route friction or a supported state-control requirement. |
| RM-005 | proposed | PAL games run at their intended timing and pitch | Ufouria (Europe) supplies named regional interest; PAL remains the main regional alternative. Promote on approved regional demand and a bounded CPU/PPU/APU/DMA, AV metadata and state-identity test plan with permitted regional fixtures; NTSC score is insufficient. |
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
| RM-007 | proposed | One additional cartridge board family works through all three deliverables | The owner's shortlist now gives named MMC2 demand through Punch-Out!! (mapper 9), still outside this queue. Prefer evaluating that bounded board before an unnamed mapper when expansion is selected. Promote on scope approval, hardware references and permitted validation; consider 66/206 or 9/10 individually. Include state, persistence when applicable and host parity. No mapper is implemented by discovery. |
| RM-009 | proposed | A measured target device meets frame/run-ahead budgets faster | Still proposed: v5 met the existing +1% gate and established no new target-device budget. Promote when a real performance budget and profile identify a bottleneck. Preserve correctness and determinism; benchmark target-device improvement alongside existing instruction-count gates. |

These are priority choices, not technical dependencies on perfect NTSC accuracy. Punch-Out!! is concrete demand that may promote mapper 9 when requested; current positive Deck reports supply no measured optimization budget, and in-game slowdown alone does not justify a core performance rewrite. RM-008's actual Playstead path stays Near, while a new native host, managed exact Continue, achievements and netplay remain conditional on a real consumer, supported control boundary and acceptance.

The [mapper scope note](../research/milestone-v6-discovery/MAPPER-SCOPE.md) records why six common IDs already reach a large historical share of the licensed library, without claiming a compatibility percentage. Hundreds of assigned IDs include many variants and specialized boards; numbering capacity is not a delivery target. The owner's unsupported shortlist needs IDs 9, 5 and 69, with Ufouria requiring PAL separately. Prefer MMC2/Punch-Out!! when mapper expansion is selected; relative difficulty varies and MMC5 remains a larger independent outcome. This clarification changes no horizon commitment or next-milestone ordering.

## Long

| ID | Status | Direction | What must be established before promotion |
|---|---|---|---|
| RM-010 | proposed | Selected Famicom expansion-audio boards | Gimmick! on the shortlist supplies Sunsoft-5B interest, but no approved scope. Choose one board; pair banking/IRQ behavior with owned integer audio, mixing/state continuity and permitted tests. Silent partial support is not completion. |
| RM-011 | proposed | MMC5 | Castlevania III (USA) supplies named interest, but its breadth keeps it Long pending an explicit choice. Scope PPU snooping, tile/attribute banking, ExRAM, IRQ, audio and states independently; do not model it as renamed MMC3. |
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

Authorities: [v6 audit](v6-MILESTONE-AUDIT.md), [v5 audit](v5-MILESTONE-AUDIT.md), [v5 closeout](v5-CLOSEOUT.md), phase review dispositions and STATE.md Deferred Items. Debt is preserved without implying a new approved maintenance scope.

- Delivered: JAM attribution and explicit run-ahead-under-JAM coverage close the two v4 tune-up gaps. Later accepted vectors pass; the older failed vector run remains history.
- Collector debt: Phase 20 CR-01 omits Git modes, WR-03 scans ignored fixtures/fails relative SOURCE_REF, WR-04 duplicates a validator; Phase 21 WR-01 omits actual filesystem modes (the same defect as Phase 20 WR-05). Independent all-input checks mitigate only the accepted historical runs. Repair recurring enforcement and collect fresh source-bound evidence before treating a new run as qualified.
- Phase 21 historical measurement attribution and README revision-8/revision-9 advisory remain open. Phase 19 per-frame full-state cost, sprite comparator citation and early-stage test-coverage limit remain recorded.
- Local macOS nofp Apple stack-protector references remain platform debt; hosted Linux acceptance is separate. Real-campaign minimization remains proven only by controls. Historical parser/summary-frontmatter metadata warnings remain disclosed.
- Two older Phase 16 debug sessions and the Phase 21 inhibition debug record remain acknowledged without changing their own verdict/status; three proposed testing todos are deferred.
- WINDOWS entry 18's fixed disposition was reconciled to retained source/verification, its missing JSON restored and the unsupported table kind normalized. Historical refusal of a Phase 21 append remains recorded; no missing deviation was invented.
- Physical exact-timestamp ordering and judgment-tier provenance remain explicit limits. The 27 remaining AccuracyCoin FAIL rows and five unscored DRAW entries are not waived or newly committed.
- v6 Phase 22 summary metadata omits the six READY completions, manually reconciled against passed verification. Phase 23 retains five heuristic fallback edges and unexplained intermittent document-test timing anomalies. The interrupted full-CI/campaign outcome is unknown; no full pass is claimed and the workflow was not rerun. Windows/PowerShell, some host recovery and cross-build save behavior remain unqualified. Judgment-tier prohibitions remain flagged, not accepted as verification. See the [v6 audit](v6-MILESTONE-AUDIT.md).
- Package publication of v4 RM-001/RM-003 and v5 remains pending in PR #56, rechecked open during v6 opening; the latest published release is v0.1.11. Completed planning milestones are distinct from downloadable releases. Installation guidance must recheck the actual package when implemented.
- Next-milestone discovery found a player-facing README still opening with v3/behavior revision 8 while current source pins revision 9. RM-014 now commits the short entry point/topic-guide delta; it remains unfinished until verified. Preserve existing runner/API/release consumers when relocating prose. The two recent CI-overhead quick tasks are already delivered and must not be re-promised.
- The playtest checkpoint leaves reported internal seams, OCEAN wobble, timer appearance and scene-specific slowdown/audio questions unclassified. Private deterministic replay is repeatability evidence, not a hardware oracle or a new public CI guard. Controller routing and VSync read-back do not establish those symptoms resolved. Daily-use state upgrade/rollback policy remains a player-readiness gap. See the [mini postmortem](../playtesting/POSTMORTEM.md).

## Refresh procedure

At each new-milestone and complete-milestone step, the agent reads this file, project state, the last audit, seeds and release evidence. It reconciles shipped/partial/parked items, refreshes each promotion trigger, records changed ordering and presents a preferred next milestone with alternatives. This is part of the invoked step, with no extra lifecycle command or manual reminder.

After scope approval, promote selected IDs to Immediate/committed and link requirements/phases. At closeout, mark only evidenced outcomes shipped, keep real debt visible, refresh the remaining horizons and archive the agreed direction with that milestone. A newly reconstructed boundary snapshot must say when it was reconstructed. Phase discoveries change the ledger only when they affect direction, prerequisites or acceptance.

Keep the file small. Use existing Markdown and git; no roadmap service or parser dependency. A structural validator is optional if omissions recur; it cannot prove that priorities were reconsidered. Small owned helpers and justified local duplication are preferable to extra dependency trees, within the repository's ownership and clean-room rules.

## Review log

- 2026-10-31, v6 closeout: RM-014 shipped with all ten requirements, two passed phases and three passing end-to-end flows. The README and canonical guides cover installation/build identity, controls, progress safety and bounded scene replay; no runtime, public ABI, state format or hash baseline changed. Phase 23 did not establish a core fault, so the five remaining DMA failures remain the default next candidate; RM-015 may precede them only on bounded source/native confirmation. PAL remains the regional alternative, and scoped maintenance the smaller alternative. Mid (MMC2/Punch-Out!! demand and measured performance budgets) and Long (selected expansion audio, MMC5, FDS and peripherals) remain proposed with unchanged triggers. The four horizons were refreshed; no future item was promoted. PR #56 for 0.1.12 remains open with failed/cancelled required checks, and v0.1.11 remains latest published. The owner authorized automatic publication only after required checks pass; no publish occurred and the interrupted campaign was not rerun. v6 and carried debt remain in the [audit](v6-MILESTONE-AUDIT.md); seed files and historical safety stops remain unchanged.
- 2026-10-30, v6 opening: the owner selected the recommended player-readiness scope over scene investigation, remaining DMA, MMC2 and PAL. Promoted RM-014 to Immediate/committed, crediting the existing setup and 22 reports and retaining the docs, install identity, progress policy and bounded reproduction delta. Reuse the completed research; SEED-006 contributes only demonstrated documentation/contract repairs. Overscan is deferred. Near retains conditional RM-015, otherwise five DMA failures with three former candidates protected; Playstead remains consumer-owned and PAL/Dendy independently scoped. Mid still favors named MMC2/Punch-Out!! demand when expansion is chosen and requires measured budgets for optimization. Long MMC5/5B/FDS/peripheral triggers are unchanged because no new hardware or device acceptance evidence appeared. Release PR #56 remains open and v0.1.11 is latest published, rechecked during this step. All audit debt, evidence limits and runtime stops remain; seed files are unchanged. The owner subsequently approved all ten concrete requirements and Phases 22–23 with “yes i approve”; milestone opening is complete and Phase 22 discussion is next.

- 2026-10-13, v4 opening: promoted RM-001/RM-002/RM-003 from the approved proposal to Immediate/committed. Complete measurement and stronger execution/host evidence precede unnamed accuracy repairs. Retained PAL as the preferred compatibility alternative, with Dendy separately scoped. Deferred larger hardware and host work until evidence or demand justifies it. Historical delivered seed outcomes remain credited; no seed file was changed.
- 2026-10-24, v4 closeout: marked RM-001, RM-002 and RM-003 shipped with evidence. Publication of RM-001 and RM-003 waits on PR #55 and the release after it. The measured baseline met RM-004's trigger, so the order changed: RM-004 is now the preferred next milestone, narrowed to the 11-row NMI/VBlank and sprite-0 cluster. DMA is the strongest alternative cluster. RM-005 (PAL) remains the main alternative milestone and moves behind RM-004, because the baseline shows broad NTSC timing gaps that would carry into PAL timing. Mid and Long items are unchanged; no new demand or performance budget appeared. Carried v4 debt is listed above. No seed file was changed.
- 2026-10-25, v5 opening: promoted RM-004 to Immediate/committed with the SEED-006 tune-up (JAM attribution, run-ahead-under-JAM). The owner chose it over adding the DMA cluster, doing DMA first, or PAL. Ordering otherwise unchanged: DMA is the preferred next cluster, then PAL (RM-005); Mid and Long items are unchanged. Release PR #56 (0.1.12), which publishes RM-001 and RM-003, was still open at the opening. No seed file was changed.

- 2026-10-30, v5 closeout: RM-004 and the JAM tune-up shipped with all eleven requirements and three passed phases; package release PR #56 remains open. Re-measurement of the committed scoreboard narrows the old eight-row DMA candidate set to five failures and three protected passes, so remaining DMA retains first position without re-promising delivered work. PAL remains the main alternative, maintenance-only cleanup the smaller alternative; no new scope is approved. Mid priorities remain demand/profile-triggered and Long capabilities remain exploratory because v5 supplied neither new consumer demand nor regional/hardware acceptance evidence. Recurring collector defects and all evidence limits remain visible; the planning-ledger representation is repaired from retained evidence. The four horizons and triggers were reconsidered in this closeout; seeds remain unchanged.
- 2026-10-30, next-milestone discovery: new owner direction asks for real-game feedback, Playstead readiness, concise public docs and lessons from active sibling cores. Independent Muhtari/peer passes and direct Playstead inspection support RM-014 as the preferred next proposal and move RM-008 from Mid to Near on concrete consumer demand. Immediate remains uncommitted. DMA remains the next accuracy cluster; PAL stays the regional alternative. Mid board/performance work and Long hardware remain conditional because no new title demand, target budget or hardware acceptance evidence promotes them. Retain Dendy's independent regional trigger, all audit debt, published-versus-merged distinctions and existing stops. Do not copy sibling emulation code, import a shared framework or count a handoff as consumer integration. [Synthesis and source reports](../research/milestone-v6-discovery/SYNTHESIS.md) preserve rationale, tradeoffs and proposed slices; seeds remain unchanged.
- 2026-10-30, owner playtesting: two Mac smoke sessions were sufficient initial signal despite unattributed sluggishness/audio tearing. The owner requested a Steam Deck daily-play route through existing ES-DE/RetroArch, with reference-only collections, normal progress storage and per-game core choices; this can precede deeper Playstead integration. Local installation/controller/display/launcher work supplies partial RM-014 evidence and no universal compatibility claim. Immediate remains uncommitted; DMA follows player readiness, PAL remains the regional alternative, and Mid/Long retain their demand/evidence triggers. Future emulator projects should consult the PII-free [lessons](../playtesting/LESSONS.md). No new phase, requirement, dependency or historical safety waiver was selected.
- 2026-10-30, diminishing-returns checkpoint: 22 reports across all six mapper IDs are enough to stop the broad sweep. Prefer RM-014's remaining documentation, progress-safety and repeatable-report gaps; retain already delivered local setup as partial evidence. Added proposed RM-015 because named scene findings offer a more direct player-quality target than another generic game pass. It moves ahead of DMA only when native/source-based triage establishes a core fault; otherwise DMA remains next. Actual Playstead stays a separate Near consumer outcome, PAL the regional alternative and Dendy independently conditional. Mid now acknowledges Punch-Out!!/MMC2 demand without approving it; Long acknowledges Gimmick!/5B and Castlevania III/MMC5 interest without promoting their broad hardware scope. No target-device performance budget, FDS or peripheral demand was established. Immediate stays empty, all audit debt/stops remain, seed files are unchanged and no publication was rechecked or claimed. [Postmortem](../playtesting/POSTMORTEM.md) records methodology and proposed milestone outcomes.

- 2026-10-30, mapper clarification: recorded the owner's request to retain mapper-count context before clearing the session. Six supported common IDs do not imply six equal-sized tasks out of a fixed total; historical prevalence is not measured compatibility. Prefer MMC2/Punch-Out!! when expansion is selected, with MMC5/5B and PAL independently scoped. All four horizons, promotion triggers, audit debt and current ordering remain unchanged; no additional scope was approved. See the [mapper note](../research/milestone-v6-discovery/MAPPER-SCOPE.md).

## Why this shape

A separate product ledger preserves direction while GSD rewrites and archives execution phases. Seeds alone lack current ordering; mixing speculative horizons into execution phase headings invites accidental promotion. GitHub distinguishes exploratory direction from work selected for delivery, and Rust separates broad roadmaps from concrete goals. These practices inform this project's smaller readiness horizons rather than importing dates or governance machinery.

Primary sources checked 2026-10-13: [AccuracyCoin at the proposed pin](https://github.com/100thCoin/AccuracyCoin/blob/6bee919529289c555803b612b8c9e4f7eaaeeabb/README.md), [GitHub roadmap](https://github.com/github/roadmap), [Rust roadmaps](https://goals.rust-lang.org/2026/roadmaps.html), [Rust goal format](https://goals.rust-lang.org/about/goal-format.html), [RetroArch network interface](https://docs.libretro.com/development/retroarch/network-control-interface/), [LLVM libFuzzer](https://llvm.org/docs/LibFuzzer.html). Hardware details in the preparation files preserve their source tiers; NESdev is a curated community reference, not official console documentation.
