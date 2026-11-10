# Phase 28 evidence-maintenance decision — 2026-11-10

**Status: owner approved the scoped remedy and fresh route on 2026-11-10; original proposal below retained.**
This is a review and concrete proposal, not an executable plan or new grant.
D-42 and its failed ledger remain terminal. No source, checker, baseline,
acceptance run, candidate, remote operation or publication changed in this step.

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

## Recommendation

Keep the historical AccuracyCoin baseline byte-for-byte intact. Add a narrow,
versioned binding that distinguishes the historical campaign's inputs from a
future candidate's inputs, and independently test that distinction. Then qualify
one separately authorized successor from the already completed package/profile
source. Do not replace the historical hash, hide registrations outside the input
closure, generalize the closure policy, or use focused checks as release proof.

Three bounded, read-only GPT-6 Luna reviews covered evidence/build identity,
CI/release operations, and provenance/product boundaries. A second adversarial
pass challenged the synthesis. The settled technical recommendation is dual
identity; implementation and a new allowance remain an owner decision.

## What the read-only check establishes

[Independent check](28-EVIDENCE-BINDING-CHECK.json) reconstructs the fingerprint
from Git blobs without invoking any project checker, test, build or campaign.
It reproduces both recorded 56-path fingerprints and checks all six retained
failure-artifact digests. The only changed protected path between approved base
`b25146c6efaac41c58f60555f59f0fe79a90ca68` and failed frozen source
`2e0f2057304f60bab9a06a085a3e9477f1da022b` is `tests/CMakeLists.txt`.
Its diff adds `install.gameplay`, `provider.profile`,
`provider.profile.controls` and `provider.receipt.controls` with their commands
and properties. No production/build/workflow, baseline, or AccuracyCoin-checker
bytes changed. The original [failed identity](28-RESET-IDENTITY.json) and
[failure report](28-RESET-FAILURE.md) remain unchanged.

The historical metadata records `source_revision=d4802809ff41`, `source_dirty=1`,
its original 56-input hash, 146 scored rows (119 PASS, 27 FAIL), 5 DRAW rows,
ROM pin, report digest and dirty-source explanation. Those declarations remain
historical. A matching recorded digest establishes integrity relative to the
retained record; it does not independently authenticate the old dirty checkout
or establish hardware correctness. The failed full workflow is still failed.

## Options and tradeoffs

| Option | Practical benefit | Cost or failure mode | Disposition |
|---|---|---|---|
| Historical baseline plus narrow separate current binding | Keeps old measurements truthful while making the new candidate's inputs explicit | Requires a small verifier contract change and adversarial controls | Recommended, subject to approval below |
| Replace `build_inputs_sha256` in the historical metadata | Smallest textual edit | Conflates historical measurement inputs with a new candidate unless a broader baseline version/provenance migration is designed | Rejected for this narrow route |
| Move registrations into an unhashed include | Could recover the old digest | Hides executed test configuration from the protected closure; moving through a newly hashed include still changes the parent hash | Rejected |
| Redesign which CMake/dependency inputs are protected | Could solve a general evidence-policy problem | Broader dependency-closure proof and migration; unnecessary for four exact additions | Deferred |
| Leave acceptance stopped | Consumes no new allowance | Provider handoff remains blocked; MMC2 also has an independent stopped full-CI gate | Valid alternative if approval is declined |

