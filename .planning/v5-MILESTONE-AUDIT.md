---
milestone: v5
audited: 2026-10-30T17:17:18Z
status: tech_debt
scores:
  requirements: 11/11
  phases: 3/3
  integration: 8/8
  flows: 11/11
gaps:
  requirements: []
  integration: []
  flows: []
tech_debt:
  - phase: 19-jam-reporting-and-sprite-0-hit
    items:
      - "IN-01 remains open: full native state is saved for each converted adapter frame; measured cost remains recorded."
      - "IN-02 remains open: sprite-0 comparator hardware comment cites unrelated sections."
      - "Navigation/start-stage JAM uses the shared early-stage path; the constructed early-stage test covers boot directly."
  - phase: 20-nmi-line-and-vblank-flag
    items:
      - "CR-01 remains deferred: recurring collector validation omits Git modes; independent 418-input mode/blob checks mitigate the accepted historical run only."
      - "WR-03 remains deferred: recursive scoreboard lookup reaches ignored build fixtures and relative SOURCE_REF fails; accepted collection used an absolute clean-equivalent checkout."
      - "WR-04 remains deferred: duplicate identical Nightly validator definition."
      - "WR-05 is the same actual-filesystem mode gap as Phase 21 WR-01; count that defect once."
      - "Historical summary parser warnings and two unclassified probe labels remain metadata debt, not missing behavior evidence."
  - phase: 21-interrupt-latency-and-hijack
    items:
      - "WR-01 remains deferred: actual worktree executable-mode changes are absent from recurring source identity; independent 428-input filesystem mode/raw-blob checks mitigate its historical accepted run only."
      - "WR-02 remains open: historical measurement prose attributes a different capture to the canonical metadata."
      - "README overview says behavior revision 8 while detailed documentation and public header pin revision 9."
      - "Local Apple-toolchain nofp undefined-symbol failure remains platform-limited debt; accepted hosted Linux nofp evidence is separate."
  - phase: planning-and-carried-debt
    items:
      - "WINDOWS ledger is inconsistent: counters/table contain 18 entries but authoritative JSON contains 17, missing table entry 18; append previously refused."
      - "Five requirement completion claims remain absent from execution-summary frontmatter by verifier-ownership policy; this audit explicitly reconciles them against later verification and executable evidence."
      - "PROJECT/ROADMAP retain historical merge-pending prose; STATE, this audit and live GitHub supersede it. Reconcile lifecycle summaries during closeout."
      - "Real-campaign minimization is proven only by controls; older acknowledged debug sessions and metadata warnings remain preserved."
evidence_limits:
  - "Immutable hosted acceptance remains bound to its original candidate/tested-merge identities. Final-head green workflows do not create a replacement collected bundle."
  - "Physical exact-timestamp CPU/PPU precedence is unmeasured; claims cover the documented finite-margin deterministic model."
  - "Judgment-tier clean-room/provenance prohibitions remain flagged; no manual testing or sign-off gate is introduced."
  - "The canonical baseline retains 27 failing AccuracyCoin rows and five unscored DRAW entries; those are outside the approved eleven-row repair scope."
nyquist:
  enabled: false
  overall: skipped
  compliant_phases: []
  partial_phases: []
  not_validated_phases: []
  missing_phases: []
---

# v5 milestone audit: NMI, VBlank and sprite-0 timing

**Verdict: tech_debt. All eleven requirements are satisfied, all three phase
reports remain current and passed, and no blocking integration or end-to-end gap
was found.** Deferred collector defects, documentation inconsistencies and
planning-record debt remain visible. This audit does not accept new risk, close
the milestone, tag a release, or authorize further implementation.

## Scope and definition of done

The approved scope is Phases 19–21, all 27 plans, eleven requirements and the
eleven named AccuracyCoin timing rows at pin `6bee919`. The milestone also
requires preservation of starting PASS rows, deterministic output and state
continuation through the library, runner and libretro adapter. The three
deliverables remain unchanged. No general DMA cluster, PAL work, new pin,
row-specific timing policy or higher 1800-frame cap was included. Phase 21's
bounded DMC retry/controller open-bus prerequisite was explicitly approved and
is documented in its plan summary and phase audit.

