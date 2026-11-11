# Phase 28: Qualified NES gameplay package - Research

> Current D-42 planning uses [28-RESET-RESEARCH.md](28-RESET-RESEARCH.md) as the bounded source-grounded addendum. The APU/capture route below is historical and retired; its old active-source, 426-path, regional compatibility and successor-authority instructions do not apply to the separate exact-base package reset. Preserve its failed evidence and stops rather than executing it again.

**Researched:** 2026-11-08
**Domain:** Source-bound performance evidence tooling and conditional release recovery
**Confidence:** HIGH for repository contracts; MEDIUM for future execution feasibility

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

## Implementation Decisions

### Licensed NROM gameplay route and progress oracle
- **D-01:** Keep DABG as the selected NROM. Its pinned upstream source commit is `5ecc60b6af3f726851bfeb1c2555388c5953e0c0`; its manifest ROM SHA-256 is `eca79b9d0b546e96c1bc73099e239539fb9c7773ff10a431bb3a9cd6763208ca`. Nesteroids was investigated first but not selected after its source-to-ROM mapping could not be defended from the available artifacts. Do not silently switch the accepted scenario back to Nesteroids; remove stale Nesteroids-candidate wording from phase acceptance records when replanning.
- **D-02:** Preserve the existing DABG source-mapped gameplay predicate and bounded controls documented in `docs/nrom-gameplay.md` and the provider profile. The rebuilt DABG source/map evidence supports the RAM locations used; no guessed addresses are acceptable. A semantic change to the scenario, its oracle, or its input recipe refreezes the source identity and requires affected requalification.
- **D-03:** Keep the accepted DABG route deterministic and bounded, including its positive gameplay predicates, visible native-pixel response, non-silent gameplay PCM, and no-input/refused-load controls. Do not replace these with input activity, a title picture, audio alone, or frame hashes alone.

### LibFuturo provider handoff
- **D-04:** Produce a Nesturbator-owned, exact-version NTSC profile based on the current public API and accepted package/source. Cover identity, region and metadata policy, load/refusal and active-session postconditions, controller sampling, pixel interpretation, exact video/audio cadence and bounds, lifecycle, and consumer limits. State that an NTSC preference does not override explicit PAL metadata and Dendy remains refused.
- **D-05:** Keep the public C API unchanged unless current consumer evidence demonstrates a missing capability. Do not edit the sibling LibFuturo checkout or claim its installed consumer, native host, or gameplay acceptance. Distinguish battery-backed game memory from opaque machine snapshots; mark capabilities without current consumer proof unqualified. PAL-through-LibFuturo, browser/device behavior, and Playstead remain unqualified.

### Package and release lineage
- **D-06:** Preserve v0.1.12 as the latest published baseline. PR #64 currently still carries version `0.1.12`; that source version is not a new release and cannot qualify LibFuturo. Do not force a version or claim local packages are released; let configured Release Please derive the next version from merged commits.
- **D-07:** Freeze and qualify the exact final source/package/profile/scenario identity before publication. The published receipt must bind the release tag/source/version, ABI, behavior revision, state format, actual archive digests and attestations, profile and gameplay-guide digests, DABG source commit and ROM digest, explicit Zlib license source/terms, and required notice location/digest. PKG-01 remains pending until this published receipt exists and verifies. Follow `.planning/VERIFICATION-POLICY.md`; do not transfer historical results to the new candidate.

### Verification and release sequencing
- **D-08:** Keep the current verification result truthful. PR #64 remains the single candidate at remote head `36aab4cdbc57f7f4ba2a14cb90b3b73f89640f0c`. The accepted local ACT source is a distinct commit, `1c674c1869aa0feb0418e05c90d5214fa5dc6617`, based on `b25146c6efaac41c58f60555f59f0fe79a90ca68`; its full local workflow passed all 726 tests in 406 seconds, but ACT measured +2.46% for Nesteroids, +2.46% for Runner, and +2.69% for MMC4 against the unchanged +1% gate. Hosted Nightly run `41931113053` completed successfully; no PR update, new hosted run, merge, or publication occurred. Preserve the earlier failed exact-head run `41756835289` and keep all candidate, merge, and hosted identities distinct. Discard the earlier +0.21% scratch-branch result as candidate evidence. Do not lower gates, infer unproven causes, or transfer historical/local evidence into hosted acceptance.
- **D-09:** Close the remaining evidence through the existing v9 release path, then rerun final phase verification. Keep PR #64 as candidate; bundle diagnosed corrections after focused checks; use one behavior-preserving, narrowly profiled performance correction within Phase 28, moving semantic or broad accuracy changes to separate accuracy scope. Fix Windows verification adapters narrowly unless investigation proves a package/core defect. At the release boundary, freshly triage related PRs; bind exact candidate head and tested merge separately; verify resulting implementation-main CI; require the release-PR and release-source checks that Nesturbator's release workflow/receipt define; observe Release Please tag/source/version; and verify actual artifacts before closing PKG-01/PROF-01. The release-source check is Nesturbator provider evidence and does not expand LibFuturo GATE-02, which remains exact-candidate plus resulting-main CI. Keep the phase pending until the published receipt passes. Preserve all existing stops, budgets, and unrelated work.

### Candidate recovery and LibFuturo priority
- **D-10:** Keep PR #64 as the single candidate. Complete focused local diagnosis and checks for all known failures, bundle the fixes, then refresh its head for the next hosted candidate run only under existing CI authority. Do not dispatch a new hosted run as part of this discussion or planning step.
- **D-11:** Make a narrowly profiled, behavior-preserving optimization for the measured performance regression in Phase 28. If evidence requires semantic behavior changes or a broad accuracy fix, stop and route that work to separately approved accuracy scope.
- **D-12:** Keep Windows fixes in the verification adapters unless investigation establishes a package or core defect. Logs establish that runner-output parsing failed and that relocated consumer build/run reached a later link-evidence assertion; they do not prove the hypothesized line-ending or path-normalization causes.
- **D-13:** Unblocking LibFuturo is the first handoff priority once Nesturbator has a qualified release and complete published receipt. Do not modify LibFuturo or assume its unpublished package layout/profile. LibFuturo GATE-02 remains its approved exact-candidate and resulting-main CI gate; Nesturbator's release-source CI remains a separate provider-side release gate.
- **D-14:** After a valid release exists, send one complete LibFuturo handoff with tag/source/build/profile identity, archive digests, published receipt and notice evidence, and passing exact-candidate, resulting-main, and Nesturbator-required release-source CI links. Do not promise a calendar date while the candidate is failing its gates.

