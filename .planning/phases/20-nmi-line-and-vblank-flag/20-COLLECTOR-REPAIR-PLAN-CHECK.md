---
phase: 20-nmi-line-and-vblank-flag
status: passed
checked: 2026-10-27
revision_cycles: 1
blockers: 0
warnings: 0
info: 0
---

# Collector repair plan check

Independent review and a fresh independent recheck cover new 20-11, new 20-12 and revised unexecuted 20-10. Completed 20-01 through 20-09 remain historical. This is planning verification, not implementation or hosted acceptance.

The first check found zero blockers and one warning: 20-12 Task 2 claimed job/artifact/source availability checks but its command only checked successful run identities. Revision 1 assigns shared production metadata-only `MODE=availability` and its offline controls to 20-11, then makes 20-12 invoke that exact command. The recheck returned **VERIFICATION PASSED**, with zero issues. The mode and new CTests are explicitly future implementation work, not existing capabilities claimed as tested.

| Check | Result |
|---|---|
| Execution graph | Completed 20-09 -> 20-11 wave 10 -> 20-12 wave 11 -> 20-10 wave 12; acyclic, only 20-11 initially ready |
| Collector repair | Root/nested unique paths, authentic executed per-job results, exact m2 name, supported CLI/errors, canonical campaign destinations, perf/Nightly directory and collision checks, stale-success refusal |
| Offline proof | Planned production collect/validate adapter and metadata-only availability controls; no real network or fixture-based hosted claim |
| Fresh delivery | Persistent-authorized existing PR 59 only; new candidate/run/attempt/job/artifact/merge/base and complete effective-source mapping before content collection |
| Scope | No completed-history rewrite, merge, force push, new PR, workflow dispatch, unrelated CI proposal or dependency |
| Requirements | NMI-01, NMI-02, KEEP-02, KEEP-03 assigned; none marked complete |
| Decisions | Deterministic coverage check: 24/24 covered |
| Fallbacks | Seven existing dispositions and descriptor-less clean-room prohibition retained without manual sign-off |
| Structure | All three active plans have two tasks and valid gap-closure frontmatter/plan structure; no errors or warnings |
| Verify probes | 42 commands: path probe not applicable to these command forms, no severity findings/read errors; failure-direction probe ok, no findings |

Research and external bounce were skipped for `--gaps`. The context-drift advisory about older NMI oracle research remains visible in the planning diagnostic; no hardware decision or research artifact was rewritten. Phase 20 remains open at 9/12 plans complete, pending execution, fresh content acceptance and independent phase verification.