The final campaign records 119 PASS / 27 FAIL over 146 terminal scored rows,
plus five inventory-only DRAW entries. All eleven selected rows pass. The
v4 starting 101-PASS authority is preserved through the sequential 108- and
114-key protected authorities; the final accepted six-lane gate protects all
114 Phase 21 starting PASS keys. Aggregate score improvement is not substituted
for the exact-key target and regression gates.

Inputs: [project](PROJECT.md), [requirements](REQUIREMENTS.md),
[roadmap](ROADMAP.md), all phase verifications, all 27 summary frontmatters,
review dispositions, evidence ledgers and the fresh
[integration audit](v5-INTEGRATION-AUDIT.md). No phase report or historical
summary was rewritten to make the audit pass.

## Phase verification and freshness

| Phase | Current report | Verified truths | Plans | Requirement verdict |
|---|---|---:|---:|---|
| 19 — JAM reporting and sprite-0 hit | [passed](phases/19-jam-reporting-and-sprite-0-hit/19-VERIFICATION.md), 2026-10-30 | 4/4 | 6/6 | JAM-01/02, SPR-01/02 satisfied |
| 20 — NMI line and VBlank flag | [passed](phases/20-nmi-line-and-vblank-flag/20-VERIFICATION.md), 2026-10-30 | 32/32 | 12/12 | NMI-01/02, KEEP-02/03 satisfied |
| 21 — Interrupt latency and hijack | [passed](phases/21-interrupt-latency-and-hijack/21-VERIFICATION.md), 2026-10-30 | 32/32 | 9/9 | INT-01/02, KEEP-01 satisfied |

The verification owner returned `passed` for each directory in this audit.
There are no missing, stale, gaps-found or behavior-unverified phase reports.
The final contract is ABI 1, state format 3, behavior revision 9. Phase 20's
historical format-2/revision-8 checkpoint is superseded for current state
compatibility; formats 1 and 2 are refused before mutation.

## Requirements: three-source reconciliation

Every requirement appears in the traceability table and a phase verification
requirements table. No requirement is orphaned. Six have summary completion
claims. The other five initially resolve to `partial — verify manually` under
the workflow matrix and are reconciled below using the later verification,
source tracing and focused executable checks.

| Requirement | Phase | VERIFICATION | SUMMARY `requirements-completed` | Register | Final |
|---|---:|---|---|---|---|
| JAM-01 | 19 | SATISFIED | 19-02, 19-06 | [x] Complete | satisfied |
| JAM-02 | 19 | SATISFIED | 19-03, 19-06 | [x] Complete | satisfied |
| SPR-01 | 19 | SATISFIED | 19-04, 19-05, 19-06 | [x] Complete | satisfied |
| SPR-02 | 19 | SATISFIED | 19-04, 19-05, 19-06 | [x] Complete | satisfied |
| NMI-01 | 20 | SATISFIED | 20-03, 20-04, 20-06 | [x] Complete | satisfied |
| NMI-02 | 20 | SATISFIED | 20-03, 20-04, 20-06 | [x] Complete | satisfied |
| KEEP-02 | 20 | SATISFIED | none; verifier owns closure | [x] Complete | satisfied after evidence reconciliation |
| KEEP-03 | 20 | SATISFIED | none; verifier owns closure | [x] Complete | satisfied after evidence reconciliation |
| INT-01 | 21 | SATISFIED | none; verifier owns closure | [x] Complete | satisfied after evidence reconciliation |
| INT-02 | 21 | SATISFIED | none; verifier owns closure | [x] Complete | satisfied after evidence reconciliation |
| KEEP-01 | 21 | SATISFIED | none; verifier owns closure | [x] Complete | satisfied after evidence reconciliation |