### DABG license, notice, and receipt contract
- **D-15:** Make the final receipt explicitly bind DABG's upstream source URL/commit and ROM digest to a structured rights record: SPDX identifier, pinned license URL/commit, exact license-text digest, permitted-use terms and restrictions, and required-notice path/digest. Validate the checked-in notice against the pinned upstream text. A project source commit, ROM digest, or manifest pin alone is insufficient as the receipt's consumer-facing rights record.
- **D-16:** Preserve the upstream Zlib notice verbatim in Nesturbator's source distribution. Prefer a dedicated `tests/roms/licenses/DABG-ZLIB-LICENSE.txt` copied unchanged from the pinned source, indexed by `THIRD-PARTY-NOTICES.md`, and referenced by path and digest in the receipt. The notice permits use (including commercial use), alteration, and redistribution; it requires accurate origin, marking altered source versions, and retaining the notice in source distributions. Any downstream distribution that includes the ROM must carry the same notice. Current component archives omit DABG; if a future package includes the ROM, its inventory and installed contents must include the notice. Do not claim LibFuturo's downstream repository has been updated.
- **D-17:** Keep the generated receipt at `docs/provider-ntsc-release.json` as a post-publication evidence overlay in the source repository, not as a binary package install file or release asset. Give it the standalone notice path/digest a downstream ROM distributor can carry; record that current component archives omit the ROM so no package-install claim is made without evidence.

### APU performance recovery
- **D-18:** Add one separate, conditional APU frame-counter optimization plan within Phase 28. It is release-blocker recovery for the existing performance acceptance, not a new product capability or a new phase. Do not implement it under the current bus-only 28-04 plan.
- **D-19:** For the first attempt, profile the exact accepted candidate and the existing three Cachegrind workloads before implementation. The resulting exact-source profile was a NO-GO: its conservative replacement-cost estimate clears Nesteroids and Runner but misses M4_P256K_C256K by 6,474,328 instructions. This is an estimated shortfall for the proposed replacement, not a measurement of an implemented optimization; the unchanged candidate still fails the comparator. Tasks 2/3 and Plan 28-06 were not authorized. Do not retry Plan 28-07 Task 1 on the unchanged source or transfer the earlier scratch-branch result. D-21 records the only approved follow-up scope. Keep the existing per-workload, tool, fixture, and CI limits; no hosted run occurs during discussion or planning.
- **D-20:** Preserve the current guest-visible frame-counter, IRQ, and audio behavior. Keep `frame_cycle` as the serialized source of truth; any event schedule/cache is derived per-instance state and must preserve ABI 1, behavior revision 17, state format 6, and the public API, with no new dependency. Rebuild or invalidate it after instance/power initialization, cartridge load, soft reset, frame-counter reset completion, and save-state load. Add load-initialization coverage plus NTSC/PAL four-step and five-step boundary, IRQ, reset-delay, and restored-state tests. Include PAL frame-cycle values accepted by the state loader even where they exceed the live four-step wrap. If correct optimization requires guest-visible timing changes, serialized-state changes, or a broader accuracy fix, stop and route that work to separately approved accuracy scope.
- **D-21:** Allow at most one new, separately planned APU frame-counter recovery attempt after the D-19 NO-GO. This supersedes only D-19's stop against considering another recovery scope; it does not reopen or retry 28-07 Task 1. Before production edits, research and planning must identify a distinct, behavior-preserving approach and calculate conservative replacement cost against all three existing workloads, showing a plausible path under the unchanged raw +1% comparator. Count guard, dispatch, rebuild, and required event costs; do not treat the 178.6M attributed scan cost alone as evidence of success. Keep the exact workload, tool, fixture, and resource limits and all D-20 compatibility constraints. A new candidate's required qualification belongs to the new plan and must use its own exact source identity. If no defensible pre-edit cost case exists, or the single new candidate fails the unchanged acceptance or compatibility gates, keep PR #64 blocked and stop; no further retry or broader subsystem optimization is authorized. Discussion and planning do not authorize implementation, hosted runs, candidate refresh, publication, or a restart of the stopped CI/AccuracyCoin campaign.

### Source ownership after the 28-08 preflight
- **D-22:** The active `phase/28-qualified-nes-gameplay-package` branch owns any successor work. The current checkout is at `6a902c092838395e86a6512d368fe9be1022443a`, but this commit and branch name are not a qualified candidate identity. Do not switch to or transplant the accepted `1c674c1` source under the halted plan's existing constraints. Freeze a new clean commit, tree, and full input manifest only after the source reconciliation below; preserve unrelated dirty work outside that identity. Changing source after the new identity is frozen invalidates its feasibility and candidate evidence. — **Reversibility:** costly — changing the frozen source invalidates the 426-input identity and all evidence tied to it.
- **D-23:** Review the 28 differing or missing qualification inputs recorded by `28-08-PROFILE.md` against the active branch, preserve intended Phase 28 work, and restore/verify the pinned DABG rights and notice contract from D-15 through D-17. Do not force the active branch to match `1c674c1`; do not accept the differences silently. Once reconciled, regenerate hashes for the complete 426-input set on the clean source commit. Treat prior `1c674c1` results and the 28-07/28-08 estimates as historical only; none transfers to the new source. If any input or rights record cannot be reconciled, stop before the APU probe and keep the release blocked.

