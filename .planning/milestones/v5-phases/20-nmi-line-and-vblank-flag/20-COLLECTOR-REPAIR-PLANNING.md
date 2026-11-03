# Phase 20 collector repair planning

Planning only, 2026-10-27. This diagnostic records retained local evidence and task assignments; it changes no collector, tests, workflow, remote state or achieved requirement. Completed 20-01 through 20-09 remain historical. Plan 20-10 has zero completed tasks, no task commits and no SUMMARY.

## Observed failure and scope

The existing delivery precondition passed: candidate `e6e83a31e413e12abe9dee142e3c0f7143d63eb4`, PR 59, tested merge `11533e456d41f740751c8353f947d65017aae692`, base `ab9c2e6eb822c06f0dfb0f08a0ca42ec541358be`, CI `40514060876/1` and Nightly `40513950000/1` were completed/success. Effective digest `de6d4355222063e019c7a3a94e30a404b52041f3d994659d445f12b4a23318ab` covers 415 files. Availability did not establish content acceptance.

Collection failed at `expected one artifact content matching **/LastTest.log, found 0`. No manifest or source-identity acceptance record was produced. The wrapper's pending_delivery verdict misclassified an acquired-content failure. The invocation needed a temporary scratch shim because installed gh rejects api --repo; that local invocation workaround is not a repair strategy.

