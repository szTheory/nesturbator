## ISSUES FOUND

**Phase:** 16 — Host evidence and roadmap
**Plans reviewed:** 16-40 and 16-41, plus the live dependency index
**Issues:** 0 blockers, 1 warning, 0 info

### Coverage and plan summary

| Requirement | Plans | Status |
|---|---|---|
| EVID-01 | 40, 41 | Covered |
| EVID-02 | 40, 41 | Covered |
| EVID-03 | 40, 41 | Covered |
| EVID-04 | 40, 41 | Covered |
| DIR-01 | 40, 41 | Covered |
| DIR-02 | 40, 41 | Covered |

| Plan | Tasks | Declared paths | Wave | Dependency | Status |
|---|---:|---:|---:|---|---|
| 40 | 2 | 6 | 10 | 30 | Ready |
| 41 | 3 | 13 | 11 | 40 | Blocked pending 40 |
| 07 | 3 | 1 | 12 | 41 | Blocked pending 41 and separate parent gates |
| 08 | 2 | 1 | 13 | 07 | Blocked pending publication |

The live index contains 14 plans: 10 complete and four incomplete (40, 41, 07, 08). Plans 38 and 39 are excluded as superseded; their bodies remain unchanged. The current dependency chain is 30 → 40 → 41 → 07 → 08, with 40 the only ready plan. The 38-terminal STATE/ROADMAP entry is the expected pre-completion state and is not a missing planning step.

Plan 40 makes substantive corrections: the actual receipt-refusal result and executor-reported B readback mismatch remain distinct; the B result's unavailable parent provenance is preserved; the new A/B algorithm binds `P`, `J_A = P + I`, and `J_B = J_A + C` exactly once; and the same source path must pass a read-only rehearsal before either first write. It then requires a genuine unchanged copied-34 CLI failure, coherent prerequisite repair, and complete prefrozen plus unchanged-final-byte preparation matrices. Plan 41 separately requires its own prewrite rehearsal, fresh complete 12-row admission, current-source acceptance, finite CI/commit/review work, and the independent parent/native/consent/publication/released-byte gates. No present runtime, source, or publication success is claimed.

Task structure is valid for both plans. All tasks have files, actions, verification and measurable completion criteria. The declared key links cover the 40 code-only export into 41, production admission through the finalized 41 bundle, and 41's result into 07. There is no dependency cycle or same-wave coupling. The plans retain all 17 locked decisions and six requirements, leave deferred scope out, retain four flagged assumptions, and introduce no new prohibitions. RESEARCH.md's Open Questions are marked RESOLVED. No architectural responsibility mismatch or current review artifact requiring incorporation was found.

Scope estimates were checked with the calibrated estimate query. Plan 40 is 84,500/100,000 tokens; Plan 41 is 99,900/100,000. Both are low confidence, calibration factor 1, sample count 0, and neither is over budget. Plan 41's 13 paths exceed the 10-path warning threshold. The plan assigns them 5/5/3 across three tasks and explains why dividing the one-owner admission, runtime allowance, and once-only formal/review transaction would change authority or duplicate proof. That rationale makes a safe split unavailable, but it does not erase the fresh warning; Plan 39's prior warning disposition does not transfer.

The supplied verification probes report zero blockers and warnings across 336 commands. The verify-paths probe is `not_applicable`; it is not runtime evidence. No tests, builds, or plan execution were run for this review.

### Warning — current scope disposition

**1. [scope_sanity] Scope above the 10-path warning threshold must receive a fresh disposition.**

- Plan: 16-41
- Evidence: Plan 41 declares 13 unique write paths. Its own amendment states that this is a scope exception and requires fresh review; the 99,900-token estimate is low-confidence with zero calibration samples. The prior Plan 39 warning approval is historical only. The recorded single-owner/once-only rationale supports keeping the paths together, but does not remove the warning.
- Example fix (non-binding): Keep this warning visible for the current gate to accept explicitly, or reduce the write set only if that can be done without splitting or duplicating the locked single-owner acceptance transaction.

### Structured Issues

```yaml
issues:
  - plan: "16-41"
    dimension: "scope_sanity"
    severity: "warning"
    required_property: "A plan's scope above the 10-path advisory threshold receives a current, explicit disposition."
    description: "Plan 41 declares 13 unique write paths. Its amendment identifies the excess as a current scope exception and requires fresh review; the previous Plan 39 warning approval does not apply. The low-confidence estimate is 99,900/100,000 tokens with zero calibration samples."
    task: null
    fix_hint: "The current gate may explicitly accept the documented one-owner exception, or the write set may be reduced if that preserves the same authority and once-only proof boundaries."
```

### Recommendation

No blocker prevents execution. One warning remains for a fresh current disposition; the plans explain why a safe split would violate the single-owner acceptance boundary. Execute 40 first and stop at its administrative reconciliation. 41 requires a separate invocation after genuine 40 success and parent reconciliation.

### Parent disposition after user response

The checker finding above remains unchanged: zero blockers and one scope warning.
The user asked for the recommendation and authorized it. The parent recommended
accepting Plan41's documented13-path exception because splitting the ordered
single-owner authority and once-only proof transaction has no safe transfer in
the current contract. The current warning is accepted for planning completion;
it is not removed, reduced to INFO, or inherited from Plan39's prior approval.
The99,900/100,000-token estimate stays low-confidence with zero calibration
samples. All runtime/source/CI/review/parent/native/consent/publication acceptance
remains pending. Planning verification is accepted with override.

Final coverage:6/6requirements,17/17decisions,39source groups,23/23post-plan items.
Both336-command probes report zero findings; path not_applicable is not runtime
proof. Historical build comparison matched105,538entries/81,682raw hashes/four
metadata-only files and22frozen38bootstrap files. Six existing dirty files and
historical plan bodies remain exact. No40/41root or execution was created.