### D-21 attempt-budget disposition
- **D-24:** Plan 28-08's source-lineage preflight did not consume the one D-21 APU candidate attempt: it halted before generated-code/footprint feasibility, baseline tests, production edits, ACT comparison, or a new candidate identity. Plan 28-08 remains closed and must not be restarted. After D-22 and D-23 are satisfied, one successor plan may test the already-researched direct-index immutable event-table approach as the sole remaining candidate. It must recompute generated-code, table-footprint, baseline-behavior, and conservative three-workload cost evidence against the new exact source before edits; prior margins are research history, not a GO. Keep the raw +1% comparator, D-20 behavior and compatibility requirements, and all existing workload/tool/resource limits. If feasibility is not a GO or the single candidate fails, stop without another attempt; keep PR #64 and Plan 28-06 blocked. Discussion/planning authorize no implementation, tests/probes, hosted CI, candidate refresh, publication, or campaign restart.
- **D-25:** Revise only the local ACT evidence-capture proposal after the 28-10 raw-output halt. The generated local workflow should write beneath ACT's runtime `$GITHUB_WORKSPACE` in the bind-mounted `act-workspace-N`, rather than hard-coding `/github/workspace`; the host-side driver must then require and hash all 18 nonempty Cachegrind outputs and `summary.md` before parsing or considering feasibility. Bind those bytes to the exact source/input manifest and pinned ACT/image/compiler/Cachegrind/workload identities, verify one valid `summary:` count per raw file and stable repeated base samples, and independently recompute the unchanged integer +1% gate. Missing, malformed, unstable, or inaccessible evidence remains NO-GO; rounded console rows never substitute for raw files. The path mismatch is a supported diagnosis, not a proven cause until a separately authorized capture demonstrates host visibility. Do not change source, measurement tools, workloads, thresholds, time/resource limits, or attempt budget. This context update and any re-planning do not authorize another ACT profiling run, tests/probes, production edits, hosted CI, publication, or campaign restart; the existing stop remains until separately authorized.
- **D-26:** Create one successor, capture-only plan for a fresh local ACT attempt. The 28-11 `capture-1` root is spent and must never be reused; allocate a distinct new root (`capture-2`) and preserve the frozen 28-10 measured source commit `448055b5cc583c330543ac2893f419f7e3b0c6c1`, tree `d00f1a8425ff8de1db729140a92134c441c39b3b`, and 426-input manifest digest `e9044a7b696e171ed08d40b221bfa49449b72bfd745c92cb88203a076fd04b6e`. The capture helper and its test hashes are a separate tool identity; update and bind those without changing the measured workload identity. The Docker compiler/Cachegrind version preflight must explicitly request `linux/amd64`, matching the existing ACT `--container-architecture linux/amd64` setting. Keep the pinned image and tools, all 18 raw outputs plus summary, raw identity/hash/stability checks, integer +1% comparator, and existing time/resource limits unchanged. The plan performed its one new capture; the `capture-2` root is consumed and must not be retried or replaced with a fallback root. The capture verified all required artifacts and stable bases, but the unchanged raw +1% comparator failed for Nesteroids, Runner, and M4_P256K_C256K. ACT observed the runtime workspace and the expected artifacts were host-visible for this run; do not generalize that observation to other hosts. No capture runs during discussion or planning. This supersedes D-25 only for the completed capture. The capture does not consume or renew the remaining APU candidate attempt, authorize production edits, qualify a candidate, refresh PR #64, unblock Plan 28-06, authorize hosted CI/publication, or restart the stopped campaign. Keep PKG-01 and PROF-01 pending until their existing gates pass.

- **D-27:** Proceed to one feasibility-first successor plan for the already researched direct-index immutable APU event-table approach, using only the still-unused D-21 allowance. First freeze a clean source commit, tree, and full 426-input manifest from the reconciled active branch under D-22/D-23; do not transfer the 28-10 capture or older 1c674c1 cost estimates as candidate evidence. Before production edits, recalculate on that new exact source identity and account for generated instructions, guard/dispatch/rebuild/event cost, table footprint, and conservative savings against each of the three unchanged raw +1% workload gates. Require the complete D-20 NTSC/PAL behavior, lifecycle, IRQ/reset, and restored-state evidence before treating a candidate as qualified; keep `frame_cycle` authoritative and serialized, with derived per-instance state only. If the source-bound feasibility case cannot clear every workload with a credible margin, stop before product edits. If the single candidate later fails, stop without another attempt; keep PR #64 and Plan 28-06 blocked. This discussion authorizes planning only. It does not authorize tests/probes, production edits, another capture, hosted CI, candidate refresh, publication, or campaign restart.

