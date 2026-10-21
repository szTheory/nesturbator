# Independent plan check: Phase 16 gap plans 42/43

## ISSUES FOUND

**Phase:** Host evidence and roadmap  
**Plans checked:** 42 and 43, with downstream contract review of 07 and 08  
**Issues:** 1 blocker, 1 warning, 0 info

The plans preserve the Phase 16 decisions and historical evidence, but the live downstream release plans still contain prerequisites for Plan 35 that the new 42 → 43 chain does not produce. Plan 43 also carries a fresh scope warning that remains unresolved; the earlier Plan 41 disposition does not apply to it.

## Goal-backward coverage

The phase goal remains current-host evidence, truthful released MMC3 metadata, and an evidence-backed rolling-direction contract. The six requirement IDs appear in both new plans' `requirements` fields. The phase's existing host and direction work remains credited; EVID-04 stays gated by the downstream native/consent/publication work in 07 and the downloaded-byte checks in 08.

| Requirement | Plan coverage | Status |
|---|---|---|
| EVID-01 | 42, 43; prior host plans | Retained; exact progression and origin controls remain required |
| EVID-02 | 42, 43; prior host plans | Retained; retry and one-shot action limits remain explicit |
| EVID-03 | 42, 43; prior fixture plans | Retained; basename and supplied-byte identity remain explicit |
| EVID-04 | 42, 43 → 07 → 08 | Retained; fresh exact-head CI/consent/publication and 18-archive/six-metadata evidence remain blocking |
| DIR-01 | 42, 43; prior direction plan | Retained; no future-priority claim is introduced |
| DIR-02 | 42, 43; prior direction plan | Retained; milestone-boundary upkeep remains inside the invoked step |

The current context coverage remains 17/17 locked decisions. The 39 source groups are retained. All eight fallback edges remain accounted for: EVID-02 idempotency/concurrency and EVID-03 empty/encoding are explicit inherited truths; the other four remain flagged unresolved assumptions. No deferred idea, new SPEC, or new requirement is added.

Plan 42 keeps Plan 40's terminal-incomplete status, the unavailable failed-tool envelope/source/exit/raw-output boundary, the occupied bootstrap, Plan 26's active pending diagnosis, historical UNKNOWNs, the four metadata-only files, and the original six dirty paths. Its new literal mode oracle distinguishes read-only authority=false, successful durable-operation authority=true, and refusal=false; it keeps operation authority separate from runtime admission. The task also requires prospective-source positive/negative controls in addition to the retained-envelope check. Plan 42 exports preparation code only and requires a separate invocation and administrative reconciliation before 43.

Plan 43 requires its own pre-write oracle and fresh ordered 12-row admission, then retains the source, lifecycle, formal, full-CI-before-commit, ten-blob, independent-review, parent, native, exact-head consent, normal publication, and 08 released-byte duties. The plans do not transfer executor acceptance from 40/41 or treat 42 preparation as runtime admission.

The declared dependency graph is acyclic and ordered as intended: 42 (wave 10, depends on 30) → 43 (wave 11, depends on 42) → 07 (wave 12, depends on 43) → 08 (wave 13, depends on 07). The plan-structure/schema result supplied for this review passes. The 42/43 task actions contain concrete files, actions, verification and completion criteria. The supplied failure-direction probe reports `status=ok` with no findings for all 347 commands; its current-plan subset is 11 commands, all `ok`.

The supplied command-path probe reports `status=not_applicable` for all 347 commands, including the 11 current-plan commands. It provides no path-resolvability verdict, so this check treats it as neither a path pass nor runtime evidence. RESEARCH.md marks open questions resolved and contains no Architectural Responsibility Map; no phase PATTERNS.md was found. AGENTS.md conventions are reflected in the paired source/test/documentation scope, protected config, and no-new-dependency boundary.

## Blocker

**1. [dependency_correctness] Every live successor prerequisite must be satisfied by the declared predecessor's planned outputs or explicitly mapped to an equivalent artifact.**

