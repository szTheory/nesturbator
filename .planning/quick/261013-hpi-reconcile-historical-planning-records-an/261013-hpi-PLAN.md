---
phase: quick
plan: 261013-hpi
type: execute
wave: 1
depends_on: []
autonomous: true
must_haves:
  truths:
    - "Historical record closures distinguish completed fixes from accepted deviations."
    - "Release readiness reports publication only when current hosted evidence proves it."
    - "Unrelated technical debt remains visible and historical observations survive."
  artifacts:
    - path: .planning/WINDOWS.md
      provides: Synchronized dispositions and evidence for the ten historical entries
    - path: .planning/MILESTONES.md
      provides: Dated release-blocker closure and accurate publication status
  key_links:
    - from: .planning/WINDOWS.md
      to: .planning/milestones/v1-phases/03-a-real-game-in-retroarch/03-VERIFICATION.md
      via: Hosted run and exact-head references
files_modified:
  - .planning/WINDOWS.md
  - .planning/milestones/v1-phases/03-a-real-game-in-retroarch/deferred-items.md
  - .planning/milestones/v3-phases/13-rewind-and-run-ahead-in-retroarch/deferred-items.md
  - .planning/debug/DEBUG-automation-first-uat.md
  - .planning/debug/resolved/DEBUG-automation-first-uat.md
  - .planning/MILESTONES.md
  - .planning/PROJECT.md
---

<objective>
Reconcile historical planning records with completed work, distinguish accepted historical deviations from actual defects, and record release readiness accurately before the next milestone.

This is standard quick mode: discussion, research and validation agents are disabled. Discovery level 0 applies to documentation and existing ledger tooling. No source, workflow, test or dependency changes are needed. The orchestrator owns STATE.md, final release verification, summary and commits; this executor must not push, open a PR, merge or publish a release.

Assumption-delta probe: skipped because this quick task has no resolvable phase CONTEXT.md assumption scope. Use the explicit task evidence below; do not infer missing history.
</objective>

<context>
@AGENTS.md
@.planning/STATE.md
@.planning/PROJECT.md
@.planning/MILESTONES.md
@.planning/WINDOWS.md
@.planning/preparation/README.md
@.planning/preparation/ENGINEERING.md
@.planning/milestones/v1-phases/02-the-cpu-matches-the-public-vectors/02-09-SUMMARY.md
@.planning/milestones/v1-phases/02-the-cpu-matches-the-public-vectors/02-10-SUMMARY.md
@.planning/milestones/v1-phases/02-the-cpu-matches-the-public-vectors/02-11-SUMMARY.md
@.planning/milestones/v1-phases/03-a-real-game-in-retroarch/03-VERIFICATION.md
@.planning/milestones/v2-phases/05-tune-up-and-v1-debt/05-CI-RECORD.md
@.planning/milestones/v3-phases/13-rewind-and-run-ahead-in-retroarch/13-08-SUMMARY.md
</context>

<tasks>

<task type="tracer">
  <name>Reconcile the broken-windows ledger from evidence through its status reader</name>
  <files>.planning/WINDOWS.md</files>
  <action>
    Capture the initial windows status and audit-uat JSON using the existing GSD CLI. Resolve GSD_TOOLS to the installed gsd-core/bin/gsd-tools.cjs without writing an absolute personal path to tracked files. Use windows fixed and windows waive to keep frontmatter counts, Markdown and the embedded JSON synchronized. Preserve original descriptions, recorded timestamps and historical file references. Record dated evidence in the supported reason field if available; otherwise add a dated evidence section after the generated ledger, after all CLI mutations are finished.

    First close ID 1 using 02-09-SUMMARY.md and commit 08c1df4: the mutation self-test uses the live classifier. Query windows status to prove this one record flows through the writer and reader before processing the remaining records.

    Then mark fixed: ID 2, explicit release retry counter, 02-11-SUMMARY.md commit fe21b93; ID 3, bounded metadata retry and distinct pending exit status, same summary commit c1f1c3c; ID 5, completed rom-loader-fuzz job in Nightly run 37962202123 recorded in 03-VERIFICATION.md; ID 8, all six hosted builds and required CI passed on exact remediation SHA 20331371c8036cd5fccec6491d25262e3a2c0364 in run 37994563525; IDs 9 and 10, local GUI test registration was removed in Phase 05, policy.no-skip enforces registered tests, and hosted retroarch-e2e passed in run 38061698752 on e853abe per 05-CI-RECORD.md. Explain that the local tests were superseded, not repaired locally.

    Waive with explicit reasons: ID 4, a missing historical test-first RED commit cannot be supplied retroactively and remains a documented process deviation; ID 6, per-dot caller-owned framebuffer was an intentional allocator-contract choice (Phase 03 rendering summary and public API); ID 7, the temporary declaration digest was deliberately refreshed for Phase 03's planned input API (03-05-SUMMARY.md). Do not change design or invent a past test commit. Confirm each cited artifact or commit exists before disposition; if evidence contradicts the proposed closure, leave that record open and report it. Expected final counts with current evidence are fixed 7, waived 3, open 0, total 10.
  </action>
  <verify><automated>node "$GSD_TOOLS" windows status</automated></verify>
  <done>All ten records have evidence-backed dispositions; generated counts, table and JSON agree, and original observations remain recoverable.</done>
</task>

