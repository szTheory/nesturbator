---
phase: 14-runner-process-contract
reviewed: 2026-10-12T00:00:00Z
depth: standard
files_reviewed: 16
files_reviewed_list:
  - README.md
  - runner/CMakeLists.txt
  - runner/exit_status.h
  - runner/main.c
  - runner/state_file.c
  - runner/state_file.h
  - runner/stop.c
  - runner/stop.h
  - runner/stop_internal.h
  - tests/CMakeLists.txt
  - tests/cmake/runner_exit_table.cmake
  - tests/cmake/runner_stop.cmake
  - tests/runner/save_rom.c
  - tests/runner/test_console.c
  - tests/runner/test_signal.c
  - tests/runner/test_stop_win.c
findings:
  critical: 0
  warning: 3
  info: 4
  total: 7
status: issues_found
---

# Phase 14: Code Review Report

**Reviewed:** 2026-10-12
**Depth:** standard
**Files Reviewed:** 16
**Status:** issues_found

## Summary

The graceful-stop design holds up. I traced these paths and found no correctness defect in any of them:

- The POSIX handler does only `signal(sig, SIG_DFL)` and a `sig_atomic_t` store. Both are permitted in a handler (C17 7.14.1.1; POSIX lists `signal` as async-signal-safe).
- The flag is polled once per frame, before the frame starts.
- A signal that lands after the last frame completes exits 0 and prints the audio hash.
- The exit order is stdout flush, save flush, stop line, `report_unmet`, then `stop_done`.
- `status`/`stopped` interact correctly: a flush failure gives exit 1, and a stopped run with an unmet output gives exit 1.
- The Windows handler touches only Interlocked variables. A second CTRL_C or CTRL_BREAK is declined, so it reaches the default handler. The close event is bounded by the cap and grace.
- `--raise-after` is validated against `--frames` and the loaded state's frame.

The remaining issues concern robustness of the handler installation, test timing margins that are likely to flake, and POSIX/Windows parity.

## Warnings

### WR-01: Handlers are installed over an inherited SIG_IGN, and restart semantics depend on `signal()` and a feature-test macro

**File:** `runner/stop.c:1-5, 19-29`
**Issue:**
- `nesturbator_run_stop_install` calls `signal(SIGINT, on_signal)` and `signal(SIGTERM, ...)` unconditionally. A runner started from a non-interactive shell background job (`cmd &`) or under a supervisor that deliberately set `SIGINT` to `SIG_IGN` now stops on a signal it was meant to ignore. The convention is to install only if the current disposition is not `SIG_IGN`. The test helper's own `reset_child_signals` comment acknowledges this inheritance (test_signal.c:182).
- The behavior the README and `runner.signal.second` promise is that a blocked write is restarted and not failed with EINTR. That depends on `signal()` giving BSD/SA_RESTART semantics. The code obtains it on glibc only through `#define _DEFAULT_SOURCE` placed before the first include. That is correct today, but it is fragile: the macro is invisible to anyone reordering includes, and `signal()` semantics are implementation-defined.
- The handler does not save and restore `errno`. If `signal()` ever fails inside the handler, it can clobber the `errno` that `state_file.c:38` tests for `EINTR`.

**Fix:** Use `sigaction`, with `SA_RESTART | SA_RESETHAND` and an empty `sa_mask`, for SIGINT and SIGTERM. `SA_RESETHAND` makes the reset-to-default atomic with delivery and removes the `signal()` call from the handler. Skip installation when `sigaction(sig, NULL, &old)` reports `old.sa_handler == SIG_IGN`. Then the `_DEFAULT_SOURCE` macro can be replaced with `_POSIX_C_SOURCE 200809L`.

```c
static void install_one(int sig)
{
    struct sigaction old, sa;
    if (sigaction(sig, NULL, &old) == 0 && old.sa_handler == SIG_IGN)
        return;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_signal;          /* body: stop_flag = 1 only */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_RESETHAND;
    (void)sigaction(sig, &sa, NULL);
}
```

### WR-02: Test timeouts of 1 s and 2 s are far below the helpers' own deadlines and likely to flake on slow hosted runners

**File:** `tests/CMakeLists.txt:~231` (`NESTURBATOR_SIGNAL_TIMEOUT 2`) and `~262` (`runner.stop.win_console ... TIMEOUT 1`)
**Issue:**
- The helpers carry deadlines of 10 s to start (`READY_DEADLINE_MS 10000`), 5 s to exit, and 20 s for the Windows save (`SAVE_WAIT_MS 20000`). CTest then kills the helper after 2 s (POSIX) or 1 s (Windows). The helpers' deadlines are unreachable, and their fail-safe path never runs.
- The helper's `kill_and_fail` SIGKILL/TerminateProcess cleanup is bypassed on a CTest timeout. The runner it spawned has `--frames 4294967295` and keeps running until something else kills it.
- The values are "twice the slowest hosted run" of one CI run (38322038900). A single sample of a shared-runner distribution is a thin basis: process start on Windows or a cold macOS runner can double on a bad day.
- `runner.signal.second` alone spends at least 500 ms in `still_alive_for`, plus startup, plus term-until-dead polling.
- A one-line `set(NESTURBATOR_SIGNAL_TIMEOUT 2)` with a run-ID comment is also mis-indented inside the `if(NOT WIN32)` block.

