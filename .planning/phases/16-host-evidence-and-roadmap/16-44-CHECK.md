## ISSUES FOUND

**Phase:** 16-host-evidence-and-roadmap
**Plans checked:** 16-44, 16-45; downstream gates 16-07 and 16-08
**Issues:** 2 blocker(s), 1 warning, 0 info

### Coverage summary

| Requirement | Coverage | Status |
|---|---|---|
| EVID-01 | 44/45 preparation and acceptance; 07/08 release gates | Covered |
| EVID-02 | 44/45 host evidence; 07/08 release gates | Covered |
| EVID-03 | 44/45 host evidence; 07/08 release gates | Covered |
| EVID-04 | 44/45 released-artifact evidence; 07/08 publication/archive gates | Covered |
| DIR-01 | 44/45 direction evidence; 07/08 release gates | Covered |
| DIR-02 | 44/45 direction evidence; 07/08 release gates | Covered |

The successors retain D-01 through D-17, all 39 source groups, the four explicit and four flagged edge duties, and the unresolved assumption flags. Plan 44 remains preparation-only and stops; 45 owns fresh acceptance; 07 and 08 still require their separate native, owner-consent, publication, archive, checksum, attestation, and exact-release gates. The historical 42/43 outcomes and unavailable 42 envelope remain uncredited. No coverage reduction or downstream handoff omission was found.

Static structure validation passed for all four plans: 44 has 2 complete tasks, 45 has 3, 07 has 3, and 08 has 2. The supplied path probe reports `not_applicable`, 358 commands, no read error, and no findings; this is not path or runtime proof. The supplied failing-direction probe reports `ok`, 358 commands, no read error, and no findings. Six baseline dirty files remain byte-identical and unstaged. No prospective control, build, or runtime check was run.

### Blockers — these properties must hold

**1. [transport_feasibility] Each outer `functions.exec` envelope is bounded and independently checked before first-write authority.**

- Plan: 16-44 and 16-45
- Evidence: The amendment requires an exact 16,384-byte outer emission bound, exact byte measurement including provider wrapper fields “before” forwarding, and a measured wrapper allowance, but it supplies no numeric allowance or receiver/API that exposes the complete outer envelope to the producer before `text()` returns it. The `functions.exec` script can store and emit nested tool results, but it cannot observe the provider-created envelope that will wrap the script result. The plan therefore cannot perform its stated exact prewrite measurement for that boundary; the 18,000-token tool limit does not establish the separate byte bound.
- Required property: The complete outer response, including a concrete bounded wrapper allowance, must be observable and independently validated before any first-write authority is granted.
- Example fix (non-binding): Provide an outer receiving step/API that exposes the complete returned envelope and fixed wrapper bound to the prewrite validator, or revise the transport mechanism so its claimed boundary is one the executor can actually measure while retaining the full actual result.

**2. [verification_derivation] Every promised blob-table rejection rule has an explicit synthetic negative case and expected failure reason.**

- Plan: 16-44 and 16-45
- Evidence: The transport contract promises rejection of duplicate keys, inconsistent lengths/hashes, invalid encoding, cycles, dangling references, trailing data, and unreferenced payloads (16-44-AMENDMENT.md:15). Its named synthetic mutant set covers malformed/truncated output, terminal/end-marker failures, wrong length/hash/reference, stale key, switched mode/authority, and extra stream (16-44-AMENDMENT.md:39), but does not explicitly exercise duplicate keys, invalid encoding, cyclic references, or unreferenced payloads, each with a declared reason and zero-write assertion. A general malformed-output case does not establish those distinct decoder invariants.
- Required property: Every rejection invariant that protects complete, lossless blob closure must be independently exercised with its expected reason and zero proposed writes.
- Example fix (non-binding): Add named mutants and expected reason codes for each uncovered invariant to the finite prewrite case table.

### Warning — should be resolved or explicitly carried

**1. [scope_sanity] The current 45 plan must carry an explicit disposition for its over-threshold scope and new transport work.**

- Plan: 16-45
- Evidence: 45 declares 13 paths across 3 tasks with per-task footprints of 5/5/3, and a 99,900-token estimate with `confidence: low` and `sample_count: 0`. Its amendment adds outer-envelope transport duties while retaining that estimate. The scope disposition is inconsistent: 16-45-PLAN.md says the earlier 43 acceptance applies to the “identically scoped” successor, while 16-44-AMENDMENT.md:149 says to carry a new visible warning with no inherited approval. The footprint is mechanically unchanged, but the new transport acceptance obligations are substantive additional work; I do not count the earlier approval as a waiver.
- Required property: The current scope warning and its disposition must be stated consistently, with the added transport work represented in the scope estimate or called out as uncalibrated.
- Example fix (non-binding): Align the amendment and plan on the current warning status and make explicit how the added transport work is represented in the low-confidence estimate.

### Structured issues

```yaml
issues:
  - plan: "16-44, 16-45"
    dimension: "transport_feasibility"
    severity: "blocker"
    required_property: "The complete outer response, including a concrete bounded wrapper allowance, must be observable and independently validated before any first-write authority is granted."
    description: "The plans require exact pre-forwarding measurement of a provider-created functions.exec envelope, but the producer cannot observe that envelope before returning and no numeric wrapper allowance or independent receiver is specified."
    fix_hint: "Provide an outer receiving step/API that exposes the complete returned envelope and fixed wrapper bound to the prewrite validator, or use a transport mechanism whose claimed boundary the executor can measure while retaining the full actual result."
  - plan: "16-44, 16-45"
    dimension: "verification_derivation"
    severity: "blocker"
    required_property: "Every rejection invariant that protects complete, lossless blob closure must be independently exercised with its expected reason and zero proposed writes."
    description: "The blob contract promises rejection of duplicate keys, invalid encoding, cycles, and unreferenced payloads, but the listed synthetic mutants do not explicitly exercise those cases with declared reasons."
    fix_hint: "Add named mutants and expected reason codes for each uncovered invariant to the finite prewrite case table."
  - plan: "16-45"
    dimension: "scope_sanity"
    severity: "warning"
    required_property: "The current scope warning and its disposition must be stated consistently, with the added transport work represented in the scope estimate or called out as uncalibrated."
    description: "The 13-path, 5/5/3 task footprint exceeds the warning threshold and adds transport acceptance work while retaining a low-confidence 99,900-token estimate; the plan claims earlier 43 acceptance applies while the amendment says to carry a new warning with no inherited approval."
    fix_hint: "Align the plan and amendment on the current warning status and make explicit how the added transport work is represented in the estimate."
```

### Recommendation

2 blockers and 1 warning require revision before execution. The deterministic failure-direction probe is clear, but the path-resolvability probe is `not_applicable` and supplies no path evidence. The transport gate cannot be accepted as written until the outer response boundary is executable and all promised decoder rejection invariants have planned negative controls.
