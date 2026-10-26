---
created: 2026-10-26T17:32:52.434Z
title: Make runner signal checks deterministic
area: testing
severity: minor
scope: proposed
files:
  - tests/runner/test_signal.c:37
  - tests/runner/test_signal.c:280
  - tests/runner/test_signal.c:493
  - tests/CMakeLists.txt:233
---

## Problem

The POSIX `runner.signal.*` tests assert a 2000 ms signal-to-exit limit and run
serially. The graceful-stop case includes filesystem flush work within that
window. The 2026-10-26 findings proposed event-order assertions so host scheduling
and storage latency contribute less noise and these tests can run concurrently.
The current harness already establishes readiness before delivering a real signal;
preserve that synchronization and its external-process coverage.

## Solution

Proposed scope for future prioritization. Assert observable ordering for readiness,
signal delivery, frame completion, save persistence and process exit. Keep real
SIGINT/SIGTERM, second-signal, blocked-flush, load-error and benchmark cases, with
bounded outer timeouts to detect hangs. Treat latency as diagnostic output when
it is not the behavior being proved. Review the original RUN-02 stop contract
before changing its two-second assertion, and retain an explicit bounded-stop
oracle where required by that contract.

Confirm scratch-path and process isolation under concurrent and randomized CTest
runs; remove `RUN_SERIAL` after those checks demonstrate independence. Compare
runtime and prove that broken ordering, missing save flushes and a missing second
signal action still fail the tests.

Source: `ci-efficiency-findings-2026-10-26` project memory and current runner test
registration. Severity minor confirmed in the capture session. This proposal does
not add requirements to Phase 20 or the committed v5 scope.
