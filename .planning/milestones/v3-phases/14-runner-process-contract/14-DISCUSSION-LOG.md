# Phase 14: Runner process contract - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-10-12
**Phase:** 14-runner-process-contract
**Areas discussed:** Signal at a set frame, Windows events, Stop output, Exit table source

The owner selected all four areas and asked for a research fan-out per area (every stakeholder
lens, primary sources, adversarial pass, copy over dependency) with one coherent recommendation
per area, followed automatically. Each area went to an advisor agent (minimal_decisive tier) and
the recommendations were reconciled in CONTEXT.md.

---

## Signal at a set frame

| Option | Description | Selected |
|--------|-------------|----------|
| Test-aid option in the shipped runner (`--raise-after`) | The runner signals itself after frame N, so the stop is exact to the frame on all six hosts and the tested binary is the shipped one | ✓ |
| Compile-time hooked second runner | Keeps test surface out of the release, but the tested binary is not the shipped one | |
| External helper only | Real delivery, but not frame-exact, and Windows console delivery is unverified on hosted runners | (Linux and macOS only, for criterion 2) |

**User's choice:** the research recommendation.
**Notes:** Probes confirmed the second-signal kill and the stall-in-`write()` technique on macOS.

---

## Windows events

| Option | Description | Selected |
|--------|-------------|----------|
| `SetConsoleCtrlHandler` for C, Break and Close, using Interlocked state | Only this primitive can hold the process alive for the flush on CLOSE | ✓ |
| CRT `signal(SIGINT/SIGBREAK)` only | Cannot handle console close, so it fails criterion 4 | |
| Also handle LOGOFF and SHUTDOWN | Only services receive these events, and it would break service wrappers | |

**User's choice:** the research recommendation.
**Notes:**
- Reconciliation: the frame seam calls the handler routine directly on Windows, because `raise()` does not reach a console handler.
- A real Ctrl+Break end-to-end test is attempted. It fails loudly on a hosted runner and is kept or removed only on CI evidence.
- Microsoft documents that Task Manager's End task sends CLOSE, so the README limits "cannot be handled" to forced termination.

---

## Stop output

| Option | Description | Selected |
|--------|-------------|----------|
| Exit 0 unless a requested output was not reached, then 1 | Playstead's command still gets 0, and a stopped check never looks like a pass | ✓ |
| Always 0 after a good flush | A Ctrl+C on a `--hash-frame` run would look like success | |
| A distinct status | Contradicts the decided exit 0 and the fixed table | |

**User's choice:** the research recommendation.
**Notes:**
- Reconciliation: the seam researcher's stdout-equality test drops `--hash-audio`, because a stopped `--hash-audio` run prints nothing and exits 1 (D-09).
- `SA_RESETHAND` from one report was replaced by the decided C17 `signal()` with `SIG_DFL`.

---

## Exit table source

| Option | Description | Selected |
|--------|-------------|----------|
| Runner-internal `runner/exit_status.h` X-macro driving the usage text, plus a CMake check of header, `--help` and README | One definition site and no public ABI | ✓ |
| Public `include/nesturbator.h` | The library never returns these codes, so it adds semver burden | |
| Plain `#define`s and two text checks | Two hand copies to drift | |

**User's choice:** the research recommendation.
**Notes:**
- Reconciliation: a real `--help` (stdout, exit 0) is added, because criterion 3 names it. The researcher had proposed treating the usage text as `--help`.
- The proposed bare-literal source scan was dropped as ceremony. The behaviour tests guard instead.

## Claude's Discretion

- How the work is split into plans, the exact row and README wording, and extra test names.

## Deferred Ideas

- SIGUSR1 flush.
- RUN-04 (`--expect`, `--capabilities`).
- LOGOFF/SHUTDOWN handling.
- A per-runner flush-time CI step.
