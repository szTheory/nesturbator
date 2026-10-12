# Phase 14: Runner process contract - Research

**Researched:** 2026-10-12
**Domain:** C17 signal and Windows console-control handling in a headless CLI; exit-status contract; CTest process tests
**Confidence:** HIGH for the code map, POSIX behaviour (measured in this session on macOS and in a Linux/glibc 2.41 container) and the Microsoft API facts; MEDIUM for hosted-Windows console delivery (no authoritative report either way; D-16 is the empirical gate) and for fsync cost on hosted macOS runners (not measured)

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions and Claude's Discretion

## Implementation Decisions

The owner selected all four areas and asked for a research fan-out per area (every stakeholder
lens, primary sources, adversarial pass) and one coherent recommendation per area, followed
automatically. The decisions below are those recommendations, reconciled with each other.

Carried forward (v3 research, not re-decided): flag-only handlers; C17 `signal()` on POSIX with
`signal(sig, SIG_DFL)` first in the handler, no `sigaction`, threads or `signalfd`; crash signals
unhandled; exit 0 after a successful flush (128+n is not used); the flush runs in the main loop
between frames, at the point `--save-interval` uses, through `runner/save.c`.

### Stop mechanism (RUN-02)
- **D-01:** A new runner module `runner/stop.c` + `runner/stop.h` with `stop_install()`,
  `stop_requested()` (polled at the top of each loop iteration, before frame n runs) and
  `stop_done()` (called by the main thread after the exit flush). Windows code sits behind
  `#ifdef _WIN32` with `WIN32_LEAN_AND_MEAN`; the file header says it is runner-only (the core
  still links only the C memory functions; `runner/save.c` already uses kernel32).
- **D-02:** POSIX: `signal(SIGINT, h)` and `signal(SIGTERM, h)`; `h` stores 1 to a
  `static volatile sig_atomic_t` after `signal(sig, SIG_DFL)`.
- **D-03:** Windows: `SetConsoleCtrlHandler(handler, TRUE)` only; no CRT `signal()` on Windows.
  State is `static volatile LONG` written with `InterlockedExchange`/`InterlockedCompareExchange`
  (the handler runs on a system thread; `sig_atomic_t` is not a cross-thread guarantee, and MSVC
  ARM64 defaults to `/volatile:iso`).
  - CTRL_C_EVENT and CTRL_BREAK_EVENT: the first sets the flag and returns TRUE; a second returns
    FALSE, so the default handler ends the process at once.
  - CTRL_CLOSE_EVENT: set the flag, then wait in a `Sleep(10)` loop until `stop_done()` or about
    4500 ms (the system terminates the process 5000 ms after CLOSE, SPI_GETHUNGAPPTIMEOUT). After
    `stop_done()` keep waiting (bounded, about 1 s) so the main thread's own exit sets the status.
  - Every other event (LOGOFF, SHUTDOWN): return FALSE, not handled.
  - Never call `AttachConsole`, `AllocConsole` or `FreeConsole` in the runner (they reset the
    handler list). Ignore `SetConsoleCtrlHandler`'s result; a runner without a console gets no
    events.
- **D-04:** Handlers are installed after command-line validation, on the normal run path only,
  so usage errors stay exit 2. `--accuracycoin-page` installs none (default action; no battery
  file); the header comment says so.
- **D-05:** A signal before the loop (during cartridge, battery or `--load-state` load) is seen at
  the first loop check; loads are never aborted midway. A load error (4 or 5) or an earlier
  failure takes precedence over the stop. Pre-loop outputs for the start frame still happen.
- **D-06:** A signal during a `--state-at-frame` write or the exit flush is only a flag, so the
  write completes. A second signal there kills at once; the real file is never torn (temp and
  rename) and a stale `.tmp` is removed by the next write (`runner/save.c` already does this).
  The README says a second signal at any time, including during the flush, ends the runner.

### Stop output and exit status (RUN-02 within the RUN-03 table)
- **D-07:** After the loop ends on the flag: `fflush(stdout)` (complete lines only; hash lines are
  printed between frames and the handler never touches stdio), then the existing
  `save_if_changed` exit flush (written only if the generation changed; nothing without
  `--save-dir`), then `stop_done()`.
- **D-08:** A stopped run exits 0 after a good flush unless something requested was not
  produced, and then it exits 1. Unmet means a `--hash-frame N`, `--hash-state N`,
  `--state-at-frame N:FILE` or `--dump-frame N:FILE` whose frame was not reached, `--hash-audio`
  (any stop), or a `--movie` that did not reach its end. `--frames N` not reached is not unmet:
  a signal ends an open-ended run cleanly. A failed flush is 1. No new status; 3 and 77 stay
  reserved. Playstead's command (`--frames` large, `--save-dir`, no hash or dump options) gets 0.
  This refines RUN-02's "exits 0" so a stop is never mistaken for a completed check; the README
  states it. — **Reversibility:** costly — launchers and scripts read this contract once it ships
  in a release.
- **D-09:** On a stopped run `--hash-audio` prints nothing (a hash over a truncated run would look
  like a full one).
