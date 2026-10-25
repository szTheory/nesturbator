# Phase 16 PID continuation — independent plan check

**Mode:** gap closure
**Phase goal:** Host builders can trust current RetroArch state/progression evidence and the released MMC3 description, while milestone starts and closeouts preserve evidence-backed product direction.
**Plans checked:** 12 live plans; 29 tasks
**Current status:** 9/12 plans complete. Plan 22 is ready with 0/3 tasks accepted; plans 07 and 08 remain ordered behind it.
**Verdict:** No plan defects found. This is a planning review, not runtime or release acceptance.

## Goal and requirement coverage

| Requirement | Covering live plans | Coverage |
|---|---|---|
| EVID-01 | 01, 02, 03, 15, 21, 22 | Earlier progression work is retained; 22 repairs current driver evidence prerequisites and 07 retains the fresh native gate. |
| EVID-02 | 03, 09, 15, 21, 22 | Retry, one-shot, timing, cleanup, and ownership contracts are retained and extended by 22. |
| EVID-03 | 04 | Exact fixture identity and supplied-byte behavior remain in the completed plan. |
| EVID-04 | 05, 07, 08, 09, 15, 21, 22 | Plan 05 defines truthful support; 07 requires final-head native checks, exact-head consent, and normal publication; 08 verifies the actual published bytes. |
| DIR-01, DIR-02 | 06 | Installed roadmap and same-step lifecycle evidence remain in the completed plan. |

The live chain delivers the phase goal in order: completed direction and host work remains credited; 22 repairs the current local safety and acceptance prerequisites; 07 holds the final PR/native/consent/publication sequence; 08 verifies the published release assets and metadata. Plan 22 makes no publication claim, and 07 cannot proceed until its current 22 artifacts and mandatory parent gates exist. The index reports 07 not ready on 22 and 08 not ready on 07.

| Plan | Tasks | Wave | Current index status |
|---|---:|---:|---|
| 16-01 | 2 | 1 | Complete |
| 16-02 | 2 | 2 | Complete |
| 16-03 | 3 | 3 | Complete |
| 16-04 | 2 | 4 | Complete |
| 16-05 | 2 | 5 | Complete |
| 16-06 | 2 | 1 | Complete |
| 16-09 | 3 | 6 | Complete |
| 16-15 | 2 | 7 | Complete |
| 16-21 | 3 | 8 | Complete |
| 16-22 | 3 | 9 | Ready; no current task acceptance |
| 16-07 | 3 | 10 | Blocked by 16-22 |
| 16-08 | 2 | 11 | Blocked by 16-07 |

The installed `phase-plan-index 16` is the live-plan source; superseded 10–14 and 16–20 are excluded. No plan 23 is present. Completed 21 is retained as prior evidence and is not replayed.

## Plan 22 and handoff review

Plan 22's three tasks address the current checkpoint directly. Task 1 audits the five unaccepted source/doc edits before freezing, corrects the writer's driver/helper PID distinction, adds actual-writer controls, finalizes all post-launch failure paths, and requires separate old-source RED and corrected-source GREEN classifier commands under one frozen oracle. Task 2 carries the selected descriptor reservation through lifecycle, legacy alias/sidecar, readiness, and cleanup controls within existing limits. Task 3 gates a new committed candidate on the fresh once-only hygiene, registered CI/ASan, full 649-test workflow, three local archives, independent source review, and separately required parent gates. The plan keeps the old three roots exhausted, uses the new three-round/3600-second allowance, and does not award runtime or requirement credit in advance.

The only current review finding in `16-22-REVIEW.md` is CR-01: cleanup could unlink a foreign replacement after a pathname identity check. The selected descriptor-owned reservation removes shared-path deletion from release; the active tasks also require the preserved old driver's in-window damage and corrected driver's same-oracle preservation proof. The earlier one-command tracer issue recorded in `16-22-LOCK-CHECK.md` is resolved in the current plan by two ordered RED/GREEN commands, each with a successful classifier result and retained raw status. The new PID amendment extends that contract with driver-versus-helper identity and failure-path wait/stream collection.

Plan 07 has a valid 22 dependency and makes consent specific to the live PR URL and exact checked head. It performs fresh native required/perf/pinned-host checks before presenting consent, and puts the merge/publication mutation after the blocking-human decision. Plan 08 depends on 07 and checks release identity, checksums, source-bound archives, attestations, six metadata copies, and exact-release CI. Local packages cannot satisfy the publication requirement.

## Structure, dependencies, and execution scope

`verify.plan-structure` returned `valid: true` with no errors or warnings for all 12 indexed plans. All 29 tasks have their required files, action, verify, and done elements; the checkpoint task in 07 is structurally valid. Dependencies are acyclic and wave order is consistent: 01 → 02 → 03 → 04 → 05 → 09 → 15 → 21 → 22 → 07 → 08, with 06 independently in wave 1 and also a dependency of 09. The only same-wave pair is 01/06; their declared file sets and actions do not share a mutable resource.

Key links are covered: 22 connects the existing registered session controls to the driver and binds its new acceptance disposition to 07; 07 connects the approved merge to the existing release workflow; 08 uses the existing archive/support checks against the immutable release source. The shared old/new race proof names one frozen semantic oracle and unchanged foreign-object assertions, so the cross-plan evidence contract is compatible.

