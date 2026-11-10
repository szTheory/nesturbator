# Phase 28: Qualified NES gameplay package - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-11-09
**Phase:** 28-qualified-nes-gameplay-package
**Areas discussed:** Performance recovery disposition after Plan 28-12; measured receipt inputs and tool evidence; evidence route after Plan 28-17 capture-3 integrity failure; profiling evidence and stop confirmation after D-37

---

## Workflow gates

- **Existing context:** The owner chose to update it.
- **Existing plans:** The owner chose to continue and replan after this discussion.
- **Owner direction:** Consider all relevant perspectives, synthesize a recommendation, and apply it.

## Performance recovery disposition after Plan 28-12

| Option | Description | Selected |
|--------|-------------|----------|
| Keep the current stop; leave 28-13 pending | Preserve D-28 and require a separate evidence-authority decision before further work. | ✓ |
| Authorize only the documentation-only NO-GO record | Adds source/rights audit detail but cannot provide a performance pass or unblock PKG-01/PROF-01. | |
| Propose a separate performance-evidence path | Would require a distinct explicit authority decision and an admissible source-bound route under the unchanged per-workload gate. | |

**Disposition applied:** The owner requested specialist fan-out and automatic application of the synthesized recommendation. Performance engineering, verification/release governance, and product/consumer reviews all recommended preserving the stop. The verified capture failed all three unchanged raw +1% workload gates; documentation-only work cannot meet the release requirements. Plan 28-13 remains pending and unexecuted, both capture roots remain spent, and the D-21 allowance remains unused.

**Notes:** This discussion permits the owner's requested re-planning step only. It does not authorize the 28-13 read/hash/document task, any new measurements or tests, source edits, hosted CI, publication, or a campaign restart. Phase 28 remains blocked on PKG-01 and PROF-01.

---

## the agent's Discretion

- Applied the three-lens recommendation because it preserves the existing safety stop and does not lower acceptance gates or expand evidence authority.

## Deferred Ideas

- Revisit a new evidence-authority path only through a separate explicit owner decision that identifies a credible, source-bound route to the remaining requirements.

## Follow-up: source-bound D-21 recovery route

| Option | Description | Selected |
|--------|-------------|----------|
| Keep the stop and do no more planning | Leave the D-21 allowance unused and require a later decision. | |
| Replan one bounded source-bound route | Plan a fresh baseline, an all-workload pre-edit cost gate, and at most one fresh-root candidate comparison under D-21. | ✓ |

**Disposition applied:** The owner approved planning only. The plan may assign `capture-3` to the frozen current-source baseline and `capture-4` to the one candidate comparison. `capture-1` and `capture-2` stay spent. The plan must preserve the current raw +1% gate, all three workloads, D-20 compatibility, and existing limits; stop before edits if the conservative pre-edit case does not credibly clear every workload. Any later candidate failure is terminal. No tests, builds, probes, measurements, profiling, captures, source edits, hosted CI, PR update, publication, or campaign restart are authorized by this decision. The owner must separately authorize execution before the plan's tasks run.

## Follow-up: measured receipt inputs and tool evidence

| Option | Description | Selected |
|--------|-------------|----------|
| Keep 426; exclude tools | Exclude the capture helper and its synthetic test from measured receipt inputs; bind both hashes separately. | ✓ |
| Revise route to 428 inputs | Treat the helper and its test as measured inputs and replace the accepted 426-path contract. | |

**Disposition applied:** The owner selected “Keep 426; exclude tools.” The current `pr_local_manifest` selector matches 428 paths because it includes `scripts/phase28_act_capture.py` and `tests/perf/test_phase28_act_capture.py`; the 28-09 and 28-10 accepted baselines each contain 426 and omit those two paths. The 28-13 plan update must revise the selector and its independent checks to exclude exactly those tool files, retain the selector itself as measured input, and bind helper/test hashes separately. The updated selector requires a newly computed 426-path manifest digest at the later source freeze. Preserve the existing dirty work and partial-plan commits. This discussion records the contract and authorizes planning only; it does not start source freeze, capture-3, or additional tests/builds/probes.

## Follow-up: continuation after the manual 28-13 halt

| Option | Description | Selected |
|--------|-------------|----------|
| Create a new numbered successor and rewire 28-14/15 | Preserve 28-13's halted record; put the incomplete pre-edit work under a fresh plan and make downstream plans depend on its checked result. | ✓ |
| Reopen or rewrite halted 28-13 | Preserves existing plan numbers but risks erasing the truthful manual close-out and treating incomplete verification as a valid predecessor. | |
| Stop and defer the D-30 route | Preserves all current stops but leaves the Phase 28 performance gate blocked. | |