<task type="auto">
  <name>Append closure evidence to the deferred items and diagnosed debug record</name>
  <files>.planning/milestones/v1-phases/03-a-real-game-in-retroarch/deferred-items.md, .planning/milestones/v3-phases/13-rewind-and-run-ahead-in-retroarch/deferred-items.md, .planning/debug/DEBUG-automation-first-uat.md</files>
  <action>
    Preserve the original observed failures and audit acknowledgements. Add clearly dated 2026-10-13 resolution entries, with current statuses recognized by the existing audit reader. For the Phase 03 local retroarch.testframe deferral, record resolved by removal of local GUI tests and required hosted coverage in Phase 05, citing 05-CI-RECORD.md run 38061698752; retain the original acknowledged status as historical context rather than making it look newly successful locally.

    For the heading-only Phase 13 Intel timeout entry, append status resolved and cite 13-08-SUMMARY.md: run 38295163207 measured Intel at 640 seconds, build budget became 22 minutes, and final run 38297707729 on 137d855 passed every CI job. Note separately that current ci.yml uses 23 minutes following later measurements; do not alter a workflow or rewrite the historic 22-minute fix.

    Close DEBUG-automation-first-uat.md only after tracing all three requested remedies to 02-09, 02-10 and 02-11 summaries and their surviving policy checks, result collector and GSD-HANDOFF.md rule. Update current frontmatter to resolved with the closure date, retain original diagnosis and audit_acknowledged metadata, and append resolution evidence that distinguishes the implemented automatic checks and evidence collector from clean-room provenance, which remains a judgment claim. Do not describe the current release as published merely because the collector exists. If any original remedy lacks evidence, keep that part explicitly unresolved.

    Run audit-uat again and inspect the item-level delta. Archived milestone files may be outside the tool's active-phase scan; inspect those edited files directly too. A zero aggregate count alone is insufficient proof. Leave unrelated substantive debt, especially Phase 13 WR-01 through WR-03 and milestone audit findings, untouched.
  </action>
  <verify><automated>node "$GSD_TOOLS" audit-uat</automated></verify>
  <done>Each targeted historical issue has a dated closure supported by named artifacts; original observations survive and unrelated unresolved debt stays open.</done>
</task>

<task type="auto">
  <name>Correct release metadata and verify the documentation-only scope</name>
  <files>.planning/MILESTONES.md, .planning/PROJECT.md</files>
  <action>
    Obtain current read-only GitHub evidence with gh pr view 35 --json state,mergedAt,mergeCommit,url,statusCheckRollup; gh run view 38432194730 --json status,conclusion,headSha,url,jobs; and gh release view v0.1.10 --json isDraft,publishedAt,targetCommitish,url,assets. Parent/orchestrator owns the final publication check; share the timestamped observations with it. PR 48's formatting fix merged as b8e2824, and PR 35 merged on 2026-10-13T16:38:38Z as da2bf97 after 15 successful CI jobs. Those facts close the formatting/merge blocker, independently of publication.

    Update the v3 timeline's current release status and append a dated release-blocker closure to MILESTONES.md while preserving what was true at milestone close. Correct PROJECT.md's current-state release paragraph to match observed status. At planning time v0.1.10 is a draft with zero assets and publication run 38432194730 is still running; report publication pending unless current successful publication evidence supersedes this. A merged release PR is insufficient. A published claim requires a successful exact-head publication run, non-draft release with publication timestamp, and expected asset set per ENGINEERING.md and the release workflow. If publication fails or remains pending, state that remaining readiness limitation and let the parent retain it in STATE.md. Do not retry or mutate release workflows during this cleanup.

    Review the scoped git diff and run existing hygiene checks. Confirm that only the six declared planning records plus parent-owned quick artifacts and STATE.md changed, no C or workflow behavior changed, and no new tests were added. Report the remaining substantive debt without closing or copying it into fake fixes. Give the orchestrator final evidence and validation results for its summary and STATE.md update; do not edit STATE.md here.
  </action>
  <verify><automated>git diff --check &amp;&amp; sh scripts/hygiene.sh --tree</automated></verify>
  <done>Active release metadata matches current evidence; historical blocker observations remain dated, pending publication is not overstated, and documentation passes diff and hygiene checks.</done>
</task>

<verification>
Run windows status and audit-uat before and after changes. Compare targeted records, not only aggregate totals. Check preserved historical text, working evidence links and exact commit/run references. Run git diff --check and scripts/hygiene.sh --tree; inspect git diff --stat and the full scoped diff. No behavior change means no new C tests or full emulation-suite rerun is required for this task.
</verification>

<success_criteria>
Seven fixed and three waived ledger entries are supported by evidence, or any contrary evidence leaves the affected item explicitly open. The three historical deferred/debug records have honest current dispositions. PROJECT.md and MILESTONES.md distinguish completed release-PR merge from publication. Substantive technical debt remains visible. The parent receives any pending release result for its final readiness check.
</success_criteria>

<output>
Return changed paths, exact validation commands/results, the before/after ledger counts, closure evidence and any unresolved release status to the orchestrator. The orchestrator creates 261013-hpi-SUMMARY.md and updates STATE.md, then handles commits and the next workflow step.
</output>

<execution_adjustment>
Final integration review archives the resolved debug record under
`.planning/debug/resolved/`, following the installed GSD debug convention.
This keeps the progress workflow's root-level active-session count accurate;
relative evidence links move with the record. The current PROJECT.md CTest
count includes the release-format regression (634); the historical milestone
count remains 633. No implementation scope changes.
</execution_adjustment>