[20-10-SUMMARY.md](phases/20-nmi-line-and-vblank-flag/20-10-SUMMARY.md)
explicitly assigns requirement completion to independent phase re-verification.
[21-09-SUMMARY.md](phases/21-interrupt-latency-and-hijack/21-09-SUMMARY.md)
likewise leaves closure pending parent verification. These are truthful
execution-time snapshots. The subsequent passed phase reports close their
requirements; a historical pending key does not override that later evidence.
This is the same evidence-reconciliation policy documented by the v4 audit.

| Reconciled requirement | Specific evidence beyond its checked register entry |
|---|---|
| KEEP-02 | Phase 20 current verification traces causal hash pins, equal-tick key controls and empty-PCM history; Phase 21 source-bound acceptance contains six equal inventories. Fresh `hash.inventory.self_test` and `state.format_pin` pass. |
| KEEP-03 | Current visitor/validation is traced through core, runner and actual adapter restore. Fresh NMI/entry replay, rejection, runner roundtrip/idempotency/legacy-format refusal and libretro serialize/runahead pass. |
| INT-01 | Production APU → bus → legal poll → accepted request → shared entry is traced. Fresh `core.irq_poll` and exact eleven-target campaign gate pass; current phase verification also covers `cpu.unit`. |
| INT-02 | Production captured NMI → staged BRK/IRQ vector choice is traced. Fresh entry/state/fork and actual adapter runahead tests pass; both overlap target keys pass. |
| KEEP-01 | Fresh campaign and no-lost-pass checks pass. The immutable accepted bundle records all 114 protected keys on six lanes, vectors 258/258, MMC3 13/13, Holy Mapperel, boot-to-play and original performance gate; final PR CI and Nightly are independently green. |

All five partial metadata checks are resolved for implementation coverage.
Their summary-frontmatter omissions remain disclosed metadata debt. There are
zero unsatisfied or unresolved partial requirements, so the FAIL gate does not
force `gaps_found`.

## Integration and end-to-end results

The skill's specialized integration checker traced and tested these eight
connection groups. Its [report](v5-INTEGRATION-AUDIT.md) contains source call
paths, the eleven-row requirements integration map and the complete flow list.

| Connection group | Result |
|---|---|
| Sprite identity/hit → split PPUSTATUS sampling | WIRED |
| PPU NMI output → production bus capture | WIRED |
| Captured history → instruction-specific poll and acceptance | WIRED |
| Accepted IRQ/NMI history → shared staged entry/vector takeover | WIRED |
| Runtime histories → state visitor, validation and public state API | WIRED |
| State API → runner save/load/replay | WIRED |
| State API → actual libretro restore/runahead and JAM cache | WIRED |
| Campaign → exact targets/protected authority → collector/platform evidence | WIRED |

All eleven requirement-aligned producer-to-observable flows complete. No
relevant orphaned contract or missing expected call path was found. HTTP/API
routes and authentication checks are inapplicable to the three native products.

Fresh focused verification used the existing current-source `build/ci` tree:

- Runtime, campaign, native state, runner and actual adapter: 24/24 passed,
  including three fixture prerequisites, in 28.42 seconds.
- Offline Phase 20/21 collector controls and hash inventory: 3/3 passed in
  38.55 seconds. These exercise validators without fetching hosted artifacts.

Raw logs: `build/v5-milestone-audit/focused-ctest.log` and
`build/v5-milestone-audit/focused-evidence-ctest.log`. The existing current
phase reports retain their full CI/package evidence. No new full-suite run or
new immutable hosted collection was needed for this documentation-only audit.

## Delivery and evidence boundaries

