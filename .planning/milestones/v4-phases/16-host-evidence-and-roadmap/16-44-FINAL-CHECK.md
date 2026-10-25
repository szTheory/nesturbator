## ISSUES FOUND

**Phase:** 16-host-evidence-and-roadmap
**Plans checked:** 16-44 and 16-45, plus the parent scope disposition
**Issues:** 0 blocker(s), 1 warning, 2 info

The parent disposition is present and satisfies the required property: it accepts both 45 scope warnings as visible advisories, explains why the one-owner plan remains, and does not claim owner approval, predecessor approval, context fit, or execution authority. This matches `context-budget.md`, which says the over-budget check is advisory and never blocks. The disposition preserves the phase gates and does not reduce scope.

The scope numbers remain visible: 45 has 13 paths across 3 tasks with 5/5/3 per-task footprints; its estimate is 102,900 tokens against a 100,000-token smart-zone budget (+2,900), with factor 1, sample count 0, and low confidence. Plan 44 remains 93,500 tokens, and the combined estimate is 196,400. The record says to keep both warnings visible and leaves all runtime, consent, publication, and release gates intact.

However, stale pending-disposition language remains in the plans, so the record is not reflected consistently everywhere. `16-45-PLAN.md:303` still calls them “CURRENT OPEN scope warnings for parent review,” despite `16-45-PLAN.md:309` saying they are parent-disposed. `16-45-AMENDMENT.md:119` also says “CURRENT OPEN warnings for parent review,” despite its opening disposition text saying they are parent-disposed. `16-44-AMENDMENT.md:178` still says to carry a new warning for independent checker/parent review. These are status inconsistencies, not missing approval or a reason to block on the advisory itself.

No hidden scope addition, scope reduction, predecessor approval transfer, or weakening of the actual acceptance/publication gates was found. The earlier transport review remains closed; this targeted check did not repeat transport or execution validation. Prior CHECK and RECHECK reports remain untouched.

### Advisory information

**1. [scope_sanity] 45 retains the accepted visible path-count advisory.**

- Plan: 16-45
- Evidence: 13 declared paths exceed the 10-path advisory threshold; the parent disposition accepts retaining the visible advisory.
- Required property: The path-count advisory remains visible and is not treated as owner approval or a context-fit guarantee.

**2. [scope_sanity] 45 retains the accepted visible smart-zone advisory.**

- Plan: 16-45
- Evidence: 102,900 tokens exceed the 100,000-token estimate budget by 2,900; the estimate is low-confidence with factor 1 and sample count 0. The parent disposition accepts retaining the visible advisory.
- Required property: The estimate warning remains visible and is not treated as a context-fit guarantee.

### Warning

**1. [context_compliance] The plans must state the recorded disposition consistently.**

- Plans: 16-44 and 16-45
- Evidence: Current-plan text still describes the warnings as open or awaiting parent review after the parent disposition has accepted them as visible advisories.
- Required property: All current scope-status descriptions agree with `16-44-SCOPE-DISPOSITION.md`: the advisories remain visible, their disposition is recorded, and they do not imply owner approval or execution authority.
- Example fix (non-binding): Replace stale pending-review wording with a reference to the parent disposition while preserving both numeric advisories.

### Structured issues

```yaml
issues:
  - plan: "16-45"
    dimension: "scope_sanity"
    severity: "info"
    required_property: "The path-count advisory remains visible and is not treated as owner approval or a context-fit guarantee."
    description: "The 13-path plan exceeds the 10-path advisory threshold; the parent disposition accepts retaining this visible advisory."
    fix_hint: "Keep the numeric advisory visible in downstream status."
  - plan: "16-45"
    dimension: "scope_sanity"
    severity: "info"
    required_property: "The estimate warning remains visible and is not treated as a context-fit guarantee."
    description: "The 102900-token estimate exceeds the 100000-token budget by 2900 with low confidence, factor 1, and sample_count 0; the parent disposition accepts retaining this visible advisory."
    fix_hint: "Keep the numeric advisory and low-confidence status visible in downstream status."
  - plan: "16-44, 16-45"
    dimension: "context_compliance"
    severity: "warning"
    required_property: "All current scope-status descriptions agree with the recorded parent disposition while retaining the numeric advisories and withholding owner approval or execution authority."
    description: "16-45-PLAN.md:303 and 16-45-AMENDMENT.md:119 still call the warnings open for parent review, while nearby current text references the completed parent disposition; 16-44-AMENDMENT.md:178 still requests independent checker/parent review."
    fix_hint: "Replace stale pending-review language with a reference to 16-44-SCOPE-DISPOSITION.md while preserving the visible advisories."
```

### Recommendation

The scope disposition itself is valid, and its advisories are nonblocking. Revise the remaining pending-review wording for consistency before treating this targeted disposition check as passed.
