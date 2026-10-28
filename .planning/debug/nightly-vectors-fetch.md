---
status: verifying
trigger: "Nightly vectors-full fetch fails with curl 28 and bad pack header; resolve the Phase 20 delivery blocker"
created: 2026-10-28
updated: 2026-10-28
goal: find_and_fix
---

## Current Focus

reasoning_checkpoint:
  hypothesis: Checkout bulk-prefetch of 260 missing blobs causes a stalled smart-HTTP pack request, which hits the unchanged 1000 B/s for 60 s cutoff.
  confirming_evidence:
    - Cold original reproduction ends with the exact hosted curl28/bad-pack-header/e9557999 checkout error.
    - Initial pinned tree fetch takes 0.4 s; trace records 260 blob wants and only empty sideband heartbeats before failure.
    - Explicit fetch of 16 pinned vector blobs succeeds in 4.97 s with identical transport policy; pinned raw file also verifies.
  falsification_test: If explicit bounded blob prefetch still leaves checkout requesting missing blobs, or a cold complete fetch fails under the same budget, the proposed repair is inadequate.
  fix_rationale: Reuse Git and pinned tree IDs, fetching at most 16 objects per request before checkout; avoids the observed failing request shape without changing pin, budget, or validation.
  blind_spots: Upstream server internals are unobserved; 16-object batches can still fail during a true outage; fresh hosted execution remains required.
  candidate_causes: [code: implicit unbounded checkout blob batch, environment: smart-HTTP pack response stalls, config: invalid pin]
  and_gate: yes; bulk checkout request shape plus upstream response delay combine. Invalid pin is refuted by successful tree and matching raw file.
bug_class: Heisenbug-Mandelbug
next_action: Commit explicit focused repair files, update existing PR59 description, publish the same branch normally, and discover actual new automatic CI/Nightly run identities.

## Symptoms

expected: The configured Nightly vectors-full job cold-fetches the pinned 65x02 input, verifies every pinned file, and runs its configured vectors without skipped results.
actual: Repeated hosted runs fail at cpu.vectors-full.fetch; dependent vector cases are not run and Phase 20 current-source availability is blocked.
errors: Git exit 128 after curl 28 (less than 1000 bytes/sec for 60 seconds), bad pack header, and failure to retrieve pinned promisor object e95579991e0ab08ee8fe79b57bcae7f71a2b9d02.
reproduction: Existing Nightly run 40580594558 attempts 1 and 2 on candidate afba1d091bba916a9fe803f71e18788343a16219; attempt-2 failed vectors-full job 122471173432. Use its retained diagnostic and the configured cold fetcher path.
started: Intermittent upstream vector-fetch failures recorded on 2026-10-26 and repeated on 2026-10-27; other candidates have successful vectors-full runs. Exact-head CI 40580594574/1 passed for afba1d0.

## Constraints

- Preserve the existing phase branch, pins, complete checksum inventory, cold-fetch policy, content gates, and unrelated config edit.
- The user requested diagnosis and repair of this blocker. Normal existing-PR delivery is persistently authorized; no merge, force push, new PR, or workflow_dispatch.
- Complete this debug step only; leave Plan 20-10 content acceptance and phase verification for execute-phase.
- Old candidate availability and diagnostic replay are historical evidence, not acceptance of a repaired source.

## Evidence

- Phase state, 20-12 halted summary, and 20-DELIVERY.md record the failed exact-source acquisition. Current tracked working-tree change before this session is only .planning/config.json.

- Hosted failure is at checkout FETCH_HEAD, after successful depth-1 blobless fetch; 62.59 seconds total, so commit acquisition succeeded.
- No knowledge-base.md or project skills directories exist; configured debugger skills query was empty. SBFL skipped: no failing test with per-test coverage.

- Cold original local trace: initial fetch finishes in 0.4 s; checkout launches a promisor fetch with 260 blob wants (including .gitignore e9557999, not a commit). Packet trace shows empty sideband heartbeats but no pack payload. Official Git partial-clone docs confirm checkout bulk-prefetches all needed blobs.
- Direct pinned 00.json: 5,441,451 bytes in 0.87 s, SHA-256 matches inventory; invalid pin/content unavailable is refuted.