Task/file counts stay within the documented thresholds: no plan exceeds 3 tasks or 8 modified files. Calibrated estimate checks used `--calibrated`; the tool budget is 100,000 tokens for each estimate. Estimates are low-confidence because the calibration sample count is zero, so they are projections rather than precise measurements.

| Plan | Estimate | Budget | Result |
|---|---:|---:|---|
| 16-01 | 38,000 | 100,000 | Within; low confidence |
| 16-02 | 30,000 | 100,000 | Within; low confidence |
| 16-03 | 42,000 | 100,000 | Within; low confidence |
| 16-04 | 24,000 | 100,000 | Within; low confidence |
| 16-05 | 22,000 | 100,000 | Within; low confidence |
| 16-06 | 12,000 | 100,000 | Within; low confidence |
| 16-07 | 24,000 | 100,000 | Within; low confidence |
| 16-08 | 18,000 | 100,000 | Within; low confidence |
| 16-09 | 44,000 | 100,000 | Within; low confidence |
| 16-15 | 34,000 | 100,000 | Within; low confidence |
| 16-21 | 42,000 | 100,000 | Within; low confidence |
| 16-22 | 76,000 | 100,000 | Within; low confidence |

## Other dimensions and limits

- **Context compliance:** The 17 locked decisions are retained across completed plans and the 22 → 07 → 08 chain. Deferred hardware, APIs, platforms, and future lifecycle work are not added. Plan 22 preserves the explicit TCP/UDP, same-UID, and SIGKILL limits.
- **Architectural tier:** The research responsibility map assigns commands/deadlines/artifact publication to the test shell driver, permitting a narrow helper only after a failed shell control. Plan 22 applies that boundary using existing Perl/platform facilities after the recorded shell race; it adds no dependency, public API, registered test, or host platform.
- **Nyquist:** Checks 8a–8e are skipped because project configuration disables Nyquist validation and the research has no Validation Architecture section. Check 8f consumed the supplied probe without re-deriving its verdict.
- **Research resolution:** `16-RESEARCH.md` has `## Open Questions (RESOLVED)`; no responsibility-tier mismatch was found.
- **Patterns and project skills:** No phase PATTERNS.md and no `.codex/skills/` or `.agents/skills/` directories exist. Project `AGENTS.md` was read; the plan preserves owned code, no new dependency, paired behavior documentation, automated CI requirements, and one-step lifecycle boundaries.
- **Cross-plan transformations:** No incompatible shared-data transformation or undeclared same-wave writer/reader coupling was found.
- **Verify probes:** The supplied failing-direction probe reports `status: ok`, 138 commands, 0 blockers, 0 warnings, and `readError: null`. The supplied path probe reports `status: not_applicable`, 138 commands, 0 blockers, 0 warnings, and `readError: null`. The latter is not path resolution or runtime proof; no such claim is made here.
- **Execution boundary:** No tests, driver runs, host runs, runtime acceptance, native CI, publication, consent request, or lifecycle command were performed for this plan check.

## Structured issues

```yaml
issues: []
```

## Initial review verdict (before focused recheck)

**Phase:** 16 — Host evidence and roadmap
**Plans verified:** 12 live plans
**Status:** The initial full-plan review found no blocker, warning, or advisory. Its budget-class conclusion is superseded by the focused recheck below, which reviews the planner’s clarification before the final verdict. Runtime and publication claims remain unaccepted until their planned evidence is produced.

## Focused recheck — explicit development/formal budget classes

Rechecked the revised budget contract at `16-22-PLAN.md:89–91` and `16-22-PID-AMENDMENT.md:77–83,87–93`, against all active Plan 22 verification commands and the retained lock checkpoint/review context. The clarification closes the previously identified ambiguity: task 1/2 launches explicitly use development mode and only development launches read/clamp against the checked round/aggregate remainder; task 3 switches to formal mode only after accepted, closed development and uses its own once-only formal ledger slots and declared fixed deadlines. Formal rows explicitly have `development_remaining_ms=null` and `development_accounting=false`. Root/mode mismatch and missing mode reject launch. The three-round/3600-second allowance and all command bounds remain unchanged.

Every automated command in the three active tasks is wrapped by `p16_bounded` (3 task-1, 3 task-2, and 9 task-3 commands). The execution context directs the executor to export the corresponding mode for the active task, and the amendment requires that the function and command share an orchestration shell. The formal CI/ASan configure-build-test sequences, full workflow, archive check, and hygiene command all target the formal root; parent build/post-wave/execute:post/review/full-workflow gates are expressly outside both allowances under parent-owned accounting. The checkpoint keeps those gates pending, and the new plan still grants no runtime acceptance or 22 summary in advance. I found no remaining budget-class or gate-accounting conflict.

The supplied deterministic probes were read as returned: path probe `not_applicable`, 138 commands, 0 findings, `readError: null`; failing-direction probe `ok`, 138 commands, 0 findings, `readError: null`. Path status is not path/runtime validation and is not represented as such. Plan 22 remains 3 tasks, all unaccepted; no tests or runtime/gate commands were run in this planning review.

## VERIFICATION PASSED

The initial 12-plan review remains valid after the focused budget-class recheck. The phase plans are structurally and goal-backward complete; this result establishes planning validity only, not runtime, formal acceptance, parent-gate completion, native CI, consent, or publication.