**Disposition applied:** The owner asked for a recommendation and authorized following it. Three focused reviews across planning/dependencies, measurement evidence, and release authority converged on the successor plan. Keep the 28-13 halted summary immutable; it says revised Task 1 remains unverified, Task 2 did not start, and no source freeze or capture-3 occurred. Rewire 28-14/15 to a new successor's independently checked output, and reconcile the plan index, ROADMAP, and STATE during planning. The existing D-30 execution grant was recorded for plans 28-13/14/15; verify its exact coverage against the new plan identifier and actions before execution. If coverage is unclear or insufficient, stop before task work. This discussion and the next planning step do not run tests, builds, source freeze, captures, production edits, hosted CI, publication, or campaign work. Roots 1/2 remain spent; roots 3/4 and the sole candidate allowance remain unused.

## Dependency preference

**Disposition applied:** Carry forward the owner's preference to use existing tools, the standard library, or a small owned helper before adding a dependency. No new runtime/tooling package or recurring CI lane is justified for this continuation.

## Follow-up: recovery after Plan 28-16 halted

| Option | Description | Selected |
|--------|-------------|----------|
| Stop the D-30 route here | Preserve all budgets, but leave the remaining Phase 28 performance gate blocked. | |
| Plan a fresh successor with corrected, independently checked tooling | Preserve 28-16's failure records, repair the actual evidence path before another freeze, and keep all capture/candidate gates and limits. | ✓ |

**Disposition applied:** The owner asked for specialist fan-out and directed the assistant to apply the synthesized recommendation. Planning a fresh numbered successor is the recommended path because the current blockers are defects in the measurement tooling, not evidence that the APU candidate passed or failed. The route must not reuse the rejected 28-16 baseline or retry its failed one-shot stage. A successor should prove the real freeze/build/feasibility/qualification/capture-admission chain: include the baseline role, use actual CMake targets, independently recompute and bind feasibility, and require the source-bound qualification and one-candidate records before capture admission. The three-workload raw +1% gate, 426 measured inputs, existing limits, and D-20/D-21 constraints remain unchanged. This discussion spent no capture or candidate. Recheck the grant against the successor's exact plan identity and scope before execution; this discussion authorizes planning only.

**Research note:** The decision rests on the repository's retained plan, helper, driver, freeze, and test evidence. External research would not resolve these project-specific authority and contract questions.

## the agent's Discretion

- Keep the route on owned tools and standard-library code; do not add a dependency or recurring CI lane without a demonstrated need.

## Follow-up: after Plan 28-17 capture-3 integrity failure

| Option | Description | Selected |
|--------|-------------|----------|
| Preserve the terminal stop | Keep capture-3 spent and non-qualifying; keep capture-4 for the conditional candidate; leave 28-14/15 blocked. | ✓ |
| Salvage capture-3 as feasibility evidence | Re-verify or reinterpret the retained raw files and use their logged comparator values. | |
| Reassign a root or capture under the current grant | Use capture-4 as a baseline or create a new root without renewed authority. | |
| Continue only after a separate new grant | If the owner wants to pursue the APU path, define a new exact baseline scope and limits before planning or execution. | Deferred |

**User's choice:** The owner requested quality recommendations and directed applying the recommended course.

**Disposition applied:** Three independent reviews covered evidence admissibility, performance engineering, and authority/provenance. All recommended preserving the halt under the current D-30/D-34 scope. Plan 28-17's only capture-3 invocation failed integrity before an accepted inventory, verified counts, or feasibility model. The +2.46%, +2.46%, and +2.69% console rows are diagnostic only. Re-running verification on the spent root is not safely read-only because verification can write inventory or capture records. Keep capture-4 candidate-only and the sole D-21 candidate unused; do not run a probe, feasibility producer, candidate, or another capture. A new baseline route needs a separate owner grant naming its exact root, actions, and limits. This discussion does not supply that grant.

**Notes:** The specific identity mismatch is a project-owned capture-helper contract defect: live preparation adds a `capture_ledger` entry that the verifier's reconstructed route omits. The synthetic contract did not exercise that live augmentation path. The recommendation is based on the repo's source-bound evidence and authority records; outside ecosystem research would not change the project-specific scope decision.

## the agent's Discretion

- Applied the preserve-the-stop recommendation because no admissible evidence route remains inside current authority.

