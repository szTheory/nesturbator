# Phase 16 readiness timing repair evidence

**Recorded:** 2026-10-15
**Repair source:** `e2bed6aeb85b773e44597e8f5d27de94d2dc383f`
**Review:** [16-REVIEW.md](16-REVIEW.md), clean at that exact source commit

## Native failure and scope

The retained native failure is PR #50's macOS Intel `pull_request` CI run [38740783575](https://github.com/szTheory/nesturbator/actions/runs/38740783575), on PR head `30a009838b741a429863e7cb5143526fbaad8ed7`. In `build/phase16-release/macos-intel-failed.log`, the sole failure is `readiness-response-stall`: its complete prefix took 26,632 ms against the unchanged strict `<25000 ms` bound. The preceding delayed-read case passed at 20,728 ms. The exact response, readiness operation interval, one-send/journal assertions, and child/host reaping had passed before the outer prefix-duration assertion failed.

The inherited `failure-diagnosis.json` incorrectly labels the failure as `readiness-response-delay`. The source assertion and native log establish that the stalled-response case failed. No failed native job was rerun, and these observations do not establish an Intel-specific scheduler or host root cause.

## Measurement method and identities

The baseline focused invocation ran the actual stalled-response prefix and failed the newly added coherent-journal accounting assertion on the pre-repair driver. This is RED evidence for the row-accounting defect; it did not reproduce the native `<25000 ms` failure. The baseline source was `c5459a096d066574a2c25b5dd85e9ef599a4de62` (test registration and RED assertion, with the prior driver unchanged). The first repaired-driver run used source `526b1e049b4ddf9fde568acb4389b35a2df2cd97`; the final two-case focused and full-matrix checks used `e2bed6aeb85b773e44597e8f5d27de94d2dc383f`.

All local measurements used the `ci` CMake preset on Darwin arm64 with CMake/CTest 4.4.3, Ninja and Release configuration. The fixture duration is kept separate from driver setup, the readiness operation, later prefix work, cleanup, and the enclosing CTest process. Values below come from the raw JUnit and console output linked at the end of this record. Monotonic journal times are attempt-local; enclosing CTest values include harness setup and, in the full matrix, earlier controls.

| Run | Fixture | Case | Readiness operation | Later prefix | Attempt through cleanup | Driver plus cleanup | Enclosing test |
|---|---:|---|---:|---:|---:|---:|---:|
| Focused RED, old driver | generated three-boundary prefix | response stall, full prefix reached `PREFIX_OK` | 5,087 ms | completed | completed | completed | 12.02 s; failed only because elapsed 5,756 + remaining 24,238 = 29,994 ms, six milliseconds short of the 30,000 ms journal invariant |
| Focused tracer, repaired driver | 774 ms | response stall | 5,073 ms | 5,056 ms | 10,917 ms | 10,970 ms | 11.88 s |
| Focused two-case CTest, final source | 704 ms | delayed | 1,624 ms | 4,931 ms | 7,321 ms | 7,366 ms | 19.38 s for both cases |
| Focused two-case CTest, final source | 704 ms | response stall | 5,106 ms | 5,173 ms | 10,986 ms | 11,039 ms | 19.38 s for both cases |
| Complete session matrix, final source | 5,652 ms | delayed | 1,610 ms | 4,920 ms | 7,224 ms | 7,269 ms | 284.90 s for the matrix |
| Complete session matrix, final source | 5,652 ms | response stall | 5,063 ms | 4,912 ms | 10,681 ms | 10,724 ms | 284.90 s for the matrix |

The focused and complete controls retain the original operation windows (`1200 < delayed < 4500 ms`, `4500 <= stall < 7000 ms`), strict driver `<25000 ms` limit, one readiness send, exact isolated-path reply, full-prefix verdicts, one-outstanding-child supervision, child/host reaping, unchanged attempt budgets and cleanup. No default budget, fake-child sleep, or bound was increased. Local arm64 timings are not evidence that the native Intel host has a particular scheduling cause.

## Bounded source change

The former inactive readiness-row path launched five Perl subprocesses: one for elapsed time, a nested second clock sample for remaining time, and three sanitization filters. The changed `record_row` path performs the row's coherent monotonic accounting and sanitization in one existing Perl invocation. The active operation stop observation, inactive last-operation values, stopped-operation observation, nonnegative remaining time, eleven-column TSV interface, sanitization substitutions, terminal failure reason, child deadlines and cleanup handling remain explicit. This five-to-one count is a source call-path count; it does not claim a separately instrumented native process trace or prove why the Intel runner exceeded its elapsed bound.

The new registered `retroarch.session.readiness` control selects delayed and stalled reads without first executing unrelated controls. The same shared cases remain in the complete `retroarch.session.controls` matrix. Both drive the actual prefix through fixture validation, readiness, startup captures, pause, native anchor, advance, `PREFIX_OK`, and owned-process cleanup. The focused CTest guard is 60 seconds; the existing full-matrix guard remains 900 seconds.

## Verification and review

- The RED focused CTest exited nonzero on the intended journal invariant with a fully completed response-stall prefix; raw evidence: [JUnit](../../../build/phase16-timing/16-09-task1-red-focused-junit.xml), [console](../../../build/phase16-timing/16-09-task1-red-focused-console.log), and ignored classifier evidence [JSON](../../../build/phase16-timing/16-09-task1-red-focused-evidence.json).
- The repaired-driver focused tracer passed; raw [JUnit](../../../build/phase16-timing/16-09-task1-tracer-junit.xml).
- The final focused delayed/stalled CTest passed in 19.38 seconds; raw [JUnit](../../../build/phase16-timing/16-09-task2-green-focused-junit.xml).
- The final complete session control matrix passed in 284.90 seconds, including the existing progression, negative, send-count, isolation, interruption and cleanup controls; raw [JUnit](../../../build/phase16-timing/16-09-task2-green-full-controls-junit.xml) and [console](../../../build/phase16-timing/16-09-task2-green-full-controls-console.log).
- `cmake --workflow --preset ci` completed all four steps, including packaging. CTest reported `100% tests passed out of 649`, total test time 296.60 seconds; the package step generated the library, runner and libretro archives. Full ignored output: [workflow log](../../../build/phase16-timing/16-09-workflow.log).
- Fresh deep review covered all six changed repair source/documentation paths at `e2bed6aeb85b773e44597e8f5d27de94d2dc383f`; it reports zero actionable findings. [Review dispositions](16-REVIEW-DISPOSITION.md) retain WR-01 through WR-06 as fixed with `open: 0`.

The four Plan 16-09 task commits are `c5459a0` (focused RED control), `526b1e0` (single-sample journal row and docs), `ff6f058` (delayed/stalled case coverage), and `e2bed6a` (complete assertions and integrity controls). These are local branch commits only; this plan did not mutate PR or release state.

## Handoff

Plan 16-07 Task 1 can resume by reconciling the already-existing preparation commit `30a0098` and reusing PR #50. It must use normal hooks and require fresh native PR CI on the final pushed head, including perf and pinned RetroArch, before any exact-head merge-consent request. A terminal native CI failure remains a stop condition. This record does not claim a native rerun, root-cause diagnosis, publication, merge consent, EVID-04 closure, or Phase 16 completion. Plan 16-08 still owns verification of the actual released bytes.