Retained raw artifact `12737934863`, `accuracycoin-evidence-linux-x64-40514060876-1`, is 142169 bytes with ZIP SHA-256 `379b59e531393c154ee4ee2e0e4393fa2ca912492108eedad9c1e1c93e2a394c`. Its root has LastTest.log, full/* and protected-main/*, without a wrapper directory. The log is 121 bytes of start/end records, not terminal CTest rows. Captured workflow logs contain actual passing rows for each build matrix lane. The exact registered mapper test is `holymapperel.m2.decode`; the collector and fixture currently expect a nonexistent ROM-derived name.

| Measured/static edge | Planned resolution |
|---|---|
| Double-star matching misses root LastTest.log; nested/root duplicates could be ambiguous | 20-11 Task 1 component-boundary suffix enumeration, depth-zero support and exact uniqueness |
| Empty LastTest.log has no execution proof; fixture invents summary lines | 20-11 Task 1 raw preservation plus exact job/run/attempt/step-bound executed CTest logs |
| Wrong Holy Mapperel expectation | 20-11 Task 1 use existing exact m2 registration; near-match refusal control |
| Metadata/ZIP gh api --repo is unsupported; (?i) error regex is unsupported CMake | 20-11 Task 1 supported qualified GET endpoint arguments and lowercase diagnostic matching |
| Campaign normalization writes build-linux-x64/full while validator reads build-linux-x64/accuracycoin/full | 20-11 Task 1 canonical destination shared with validator |
| Perf and Nightly copy targets are not created; flattened names can overwrite ambiguity | 20-11 Task 1 explicit target directories and unique expected logical files |
| Failed recollection can retain prior manifest while verdict wraps every failure as pending_delivery | 20-11 Task 1 invalidate old success outputs and distinguish acquired-content failure |
| Synthetic validator-only controls bypass the broken adapter | 20-11 Task 1 production collect/validate subprocesses with owned cross-platform fake gh, real-shaped ZIP/log metadata, no production fixture bypass/network |
| Collector/helper/test/README changes invalidate the old full effective digest | 20-11 Task 2 measured local preparation; 20-12 fresh delivery/acquisition; 20-10 actual collection |
| 20-12's former rollup-only command did not verify its required job/artifact/source-mapping handoff | 20-11 Task 1 shared production MODE=availability plus offline metadata controls; 20-12 Task 2 runs that exact no-download metadata gate |

The bounded fake-gh helper uses existing C17/CMake only; no package install, new runtime/test dependency, bundled ROM or workflow modification is planned. It serves sanitized generated fixtures with realistic structure, not copied unbounded hosted archives/logs. Raw old ZIPs and provenance stay ignored scratch data. Existing hash/perf/campaign/host/Nightly controls remain active.

## Execution dependency and authorization

`20-09 (complete) -> 20-11 (wave 10 local repair) -> 20-12 (wave 11 authorized delivery/acquisition) -> 20-10 (wave 12 read-only content acceptance)`.

20-11 Task 1 creates a usable repaired adapter and MODE=availability backed by shared metadata/source validators with realistic offline controls; Task 2 measures and gates its final local source. 20-12 Task 1 publishes through the existing phase branch and draft PR 59, Task 2 acquires completed automatic PR CI/Nightly, writes fresh delivery.json and runs the metadata-only availability command against the captured candidate and expected PR 59. Its availability/availability.txt records exact successful jobs/matrix identities, fresh required artifact IDs, tested-merge/protected-base mapping and full effective-source equivalence with metadata digests, without downloading archives/logs or claiming content acceptance. This shared mode is explicitly future production work in 20-11, not a command already present at planning time. 20-10's existing two tasks retain their availability precondition and full collect/validate content gates, consume the new source-bound bundle and publish bounded content/measurement records. Every plan has two tasks; shared evidence prose is serialized and no dependency points back to 20-10. Existing plan numbers stay stable so executed history is not rewritten.

20-DELIVERY.md records the owner's persistent authorization to unblock delivery and normal evidence-driven repairs. It permits normal phase-branch push/update of existing draft PR 59 and its automatic PR CI/Nightly; no repeat authorization is required. No merge, force push, branch change, new PR, dispatch, unrelated CI efficiency proposal or additional lifecycle command is included. Missing external availability is reported factually, never converted into a manual test-sign-off or waiver. The planning command itself does not publish or start work.

## Multi-source coverage audit

COVERED means assigned, not achieved. NMI-01, NMI-02, KEEP-02, KEEP-03 and the phase remain pending independent verification after real 20-10 acceptance.

| Source | ID | Outcome/constraint | Assignment | Status |
|---|---|---|---|---|
| GOAL | Phase 20 | Console-correct $2002/$2000 VBlank/NMI behavior, six target rows, race history replay, retained PASS set | Historical 20-01..08; 20-09 preparation; 20-11 repair, 20-12 fresh runs, 20-10 hosted proof | COVERED |
| REQ | NMI-01 | Flag transition timing and CPU-visible reads/writes | 20-01/02/07; 20-11 Task 1 + 20-10 hosted verification | COVERED |
| REQ | NMI-02 | Named NMI behavior targets pass | 20-01/02/03/06; 20-11 Task 2, 20-12, 20-10 | COVERED |
| REQ | KEEP-02 | Causal native hash repins and six-platform equality | 20-02/03/06/08/09; unchanged gates in 20-11, fresh 20-12, real 20-10 | COVERED |
| REQ | KEEP-03 | Race history save/load/rewind/run-ahead and old-format refusal | 20-02..05/08; 20-11 preserves source/host gate, 20-12, 20-10 | COVERED |
| RESEARCH | R1 | Existing C17/owned helpers and local tier responsibility | Historical implementation; 20-11 Task 1 uses owned C/CMake and validators | COVERED |
| RESEARCH | R2 | Separate status samples and line/capture/poll state | 20-01/02/03/07; 20-11 preserves existing outcomes | COVERED |
| RESEARCH | R3 | Pinned complete alternatives/masks/actual end sequences | 20-01/03/06/09; 20-11/12/10 retain exact target gate | COVERED |
| RESEARCH | R4 | Six finite-margin oracle outcomes, internal phi2/M2 distinction, physical-tie limit | 20-02/03/06; retained in 20-11 Task 2 and 20-10 | COVERED |
| RESEARCH | R5 | Default alignment, independently justified single global phase | 20-02/03; repair does not change alignment | COVERED |
| RESEARCH | R6 | Exact set/clear and meaningful saved audio | 20-04/08; existing tests retained by 20-11 ci | COVERED |
| RESEARCH | R7 | Honest edge-before-poll feasibility and origin/endpoint labels | 20-04/05/08; 20-11 Task 2 and 20-10 retain qualifications | COVERED |
| RESEARCH | R8 | Visitor/validation, format 2/revision 8 and atomic format-1 refusal | 20-02/03/04/08; repair does not change state/API | COVERED |
| RESEARCH | R9 | Native/guest/host equality, wrong-load/history and instance controls | 20-04/05/08; retained ci and 20-10 real host | COVERED |
| RESEARCH | R10 | Actual campaign/hash/provenance, dirty local versus clean hosted | 20-11 Task 2, 20-12 fresh acquisition, 20-10 acceptance | COVERED |
| CONTEXT | D-01 | Complementary owned dot, CPU bus and pinned integration evidence | 20-01/06/07; 20-11 Task 1/2, 20-10 | COVERED |
| CONTEXT | D-02 | Source-derived expectations and named coordinates | 20-01/02/07/08 retained, no oracle rewrite | COVERED |
| CONTEXT | D-03 | Both race boundaries, mirrors and ordinary controls | 20-01/02/07 retained by full ci | COVERED |
| CONTEXT | D-04 | Early bit 7, late sprite bits, one read and side effects | 20-01/02/07 retained | COVERED |
| CONTEXT | D-05 | Enable/re-enable, extra NMI and STA/INC Control 8/9 | 20-01/02/03; 20-11 Task 2 and 20-10 exact target proof | COVERED |
| CONTEXT | D-06 | Reproducers before fixes, honest passing guards | Historical reproducers; 20-11 Task 1 fail-first adapter controls | COVERED |
| CONTEXT | D-07 | Bounded wrong-model controls hit intended oracles | 20-01/02/03/07/08/09; 20-11 Task 1 stage-specific refusals | COVERED |
| CONTEXT | D-08 | Derived line versus retained request/poll | 20-02/03/07 retained | COVERED |
| CONTEXT | D-09 | Integer timeline and coherent split-sample access | 20-02/07 retained, no core change | COVERED |
| CONTEXT | D-10 | Amended finite-margin development gate and explicit physical limit | 20-02/03/06; 20-11 Task 2 and 20-10 preserve limits | COVERED |
| CONTEXT | D-11 | Existing alignment before justified global change | 20-02/03 retained, no new offset | COVERED |
| CONTEXT | D-12 | Whole-sequence alternatives and persistent phase/history | 20-01/02/03 retained | COVERED |
| CONTEXT | D-13 | Local seams, no scheduler/API/profile/Phase 21 expansion | Historical implementation; 20-11 owned local adapter and 20-12 delivery only | COVERED |
| CONTEXT | D-14 | Format 1->2/revision 7->8 with refusal precedence | 20-02/04; unchanged pins in 20-11 Task 1/2 | COVERED |
| CONTEXT | D-15 | Serialize persistent history in its causal change | 20-02/03/08 retained; no new machine field | COVERED |
| CONTEXT | D-16 | Bounds/reachable histories, canonical state, atomic refusal | 20-03/04/08 retained | COVERED |
| CONTEXT | D-17 | Internal exact cutpoints versus public frame replay | 20-04/05/08; 20-11 Task 2/20-10 preserve labels | COVERED |
| CONTEXT | D-18 | Exact set/clear, nonzero audio, bounded feasibility | 20-04/08 retained | COVERED |
| CONTEXT | D-19 | Real loads, guest markers, negative controls and independent instances | 20-04/05/08; 20-11 Task 2, 20-10 real-host gate | COVERED |
| CONTEXT | D-20 | One branch/PR, ordered green changes and separately authorized delivery | 20-11 local repair -> 20-12 existing PR -> read-only 20-10 | COVERED |
| CONTEXT | D-21 | Cold original 108 PASS authority, measured six target outcomes | 20-11 unchanged per-lane controls + campaign; 20-12/10 real results | COVERED |
| CONTEXT | D-22 | Existing pin/caps/lanes, six hashes, vectors/MMC3/Holy Mapperel/boot, host, +1% | 20-11 exact actual CTests and unchanged validators; 20-12/10 fresh evidence | COVERED |
| CONTEXT | D-23 | Actual causal hashes/fingerprint/dirty provenance and clean-hosted debt | 20-11 Task 2 measured identity; 20-12 new source-bound runs; 20-10 content proof | COVERED |
| CONTEXT | D-24 | Owned integer instance state, no dependency/ROM/device/global latch, clean room/config/one step | All three plans' explicit scope and config exclusion | COVERED |

Verification gaps 2/3/4 and WR-01/WR-02 keep completed 20-07/08 assignments. Deferred Phase 21 IRQ latency/hijack, DMA/OAM/open-bus clusters, PAL/chip profiles, public debugger/stepping APIs, fetch-only Nightly expansion and unpromoted testing todos remain excluded. Context drift is advisory: current amended CONTEXT overrides older research; --gaps does not rewrite research. No source item is missing or silently deferred.

## Spec-less fallback and prohibition dispositions

There is no SPEC. The retained seven-row probe in scratch classifies the same historical fallback set; it is not new scope or a human approval request.

| Requirement/edge | Disposition carried forward |
|---|---|
| NMI-01 unclassified | Flagged-unverified classification assumption; explicit D-01..D-12 oracles remain covered by historical tests and exact hosted targets; no invented classifier resolution |
| NMI-02 unclassified | Flagged-unverified classification assumption; same finite-margin and whole-sequence limits retained |
| KEEP-02 adjacency | Resolved explicit by 20-08 equal-tick distinct full-key inventory plus duplicate/missing controls; 20-11 retains production hash gate; actual hosted equality still pending 20-10 |
| KEEP-02 empty | Resolved explicit by 20-08 frame-zero/empty-PCM canonical state and nonempty continuation; no empty/missing hosted bundle accepted |
| KEEP-02 ordering | Resolved explicit by sorted full-key inventory and duplicate/missing rejection; normalization requires unique logical input without flatten collisions |
| KEEP-03 idempotency | Resolved explicit by repeated canonical save/load/save and runner idempotent replay in 20-04/05/08; collector failure/retry control separately preserves truthful manifest state |
| KEEP-03 concurrency | Resolved explicit by synchronous two-instance interleaving and independent latch histories in 20-04/05; no asynchronous save/thread API introduced |

The clean-room/unmanifested-ROM prohibition is retained descriptor-less and flagged-unverified, with hygiene as supporting evidence only. Do not invent a node-test/lint enforcement descriptor, claim machine-proven clean room, waive it or turn it into a manual test-sign-off checkpoint. Assumption-delta detected=false introduces no identity question or approval gate.

## Verification grounding and planning controls

Existing CTest names were located in tests/CMakeLists.txt; m2 is supplied by tests/holymapperel/roms.cmake and appears in retained passing build rows. The command form inherits proven 20-09/prior phase build-and-ctest gates, and full `cmake --workflow --preset ci`. The new adapter CTest is explicitly created in 20-11 Task 1. No negative grep/count gates are authored. Discovery Level 0 uses current owned patterns, local CLI help and retained artifacts; no further implementation research, package audit, security threat register or SPEC was invented. Calibration: factor 1, sample_count 0, derived confidence low. Reversibility gates remain enabled; only already authorized normal publication is costly, with no newly surfaced one-way choice.
