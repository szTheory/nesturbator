---
status: resolved
trigger: "Investigate Nightly run 40721285567: unexpected PUBLICATION_UNSTABLE and readiness cleanup exceeding 25 seconds"
created: 2026-10-28T17:00:39Z
updated: 2026-10-28T17:53:00Z
---

## Current Focus

hypothesis: Native Windows newline expansion changes the new CMake binary fixture before the correct binary reader hashes it; prior clock/hash mechanism repairs remain preserved history.
test: Compare exact fixture hex and independent FNV for intended versus CRLF-expanded bytes, then reuse the owned native binary writer and reject newline/byte mutations before hash comparison.
expecting: A platform-neutral binary writer retains CR/LF/high-byte coverage and the same frozen FNV on every OS.
bug_class: bohrbug
candidate_causes: [code repeated process launches and fixture replay, config 1000-ms publication cap, environment Intel scheduling latency, data corrupt or changing publication]
and_gate: yes; a small test publication window combines with helper overhead; the readiness failure has successful semantic results but excessive later-prefix work.
prior_reasoning_checkpoint:
  hypothesis: Repeated interpreter clock launches amplify startup cost across the prefix; interpreted bytewise hash work can consume the publication window before a second stability observation, preempting the intended load-send fault.
  confirming_evidence:
    - Original driver with 100-ms simple clock cost reports successful readiness/progression but fails total 25000-ms bound at 26070 ms.
    - Independent original-driver load control with 1050-ms interpreted hash cost reports PUBLICATION_UNSTABLE at 2225 ms instead of load-probe_SEND_FAILED, matching hosted fault preemption.
  falsification_test: Under unchanged controlled helper cost and strict deadlines, native clock/hash reuse fails the same conditions or changes FNV identities, startup validation, fault reasons or one-shot sends.
  fix_rationale: Reuse the already owned runner monotonic clock and progression bounded FNV code through required frame_index, removing redundant interpreter work from repeated observations without changing time budgets or semantics.
  blind_spots: Hosted failure has no per-process trace; controlled cost reproduces the mechanisms but does not establish precise hosted scheduling or interpreter-duration attribution. New source still needs Intel hosted validation.
  candidate_causes:
    - code repeated interpreter startup and bytewise hash work within timed observations
    - config narrow test-only 1000-ms publication window
    - environment startup or scheduling cost on hosted Intel
    - data genuinely changing or malformed screenshots (excluded by immutable fixture controls)
  and_gate: yes; bounded work combines with environment/helper cost to preempt otherwise valid fixture semantics. Keep the existing publication and total deadlines unchanged.
next_action: Local fixture-portability repair is terminal and committed; parent delivery must obtain fresh hosted validation of the latest source, especially Windows, without relabeling the prior successful Intel source.

reasoning_checkpoint:
  hypothesis: CMake's native Windows file writer inserts CR before the LF in the binary vector, so the correct binary hash reader observes different bytes.
  confirming_evidence:
    - Windows arm64 returned 8bb25531c6274ace; local independently written intended bytes 010a0d7f80ff hash to 112184356382ae07, while exact CRLF-expanded bytes 010d0a0d7f80ff hash to the hosted result with both independent FNV and actual frame_index.
    - The only newline-bearing new fixture is written by CMake file(WRITE); existing native marker_write already opens wb and writes explicit bytes.
  falsification_test: Native wb fixture still has hex other than 010a0d7f80ff or unchanged binary reader disagrees with frozen FNV; deliberately inserted CR/changed bytes pass the exact-byte gate.
  fix_rationale: Use the owned native binary writer for this fixed binary vector, then assert exact hex before hashing; this removes text newline conversion while retaining all original binary coverage and hash expectation.
  blind_spots: Original Windows artifact did not retain hex; conversion is inferred from exact matching transformed-byte hash. Native Windows validation of this latest source remains pending hosted execution.
  candidate_causes: [code text fixture writer used for binary bytes, environment Windows newline conversion, data CR inserted before LF, code hash reader bug excluded by independent intended/transformed controls]
  and_gate: yes; newline-bearing fixture plus a text-mode writer on Windows are both required. Other new fixtures contain only x/foobar or are empty and have no line terminators.

## Symptoms