- Plans: 16-43, 16-07, 16-08
- Evidence: [16-07-PLAN.md](16-07-PLAN.md#L5) now declares `depends_on: ["16-43"]`, but its current prerequisite and task precondition still demand genuine Plan 35 `SUMMARY`, `SOURCE-ACCEPTANCE`, `REVIEW`, and `REVIEW-DISPOSITION` artifacts ([16-07-PLAN.md](16-07-PLAN.md#L106), [16-07-PLAN.md](16-07-PLAN.md#L117)). Plan 43's planned outputs are named `16-43-*`, including its own source acceptance, review, disposition, reconciliation, and success-only summary ([16-43-PLAN.md](16-43-PLAN.md#L204)); it does not plan to produce Plan 35 artifacts. Plan 08's active dependency audit still names the 30 → 34 → 35 → 07 → 08 chain ([16-08-PLAN.md](16-08-PLAN.md#L206)), despite its new scheduling metadata placing 07 after 43. The scheduling note that 43 instantiates Plan 35's duties does not map the literal 35 artifact prerequisites to 43 outputs.
- Required property: A successor's executable prerequisites and read paths resolve to artifacts produced by its declared predecessor, or explicitly identify the predecessor artifacts that fulfill the inherited duty.
- Example fix (non-binding): Rebind 07's precondition, `read_first`, and active-chain text to 43's genuine outputs and parent reconciliation, with an explicit mapping of the retained Plan 35 duties; update 08's active-chain paragraph to the 42 → 43 → 07 → 08 schedule while preserving historical text as historical.

## Warning

**1. [scope_sanity] Plan 43's current scope exception needs a fresh disposition; the prior Plan 41 warning approval does not transfer.**

- Plan: 16-43
- Evidence: Plan 43 declares 13 paths across three tasks with 5/5/3-path ownership, exceeding the 10-path warning threshold. Its calibrated estimate is 99,900 tokens against the 100,000-token smart-zone budget (99.9%), with confidence `low`, factor 1, and sample count 0. The plan itself says this is a new warning requiring fresh parent review and that historical approval does not transfer ([16-43-PLAN.md](16-43-PLAN.md#L210), [16-43-PLAN.md](16-43-PLAN.md#L214)). The estimate is under the configured ceiling, so this is not an over-budget finding. I found no safe independent plan split: the plan binds admission/provenance, paired correction, and once-only formal/review work to one executor, clock, and acceptance transaction ([16-43-PLAN.md](16-43-PLAN.md#L218)). That rationale explains the retained scope but does not itself resolve the fresh warning.
- Required property: A new scope exception above the per-plan file warning threshold has a current disposition; a prior plan's approval cannot authorize it.
- Example fix (non-binding): Obtain a fresh disposition for the 13-path/99,900-token scope, or revise the scope only through a split that preserves all single-owner, once-only, ordering, and review-through-completion constraints.

## Structured issues

```yaml
issues:
  - plan: "16-07-PLAN.md"
    plans: ["16-43-PLAN.md", "16-07-PLAN.md", "16-08-PLAN.md"]
    dimension: dependency_correctness
    severity: blocker
    required_property: "A successor's executable prerequisites and read paths resolve to artifacts produced by its declared predecessor, or explicitly identify the predecessor artifacts that fulfill the inherited duty."
    description: "Plan 07 depends on Plan 43 but its precondition/read_first still require genuine Plan 35 SUMMARY, SOURCE-ACCEPTANCE, REVIEW, and REVIEW-DISPOSITION artifacts. Plan 43 plans only 43-named acceptance/review/reconciliation outputs and a 43 summary. Plan 08's active dependency audit also still names the 30→34→35→07→08 chain. The new scheduling metadata does not map the literal Plan 35 artifact prerequisites to Plan 43 outputs, so the live 07→08 path cannot be shown satisfiable from the declared plan set."
    fix_hint: "Bind 07's executable prerequisites to the genuine 43 outputs and parent reconciliation, explicitly map inherited Plan 35 duties to those outputs, and update 08's active chain."
  - plan: "16-43-PLAN.md"
    dimension: scope_sanity
    severity: warning
    required_property: "A new scope exception above the per-plan file warning threshold has a current disposition; a prior plan's approval cannot authorize it."
    description: "Plan 43 declares 13 paths across 3 tasks (5/5/3) and estimates 99,900 tokens against the 100,000-token smart-zone budget, with low confidence, factor 1, and zero calibration samples. The plan acknowledges a fresh scope warning and says historical approval does not transfer, but no current disposition is present. No safe independent split is supported by the stated single-owner, once-only acceptance boundaries."
    fix_hint: "Obtain a fresh disposition for the current scope, or revise scope only if every finite authority, ordering, and review binding remains intact."
```

## Recommendation

One blocker and one warning require disposition. Return the plan set for revision on the 07/08 artifact contract. Keep Plan 43's scope warning open for a fresh decision; no safe independent split was identified, and the prior Plan 41 warning approval is not applicable. Do not mark this plan set passed or execute 07/08 against the stale Plan 35 prerequisites.