## Deferred Ideas

- The owner may separately authorize a fresh source-bound baseline route with an exact new root and limits. Until then, Phase 28's APU recovery and dependent release plans remain blocked.

---

## Follow-up: recovered Plan 28-17 result and APU table disposition

| Option | Description | Selected |
|--------|-------------|----------|
| Update it | Continue the discussion and revise the existing context with the recovery result. | ✓ |
| View it | Show the existing context and stop. | |
| Skip | End the discussion without changing the context. | |

**User's choice:** Update it, following the recommendation to record the recovery result and retire the current APU table attempt.

**Disposition applied:** The recovered capture verifies integrity but fails the unchanged +1% gate on all three workloads. The raw files do not provide the `src/apu.c` line counts needed for a defensible savings estimate. The direct-index APU table attempt is retired without candidate edits; capture-3 stays spent, capture-4 remains untouched and candidate-only, and the D-21 candidate remains unused. No concrete measurable alternative was identified, so no new measurement is planned or authorized. A future route requires a source-backed measurable alternative and a separate grant naming its exact baseline root, actions, and limits. See D-37 in `28-CONTEXT.md` and [28-17-RECOVERY.md](28-17-RECOVERY.md).

---

## Follow-up: profiling evidence and stop confirmation after D-37

| Option | Description | Selected |
|--------|-------------|----------|
| Preserve D-37 and wait for a distinct source-backed candidate plus exact grant | Keep the current performance and release stop; do not plan or run a new profile now. | ✓ |
| Treat debug symbols as sufficient reason to profile the retired event-table idea again | Could expose line counts, but does not supply a new candidate or cost case and conflicts with D-37's stop for this approach. | |
| Start another APU optimization idea now | Region/mode restructuring or a derived next-event cursor has no measured conservative path under all three rows. | |
| Waive performance or use local artifacts as release proof | Would contradict the unchanged comparator and published-receipt gate. | |

**User's direction:** Follow the recommendation and apply it.

**Disposition applied:** Three independent reviews considered profiling/tooling, APU timing and state compatibility, and product/release evidence. All recommend preserving D-37. Plan 28-17's Release build omitted debug information, which explains the missing src/apu.c line records. Plan 28-07 had already demonstrated an optimized diagnostic binary with debug information and matching .text and 60/360 instruction totals, but on source 1c674c1; its event-cursor cost model still missed M4 by 6,474,328 instructions. That method precedent does not transfer to current source or establish a new candidate. The unchanged three-workload +1% gate and all release/provenance requirements remain. Keep capture-3 spent, capture-4 candidate-only, the D-21 candidate unused, and the dependent release work blocked. Any distinct future performance route needs a measurable source-backed case and separate explicit authority naming its exact baseline root, actions, and limits. The official [Cachegrind manual](https://valgrind.org/docs/manual/cg-manual.html) and [GCC debugging options](https://gcc.gnu.org/onlinedocs/gcc/Debugging-Options.html) support the debug-information finding; they do not alter project-specific authority or acceptance.

**Notes:** No files outside planning documentation were changed. No tests, builds, captures, measurements, hosted CI, PR updates, or release actions were run.

---

## Follow-up: bounded v9 release-recovery authorization

| Option | Description | Selected |
|--------|-------------|----------|
| Authorize the proposed bounded route as written | Inspect existing CI evidence, then allow one optimized `-g` Cachegrind diagnostic on the exact baseline/candidate roots only if source attribution is missing; permit at most one narrow correction if a conservative model can bring all three workloads within +1%, followed by one local workflow and one exact-head required CI run under the recorded limits. | ✓ |
| Specify a different root, action set, or limit | Replace any part of the proposed scope before planning or measurement. | |

**User's choice:** “yes i authorize” — the proposed scope as written.

**Disposition applied:** D-39 records the exact roots, actions, limits, and stop conditions. D-37/D-38 remain preserved for the retired D-21 table route; capture-3 stays spent and unchanged, capture-4 stays untouched, and no candidate or measurement has run. The v9 provider release is prioritized ahead of starting MMC2, but LibFuturo's installed consumer, NES host, and gameplay acceptance remain consumer-owned.

**Notes:** This discussion updated planning context only. No build, test, profile, measurement, hosted CI, PR update, merge, or release action ran.

---

## Follow-up: package-only release route review (2026-11-09)

