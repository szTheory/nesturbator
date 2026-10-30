# Adversarial review: next milestone discovery

Reviewed 2026-10-30: [SYNTHESIS](SYNTHESIS.md),
[PLAYSTEAD-BOUNDARY](PLAYSTEAD-BOUNDARY.md) and the three completed peer notes.
One bounded review; no runtime, new fan-out or broad source rescan.

## Verdict

The recommendation is coherent and appropriately proposed. Prefer **player-ready
NES delivery** as the default next milestone: truthful short onboarding,
preserved topic documentation, qualified artifact/host identity and a concrete
Playstead handoff. Offer **actual Playstead integration** as an explicit wider
alternative with consumer ownership and paired acceptance. Do not imply the
readiness milestone completes that integration.

The proposed reprioritization follows the owner's new interest in trying games
and launcher integration. It does not establish a new hardware defect, waive
existing evidence debt or approve execution phases. DMA remains the next accuracy
candidate and PAL remains independently scoped. Further generic research would
not change this recommendation.

## Finding addressed before presenting a contract

**State behavior revision is recorded, not enforced by the core loader.**
PLAYSTEAD-BOUNDARY's earlier handoff list said “state format/behavior identity and
incompatibility refusal.” That could have implied the core rejects behavior-revision
mismatches. The [public header](../../../include/nesturbator.h), lines 97–99,
explicitly says a state records behavior revision but a load does not enforce it.
Its format and cartridge/board/region checks are separate contracts.

Recommended wording: name current format and format-mismatch refusal; describe
existing ROM/board/region identity checks; record behavior revision for
provenance. A Playstead support row may choose a stricter qualified identity
policy, but that would be consumer policy, not an existing core guarantee.

**Addressed:** reread PLAYSTEAD-BOUNDARY lines 53–59 after the parent's correction.
It now explicitly distinguishes native format/ROM/mapper/submapper/region
refusal, recorded but unenforced behavior revision, and a possible stricter
consumer policy. The material wording issue is resolved; no open blocker remains
in this research synthesis.

## Boundaries to preserve in the final choice

| Risk | Assessment and concrete guard |
|---|---|
| Quietly substituting a document for requested integration | The synthesis correctly makes the distinction. Present the choice once in ordinary product terms: make NES easy to try and ready for the consumer, or also deliver the actual Playstead launch/save/return flow. Wider integration requires a Playstead-owned task and must respect its existing Muhtari priority. |
| Reimplementing existing capabilities | Already avoided: states, rewind/run-ahead, battery and real RetroArch checks are credited. Require a demonstrated package/input/consumer delta before expanding the host harness. |
| Turning voluntary game testing into recurring manual UAT | The short feedback prompt is sound. Keep it optional and conversational; the agent gathers technical identity/logs. Do not make milestone completion depend on an owner questionnaire or unknown commercial folder arrival. Repeatable required behavior uses permitted owned fixtures. |
| Overselling compatibility | The notes distinguish the finite AccuracyCoin score, supported board profiles, an individual play report and full-library behavior. Keep those claims separate. Metal Slug X evidence belongs to Glueyneo's narrow existing scene, not this NES milestone. |
| Documentation split growing into a rewrite | The demonstrated problem is navigation and stale status, not an excuse to redesign every guide/API. Move useful prose and its existing test/release consumers together; retain required warnings, negative controls and install extraction semantics. Arbitrary sibling byte quotas are unnecessary. |
| Spending runtime authority during research | No runtime has been performed or authorized by the proposal. Identify the intended NES/Neo Geo session and selected content first; existing stops and exhausted allowances remain unchanged. Avoid introducing unrelated approval ceremonies for ordinary permitted work. |
| Collector debt becoming another whole milestone by default | Existing defects need repair before a new accepted collection relies on them. The scope is the actual source/mode/lookup boundary, with representative fault detection; no new generalized evidence service or duplicated matrix. |
| Dependency or implementation transfer violating ownership | Correctly rejected: Nesturbator's own-code rule is stricter than Muhtari's permissive-seed rule. Transfer mechanisms and lessons, then implement locally with existing C/CTest/CMake/platform facilities. |

## Recommended final decision

Present the player-ready scope as the preferred **proposal**, explain what it
delivers and explicitly state that actual Playstead qualification remains open.
Let the owner select the wider actual-integration outcome if that is their intent.
Do not ask again about already delegated routine technical choices. Keep the
session's target clear. After this review, the owner confirmed NES here and
explained that the Neo Geo details were pasted context from a parallel milestone
discussion; the synthesis and handoff now record that clarification.

No further adversarial pass is useful without changed scope or new consumer
evidence. This review verifies the recommendation's consistency, not runtime
readiness or behavioral acceptance.
