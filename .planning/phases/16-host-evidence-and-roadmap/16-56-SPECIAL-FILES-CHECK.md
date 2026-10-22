## ISSUES FOUND

**Phase:** 16 — Host evidence and roadmap  
**Plans checked:** 17 live plans  
**Issues:** 0 blocker(s), 2 warning(s), 0 info

### Coverage Summary

| Requirement | Covering plans | Status |
|---|---|---|
| EVID-01 | 01, 02, 03, 56, 51–54 | Covered by planned work; not accepted evidence |
| EVID-02 | 03, 09, 15, 21, 56, 51–54 | Covered by planned work; not accepted evidence |
| EVID-03 | 04, 56, 51–54 | Covered by planned work; not accepted evidence |
| EVID-04 | 05, 07, 08, 56, 51–54 | Covered, with actual release gates still pending |
| DIR-01 | 06, 56, 51–54 | Covered by the installed direction and preservation gates |
| DIR-02 | 06, 56, 51–54 | Covered by the milestone-boundary contract and retained gates |

### Plan Review

The phase goal and six requirements match `ROADMAP.md` and `REQUIREMENTS.md`. All 17 live plans have valid structure; each has 2–3 tasks and 1–8 planned files, with Files, Action, Verify, and Done present. The custom tag audit found that older Plan 30 expresses acceptance in each measurable `<done>` field rather than separate `<acceptance_criteria>` tags; no Done criterion is missing. Every automated command has a corresponding `<fails_when>`. The declared dependencies are acyclic and wave-consistent: 56 (wave 10) follows 30, 51 (11) follows 56, 52 (12) follows 51, 53/54 (13/14) continue after 52, then 07/08 (15/16). The changed plans claim all six requirement IDs.

Plan 56 has a clear fresh capture authority: `p56-special-metadata-01` binds the new exact source/request/receiver/schema/expected-case closure. It preserves the failed source and partial prefix as immutable data, prohibits retry or proof transfer, and requires the protected baseline to be captured inline before construction. The amendment explicitly assigns that first baseline to the existing inline Node producer and independent caller receiver, so it does not depend on helpers that do not exist yet. Only the already-approved two helpers and construction journal may be created under the earlier construction allowance; the new schema and `--test-special-file-snapshot` are incorporated before journal close and final-source freeze. Task 2 validates and exports those frozen bytes.

The schema covers FIFO 4096, socket 49152, character device 8192, and block device 24576 with exact tagged metadata-only rows and no content fields. It requires raw canonical bigint integer tokens, lossless caller parsing, exact key/type/status validation, complete root and entry coverage, full canonical-byte comparison, bracketed lstat and directory-name stability, and four-kind positive and reason-specific negative controls. It keeps the four prior unreadable regular files as distinct limitations. Controls use in-memory doubles, label synthetic cases as non-authoritative, and require refusal before a substituted special entry receives a content operation. The amendment states that lstat bracketing is a local stability check, not an atomic snapshot or hostile ABA guarantee; that limitation is explicit and does not weaken the required no-content-access behavior for entries classified as special.

Plans 51 and 52 each repeat the full schema, receiver, stability, coverage, and control gate in their own contexts before their own first write, with `p51-special-metadata-01` and `p52-special-metadata-01`. They keep independent read/compare and finite-graph gates, and import code only after their own acceptance. Plan 52 retains the single 52/53/54 native-child owner and 12-row admission sequence. The later 07/08 plans retain exact-head native checks, affirmative consent, normal publication, 18 tag-bound archives/checksums/attestations, six metadata assets, and exact-release CI. No phase success or release acceptance is claimed by these plans.

The special-file correction honors the locked D-01–D-17 context, preserves deferred ideas, and introduces no third helper, dependency, new runtime owner, or release shortcut. The RESEARCH.md responsibility map has no conflicting tier assignment for this evidence-preparation tooling; its open questions are marked resolved. Nyquist validation is explicitly disabled in project config, and no phase `PATTERNS.md` exists. The six originally dirty paths, Plan 26 pending state, historical unknowns, and later consent/publication/released-byte gates remain explicit preservation duties.

### Scope and Tool Results

