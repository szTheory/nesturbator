# Phase 14: Runner process contract - Context

**Gathered:** 2026-10-12
**Status:** Ready for planning

<domain>
## Phase Boundary

nesturbator-run can be stopped at any time by a launcher (Playstead sends SIGTERM, then SIGKILL
after 2 s) without losing a battery save, and every outcome is one documented exit status
(RUN-02, RUN-03). The runner finishes the current frame, flushes the battery save and exits 0,
or 1 if the flush failed; a second signal ends it at once. On Windows, Ctrl+C, Ctrl+Break and
console close are handled the same way. The exit-status table (0, 1, 2, 4, 5; 3 and 77 reserved)
is defined once in a runner header and is checked against `--help` and the README.

Not in this phase: `--expect` (3) and `--capabilities` (77) (RUN-04, future), SIGUSR1 flush on
demand, CTRL_LOGOFF/CTRL_SHUTDOWN handling, speed work (Phase 15).

</domain>

<decisions>
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
  - **Reconciliation (planning, 2026-10-12):** the literal form above conflicts with D-08: an
    unreached `--hash-frame N+1` makes a stopped run exit 1, not 0. D-12's intent is kept by two
    tests: `runner.stop.frame_exact.<sig>` (Plan 02) runs the happy path without
    `--hash-frame N+1` and asserts exit 0, equal stdout and `.sav`, no `.tmp` and the stop line;
    `runner.stop.unmet_hash_frame` (Plan 05) runs `--hash-frame N --hash-frame N+1` and asserts
    stdout equal to the unstopped run (so frame N+1 never ran), exit 1 and the not-reached line
    (research Pitfall 2). The decision itself is unchanged.
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

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Requirements and scope
- `.planning/ROADMAP.md` Phase 14 — goal, success criteria 1-4, research flag
- `.planning/REQUIREMENTS.md` RUN-02, RUN-03 (and RUN-04, deferred)

### Prior research and contract
- `.planning/research/STACK.md` §2 — C17 `signal()` handler, Windows CRT/console facts, exit-status rationale
- `.planning/research/ARCHITECTURE.md` §6.4–6.5 — signal flush design, exit status table
- `.planning/research/PITFALLS.md` pitfall 13 — handler hygiene and deterministic signal tests
- `.planning/research/FEATURES.md` runner rows — Playstead's SIGTERM-then-SIGKILL-after-2-s contract
- `.planning/preparation/LIBRETRO-AND-RUNNER.md` §5 — runner exit statuses
- `.planning/preparation/ENGINEERING.md` §5 — CI timeouts at twice the measured slowest run

### External primary sources (cited in code comments)
- Microsoft Learn: `HandlerRoutine`, `SetConsoleCtrlHandler`, `GenerateConsoleCtrlEvent`, process creation flags, CRT `signal`
- POSIX / C17 `signal` (`volatile sig_atomic_t`, `SIG_DFL`)

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `runner/save.c` `nesturbator_run_file_replace`: tmp, exclusive create, fsync/F_FULLFSYNC/_commit, rename/MoveFileExA; removes a stale `.tmp`.
- `runner/main.c` `save_if_changed` (line ~361) and the main loop (line ~1097): the stop check and exit flush go here.
- `tests/cmake/runner_save.cmake`, `tests/cmake/runner_state.cmake`: runner child-process cases with save dirs; `unwritable`, `dir_at_tmp` and `mismatch` shapes.
- `tests/runner/save_rom.c` (battery fixture), `tests/runner/movie_fixture.h` (movie generator), `tests/test_process.h` (child process ideas).
- `tests/cmake/status_table.cmake`: header-vs-README table parser to copy for `runner_exit_table.cmake`.
- `nesturbator_runner_test(NAME ... ARGS ... EXIT n)` helper in `tests/CMakeLists.txt`.

### Established Patterns
- `usage()` prints to stderr and returns 2; there is no `--help` yet.
- `NESTURBATOR_SHORT_RUNS` (preset asan) shortens long tests; CTest TIMEOUTs follow measured runs.
- Windows code in the runner already uses `windows.h` under `_WIN32`; MSVC builds with `/W4` and warnings as errors.

### Integration Points
- `runner/CMakeLists.txt` (add `stop.c`), `tests/CMakeLists.txt` (new tests, labels), `README.md` runner section, `runner/main.c` header comment and usage text.

</code_context>

<specifics>
## Specific Ideas

- Probes on macOS 25.6 (researcher):
  - With the decided handler, a second `raise(SIGTERM)` kills the process (shell status 143).
  - A runner blocked in `write()` to a full pipe survives the first SIGTERM (restart semantics) and dies on the second.
  - Two identical pending standard signals coalesce, so send the second only after the first is delivered.
- Exit-flush cost measured at 14 ms max locally; the 2 s budget is not at risk from fsync on local disks.

</specifics>

<deferred>
## Deferred Ideas

- SIGUSR1 "flush now" (POSIX only); it is not in RUN-02.
- `--expect` (exit 3) and `--capabilities` (exit 77) (RUN-04).
- CTRL_LOGOFF and CTRL_SHUTDOWN handling (service wrappers). Microsoft says only services receive them.
- A one-off CI step that prints the exit-flush time per hosted runner (D-15 already asserts the bound).

</deferred>

---

*Phase: 14-runner-process-contract*
*Context gathered: 2026-10-12*
