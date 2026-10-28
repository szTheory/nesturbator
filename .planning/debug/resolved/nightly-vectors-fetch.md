---
status: resolved
trigger: "Nightly vectors-full fetch fails with curl 28 and bad pack header; resolve the Phase 20 delivery blocker"
created: 2026-10-28
updated: 2026-10-28T14:38:01Z
goal: find_and_fix
---

## Current Focus

hypothesis: Both confirmed causes are repaired: bulk lazy promisor acquisition and the new Windows fixture newline mismatch.
test: Local controls/full CI, actual six-platform CI/Nightly, live production availability and exact raw metadata retention all pass.
next_action: Stop this debug step; Plan 20-10 read-only content acceptance is the next separately invoked execution step.

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

- Normal hooks commit/push published b343083fe11a626fa5e768a03391539e949de93d on existing PR59 branch. Actual automatic PR CI 40696814449/1 and Nightly 40696814469/1 created at 2026-10-28T13:28:59Z match candidate. PR synthetic merge 32762001e3f156eac978b5b307a5d3605685f718; base ab9c2e6eb822c06f0dfb0f08a0ca42ec541358be. Runs active; availability not yet claimed.

- Fresh Nightly 40696814469/1 job 122845687330 vectors-full completed success on published b343083, confirming actual hosted cold acquisition no longer blocks this candidate. MMC3 and all three fuzz jobs also passed; suite-flake/session-controls and CI remain active. No rerun.

- Hosted Windows regression: CI 40696814449/1 x64 job 122847503174 and arm64 job 122847503220 fail only new vectors.fetch.transport. Logs retained as build/phase20-hosted-evidence-diagnostics/ci-job-122847503174.log and ci-job-122847503220.log. Exact 00.json actual hash/size identifies 00+CRLF versus expected 00+LF. Both have passed the preceding bounded prefetch/checkout model; final byte inventory correctly rejects fixture portability error.

- Corrected newline-free two-byte fixture control passes; owned forced-CRLF mutant fails with original 56a468... size4 evidence. Supplemental independent review LOOKS_GOOD; full ci rerun active. Only test bytes change; production source, canonical pins and policy unchanged.
- Production source helper measures a08da046cedeecbdd28227cc53d54e3d3e7dff2a93ee88aab9ead67da47bf61c /417 files; campaign fingerprint is unchanged 9b4d2199c8d6464c16d16f51766174aa64d633fb0a9d8b2968bac55013113993 /56 inputs.

- Corrected full cmake --workflow --preset ci passes 687/687 in 382.27 s and produces all three packages. Supplemental review accepted; actual corrected Windows verification will come from new automatic candidate-bound jobs. Config SHA unchanged.

- Normal publication of final candidate fe70d619d6a3cf7b80ed1867ff9faf4c7c9053b1 succeeds with hooks. Actual automatic CI 40700458574/1 and Nightly 40700458462/1 created 2026-10-28T13:52:29Z match exact source, event and workflow paths. Tested merge ef6d9e5ec2c94ddfd431c1d7162f620722a145df; base ab9c2e6eb822c06f0dfb0f08a0ca42ec541358be. Captured candidate file updated; old b343083 cannot satisfy final-source availability.

- Final-source Nightly 40700458462/1 vectors-full job 122858544021 passes (13:52:32Z to 13:54:49Z), along with MMC3/all three fuzz jobs. Final-source Windows arm64 build job 122859486841 and both Linux matrix builds pass; real corrected Windows evidence now confirms the synthetic byte repair. Other required final-source jobs remain active/queued; availability not yet claimed.

- Final Nightly 40700458462/1 completed SUCCESS at 2026-10-28T14:09:58Z; all seven test jobs pass and the schedule/main-only report is correctly skipped on this PR. Both Windows x64 122859486522 and arm64 122859486841 pass. CI has 15 completed successes; only Intel macOS build 122859486834 remains active (started 14:13:39Z), before hash/required rollups.

- Final CI 40700458574/1 completed success at 14:27:30Z with all 18 jobs. Final Nightly 40700458462/1 succeeds. Live shared availability passed at 2026-10-28T14:34:08Z; record SHA feb2ee889e32ed4ecea12214d8b7f745389afbb950525bb855bf9d6015bd6d9a, metadata manifest 85106b6520c1004254d6f67cf114a3359060797999a1f085051e188a2105f542, all 20 raw JSON responses retained and individually verified under availability/metadata. First hash-only proof preserved historically.

## Eliminated