| Option | Description | Selected |
|--------|-------------|----------|
| Preserve the current stop and request an exact grant before any new candidate | Keep D-39 terminal; use the package-only route only after the owner accepts the profile/consumer contract reset and names its source root, actions, and limits. | ✓ |
| Start a candidate immediately from PR #64 base `b25146c6efaac41c58f60555f59f0fe79a90ca68` | Could avoid PR #64's runtime performance delta, but requires changing the accepted region-profile contract and requalifying the consumer and release evidence. | |
| Reuse PR #64's current profile/tests on that base | Does not compile or validate truthfully: they require regional APIs absent from the base. | |

**Disposition applied:** Independent route and consumer reviews found a technically plausible package-only candidate, but it is not authorized by D-39 or the existing Plan 28-06. The candidate would need to start from the exact base above without emulator-source or build-flag edits, replace the current region-dependent profile/tests with a truthful NTSC-only contract, and requalify the DABG package evidence. It would still require one local workflow, exact-candidate required CI (including its performance comparator), resulting-main CI, release-source checks, and published-artifact proof. The current profile/tests use region APIs introduced by PR #64; the existing accepted Phase 28 region contract and Plan 28-06 cannot be silently weakened or rebound. No candidate, test, workflow, CI, measurement, PR, or release action was started. The recommendation is to hold at this boundary and ask for a separate exact owner grant before planning the reset route.

