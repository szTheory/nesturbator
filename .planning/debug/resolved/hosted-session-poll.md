---
status: resolved
trigger: "PR 61 Nightly failed again on retry with DELAY_PUBLICATION_POLL_DEADLINE; focused interruption check passed six local runs. Resolve hosted session-control failures before merging."
created: 2026-10-29
updated: 2026-10-29
---

## Current Focus

hypothesis: Both reproduced mechanisms are repaired and actual hosted session qualification passes on f9ae22e.
test: Completed discriminating red/green controls, full local CI, required hosted CI and all hosted session jobs with unchanged limits.
expecting: No further repair work in this debug step; source-changing failures remain distinct from later evidence-only publication checks.
next_action: Observe publication checks and the remaining Nightly suite-flake job; merge PR 61 separately once checks are green. No merge, teammate-review prompt or further lifecycle command ran.

## Symptoms

expected: Required CI and Nightly session controls pass, preserving exact fault reasons, one-shot commands, progression and cleanup bounds.
actual: Required CI 40943821804 succeeds on head 2940249; Nightly 40943815484 retry fails retroarch.session.controls in 249.79 s. Source-root and readiness controls pass.
errors: state_session: DELAY_PUBLICATION_POLL_DEADLINE after successful prefix boundaries 1 and 2. Earlier reported interruption failure has six passing focused local runs, which do not cover this sleep path.
timeline: Final PR 61 shipping-head Nightly on 2026-10-29, after prior accepted phase evidence on candidate 54c7db2.
reproduction: Dedicated macos-15-intel job builds frame_index and nesturbator-run, then runs ctest --preset ci -L '^retroarch-session$' --no-tests=error.

## Eliminated

## Evidence

- timestamp: 2026-10-29
  checked: Fresh CI 40956561386/1 on f9ae22ecb04df41d6d7394e14b590b47e689c138.
  found: All 18 jobs succeed, including CI required, six platform builds, hash equality, both dedicated session jobs, ASan, performance and actual RetroArch integration. Nightly still has only its suite-flake job running at this observation; its other six work jobs succeed.
  implication: Required hosted qualification is complete on the repair source. Preserve the separate Nightly rollup status until its final job completes.

- timestamp: 2026-10-29T21:31:46Z
  checked: Fresh Nightly 40956561408/1 Intel session job 123738058128 on repair f9ae22ecb04df41d6d7394e14b590b47e689c138.
  found: All three tests pass: source-root 35.99 s, full session controls 576.46 s, readiness 34.42 s; total 646.95 s. Required dedicated Linux and Apple Silicon session jobs also pass. Full CI and Nightly rollups are still running other jobs at this observation.
  implication: Actual hosted Intel validates the repaired path and both unchanged exact failure contracts. Prior failed attempts 40943815484/1 and /2 remain historical evidence.

- timestamp: 2026-10-29T21:17:00Z
  checked: Final cmake --workflow --preset ci after the last source change; tree hygiene, clang-format 18 header check, shell syntax and diff whitespace.
  found: 695/695 tests pass in 423.66 s and all three release ZIPs are generated. Session matrix 250.28 s, readiness 17.95 s, source-root 6.22 s. Other checks exit zero; config edit remains byte-identical and unstaged.
  implication: Local repair verification is complete; hosted qualification remains required for this new source.

- timestamp: 2026-10-29
  checked: Permanent actual-driver initialization control before and after the timing-order change.
  found: Red control reproduces both exact hosted messages, journal field invalid and journal-close-failed. Repaired control retains INTERRUPTED_TERM, three accepted owned trace events, one QUIT, reaped host and released reservation. Both ordinary and initialization cases run inside the registered full session control.
  implication: Regression is discriminating and deterministic; historical interleaving remains compatible rather than directly observed.

- timestamp: 2026-10-29
  checked: First full CI attempt and instrumentation work budget.
  found: The first full workflow was deliberately interrupted after 683 completed tests while the session matrix ran. The sleep shim was initially installed in existing readiness/load helper controls too; it is now restricted to the new poll regression to avoid extra shell startup in strict existing timers.
  implication: Treat the interrupted workflow as non-acceptance. Final source is being verified by a fresh complete workflow, with the original readiness/load instrumentation cost preserved.

