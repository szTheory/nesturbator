# Rolling roadmap governance recommendation

Record note: this research was performed before scope approval. The user approved v4 on 2026-10-13; the original recommendations and adversarial reasoning below are preserved as evidence, not additional committed scope.


Researched 2026-10-13. Read-only assignment; no milestone scope approved, no project files changed. Recommendation confidence: MEDIUM. The installed seam returned MEDIUM for `classify-confidence --provider websearch --verified`; choices below are deductions rather than proven governance outcomes.

## Recommendation and evidence

Create `.planning/PRODUCT-ROADMAP.md` as a small, durable product-direction ledger. Keep `.planning/ROADMAP.md` as the active GSD execution contract. Link the product ledger from PROJECT.md, AGENTS.md and the preparation index after approval. Do not introduce a roadmap service, package, generated board or generalized planning engine.

The repository already intends a rolling roadmap: SEED-006's Notes say that the current roadmap is near term and seeds carry later milestones. But seeds mix shipped work, recurring obligations and future ideas. SEED-001's common boards and SEED-002's state/runner outcomes shipped in v2/v3 while their frontmatter remains dormant; SEED-004 mixes delivered performance work with outstanding execution fuzzing; SEED-003 combines conformance and regional timing; SEED-005 combines several independent hardware families. Those are valuable source narratives, but an inadequate current ordering ledger. PROJECT.md lists candidates without an agreed next milestone. STATE.md says “Awaiting next milestone.”