expected: All three Intel macOS retroarch-session tests pass. The load-send-once fault reports exactly load-probe_SEND_FAILED, and readiness-delayed observes its delayed response and completes driver plus cleanup within 25000 ms.
actual: Nightly 40721285567 session-controls job 122932568072 failed two of three tests. All other executed Nightly jobs succeeded. Required PR CI 40721285683 passed all 18 jobs on the same candidate head.
errors: load-send-once returned PUBLICATION_UNSTABLE instead of load-probe_SEND_FAILED (driver plus cleanup 13776 ms). readiness-delayed operation took 2693 ms within its [1201,4500) ms range, but driver plus cleanup took 29927 ms and exceeded 25000 ms; fixture 4569 ms, setup 267 ms, later prefix 20167 ms, driver status 0/reason OK.
reproduction: Nightly macos-15-intel runs cmake --preset ci, cmake --build --preset ci --target frame_index nesturbator-run, then ctest --preset ci -L '^retroarch-session$' --no-tests=error. Preserve these strict outcomes and derive deterministic targeted controls before the repair.
timeline: Observed in the final shipping-head Nightly on 2026-10-28. Candidate head is 6d2d1370d8fa8bf6c96c2e431a90a0d1b2d955a2; prior local ci passed 687/687 in 404.35 seconds and its session controls passed in 261.53 seconds. The previously accepted Phase 20 hosted bundle identifies older effective-equivalent candidate fe70d61; retain that history separately from any source-changing repair.

## Eliminated

- hypothesis: Corrupt or changing synthetic screenshot bytes explain the controlled PUBLICATION_UNSTABLE failure.
  evidence: The failed publication remains byte-for-byte equal to the immutable 184335-byte generated fixture after the driver exits; fake transport copies synchronously and exits before the driver observes it. Charging interpreted hash time alone triggers the failure.
  timestamp: 2026-10-28T17:19:45Z

- hypothesis: Raw bytewise hash CPU cost alone explains all of the slow full prefix.
  evidence: One local screenshot hash takes 19 ms; independent clock-only charging expands later-prefix 5121 to 21923 ms while readiness remains semantically valid. Separate clock and hash mechanisms are required.
  timestamp: 2026-10-28T17:19:45Z

## Evidence

- timestamp: 2026-10-28T17:16:14Z
  checked: Actual refreshed production campaign and full-source digest helper.
  found: Campaign exit 0, all 22 pages and 146 scored rows completed, 114 PASS/32 FAIL, no skip/not-run/timeout, five DRAW. Report SHA-256 remains 29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac. Current 56-input fingerprint is 424db30d3e712a95c6ebacd7785b710ca67beff078269dd9346c654d390ecc84, source revision 6d2d1370d8fa dirty 1, host_elapsed_guest_ns 4678903000. Production effective-source digest is bfcf8e6c94f5e2ee4cdc01cb1822f5fb04d273abb15dae534a66dc51895d2b79 over 418 files.
  implication: Current baseline metadata is actual local evidence; historical hosted 417-file acceptance remains historical. New-source CI/Nightly validation is required and is outside this no-push step.

- timestamp: 2026-10-28T17:14:00Z
  checked: Fresh full CI workflow after final controlled green.
  found: Final controlled load reason correct at 9762 ms; delayed/stalled readiness totals 7057/10424 ms and operations 1607/5051 ms, with active-shim assertions passed. Full workflow caught missing cmake_minimum_required in new helper test and stale AccuracyCoin baseline input fingerprint caused by the required public-header comments. Stopped stale workflow with exit 130 before accepting it.
  implication: Correct the new script's policy declaration and obtain actual current-source campaign evidence; never relabel old hosted artifacts as current.

- timestamp: 2026-10-28T17:12:00Z
  checked: Final helper usage assertion added after first successful native controls.
  found: Load control failed the new usage assertion although exact load-probe_SEND_FAILED and progression passed. Its wrapper also counted the harness's final timing sample, yielding two clocks; readiness scopes PATH only to the driver. Restored PATH after the load driver returns so the usage assertion measures the driver alone.
  implication: Fixture instrumentation scope error corrected; no driver behavior or deadline adjustment. Re-running final controls before fresh full workflow.