- timestamp: 2026-10-29
  checked: Actual campaign after explicit ci configure/build.
  found: Compiled source 2940249e30ef dirty 1; 119 PASS/27 FAIL, 146 terminal rows, five DRAW, no skip/not-run/timeout, report 56ebd01adb97b3916fde36f8739602284d951a1fd00d84285bb4ef441fb463a3 unchanged. Fingerprint 573904875fa3641acdc4936c2f116e2594c3366d2abe4eec70841af7d5ee4ac5 over 56 inputs; host_elapsed_guest_ns 5577072000.
  implication: Public-header documentation fingerprint is refreshed from measured evidence. An earlier capture's stale configure-time revision d85a058dd7e1 is preserved but not used as the baseline.

- timestamp: 2026-10-29
  checked: Controlled actual-driver clock wrapper after session.cfg exists, before first readiness operation clock returns.
  found: TERM yields exit 130, stderr journal field invalid, header-only journal, journal-close-failed trace row with JOURNAL_FAILED=1; driver trace rejects TRACE_EVENT. Host is reaped and QUIT sent once. An earlier wrapper gated only on host.pid missed this boundary and passed, showing why ordinary timing repeats are insufficient.
  implication: Reproduces the first hosted failure signature through partial operation initialization. Exact historical interleaving remains unobserved because hosted raw journal/trace rows were not uploaded.

- timestamp: 2026-10-29
  checked: Charged only old Perl sleep startup by 1.2 seconds in the real valid-prefix driver.
  found: Original driver fails DELAY_PUBLICATION_POLL_DEADLINE. Fractional platform sleep reaches all three prefix verdicts with no interpreted sleeps. Independently charging platform poll sleep restores the exact deadline failure at driver+cleanup 2035 ms with host reaped.
  implication: Removing interpreter launch fixes a demonstrated mechanism without increasing the delay+1000 ms child budget. Original hosted per-process timing attribution remains bounded by missing trace evidence.

- timestamp: 2026-10-29
  checked: First Nightly attempt job 123693789780 (attempt lookup via gh API because the connector exposes latest-attempt jobs only and ci_monitor.cjs is absent).
  found: Controls fail in 429.06 s with FAIL interruption first failure was not retained and driver-trace-TRACE_EVENT; readiness and source-root pass.
  implication: Keep the interruption failure separate from the retry's poll sleep failure and retain both original attempts.

- timestamp: 2026-10-29
  checked: Actual latest Nightly session job 123707355846 via GitHub connector.
  found: Exact DELAY_PUBLICATION_POLL_DEADLINE; readiness 44.29 s passes, source-root 21.16 s passes, all six other work jobs succeed. Uploaded LastTest.log contains no per-process sleep trace.
  implication: Hosted interpreter-startup attribution is a hypothesis until controlled reproduction; retained phase acceptance remains historical.

- timestamp: 2026-10-29
  checked: state_session.sh run_child and sleep_ms.
  found: Every nonzero sleep launches perl -MTime::HiRes=sleep; a 50 ms poll gets delay+1000 ms child budget. Native clock/hash repair does not remove this interpreter.
  implication: Test sleep-only startup cost separately from clock/hash and preserve the unchanged budget.

## Resolution

root_cause: Reproduced two compatible mechanisms: per-poll interpreter startup can consume the 1050 ms supervised sleep budget; publishing ACTIVE_OPERATION before its deadline permits TERM to disable journal writing through a partially initialized operation.
fix: Use existing fractional platform sleep under the same delay+1000 ms budget; initialize timing fields before publishing ACTIVE_OPERATION. Add discriminating actual-driver controls and print failed journal/trace evidence in the uploaded CTest log.
verification: Repair f9ae22ecb04df41d6d7394e14b590b47e689c138 passes local CI 695/695 plus all packages, CI 40956561386/1 all 18 jobs, both required session jobs and Nightly 40956561408/1 Intel 3/3. Nightly has six successful work jobs and suite-flake still running at handoff; no job has failed. Later evidence-only publication checks must be observed before merge. Original hosted per-process timing and exact interruption interleaving were not retained, so mechanism reproduction is not direct historical attribution.
files_changed: tests/retroarch/state_session.sh, tests/retroarch/test_state_session.sh, README.md, include/nesturbator.h, tests/accuracy/baseline-metadata.txt, tests/accuracy/measurements.md, debug/state records.