- **D-10:** stderr on stop, always: `nesturbator-run: stopped by signal after frame N` (N = last
  completed frame; the loaded state's frame, or 0, if none ran). When D-08 gives 1 for an unmet
  output, one more line naming the first one in command-line order, e.g.
  `nesturbator-run: not reached: --hash-frame 600` (same shape for `--hash-state N`,
  `--state-at-frame N:FILE`, `--dump-frame N:FILE`, `--hash-audio`, `--movie FILE`). Nothing is
  added to stdout. On a CLOSE-initiated stop these lines are best-effort (Microsoft: console
  functions may fail during CLOSE).

### Test seam and signal tests (RUN-02, criteria 1 and 2)
- **D-11:** A documented test-aid option in the shipped runner: `--raise-after N[:SIG[:K]]`.
  After frame N's outputs and any `--save-interval` flush, and before the next flag check, the
  runner signals itself K times (K = 1 or 2, default 1); N = 0 means before the first frame.
  SIG is `int` or `term` on POSIX (`raise()`, default `term`), and `int` or `break` on Windows,
  where the option calls the console handler routine directly with CTRL_C_EVENT or
  CTRL_BREAK_EVENT (and, when it returns FALSE, ends the process as the default handler would).
  Other spellings, a repeat, or `--accuracycoin-page` give exit 2. It is listed in `--help` and
  the README as a test aid that signals the runner's own process.
- **D-12:** Frame-exact test on all six platforms, with the battery fixture:
  `--frames 4294967295 --hash-frame N --hash-frame N+1 --raise-after N --save-dir A` against
  `--frames N --hash-frame N --save-dir B`. Asserts exit 0, the two stdouts equal (so no frame
  N+1 line), the `.sav` files equal, no `.tmp`, and the stop line. Repeated for `int` (and
  `break` on Windows).
- **D-13:** The other in-process stop tests, all on six platforms:
  - Flush failure gives 1: `--save-dir` missing, and a directory at `battery.sav.tmp` (the
    existing `unwritable` and `dir_at_tmp` shapes; no chmod).
  - `--raise-after 0`: exit 0, empty stdout, a seed `.sav` unchanged.
  - Unmet outputs give 1 and the `not reached` line: `--hash-frame` beyond the stop,
    `--hash-audio`, and `--movie` not at its end.
  - No `--save-dir`: nothing written, exit 0.
  - K = 2: killed (POSIX death by SIGTERM or SIGINT; on Windows any status but 0 and 1), no stop
    line, and the seed `.sav` whole and unchanged.
- **D-14:** Windows handler unit test (no console needed): call the handler for CTRL_C twice
  (TRUE, then FALSE); run it for CTRL_CLOSE_EVENT on a second thread and assert it returns only
  after `stop_done()` and within the cap; cover cap expiry with a short cap exposed through an
  internal header. This is the only automated check of CLOSE, which no API can generate.
- **D-15:** External signal test, Linux and macOS (`if(NOT WIN32)`), a C helper
  `tests/runner/test_signal.c` (fork, exec, pipes, `kill`, `waitpid`; ideas from
  `tests/test_process.h`):
  - Start `--frames 4294967295 --save-dir W --save-interval 1` on the battery fixture. Wait on a
    condition, not a sleep: poll for the `.sav` at full length.
  - SIGTERM, timed from `kill` with `CLOCK_MONOTONIC`. Assert exit 0 within 2 s, the `.sav` whole
    (full length, expected content), and no `.tmp`. Same again with SIGINT.
  - Second signal: a `--movie` run whose stdout pipe is never read stalls in `write()` once the
    pipe is full. The first SIGTERM leaves it alive (bounded `WNOHANG` poll); the second ends it
    by SIGTERM within a bounded wait. Send the second only after the first was delivered.
  - Every wait has a hard deadline and SIGKILLs the child on failure, so a hung runner cannot hang
    CI. CTest TIMEOUT is twice the slowest measured run (ENGINEERING section 5).
- **D-16:** Real Windows console delivery: the planner adds one end-to-end Ctrl+Break test. The
  runner starts with `CREATE_NEW_CONSOLE`; a sender does `FreeConsole`, `AttachConsole(pid)`,
  registers a TRUE handler, then `GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, 0)`. Expect exit 0,
  the whole `.sav` and no `.tmp`. If it fails on a hosted Windows leg, the failure carries the
  `GetLastError` text. Then, with that CI evidence in the SUMMARY, the planner either keeps it or
  removes it, never skips silently; D-11 to D-14 still cover Windows. Never send CTRL_C_EVENT to
  a process group (Microsoft: unsupported).
- **D-17:** Measured fsync cost (32 KiB write with F_FULLFSYNC, local SSD): 14 ms max over 50 runs.
  D-15's 2 s assertion on hosted runners is the latency check; no extra job.

### Exit-status table (RUN-03)
- **D-18:** `runner/exit_status.h` (runner-internal, not installed, not in `nesturbator.h`) holds
  one X-macro table `RUNNER_EXIT_TABLE(X)`:
  - `X(RUNNER_EXIT_DONE, 0, "done")`
  - `X(RUNNER_EXIT_FAILURE, 1, "failure")`
  - `X(RUNNER_EXIT_USAGE, 2, "usage error")`
  - `X(RUNNER_EXIT_SAVE_SIZE, 4, ".sav size mismatch")`
  - `X(RUNNER_EXIT_STATE_REJECTED, 5, "state rejected")`

  It also holds the enum generated from that table and `RUNNER_EXIT_RESERVED_TEXT "3 and 77 are
  reserved"`. 3 and 77 get no enumerators, so nothing can return them. Cite LIBRETRO-AND-RUNNER
  section 5. The planner may refine the row text, which is then checked everywhere.
- **D-19:** Add a real `--help`: the usage text, including the exit table built from the X-macro,
  goes to stdout and the runner exits 0. A bad command line still prints the same text to stderr
  with exit 2. Criterion 3 names `--help` and this is the GNU convention.
- **D-20:** The README gets one `### Exit status` section, with a table of 0, 1, 2, 4 and 5 and the
  reserved sentence. The row for 1 mentions a failed flush and a stopped run that missed a
  requested output (D-08). The scattered "Exit status:" sentences (near README lines 1185 and
  1221) become links to it. The `runner/main.c` header comment, which omits 5 today, points to
  `exit_status.h`.
- **D-21:** `runner/main.c` uses the named constants for every value that becomes the process
  status. Boolean helpers such as `parse_count_n` stay 0/1; a one-line comment in
  `exit_status.h` notes this. No source-scanning guard for bare literals: the behaviour tests are
  the guard.
- **D-22:** `tests/cmake/runner_exit_table.cmake` (test `runner.exit.table`, same style as
  `tests/cmake/status_table.cmake`). It:
  - parses the header rows and the README `### Exit status` rows;
  - runs `nesturbator-run --help`;
  - asserts the number-to-text pairs are identical in all three, the set is exactly
    {0, 1, 2, 4, 5}, and 3 and 77 appear only in the reserved sentence.
- **D-23:** Each status has its own test, all with the CTest label `exit_status`, so that
  `ctest -L exit_status` lists the full set:
  - new: `runner.exit.done` (exit 0), `runner.exit.failure` (missing ROM, exit 1),
    `runner.exit.usage` (no arguments, exit 2), `runner.exit.help` (exit 0, table on stdout);
  - existing, relabelled: `runner.save.mismatch` (4), `runner.state.reject_game` (5),
    `runner.save.unwritable` (1);
  - the D-12 and D-13 stop tests for 0 and 1.

### Documentation (RUN-02 criterion 4, rule 6)
- **D-24:** The README process-contract section says:
  - On a stop the runner finishes the frame, flushes, and exits 0 or 1 (D-08). A second signal
    ends it at once.
  - On Windows, Ctrl+C, Ctrl+Break and console close are handled. A launcher stops the runner
    with `GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT)` after starting it with
    `CREATE_NEW_PROCESS_GROUP`.
  - No event reaches a runner without a console (`DETACHED_PROCESS`, `CREATE_NO_WINDOW`), and
    logoff and shutdown are not handled.
  - A forced termination (`TerminateProcess`, `taskkill /F`, power loss) cannot be handled. The
    last committed `.sav` survives it.
  - SIGTERM is not a Windows mechanism.
  - `--raise-after` is a test aid.

  The same facts go in the header comments of `runner/main.c` and `runner/stop.h`, citing POSIX
  `signal` and the Microsoft Learn `HandlerRoutine` and `SetConsoleCtrlHandler` pages.

### Claude's Discretion
- How the work is split into plans, the exact wording of the X-macro row text and README prose,
  and the CTest names beyond those given above.


### Deferred Ideas (OUT OF SCOPE)

- SIGUSR1 "flush now" (POSIX only); it is not in RUN-02.
- `--expect` (exit 3) and `--capabilities` (exit 77) (RUN-04).
- CTRL_LOGOFF and CTRL_SHUTDOWN handling (service wrappers). Microsoft says only services receive them.
- A one-off CI step that prints the exit-flush time per hosted runner (D-15 already asserts the bound).

</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| RUN-02 | On SIGTERM or SIGINT the runner finishes the current frame, flushes the battery save, and exits 0, or 1 if the flush failed. The signal handler only sets a flag. A second signal kills it immediately. On Windows, Ctrl+Break and console close are handled the same way, and the README says that a forced termination cannot be handled. | Edit-point map (Section "Runner code map"); glibc `signal()` semantics finding (Pitfall 1); Windows handler model; test inventory |
| RUN-03 | Exit statuses are one table, in one header, `--help` and the README: 0 done, 1 failure, 2 usage, 4 battery file wrong length, 5 state rejected | X-macro header + `cmake -P` comparer recipe; every-exit-site table; `--help` parse point |
</phase_requirements>

## Summary

The design is settled, so this research is about edit points, platform facts that could break the settled design, and how to test it. The runner is one 1172-line `runner/main.c` with a single main loop (lines 1097-1155) and a single exit flush (lines 1157-1159); `runner/save.c` already writes through temp-file, exclusive create, `F_FULLFSYNC`/`fsync`/`_commit` and rename/`MoveFileExA(..., MOVEFILE_WRITE_THROUGH)`. There is no `--help`, no signal code, and no named exit constants; `usage()` returns the literal 2. Tests run the runner through three existing harness shapes (`nesturbator_runner_test` -> `expect_output.cmake`; `runner_save.cmake`/`runner_state.cmake` CASE scripts; C helper executables). All of those can host the new tests without new machinery.

**Four findings change or constrain the plan** and the planner must act on them:

1. **glibc `signal()` is System V under the project's strict `-std=c17`.** `CMAKE_C_EXTENSIONS OFF` makes GCC define `__STRICT_ANSI__`, so glibc gives `signal()` SysV semantics (no `SA_RESTART`). Measured below: a blocked `write()` returns `EINTR`, and the same hazard applies to the exit flush's `fwrite`/`fclose` if a first signal lands mid-flush (D-06 requires the write to complete). One line fixes it without `sigaction`: `#define _DEFAULT_SOURCE` at the top of `runner/stop.c`. macOS and musl are BSD-style already. See Pitfall 1.
2. **D-12 conflicts with D-08 as written.** D-12 asserts exit 0 with `--hash-frame N+1` on the command line, but D-08 makes an unreached `--hash-frame` an unmet output and exit 1. The happy-path frame-exact test must drop `--hash-frame N+1`; the N+1 variant is the D-13 "unmet" case (exit 1, one stdout line). See Pitfall 2.
3. **The D-15 external test with `--save-interval 1` leaves nothing to flush after the signal** (the battery fixture's span stops changing after frame 1), so it proves "whole file, no `.tmp`, exit 0 in 2 s" but not the post-signal fsync cost. A second variant (no interval, readiness from a `--hash-frame 1` line on a stdout pipe) makes the exit flush real. See Pitfall 3.
4. **Windows `CREATE_NEW_PROCESS_GROUP` is ignored with `CREATE_NEW_CONSOLE`** (Microsoft), and `GenerateConsoleCtrlEvent` only reaches processes on the caller's console. D-16's design (new console, sender `AttachConsole`, group id 0) is the one that is consistent with the documentation; D-24's launcher recipe (`CREATE_NEW_PROCESS_GROUP` plus `GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, pid)`) additionally requires the launcher to share the runner's console. The README must say so.

**Primary recommendation:** Build in this order: `exit_status.h` + `--help` + named constants (pure refactor, locks RUN-03 tests), then `stop.c` + loop integration + `--raise-after` + in-process stop tests, then the POSIX external helper, then the Windows handler unit test and the Windows end-to-end test, then README/header comments. Put `#define _DEFAULT_SOURCE` first in `stop.c`.

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| Stop request capture (signal / console event) | Runner process (`runner/stop.c`) | OS (kernel signal delivery, console subsystem) | Flag-only handler; the core library must stay free of OS code (CLAUDE.md rule 1) |
| Finish frame, flush, choose status | Runner main loop (`runner/main.c`) | `runner/save.c` | Flush runs between frames through the existing `save_if_changed` |
| Exit-status table | Runner header (`runner/exit_status.h`) | README, `--help` text, CTest comparer | One definition, two renderings, one checking test |
| Signal delivery tests | Test helpers + CMake scripts | CI matrix (six runners) | Deterministic in-process seam (`--raise-after`) everywhere; real external signals on Linux/macOS; real console event on Windows as an evidence-gated test |
| Core library (`src/`, `include/`) | Unchanged | | Nothing in this phase touches it |

## Standard Stack

No new packages. Everything is the C17 standard library, POSIX (`signal`, `raise`; test helpers add `fork`, `execv`, `pipe`, `kill`, `waitpid`, `clock_gettime`, `nanosleep`, `ioctl FIONREAD`), and kernel32 on Windows (`SetConsoleCtrlHandler`, `InterlockedExchange`, `Sleep`, `GetTickCount64`; test helper adds `CreateProcessA`, `AttachConsole`, `FreeConsole`, `GenerateConsoleCtrlEvent`). CMake 3.25+ is already required; local CMake is 4.4.3, CI uses 3.31.x [VERIFIED: `cmake --version` in this session; container run printed 3.31.6].

### Core
| Component | Version | Purpose | Why Standard |
|-----------|---------|---------|--------------|
| C `signal()` + `volatile sig_atomic_t` | C17 | POSIX stop flag | Locked by CONTEXT (no `sigaction`) |
| `SetConsoleCtrlHandler` | kernel32 | Windows stop flag | Locked by CONTEXT; no CRT `signal()` on Windows |
| X-macro table | C17 | One exit-status definition | Locked (D-18) |

### Alternatives Considered
None open. CONTEXT locks flag-only handlers, `signal()` rather than `sigaction`, and X-macro generation.

**Installation:** none.

## Package Legitimacy Audit

No external packages are installed or recommended in this phase. Audit not applicable. **Packages removed due to [SLOP] verdict:** none. **Packages flagged [SUS]:** none.

## Runner code map (item 1)

All line numbers are from `runner/main.c` at HEAD ef34ec4, read this session. [VERIFIED: runner/main.c]

### Structure
| Concern | Where | Notes |
|---------|-------|-------|
| Header comment (omits status 5) | main.c:1-35, exit sentence at 33-34 | D-20: point it to `exit_status.h`; add stop contract text |
| Usage text | `static int usage(const char *why)` main.c:54-76 | Prints `nesturbator-run: <why>` then the usage block to **stderr**, `return 2;` |
| Option struct | `typedef struct options` main.c:186-207 | Has `show_version`; add `show_help`, `raise_after` fields |
| Parser | `parse_options` main.c:221-346 | `--version` handled in-loop at 240-243 (returns 0 with `show_version = 1`, so options before it are validated and options after it are not). Unknown option falls to 319. Value-taking options read `value = argv[i++]` at 250, so a **new standalone switch (`--help`) must be tested before line 250**, like `--hash-audio` at 244 |
| Frame-range checks | `check_frame_limits` 866-888, `check_ranges` 892-921 | Both `return usage(why)` (2). Add `--raise-after N` bound check here |
| Battery state | `battery_save` 349-355; `save_if_changed` 361-374 | Returns 0 or 1; leaves the generation unrecorded on failure so the exit flush retries |
| State helpers | `load_state_file` 776-806 (returns 0, 1, or **5** at 796), `print_state_hashes` 810-834, `write_states` 839-861 | |
| AccuracyCoin path | `run_accuracycoin` 574-717 | Returns 1/0; no battery file; D-04: no handlers on this path |
| `main` | 923-1172 | Version exit 945-954; movie read 957-970; `check_frame_limits` 971; `nesturbator_create` 982; ROM + battery load 989-1053; `--load-state` 1056-1065; `check_ranges` 1066-1073; accuracycoin 1082-1091; pre-loop outputs 1094-1096; **loop 1097-1155**; **exit flush 1157-1159**; audio hash print 1160-1161; cleanup 1163-1166; final `fflush(stdout)` 1167-1171 |
| Atomic writer | `runner/save.c` `nesturbator_run_file_replace` 117-180, `nesturbator_run_save_flush` 182-189 | Reused for `.sav`, `--state-at-frame`; no change needed |

### Every status-producing site (D-21 edit list)
| Line(s) | Today | Meaning | Constant |
|---------|-------|---------|----------|
| 75 | `return 2;` in `usage` | usage error | `RUNNER_EXIT_USAGE` |
| 233 | `return 1;` (parse_options OOM) | failure | `RUNNER_EXIT_FAILURE` |
| 242 | `return 0;` after `show_version` | parse ok | `RUNNER_EXIT_DONE` |
| 345 | `return 0;` (options valid) | parse ok | `RUNNER_EXIT_DONE` |
| 584-710 | `return 1;` x many in `run_accuracycoin`, `return 0;` 716 | failure / done | Constants (this function returns a process status) |
| 796 | `return 5;` in `load_state_file` | state rejected | `RUNNER_EXIT_STATE_REJECTED`; the 1s at 784/789 are failure |
| 875,884,898,907,917 | `return usage(why);` | 2 | unchanged (flows from `usage`) |
| 943 | `return status;` after parse | propagates 1 or 2 | |
| 953 | `return fflush(stdout) == 0 ? 0 : 1;` | version | constants |
| 961, 987, 1001, 1011, 1021, 1047 | `return 1;` | movie unreadable, create failed, ROM unreadable/rejected, OOM | `RUNNER_EXIT_FAILURE` |
| 967 | `return 2;` | `--frames` != movie count | `RUNNER_EXIT_USAGE` |
| 1039 | `return ls == NESTURBATOR_RUN_SAVE_MISMATCH ? 4 : 1;` | **only place status 4 is produced** | `RUNNER_EXIT_SAVE_SIZE` / `RUNNER_EXIT_FAILURE` |
| 1063, 1072, 1090, 1171 | `return status;` | propagated | |
| 1096, 1115, 1125, 1138, 1145, 1150, 1154, 1159 | `status = 1;` | loop and flush failures | `RUNNER_EXIT_FAILURE` |
| 1169 | `return 1;` after failed final `fflush(stdout)` | failure | `RUNNER_EXIT_FAILURE` |

Boolean helpers (`parse_count_n`, `parse_frame_index`, `listed`, `accuracy_*`, `describe_rejection`) return 0/1 as truth values and must stay literal (D-21). `runner/save.c` and `runner/state_file.c` return 0/1 as success/failure; they never reach the process status directly, so they stay as they are. No `exit()` call exists anywhere under `runner/`. [VERIFIED: grep `exit(` over runner/*.c, no match]

### Where the stop logic lands
- **Loop head** (main.c:1097): `for (uint64_t n = start_frame + 1u; n <= opt.frames && status == 0; n++)` becomes also `&& !nesturbator_run_stop_requested()`. Track `done_frame` (last completed frame; start value `start_frame`) for the D-10 line. The flag is read **at the top of each iteration**, so a flag raised by `--raise-after` at the end of body N is seen before frame N+1 runs (D-11).
- **`--raise-after` fire point:** after the `save_interval` block (line 1154) at the end of the body, `if (n == raise_n)`. For `N = 0`, fire after the pre-loop outputs (line 1096) and before the `for`. A load-state run whose `N < start_frame` never fires: validate in `check_ranges` (usage 2) or define as pre-loop fire; pick one and test it (Open Question 2).
- **Install point:** after `check_frame_limits` succeeds (line 976) and only when `opt.accuracy_page == 0u`. That covers D-05 (signal during cartridge/battery/state loads is just a flag seen at the first loop check; load errors return their own status first because the early returns never look at the flag) and D-04.
- **Exit sequence** (D-07): after the loop, if the flag was observed: `fflush(stdout)`, existing `save_if_changed` flush (line 1158), then `stop_done()`. Gate line 1160-1161 (`print_audio_hash`) on "not stopped" (D-09). Print the D-10 stderr lines **before** `stop_done()` and before cleanup so a CLOSE-initiated stop prints within the window.
- **Unmet outputs (D-08/D-10):** options keep six separate arrays (`hash_frames`, `dump_frames`, `state_frames`, `hash_states`, plus flags), so "first in command-line order" needs an argv index stored beside each entry (add parallel `uint32_t *..._arg` arrays or a single request list `{kind, n, path, argi}`), then take the minimum argi over entries with `n > done_frame` (for `--movie`: `done_frame < opt.frames`; for `--hash-audio`: always on a stop). `--state-at-frame N` and `--hash-state N` with `N == start_frame` are produced pre-loop (lines 1094-1095) and are never unmet.

### Existing test harness (what a new test plugs into)
All in `tests/CMakeLists.txt` (line numbers HEAD) [VERIFIED: tests/CMakeLists.txt]:
- `nesturbator_runner_test(NAME ... ARGS ... EXIT n [IGNORE_STDOUT] [EXPECT lines...] [EXPECT_ERR text] [CLEAN_DIR d])` (lines 8-24) wraps `cmake/expect_output.cmake`, which compares exact stdout (CR stripped), exit status, and one stderr substring. **It takes no LABELS and no TIMEOUT**; add `set_tests_properties(<name> PROPERTIES LABELS exit_status)` after each call, or extend the function with a `LABELS` one-value keyword.
- CASE-script pattern: `runner_save_case(case)` (lines 148-158) and `runner_state_case(case)` register `runner.save.<case>` / `runner.state.<case>` running `cmake -DRUNNER=... -DFIXTURE=... -DWORK=<per-case dir> -DCASE=... -P cmake/runner_save.cmake`, `FIXTURES_REQUIRED runner_save TIMEOUT 30|60`. Safe under `ctest -j` because each case has its own WORK dir.
- Fixtures: `runner.save_rom` (`tests/runner/save_rom.c`) writes `battery.nes`, `plain.nes`, `jam.nes`, `seed.sav`, `short.sav` into `${CMAKE_CURRENT_BINARY_DIR}/save_fixture` under fixture `runner_save`. `battery.nes` stores `SAVE` at `$6000-$6003`, copies `$6100` to `$6004`, then loops (`JMP *`), so the span changes in frame 1 and never again. A fresh `.sav` begins `5341564500`; with `seed.sav` byte 4 becomes `42` (`runner_save.cmake` roundtrip/fresh cases).
- C helper executables register as `add_executable(runner.x runner/test_x.c ${PROJECT_SOURCE_DIR}/runner/save.c)` + `target_include_directories(... ${PROJECT_SOURCE_DIR}/runner)` + `nesturbator_warnings(...)` + `$<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>` + `add_test` (lines 246-274). POSIX-only ones use `if(UNIX)` (line 268) and tests that cannot run somewhere are **not registered** there: `policy.no-skip` fails on `SKIP_RETURN_CODE`, `SKIP_REGULAR_EXPRESSION` or `DISABLED` [VERIFIED: tests/cmake/skip_policy.cmake:16-31]. So: `if(NOT WIN32)` for the external signal test, `if(WIN32)` for the console tests, never a skip.
- `check.h`: `CHECK`, `CHECK_EQ_U64`, `CHECK_DONE()`; helpers end `main` with `CHECK_DONE()`.
- `tests/test_process.h` runs a child with `fork`/`execv`/`waitpid(…, 0)` or `CreateProcessA` + `WaitForSingleObject(INFINITE)`: **ideas only**. It has no deadline, no pipe, no signal, so `test_signal.c` needs its own spawn code; `test_process.h` would hang CI on a stuck runner.
- Existing exit-status tests: `runner.save.mismatch` (4) via `runner_save_case(mismatch)`; `runner.state.reject_game` (5) via `runner_state_case(reject_game)`; `runner.save.unwritable` (1); `runner.usage.noargs` (2, line 63-64); `runner.dump.unwritable` (1). A passing run is `runner.nohash` (exit 0, line 41).
- Version precedent for `--help` placement: `runner_version.cmake` and `runner.usage.version_after_bad` (`--bogus --version` is exit 2). Mirror that: `--help` handled where the loop reaches it.
- `tests/cmake/script_policy.cmake` requires the exact line `cmake_minimum_required(VERSION 3.25)` in **every** `tests/cmake/*.cmake` [VERIFIED: script_policy.cmake:16-21]. New scripts must have it.
- Presets: `ci`/`ci-msvc`/`asan`/`dev` run **all** registered tests (`execution.jobs: 4`); `nofp` and `hygiene` filter by label [VERIFIED: CMakePresets.json testPresets]. Adding label `exit_status` does not exclude a test from `ci`/`asan`.

## Windows console-event delivery on hosted runners (item 2a)

### What Microsoft documents [CITED]
- `GenerateConsoleCtrlEvent` "Sends a specified signal to a console process group that shares the console associated with the calling process." `CTRL_C_EVENT`: "This signal cannot be limited to a specific process group. If dwProcessGroupId is nonzero, this function will succeed, but the CTRL+C signal will not be received by processes within the specified process group." Group id: "Only those processes in the group that share the same console as the calling process receive the signal. ... if a process in the group creates a new console, that process does not receive the signal." "If this parameter is zero, the signal is generated in all processes that share the console of the calling process." (learn.microsoft.com/en-us/windows/console/generateconsolectrlevent)
- `CREATE_NEW_PROCESS_GROUP`: "If this flag is specified, CTRL+C signals will be disabled for all processes within the new process group. This flag is ignored if specified with CREATE_NEW_CONSOLE." `DETACHED_PROCESS`: no inherited console; `CREATE_NO_WINDOW`: "the console handle for the application is not set." (learn.microsoft.com/en-us/windows/win32/procthread/process-creation-flags)
- `SetConsoleCtrlHandler`: "Calling AttachConsole, AllocConsole, or FreeConsole will reset the table of control handlers in the client process to its initial state. Handlers must be registered again when the attached console session changes." Also: "Console functions, or any C run-time functions that call console functions, may not work reliably during processing of any of the three signals [CLOSE, LOGOFF, SHUTDOWN]." (…/setconsolectrlhandler)
- `HandlerRoutine`: "the system creates a new thread in the process to execute the function"; "it is possible that the handler function will be terminated by another thread in the process. Be sure to synchronize"; handlers run "last-registered, first-called" until one returns TRUE; default handler "calls ExitProcess"; `CTRL_CLOSE_EVENT` timeout "system parameter SPI_GETHUNGAPPTIMEOUT, 5000ms"; "CTRL_C, CTRL_BREAK | any | no timeout"; LOGOFF/SHUTDOWN "received only by services". (…/handlerroutine)
- CRT `signal`: "By default, signal terminates the calling program with exit code 3, regardless of the value of sig" and "SIGINT is not supported for any Win32 application"; "The SIGILL and SIGTERM signals aren't generated under Windows." This is why the runner must not use CRT `signal`/`raise` on Windows: a default-disposition `raise(SIGTERM)` would exit **3**, a status this phase reserves. (learn.microsoft.com/en-us/cpp/c-runtime-library/reference/signal)
- MSVC `volatile`: "If the compiler targets ARM (except ARM64EC), this is the default interpretation [/volatile:iso]"; "We strongly recommend that you use /volatile:iso along with explicit synchronization primitives ... for memory that is shared across threads." Supports D-03's `InterlockedExchange` on a `volatile LONG`. (learn.microsoft.com/en-us/cpp/build/reference/volatile-volatile-keyword-interpretation). The MS page's ARM64 wording lists "ARM (except ARM64EC)"; treat ARM64 as /volatile:iso, which is the conservative reading either way.

### Implications for the settled design
1. **D-16's shape is the documentation-consistent one.** Runner started with `CREATE_NEW_CONSOLE` has its own console; `CREATE_NEW_PROCESS_GROUP` would be ignored there anyway. The sender must be on that console: `FreeConsole()`, `AttachConsole(runner_pid)`, **then** `SetConsoleCtrlHandler(TRUE_handler, TRUE)` (AttachConsole resets the table, so registering earlier is lost), then `GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, 0)`. The sender is itself on that console, so it receives the event too; its handler must return TRUE or it dies.
2. **D-24's launcher recipe works only when launcher and runner share a console** (launcher is a console program whose console the runner inherits, runner created with `CREATE_NEW_PROCESS_GROUP`, event aimed at the runner's pid). A GUI or service launcher has no console to share; it must `AttachConsole` to a runner started with its own console (the D-16 pattern) or use `CREATE_NEW_CONSOLE`. The README should state the two preconditions in one sentence each rather than promise the bare recipe. (This refines, does not reverse, D-24.)
3. **Handler-install race.** gitlab-runner's `TestKiller` CTRL_BREAK subtest was quarantined for intermittently exiting `0xc000013a` (STATUS_CONTROL_C_EXIT) instead of the expected code: the test sent the event after a fixed one-second sleep, and on a slow Windows CI runner the default handler can run before the target installs its handler. [CITED: gitlab.com/gitlab-org/gitlab-runner/-/merge_requests/7075] Mitigation here is free: the test waits for the `.sav` at full length (written after frame 1 with `--save-interval 1`), and handler installation precedes the first frame, so by then the handler is installed. Do **not** use a sleep.
4. **Default-handler exit status is `0xC000013A`.** Same source. For `--raise-after` on Windows, when the handler returns FALSE, end the process with `ExitProcess(0xC000013Au)` (define the constant locally; `STATUS_CONTROL_C_EXIT` is in `ntstatus.h`, which conflicts with `windows.h`'s `WIN32_NO_STATUS` handling). `runner.stop.double` on Windows asserts "not 0 and not 1" per D-13.
5. **Known failure modes of the end-to-end test (what to print on failure):** `AttachConsole` fails with `ERROR_INVALID_HANDLE` (6) if the target has no console (Microsoft AttachConsole page, via search result; verify when writing) or `ERROR_ACCESS_DENIED` (5) if the caller still has a console, hence `FreeConsole()` first; `AttachConsole` can also fail while the new console is still initialising, so retry it for up to about 2 s with `Sleep(20)` and report the last `GetLastError`. A hosted step may have no usable desktop for `CREATE_NEW_CONSOLE`; `CreateProcessA` failure must print `GetLastError` too.
6. **Hosted-runner evidence:** I found **no** authoritative statement or credible report either way about `GenerateConsoleCtrlEvent`/`AttachConsole` on GitHub-hosted `windows-2025` or `windows-11-arm`. CPython's `Win32KillTests` (CTRL_C/CTRL_BREAK to a `CREATE_NEW_PROCESS_GROUP` child, readiness-polled) is the closest in-the-wild test and it exists in a project that runs on GitHub Actions, but I could not retrieve a report tying its result to the hosted runner. [CITED: search results for cpython `Lib/test/test_os/test_windows.py`; no claim beyond existence] Treat hosted delivery as unproven: **[ASSUMED]** that a `CREATE_NEW_CONSOLE` child plus `AttachConsole` works on both Windows legs. That is exactly D-16's keep-or-remove gate; the plan needs a task that pushes the branch, reads both Windows legs, and records the result in the SUMMARY.

## fsync cost against the 2 s budget (item 2b)

- What runs at exit: `nesturbator_run_file_replace` does one `fopen("wbx")`, `fwrite` of the span (8 KiB for the fixture), `fflush`, then `sync_stream`, `fclose`, then rename. `sync_stream` is `_commit` on Windows (`FlushFileBuffers`), `fcntl(F_FULLFSYNC)` with `fsync` fallback on macOS, `fsync` elsewhere. Windows additionally uses `MoveFileExA(... MOVEFILE_WRITE_THROUGH)`. [VERIFIED: runner/save.c:101-115 `if (fcntl(fileno(f), F_FULLFSYNC) == 0) {` and runner/save.c:163 `if (MoveFileExA(tmp, path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) == 0) {`]
- Measured by the discuss session: 14 ms max over 50 runs for a 32 KiB write with `F_FULLFSYNC` on a local SSD (D-17, CONTEXT). I did not re-measure; nothing in this session contradicts it.
- Hosted runners: **[ASSUMED]** fsync on Linux ext4/overlay and Windows NTFS for an 8 KiB file is tens of milliseconds at most; macOS `F_FULLFSYNC` inside a virtualised hosted VM is the one with unknown cost (it asks the virtual drive to flush to stable storage). No measurement exists. The budget has about two orders of magnitude of slack on local hardware, so the risk is low but real only on `macos-15`/`macos-15-intel`.
- Mitigation that costs nothing: the D-15 helper prints `signal-to-exit <ms>` to stderr on success as well as failure, so the first CI runs record the number per leg in the log; the deferred "one-off CI step" is then unnecessary. Assert `<= 2000` ms from the `kill()` return to `waitpid` reaping.
- Scheduling noise is the larger risk than fsync: CTest runs 4 jobs in parallel with heavy tests (`vectors.*`, `runner.boot_to_play`, asan). Give the one timing-sensitive test `RUN_SERIAL TRUE` (it runs for well under 5 s; precedent for `COST` exists, `RUN_SERIAL` is unused so far [VERIFIED: grep of tests/CMakeLists.txt found none]) or accept the bound and rely on the generous kill-to-exit margin. Recommendation: `RUN_SERIAL`, because a false failure on a loaded macOS leg is more expensive than 5 s of serial time.

## Architecture Patterns

### System Architecture Diagram

```
 launcher (Playstead, ctest helper, shell)
   |  SIGTERM / SIGINT (POSIX)            Ctrl+C / Ctrl+Break / console close (Windows)
   v                                                 v
 +--------------------------+            +-------------------------------+
 | on_signal(sig)           |            | console_ctrl_handler(type)    |  (system-created thread)
 |  signal(sig, SIG_DFL)    |            |  C/BREAK: 1st -> flag, TRUE   |
 |  flag = 1                |            |           2nd -> FALSE        |
 +------------+-------------+            |  CLOSE: flag, wait stop_done  |
              |  volatile sig_atomic_t   |         <= 4500 ms, then wait |
              |                          |         ~1 s for process exit |
              |                          +---------------+---------------+
              |   InterlockedExchange (volatile LONG)    |
              +-------------------+----------------------+
                                  v
   main(): parse -> validate -> [install handlers] -> load ROM/.sav/state
        -> pre-loop outputs -> [--raise-after 0 fires]
        -> loop: top: stop_requested()? --yes--> break
                 run frame n -> outputs -> save-interval flush -> [--raise-after n fires]
        -> after loop:
             stopped?  yes: fflush(stdout); save_if_changed (exit flush); unmet outputs? ; stderr lines
                       no : save_if_changed; print --hash-audio
             stop_done()  -> cleanup -> final fflush -> return status
                                  v
        exit 0 (done / good stop) | 1 (failure, failed flush, unmet) | 2 usage | 4 .sav size | 5 state rejected
        second signal at any time: SIG_DFL (POSIX) / FALSE -> default handler (Windows): immediate death, real file never torn
```

### Recommended Project Structure
```
runner/
├── exit_status.h     # RUNNER_EXIT_TABLE(X), enum, RUNNER_EXIT_RESERVED_TEXT (D-18)
├── stop.h            # stop_install / stop_requested / stop_done (+ header comment: contract, citations)
├── stop.c            # #define _DEFAULT_SOURCE first; POSIX and #ifdef _WIN32 halves
├── stop_internal.h   # Windows-only: handler routine, close cap setter, reset (test seam)
└── main.c            # named constants, --help, --raise-after, stop integration
tests/
├── runner/test_signal.c        # POSIX external helper (fork/exec/pipes/kill/waitpid)
├── runner/test_stop_win.c      # Windows handler unit test (links runner/stop.c)
├── runner/test_console.c       # Windows end-to-end Ctrl+Break (D-16)
├── cmake/runner_stop.cmake     # in-process stop cases (CASE pattern)
└── cmake/runner_exit_table.cmake
```
Function naming: existing runner symbols use the `nesturbator_run_` prefix (`nesturbator_run_save_flush`); the CONTEXT shorthand `stop_install()` should become `nesturbator_run_stop_install()` etc. so `-Wmissing-prototypes` and the naming convention are both satisfied. Any non-`static` function needs a prototype in a header [VERIFIED: CMakeLists.txt:20 `-Wmissing-prototypes`].

### Pattern 1: POSIX handler (D-02) with BSD semantics pinned
```c
/* runner/stop.c. Runner-only: the core links only the C memory functions. */
#define _DEFAULT_SOURCE /* glibc: signal() gets BSD (SA_RESTART) semantics under -std=c17; see Pitfall 1 */
#include "stop.h"
#include <signal.h>

static volatile sig_atomic_t stop_flag;

static void on_signal(int sig)
{
    (void)signal(sig, SIG_DFL); /* second signal: default action, immediate */
    stop_flag = 1;
}

void nesturbator_run_stop_install(void)
{
    (void)signal(SIGINT, on_signal);
    (void)signal(SIGTERM, on_signal);
}

int nesturbator_run_stop_requested(void) { return stop_flag != 0; }
```
Why `SIG_DFL` first is safe: `signal()` is async-signal-safe in POSIX; the handler touches nothing else. Under BSD semantics the disposition is not reset on entry (measured below: `restart=1 resethand=0 nodefer=0`), so the explicit reset is what makes the second signal fatal. `signal` returns the previous handler; casting to `void` satisfies `-Wunused-result` style warnings. [CITED: man7.org/linux/man-pages/man2/signal.2.html: "If such a feature test macro is not defined, then signal() provides System V semantics"; BSD semantics "equivalent to calling sigaction(2) with ... sa.sa_flags = SA_RESTART"]

`raise(SIGTERM)` in a single-threaded process delivers before `raise` returns (POSIX), so `--raise-after` K=2 is deterministic: first call runs the handler (flag, `SIG_DFL`), second call kills the process by SIGTERM. Verified on macOS by the discuss session (shell status 143).

### Pattern 2: Windows handler (D-03)
```c
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static volatile LONG stop_state;      /* 0 idle, 1 requested */
static volatile LONG stop_finished;   /* 1 after stop_done() */
static volatile LONG stop_seen;       /* number of C/BREAK events so far */

BOOL WINAPI nesturbator_run_stop_handler(DWORD type)
{
    switch (type) {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
        InterlockedExchange(&stop_state, 1);
        return InterlockedIncrement(&stop_seen) == 1 ? TRUE : FALSE; /* 2nd: default handler ends the process */
    case CTRL_CLOSE_EVENT: {
        ULONGLONG start = GetTickCount64();
        InterlockedExchange(&stop_state, 1);
        while (InterlockedCompareExchange(&stop_finished, 0, 0) == 0 &&
               GetTickCount64() - start < close_cap_ms) /* 4500 in production */
            Sleep(10);
        /* then keep waiting (bounded, ~1 s) so main's own return sets the status */
        ...
        return TRUE;
    }
    default:
        return FALSE;
    }
}
#endif
```
Details the compiler will check: `for (;;)`/bounded loops rather than `while (1)` (MSVC /W4 C4127); `GetTickCount64` returns `ULONGLONG`; compare as unsigned; no CRT `signal()`; `InterlockedCompareExchange(&v, 0, 0)` is the idiomatic atomic read. Because the handler runs on a system thread, it must not call stdio; the stdio stop line is printed by the main thread before `stop_done()`.

### Pattern 3: `--help` and the exit table
```c
/* runner/exit_status.h (runner-internal; not installed; not in nesturbator.h) */
#define RUNNER_EXIT_TABLE(X)                      \
    X(RUNNER_EXIT_DONE, 0, "done")                \
    X(RUNNER_EXIT_FAILURE, 1, "failure")          \
    X(RUNNER_EXIT_USAGE, 2, "usage error")        \
    X(RUNNER_EXIT_SAVE_SIZE, 4, ".sav size mismatch") \
    X(RUNNER_EXIT_STATE_REJECTED, 5, "state rejected")
enum {
#define RUNNER_EXIT_ENUM(name, value, text) name = value,
    RUNNER_EXIT_TABLE(RUNNER_EXIT_ENUM)
#undef RUNNER_EXIT_ENUM
};
#define RUNNER_EXIT_RESERVED_TEXT "3 and 77 are reserved"
```
`usage()` becomes `print_usage(FILE *out)`; `--help` -> stdout + 0, bad line -> stderr + 2. Print the table with one `printf("  %d  %s\n", value, text)` per row via the same X-macro so `--help` cannot drift from the header. Keep `usage(why)` printing `nesturbator-run: <why>` first (existing tests match `EXPECT_ERR` on messages).

### Anti-Patterns to Avoid
- **Reading `stop_requested()` anywhere but loop head** (and the post-loop branch): mid-frame checks break "finish the current frame".
- **Flushing from the handler or calling stdio there.** Flag-only is the locked contract.
- **`AttachConsole`/`AllocConsole`/`FreeConsole` inside the runner** (resets the handler table; D-03). They are allowed in the *test sender*.
- **Sleeps as readiness.** Every wait is a condition with a hard deadline.
- **Testing the handler with a real `kill` in the in-process cases.** Use `--raise-after`; it is the deterministic seam.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Exit table in three places | Three hand-typed lists | One X-macro, `--help` generated from it, README checked by a script | Drift is exactly what RUN-03 forbids |
| Atomic `.sav` write | New writer | `nesturbator_run_file_replace` / `nesturbator_run_save_flush` | Already tmp+fsync+rename, stale-`.tmp` cleanup, symlink/dir refusal, tested |
| Child spawn in tests (Windows) | New quoting code | Copy the quoting loop from `tests/test_process.h:34-65` | Handles backslash/quote rules already |
| Process-death classification in CMake scripts | Signal-number parsing | `execute_process(RESULT_VARIABLE)` strings | Measured: SIGTERM -> `Subprocess terminated`, SIGINT -> `User interrupt`, SIGKILL -> `Subprocess killed` on macOS CMake 4.4.3 and Linux CMake 3.31.6 |
| Movie file for stall/unmet tests | CMake `file(WRITE)` | Extend `tests/runner/save_rom.c` (it already exists because CMake strings cannot hold NUL bytes) | Format: 16-byte header `NMOVIE1\0`, version u32 `1` at offset 8, count u32 at 12, then 4 bytes per frame (`port0, 0, port1, 0`), file size exactly `16 + 4*count`, count <= 1,000,000 [VERIFIED: runner/movie.c:46-62 `expected = MOVIE_HEADER_SIZE + (uint64_t)count * 4u;`; tests/runner/test_movie.c:10-18] |

## Runtime State Inventory

Not a rename/refactor/migration phase. Omitted. (One adjacent note: the README's two scattered "Exit status:" sentences at lines 1185 and 1221 are replaced by links; no stored data or registered state is involved.)

## Common Pitfalls

### Pitfall 1: glibc `signal()` is System V under strict `-std=c17` (HIGH impact)
**What goes wrong:** `CMakeLists.txt:14` is `set(CMAKE_C_EXTENSIONS OFF)`, so GCC compiles with `-std=c17` and defines `__STRICT_ANSI__`; glibc then does not define `_DEFAULT_SOURCE`, and `signal()` resolves to System V semantics. Measured in `gcc:14` (glibc 2.41):

```
-std=c17 -D_POSIX_C_SOURCE=200809L   ->  flags: restart=0 resethand=1 nodefer=1
                                          blocked write(): chunk 16 write returned -1 errno=4 (EINTR)  ; exited after first SIGTERM
-std=c17 -D_DEFAULT_SOURCE           ->  flags: restart=1 resethand=0 nodefer=0
                                          still alive after first SIGTERM ; dead after second
```
[VERIFIED: test program run in a Linux container in this session; `_POSIX_C_SOURCE` alone (what `runner/save.c` defines) does not change it]. On macOS (Apple libc) and musl, `signal()` is BSD-style with `SA_RESTART`.
**Why it matters here:** (a) a first signal that arrives while stdout is blocked, or in the middle of `fwrite`/`fclose` of the save flush, makes the syscall fail with `EINTR`; glibc stdio does not retry, so the flush fails and the runner exits 1 with the previous save (violates D-06 "the write completes" and RUN-02); (b) the D-15 stalled-pipe test is designed on restart semantics ("first SIGTERM leaves it alive") and would behave differently on Linux (the process would leave the blocked write, then block again in the D-07 `fflush(stdout)`, so the test could still pass but for the wrong reason).
**How to avoid:** `#define _DEFAULT_SOURCE` as the very first line of `runner/stop.c` (before any include, as `runner/save.c` does with `_POSIX_C_SOURCE`). This is a feature-test macro, not a language extension, and keeps CONTEXT's "no `sigaction`". The disposition and its flags are process-wide and set at install time, so no other file needs the macro. Add a comment citing signal(2). The D-15 helper can then assert the Linux behaviour (first SIGTERM leaves the stalled runner alive) as a regression check for this exact line.
**Warning signs:** Linux-only failure of the stall test or of a mid-flush case; `.sav` unchanged after a signal during a long `--save-interval` write.

### Pitfall 2: D-12 contradicts D-08 (test shape, not design)
D-12: `--frames 4294967295 --hash-frame N --hash-frame N+1 --raise-after N` "Asserts exit 0". By D-08 a `--hash-frame N+1` whose frame was not reached makes the stop exit 1 with `nesturbator-run: not reached: --hash-frame <N+1>`.
**Fix without reopening any decision:** happy path A = `--frames 4294967295 --hash-frame N --raise-after N --save-dir A` (exit 0, stdout equal to B = `--frames N --hash-frame N --save-dir B`, `.sav` equal, no `.tmp`, stderr contains `stopped by signal after frame N`); the "no frame N+1" proof is the D-13 unmet case `... --hash-frame N --hash-frame N+1 --raise-after N` (exit 1, stdout still exactly the single frame-N line, stderr names `--hash-frame N+1`). Both are exact and cross-platform.

### Pitfall 3: the D-15 `--save-interval 1` variant has nothing to flush after the signal
The battery fixture's CPU writes the span in frame 1 then spins; `save_if_changed` returns 0 on the generation check, so after SIGTERM the exit flush is a no-op. It still proves the handler, exit 0, a whole `.sav`, and the latency of process teardown.
**Recommended second variant (same helper):** no `--save-interval`; readiness is the stdout line from `--hash-frame 1` read from a pipe the helper owns (the line appears only after the handlers are installed and frame 1 ran, and the `.sav` does not exist yet); then SIGTERM; the runner must write the `.sav` at exit (real fsync + rename) and the helper asserts `.sav` is full length with `SAVE` at offset 0, no `.tmp`, exit 0, `<= 2000` ms. This is the only test that exercises "flush after the signal" with fsync inside the 2 s window.

### Pitfall 4: a signal before handlers are installed kills the child (exit by signal)
The helper must not send until the runner is past `nesturbator_run_stop_install()`. Both readiness conditions in Pitfall 3 are after install. Never "sleep 200 ms".

### Pitfall 5: inherited signal dispositions and mask
Ignored dispositions and blocked masks survive `fork`+`exec`. A background job of a non-interactive shell has SIGINT ignored. In the helper's child, before `execv`, set `signal(SIGINT, SIG_DFL)`, `signal(SIGTERM, SIG_DFL)` and `sigprocmask(SIG_SETMASK, &empty, NULL)`. The runner itself installs unconditionally (`signal()` replaces an inherited `SIG_IGN`).

### Pitfall 6: stalled-stdout test fill level and coalescing
- Fill: Linux pipe default is 64 KiB (measured: 64000 bytes accepted in 1000-byte writes before `EAGAIN`), macOS 65536 (measured). A movie run prints about 90 bytes per frame, so use a movie of at least 2000 frames (about 180 KB); frame limit is 1,000,000. Readiness for "stalled": `ioctl(read_fd, FIONREAD, &n)` returns the bytes in the pipe on both systems [VERIFIED: test run, macOS 65536 and Linux 64000]; stalled when `n > 0` and unchanged across, say, three polls 50 ms apart. A stall check by FIONREAD is a stability window, not a sleep for readiness of the *runner's handler* (handlers were installed long before).
- Coalescing: two identical standard signals sent before the handler ran count once. Send the first, wait for a bounded `waitpid(WNOHANG)` window (alive expected), then send the second **repeatedly every 100 ms until the child dies or the deadline passes**; after the first handler ran, any further SIGTERM is fatal, so repeats cannot hurt, and the test still proves "a later signal ends it". Assert `WIFSIGNALED && WTERMSIG == SIGTERM`.
- Leave the read end of the pipe open and unread in the helper for the whole test (closing it gives SIGPIPE/EPIPE, a different stop).

### Pitfall 7: `--raise-after` edge cases the table does not list
- `N` above `--frames` never fires (reject as usage in `check_frame_limits`, like `--hash-frame`).
- `N < start_frame` with `--load-state` (reject or fire pre-loop; test whichever is chosen).
- A repeat of the option, a bad `SIG`/`K`, or `--accuracycoin-page` -> exit 2 (D-11); add `runner.usage.raise_after.*` cases.
- Windows default `SIG` is unspecified in D-11; choose `break` (it is the launcher's mechanism) and say so in `--help` and README.
- A flag raised after the **last** frame (`--raise-after N` with `N == --frames`): the loop ends naturally, nothing is cut short. Recommended: treat the run as complete (exit 0, print `--hash-audio`, no `stopped` line) and define "stopped" as "the flag was seen at a loop-head check or pre-loop with frames remaining". Otherwise a finished check would be mislabelled and its audio hash withheld. See Open Question 1.

### Pitfall 8: README table text vs header text
The existing README writes ``4 `.sav` size mismatch`` with backticks. The comparer must strip backticks from README cells (or the README row must omit them), or the number-to-text pairs differ. Decide once in `runner_exit_table.cmake` and document it in the script's header comment.

### Pitfall 9: CMake list handling of README/`--help` lines
`file(STRINGS)` lists treat `;` as separators and unbalanced `[` `]` confuse list operations. `status_table.cmake` works on the README by reading lines and testing each with `MATCHES`. Follow that: `file(STRINGS ... lines)` + `foreach` + `if(line MATCHES "^\\| ([0-9]+) \\| (.*) \\|$")`, restricted to the lines between `### Exit status` and the next `#` heading. For the header, `file(READ)` the whole file and `string(REGEX MATCHALL "X\\(RUNNER_EXIT_[A-Z_]+, *[0-9]+, *\"[^\"]*\"\\)" rows "${text}")` then per-row `string(REGEX REPLACE ...)`. For `--help` output, strip `\r` (Windows) first, as `expect_output.cmake` does.

### Pitfall 10: ASan and the stop paths
`asan` (Clang 18, `-fsanitize=address,undefined -fno-sanitize-recover=all`) runs every registered test. The graceful stop path must still reach the existing cleanup (`free(save.shadow)`, `nesturbator_destroy`, `nesturbator_movie_free`, `free_options`) so LeakSanitizer stays quiet; K=2 deaths are by default action and skip LSan. With `NESTURBATOR_SHORT_RUNS` set, nothing here needs shortening (frame counts are tiny), but the POSIX helper's absolute deadlines (2 s kill-to-exit) must tolerate ASan start-up; measure on the asan lane and set `TIMEOUT` to twice the slowest measured run (ENGINEERING section 5; Phase 13 had to re-measure CI timeouts after hosted asan ran several times slower than local).

### Pitfall 11: MSVC `/W4` as errors, Windows arm64
`CMAKE_COMPILE_WARNING_AS_ERROR=ON` for `ci-msvc`. Watch: C4100 (unreferenced parameter in a handler or stub), C4127 (constant condition), C4244/C4267 on `ULONGLONG`/`DWORD` narrowing in elapsed-time arithmetic, C4702 (unreachable code after `ExitProcess`). `windows-11-arm` uses VS 2022 17.14 and `windows-2025` VS 2026 [CITED: ENGINEERING section 5]; same flags. ARM64 `/volatile:iso` is why shared state is `volatile LONG` plus `Interlocked*`, not `sig_atomic_t` (D-03). In `test_stop_win.c` use `CreateThread` + `WaitForSingleObject` with a timeout and always close handles.

### Pitfall 12: D-07 order versus a stalled stdout (risk note, not a change)
The exit sequence flushes stdout **before** the save. If a launcher leaves a full pipe un-drained while the runner prints (movie, hash lines), `fflush(stdout)` blocks and the SIGKILL at 2 s lands before the `.sav` write. Playstead's documented command prints nothing, so the contract holds for it; the README process-contract section should say the launcher must drain or discard stdout. If the owner later wants the save to win, swapping the two calls is a one-line change.

### Pitfall 13: hygiene
`scripts/hygiene.sh` flags any tracked file whose name ends `.nes .fds .unf .unif .nsf .nsfe .sav .srm .state` unless listed in `tests/roms/manifest.txt` [VERIFIED: scripts/hygiene.sh `ROM_NAME` line]. All `.sav`, `.nes`, movie and scratch files are generated under `${CMAKE_CURRENT_BINARY_DIR}`; commit none. No home paths or emails in comments or the README (hook runs on every commit).

## Concrete C17 notes (item 3)

| Topic | Fact | Source |
|-------|------|--------|
| `volatile sig_atomic_t` | Only object type a handler may portably store to besides lock-free atomics; reads in the main loop use plain volatile reads | C17 7.14.1.1 [ASSUMED: from standard text memory] |
| `signal(sig, SIG_DFL)` inside handler | Allowed (async-signal-safe); makes the next signal fatal | POSIX signal(2) [CITED: man7.org signal(2)] |
| Re-raise | Not needed; exit goes through normal return so exit status is 0/1, not 128+n (CONTEXT) | CONTEXT carry-forward |
| `raise()` | Single-threaded: handler runs before `raise` returns | POSIX [ASSUMED: from standard text memory; observed on macOS by researcher] |
| Spawn in helper | `fork` + `execv` (macro `_POSIX_C_SOURCE 200809L` required in strict C17, as `test_save_replace.c` does); `posix_spawn` is not needed because the child needs `signal`/mask reset between fork and exec | tests/runner/test_save_replace.c:5 |
| Deadlines | `clock_gettime(CLOCK_MONOTONIC)` (macOS >= 10.12, Linux), `nanosleep` for 20-50 ms polls (not `usleep`) | POSIX |
| Poll loop | `waitpid(pid, &st, WNOHANG)` until reaped or deadline; on deadline `kill(pid, SIGKILL)`, reap, fail | |
| Exit status | `WIFEXITED`/`WEXITSTATUS`; for death `WIFSIGNALED`/`WTERMSIG` | |
| Calling the Windows handler in a unit test | Declare `BOOL WINAPI nesturbator_run_stop_handler(DWORD)` in `stop_internal.h`; the test links `runner/stop.c` and calls it directly (no console needed). A close-cap setter and a state reset are in the same internal header. Cases: C twice -> TRUE, FALSE; CLOSE on a second thread returns only after `nesturbator_run_stop_done()` and within cap; CLOSE with `done` never called returns at the (short) cap | D-14 |
| `--raise-after` on Windows | Calls the handler routine directly with `CTRL_C_EVENT`/`CTRL_BREAK_EVENT`; on FALSE, `ExitProcess(0xC000013Au)` | Section 2a item 4 |

## Test harness recipes (items 4 and 5)

### `tests/cmake/runner_exit_table.cmake` (D-22)
Register next to `state.status_table` (tests/CMakeLists.txt:933-936) as `runner.exit.table`:
```
add_test(NAME runner.exit.table
  COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${PROJECT_SOURCE_DIR} "-DRUNNER=$<TARGET_FILE:nesturbator-run>"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/runner_exit_table.cmake)
set_tests_properties(runner.exit.table PROPERTIES LABELS exit_status)
```
Script outline: first line after comments `cmake_minimum_required(VERSION 3.25)` (script_policy test enforces this exact line). (1) `file(READ "${SOURCE_DIR}/runner/exit_status.h" h)`; `string(REGEX MATCHALL ...)` for table rows -> list `"0:done" "1:failure" ...`; read `RUNNER_EXIT_RESERVED_TEXT`. (2) README: `file(STRINGS "${SOURCE_DIR}/README.md" lines)`, find the exact line `### Exit status`, collect `| N | text |` rows until the next heading, strip backticks; capture the reserved sentence. (3) `execute_process(COMMAND "${RUNNER}" --help OUTPUT_VARIABLE out RESULT_VARIABLE rc)`; require `rc` 0, strip `\r`, collect `^ +([0-9]+) +(.+)$` rows between an `Exit status` line and the end, and the reserved text. (4) Compare as sorted lists (copy `status_table.cmake`'s `list(SORT ... COMPARE NATURAL)` + membership loops + duplicate check by count); require the set to equal exactly `0;1;2;4;5`; require `3` and `77` to occur nowhere else than in the reserved sentence (scan table rows only). (5) README claim guards, in the style of `status_table.cmake`'s `has_sentence`: the README contains the key RUN-02 criterion-4 phrases (Ctrl+Break handled, console close handled, forced termination cannot be handled, second signal ends the runner). This gives success criterion 4 an automated check, which rule 6 and "nothing waits on a person" imply.

### Test inventory: criterion -> test -> label -> platforms
| Criterion | Test (proposed name) | Script / helper | Labels | Where |
|-----------|----------------------|-----------------|--------|-------|
| 1 frame-indexed, flush, exit 0 | `runner.stop.frame_exact.term`, `.int` | `runner_stop.cmake` (CASE), `--raise-after N[:SIG]` | `exit_status` | POSIX legs |
| 1 same on Windows | `runner.stop.frame_exact.int`, `.break` | same, `break` and `int` via direct handler call | `exit_status` | Windows legs |
| 1 exit 1 on failed flush | `runner.stop.flush_missing_dir`, `runner.stop.flush_dir_at_tmp` | same script, existing `unwritable`/`dir_at_tmp` shapes | `exit_status` | all six |
| D-13 rest | `runner.stop.at_zero`, `.unmet_hash_frame`, `.unmet_hash_audio`, `.unmet_movie`, `.nodir`, `.double` | same script | `exit_status` for those giving 0/1 | all six |
| 2 external SIGTERM / SIGINT, 2 s, whole `.sav`, no `.tmp` | `runner.signal.term`, `runner.signal.int` (interval variant) and `.term_exitflush` (Pitfall 3 variant) | `test_signal.c` | `exit_status` optional | `if(NOT WIN32)` |
| 2 second signal ends it | `runner.signal.second` | `test_signal.c` stalled movie run | | `if(NOT WIN32)` |
| 3 table identical in three places | `runner.exit.table` | `runner_exit_table.cmake` | `exit_status` | all six |
| 3 each status has a test | new `runner.exit.done` (0: `--frames 1`), `.failure` (missing ROM, 1), `.usage` (no args, 2), `.help` (0, stdout has the table); relabel `runner.save.mismatch` (4), `runner.state.reject_game` (5), `runner.save.unwritable` (1) | `nesturbator_runner_test` / existing CASE tests | `exit_status` | all six |
| 4 README Windows text | `runner.exit.table` README guard | | | all six |
| Windows handler logic | `runner.stop.win_handler` | `test_stop_win.c` | | `if(WIN32)` |
| Windows real delivery | `runner.stop.win_console` | `test_console.c` | | `if(WIN32)`, evidence-gated (D-16) |

Relabelling existing CASE tests: add `set_tests_properties(runner.save.mismatch runner.save.unwritable runner.state.reject_game PROPERTIES LABELS exit_status)` after the case loops. `ctest -L exit_status` matches labels by regex; no other label contains that substring.

**Fixtures and timeouts.** Every new runner test that needs the battery ROM uses `FIXTURES_REQUIRED runner_save`. In-process stop cases: TIMEOUT 30 (like `runner.save.*`; they run a few frames). `test_signal.c` cases: helper hard deadline per wait (about 5 s each), CTest TIMEOUT = twice the slowest measured hosted run, recorded in a comment citing the run id (ENGINEERING section 5; Phase 13 re-measured job timeouts: build 22 min, asan 14 min, so a few seconds of new tests do not move those). Add `RUN_SERIAL TRUE` to the timing-sensitive `runner.signal.*` tests (Section 2b).

**Sampling:** per task: `ctest --preset dev -L exit_status` (or `ctest --test-dir build/dev -R 'runner\.(stop|signal|exit)'`); per wave: `cmake --workflow --preset ci`; phase gate: `ci` + `asan` workflows green and the six-leg PR CI.

**Wave 0 gaps:** `runner/exit_status.h`, `runner/stop.{h,c}`, `tests/cmake/runner_stop.cmake`, `tests/cmake/runner_exit_table.cmake`, `tests/runner/test_signal.c`, `tests/runner/test_stop_win.c`, `tests/runner/test_console.c`, movie fixture bytes in `save_rom.c`. No new framework.

## Code Examples

### CMake: classify a child's death (verified output)
```cmake
execute_process(COMMAND "${RUNNER}" ${args} OUTPUT_QUIET ERROR_VARIABLE err RESULT_VARIABLE rc)
if(WIN32)
  if(rc STREQUAL "0" OR rc STREQUAL "1")
    message(FATAL_ERROR "second signal must kill the runner, got ${rc}")
  endif()
elseif(NOT rc MATCHES "terminated|interrupt")      # 'Subprocess terminated' / 'User interrupt'
  message(FATAL_ERROR "second signal must kill the runner, got '${rc}'")
endif()
```
[VERIFIED: macOS CMake 4.4.3 and Linux CMake 3.31.6 printed `Subprocess terminated` and `User interrupt`; Windows returns the numeric status, expected 0xC000013A per Microsoft/gitlab-runner, but only "not 0 and not 1" is asserted]

### POSIX helper skeleton (deadline-bounded)
```c
#define _POSIX_C_SOURCE 200809L
/* spawn: pipe(out); fork(); child: dup2, reset signal dispositions and mask, execv(runner, argv)
   wait_for_sav(path, 8192, deadline)  -> polls stat() every 20 ms (nanosleep), fails at the deadline and SIGKILLs
   t0 = now(); kill(pid, SIGTERM); reap with WNOHANG polls until deadline; ms = now() - t0
   CHECK(WIFEXITED(st) && WEXITSTATUS(st) == 0); CHECK(ms <= 2000); fprintf(stderr, "signal-to-exit %ld ms\n", ms);
   file: size == 8192, first 4 bytes "SAVE"; no "<name>.sav.tmp" */
```

### Windows end-to-end sender (D-16), order matters
```c
/* parent (the test) */
CreateProcessA(runner, cmdline, NULL, NULL, TRUE /*inherit std handles*/,
               CREATE_NEW_CONSOLE, NULL, NULL, &si /*STARTF_USESTDHANDLES to a file*/, &pi);
/* wait for the .sav at full length (handler is installed before frame 1) */
FreeConsole();
for (;;) { if (AttachConsole(pi.dwProcessId)) break; /* retry <= 2 s; report GetLastError */ }
SetConsoleCtrlHandler(sender_handler /* returns TRUE */, TRUE);   /* AFTER AttachConsole */
GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, 0);                    /* never CTRL_C_EVENT to a group */
WaitForSingleObject(pi.hProcess, 5000);  GetExitCodeProcess(...); /* expect 0, whole .sav, no .tmp */
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| `sigaction` with explicit flags | `signal()` + feature macro (locked by CONTEXT) | v3 research | Smaller code; must pin BSD semantics on glibc (Pitfall 1) |
| 128+n exit after a signal | exit 0 after a clean flush | CONTEXT | Launcher reads one status |
| CRT `signal(SIGINT)` on Windows | `SetConsoleCtrlHandler` | Microsoft: "SIGINT is not supported for any Win32 application" | No hidden thread surprises |

**Deprecated/outdated:** none relevant.

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | A `CREATE_NEW_CONSOLE` child plus `AttachConsole` + `GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, 0)` works on GitHub-hosted `windows-2025` and `windows-11-arm` | Windows delivery | D-16 test fails on a leg; D-16 already defines the keep-or-remove decision with evidence |
| A2 | fsync/`FlushFileBuffers` for an 8 KiB file on hosted runners costs tens of ms; `F_FULLFSYNC` on virtualised macOS is the unknown | fsync cost | The 2 s assertion flakes on macOS legs; mitigation: log the measured ms, `RUN_SERIAL` |
| A3 | C17 and POSIX statements about `sig_atomic_t` and `raise()` timing (from standard text, not re-read this session) | Concrete C17 notes | Low; behaviour also observed on macOS and Linux in this session |
| A4 | AttachConsole error codes (`ERROR_INVALID_HANDLE` when target has no console, `ERROR_ACCESS_DENIED` when already attached) were seen only in a search-result summary of the Microsoft page | Windows delivery item 5 | Only affects failure message wording; re-read the page when writing the helper |
| A5 | Pipe capacity is 64 KiB on the hosted Linux and macOS images (measured locally only: macOS 65536, Linux container 64000) | Pitfall 6 | A smaller pipe makes the stall come sooner; a 2000-frame movie is safe for any pipe up to ~180 KB |
| A6 | The Windows `--raise-after` default `SIG` should be `break` | Pitfall 7 | Cosmetic; D-11 leaves it open |

## Open Questions

1. **A flag set after the last frame (`--raise-after N`, `N == --frames`, or a real signal during the final frame).**
   - What we know: D-07 says "after the loop ends on the flag"; D-09 withholds `--hash-audio` on "any stop".
   - What's unclear: whether a signal that arrives after all work finished is "a stop".
   - Recommendation: only a loop-head or pre-loop observation with frames remaining is a stop; otherwise complete normally. Add a test (`runner.stop.late`).
2. **`--raise-after N` below a loaded state's frame.** Recommendation: usage error (exit 2) in `check_ranges`, tested.
3. **Hosted Windows console delivery** (A1): answered only by CI. Plan a task that pushes the branch, reads both Windows legs, and records the outcome and the keep/remove decision in the SUMMARY (D-16).
4. **Stdout flush order** (Pitfall 12): leave as decided; confirm the README tells launchers to drain or discard stdout.

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| CMake | all builds and `cmake -P` scripts | yes | 4.4.3 local; CI 3.31.x | |
| Ninja, clang (Apple) | local `dev`/`ci`/`asan` presets | yes (a `build/` tree with presets exists) | | |
| Docker (Linux/glibc container) | verifying Linux signal behaviour | yes | `gcc:14`, glibc 2.41, CMake 3.31.6 in a container | |
| Windows toolchain | MSVC `/W4`, console tests | no (macOS host) | | Six-leg PR CI is the only Windows evidence; plan for one push-and-read round trip |

**Missing with no fallback:** none. **Missing with fallback:** local Windows; use PR CI.

## Validation Architecture

`workflow.nyquist_validation` is `false` in `.planning/config.json`, so the formal template is skipped; the task asked for the mapping, which is the "Test inventory" table above. Framework: CTest + in-repo `check.h` and CMake `-P` scripts; quick command `ctest --test-dir build/ci -L exit_status` plus `-R 'runner\.(stop|signal)'`; full suite `cmake --workflow --preset ci`; sanitizers `cmake --workflow --preset asan`; Windows leg `ci-msvc` in CI only.

## Security Domain

`workflow.security_enforcement` is `false`; section omitted. Notes kept out of scope: the runner opens only paths the caller names; `--raise-after` signals only the runner's own process, never another pid (D-11 text says so; keep it so).

## Project Constraints (from CLAUDE.md)

- C17 without extensions; core links only C memory functions (runner-only OS code is fine, as `save.c` already uses kernel32). `_DEFAULT_SOURCE` in `stop.c` is a feature-test macro, not a language extension.
- Own code only; no new dependencies; `libretro.h` is the only vendored file.
- Clean room: do not open GPL/LGPL emulator source (not needed; sources here are POSIX, C and Microsoft docs).
- Clean tree: no ROM/BIOS bytes beyond manifest; no personal paths/emails; commits use the noreply identity.
- Deterministic core, integer-only: runner timing code is integer (`clock_gettime`, `GetTickCount64`).
- Every behaviour change ships with a test in `cmake --workflow --preset ci`; README and header comments in the same change; automated checks only, none waits on a person.
- One GSD step per command; phase branch; merge by PR with a Conventional Commit title.
- Style: small modules, plain control flow, fixed-width types; hardware/OS comments cite their source.

## Sources

### Primary (HIGH confidence)
- Repository files read this session: `runner/main.c`, `runner/save.c`, `runner/CMakeLists.txt`, `tests/CMakeLists.txt`, `tests/cmake/{expect_output,status_table,script_policy,skip_policy,runner_save,runner_version}.cmake`, `tests/test_process.h`, `tests/check.h`, `tests/runner/{save_rom,test_movie,test_save_replace}.c`, `runner/movie.c`, `CMakeLists.txt`, `CMakePresets.json`, `scripts/hygiene.sh`, `.github/workflows/ci.yml`
- Measurements in this session: glibc 2.41 `signal()` flags and EINTR (container), macOS and Linux `FIONREAD`/pipe capacity, CMake `execute_process` death strings
- Microsoft Learn: GenerateConsoleCtrlEvent, HandlerRoutine, SetConsoleCtrlHandler, Process Creation Flags, CRT signal, /volatile
- man7.org signal(2)

### Secondary (MEDIUM confidence)
- gitlab.com/gitlab-org/gitlab-runner/-/merge_requests/7075 (CTRL_BREAK race, exit 0xc000013a), read via search summary (page itself returned 403)
- CPython `Win32KillTests` existence (search results only)

### Tertiary (LOW confidence)
- AttachConsole error-code behaviour (search summary only; re-read Microsoft page when writing the helper)

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH, nothing new
- Architecture / code map: HIGH, read directly
- POSIX behaviour: HIGH, measured on both OS families
- Windows hosted delivery: MEDIUM-LOW, documented API semantics are clear but hosted-runner behaviour is unobserved
- Pitfalls: HIGH for 1-3, 6, 8-9; MEDIUM for 10-12

**Research date:** 2026-10-12
**Valid until:** 2026-11-12 (runner images and Windows image changes in ENGINEERING section 5 are the fast-moving part)