- **D-28:** Keep D-27's planning-only authority in force. Plan 28-13 remains pending and unexecuted; this context refresh and replanning do not authorize its read/hash/document task, new performance evidence, tests, probes, compilation, profiling, captures, product edits, candidate refresh, hosted CI, publication, or campaign restart. The 28-12 capture record is intact, but all three measured workload rows fail the unchanged raw +1% gate. Plan 28-13 has no admissible current-source raw instruction counts or dynamic cost terms; its static review can only record NO-GO and cannot establish a performance GO. Preserve both spent capture roots, the unused D-21 candidate allowance, blocked PR #64 and Plan 28-06, and pending PKG-01/PROF-01. Revisit the boundary only through a separate, explicit evidence-authority decision. No new dependency or alternate optimizer is approved.
- **D-29:** Keep the D-28 authority stop after the verified 28-12 capture failed all three unchanged raw +1% workload gates. Leave Plan 28-13 pending and unexecuted; do not authorize its read/hash/document task or a new performance-evidence path through this discussion. The remaining D-21 candidate allowance stays unused and both capture roots stay spent. A documentation-only NO-GO record could improve audit detail, but cannot satisfy PKG-01 or PROF-01 or unblock Plan 28-06/PR #64. Revisit only through a separate, explicit evidence-authority decision that identifies an admissible source-bound path to the requirements. This discussion authorizes the requested re-planning step only; it does not authorize execution or evidence collection. Recommendation applied under the owner's instruction to synthesize specialist views and follow the recommendation.
- **D-30:** Approve re-planning one source-bound recovery route under the unused D-21 allowance. The plan may allocate two distinct fresh roots: `capture-3` for a baseline on the frozen, reconciled current source and `capture-4` for the single candidate comparison. Preserve `capture-1` and `capture-2` as spent; do not reuse either or add fallback roots. Before any baseline capture, freeze the active source commit and tree, the complete 426-input manifest, and DABG rights evidence. Preserve all existing dirty files; do not clean, reset, or stash the checkout. If a dirty file overlaps the measured inputs, stop and resolve its ownership before freezing the identity. Update and synthetically verify the capture helper for these explicit roots without running a live capture. Use the existing pinned tools, workloads, raw per-workload +1% gate, and all existing time/resource limits. Before production edits, calculate conservative cost for the sole D-21 direct-index immutable APU event-table candidate against each of the three workloads, including generated code, footprint, guards, dispatch, rebuild, and event costs. If any workload lacks a credible path through the unchanged gate, stop before production edits. If every row has a credible pre-edit GO, allow at most one candidate and compare it using `capture-4`; any failed, incomplete, unstable, or incompatible result is terminal, with no retry or alternate optimizer. Keep every D-20 compatibility and behavior constraint. This decision authorizes planning only. No tests, builds, probes, measurements, profiling, captures, code changes, hosted CI, PR update, publication, or campaign restart may begin until execution is separately authorized. Plan 28-13 remains pending and unexecuted; planning may revise or supersede it, but its documentation-only NO-GO task is not authorized to run. D-29's stop is superseded only to permit planning this bounded route; its execution and release stops remain.
- **D-31:** Preserve the accepted 426-path measured-input manifest. Exclude exactly `scripts/phase28_act_capture.py` and `tests/perf/test_phase28_act_capture.py` from `pr_local_manifest`; bind their content hashes separately as capture-tool identity. The current broad `scripts/` and `tests/` selection includes both files and yields 428 paths, so revise that selector and its independent checks as part of the 28-13 plan update before freezing source. Keep the selector implementation itself in the measured inputs; its changed bytes require a newly computed manifest digest at freeze, while the accepted manifest remains 426 paths. Keep all current dirty work and existing partial-plan commits. This decision resolves tool-vs-measurement ownership for planning and does not itself authorize source freeze, capture-3, or any additional test/build/probe; those actions remain governed by the authorized 28-13 plan, its gates, and limits.

### Continuation after the 28-13 halt
- **D-32:** Preserve the manual halted close-out of Plan 28-13 and its summary as historical truth. Do not resume or relabel it as successful: revised Task 1 remains unverified, the earlier 27-test result predates the current helper/test bytes, Task 2 did not start, and no source/rights freeze or capture-3 occurred. Continue the D-30 route only through a fresh, separately numbered successor plan for the unfinished pre-edit work. Update 28-14/15 so they depend on that successor's independently checked result rather than success from halted 28-13. Reconcile plan index, ROADMAP, and STATE in planning. Before execution, verify the recorded D-30 grant against the successor's exact identifier, actions, and limits; a new plan number does not renew authority. If coverage is unclear or insufficient, stop before task work. Keep capture-1/2 spent, capture-3/4 unused, the D-21 candidate allowance unused, PR #64 and 28-06 blocked, publication pending, and the stopped campaign untouched. Discussion and planning spend no evidence budget and authorize no execution.

### Dependency preference
- **D-33:** Prefer existing tools, the standard library, or a small owned helper over adding a dependency. This continuation needs no new runtime/tooling package or recurring CI lane; add one only if a demonstrated requirement cannot be met simply with what the project already owns.

### Recovery after the 28-16 halt
- **D-34:** Preserve Plan 28-16's failed one-shot stage and rejected baseline record as historical evidence. Continue only by planning a fresh, separately numbered successor for the unfinished pre-edit route. Before another source freeze, that plan must repair and verify the real producer-to-consumer path: emit `role: baseline`; request actual CMake build targets rather than CTest case names; exercise real freeze, build, feasibility, qualification, and capture-admission paths instead of replacing producers/checkers with stubs; independently recompute feasibility from source-bound evidence; and bind capture admission to feasibility GO, local qualification, the single-candidate ledger, and the approved delta. Do not retry Plan 28-16's failed stage or reuse its rejected freeze. Preserve the 426 measured inputs with helper/test hashes bound separately, DABG rights, D-20 behavior, the unchanged three-workload +1% gate, existing resource limits, capture-1/2 as spent, capture-3/4 as unused, and the sole D-21 candidate allowance as unused. Reconcile the plan index, ROADMAP, STATE, and 28-14/15 dependencies in planning. Planning authorizes no successor execution or capture; before execution, compare the recorded grant against the new plan's exact identifier, actions, roots, and limits, and stop if coverage is unclear or insufficient.

### Reviewed Todos
- Defer `2026-10-26-add-reproducible-owned-property-tests.md`: no demonstrated need in this package/profile/gameplay handoff; preserve its existing record.
- Defer `2026-10-26-make-runner-signal-checks-deterministic.md`: no demonstrated need in this phase; preserve its existing record.

### the agent's Discretion
- Preserve the already source-qualified DABG timings, progress checks, and thresholds in `docs/nrom-gameplay.md`. If scenario inputs or predicates change, explain why and requalify the affected evidence.
- Reuse the existing relocated installed-package consumer and boot-to-play patterns; do not broaden consumer scope unless a diagnosed failure requires it.
- Do not reopen the superseded Nesteroids route without new source-to-ROM evidence. If DABG rights/notice provenance cannot be completed, keep publication blocked and report the missing evidence.
- Add a public API change only if the current consumer profile or test exposes a demonstrated gap. Prefer a small owned helper or justified duplication to another dependency.
- Choose the exact transient representation and test placement for the one permitted follow-up, provided D-20/D-21 remain satisfied. Keep derived state per-instance; do not add a dependency. If no alternative has a conservative cost case that can meet all three unchanged rows, stop before product edits.

