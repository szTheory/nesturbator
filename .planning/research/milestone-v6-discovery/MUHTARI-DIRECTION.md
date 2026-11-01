# Muhtari direction lessons for Nesturbator

Inspected: 2026-10-30. Research input to the next milestone; no implementation,
runtime acceptance or scope approval is claimed.

## Provenance and boundaries

Muhtari HEAD was `a0834db21b2b22f30d2147458aeec27df6ae1530`. Its working tree
also had modified config, preparation DECISIONS/GSD-HANDOFF and an untracked
packaged-input debug record. Findings from those working files are current local
observations, not statements that the commit contains their exact bytes. No
emulator execution, ROM/BIOS inspection or prohibited emulator-source reading was
performed. External sources were unnecessary for this pass: the owner-intent
records, implementations' public documentation and evidence ledgers are primary
sources for the decisions assessed here.

Read source intent before transferring mechanics:

- [Muhtari INTENT](../../../../muhtari/.planning/INTENT.md), lines 8–38:
  ordinary play, safe progress and easy setup precede AI/TAS; the frontend owns
  windows/devices. Voluntary gameplay feedback is scoped evidence, not recurring
  owner verification.
- [INTENT](../../../../muhtari/.planning/INTENT.md), lines 40–95: rolling
  horizons, bounded role-specific research, local lesson recall, uncertainty and
  dependency ownership. These are owner preferences, not technical proof.
- [Muhtari Phase 12 context](../../../../muhtari/.planning/phases/12-player-ready-delivery-and-playstead-bridge/12-CONTEXT.md),
  D-01–D-11: accepted discussion decisions. They are not delivered outcomes.
- [Current Muhtari STATE](../../../../muhtari/.planning/STATE.md): Phase 12 has
  zero accepted plans and an actual-host input failure. The later readiness and
  state plans must not be reported as shipped merely because research exists.

## Recommendations

| Decision | Disposition here | Recommendation and tradeoff | Evidence / reconsider when |
|---|---|---|---|
| Put usable play before another score increment | Scope next | Propose a bounded player-readiness/Playstead bridge milestone ahead of the five DMA repairs, with current release truth, straightforward onboarding and one consumer-owned integration proof. Nesturbator already has portable states, rewind and run-ahead, so do not import Muhtari's state implementation phases. This delays accuracy points but gives feedback on what actually blocks play. Keep DMA preferred after readiness unless a reproduced title failure changes priority. | Muhtari INTENT lines 8–24; Nesturbator [PROJECT](../../PROJECT.md), Current State and Next Milestone Goals. New user direction changes product priority; it does not prove that DMA caused a played-title defect. |
| README audience and first useful action | Adopt in next scope | Lead with NES play/current limitations, download/install, Load Core **and Load Content**, controls, saves and a short docs ladder. Add GUI and quoted CLI happy paths. Preserve useful integration/build/testing prose in focused `docs/` pages rather than discarding it. Avoid a new docs framework. | Muhtari [README](../../../../muhtari/README.md), lines 1–61; [onboarding lesson](../../../../muhtari/.planning/LESSONS.md), lines 200–234. Nesturbator [README](../../../README.md) is 2,421 lines / 159,794 bytes; install is at line 2024 and stops at the test card at 2037–2038. |
| Documentation truth and migration | Adopt now as research finding; implement with scope | Fix stale status/revision/count prose as part of the docs change. Separate published package capabilities from merged milestone capabilities. Preserve machine-tested contracts when relocating sections: runner options/status tables, save-state warnings, info/archive claims and install-line consumers currently read README directly. Move their authoritative target and links together, exercising existing positive/fault consumers. | Nesturbator README lines 6–29 says v3 and revision 8; [public header](../../../include/nesturbator.h), lines 55–100 pins behavior 9/state format 3. Existing consumers: `tests/cmake/{runner_exit_table,runner_options,status_table,info_claims,check_archives,check_install_line}.cmake`; release-please config and CI reference README too. |
| Public header brevity | Scope later / small opportunity | Keep ABI/error/ownership semantics discoverable at the API. Move lengthy historical revision/hardware explanation only if the docs migration makes maintenance easier; do not strip callers' required contract or change ABI for presentation. Muhtari's exact byte ceilings are its own choices, not evidence of the right NES limit. | Muhtari [ENGINEERING](../../../../muhtari/.planning/preparation/ENGINEERING.md), lines 242–247. Nesturbator header is 766 lines / 52,677 bytes. A size alone does not demonstrate a correctness defect. |
| Playstead quick path and ownership | Scope next | Reuse external RetroArch with separately verified app/core artifacts; Nesturbator owns core/release documentation and capability evidence, Playstead owns adapter/install/cache/focus/input/process/storage/UI. Prepare a source-linked handoff and paired acceptance boundary. Avoid implementing a launcher in this core repository. | Muhtari [PLAYSTEAD-HANDOFF](../../../../muhtari/.planning/research/PLAYSTEAD-HANDOFF.md), lines 29–50; Phase 12 D-02–D-08. Parent discovery is checking the current Playstead tree; Muhtari's earlier inspected revision was dirty and must not be treated as today's consumer. |
| Exact Continue versus recent play | Adopt contract distinction | Start with existing RetroArch-managed save/load. Recent-play/relaunch does not mean exact restore. Playstead-managed Continue/history needs identity, a supported capture/load seam, and an explicit policy for loading an older state over newer durable save progress. Core serialization support alone cannot prove host custody. | Muhtari handoff lines 52–67; Nesturbator [README](../../../README.md), Save states and libretro sections. Nesturbator already warns that rewind reverts save RAM. Native host/state history remains later unless current consumer friction justifies it. |
| Packaged host evidence | Adopt next scope where a new claim needs it | Distinguish build-tree core, candidate extracted archive and published downloaded bytes. Pin the separately supplied RetroArch release and record the tested source/config/core identity. An actual Playstead launch/input/orderly-return/save result needs paired consumer evidence; runner or loaded-module tests are supporting evidence only. Reuse existing packaged install/RetroArch controls instead of rebuilding a broad host matrix. | Muhtari Phase 12 D-04–D-07 and handoff lines 46–50. Nesturbator already has actual RetroArch progression/state/battery tests; preserve credit for those instead of claiming host verification starts from zero. |
| CI/process efficiency | Adopt principles; require local mechanism | For a demonstrated recurring bottleneck, profile the existing consumer, batch local fixes and obtain comparable complete before/after work. Audit every required job before attributing failure to one known issue. Preserve advisory distinctions, failed attempts, exact inventories and source binding. Do not import arbitrary two-round limits or performance quotas without a NES decision. | Muhtari [GSD-HANDOFF](../../../../muhtari/.planning/preparation/GSD-HANDOFF.md), lines 114–143; [LESSONS](../../../../muhtari/.planning/LESSONS.md), lines 107–120. Nesturbator already has CI-overhead quick tasks and collector debt in [STATE](../../STATE.md). |
| Lessons memory | Adopt small local record | Keep source, mechanism, confidence, local differences, evidence and revisit trigger in a discoverable local ledger/index. Instructions should direct agents to relevant entries before repeating work. A sibling repair remains a hypothesis until reproduced here. Do not add a memory service, mandatory export ritual or full cross-repo framework. | Muhtari [LESSONS protocol](../../../../muhtari/.planning/LESSONS.md), lines 13–40; INTENT lines 55–68. Some Muhtari lessons originated in Nesturbator; transferring them back as new capabilities would double-count existing work. |
| Rolling roadmap | Already adopted; refresh priority | Nesturbator's four-horizon ledger already implements the main Muhtari lesson. Update the proposed near-term ranking for the user's new play/integration intent; leave execution phases uncommitted until the milestone approval. Preserve partial outcomes, debt, package/publication distinctions and triggers. | Nesturbator [PRODUCT-ROADMAP](../../PRODUCT-ROADMAP.md), lines 113–142; Muhtari [PRODUCT-ROADMAP](../../../../muhtari/.planning/PRODUCT-ROADMAP.md), lines 10–81. No new roadmap service or governance artifact is needed. |
| Dependency philosophy | Already adopted; retain stricter NES rule | Reuse existing code and platform facilities first; a small owned helper or short stable duplication can be preferable to a package. Compare real security/update/correctness cost, not dependency count alone. Transfer mechanisms and intent, not Muhtari emulator code: Nesturbator's own-code rule permits only vendored `libretro.h`, stricter than Muhtari's permissive-seed policy. | Nesturbator [AGENTS](../../../AGENTS.md), Rules 1/4 and Style; Muhtari ENGINEERING lines 257–291; INTENT lines 70–80. Reject a shared emulator framework or copied CPU/hardware implementation as an incidental lesson transfer. |