- hypothesis: Configured pin is invalid or vector data no longer exists.
  evidence: Exact pinned commit/tree fetch succeeds; raw 00.json matches its expected SHA-256; all 256 files verify after bounded Git acquisition.
  timestamp: 2026-10-28

## Resolution

root_cause: Production acquisition failure: checkout asks the promisor remote for all 260 sparse-checkout blobs in one request; the measured request returns no pack payload before the existing low-speed cutoff, although initial pinned tree and bounded blob transfers succeed.
secondary_fix: Remove unnecessary line endings from synthetic payloads and hash/verify the same two ASCII opcode bytes.
secondary_root_cause: Newly added offline fixture uses native newline writes with an LF-only hash oracle; Windows emits CRLF, causing strict byte checks to reject synthetic data.
fix: Prefetch all required pinned sparse-checkout tree blob IDs in at most 16-object Git requests before checkout, preserving depth-1/blobless initial fetch, no-tags/no-write-fetch-head and unchanged low-speed policy.
oracle_type: derived (pinned file inventory and bounded request transport model)
verification:
  target_test: {result: pass}
  mutation_check: {result: skipped, reason_if_skipped: Stryker is unavailable/not configured for CMake; owned scoped batch/depth/CRLF mutants were all rejected}
  no_op_deletion: {result: pass, deletion_justified_by_rca: false}
  adjacent_tests: {result: pass, suites_run: [vectors-full 258/258, focused 7/7, complete ci 687/687 plus all three packages]}
  revert_and_reconfirm: {result: pass, bug_returned_on_revert: true, fixed_on_reapply: true, method: original-code scratch replay}
  portability_target: {result: pass, forced_crlf_mutant: rejected, independent_review: LOOKS_GOOD, full_ci: 687/687 pass plus all three packages}
  hosted_ci: {result: pass, run: 40700458574/1, jobs: 18/18, platforms: 6/6}
  hosted_nightly: {result: pass, run: 40700458462/1, test_jobs: 7/7}
  availability: {result: pass, record_sha256: feb2ee889e32ed4ecea12214d8b7f745389afbb950525bb855bf9d6015bd6d9a, retained_metadata_files: 20, full_manifest_verified: true}
  guardrail_verdict: accepted
files_changed: [tests/cmake/fetch_vectors.cmake, tests/cmake/vectors_transport.cmake, tests/CMakeLists.txt, README.md, tests/accuracy/baseline-metadata.txt, tests/accuracy/measurements.md]

## Specialist Review

Supplemental Windows fixture correction: independent focused re-review LOOKS_GOOD; newline-free generated payload, size and SHA oracle agree, with every transport/inventory control retained.

engineering:debug/general specialist skill unavailable in configured roots. Independent focused gsd-code-reviewer review found no production implementation defect and requested explicit initial-fetch depth/filter/pin/low-speed assertions. Added exact initial-fetch argv and shared environment assertions; final focused recheck LOOKS_GOOD: exact initial argv and all-fetch environment assertions close the sole coverage warning; no remaining focused issue.

## Prevention

- Code branch: a blobless initial fetch defers data acquisition to checkout; Git bulk-prefetches all 260 needed blobs at once. Sparse checkout limits paths, but did not bound that pack request. The transport model now rejects lazy checkout before all required blobs arrive and any explicit batch over 16.
- Environment branch: the measured bulk request provided no pack payload before the strict cutoff, while the same pinned tree and smaller requests succeeded. Upstream server internals remain unknown; true transport outages still fail with the unchanged policy.
- Why not caught: no offline gate existed for checkout request shape; the previous fetch guards tested safe directory ownership only. Cold Nightly caught the transient failure, but a rerun did not correct the request shape.
- Recurrence guard: tests/cmake/vectors_transport.cmake, registered as vectors.fetch.transport in the ci workflow, covers exact initial pin/depth/filter, every request cutoff, bounded prefetch, all blobs before checkout and final inventory refusal. Original-code replay and oversized-batch/depth mutants fail at their intended boundaries.
- MemPalace tool and CLI are unavailable; knowledge-base.md is the durable fallback when this session is archived.

Automated confirmation follows AGENTS.md rule 6 and the explicit task authorization: no manual sign-off is required. Plan 20-12 is available; 20-10, all four requirements and phase verification remain pending. The final local evidence-only commit is distinct from published source fe70d61.

Secondary prevention: synthetic transport payloads need no text newline, so they now contain exactly two ASCII opcode bytes on every platform. Both actual Windows builds verify the correction; a forced-CRLF mutant remains rejected.