- timestamp: 2026-10-28T17:11:00Z
  checked: Repaired controls and independent single-component reversions.
  found: Both-native green load reason load-probe_SEND_FAILED at 8973 ms; readiness delayed/stalled operations 1591/5053 ms and totals 6900/10281 ms. Reverting only hash recreated PUBLICATION_UNSTABLE at 1982 ms. Reverting only clock left delayed barely passing at 24972 ms but stalled failed 28591 ms, with operation 5844 ms still valid. Each revert retained the other native helper.
  implication: Independent causal controls establish separate publication and clock repair effects. Full native source restored before final verification. Binary FNV vectors include high bytes and CR/LF to preserve Windows binary-read behavior.

- timestamp: 2026-10-28T17:10:44Z
  checked: Original-driver independent helper-cost load-send control.
  found: Charging 1050 ms only to interpreted FNV produced PUBLICATION_UNSTABLE at 2225 ms, before the intended LOAD_STATE_SLOT 0 send. Fixture bytes remain immutable, and 1000-ms publication cap and exact load-probe_SEND_FAILED oracle are unchanged.
  implication: Confirmed publication preemption mechanism. Exact hosted per-process cause remains unobserved; describe measured reproduction and limits honestly.

- timestamp: 2026-10-28T17:10:44Z
  checked: Local original-driver readiness baseline and bounded interpreter clock startup controls.
  found: Baseline delayed/stalled driver+cleanup 7473/10824 ms. Adding 60 ms per simple Perl clock call yielded 19268/22641 ms with valid semantics; adding 100 ms produced readiness-delayed operation 2257 ms, later-prefix 21923 ms and total 26070 ms, failing the unchanged 25000-ms bound. Hundreds of clock interpreter launches account for the amplified prefix work.
  implication: Confirmed interpreter-startup amplification independently of malformed replies, cleanup leaks or wrong progression. Native clock reuse can remove this repeated interpreter cost without relaxing the deadline.

- timestamp: 2026-10-28T17:10:44Z
  checked: Knowledge base, historical Phase 16 files, driver and harness source, hosted failure rows.
  found: No matching resolved KB pattern. Historical sessions remain untouched. Fault controls use PUBLICATION_MS=1000 and readiness uses 5000; both replay --progression-selftest inside each invocation. The hosted readiness operation succeeds; 20167 ms is later-prefix work. Local 184335-byte Perl FNV takes 19 ms.
  implication: Distinguish process startup/redundant replay cost from publication mutation. SBFL unavailable (no per-test coverage); rr unavailable on macOS; use controlled startup latency and targeted measurements, not flaky coverage ranking.

- timestamp: 2026-10-28T17:00:39Z
  checked: Nightly run 40721285567 job metadata and raw job 122932568072 log via GitHub connector.
  found: vectors-full, execution-fuzz, rom-loader-fuzz, state-fuzz, suite-flake and mmc3-oracle succeeded; session-controls failed; report skipped for PR event. Raw job log retained in scratch as /tmp/nesturbator-nightly-40721285567-session-controls.log.
  implication: Start with the owned host-session tests, preserving all other outcomes. Required CI success does not resolve the Intel-specific failure.

## Prior local resolution (first delivery)

root_cause: Repeated Perl interpreter launches amplify clock-observation cost across the full prefix; interpreted bytewise artifact hashing can exhaust the narrow publication window before stable fixture observations, preempting the intended load-send failure. Both require helper/environment cost combined with existing finite budgets; precise hosted per-process timing was not captured.
fix: Reuse the owned runner monotonic clock and shared bounded FNV-1a reader through native frame_index modes after bootstrap. Preserve all budgets, identities, exact faults and one-shot counts; add independently charged clock/hash controls and frozen binary/cap vectors.
verification:
  target_test: { result: pass, command: 'cmake --workflow --preset ci', tests: 688/688, ctest_wall_seconds: 478.23, packages: [library, runner, libretro] }
  mutation_check: { result: skipped, reason: no Stryker configured for C/shell; independent single-helper reversions are tested directly }
  no_op_deletion: { result: pass, deletion_justified_by_rca: true }
  adjacent_tests: { result: pass, suites_run: [all CI tests, native helper vectors and cap boundaries, source-root preflight, actual 22-page campaign, production progression and cleanup matrix] }
  revert_and_reconfirm: { result: pass, bug_returned_on_revert: true, fixed_on_reapply: true, independent_reverts: [interpreted hash, interpreted clock] }
  guardrail_verdict: accepted