## Deferred Ideas

- PAL-through-LibFuturo, consumer host playback, Playstead acceptance, physical-device pacing, direct hardware qualification, machine snapshots, and broad mapper/title compatibility are not established by this phase.
- Keep the interrupted CI/AccuracyCoin campaign stopped and unknown.
- Do not expand the APU work beyond the one additional, behavior-preserving frame-counter attempt in D-21. Semantic timing or serialized-state changes require separate accuracy scope.

### Reviewed Todos (not folded)
- `2026-10-26-add-reproducible-owned-property-tests.md` — reviewed and deferred because this phase has no demonstrated property-testing need.
- `2026-10-26-make-runner-signal-checks-deterministic.md` — reviewed and deferred because this phase has no demonstrated need for a broader runner signal change.
- `2026-10-26-evaluate-scoped-nightly-mutation-testing.md` — reviewed and deferred; its weak keyword match is unrelated to this release blocker and does not justify a new testing tool or CI lane in Phase 28.
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|-----------------|
| PKG-01 | A consumer can identify and install a published Nesturbator package built from the exact accepted current source, with consistent package/header version, ABI, behavior revision and state-format identity; the release includes the library, runner and libretro artifacts with verified checksums. | The pre-edit successor supplies no package evidence by itself. Its all-workload result gates later candidate qualification and final publication evidence; it must not mark PKG-01 complete. |
| INST-01 | A clean C consumer can configure, build and run against the installed library package after the install prefix is relocated, without resolving headers or libraries from the source or build tree; it can load a valid permitted ROM, advance frames with input, obtain video/audio, and verify that a refused replacement load preserves the active game. | Already complete. Recovery plan should not reopen or re-run this consumer scope. |
| PROF-01 | A version-bound first-route profile identifies the exact Nesturbator package/source and accurately describes its selected NTSC content and region limits, load/refusal behavior, controller sampling, video representation, audio format/cadence/buffer bounds and lifecycle; it distinguishes battery-backed game memory from machine snapshots and marks every capability without current consumer proof as unqualified. | Still pending. A pre-edit measurement/tooling plan cannot complete the final package-bound profile; retain it as a downstream release requirement. |
| GAME-01 | One ROM already permitted by the repository manifest has a bounded, reproducible boot-to-gameplay input scenario that runs through the relocated installed package; a game-specific progress predicate demonstrates an interactive transition with visible response and non-silent audio. A title screen, input activity, audio alone or frame hash alone does not satisfy the requirement. DABG is the selected, source-qualified NROM scenario. | Already complete. Recovery plan should preserve its identity and not claim new gameplay qualification. |
</phase_requirements>

## Summary

Plan 28-16 did not produce baseline capture data or feasibility evidence. Its frozen record lacks the required baseline role, the one-shot baseline-cost ledger records an invalid build target, no feasibility producer exists, and the feasibility check trusts a declared status. The source-bound capture helper still maps the baseline role to Plan 28-16. These are contract defects in the real producer-to-consumer path, so prior synthetic pass results do not establish that freeze, build, feasibility, qualification, or capture admission works end to end. [VERIFIED: .planning/phases/28-qualified-nes-gameplay-package/28-16-SUMMARY.md:14-32; build/phase28-d30/run.py:25-26,52-66,436-470,515-524,551-577; scripts/phase28_act_capture.py:113-124,291-314]

**Primary recommendation:** Plan a fresh numbered pre-edit successor, expected to be 28-17. Preserve 28-13 and 28-16 as halted evidence. Before any new freeze, fix the role/plan/source mappings, use real CMake targets, implement evidence-producing and independently checking feasibility paths, and exercise the actual driver/helper code paths in temporary synthetic fixtures while stubbing only external process boundaries. Later live work remains gated by exact grant matching and its one-shot roots. [VERIFIED: .planning/phases/28-qualified-nes-gameplay-package/28-CONTEXT.md:74-75; 28-13-SUMMARY.md:34-53; 28-16-SUMMARY.md:20-34]

This successor owns pre-edit tooling and feasibility only. It cannot complete the single D-21 candidate, release, profile, or gameplay requirements. A checked NO-GO or incomplete evidence stops before production edits and leaves capture-4 and the candidate allowance unused. Planning does not authorize source freeze, captures, production edits, hosted CI, PR updates, release, or campaign work. The exact existing grant must be checked against the new plan identifier, actions, roots, and unchanged limits before any execution side effect; renumbering grants no authority. [VERIFIED: .planning/phases/28-qualified-nes-gameplay-package/28-CONTEXT.md:74-75; .planning/STATE.md:17-37]

## Architectural Responsibility Map

| Capability | Primary tier | Secondary tier | Rationale |
|------------|--------------|----------------|-----------|
| 426-input source and DABG rights freeze | Owned Python/CMake tooling over committed Git blobs | Phase evidence JSON | Existing freeze reads the receipt selector, Git tree, 28-09 reconciliation, prior baseline, DABG rights, and exact dirty-input classification. |
| Baseline build and focused behavior checks | CMake targets and CTest cases in an isolated frozen-source tree | Standard-library driver | Keep executable target names separate from test names; the failed driver passed a CTest name to cmake --build. |
| Workload cost and feasibility result | Source-bound Python evidence producer and independent verifier | Existing integer Cachegrind comparator | Feasibility must be calculated and bound to exact frozen inputs, raw measurements, and conservative replacement-cost terms; a status field alone is not evidence. |
| Baseline and candidate capture admission | scripts/phase28_act_capture.py with closed source-role/root mappings | Driver-produced authorization and route ledgers | The helper enforces one-shot root ownership. The new baseline plan number and its artifact path must agree across driver, helper, tests, and plan. Candidate admission must be downstream of the checked GO, exact local qualification, candidate ledger, and approved delta. |
| Conditional APU implementation | Existing Plan 28-14 after successful pre-edit GO | Plan 28-15 final capture and identity | The recovery successor must stop before edits; only Plan 28-14 owns the sole conditional candidate, and Plan 28-15 owns capture-4/final profile evidence. |

