# Installed product direction and v4 opening evidence

**Checked:** 2026-10-13 against repository tree `48482deb4048654eeb6009a8561d316844ef4b0f`.
The current direction record was introduced at `0f41b328a53b058d0548feae812a14b9322ce23d`.

## Actual v4 opening

The opening is recorded in [PRODUCT-ROADMAP.md](../../PRODUCT-ROADMAP.md), not inferred from a future boundary. Its header says it was reviewed on 2026-10-13 at v4 opening and that the user approved the scope. The review log records promotion of only RM-001, RM-002 and RM-003 from the approved proposal to `committed` in Immediate. It explains the ordering: complete measurement and stronger host/execution evidence precede unnamed accuracy repair; PAL remains the preferred compatibility alternative when the NTSC baseline has no broad blocker; Dendy stays separately scoped; larger hardware and host work wait for evidence or demand.

The linked [PROJECT.md](../../PROJECT.md) identifies v4 as the active three-phase scope and points to this direction ledger; [ROADMAP.md](../../ROADMAP.md) records v4 as approved and maps RM-002's Phase 16 work across EVID-01–04; [REQUIREMENTS.md](../../REQUIREMENTS.md) says the requirements were user-approved on 2026-10-13 and maps DIR-01/DIR-02 and the host outcomes to Phase 16. RM-001 remains linked to Phase 17 / CONF-01–04 and RM-003 to Phase 18 / ROB-01–04; the execution roadmap still lists Phases 17 and 18 as not started. [STATE.md](../../STATE.md) retains the sequential pointer at Plan 2 of 6, so independent wave-1 plan 16-06 does not displace 16-02 as the next pending plan. This record does not claim the later phases are complete.

That RM-002 choice follows the [v3 audit](../../milestones/v3-MILESTONE-AUDIT.md): WR-01 identifies dropped/doubled `FRAMEADVANCE` ambiguity, WR-02 identifies missed network commands, WR-03 identifies substring-based fixture selection, and the audit's open IN findings include the `.info` description omitting MMC3.

The [v3 direction snapshot](../../milestones/v3-PRODUCT-ROADMAP.md) explicitly says it was reconstructed on 2026-10-13 after v3 closeout from the pre-approval proposal and did not exist at historical closeout. The current v4 review log records the later approval and opening. This preserves the reconstruction date without attributing the v4 review to the earlier closeout.

## Four installed horizons

| Horizon | Current outcomes and statuses | Evidence, ordering, and promotion condition |
|---|---|---|
| Immediate | RM-001, RM-002, RM-003 — `committed` | RM-001 is Phase 17's complete pinned NTSC inventory and bounded baseline; RM-002 is Phase 16's discriminating existing-host evidence; RM-003 is Phase 18's bounded accepted-program and lifecycle robustness. Each links its requirement IDs. These are the three outcomes the v4 opening approved. |
| Near | RM-004, RM-005, RM-006 — `proposed` | After the full v4 baseline, material gameplay/timing gaps promote named NTSC repair RM-004; otherwise PAL RM-005 is the preferred compatibility milestone, with regional evidence required. Dendy RM-006 is scoped separately after regional infrastructure and demand/evidence justify it. The text says NTSC perfection is not a technical prerequisite for other work. |
| Mid | RM-007, RM-008, RM-009 — `proposed` | Additional board work needs a named title/host need and licensed validation; host workflow work needs a launcher/distribution requirement or reproducible platform defect; optimization needs a real device budget and measured bottleneck. Concrete demand can change ordering. |
| Long | RM-010, RM-011, RM-012, RM-013 — `proposed` | Expansion audio, MMC5, FDS, and additional peripherals each carry board-specific feasibility, lawful fixture, host, timing, state, or demand conditions before promotion. No future milestone dates or package versions are promised. |

The ledger defines allowed statuses as `committed`, `proposed`, `shipped`, `parked`, and `retired`, with evidence/return conditions for shipped, parked, and retired outcomes. All currently listed Immediate/Near/Mid/Long entries use the statuses shown above; stable RM identifiers are not recycled. The review log records why this ordering was chosen and identifies PAL as the important alternative.

The detailed acceptance direction for the three committed outcomes is also present in the ledger: RM-001 preserves an old-pin comparison, validates the immutable AccuracyCoin pin/decoder together, inventories scored tests separately from DRAW, retains raw bounded outcomes and accounts for the 17 protected passes; RM-002 requires distinguishable or absolute state evidence, bounded attributable read retries, one-shot unconfirmed mutations, exact fixture basenames, truthful MMC3 support and retained host artifacts; RM-003 requires execution beyond loader/state fuzz, all six supported board families, deterministic lifecycle continuation, fixed CI replay and finite measured sanitizer evidence, with loops/JAM given explicit outcomes. These are acceptance directions, not evidence that the future plans have shipped.

## Opening structure checks

The following plan commands ran from the repository root against the checked tree above:

```text
git ls-files --error-unmatch AGENTS.md .planning/PRODUCT-ROADMAP.md .planning/PROJECT.md .planning/ROADMAP.md .planning/REQUIREMENTS.md .planning/preparation/README.md .planning/preparation/GSD-HANDOFF.md .planning/milestones/v3-MILESTONE-AUDIT.md .planning/MILESTONES.md
exit: 0
result: all nine canonical sources were tracked

node -e 'const f=require("fs"),p=f.readFileSync(".planning/PRODUCT-ROADMAP.md","utf8");for(const h of ["Immediate","Near","Mid","Long"])if(!p.includes("## "+h+"\n"))throw Error("missing horizon "+h);for(const id of ["RM-001","RM-002","RM-003"])if(!new RegExp("\\| "+id+" \\| committed \\|").test(p))throw Error("missing approved commitment "+id);if(!p.includes("2026-10-13, v4 opening:")||!p.includes("scope approved"))throw Error("opening evidence absent");console.log("direction opening: 4 horizons, 3 commitments and actual opening record checked");'
exit: 0
output: direction opening: 4 horizons, 3 commitments and actual opening record checked
```

These checks establish tracked sources, section/ID structure, and the presence of the recorded opening. The source inspection above establishes the outcome links, conditional ordering, and actual opening rationale; the automated check alone does not assess priority quality.

## Claim boundary

This is a one-time check of the installed direction record and the recorded v4 opening. It does not establish that a future new-milestone or complete-milestone step will follow the installed instructions, and it does not mark any future outcome shipped.

## Both boundary instructions

[AGENTS.md Rule 8](../../../AGENTS.md) and the current [GSD handoff boundary section](../../preparation/GSD-HANDOFF.md) cover both new-milestone and complete-milestone steps. The [product roadmap refresh procedure](../../PRODUCT-ROADMAP.md) repeats the operating contract.

| Boundary | Required work inside the invoked step |
|---|---|
| New milestone | Read project state, the latest audit, original seeds and release evidence with the product roadmap; reconcile delivered and partial outcomes and retain unresolved debt; refresh Immediate/Near/Mid/Long and their promotion triggers; record why ordering changed or stayed; present the preferred next scope and alternatives; move only approved outcomes to Immediate/`committed` with requirement and phase links; include the refreshed roadmap in that step's commit. Original seed files remain intact. |
| Complete milestone | Reconcile current evidence in the same step; mark only acceptance-backed outcomes shipped and keep pending publication pending; retain debt; refresh future horizons and triggers; archive the agreed direction as that milestone's dated product-roadmap snapshot. No second lifecycle command is used for upkeep. |

The handoff also requires reconstructed snapshots to state their actual recording date. That is reflected in the v3 snapshot's 2026-10-13 reconstruction note; it is not represented as a historical v3 closeout artifact. This plan verifies the installed rules and the recorded opening only; future closeout behavior remains unobserved.

## Seed and debt reconciliation

The current product roadmap's disposition table matches the original seed narratives while leaving their source files unchanged:

| Source | Current disposition in the product roadmap |
|---|---|
| [SEED-001](../../seeds/SEED-001-library-coverage.md) | Common boards and battery saves delivered in v2; archived milestone evidence remains the source for that outcome. |
| [SEED-002](../../seeds/SEED-002-save-states-and-runner-contract.md) | Save states, rewind/run-ahead and runner process contract delivered in v3; actual netplay remains uncommitted. |
| [SEED-003](../../seeds/SEED-003-accuracy-and-pal.md) | Partial: RM-001/RM-004 cover measured NTSC accuracy and RM-005/RM-006 cover regional outcomes. The original all-pass aspiration does not create an all-pass v4 requirement. |
| [SEED-004](../../seeds/SEED-004-robustness-and-speed.md) | Partial: perf gates and state fuzz shipped; RM-003 covers remaining execution robustness and RM-009 covers measured future optimization. |
| [SEED-005](../../seeds/SEED-005-more-hardware.md) | Split into separate proposed board, expansion-audio, MMC5, FDS and peripheral outcomes. |
| [SEED-006](../../seeds/SEED-006-milestone-tune-up.md) | Recurring at each milestone; never globally complete. |
| [Playability seed](../../seeds/SEED-261009-zs2-single-game-playability-check.md) | Existing boot-to-play evidence is credited; another integrated playability outcome is promoted only when a new claim needs it. |

The original files retain their historical text and `dormant` frontmatter. Current delivered/partial/recurring status comes from the product-roadmap disposition ledger, so stale seed frontmatter does not reopen shipped work. No seed source or priority was edited by this verification.

The [v3 audit](../../milestones/v3-MILESTONE-AUDIT.md) remains `tech_debt` and records 17 items across Phases 11, 13, 14, 15 and integration. The opening promotes the Phase 13 WR-01/WR-02/WR-03 evidence weaknesses and the omitted MMC3 `.info` description into RM-002 / Phase 16. Those findings remain open until their Phase 16 host work is implemented and verified. Other review/info findings, the macOS-only real-host state/rewind/run-ahead lane, the adapter's lack of a save-generation signal, the stale runner comment, and summary bookkeeping remain in the audit, State, or milestone record. The later HPI reconciliation records closure of the v0.1.10 release blocker and Phase 15 WR-04; it does not erase unrelated v3 debt. No unrelated debt was folded into this verification.

