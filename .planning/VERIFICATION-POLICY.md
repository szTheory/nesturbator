# Verification and CI/CD efficiency policy

Owner direction recorded 2026-11-01. Applies to OpenGSD discussion, planning,
implementation, verification, review, shipping and release work in this project.

## Default workflow

1. During planning, identify the observable outcome and cheapest independent
   oracle. Resolve verification access and seams before implementation ends.
2. Reuse existing unit, seam, integration, smoke or end-to-end checks. Exercise
   the actual boundary when a lower-level check cannot detect the failure.
   Prefer a bounded local probe or throwaway script for one-time questions.
3. Inspect test selection, fixture closure, source/build identity and resource
   limits before execution. Retain explicit stops and no-retry constraints.
4. Run useful authorized checks and review their output before handing off.
   Automated evidence and bounded agent review of source/documentation can
   discharge technical questions; an agent review is identified as such.
5. Add a maintained CI check only for a recurring failure with independent
   expected behavior, useful detection beyond existing coverage, deterministic
   execution, and justified runtime/maintenance cost. Place it in an existing
   target or lane when practical. Record why a local check stays local.
6. Record the result, evidence, limits and disposition once. Preserve unique
   regression coverage and reuse still-current evidence. Reopen only for an
   attributable source/contract change, new failure, missing evidence or newly
   necessary owner decision, rather than a stale checklist or generic flag.

The target is zero avoidable human UAT. Hand a check to the owner only when
its remaining uncertainty needs unavailable physical/sensory access, an owner
preference, materially new authority/cost/risk, or an irreversible choice. State
the specific unavailable evidence and practical consequence. Historical
`human_judgment` labels alone do not make an owner checkpoint necessary: first
reassess them against this policy and record an explicit superseding disposition.

## Authority and evidence limits

Automation does not create permission, certify undocumented past approval,
establish hardware correctness from hashes, or accept unknown risk. Consult
existing authority for the action being considered; preserve a stop automatically
when no new action is required. Acknowledging a preserved boundary is not UAT.
If a necessary new action lacks authority, complete its concrete reviewable
preparation and ask only about that action. Retain attribution uncertainty as a
limitation; do not invent a historical authorization or erase a real failure.

Existing required acceptance stays intact. A cheap probe supplements rather
than replaces full CI, platform equality, performance or other approved gates.
Evidence reused for unchanged behavior stays bound to its original candidate.

## Evaluate CI and release efficiency

Whenever CI/CD is running or has completed, inspect available run/job/step
timing and failures read-only during other useful work or at completion. Use
recorded metadata when a fresh fetch adds no value. Identify the actual critical
path, repeated work, setup overhead, avoidable reruns and duplicate triggers;
distinguish elapsed time, summed job runtime and billing estimates.

Prefer removing unnecessary work and reusing current results before adding
caches, dependencies or concurrency. Preserve every unique assertion, required
platform, failure diagnostic, artifact/provenance check, resource bound and
publication gate. Implement low-risk improvements within the authorized scope;
record a concrete measured opportunity when it requires separate scope. A
completed review may correctly find no immediate safe change.

An efficiency review does not itself trigger CI, restart a stopped campaign,
push, merge or publish. Shipping/release steps still use their existing authority
and exact-current-head gates. This policy does not start another lifecycle step.

## Milestone release cadence

For each milestone that delivers a release-worthy change, treat release as a
closeout gate rather than deferred cleanup:

1. Run the accepted local workflow within its existing time and resource limits;
   diagnose failures before any bounded retry.
2. Triage related open PRs and verify the exact candidate's required CI. After
   merge, confirm the required main-branch CI is green on the resulting head.
3. Check that project-owned worktrees are clean. Preserve unrelated user work;
   if it leaves the release tree dirty, establish ownership and resolve that
   before publishing rather than discarding it.
4. Publish the package from the verified source and confirm the expected
   archives, checksums and version/source identity. Record the release evidence
   and next action in project state.

This is standing owner authority for the normal milestone release path, including
its required hosted checks, pushes, PRs, merge and publication once their gates
pass. A milestone without a package-worthy change still completes its CI, PR
triage and cleanliness checks and records why no package was published. This
cadence never restarts a stopped or interrupted CI/acceptance campaign, changes
its limits, or upgrades unknown historical outcomes into evidence.

## Applied precedent

[Phase 24 disposition](phases/24-correct-remaining-ntsc-dma-behavior/24-AUTOMATION-DISPOSITION.md)
replaces a redundant three-item UAT checkpoint with preserved operational
authority limits, source/documentation review and existing native-state checks.
It records why no new permanent CI suite or workflow run was needed. Future
verification should reuse that disposition under the same conditions.
