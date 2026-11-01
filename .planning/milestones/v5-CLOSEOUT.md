# v5 closeout: NMI, VBlank and sprite-0 timing

Completed: 2026-10-30. Planning milestone v5 is distinct from package 0.1.12.

## Outcome and authorization

All three phases were complete and current/passed immediately before archival:
Phase 19 had six plans, Phase 20 twelve, Phase 21 nine. All 27 have summaries.
All eleven requirement checkboxes and traceability rows are complete. The
[milestone audit](v5-MILESTONE-AUDIT.md) reports `tech_debt`, with no blocking
requirements, integration or end-to-end gaps. The [integration audit](v5-INTEGRATION-AUDIT.md)
records eight wired groups and eleven complete flows.

The owner approved the recommended acknowledgement-and-close option with
“follow ur recs” after the six open records and carried audit debt were presented.
This is `override_closeout` because records were explicitly deferred, not because
phase verification or product requirements were waived. All six acknowledgements
were written successfully by `audit-open acknowledge`; the follow-up scanner
reported zero open and sixteen acknowledged records. Six are newly acknowledged,
ten are carried from earlier closes. Artifact statuses and historical verdicts
remain unchanged. Full disclosure is in [STATE.md](../STATE.md#deferred-items).

| Newly acknowledged record | Disposition at close |
|---|---|
| Debug session `entry-review-inhibition`, `verifying` | Preserve causal repair evidence and its own status; phase verification separately covers the final behavior. |
| Proposed owned property tests | Future testing scope; demonstrate added defect detection before introducing a helper. |
| Proposed scoped nightly mutation pilot | Future testing scope; no tool, lane or dependency selected. |
| Proposed deterministic runner signal checks | Future testing scope; preserve the existing bounded-stop contract. |
| Phase 21 WINDOWS ledger refusal | Historical refusal acknowledged; representation reconciled below. No missing deviation invented. |
| Phase 21 local macOS arm64 nofp symbols | Remains platform debt; hosted Linux nofp acceptance is separate. |

Prior acknowledgements comprise two Phase 16 debug sessions, seven seeds and
one Phase 16 platform-limited nofp record. A clear scanner does not mean these
were fixed. Completed quick tasks remain in `.planning/quick/` with their STATE
index; optional quick-task archival was left off. No phase/milestone lifecycle
command beyond this closeout ran. No new scope, runtime window or safety waiver
was granted.

## Delivered accomplishments

- AccuracyCoin reports guest JAM in every stage; libretro run-ahead keeps the
  correct cached picture and silence, including divergent timeline controls.
- OAMADDR-based sprite evaluation and its rendering reset satisfy both sprite-0
  targets with owned per-dot evidence.
- Split PPUSTATUS sampling and NMI line/capture/poll history satisfy all six
  selected NMI/VBlank rows.
- Instruction-specific IRQ acceptance and shared resumable BRK/IRQ/NMI entry
  satisfy latency and coherent vector takeover; held/released inhibited NMI
  candidates have causal source and replay controls.
- ABI 1, state format 3 and behavior revision 9 carry validated continuation
  through the library, runner and actual loaded libretro module. Earlier state
  formats are refused before mutation; causal hash changes are documented.
- The final campaign is 119 PASS / 27 FAIL over 146 terminal scored rows, plus
  five inventory-only DRAW entries. All eleven exact targets and all 114 Phase 21
  starting PASS keys pass on six lanes, with matching native inventories and
  the unchanged conformance, host and +1% performance acceptance gates.

## Statistics and reconciliation

| Measure | Result and definition |
|---|---|
| Phases / plans / tasks | 3 / 27 / 68. Tasks are the sum of all summary `actuals.tasks` fields: Phase 19 = 17, Phase 20 = 30, Phase 21 = 21. |
| CLI discrepancy | The archive command reported 49 tasks because its counter reads body `Tasks` fields/XML/headings and misses 19 tasks present in actuals. The milestone entry uses 68; historical summaries remain unchanged. |
| Timeline | Opening 2026-10-25 through closeout 2026-10-30, six calendar dates. |
| Delivery comparison | `266eda6576c0dcb588f1be985ea9890ed78371b5` (pre-roadmap opening snapshot) to merged `45a078d2de1f08e39caf1c5942fcdb57f493d8af`: 235 changed files, 40,298 insertions, 859 deletions, including planning records. Three squash delivery commits are not a count of execution task commits. |
| Owned source | 49,089 lines in tracked `.c` files; 54,436 including owned `.h`, excluding vendored `libretro.h`. Core `.c` plus public header: 8,762 lines; including core internal headers: 9,545. |
| Current CI inventory | 695 registered tests; earlier phase CI/package results remain historical evidence. No new full-suite or hosted qualification is claimed by closeout. |

## Delivery and evidence boundaries

Read-only GitHub connector queries during closeout reconfirmed
[PR #61](https://github.com/szTheory/nesturbator/pull/61) merged at
2026-10-30T14:27:46Z, source head `e5e2d7d143619073a3bed4f2f3412834ef1050c8`,
squash merge `45a078d2de1f08e39caf1c5942fcdb57f493d8af`.
[Release PR #56](https://github.com/szTheory/nesturbator/pull/56) is still open,
head `74d443f2b44857c9e366970e1b11b0f923cf4a0d`, proposing 0.1.12 with the v4
baseline/robustness and all three v5 phases. Package publication remains pending;
no downloaded release artifacts are inferred from a merged implementation PR.

The audit retains final-head green CI/Nightly evidence. Original accepted content
bundles remain bound to their recorded candidate/tested-merge/effective-input
identities: Phase 20's 418 inputs and Phase 21's 428 inputs. A local check or green
final-head workflow is not a replacement content/provenance collection. The
closeout branch starts at merged main and carries the completed audit; runtime,
tests, public documentation and workflow bytes are unchanged by archival.

Milestone tagging is disabled (`git.create_tag: false`). No tag, push, PR merge or
release publication is performed by closeout. Existing local AGENTS.md and config
edits are preserved unstaged. The archive commits remain local for normal PR
integration; they do not claim that the new archive is on main.

## WINDOWS representation reconciliation

Historical commit `e80af77b28ad790df7f59d7cf8c9e21e4ec87211` added table row 18
and raised counters to eighteen but omitted the JSON record. The row's fixed
status is supported by the [Phase 16 verification](v4-phases/16-host-evidence-and-roadmap/16-VERIFICATION.md)
and registered `retroarch.session.source_root` test. Closeout restores that
existing row to JSON and maps unsupported kind `coverage` to schema-supported
`unmet-truth` in both views. Status, description, reason and original recorded/
resolved timestamps are preserved. No risk acceptance is invented.

The actual GSD ledger parser now accepts eighteen entries with zero open, four
waived and fourteen fixed. Fresh execution of the registered source-root control
passes. This repairs the representation mismatch; it does not retroactively
invent a previously refused Phase 21 append. The original phase deferred-item
text is retained as historical evidence, with the authorized acknowledgement.

## Preserved debt and limits

The audit and phase dispositions retain original severity and mitigation:

- Phase 20 CR-01, WR-03 and WR-04: recurring Git-mode identity enforcement,
  ignored-fixture/relative-source-root lookup and duplicate validator debt.
- Phase 21 WR-01: actual worktree mode identity gap, also recorded as Phase 20
  WR-05; count that defect once. Independent accepted-run mode/blob checks do not
  repair recurring enforcement.
- Phase 21 WR-02 historical measurement-attribution warning; README's overview
  revision-8/revision-9 mismatch; summary/frontmatter/parser metadata omissions.
- Phase 19 full-state-save presentation cost, sprite comparator hardware citation
  and constructed early-stage coverage limit.
- Local Apple-toolchain nofp symbols, real-campaign minimization proven only by
  controls, and the acknowledged debug/testing records.
- Physical exact-timestamp CPU/PPU ordering remains unmeasured. Judgment-tier
  clean-room/provenance prohibitions stay flagged; no contamination or blanket
  assurance is inferred. Remaining 27 failures and five DRAW entries retain
  their original status, denominator and scope.

The v4 JAM attribution and run-ahead-under-JAM gaps are now delivered. Later
accepted full-vector evidence passes; the older failed run remains history.
No safety stop, source-identity rule, 1800-frame cap, performance threshold or
protected-PASS contract is waived. Older Phase 16 runtime windows are not renewed.

## Product direction at the boundary

The [agreed roadmap snapshot](v5-PRODUCT-ROADMAP.md) marks RM-004 shipped and leaves
Immediate empty of new commitments. Near prefers remaining DMA: five failures
in the former eight-row list, while `$4015 Read`, Misaligned OAM DMA and
`INC $4014` already pass and become protection targets. This preserves the
measured/game-relevant ordering while avoiding re-promising delivered work.
PAL is the main compatibility alternative; smaller collector/documentation
maintenance is also available. Promotion needs fresh exact-key measurement,
owned bus-cycle oracles, state/host parity and budgets, then scope approval.

Mid remains demand/profile-triggered cartridge, host and performance work. Long
remains independently scoped expansion audio, MMC5, FDS and peripherals: v5
provided neither new consumer demand nor regional/hardware acceptance evidence
that would promote them. All four horizons and promotion triggers were
reconsidered here. SEED-003 remains partial, SEED-006 recurring; seed files were
not rewritten. No future phase number or implementation scope is committed.

## Verification performed during closeout

- Canonical `init.manager`: all three phases complete/current/passed before move;
  each plan has a summary; all eleven requirements checked and Complete.
- Fresh existing-tree CTests: state format, hash inventory self-test, IRQ poll,
  interrupt entry and interrupt state: 5/5 passed in 0.22 seconds.
- Fresh campaign/authority/target/no-lost-pass, runner state roundtrip, actual
  adapter run-ahead and registered source-root controls (including fixtures):
  8/8 passed in 39.09 seconds. Raw log: `build/v5-closeout/focused-ctest.log`.
- Phase archive inventory: all 119 files retain their exact post-acknowledgement
  SHA-256 bytes after relocation. All 27 historical summaries and three phase
  verification reports also retain their original pre-closeout bytes. The only
  phase-file change was authorized deferred-item acknowledgement metadata.
- Archive/requirements/outcome/roadmap snapshot and backlog preservation,
  acknowledgement scan, strict ledger parser, local-link validation, diff scope
  and repository hygiene are checked before the archive safety commit.

The original REQUIREMENTS register is retained until archives are committed as a
safety checkpoint, then removed with `git rm` and a final commit. Fresh
requirements are created only when the next milestone scope is selected.

## Next step

`$gsd-new-milestone` selects the next scope and creates fresh requirements and
execution phases. Context may be cleared first; read [STATE.md](../STATE.md) and
[continue.md](../continue.md). No further user action is needed for this local
closeout; integration and package publication remain separate from its outcome.