Primary practice supports separating direction from execution. GitHub distinguishes exploratory work without a timeframe from work it has decided to build, and marks delivered items with release evidence; it warns that future plans can change. Rust groups concrete goals into longer development themes, and expressly says accepting a goal does not pre-decide future technical decisions. Adopt those principles, not GitHub's quarters or Rust's multi-team organization. [GitHub roadmap](https://github.com/github/roadmap), [Rust roadmaps](https://goals.rust-lang.org/2026/roadmaps.html), [Rust FAQ](https://goals.rust-lang.org/2026/faq.html).

## Horizons describe readiness, not fake dates

| Horizon | Meaning | Required detail |
|---|---|---|
| Immediate | Current approved milestone only; empty between milestones | Outcome, requirement/phase links, automated acceptance, exclusions |
| Near | Preferred candidate for the next milestone, ready enough to scope | Why next, bounded outcome, evidence, cost class, dependencies, acceptance proposal |
| Mid | Plausible following themes whose order depends on evidence | Problem, prerequisites, promotion trigger, measurement direction, open decisions |
| Long | Exploratory direction beyond reliable planning depth | Value, constraints, evidence needed to consider it |

Keep one preferred Near bundle plus alternatives, rather than calling everything next. Mid can contain two or three independent themes; Long needs only a few directions. Empty horizons are honest. No phase numbers, guessed release dates, percentage-complete values, or mandatory future milestone numbers. “After conformance results establish the failures” is a useful trigger; “Q1 next year” adds unsupported certainty. Distinguish technical prerequisites from product prioritization: execution fuzzing does not require PAL, and extra mappers need not wait for every NTSC edge test to pass.

## Minimal schema and example

Use ordinary Markdown, one overview table plus short records. Stable IDs such as `RM-001` identify outcomes rather than phases, requirements or releases. Never recycle them. Do not duplicate seed narratives: link each source. Each record carries Horizon, Status, Outcome/why, Evidence/source, Promotion trigger/dependencies, Acceptance, and Link/disposition. An optional “Would change if” line makes priority assumptions explicit. Rust's documented goal format supports explicit proposed/accepted/completed/discontinued states and a concise motivation, but this project needs neither funding tables nor named committees. [Rust goal format](https://goals.rust-lang.org/about/goal-format.html).

Header example:

```text
Reviewed: 2026-10-13 after v3 closeout
Active milestone: none
Direction revision: 1
Planning milestone IDs (v1, v2, v3...) differ from package tags (v0.1.x).
Near/Mid/Long are proposals; only approved Immediate work is committed scope.
```

Illustrative contents, to reconcile with the sibling technical reports before approval:

| ID | Horizon/status | Outcome and reason | Promotion/acceptance direction |
|---|---|---|---|
| RM-001 | Near / proposed | Trust the NTSC claims: repair v3 evidence weaknesses and establish the full current conformance baseline before choosing fixes | Baseline/pin/decoder upgrade first; then approve one observed failure cluster. Stronger frame-chain/command/ROM-selection controls, exact inventory and preserved passes |
| RM-002 | Near alternative / proposed | Hostile ROM execution cannot crash or corrupt an instance | Independent of regions; bounded sanitizer fuzz target, deterministic replay corpus, execution/reset/load properties |
| RM-003 | Mid / proposed | PAL and Dendy run at their own timing | Promote after source/test availability and state/profile/API impact are established; per-region timing and hash evidence |
| RM-004 | Mid / proposed | Selected additional cartridge boards run accurately | Choose by useful coverage and licensed tests, not one giant mapper list; state and host parity required per board |
| RM-005 | Long / proposed | Famicom expansion audio, FDS and peripherals | Split when promoted; lawful assets, host-owned BIOS/input contract, integer audio and save-state model must be feasible |
| RM-006 | Mid / proposed | Stronger real-host evidence beyond macOS | One Linux pilot and defect yield justify expansion; existing portable six-platform host/state parity remains credited |

Do not construe this table as the proposed v4 requirements. The actual AccuracyCoin scoreboard is a limited selected inventory, not proof of comprehensive suite coverage. Full CPU vectors and MMC3 oracle jobs already exist. “Every test passes” is a long-term aspiration until a pinned inventory and documented hardware support contract exist. RM-001 may split into distinct conformance and reliability outcomes; do so if acceptance would otherwise become unbounded.

Use Status `proposed`, `committed`, `shipped`, `parked`, `retired`. Horizon and status are separate dimensions. Parked needs a return trigger; retired needs a reason; shipped needs milestone/release/evidence links. A recurring tune-up remains a policy referencing SEED-006, not a single falsely finished item. Maintain a compact Dispositions table for terminal IDs, with the original outcome and reason. Git history preserves old detail; a closeout snapshot `milestones/vN-PRODUCT-ROADMAP.md` can preserve the agreed direction cheaply. Snapshots need not be machine-generated.

## Cadence and integration without global changes

At every `$gsd-new-milestone`, read the ledger with PROJECT, MILESTONES, STATE, latest audit, seeds and current release evidence. Reconcile completed/partial seeds and debt; update every item's reviewed status, triggers and ordering rationale. Present one preferred milestone proposal, exclusions and important alternatives alongside the revised horizons. The user's request to automatically follow coherent recommendations authorizes this preparation; it does not authorize guessing feature priorities or marking proposals committed. Installed OpenGSD 1.16.0 explicitly confirms the milestone version (step 3), understanding before writes (3.5), requirements (9), and phase roadmap before commit (10). Existing interactive mode and disabled auto-advance are consistent with a concrete review checkpoint.

After authorization, selected records move to Immediate/committed and link to approved requirement IDs and phases. Any historical 999.2 promotion must be explicit and cross-linked; do not silently treat every backlog item as accepted or duplicate the same outcome in both files. Leave the 999.2 execution backlog intact until the relevant workflow migration is authorized. New horizon ideas never become Phase 999.x entries merely to make GSD notice them.

At every `$gsd-complete-milestone`, reconcile committed outcomes against automated verification, exact release publication and audit debt before/alongside PROJECT evolution. Mark shipped only when the stated acceptance evidence exists; retain unresolved debt visibly. Refresh Near/Mid/Long and record why items moved, stayed or retired. Include the product file/snapshot explicitly in the closeout commit, since stock GSD commit lists do not include it. Stop after that GSD step; the next milestone is a separate command. Phase transitions update the ledger only when actual findings change a trigger or priority.

Store this procedure in a short AGENTS.md guard and GSD-HANDOFF addendum, plus a preparation-index link. No patch to the installed global workflows is needed. Current GSD-HANDOFF has stale 1.15.0/Claude-only kickoff guidance; do not copy that runtime limitation into the new contract. A governance change should reconcile those passages after approval.

## Enforcement, alternatives and adversarial checks

Start with the AGENTS guard and reviewable boundary diffs. A CI test cannot prove that priorities were thoughtfully reconsidered. If the user requires machine enforcement of the cadence immediately, add only a tiny owned CMake/CTest check in the existing hygiene infrastructure: unique RM IDs, allowed statuses, committed links, and matching reviewed milestone markers at new/closeout boundaries. No new parser dependency, CI job, timestamp TTL or “roadmap updated on every PR” rule. This is structural assurance only; semantic completeness stays a scope decision. Add it when an omission occurs or a hard invariant is explicitly requested.

Strong alternatives: everything in ROADMAP.md gives one entrypoint but competes with parser, archive and rewrite contracts; seeds alone avoid another file but hide current order and partial completion; a GitHub Project makes public filtering easy but adds API/auth synchronization and another authority. Choose a separate ledger; reconsider a hosted board only when contributor coordination actually needs it.

Adversarial questions: Does a green-but-weak test inflate readiness? Does a score change reflect a new denominator? Is a host acceptance blocker being mistaken for a hardware dependency? Is recurring tune-up becoming an unbounded clean-up milestone? Does “Long” secretly promise all Famicom hardware? Can an urgent crash reprioritize Near without fabricating a scheduled release? Answers must appear as evidence, bounded scope, changed triggers or explicit dispositions. Test/security/maintainer lenses matter here; sales, UI design and device-code roadmaps do not fit the three deliverables.

## Local evidence and limits

Read AGENTS.md; `.planning/{PROJECT,ROADMAP,MILESTONES,STATE}.md`; config.json; preparation README, DECISIONS and GSD-HANDOFF; seeds 001–006 and playability; v3 audit. Installed runtime identity reports `@opengsd/gsd-core` 1.16.0. Relevant runtime-relative paths: `workflows/new-milestone.md` steps 2.5/3/3.5/9/10; `workflows/complete-milestone.md` project evolution/archive/reorganization; `bin/lib/{roadmap-parser,roadmap,phase-id,phase-lifecycle}.cjs`. ROADMAP parsing recognizes milestone/phase headings; sentinel filtering handles 0/999 specially, and closeout preserves the Backlog while rewriting other content. This is verified code evidence, not a claim that arbitrary horizon prose is always misparsed.

External sources above are official primary documentation accessed 2026-10-13; Rust pages describe the active 2026 period. GitHub's README includes older GHES date tables, which were not used as current scheduling evidence. Research-plan seam selected websearch. Digests were not cached because this assignment permits only temporary outputs and forbids project/global-state writes. No emulator source was opened. The technical example ordering remains provisional pending synthesis; governance storage and readiness semantics are the stronger recommendation.

## Final adversarial synthesis pass

Read both temporary sibling reports on 2026-10-13. Prefer the synthesized **v4: NTSC conformance and execution safety**, with three phases starting at 16, over either report's larger speculative fixing milestone: (16) selected existing RetroArch evidence repairs and durable roadmap integration; (17) upgraded AccuracyCoin pin/decoder plus a complete bounded baseline; (18) generated accepted-profile execution/lifecycle fuzzing and cross-deliverable closeout. These outcomes are reviewable now. An unnamed accuracy repair cluster is not: its necessity, boundaries and hardware dispute risk depend on phase17 results. Phase16 must deliver stronger automated proof and truthful documentation, not merely governance paperwork. Phase18's fuzz harness supplies safety evidence, not an accuracy oracle or proof that every malformed program returns without bugs.

Strongest counterargument: this milestone measures quality without a new player-visible compatibility feature, while PAL opens a library. It earns its place only if baseline diagnostics remain bounded, the RA repairs detect known wrong sequences, and fuzzing actually reaches accepted-board execution. Do not turn a hang into an indefinite investigation, a failing count into an obligation to pass everything, or harness coverage into marketing claims. Record capped timeout/not-run outcomes. Publish the full observed inventory and choose the next outcome from it; avoid “all tests executed” wording if an upstream prerequisite prevents execution.

Next preferred Near direction is one named material DMA/PPU/interrupt cluster, conditional on the baseline and gameplay relevance. PAL first, then independently selectable Dendy, is the Near alternative when remaining failures are esoteric or revision-specific. That direction is stable while conditional ordering remains honest. Simple mappers, host preflight, one Linux-host pilot and speed remain Mid unless demand or measurements justify promotion. Expansion audio, MMC5, FDS and peripherals remain separately promotable Long directions. Do not construct technical dependencies merely to enforce this priority ordering; phase18 need not depend on conformance success, only its own existing execution/state contracts.

Keep IDs minimal: allocate one RM ID per independently promotable outcome, not per acceptance bullet, phase, recurring chore or entire hardware family. Select only the baseline portion of SEED-003, execution-safety remainder of SEED-004, and bounded recurring SEED-006 work. Record SEED-001/002 as historically delivered in the product ledger with v2/v3 evidence. Preserve seed files during new-milestone as required; product dispositions guide selection despite stale dormant frontmatter.

The requested automatic cadence needs a project instruction: each new-milestone and complete-milestone step must read, reconcile and include a product-roadmap diff in its own artifacts. The agent performs this, without another lifecycle command or repeated manual reminder. Existing review gates accept the concrete scope, not every bookkeeping operation. A structural CI validator is optional and cannot establish semantic refresh; zero-dependency AGENTS/HANDOFF instructions suffice initially.

Prepare the complete product ledger, proposed requirements/phases, instruction changes and a v3-boundary snapshot under `/tmp` before the scope checkpoint. Although governance documentation itself may be authorized, installed step3.5 says confirmation precedes project writes; keep this coherent proposal temporary until confirmation rather than creating ambiguous partially committed scope. Then install artifacts together. A v3 snapshot reconstructed now must say “recorded after v3 closeout, before v4 approval,” not claim it existed at the historical closeout. No repeated complete-milestone or new lifecycle command is warranted.