## Standard Stack

| Tool | Role | Recommendation |
|------|------|----------------|
| Python standard library | Existing capture helper and one-shot driver | Extend the owned scripts and synthetic tests; add no package. |
| CMake and CTest | Frozen-source configure/build and named behavior checks | Keep a target list for cmake --build and a separate regular-expression selection for ctest. The repository requires CMake 3.25 in tests/cmake/provider_receipt.cmake:1-2. |
| Git | Commit/tree/blob identity and frozen source archive | Continue binding source commit, tree, all 426 measured paths, and per-file blob hashes. |
| ACT, Docker, GCC, Cachegrind | Existing bounded local performance capture route | Use only the pinned route and only after all gates. Availability was not probed because the current scope forbids probes and capture work. |
| External packages or recurring CI | None needed for the corrected one-off local tooling | Do not add either; D-33 says existing tools, the standard library, or a small owned helper are sufficient absent a demonstrated gap. |

No package installation or version lookup is part of this recovery scope.

## Architecture Patterns

### Source-bound route

1. Preserve the accepted 28-09 source/rights lineage and review all dirty measured inputs without cleaning, resetting, or stashing the workspace.
2. Confirm the live selector yields exactly the accepted 426 paths; exclude only the capture helper and its test from those measured paths and bind both hashes separately. Keep the selector itself among the measured files.
3. Freeze a clean commit/tree, manifest digest, source deltas, and DABG rights identity. Require a record whose role is baseline and whose schema/path/plan identity agrees across driver, helper, tests, and plan.
4. Build the frozen source with real CMake targets. Run the independently selected CTest cases after building their targets.
5. Produce the baseline cost/evidence record from the actual driver path. Independently recompute the conservative all-workload feasibility decision from exact-source records and raw comparator inputs.
6. Gate the one baseline capture by its baseline-specific source identity, role, authorized root, and exact grant. Treat baseline capture's expected comparator failure as input to feasibility, not as candidate acceptance.
7. If and only if checked feasibility is GO, allow the later candidate plan to act. Candidate capture admission must bind its exact candidate source, local qualification, consumed single-candidate ledger, approved delta, and capture-4 root. Any missing/mismatched evidence is terminal.

### Role-specific admission sequence

The timeline makes these admission rules role-specific. Capture-3 is the pre-edit baseline and cannot require a candidate local qualification or candidate ledger that do not exist yet. It must require the accepted baseline source record, the baseline role, the matching plan/root, the existing grant, and the current source/tree binding. Capture-4 is the later candidate comparison and must require the checked all-workload GO, the exact locally qualified candidate, the one-candidate ledger, approved delta, and its own matched authority/root. This split preserves D-30's order while satisfying D-34's evidence-binding requirement.

### Real-path synthetic contract checks

The current driver exposes test adapters that can replace the producer and checker, and its contract oracle calls the synthetic adapters. Keep the actual dispatcher, producer functions, and verifier functions under test using temporary roots/records; isolate only external commands such as git/CMake/ACT at the subprocess boundary. Assert produced bytes, source bindings, independent recomputation, refusal-before-ledger behavior, and no live external call. A synthetic test result proves tool behavior only; it is never raw performance, qualification, or release evidence. [VERIFIED: build/phase28-d30/run.py:586-621,661-724; .planning/phases/28-qualified-nes-gameplay-package/28-16-SUMMARY.md:14-18]

## Don't Hand-Roll

| Problem | Avoid | Use instead | Why |
|---------|-------|-------------|-----|
| Measured input selection | New glob or copied path list | Existing pr_local_manifest selector and 28-09/28-10 accepted manifests | The existing 426-path set is an explicit qualification contract. |
| Rights qualification | Reconstructed license statement | Existing 28-09 evidence and pinned notice checks | The accepted Zlib terms and exact notice digest are source-bound. |
| Performance comparison | Rounded percentages or a replacement comparator | tests/cmake/perf_compare.cmake integer calculation and the same three workloads | The gate is per-workload and uses raw integer counts. |
| Freeze and capture records | Hand-authored success/status fixtures used as actual output | Real producers plus independent checks over their output in temporary tests | A claimed status is not evidence and was the failure mode in 28-16. |
| Tool dependencies/CI | New package or recurring job | Python standard library, CMake/CTest, Git and the bounded existing local route | No recurring detection or capability gap justifies added cost. |

## Common Pitfalls

1. **Reusing the halted Plan 28-16 freeze or stage.** The baseline record is rejected and the baseline-cost ledger is failed one-shot. Do not rerun that ledger, edit its frozen tool set, or use its source identity for another capture. [VERIFIED: 28-16-SUMMARY.md:17-24; 28-16-RESUME.md:3-7]
2. **Repeating the CMake target/test-name mix-up.** The CTest case is core.state_reject.regional while the executable target is core.state_reject. The target helper creates an executable target by name and registers a test separately. [VERIFIED: tests/CMakeLists.txt:1014-1021,1160-1168]
3. **Treating the driver oracle as evidence of real route correctness.** Synthetic adapters can replace the real producer/checker, the production driver has no feasibility producer, and its feasibility checker accepts terminal status values then returns the record. [VERIFIED: build/phase28-d30/run.py:52-66,515-524,551-556,586-621,661-724]
4. **Relying on an old hard-coded plan identity.** The helper currently declares the 28-16 baseline record and expects baseline plan 28-16; the new baseline successor requires consistent plan, filename, root, source role, and downstream dependencies. Candidate plan 28-15 remains distinct. [VERIFIED: scripts/phase28_act_capture.py:113-124,291-314,363-376]
5. **Confusing planning scope with capture authority.** No execution authority is renewed by D-34. Missing/ambiguous grant match means stop before a task side effect and preserve all roots and candidate allowance. [VERIFIED: 28-CONTEXT.md:74-75; AGENTS.md:126-155]
6. **Assuming a baseline comparator failure is a candidate NO-GO or GO.** Baseline raw counts are model inputs. The candidate can proceed only when conservative source-bound upper-cost estimates credibly clear each independent unchanged comparator row.
7. **Treating prior passing tests as current proof.** 28-13 had partial work and no current Task 1 verification; 28-16's 29 tests and synthetic driver check did not validate actual freeze/build/feasibility producers. [VERIFIED: 28-13-SUMMARY.md:40-47; 28-16-SUMMARY.md:14-22]

