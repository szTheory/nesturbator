# Installed product direction and v4 opening evidence

**Checked:** 2026-10-13 against repository tree `48482deb4048654eeb6009a8561d316844ef4b0f`.
The current direction record was introduced at `0f41b328a53b058d0548feae812a14b9322ce23d`.

## Actual v4 opening

The opening is recorded in [PRODUCT-ROADMAP.md](../../PRODUCT-ROADMAP.md), not inferred from a future boundary. Its header says it was reviewed on 2026-10-13 at v4 opening and that the user approved the scope. The review log records promotion of only RM-001, RM-002 and RM-003 from the approved proposal to `committed` in Immediate. It explains the ordering: complete measurement and stronger host/execution evidence precede unnamed accuracy repair; PAL remains the preferred compatibility alternative when the NTSC baseline has no broad blocker; Dendy stays separately scoped; larger hardware and host work wait for evidence or demand.

The committed RM-002 outcome links Phase 16 to EVID-01 through EVID-04. That choice follows the [v3 audit](../../milestones/v3-MILESTONE-AUDIT.md): WR-01 identifies dropped/doubled `FRAMEADVANCE` ambiguity, WR-02 identifies missed network commands, WR-03 identifies substring-based fixture selection, and the audit's open IN findings include the `.info` description omitting MMC3. [REQUIREMENTS.md](../../REQUIREMENTS.md) maps those four host outcomes to Phase 16. RM-001 remains linked to Phase 17 / CONF-01–04 and RM-003 to Phase 18 / ROB-01–04; the execution roadmap still lists Phases 17 and 18 as not started. This record does not claim their completion.

The [v3 direction snapshot](../../milestones/v3-PRODUCT-ROADMAP.md) explicitly says it was reconstructed on 2026-10-13 after v3 closeout from the pre-approval proposal and did not exist at historical closeout. The current v4 review log records the later approval and opening. This preserves the reconstruction date without attributing the v4 review to the earlier closeout.

## Four installed horizons

| Horizon | Current outcomes and statuses | Evidence, ordering, and promotion condition |
|---|---|---|
| Immediate | RM-001, RM-002, RM-003 — `committed` | RM-001 is Phase 17's complete pinned NTSC inventory and bounded baseline; RM-002 is Phase 16's discriminating existing-host evidence; RM-003 is Phase 18's bounded accepted-program and lifecycle robustness. Each links its requirement IDs. These are the three outcomes the v4 opening approved. |
| Near | RM-004, RM-005, RM-006 — `proposed` | After the full v4 baseline, material gameplay/timing gaps promote named NTSC repair RM-004; otherwise PAL RM-005 is the preferred compatibility milestone, with regional evidence required. Dendy RM-006 is scoped separately after regional infrastructure and demand/evidence justify it. The text says NTSC perfection is not a technical prerequisite for other work. |
| Mid | RM-007, RM-008, RM-009 — `proposed` | Additional board work needs a named title/host need and licensed validation; host workflow work needs a launcher/distribution requirement or reproducible platform defect; optimization needs a real device budget and measured bottleneck. Concrete demand can change ordering. |
| Long | RM-010, RM-011, RM-012, RM-013 — `proposed` | Expansion audio, MMC5, FDS, and additional peripherals each carry board-specific feasibility, lawful fixture, host, timing, state, or demand conditions before promotion. No future milestone dates or package versions are promised. |

The ledger defines allowed statuses as `committed`, `proposed`, `shipped`, `parked`, and `retired`, with evidence/return conditions for shipped, parked, and retired outcomes. All currently listed Immediate/Near/Mid/Long entries use the statuses shown above; stable RM identifiers are not recycled. The review log records why this ordering was chosen and identifies PAL as the important alternative.

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

This is a one-time check of the installed direction record and the recorded v4 opening. It does not establish that a future new-milestone or complete-milestone step will follow the installed instructions, and it does not mark any future outcome shipped. The publication evidence and seed/debt reconciliation are recorded in the continuation of this same direction-verification artifact after the boundary and source-history checks.
