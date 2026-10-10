---
status: diagnosed
trigger: "Phase 02 UAT test 2: automate machine-checkable prohibitions and post-merge evidence; persist an automation-first verification default"
created: 2026-10-06
updated: 2026-10-06
audit_acknowledged:
  milestone: v1
  at: 2026-10-09
  status: diagnosed
---

## Current Focus

hypothesis: Phase 2 treated several negative constraints as one-time inspection rather than executable gates, and its verification workflow routes missing gates and future GitHub events to owner acknowledgement.
test: Compare each flagged prohibition with registered CTest and CI steps, then trace release and scheduled-nightly results into GSD verification artifacts.
expecting: Some constraints have observed compliance or partial coverage but no direct enforcing oracle; GitHub workflows report operational outcomes without updating UAT state.
next_action: Plan focused policy gates, post-merge evidence collection, and a durable GSD verification rule; keep clean-room provenance as a judgment-tier item.
bug_class: Bohrbug (deterministic process/configuration gap)
candidate_causes:
  - code: missing direct negative-rule assertions in CTest/hygiene
  - config: verifier/UAT disposition requires owner acknowledgement when test-tier rule lacks a wired check
  - environment: release and schedule occur only after merge on GitHub

and_gate: yes; missing tests cause the acknowledgement gate, while later external events cause separate manual follow-up unless captured by automation.

## Symptoms

expected: Machine-checkable Phase 2 prohibitions and post-merge outcomes are enforced or observed automatically, and GSD keeps an automation-first verification default.
actual: 02-VERIFICATION.md flags six test-tier prohibitions without enforcing tests; UAT requests owner acknowledgement; release and scheduled nightly await manual follow-up.
errors: None reported.
reproduction: Read Phase 02 UAT test 2 and the Prohibitions and Human Verification Required sections of 02-VERIFICATION.md.
started: Discovered during Phase 02 UAT on 2026-10-06.

## Eliminated

- hypothesis: There is no project rule requiring command-driven verification.
  evidence: PROJECT.md Verification and Hand-offs constraints explicitly require commands and no manual testing; AGENTS.md rule 6 says checks are automated and none waits on a person.
  timestamp: 2026-10-06
- hypothesis: Phase 2 has no automated vector, hygiene, release, or nightly coverage.
  evidence: tests/CMakeLists.txt registers 256 sample opcode tests, 256 full opcode tests under the full preset, and a fetch guard; ci.yml runs the presets, nightly.yml runs vectors-full and opens/closes a rolling issue, and release.yml checks archives and attestations before publishing.
  timestamp: 2026-10-06
- hypothesis: All seven prohibitions can be proven by a source-tree test.
  evidence: The clean-room prohibition concerns whether someone opened external GPL/LGPL source, an activity not observable from repository content; 02-VERIFICATION.md itself classifies it as judgment-tier.
  timestamp: 2026-10-06

## Evidence

- timestamp: 2026-10-06
  checked: 02-UAT.md test 2
  found: It requests owner acceptance of directly observed compliance or enforcing tests; the user selected automation.
  implication: The current verification disposition itself is a human gate for machine-checkable rules.
- timestamp: 2026-10-06
  checked: 02-VERIFICATION.md Prohibitions table
  found: Six test-tier items are flagged without wired enforcing tests; one clean-room rule is judgment-tier.
  implication: Observed compliance does not persist as an executable regression guard.
- timestamp: 2026-10-06
  checked: tests/CMakeLists.txt and tests/hygiene/CMakeLists.txt
  found: Existing CTest registrations enforce output and vector behavior, release config, script policies, action pins, and a fetch guard; no named check asserts header API freeze, large tracked JSON exclusion, allowed vector-file parsing methods, or the workflow prohibitions as complete policies.
  implication: The tests give strong positive coverage but do not directly gate several stated negative constraints.
- timestamp: 2026-10-06
  checked: scripts/hygiene.sh
  found: It checks personal data, game-image names and magic, unlisted binary files, commit identity, and GSD chain settings; a large plain-text JSON file is not prohibited by its checks.
  implication: The full 1.08 GB JSON prohibition has a concrete enforcement gap despite existing hygiene automation.
- timestamp: 2026-10-06
  checked: tests/CMakeLists.txt opcode registrations and nightly.yml
  found: Both sample and full opcode tests iterate NESTURBATOR_ALL_OPCODES across 00..ff; nightly has no cache action, runs vectors-full, and reports scheduled failure through an issue.
  implication: Some desired behavior is already exercised, but no independent assertion protects the registration count or workflow policy against a later configuration edit.
- timestamp: 2026-10-06
  checked: release.yml and nightly.yml
  found: Release publication depends on CI and checks 18 archives, checksums, and attestations; scheduled nightly reports via issue. Neither workflow writes the resulting tag/assets or scheduled run outcome into Phase 02 UAT/verification state.
  implication: The operational workflows are automated; the GSD backstop remains a manual observation and reconciliation task.
- timestamp: 2026-10-06
  checked: PROJECT.md, AGENTS.md, and .planning/config.json
  found: PROJECT.md and AGENTS.md already express no manual testing, while config.json keeps human_verify_mode end-of-phase and UAT test 2 still requests acknowledgement.
  implication: A general principle exists but is not applied consistently when verification sees test-tier policy gaps; the durable record should specify the decision rule at that boundary.

## Resolution

root_cause: "The Phase 2 plan and implementation lacked a direct machine-checkable policy-to-gate mapping for six negative constraints, so the verifier correctly flagged observed-only compliance and UAT converted it into an owner acknowledgement. Separately, release and scheduled-nightly outcomes happen after merge and are automated operationally, but no automatic post-event reconciliation records their evidence in Phase 2 verification. Existing project policy says verification is command-driven, yet the UAT handoff does not operationalize that rule for these cases."
fix: "Add focused, dependency-free assertions for the machine-verifiable Phase 2 prohibitions; capture release and main-push/nightly workflow evidence automatically; codify the automation-first verification rule in GSD-HANDOFF.md without duplicating PROJECT.md. Keep clean-room provenance as a judgment-tier claim."
verification:
  files_changed: []
