## ISSUES FOUND

**Phase:** 16-host-evidence-and-roadmap
**Plans rechecked:** 16-44 and 16-45
**Status:** The prior transport blockers are resolved. Two advisory 16-45 scope warnings remain open for parent disposition.
**Issues:** 0 blocker(s), 2 warning(s), 0 info

### Transport recheck

The revised reception contract is measurable at the boundary available to the caller. It caps the nested designed stdout/stderr at 16,384 UTF-8 bytes and the single serialized `text()` payload at 8,192 UTF-8 bytes. It explicitly excludes the provider-created `functions.exec` wrapper and hidden fields from the byte claim, and records unexposed wrapper metadata as unavailable. The caller retains the original nested request/result, while the receiving model checks the actual returned text and passes that exact text to a separate read-only decoder call. That decoder checks the received text against the retained nested result and returns a validation payload capped at 1,024 UTF-8 bytes; the model checks that result before first-write authority. Visible truncation, changed or incomplete text, mismatched identity, or failed decoding stops before the first write. No self-inspection of a future wrapper or receipt-of-receipt loop is claimed.

The decoder section lists 14 explicit synthetic negative subjects with exact reason codes, including duplicate keys, invalid encodings, length/hash mismatches, direct and indirect cycles, missing/mismatched references, both trailing-data cases, unreferenced payloads, decode limit, and invalid cardinality. Each requires zero proposed writes and zero durable, clock, boot, or root actions. The plan distinguishes these specific blob invariants from generic malformed outer JSON. I found no remaining unbounded wrapper claim, self-referential receipt, or reception substitution loophole in the revised contract.

### Coverage and preservation

All six phase requirements, D-01 through D-17, the 39 source groups, four explicit edge duties, four flagged assumptions, and the prior history/terminal limits remain in the plans. Plan 44 remains preparation-only and stops; 45 retains fresh acceptance; downstream 07/08 release gates remain separate. No historical 42/43 result or unavailable 42 output receives new credit.

Plan-structure validation passes: 44 has 2 complete tasks and 45 has 3 complete tasks. The six baseline dirty files match their recorded hashes and remain unstaged. The refreshed deterministic probes both exited 0 and inspected 358 commands: path resolvability is `not_applicable` with no read error or findings, while failing-direction status is `ok` with no read error or findings. `not_applicable` is not path or runtime proof. No prospective transport control, build, or runtime check was run.

### Advisory warnings — parent disposition required

**1. [scope_sanity] The 45 plan retains an open path-count warning.**

- Plan: 16-45
- Evidence: The plan declares 13 paths across 3 tasks, with task footprints 5/5/3; the project threshold marks more than 10 paths as a warning. The amendment explicitly leaves this warning open for parent review and does not rely on 43's earlier approval.
- Required property: The current path-count warning must receive an explicit parent disposition before execution.
- Example fix (non-binding): Record the parent’s disposition while retaining the warning and the plan’s stated indivisible-transaction rationale.

**2. [scope_sanity] The 45 estimate exceeds the smart-zone budget and remains low-confidence.**

- Plan: 16-45
- Evidence: The calibrated estimate is 102,900 tokens against a 100,000-token budget (+2,900; ratio 1.029). Confidence is low, sample count is 0, and calibration factor is 1. The estimate check recommends about two slices. Plan 44 is 93,500 tokens; the combined estimate is 196,400. The amendment explicitly says the smart-zone warning remains open and that predecessor approval does not cover 45's added work.
- Required property: The current over-budget warning must receive an explicit parent disposition, with the estimate’s low confidence kept visible.
- Example fix (non-binding): Record the parent’s disposition of the warning, including whether to accept the one-owner scope or revise the slice boundary.

### Structured issues

```yaml
issues:
  - plan: "16-45"
    dimension: "scope_sanity"
    severity: "warning"
    required_property: "The current path-count warning must receive an explicit parent disposition before execution."
    description: "The plan declares 13 paths across 3 tasks with 5/5/3 per-task footprints, exceeding the 10-path warning threshold; the amendment leaves this warning open and does not carry predecessor approval forward."
    fix_hint: "Record the parent’s disposition while retaining the warning and the plan’s stated indivisible-transaction rationale."
  - plan: "16-45"
    dimension: "scope_sanity"
    severity: "warning"
    required_property: "The current over-budget warning must receive an explicit parent disposition, with the estimate’s low confidence kept visible."
    description: "The calibrated estimate is 102900 against a 100000-token budget (+2900); confidence is low, sample_count is 0, factor is 1, and the estimate check recommends about two slices. The amendment leaves this warning open and says 43's approval does not cover the added work."
    fix_hint: "Record the parent’s disposition of the warning, including whether to accept the one-owner scope or revise the slice boundary."
```

### Recommendation

No transport blocker remains. The two scope warnings are advisory, not blockers; parent disposition is still required because both are explicitly left open. No further transport-plan revision is required by this recheck.