## One adversarial synthesis

The attractive failure mode is to turn “let me play games and feed back” into
another large test-governance milestone. Keep the product outcome small and
observable: a person can install the qualified core, load supported NES content,
use controls, keep progress and return to the launcher. Automated recurring checks
belong at actual failure boundaries; a useful voluntary spot check remains welcome
without becoming a mandatory owner acceptance ritual.

Muhtari supplies a caution, not a ready-made input solution. Its current actual
RetroArch input proof failed although local tests passed; the released binary did
not supply the assumed optional upstream test driver. Its
[LESSONS](../../../../muhtari/.planning/LESSONS.md), lines 123–128 and current
STATE preserve that gap and spent gates. Copying the proposed technique, inferring
availability from upstream docs, or spending an old retry window here would repeat
the mistake. Nesturbator's existing real-host path needs a targeted delta analysis,
not replacement based on a sibling's unfinished plan.

Likewise, a README split is not a broad technical rewrite. The current executable
documentation consumers and release tooling create a concrete migration seam;
preserve each required contract while moving its authoritative prose. Do not hide
stale product claims behind a new landing page or silently relax assertions to make
the split pass. Keep collector source/mode identity repairs visible before any new
qualification relies on them.

## Remaining questions that could change scope

1. Resolved after this pass: the owner confirmed **Nesturbator (NES) here** for
   commercial-game testing. Content-folder location remains pending; no runtime
   testing was attempted here.
2. What current Playstead adapter/readiness/state capability exists for NES? Parent
   research owns the current consumer inspection; Muhtari's handoff is a pattern,
   not current NES integration evidence.
3. Is the next milestone intended to prove actual Playstead integration or only
   prepare a qualified core/handoff? Those have different repository ownership and
   acceptance, and should not be conflated.
4. Which current published package will be used for the first player session?
   Merged v5 behavior and the pending package release are separate evidence claims.

Further general fan-out would not resolve those consumer/authority questions.
The useful next work is synthesis with the actual Playstead inspection and the
other sibling passes, followed by one concrete milestone scope choice.