**Evidence:** Current PR [#64](https://github.com/szTheory/nesturbator/pull/64) remains open at `36aab4c`; required [run 41931113009](https://github.com/szTheory/nesturbator/actions/runs/41931113009) failed its +1% performance comparator, while [Nightly run 41931113053](https://github.com/szTheory/nesturbator/actions/runs/41931113053) passed but is not required CI. [v0.1.12](https://github.com/szTheory/nesturbator/releases/tag/v0.1.12) remains latest and ineligible for the approved LibFuturo gate. See D-41 in `28-CONTEXT.md`, Plan 28-06's release preconditions, the current LibFuturo STATE/product roadmap, and its installed-consumer contract.

---

## Follow-up: owner authorization for the package-only reset (2026-11-09)

**Owner's direction:** “yes i authorize that” — authorization applies to the exact bounded route presented in the preceding message.

**Authorized scope (D-42):** Create one clean candidate from base `b25146c6efaac41c58f60555f59f0fe79a90ca68`, with no emulator-source or build-flag changes. Change only the truthful NTSC-only provider profile, pinned DABG package/rights/notice evidence, Nesturbator installed-consumer tests, and release identity/records. Run one local `cmake --workflow --preset ci`, one exact-candidate required CI run, and one resulting-main CI run. Stop on any failed comparator, protected campaign, or CI gate; do not retry or create another candidate. No Cachegrind/APU work, capture changes, AccuracyCoin baseline edits, or PAL/Dendy claims.

**Applied recommendation:** Treat this as a fresh release scope from the previous candidate, not as a repair or retry of D-39. D-39 remains terminal. Use the actual public API at the exact base to write the profile and consumer assertions; requalify the DABG evidence against that base and preserve its pinned bytes and notice. Keep the current dirty MMC2 checkout and uncommitted evidence untouched by using an isolated clean candidate worktree. Do not claim that the Nesturbator package alone completes LibFuturo Phase 9: its relocated consumer, NES host, and gameplay acceptance remain LibFuturo-owned.

**Discussion result:** The independent technical/consumer, release-route, and verification-efficiency reviews converge on the same route. The profile must describe only behavior the base API exposes; the fast checks stay useful for iteration, but the authorized full local workflow and both required hosted CI gates remain unchanged. No additional CI optimization was justified, and no dependency is needed. Planning is the next GSD step; no candidate, test, local workflow, hosted CI, PR, merge, or publication has started in this discussion step.

## Execution follow-through: terminal D-42 local gate — 2026-11-10

Plans 19–21 prepared and qualified the scoped consumer/profile/rights/receipt. The single frozen-source local workflow failed its protected AccuracyCoin identity checks because the allowed CTest registration additions alter tests/CMakeLists.txt, a member of the protected 56-input fingerprint. Read-only per-file reconstruction identifies no other changed fingerprint path. Planning missed this available closure dependency before spending the run. D-42 remains terminal, with the baseline unchanged and no retry, new candidate, push/PR, hosted run or publication. No owner acknowledgement is needed to keep work stopped. The proposed next scope is narrow evidence maintenance, with every protected outcome/provenance guard retained; it is not approved. Read 28-RESET-FAILURE.md and use `$gsd-discuss-phase 28 --ws libfuturo-nes-gameplay-handoff` to discuss the choice. Context may be cleared; the owner acts next.

## Owner workflow defaults and autonomous follow-up — 2026-11-10

The owner requested a fresh-context autonomous entry that reaches the pending discussion. They explicitly prefer bounded subagent fanout across relevant stakeholder/technical roles, high-quality primary sources when research adds value, comparison of alternatives/patterns/footguns, coherent recommendations, and an adversarial pass until further review is unlikely to change the result. Routine recommendations consistent with project direction are delegated to the agent. Dependency choices retain the small owned-helper/justified-duplication preference.

They reaffirmed early useful automated verification, minimal necessary human UAT, throwaway checks for one-time questions, maintained CI only for recurring value, evidence/decision reuse to prevent churn, and CI/CD timing/cost review with all unique coverage and safety gates preserved. The authoritative rules are `.planning/DISCUSSION-POLICY.md` and `.planning/VERIFICATION-POLICY.md`, reached from AGENTS.md.

Saved entry: `$gsd-autonomous --ws libfuturo-nes-gameplay-handoff --from 28`. Pending first action: `$gsd-discuss-phase 28 --ws libfuturo-nes-gameplay-handoff`, before context-exists skip or generic execution-blocker routing. This preference/re-entry authorization does not renew D-42, permit a baseline change, allocate another candidate/run or waive a gate. The old stop stays in force while the new discussion prepares a concrete decision. No discussion result or new technical scope has been fabricated here.

## Follow-up: historical and candidate evidence identity — 2026-11-10

**Entry:** `$gsd-autonomous --ws libfuturo-nes-gameplay-handoff --from 28`.
The recorded pending discussion ran before the existing-context shortcut and
execution-blocker routing. Existing settled decisions and failed evidence were
preserved. No new user answer or execution approval is implied.

Three read-only reviewers covered evidence/build identity, CI/release authority
and efficiency, and provenance/product boundaries. Their roles used the resolved
GPT-6 Luna / xhigh routing. All recommend a separate versioned current binding
while keeping historical baseline metadata and outcomes unchanged. A bounded
adversarial pass rejected a self-authorizing mutable hash exception, required
independent controls for changed verifier bytes and warned about checker
self-inclusion. The orchestrator separately confirmed that the full configured
activation fixture launches a fresh campaign; preflight must not accidentally
consume it, and a future allowance must name it explicitly.

| Option | Disposition |
|---|---|
| Preserve historical baseline; separate narrowly checked current binding | Recommended; source/run authority pending |
| Replace historical input hash | Rejected: risks relabeling old result provenance |
| Hide registrations in an unhashed include | Rejected: evades the protected dependency boundary |
| General closure redesign | Deferred: broader than the observed four registrations |
| Preserve stop | Applied now; remains the alternative if proposal is declined |

An independent read-only probe reproduces both 56-input hashes, the sole
`tests/CMakeLists.txt` delta and all six retained failure-artifact digests.
No project checker/test/build/campaign or remote action ran. The detailed
recommendation, sources, adversarial requirements, exact proposed source/root
scope, automated checks, resource limits and owner question are recorded once
in [the decision review](28-EVIDENCE-MAINTENANCE-REVIEW.md). Its supporting
[check record](28-EVIDENCE-BINDING-CHECK.json) is source attribution, not
acceptance. Existing CI timings were reused; no CI change or new lane is justified.

**Owner choice:** pending. D-43 records discussion disposition only, not a grant.
Mark follow-up discussion handled in STATE/resume. Autonomous re-entry must
present the saved decision without repeating research or starting execution.

## Owner approval — 2026-11-10

The owner selected **“Approve the scoped remedy and fresh route”** in response
to the saved complete proposal and its approval question. D-44 authorizes that
exact proposal: narrow historical/current input binding and checker controls,
minimal successor receipt plumbing, one fresh candidate from
`2e0f2057304f60bab9a06a085a3e9477f1da022b`, the named fresh checkout/output
roots, bounded pre-freeze controls, one local full workflow with its configured
fresh campaign (four workers, 1500-second child/1800-second outer limits), one
exact-candidate required CI and one resulting-main required CI, then only the
normal successful release/publication/artifact gates. Stop on a failed actual
gate, interruption or timeout without retry, refreeze or second candidate.
Preserve baseline/result/ROM/provenance bytes and all old routes/campaign/capture
stops. This does not authorize MMC2 retry, emulator/build/workflow changes or
other profiling/capture/campaign work. Continue the authorized current workflow
without per-step acknowledgements.