Live GitHub connector queries during this audit confirm
[PR #61](https://github.com/szTheory/nesturbator/pull/61) is merged. Its final
published head is `e5e2d7d143619073a3bed4f2f3412834ef1050c8`, also the audited
local source HEAD; the squash merge is
`45a078d2de1f08e39caf1c5942fcdb57f493d8af`.
[CI 41070047664](https://github.com/szTheory/nesturbator/actions/runs/41070047664)
and [Nightly 41070047609](https://github.com/szTheory/nesturbator/actions/runs/41070047609)
both report `completed/success` for that final head. No merge or delivery
blocker remains. Release artifact publication after the squash merge was not
audited here and is not claimed.

Historical source-bound acceptance stays distinct:

| Evidence | Candidate / tested merge | Scope |
|---|---|---|
| [Phase 20 ledger](phases/20-nmi-line-and-vblank-flag/20-EVIDENCE.md) and [refresh](phases/20-nmi-line-and-vblank-flag/20-REVERIFICATION-EVIDENCE.md) | `7848cc8` / `716e8d4` | 418 effective inputs, independently checked modes/blobs, accepted CI/Nightly contents |
| [Phase 21 ledger](phases/21-interrupt-latency-and-hijack/21-EVIDENCE.md) | `54c7db2` / `faf1b91` | 428 effective inputs, independently checked actual filesystem modes/raw blobs, six lanes and host/conformance/performance evidence |

The final-head workflow successes and current local checks do not rewrite
those manifests or imply a new content/provenance collection. The phase
verifications assess current behavior separately from historical acceptance.

## Debt, fixed items and limits

The YAML above is the carried-debt inventory. The integration report's nine
warnings overlap these records; do not add their counts together. In
particular, Phase 20 WR-05 and Phase 21 WR-01 describe the same defect, while
the broader Phase 20 CR-01 remains separately recorded at its original critical
severity. It is deferred, not reclassified or fixed. Independent all-input
mode/blob evidence supplies the accepted-run mitigation; automated enforcement
for future collections is still missing. Repairs require a new effective
source and fresh hosted evidence rather than reuse of the old manifests.

Review authorities: [19 disposition](phases/19-jam-reporting-and-sprite-0-hit/19-REVIEW-DISPOSITION.md),
[20 disposition](phases/20-nmi-line-and-vblank-flag/20-REVIEW-DISPOSITION.md),
[21 disposition](phases/21-interrupt-latency-and-hijack/21-REVIEW-DISPOSITION.md).
Phase 19 WR-01/02 and Phase 20 WR-01/02 remain fixed. Phase 21's historical
CR-01 rejection and CR-02 fix remain in its review history. No disposition was
silently changed by this audit.

The planning-ledger discrepancy is reproducible now: [WINDOWS.md](WINDOWS.md)
has `total_count: 18`, table row 18, and a fenced JSON array of 17 entries whose
last ID is 17. This is a concrete representation mismatch, not an established
runtime gap or a corrected aggregate. Preserve the historical record and
reconcile it in its owning planning-health work; no ledger repair was made.

Inherited v4 JAM-attribution and runahead-under-JAM debt is now delivered by
JAM-01/02. The historical failed vector run remains history; later accepted
full-vector evidence passes. Real-campaign minimization, acknowledged
`phase16-cleanup-fixture` / `phase16-interruption` sessions, Apple-toolchain
`nofp` symbol handling and historical summary-parser warnings are carried,
without reopening old milestones or claiming those items fixed.

Physical zero-margin tie ordering, full clean-room provenance assurance and
the remaining AccuracyCoin failures are evidence/scope limits, not newly
invented approval or manual testing gates. No benchmark threshold, protected
PASS rule, source identity requirement, frame cap or hardware contract is
waived by the `tech_debt` verdict.

## Nyquist discovery

Skipped: `verify:post` has no active `validate-phase` step hook and the project
has Nyquist validation disabled. No missing-validation blocker or retroactive
validation command is introduced.

## Next step

Preferred next command: `$gsd-complete-milestone v5`. That separate step reviews
and records the debt disposition, archives v5, and performs the required rolling
product-roadmap refresh within closeout. Read product direction, seeds, latest
audit and release evidence there. Before archiving, reconcile the WINDOWS
ledger's counts and entry-18 disposition against its retained source records;
this audit establishes the mismatch but does not claim a corrected aggregate.
Preserve all four horizons, promotion triggers
and the rationale for ordering. DMA remains the currently proposed preferred
next accuracy cluster, with PAL the main alternative; neither is approved by
this audit. A scoped collector/documentation cleanup before closeout is an
available alternative if the owner chooses that additional scope.

No further lifecycle command ran. Context may be cleared before closeout;
[STATE.md](STATE.md) and [continue.md](continue.md) provide the handoff.