## Code Examples

The current implementation pins the closed D-30 baseline/candidate roots and a baseline source-record name:

> D30_CAPTURE_ROOTS = {
>     "baseline": CAPTURE_RELATIVE / "capture-3",
>     "candidate": CAPTURE_RELATIVE / "capture-4",
> }
> D30_BASELINE_RECORD = f"{PHASE_DIR}/28-16-BASELINE.json"

This exact mapping is at scripts/phase28_act_capture.py:113-118. The successor should centralize its new plan identifier and use the same canonical record path in helper, driver, fixtures, and plan.

The helper also currently selects the baseline plan from role:

> expected_plan = "28-16" if role == "baseline" else "28-15"

[VERIFIED: scripts/phase28_act_capture.py:291-305]

The baseline driver uses a CTest name in the CMake build target list:

> targets = ["nesturbator", "core.apu", "core.state", "core.state_reject.regional", ...]

Then it separately passes that same case name to CTest's -R selector. The build list should use the executable target core.state_reject; the regional test case remains core.state_reject.regional. [VERIFIED: build/phase28-d30/run.py:466-470; tests/CMakeLists.txt:1160-1168]

The current driver outputs only these producer records:

> "freeze": ".../28-16-BASELINE.json",
> "baseline-cost": "build/phase28-d30/baseline-cost.json",
> "implementation": "build/phase28-d30/candidate-source.json",
> "qualify-local": "build/phase28-d30/local-qualification.json",
> "qualify-final": "build/phase28-d30/final-qualification.json",

The mapping is build/phase28-d30/run.py:52-58. It has no feasibility output producer. Its check accepts a status from the values GO, NO-GO, or halted and returns the object without recomputation at lines 515-524.

The existing performance gate computes an integer delta and fails when the head exceeds base by more than one percent: 100 times the head-minus-base delta must not exceed the base count. [VERIFIED: tests/cmake/perf_compare.cmake:236-272]

Use this unchanged comparator and keep all three workload rows independent.

## State of the Art

| Earlier route | Current state | Planning consequence |
|---------------|---------------|----------------------|
| Plan 28-13 D-30 continuation | Halted with revised Task 1 still incomplete; manual summary is authoritative | Keep its commits and dirty partial work; do not resume or relabel. |
| Plan 28-16 D-30 successor | Halted after a rejected freeze record and failed one-shot baseline-cost stage | Treat its record, ledger and stage outcomes as immutable history. |
| 28-16 synthetic contract pass | Valid only for exercised synthetic helper/driver behaviors | Do not infer that actual freeze/build/feasibility/qualification/admission producers pass. |
| Plans 28-14/15 dependencies | Still point at halted 28-16 | Rewire both to the checked 28-17 result; retain 28-14 candidate and 28-15 capture scopes/gates. |
| ROADMAP readiness note | Calls 28-16 potentially runnable even though its summary says halted | Reconcile ROADMAP, STATE, and phase-plan-index together; current index has no ready plan. |
| Capture roots and candidate | Capture-1/2 spent; capture-3/4 and the single candidate allowance unused | Keep their exact one-shot status; no reset, fallback root, or extra candidate. |

## Project Constraints (from AGENTS.md)

- Use only owned code and tests; libretro.h is the only vendored file. The library links only C memory functions.
- Preserve the three deliverables: C library, headless runner, and libretro adapter. Do not add window, audio-device, or input-device code.
- Keep ROM/BIOS bytes limited to manifest-listed test fixtures; do not include personal paths, email addresses, or names in committed artifacts.
- Follow the clean-room rule: use hardware documents and test behavior; do not inspect or retain GPL/LGPL emulator source.
- Preserve deterministic integer-only, per-instance emulation state and current compatibility identities. D-20 specifically prohibits changing guest-visible APU timing, ABI 1, behavior revision 17, state format 6, or public API for this optimization.
- For any behavior change, add its test and update README and public-header comments in the same change. This pre-edit tooling successor makes no product behavior change and should not take ownership of those candidate tasks.
- Use one GSD step per command, keep work on the current phase branch, and compare phase-plan-index readiness with STATE/ROADMAP before later execution dispatch. Do not use --wave to mask stale plan records.
- Preserve unrelated dirty work and partial commits. Do not clean, reset, or stash the checkout. A freeze that intersects unresolved dirty measured paths must stop.
- Prefer existing tools, the standard library, or a small owned helper; no new dependency or recurring CI lane without demonstrated need.
- Keep execution limits explicit: at most four build workers, serial measurement, existing per-test/fixture caps, the 900-second ACT/perf envelope, 1500-second MODE=local envelope, and the 30-minute outer workflow bound. Preserve one-shot and no-retry stops. Current research/planning has not exercised or consumed any measurement root.
- Planning is not execution authority. Before an execution side effect, compare the exact durable grant to the new plan's identifier, actions, roots, budgets, limits, and no-retry stops; if coverage is unclear or insufficient, stop without consuming resources.

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | The fresh successor is numbered 28-17 and owns the pre-edit route only. | Summary | If the phase plan index contains a different next number at final planning, the planner must reconcile it before writing files. |
| A2 | The matching durable D-30 grant may or may not cover the successor's identifier. | Open Questions | Assuming coverage could cause unauthorized side effects; execution preflight must prove exact coverage. |

## Open Questions

