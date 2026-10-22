## ISSUES FOUND

**Phase:** 16 — Host evidence and roadmap

**Amendment checked:** Plan56 context amendment, current Plan56/51/52 references

**Plans checked:** 3 amended/current plans (56, 51, 52)
**Issues:** 1 blocker, 0 warnings, 0 info

### Coverage and contract review

| Requirement | Current plan coverage | Status |
|---|---|---|
| EVID-01 | Plan56 declares it; current 51/52 sequence retains it | Covered |
| EVID-02 | Plan56 declares it; current 51/52 sequence retains it | Covered |
| EVID-03 | Plan56 declares it; current 51/52 sequence retains it | Covered |
| EVID-04 | Plan56 declares it; current 51/52 sequence retains it | Covered |
| DIR-01 | Plan56 declares it; current 51/52 sequence retains it | Covered |
| DIR-02 | Plan56 declares it; current 51/52 sequence retains it | Covered |

The approved exception is narrowly scoped to the already planned Phase56
preflight directory, the same two helper files, and one untrusted construction
journal. The journal is explicitly distinguished from transport evidence,
bootstrap, seal, acceptance receipt, or helper-set. No new helper module or
root is added. Construction remains subject to the existing outer/nested
18,000, decoded/request/producer/caller/segment and at-most-32 sequential
packet limits, with separate awaited READ/ACK and complete actual originals.
The amendment preserves actual current source reception, complete target and
compact serialization reconstruction, target readback, initial-header and
every largest-shape empty/nonempty A/B, fresh snapshot READ plus independent
COMPARE, the entire finite graph, and independent parent/child proof before
their own nonconstruction writes or accepted import. Construction records and
candidate code remain untrusted inputs; all negative controls measure zero
*additional* mutation against the retained construction baseline, with
`authority=false`, `runtime_admission=false`, empty proposed writes, and the
intended refusal reason. Finalized actual failure remains terminal, with no
edit/retry/re-freeze. The amendment makes no runtime acceptance claim.

The current 51/52 mappings require genuine Plan56 success and actual
administrative parent reconciliation before code-only import. Diff review
shows their own before-first-write protocols remain intact in substance;
Plan52 also retains its twelve ordered rows and same-child clocks, counters,
slots, and independent parent gates. Downstream parent/native/exact-head
affirmative consent, normal publication, and the released 18-archive,
checksum, attestation, six-metadata, and exact-release-CI gates remain stated.
No successor plan is required by the checkpoint.

The current Plan56 frontmatter covers all six Phase16 requirements. Its
amendment retains D-01–D-17, 39 source groups, eight fallback edges and four
flagged unresolved assumptions. Deferred ideas remain excluded. The checked
plan structure is valid for all three changed plans (two complete tasks each);
Plan56 has eight declared modified paths, within the 5–8 target, and each task
names five files. Dependencies remain acyclic and ordered: 30 → 56 → 51 → 52 →
53 → 54 → 07 → 08. No unapproved product or runtime scope is introduced.

### Blockers — these properties must hold

**1. [task_completeness] All helper and source edits must finish under the
construction allowance before final-source freeze and finalized transport; the
accepted transport, bootstrap, and export must bind the same final bytes.**
- Plan: 16-56
- Task: 2
- Evidence: Task 1 says to “freeze complete final code/journal/source closure,”
  run the full actual transport and snapshot/graph gates, and only then write
  and seal the bootstrap. It also says to implement Task 2’s code/API/selector
  duties during construction. Task 2 nevertheless directs the executor to
  “Repair” the copied34 prerequisite and “Implement” the rehearsal, continuity
  APIs, and selectors in the same helpers. The context amendment permits source
  edits only before prospective transport finalization and says every allowed
  edit invalidates and requires rerunning the full gates; it forbids edits after
  the final freeze. The plan therefore leaves Task 2’s required code mutations
  after Task 1’s finalization, while the task boundary offers no legal way to
  alter the source and bind the bootstrap/export to the changed bytes.
- Example fix (non-binding): make the task order explicit so all source/API/
  selector/CLI repairs occur during the allowlisted construction stage before
  freeze, and the later task only runs the authorized final-source matrices,
  writes accepted evidence, and exports those exact frozen bytes.

### Warnings

None.

### Advisories

None.

### Structured Issues

```yaml
issues:
  - plan: "16-56"
    dimension: "task_completeness"
    severity: "blocker"
    required_property: "All helper and source edits finish under construction authority before final-source freeze and finalized transport; accepted transport, bootstrap, and export bind the same final bytes."
    description: "Task 1 freezes the complete code/source closure, runs the full actual transport and snapshot/graph gates, then writes and seals the bootstrap. Task 1 also directs implementation of Task 2's code/API/selector duties during construction, but Task 2 still directs repairs and implementation in the same helpers after Task 1. The context amendment permits edits only before transport finalization, requires a full gate rerun after each permitted edit, and forbids edits after freeze. The task sequence does not establish a legal point to make Task 2's required mutations and then bind final transport/bootstrap/export to those bytes."
    task: 2
    fix_hint: "Make all source construction and repair precede freeze/finalized transport, and keep post-gate work limited to acceptance matrices, evidence, and export of the same frozen bytes."
```

### Static checks and limits

`verify.plan-structure` returned valid for 16-56, 16-51, and 16-52. The supplied
path probe was consumed unchanged: 451 commands, `status=not_applicable`, zero
blockers/warnings, `readError=null`. This is not runtime/path-resolution proof.
The supplied failing-direction probe was also consumed unchanged: 451
commands, `status=ok`, zero blockers/warnings, `readError=null`. The fallback
edge report contains eight unresolved edges, four unclassified as already
recorded. No probe command text was executed and no application or phase tests
were run.

The estimates remain explicitly low-confidence, factor 1, sample count 0:
56=96k, 51/52 effective=71k each, 53=43k, 54=31k; total=312k and continuous
runtime=145k. Calibrated estimate checks are each below the 100k per-plan
budget. Actual context fit, packet counts, throughput and provider capacity
remain unknown.

**Recommendation:** Revise the Plan56 task ordering before execution. This
finding concerns executability of the approved final-code gate; it does not
challenge the user's ordering approval or authorize weakening any transport,
acceptance, or downstream gate.