oracle_type: specified (exact failure reason, one-shot sends and fixed timing caps), derived (independently frozen FNV-1a vectors)
files_changed: [tests/retroarch/frame_index.c, tests/retroarch/state_session.sh, tests/retroarch/test_state_session.sh, tests/retroarch/CMakeLists.txt, tests/cmake/session_helpers.cmake, README.md, include/nesturbator.h, tests/accuracy/baseline-metadata.txt, tests/accuracy/measurements.md, .planning/STATE.md]
source_commit: ef0bab9b081b0b66f7ebbb92be52f9f79f8cc498
effective_source_sha256: bfcf8e6c94f5e2ee4cdc01cb1822f5fb04d273abb15dae534a66dc51895d2b79
effective_source_count: 418
investigation_cycles: 1 (two independent mechanisms)
fix_cycles: 3 (native repair; shim-scope assertion correction; script-policy and measured provenance correction)
human_verification: Not requested; repository AGENTS.md rules 6 and 7 require automated checks and completion inside this authorized step. Local workflow is terminal; new-source hosted validation is explicitly pending, not passed.
limitations: The failed hosted job retained no per-process clock/hash trace. Controls reproduce the mechanisms and causally distinguish repairs; exact historical helper durations and repaired-source Intel timing remain unobserved.

## Final automated evidence

Fresh workflow exited 0 after the last effective-source edit: 688/688 tests in
478.23 seconds; library, runner and libretro packages generated. The successful
workflow log SHA-256 is `e2e1f2c43afe5366091afdd9554f82436a2058aecc07c7561e60a2e641a39fe2`.
Its serial matrix passed in 290.50 seconds, source-root preflight in 21.86 seconds
and focused two-case readiness process in 29.36 seconds. The 29.36 seconds spans
both cases and fixture generation; each driver retains its strict 25000-ms bound.

| Actual final CI control | Operation ms | Driver plus cleanup ms | Result |
| --- | ---: | ---: | --- |
| Load send fault in full matrix | reason-specific | 13466 | `load-probe_SEND_FAILED` |
| Delayed readiness in full matrix | 1671 | 9422 | exact reply, full prefix, owned cleanup |
| Stalled readiness in full matrix | 5120 | 13237 | exact reply after capped/reaped child, full prefix |
| Dedicated delayed readiness | 1741 | 11244 | operation in [1201,4500), total <25000 |
| Dedicated stalled readiness | 5134 | 15960 | operation in [4500,7000), total <25000 |

Full production sequence, all sixteen boundaries, forty rewinds, one-shot send
counts, negative fault reasons, interruption and process/descriptor cleanup all
passed. Direct cap/vector tests cover missing/empty/singleton, binary CR/LF/high
bytes, 4096-byte buffer neighbors and 1 MiB cap neighbors. The active latency
shim asserts exactly one driver bootstrap interpreter clock and zero interpreted
artifact hashes; the shim cannot silently remain dormant.

Independent causal controls:

- Original 100-ms clock cost: valid readiness operation 2257 ms, total 26070 ms,
  rejected by unchanged 25000-ms gate.
- Original 1050-ms interpreted hash cost: `PUBLICATION_UNSTABLE`, total 2225 ms;
  failed image byte-equal to immutable 184335-byte fixture, SHA-256
  `6adf2de6f03c52a8b7189e2a8789785c618674bceead1195a5232868a4c01f08`.
- Hash-only revert with native clock: same `PUBLICATION_UNSTABLE`, 1982 ms.
- Clock-only revert with native hash: delayed 24972 ms narrowly passed; stalled
  28591 ms failed while its operation 5844 ms remained valid.
- Both reapplied: final targeted exact load reason 9762 ms; delayed/stalled
  totals 7057/10424 ms. Fresh full workflow above then passed after policy and
  actual campaign provenance updates.

Raw local scratch logs: `nightly-intel-slow100-baseline.log`,
`nightly-publication-red.log`, `nightly-hash-revert.log`,
`nightly-clock-revert.log`, `nightly-helper-final2.log` and
`nightly-intel-final-ci2.log` under the temporary directory. Canonical current
campaign output is `build/nightly-intel-session-campaign/`; effective source
manifest is `build/nightly-intel-effective-source.tsv`. This durable session
retains all required outcomes if ignored/raw scratch artifacts disappear.

## Prevention

The code branch multiplied interpreter startup across hundreds of clock samples
and interpreted every artifact byte within a publication cap. The environment
branch amplified helper work on a slower scheduled runner; no per-helper trace
was captured in the original hosted run. Functional CI passed because no
deterministic helper-cost gate existed. The current test reproduces both
mechanisms independently while retaining original correctness/deadline oracles.