1. **[RESOLVED — EXECUTION GATE] Does the durable D-30 grant cover Plan 28-17, its exact actions, source roots, and preserved limits?**
   - Known: the 28-16 plan had a retained authority match. The plan number and scope are explicitly bound in local helper/driver records.
   - Unknown: this research did not inspect the ignored grant record and cannot establish that its exact plan hash or route fields are valid for 28-17.
   - Recommendation: do not inspect or infer a new authority match during planning. At execution preflight, compare the durable instruction to 28-17's exact identity/actions/roots/limits; if mismatched or unclear, stop before task work and leave every budget untouched.
   - Planning resolution: 28-17 Task 1's precondition and first action require the exact durable-grant match before any side effect, with the same match reasserted by Task 2 and downstream tasks. The factual coverage outcome remains unknown until separately authorized execution; no grant match or renewed authority is claimed here.

2. **[RESOLVED — EXECUTION GATE] Are there dirty paths in the 426-input selector that the future freeze would include?**
   - Known: driver enumerates selected dirty measured paths and aborts on any overlap. D-31 permits only the capture helper and its synthetic test to be separately hashed.
   - Unknown: exact selected-path ownership must be determined from the final successor source during authorized execution.
   - Recommendation: preserve all dirty work. Resolve ownership for every selected path before freeze; never silently exclude, reset, or stash it.
   - Planning resolution: 28-17 Task 2 requires final-selector dirty-input enumeration and resolved ownership before its one freeze, and check-baseline independently validates all 426 Git-blob rows. The factual overlap/ownership outcome remains unknown until separately authorized execution; planning claims no clean measured-input set.

3. **[RESOLVED — EXECUTION GATE] Can a current-source conservative cost model clear all three workloads?**
   - Known: the prior capture-2 raw comparator failed every workload, while the current-source 28-16 route produced no capture-3 raw data or cost result.
   - Unknown: there is no new source-bound raw baseline or independently verified feasibility model.
   - Recommendation: require source-bound raw counts and conservative generated-code, footprint, guard, dispatch, rebuild, event, and uncertainty costs. Any missing or failing workload row is terminal before production edits.
   - Planning resolution: 28-17 Task 2's source-bound feasibility producer and independent check-feasibility must recompute every workload and yield checked all-workload GO before 28-14 becomes eligible; missing, unsupported or failing evidence terminally stops before production edits. The factual feasibility outcome remains unknown until separately authorized execution; no current GO or performance pass is claimed here.

## Environment Availability

This phase continuation depends on the existing CMake/CTest and local ACT/Cachegrind route, but live availability was not checked. Running version or capability probes is outside the current planning-only scope. Record availability at authorized execution preflight using existing pinned tools; do not install packages or pull images as fallback.

| Dependency | Use | Availability |
|------------|-----|--------------|
| Python 3 standard library | Owned capture helper and driver | Not probed |
| CMake, compiler, CTest | Isolated frozen-source configure/build/focused checks | Not probed |
| ACT, Docker, GCC, Cachegrind | One bounded local capture after all gates | Not probed; no install, image pull, or replacement tool allowed |

## Security Domain

security_enforcement is explicitly false in .planning/config.json:49. The continuation is local evidence tooling, not a security-enforcement phase; omit ASVS mapping. Keep closed path/root checks, source-record hash binding, exclusive ledgers, and grant evidence fail-closed as locked correctness controls.

## Sources

### Primary project sources (HIGH confidence)
- .planning/phases/28-qualified-nes-gameplay-package/28-CONTEXT.md — latest D-30 through D-34 decisions and scope constraints.
- .planning/phases/28-qualified-nes-gameplay-package/28-13-SUMMARY.md and 28-16-SUMMARY.md — authoritative halted histories and actual failure causes.
- .planning/phases/28-qualified-nes-gameplay-package/28-16-RESUME.md — no-retry and no-reuse restrictions.
- .planning/STATE.md and .planning/ROADMAP.md — approved next action, plan-index/readiness reconciliation, and stale dependency descriptions.
- .planning/REQUIREMENTS.md — current status and exact phase requirements.
- scripts/phase28_act_capture.py — capture roots, source-role mapping, authority bindings, and current Plan 28-16 hard-coded baseline identifier.
- build/phase28-d30/run.py — current producer set, one-shot stage ledgers, invalid target list, status-only checks, and synthetic adapter contract.
- tests/CMakeLists.txt — executable target creation and regional CTest case name.
- tests/cmake/provider_receipt.cmake — accepted receipt selector and rights checks.
- tests/cmake/perf_compare.cmake and tests/perf/allow.txt — integer comparator and unchanged workload threshold contract.
- .planning/config.json — Nyquist and security workflow configuration.
- AGENTS.md — clean-room, determinism, workflow, dependency, limits, and preservation directives.

### External sources
Not needed. Findings concern current repository contracts and no external library or platform behavior changed.

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — owned Python/CMake/Git paths are present; no new package is needed.
- Architecture: HIGH for present producer/checker gaps and halted outcomes; MEDIUM for the new successor's future feasibility.
- Pitfalls: HIGH — directly verified in the rejected record, stage command, driver checks, and CMake test/target definitions.

**Research date:** 2026-11-08
**Valid until:** Until authority scope, source/rights inputs, evidence contract, or Phase 28 plan-index state changes.

## Source/evidence-binding follow-up — 2026-11-10

Current discussion research is [28-EVIDENCE-MAINTENANCE-REVIEW.md](28-EVIDENCE-MAINTENANCE-REVIEW.md),
with independent read-only results in [28-EVIDENCE-BINDING-CHECK.json](28-EVIDENCE-BINDING-CHECK.json).
Three relevant specialist reviews and a bounded adversarial pass recommend
preserving historical campaign metadata/results while separately validating
current inputs. A future changed checker is itself fingerprinted and needs
independent controls; do not overwrite history or hide CMake registrations.
The full configured activation fixture can launch a campaign, so preflight
must inspect fixture closure and use a separate synthetic/non-full control seam.
No acceptance run, checker/source change, candidate or new grant resulted.
The detailed prospective scope awaits the owner's decision; D-42 remains terminal.