## Release and version evidence

The [v3 milestone record](../../MILESTONES.md) and [read-only HPI evidence](../../quick/261013-hpi-reconcile-historical-planning-records-an/261013-hpi-SUMMARY.md) distinguish release-PR merge from publication. Release PR #35 merged on 2026-10-13 at `da2bf973813530cb9561a004e7b774011a0a7867`; run `38432194730` completed successfully, including `publish`, on that exact SHA. The recorded check at 2026-10-13T16:53:46Z confirmed non-draft package `v0.1.10`, published at 16:53:40Z, with 18 expected platform ZIPs plus `SHA256SUMS`; all 19 assets were nonempty.

That is actual publication evidence for the v3 package only. Planning milestone v4 is distinct from package version `v0.1.10`; this documentation-only Phase 16 plan supplies no evidence that Phase 16 changes have been released. No Phase 17/18 or future publication status is inferred.

## Unclassified edge flags retained

The plan's spec-less edge audit keeps four rows explicitly unresolved. This record does not reclassify them or claim exhaustive edge coverage:

| Requirement | Preserved flag | Limit of this evidence |
|---|---|---|
| EVID-01 | Unclassified assumption | Recorded marker/origin/verdict cases do not prove exhaustive host progression edge coverage; shared host work remains pending. |
| EVID-04 | Unclassified assumption | The selected MMC3 facts do not establish a complete metadata-edge taxonomy. |
| DIR-01 | Unclassified assumption | A one-time link/structure/current-evidence check does not prove the shape of every future direction edge. |
| DIR-02 | Unclassified assumption | Installation and actual opening are checked; future opening/closeout compliance is unproven. |

The plan's four other surfaced edges were classified as explicit contracts (EVID-02 idempotency/concurrency and EVID-03 empty/encoding). Their existence does not turn the four flags above into resolved assumptions. DIR-01 and DIR-02 are the only Phase 16 requirements addressed by this plan; EVID-01 remains pending, and this record does not claim a host calibration or progression gate passed.

## Boundary, source-link and preservation checks

The plan's boundary/seed check ran from the repository root:

```text
node -e 'const f=require("fs");for(const n of ["AGENTS.md",".planning/preparation/GSD-HANDOFF.md",".planning/PRODUCT-ROADMAP.md"]){const s=f.readFileSync(n,"utf8");for(const t of ["new-milestone","complete-milestone"])if(!s.includes(t))throw Error(n+" missing "+t);if(!/commit/i.test(s)||!/archiv|snapshot/i.test(s))throw Error(n+" missing commit/archive contract");}const p=f.readFileSync(".planning/PRODUCT-ROADMAP.md","utf8");for(const t of ["SEED-001","SEED-002","SEED-003","SEED-004","SEED-005","SEED-006","recurring","partial","debt"])if(!p.includes(t))throw Error("missing disposition "+t);console.log("direction boundaries: 3 instruction sources, 2 lifecycle boundaries and 6 seed dispositions checked");'
exit: 0
output: direction boundaries: 3 instruction sources, 2 lifecycle boundaries and 6 seed dispositions checked

node -e 'const f=require("fs"),p=require("path"),files=["AGENTS.md",".planning/PROJECT.md",".planning/ROADMAP.md",".planning/REQUIREMENTS.md",".planning/PRODUCT-ROADMAP.md",".planning/preparation/README.md",".planning/preparation/GSD-HANDOFF.md",".planning/MILESTONES.md",".planning/milestones/v3-MILESTONE-AUDIT.md",".planning/milestones/v3-PRODUCT-ROADMAP.md",".planning/quick/261013-hpi-reconcile-historical-planning-records-an/261013-hpi-SUMMARY.md",".planning/phases/16-host-evidence-and-roadmap/16-DIRECTION-VERIFICATION.md"];let count=0,missing=[];for(const file of files){const s=f.readFileSync(file,"utf8"),re=/\[[^\]]*\]\(([^)\s]+)(?:\s+[^)]*)?\)/g;let m;while((m=re.exec(s))){let t=m[1];if(/^(https?:|mailto:|#)/.test(t))continue;t=decodeURIComponent(t.split(/[?#]/,1)[0]);if(!t)continue;count++;const target=p.resolve(p.dirname(file),t);if(!f.existsSync(target))missing.push(file+" -> "+t);}}if(missing.length)throw Error("missing local links: "+missing.join("; "));console.log("local markdown links: "+count+" checked across "+files.length+" direction/evidence files; missing 0");'
exit: 0
output: local markdown links: 59 checked across 12 direction/evidence files; missing 0

git diff --exit-code -- AGENTS.md .planning/PRODUCT-ROADMAP.md .planning/preparation/GSD-HANDOFF.md .planning/seeds
exit: 0
output: no changes to operating instructions, approved direction or seed sources
```

The local links in the operating/approval/history evidence resolve to tracked repository files. The one-time link check verifies targets, not future compliance or the quality of future priority choices. No recurring validator, global GSD workflow change, extra lifecycle command, priority change, or seed rewrite was added.