why_not_caught: CI exercised semantic session faults but had no deliberate helper-cost control; only the Intel Nightly exposed this scheduling-sensitive amplification.
recurrence_guard: tests/retroarch/test_state_session.sh latency controls run in retroarch.session.controls and retroarch.session.readiness; retroarch.session_helpers freezes native identities, binary reads and cap/clock behavior. Single-helper reversions reproduce each failure independently.
semantic_recall: MemPalace tools unavailable; durable knowledge-base fallback used and updated.

## Continuation: hosted fixture portability

The same debug invocation resumed after publication of ef0bab9/e802a1e. CI
40736138275 Windows arm64 job 122985617442 and Windows x64 job 122985617530
each failed only retroarch.session_helpers. The frozen expected FNV was
112184356382ae07; actual was 8bb25531c6274ace. The raw arm64 log remains in
the temporary directory as nightly-intel-repair-windows-arm.log. Previous
resolution, local results and failures above remain history for that source.

### Additional evidence

- checked: Independent binary byte and FNV controls.
  found: Intended hex 010a0d7f80ff produces 112184356382ae07; exact CRLF-expanded hex 010d0a0d7f80ff produces 8bb25531c6274ace with both independent FNV and the unchanged native reader. This matches both Windows failures. Original Windows fixture hex was not retained, so the transformation is inferred from the matching controlled result.
  implication: Repair fixture materialization, retaining the expected hash and binary coverage. No native hash-reader change is supported.
- checked: All fixtures introduced by the first repair.
  found: Only the CR/LF/high-byte vector used CMake text output with line terminators. The other new fixtures are empty, foobar, or repeated x; none contain a newline. Existing marker_write opens wb and writes exact byte counts and is already used for binary marker and state artifacts.
  implication: Reuse that owned writer through one small fixed-vector test-tool mode; no new dependency or core contract change.
- checked: Repaired helper and meaningful writer mutation.
  found: Native writer creates hex 010a0d7f80ff; exact hex is required BEFORE the frozen hash comparison. Fresh child processes reject explicit CRLF and high-byte mutations. Changing the actual native writer to insert CR recreated hex 010d0a0d7f80ff and failed the byte gate, test exit 8. Restoring the writer returned target test to PASS in 0.16 s.
  implication: The strengthened test distinguishes wrong fixture bytes from hash-reader behavior and kills the observed defect before hash comparison.
- checked: GitHub connector terminal status and decoded Intel logs.
  found: Nightly 40736138106 Intel job 122985089791 passed all 3 session tests in 617.04 s (source_root 24.15 s, controls 559.40 s, readiness 33.41 s). It checked out merge d977694 of candidate e802a1e35e369a2454364384eb715e523db47749 into ab9c2e6eb822c06f0dfb0f08a0ca42ec541358be. Six other Nightly jobs succeeded; report was skipped for the PR event. CI Apple Silicon and Linux session-control jobs also succeeded; both Windows jobs showed the identical sole fixture defect.
  implication: Original Intel session failures have actual hosted confirmation for the first repair source. This does not test the latest fixture-portability source.
- checked: Cross-platform independence of the new negative controls.
  found: CMake text output for LF-bearing negative fixtures would also insert CR on Windows, confounding the high-byte-only negative. Before acceptance, moved all three fixed byte vectors to the same owned wb writer and asserted exact positive/CRLF/high-byte HEX independently. Observed hex is respectively 010a0d7f80ff / 010d0a0d7f80ff / 010a0d7f80fe. Both malformed vectors fail the fresh-process exact-byte gate, and invalid mutation names return 2. Focused final test passed 0.19 s.
  implication: No newly added binary fixture uses text output. Stopped the superseded full workflow (exit 130) and started the fresh final workflow only after this last effective-source edit.
- checked: Final CI 40736138275 job metadata through GitHub connector.
  found: All 14 non-Windows work jobs succeeded, including macOS Intel build and both required session-control jobs. The only two failing work jobs are Windows x64/arm64 with the same sole binary-fixture mismatch. CI required consequently failed; hash-equality was skipped. Nightly's seven work jobs all succeeded.
  implication: No additional supported runtime failure remains in the first repair's hosted execution. This is historical source evidence, not acceptance of the latest fixture repair.

