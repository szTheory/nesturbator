# Independent revision check: Phase 16 gap plans 42/43

## ISSUES FOUND

**Phase:** Host evidence and roadmap  
**Plans checked:** 42 and 43, with active successor contract review of 07 and 08  
**Status:** 0 blockers, 1 warning, 0 info

The prior dependency blocker is resolved. The live prerequisites now bind to artifacts and actual parent results on the 42 → 43 → 07 → 08 path. Plan 43's fresh scope warning remains open for owner disposition; the historical Plan 41 scope approval does not transfer.

## Goal-backward coverage

Phase 16's six requirement IDs remain represented by both new gap plans, and the existing host and direction evidence remains credited from the executed plans. This recheck does not claim any new execution, acceptance, publication, or phase completion.

| Requirement | Plans | Status |
|---|---|---|
| EVID-01 | 42, 43; prior host plans | Retained; exact progression, history, and origin controls remain required |
| EVID-02 | 42, 43; prior host plans | Retained; retry limits, one-shot actions, and cleanup controls remain required |
| EVID-03 | 42, 43; prior fixture plans | Retained; exact basename and supplied-byte identity remain required |
| EVID-04 | 42, 43 → 07 → 08 | Retained; fresh native gates, consent, publication, and released-byte evidence remain blocking |
| DIR-01 | 42, 43; prior direction plan | Retained; no future-priority or future-compliance claim is introduced |
| DIR-02 | 42, 43; prior direction plan | Retained; milestone-boundary upkeep remains within the invoked step |

The revision preserves all D-01 through D-17 decisions, the 39 source groups (3 goal, 6 requirement, 13 research, 17 context), and the four explicitly flagged unresolved assumptions. The four other fallback edges remain explicit inherited truths. It adds no requirement or deferred idea. Plan 40 remains terminal incomplete 0/2; Plan 41 remains unexecuted 0/3. Their bodies and prior evidence remain historical and unchanged.

## Dependency and artifact contract

The active order is acyclic and wave-consistent: completed Plan 21 (wave 8) → Plan 30 (wave 9) → Plan 42 (wave 10, depends on 30) → Plan 43 (wave 11, depends on 42) → Plan 07 (wave 12, depends on 43) → Plan 08 (wave 13, depends on 07).

Plan 42 requires a genuine full preparation before it writes its success-only `16-42-PREPARATION.md` and `16-42-SUMMARY.md`. After executor `FINAL`, its parent performs a read-only administrative reconciliation of dispatch, result, export, journal, history, and inputs. The plan explicitly stops at this handoff; it does not grant runtime acceptance or start Plan 43 automatically. Plan 43 requires those actual 42 outputs and reconciliation, then a separately invoked dispatch.

Plan 43 produces the five named successor handoff records: `16-43-RECONCILIATION.md`, `16-43-SOURCE-ACCEPTANCE.md`, `16-43-REVIEW.md`, `16-43-REVIEW-DISPOSITION.md`, and a success-only `16-43-SUMMARY.md`. Plan 43 separately retains every actual parent gate result under its validation evidence and requires same-candidate binding. Local success cannot substitute for pending parent results.

Plan 07 now requires and reads the actual 42 preparation, summary, and parent reconciliation; all five genuine 43 handoff records; and every actual separate 43 parent result. It requires one candidate identity and equality of all ten tested, committed, and current blobs. It explicitly maps historical Plan 34/35 duties to the new 42/43 evidence and says no 34/35 success summary, source acceptance, review, or disposition may be demanded, synthesized, or credited. Thus the former stale Plan 35 read-path mismatch is closed.

Plan 08's active dependency audit now names `21 → 30 → 42 → 43 → 07 → 08`; the old 30 → 34 → 35 → 07 → 08 chain appears only under a heading explicitly marked historical/superseded. Its Plan 07 prerequisite preserves actual normal publication. It retains checks for the actual 18 archives, exact 18-row checksum manifest, source attestations, all six libretro metadata files, and CI bound to the exact release commit. Plan 07 continues to require fresh exact-final-head native required/performance/pinned-host checks, affirmative consent for the exact PR head, and the normal release-please path. Historical passes, local evidence, and planning approval do not satisfy these gates.

## Structure and deterministic probes

`verify.plan-structure` reports valid structure for both plans. Plan 42 has two tasks and Plan 43 has three; each task has the required files/action/verify/done fields. The supplied path-resolvability probe was read in full: status `not_applicable`, 347 commands, no findings, including all six Plan 42 and five Plan 43 commands. This is not a path-resolvability or runtime pass. The supplied failing-direction probe was read in full: status `ok`, 347 commands, no findings, including those same 11 current-plan commands. These probe results are used as supplied and were not re-derived.

## Warning

**1. [scope_sanity] A new scope exception above the per-plan file warning threshold has a current disposition; a prior plan's approval cannot authorize it.**

- Plan: `16-43-PLAN.md`
- Evidence: Plan 43 declares 13 paths across three tasks with 5/5/3-path ownership, above the 10-path warning threshold. Its estimate is 99,900 tokens against the 100,000-token smart-zone budget, with low confidence, factor 1, and sample count 0. The plan's scope section says the current exception still requires fresh parent review and that no safe independent split is supported. This records mitigation and the warning; it is not an owner disposition. The estimate is below budget and is not an over-budget finding.
- Required property: A new scope exception above the per-plan file warning threshold has a current disposition; a prior plan's approval cannot authorize it.
- Example fix (non-binding): Obtain a fresh owner disposition for this 13-path scope, or revise the plan while preserving its single-owner, ordering, finite-authority, and review-through-completion constraints.

## Structured issues

```yaml
issues:
  - plan: "16-43-PLAN.md"
    dimension: scope_sanity
    severity: warning
    required_property: "A new scope exception above the per-plan file warning threshold has a current disposition; a prior plan's approval cannot authorize it."
    description: "Plan 43 declares 13 paths across three tasks (5/5/3) and estimates 99,900 tokens against the 100,000-token smart-zone budget, with low confidence, factor 1, and zero calibration samples. The plan states that a fresh parent disposition is still required and that no safe independent split is supported; no current owner disposition is present. The estimate is under budget."
    fix_hint: "Obtain a fresh owner disposition for the current scope, or revise it only if every single-owner, ordering, finite-authority, and review binding remains intact."
```

## Recommendation

The previous dependency blocker is resolved; no dependency revision is required. One scope warning remains open for fresh owner disposition. This check does not authorize execution or claim phase acceptance/completion.

## Owner disposition and planning completion

The owner answered "yes" to the explicit recommendation to accept Plan43's
current13-path scope exception: three tasks with5/5/3-path ownership and a
99900/100000-token estimate, low confidence, factor1 and zero calibration
samples. This is a fresh disposition for43; no39/41approval transfers. Both
independent checks found no safe split that preserves dispatch-bound admission,
the sole runtime allowance and once-only formal/review closure. The warning
remains recorded, with planning accepted by owner override. The blocker remains
resolved; no requirement, deadline, slot, acceptance test or later gate is waived.

Planning verification: passed with scope override. This approval permits
finalizing and committing the planning artifacts only.42executes first and
stops at genuine full preparation plus actual administrative parent handoff;
43requires a separately invoked step. All current-source/runtime/formal/parent,
native exact-head CI, affirmative exact-PR/head release consent, normal
publication and actual released-byte checks remain pending or blocked.