**Fix:** Set the CTest `TIMEOUT` above the helper's worst-case internal deadlines (for example 30 s). Keep the 2 s requirement as the helper's own `STOP_LIMIT_MS` assertion, which already exists. A timeout should then mean a hung helper, not a slow runner.

### WR-03: A signal that arrives while a `--movie`/`--hash-*` run is blocked on stdout is silently held until the write returns, but the same applies to the first stop check

**File:** `runner/main.c:1268-1273, 1306-1311, 1336`
**Issue:** The stop flag is polled only at the top of an iteration. Per-frame `printf` of hash lines (always, in movie mode) can block on a full pipe. README documents this for the exit flush, and the tests (`second`, `second_in_flush`) exercise it. The first signal is then delayed with no bound. The documented escape is a second signal, which kills the process without the save flush. For an open-ended launcher this means a stalled consumer turns a graceful stop into either an indefinite wait or a hard kill. This is a documented trade-off (D-06) and not a defect in the code, but the README's launcher advice covers only Windows ("The launcher must read or discard the runner's standard output"). The POSIX section lacks the same sentence.
**Fix:** Add the same drain-stdout requirement to the POSIX paragraph of "Stopping the runner" in README.md. Optionally, `fflush(stdout)` after each frame's output so that a stall shows up at the point the frame finishes instead of at exit.

## Info

### IN-01: SIGHUP is not handled on POSIX, while the Windows console-close event is

**File:** `runner/stop.c:25-29`, README "Stopping the runner"
**Issue:** On Windows, closing the console window triggers the flush. On POSIX the closest events (terminal close, SSH disconnect) deliver SIGHUP. That signal still has its default action and kills the runner without writing the battery save. The README does not mention the asymmetry.
**Fix:** Install the same handler for SIGHUP (add `NESTURBATOR_RUN_STOP_HUP` or reuse the term kind), or state in the README that SIGHUP is not handled.

### IN-02: The EINTR retry loops in `nesturbator_run_state_read` are inconsistent with the other reads and unreachable under SA_RESTART

**File:** `runner/state_file.c:26-43`
**Issue:**
- With restart semantics the retry loops never run. If restart semantics were absent, the ROM read (`main.c:1170`), movie read, and save load would fail with a spurious read error on a stop signal, just as the state read would have.
- The check `ferror(f) != 0 && errno == EINTR` depends on `errno` being set by the failing `read`, with no reset beforehand.
- The loops also cannot be shown to work by the test: `case_load_error` relies on the restart, not the retry.

**Fix:** Either remove the loops and rely on `SA_RESTART` (WR-01), or apply the same retry to every file read in the runner. At minimum, set `errno = 0` before each `fread` so the test cannot see a stale `EINTR`.

### IN-03: Duplicated `parse_kind` bodies and a misleading usage message for a repeated `--raise-after`

**File:** `runner/stop.c:40-53, 149-162`, `runner/main.c:277-278, 407-410`
**Issue:**
- The POSIX and Windows `nesturbator_run_stop_parse_kind` implementations differ only in one name and its constant. The common "int" branch is duplicated.
- `parse_raise_after` returns 0 for a second `--raise-after`, and `parse_options` then reports "needs N[:SIG[:K]]". That is misleading, since the value was well-formed. The test `runner.usage.raise_after.twice` only checks that the message contains `--raise-after`.

**Fix:** Share the "int" handling and give the duplicate case its own message ("--raise-after may be specified once").

### IN-04: `nesturbator_run_stop_done` is a no-op on POSIX and `close_cap_ms`/`close_grace_ms` are plain statics read by the handler thread

**File:** `runner/stop.c:36-38, 77-78, 121-125`
**Issue:**
- The POSIX no-op is harmless.
- On Windows, `nesturbator_run_stop_set_close_wait` writes two non-volatile, non-interlocked `DWORD`s that the handler thread reads. The writes are test-only and happen before the thread starts, so there is no live race. Still, `stop_internal.h` does not say that the function must not be called while a handler is running.

**Fix:** Add one line to `stop_internal.h` saying the setter must be called before any handler thread starts.

---

_Reviewed: 2026-10-12_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_