## Resolution

root_cause: The new binary vector used CMake file(WRITE), whose Windows native newline handling inserts CR before LF; the correct binary FNV reader therefore receives different fixture bytes. Native binary writer required to preserve intended bytes independently of platform.
fix: One fixed-vector frame_index mode reuses marker_write(wb) for the positive vector and two explicit mutations; CMake requires exact fixture and mutation hex before comparing the unchanged frozen FNV or exercising rejection. Real child-process CRLF and high-byte negatives exercise the exact byte gate. Other new fixtures were audited as newline-free.
verification:
  target_test: { result: pass, test: retroarch.session_helpers, seconds: 0.19 }
  mutation_check: { result: pass, mutant_killed: actual native writer with inserted CR rejected at hex gate before hashing; 2 persistent malformed-fixture controls also pass }
  no_op_deletion: { result: pass, deletion_justified_by_rca: true }
  adjacent_tests: { result: pass, suite: fresh final full ci workflow session 94077, tests: 688/688, seconds: 495.95, packages: [library, runner, libretro] }
  revert_and_reconfirm: { result: pass, defect_reproduced: true, fixed_on_reapply: true, scope: exact observed Windows bytes modeled in actual writer; original Windows source failed hosted, latest Windows source remains pending }
  guardrail_verdict: accepted
files_changed: [tests/retroarch/frame_index.c, tests/cmake/session_helpers.cmake]
unchanged_contracts: [native FNV reader, frozen expected hash, CR/LF/high-byte coverage, 1 MiB cap, clocks, deadlines, emulation, public header, README, AccuracyCoin input fingerprint]
investigation_cycles: 2 cumulative (initial timing investigation; deterministic hosted fixture-portability continuation)
fix_cycles: 5 cumulative (three first-delivery cycles; fixture-portability repair and independent native mutation refinement)
oracle_type: specified exact hex before derived independently frozen FNV
hosted_validation: Prior repair Intel Nightly passed; newest fixture repair still requires fresh hosted validation after publication.
source_commit: fa321e587babce0a6434406e5bff0e0c996ceb32
effective_source_sha256: 87ee34cf7b5ba45d87e748ab69b69f929a7fe7be96c02ed044bf9b1394f93202
effective_source_count: 418

### Continuation terminal evidence and prevention

Fresh `cmake --workflow --preset ci` exited 0 after the final effective-source
edit: **688/688 tests in 495.95 seconds**, all three packages generated. Native
helper test passed in 0.18 s, session controls in 302.49 s, source-root in 10.32 s
and dedicated readiness in 21.57 s. Workflow log SHA-256 is
`2d4445c11cc2ed47979016110b2bbc1657fa09ce3c9df3ea49e335fe74b6c6f5`;
raw log is `nightly-intel-portability-final-ci.log` in the temporary directory.
No effective source changed after this workflow began. Core/public-header/README
contracts, all time caps and the 56-input AccuracyCoin fingerprint are unchanged.

The source repair commit is `fa321e5` (two test files, normal hooks and configured
GitHub noreply identity). An initial sandbox-only Git staging denial was resolved
by authorized narrowly scoped tool escalation; no approval rejection occurred.
The original config remains unstaged with SHA-256
`62a22f6862efb7c74ee31baf2f22bae32d453e79aaec6726dbcf3b7213d47167`.

The latest source digest is
`87ee34cf7b5ba45d87e748ab69b69f929a7fe7be96c02ed044bf9b1394f93202`
over 418 files. The historical successful Intel job tested merge
`d977694361b9c939829490974409f43d93a683da`, candidate
`e802a1e35e369a2454364384eb715e523db47749`, protected base
`ab9c2e6eb822c06f0dfb0f08a0ca42ec541358be`; it does not test this latest
fixture repair. Fresh latest-source hosted validation is pending parent delivery.

why_not_caught: The original binary fixture used a platform text writer and compared only the eventual hash; local POSIX execution did not expand LF and could not expose the Windows materialization defect. Real Windows CI caught it before acceptance.
recurrence_guard: retroarch.session_helpers requires exact positive and mutation HEX from the owned wb writer BEFORE hash/rejection oracles; child-process byte gates reject isolated CRLF insertion and high-byte changes. An actual native-writer CR insertion mutant was killed by the exact-byte gate, then restoration passed. All other new fixture writes were audited as empty or newline-free.