CMake `include()` executes code with access to the caller's variables, so a file
move alone does not prove independence. This is a documented CMake fact; the
conclusion about this repository's closure is our source-based inference.
[Official CMake 3.25 include documentation](https://cmake.org/cmake/help/v3.25/command/include.html).

## Bounded source contract proposed for approval

Start from exact failed product source
`2e0f2057304f60bab9a06a085a3e9477f1da022b`, not the dirty MMC2 checkout, later
disposition commits, or PR 64. Reuse its completed NTSC consumer/profile/DABG
work. Permit changes only in these concerns:

- `tests/cmake/accuracycoin_activation.cmake` and
  `tests/cmake/accuracycoin_input_fingerprint_controls.cmake`: distinguish the
  preserved historical result identity from reviewed current inputs and reject
  every unsupported delta. If shared validation is needed, put the minimum
  helper in the already fingerprinted
  `tests/cmake/accuracycoin_input_fingerprint.cmake`; preserve its input selection,
  normalization and hashing algorithm.
- One versioned binding record under `tests/accuracy/`, recording the immutable
  historical identity/digests and exact registration delta. It is evidence data,
  not permission to accept whatever hash a caller supplies. Keep
  `tests/accuracy/baseline-metadata.txt`, scoreboards, migration, ROMs, campaign
  collector, result rows and thresholds byte-identical.
- Minimal successor route/scope/root binding and synthetic controls in
  `tests/cmake/provider_receipt.cmake`, plus an ignored orchestration driver.
  Its existing reset modes hard-code the old base and cannot simply consume
  another ledger. Keep the old route's rejection behavior intact.
- README/testing/release explanation and workstream planning/evidence records.
  No runtime/public-header behavior or compatibility identity changes are needed.

The checker must pin the old identity and exact old/new CMake registration
hashes/delta; the new record cannot nominate arbitrary changed paths or grant
its own exception. All other production, runner, ROM, outcome, fixture and build
inputs stay protected. Any changed verifier bytes require independent review
and mutation controls, rather than being declared behavior-equivalent by the
old CMake-only comparison.

Both activation and the fingerprint helper are themselves fingerprinted. Finish
checker code first; compute the actual new 56-input manifest afterward. Do not
embed the new full fingerprint in code that contributes to that fingerprint.
Record the final source/tree and input digest in the external candidate-bound
receipt at freeze. The full provider manifest must also cover the new binding
record and controls. It must be enumerated afresh; the old 434-path count is not
a successor oracle. Decline the route if the exact contract cannot be enforced
without a self-authorizing exception. This is a review trust boundary, not a
new cryptographic authentication claim.

## Proposed execution allowance and stops

If approved, plan fresh numbered successors; leave halted 28-22 and unstarted
28-23/24 as historical blocked records, explicitly superseding only their
future scheduling route. Reconcile the live index before execution.

Use fresh owned checkout `build/phase28-d43-candidate`, branch
`agent-phase28-d43-evidence`, and ignored output root `build/phase28-d43` with a
new exclusively created route ledger. Bind the checkout through the installed
manifest-recorded worktree adapter; do not fabricate isolation or merge into
the dirty orchestrator checkout. Preserve all D-42 roots, ledger, identities,
logs and transport bytes. If a proposed new root already exists, stop and
resolve ownership; do not overwrite it or silently choose another root.

Approve one successor candidate, pre-freeze bounded focused build/control work,
one local full workflow, one exact-candidate required CI run, and one resulting-
main required CI run. After their success, use only the normal required release-
PR/release-source checks, Release Please publication and published-artifact,
rights/notice and native-consumer receipt gates. Keep every required platform,
reproduction/hash assertion, +1% performance comparator and publication gate.

Local full workflow: four workers, existing 1500-second child limit and
1800-second outer limit; each full workflow remains within the standing
30-minute wall-clock limit and all stricter per-test/resource caps. The sole
new workflow includes its required fresh configured AccuracyCoin campaign and
protected-result fixtures. This is an explicit prospective allowance request,
not a restart of the historical interrupted collector or a retry of D-42. Do
not launch a separate campaign during preflight or afterward.

Any failed actual identity/comparator/acceptance/CI gate, interruption or timeout
stops the new route without retry, second candidate, refreeze, merge or
publication. Expected negative synthetic controls are not live acceptance
failures. No emulator, API, build-flag, workflow, performance/APU, capture,
PAL/Dendy, dependency-install, MMC2 retry or broader campaign authority is added.
LibFuturo still owns its installed consumer, native host and gameplay acceptance;
no Playstead or hardware qualification follows from Nesturbator's package.

## Verification approach and efficiency

Before freezing or consuming the full run, independently compare the complete
protected path manifests and immutable baseline/result/ROM bytes. Test exact
acceptance of the reviewed binding and rejection of missing/duplicate/unknown
fields, arbitrary hash/path exemptions, metadata/result/ROM mutations,
unreviewed CMake deltas, altered measurement inputs and malformed provenance.
Review the changed checker independently. Reuse the existing fingerprint,
activation and receipt control tests; no new CI lane or dependency is justified.

Inspect CTest discovery and fixture closure first. The live activation test
requires `accuracycoin_full_report` when full mode is enabled; selecting it can
launch `accuracycoin.campaign`. Keep pre-freeze verifier controls synthetic or
in a distinct non-full control configuration, with no live-campaign pass claim.
The full acceptance configuration must retain full mode and all fixture gates.
Then run the sole new full workflow through the receipt wrapper; do not run it
separately and invoke the wrapper a second time.

Recorded prior hosted CI had an approximately 11m17s critical path and a
10m50s macOS Intel build lane. These are historical observations, not a current
benchmark or billing estimate. D-42's 239.616-second interrupted failure is not
a successful workflow estimate. No coverage-preserving material CI change is
supported in this scope. Avoid duplicate full workflows and repeat source
reconciliation; do not add caches/concurrency/dependencies to this remedy.

## Owner decision and resume

**Approve the bounded dual-identity/checker maintenance and one fresh successor
route with the exact source, allowed changes, runs, limits and stops above?**
This is new evidence-contract and acceptance authority. The existing no-retry
and no-baseline-edit restriction does not permit it, and autonomous mode cannot
supply it. Declining leaves the current stop intact without any acknowledgement.

The follow-up discussion is handled. Re-entry presents this saved decision;
it must not repeat the reviewer fanout or dispatch execution. After approval,
record its exact scope in context, run `$gsd-plan-phase 28 --ws libfuturo-nes-gameplay-handoff`
(the discussion prerequisite is satisfied), then continue authorized work.

Entry command: `$gsd-autonomous --ws libfuturo-nes-gameplay-handoff --from 28`.
Context may be cleared; read selected STATE and 28-RESET-RESUME first. The owner
acts next on the approval question. Nothing will run automatically while it
is unanswered. These documents are saved inside the existing uncommitted
workstream migration; unrelated dirty files and Git staging remain untouched. The migration retains
legacy relative links in older history; verification checks the new current
handoff links without claiming to repair that unrelated migration.