| Plan | Tasks | Files | Wave | Estimate / budget | Result |
|---|---:|---:|---:|---:|---|
| 01 | 2 | 6 | 1 | 38,000 / 100,000 | Under budget; low confidence |
| 02 | 2 | 5 | 2 | 30,000 / 100,000 | Under budget; low confidence |
| 03 | 3 | 7 | 3 | 42,000 / 100,000 | Under budget; low confidence |
| 04 | 2 | 5 | 4 | 24,000 / 100,000 | Under budget; low confidence |
| 05 | 2 | 6 | 5 | 22,000 / 100,000 | Under budget; low confidence |
| 06 | 2 | 1 | 1 | 12,000 / 100,000 | Under budget; low confidence |
| 07 | 3 | 1 | 15 | 24,000 / 100,000 | Under budget; low confidence |
| 08 | 2 | 1 | 16 | 18,000 / 100,000 | Under budget; low confidence |
| 09 | 3 | 8 | 6 | 44,000 / 100,000 | Under budget; low confidence |
| 15 | 2 | 4 | 7 | 34,000 / 100,000 | Under budget; low confidence |
| 21 | 3 | 7 | 8 | 42,000 / 100,000 | Under budget; low confidence |
| 30 | 2 | 1 | 9 | 42,000 / 100,000 | Under budget; low confidence |
| 51 | 2 | 6 | 11 | 75,000 / 100,000 | Under budget; low confidence |
| 52 | 2 | 6 | 12 | 75,000 / 100,000 | Under budget; low confidence |
| 53 | 3 | 6 | 13 | 43,000 / 100,000 | Under budget; low confidence |
| 54 | 2 | 4 | 14 | 31,000 / 100,000 | Under budget; low confidence |
| 56 | 2 | 8 | 10 | 104,000 / 100,000 | Over budget by 4%; low confidence; recommendation: about 2 slices |

The prescribed calibrated estimate checks returned factor 1 and sample count 0 for every plan, so all figures are low-confidence projections. Plan 56's recommendation is advisory and grants no authority for a new split or removal of any gate. Plans 51–54 are each below the per-plan budget, while the explicitly continuous 52–54 runtime-group estimate totals 149,000; actual provider/context fit remains unknown. The total 328,000 projection spans separate invocations and is not treated as one context budget.

The supplied `verify-paths.json` is `not_applicable` with 455 commands, zero findings, and no read error; it is not runtime path proof. The supplied `failure-directions.json` is `ok` with 455 commands, zero findings, and no read error. No listed command was executed. The supplied eight-edge delta remains unresolved as recorded: EVID-02 idempotency/concurrency and EVID-03 empty/encoding have explicit plan controls; EVID-01, EVID-04, DIR-01, and DIR-02 remain flagged unclassified assumptions.

### Warnings

**1. [scope_sanity] A plan estimate over the smart-zone budget should be reviewed before execution.**
- Plan: 56
- Evidence: The calibrated check returns 104,000 tokens against the 100,000-token budget (1.04×), with low confidence, factor 1, and zero calibration samples. The plan has 2 tasks and 8 files, within the structural thresholds, and its estimate is explicitly a projection rather than a fit guarantee.
- Example fix (non-binding): Reassess the estimate or use a scope-preserving execution strategy before dispatch; any new plan split requires its own authority, and no required gate should be dropped to fit a number.

**2. [scope_sanity] The same-child runtime group’s projected context exceeds the smart-zone budget.**
- Plans: 52, 53, 54
- Evidence: Their stated estimates total 149,000 tokens for the single continuous 52/53/54 runtime group, above the 100,000-token budget. Individual plan estimates are below budget, but they do not establish that the shared invocation fits; calibration confidence is low with zero samples. The plans preserve all duties and require a stop if the complete work cannot fit.
- Example fix (non-binding): Establish an authorized, scope-preserving context-fit strategy for the same-child group before dispatch, or stop for a concrete amendment if actual context constraints prevent completion.

### Structured Issues

```yaml
issues:
  - plan: "56"
    dimension: "scope_sanity"
    severity: "warning"
    required_property: "An execution estimate above the smart-zone budget is reviewed before dispatch without reducing locked duties."
    description: "The calibrated estimate is 104000 tokens against a 100000-token budget (1.04x). Confidence is low, calibration factor is 1, and sample_count is 0; the estimate does not establish actual context fit."
    fix_hint: "Reassess the estimate or use a scope-preserving execution strategy; any new plan split requires separate authority."
  - plan: null
    plans: ["52", "53", "54"]
    dimension: "scope_sanity"
    severity: "warning"
    required_property: "The complete same-child runtime group has an execution-context strategy that preserves every required gate."
    description: "The continuous 52/53/54 group is projected at 149000 tokens against the 100000-token smart-zone budget. Individual plans are under budget, but the group executes as one native-child runtime invocation; actual fit is unknown and the estimate is low-confidence."
    fix_hint: "Establish an authorized scope-preserving context-fit strategy before dispatch; stop for a concrete amendment if the full duty set cannot fit."
```

### Recommendation

0 blockers and 2 warnings require review. The plans preserve the phase goal and the fresh metadata-only capture authority; the remaining concern is projected context fit, not missing schema or evidence gates. No execution proof is claimed. Verification remains `gaps_found` until the planned work and later release gates produce actual evidence.
