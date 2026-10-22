## ISSUES FOUND

**Phase:** 16 — Host evidence and roadmap
**Plans checked:** 16-50 through 16-54
**Issue count:** 3 (0 blockers, 2 warnings, 1 info)

### Coverage and verification summary

| Requirement | Plans | Status |
|---|---|---|
| EVID-01 | 16-50–16-54 | Covered |
| EVID-02 | 16-50–16-54 | Covered |
| EVID-03 | 16-50–16-54 | Covered |
| EVID-04 | 16-50–16-54 | Covered; published metadata gap remains explicitly for later release evidence |
| DIR-01 | 16-50–16-54 | Covered |
| DIR-02 | 16-50–16-54 | Covered |

The five plans structurally validate, contain 11 complete tasks, and form the acyclic chain 16-50 → 16-51 → 16-52 → 16-53 → 16-54. Every task has `read_first` and acceptance criteria; all 44 plan-level automated commands have an immediate nonempty `fails_when`. The supplied path probe is `not_applicable` (430 commands, no probe errors), so it establishes no runtime-path proof; the supplied failure-direction probe is `ok` for all 430 commands. I did not execute any command string from either probe.

The split preserves 16-50/51 as separately invoked preparation and 16-52/53/54 as one separately invoked runtime group under the same canonical child/root. Its typed continuation checkpoints, same-child native follow-up, original development and review clocks, one-use slots, and parent-after-terminal gate are explicitly carried. The current scheduling-only frontmatter for 16-07/08 points to the new chain; their historical bodies are not treated as current authority. Plans retain all 17 locked decisions, all 39 source groups, and all eight edge dispositions (four explicit and four still flagged). The 16-34/35 contracts are carried as historical duty data. The latest 16-48 failure remains an actual nonempty B response (23, 77 bytes); missing provenance remains UNKNOWN. No ownership, acceptance, clock, or slot transfer is planned.

The 16-50/51 plans preserve the strict 16-34 CLI repair and final matrices/export, complete READ/COMPARE/full-graph before first write, fixed bounds, independently retained/reconstructed actual receipts, acyclic nonrecursive manifests, and frozen admission closure. Source/test/README/header duties remain paired; native, consent, publication, and released-archive checks remain separate. There is no circular dependency or relevant architectural-tier conflict. Nyquist is disabled in project configuration; Nyquist compliance is therefore skipped.

The estimates total 284,000 projected tokens (80k/65k/65k/43k/31k), confidence low with zero completed-phase samples. The plans explicitly call these unmeasured projections, count repeated gates, and do not claim aggregate context fit; treat this as an execution-planning uncertainty, not evidence of a fit.

### Warnings

**1. [task_completeness] Plan 16-54 Task 2’s verify does not assert the required terminal cursor and completion-start.**

- **Plan/task:** `16-54-PLAN.md`, Task 2 (lines 138–145)
- **Required property:** The task’s passing verification must establish that actual checked completion-start produced the final same-owner 16-54 terminal-success cursor within the original review deadline.
- **Evidence:** The task’s acceptance criteria require the 52/53/54 summaries and final cursor to bind the same candidate and owner within 1,800 seconds. Its sole automated verify reads the review disposition, source acceptance, review file, ten committed blobs, and protected config, then prints a success label. It does not read `runtime-continuity.jsonl`, a stage summary, or completion-start evidence. The `fails_when` text mentions missing terminal evidence, but the command does not test it. The action and 16-52 amendment do require the checked final checkpoint as part of completion-start, so the execution protocol is present; the task’s final automated acceptance is not bound to that result.
- **Example fix (non-binding):** Make the checked completion-start result’s terminal cursor/receipt part of the task’s passing verification, preserving the original deadline and avoiding a post-deadline terminal operation.

**2. [verification_derivation] Current Plan 16-54 review output carries a predecessor label.**

- **Plan/task:** `16-54-PLAN.md`, Task 2, line 141
- **Required property:** Current evidence output must identify the current review group without implying provenance from a superseded predecessor.
- **Evidence:** The current `16-52-REVIEW-*` files and 52/53/54 runtime group are bound by the script, but its final printed message is `validation49 exact committed review binding: passed`. That is a stale label from the superseded 49 plan lineage and can misidentify current output.
- **Example fix (non-binding):** Use a label that names the current 52 review and group binding.

### Advisories (info)

**1. [scope_sanity] Aggregate context fit is not established.**

- **Plan:** Phase-level
- **Required property:** Execution sizing should be evidence-based before treating a multi-plan runtime group as fitting one context.
- **Evidence:** All per-plan estimates are low-confidence projections with `sample_count=0`; the continuous 52–54 group totals about 139,000 projected tokens and the five-plan aggregate is 284,000. The plans disclose the uncertainty and retain separate plan contours, so this is advisory rather than a scope blocker.
- **Example fix (non-binding):** Measure actual context use during execution and revise future estimates from those measurements.

### Structured issues

```yaml
issues:
  - plan: "16-54"
    task: 2
    dimension: "task_completeness"
    severity: "warning"
    required_property: "The passing task verification establishes the actual same-owner terminal-success cursor and checked completion-start within the original review deadline."
    description: "Task 2 acceptance requires the final cursor and completion-start, but its sole automated verify only binds the review/source report, committed blobs, and protected config. It never reads the runtime continuity cursor, stage summaries, or completion-start evidence; therefore the check can pass without proving its stated end condition. The action and amendment do require a checked final checkpoint, so this is a verification gap rather than evidence that the execution protocol omits the checkpoint."
    fix_hint: "Bind the checked completion-start result and terminal cursor into the task's passing verification without introducing a post-deadline terminal operation."
  - plan: "16-54"
    task: 2
    dimension: "verification_derivation"
    severity: "warning"
    required_property: "Current evidence output labels identify the current review group and do not imply superseded predecessor provenance."
    description: "The final print in the current 52 review-binding command says 'validation49 exact committed review binding: passed' despite binding current 16-52 files and the 52/53/54 group."
    fix_hint: "Rename the printed label to describe the current 52 review/group binding."
  - plan: null
    dimension: "scope_sanity"
    severity: "info"
    required_property: "Aggregate context-fit claims are supported by measured estimates."
    description: "Projected estimates total 284,000 tokens and the continuous 52–54 group about 139,000, with low confidence and sample_count=0. The plans explicitly disclose that these estimates are unmeasured and do not claim aggregate fit."
    fix_hint: "Use actual execution context measurements to calibrate later estimates."
```

### Recommendation

0 blockers and 2 warnings require attention before treating the plan set as fully verified; 1 info advisory remains. This review verifies plans only. It does not establish that any runtime, CI, independent review, parent gate, publication, or released-byte acceptance has actually executed or passed.