- Explicit 16-vector-blob Git fetch succeeds in 4.97 s under unchanged low-speed policy. Original cold fetch reproduces the exact hosted checkout failure.
- engineering:debug/general specialist skill unavailable in configured skill roots; orchestrator will arrange independent read-only review.

- Owned vectors.fetch.transport control fails against original checkout, passes repaired fetcher, and rejects a 17-object-batch mutant at oversized blob pack request. Replaying original bytes under SCRIPT override reconfirms RED without mutating the working tree.
- Cold complete repaired fetch succeeds and verifies all 256 pinned files. Full vectors preset passes 258/258 in 8.40 s using those freshly acquired files.
- Actual 22-page production campaign completes 114 PASS / 32 FAIL, zero skip/not-run/timeout; report hash unchanged 29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac, fingerprint 9b4d2199c8d6464c16d16f51766174aa64d633fb0a9d8b2968bac55013113993 (56 inputs). Baseline refreshed from actual measured metadata.

- Final review LOOKS_GOOD after initial-fetch assertions; depth-2 mutant is rejected at initial pinned tree fetch policy changed. Focused gates pass 7/7. Initial full ci completed 686/687 with only expected stale fingerprint activation failure before metadata refresh; final full ci is running against refreshed inputs.
- Full effective source digest 4475c98248e1eb37862abe5993724e83f199824834f8ca1f2267cc3951583d83 covers 417 files.

- Final cmake --workflow --preset ci passes 687/687 tests in 467.50 s and creates library/runner/libretro ZIPs; no failure. Effective source digest remains 4475c98248e1eb37862abe5993724e83f199824834f8ca1f2267cc3951583d83 /417 files. Config SHA-256 remains 62a22f6862efb7c74ee31baf2f22bae32d453e79aaec6726dbcf3b7213d47167. Historical delivery bundle preserved under build/phase20-hosted-evidence-history/pre-vector-repair-0039.

## Eliminated

- hypothesis: Configured pin is invalid or vector data no longer exists.
  evidence: Exact pinned commit/tree fetch succeeds; raw 00.json matches its expected SHA-256; all 256 files verify after bounded Git acquisition.
  timestamp: 2026-10-28

## Resolution

root_cause: Checkout asks the promisor remote for all 260 sparse-checkout blobs in one request; the measured request returns no pack payload before the existing low-speed cutoff, although initial pinned tree and bounded blob transfers succeed.
fix: Prefetch all required pinned sparse-checkout tree blob IDs in at most 16-object Git requests before checkout, preserving depth-1/blobless initial fetch, no-tags/no-write-fetch-head and unchanged low-speed policy.
oracle_type: derived (pinned file inventory and bounded request transport model)
verification:
  target_test: {result: pass}
  mutation_check: {result: pass, method: owned 17-object-batch mutant rejected; Stryker unavailable for CMake}
  no_op_deletion: {result: pass, deletion_justified_by_rca: false}
  adjacent_tests: {result: pass, suites_run: [vectors-full 258/258, focused 7/7, complete ci 687/687 plus all three packages]}
  revert_and_reconfirm: {result: pass, bug_returned_on_revert: true, fixed_on_reapply: true, method: original-code scratch replay}
  guardrail_verdict: accepted
files_changed: [tests/cmake/fetch_vectors.cmake, tests/cmake/vectors_transport.cmake, tests/CMakeLists.txt, README.md, tests/accuracy/baseline-metadata.txt, tests/accuracy/measurements.md]

## Specialist Review

engineering:debug/general specialist skill unavailable in configured roots. Independent focused gsd-code-reviewer review found no production implementation defect and requested explicit initial-fetch depth/filter/pin/low-speed assertions. Added exact initial-fetch argv and shared environment assertions; final focused recheck LOOKS_GOOD: exact initial argv and all-fetch environment assertions close the sole coverage warning; no remaining focused issue.
