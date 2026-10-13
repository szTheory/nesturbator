# Phase 15: Speed and close-out - Research

**Researched:** 2026-10-13
**Domain:** C17 runner benchmarking (monotonic clock, integer percentiles), Cachegrind instruction-count gate in GitHub Actions, CI timeout/pin close-out
**Confidence:** HIGH for the repo map, the Cachegrind determinism evidence (measured in an Ubuntu 24.04 container this session), the signal fix and the clock macros (compiled and run); MEDIUM for hosted-runner behaviour that was not run on a hosted x64 runner (valgrind timing, `inputs.control` typing)

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

The owner selected all four areas and asked for a research fan-out per area (every stakeholder
lens, primary sources, adversarial pass, no new dependencies) and one coherent recommendation per
area, followed automatically. The decisions below are those recommendations, reconciled with each
other and with prior decisions.

Carried forward (not re-decided):
- Speed is compared by Cachegrind `Ir` on Linux, head against merge base, and wall time is never
  gated (DEC.26, ENGINEERING §5, PERF-03).
- Timeouts are twice a measured cold run, with the run ID in a comment.
- Every `uses:` is pinned to a full SHA (`tests/cmake/action_pins.cmake`).
- The single required check `CI required` uses `if: always()` and requires every result to be
  `success`.
- Phase 14 runner conventions:
  - `RUNNER_EXIT_TABLE` statuses.
  - Usage errors exit 2.
  - stderr lines are prefixed `nesturbator-run:`.
  - stdout carries only hashes.
  - The README is checked against `--help` by a test.
- C17 `signal()` on POSIX, no `sigaction` (Phase 14 carried-forward decision; see D-20).

### Bench mode (PERF-03)
- **D-01:** `--bench N` is a run mode: it runs frames 1..N and times every one. It is mutually
  exclusive with `--frames` and `--movie` (exit 2). `--hash-frame K` (1 <= K <= N) and
  `--hash-audio` are allowed and print to stdout exactly as with `--frames N`.
  - There is no warm-up and no warm-up flag. The first frames' cold cost is real, shows in
    `max`, and keeps frame numbering identical to `--frames` (ladder: remove the need).
  - N is capped at 1000000 (exit 2 above), which bounds the sample buffer to 8 MB of
    `uint64_t`. An allocation failure uses the existing out-of-memory path.
- **D-02:** `--bench-state` is a switch that requires `--bench N` (exit 2 without it).
  - After each timed frame it calls `nesturbator_save_state` into a buffer sized once with
    `nesturbator_state_size`, then `nesturbator_load_state` from that buffer.
  - Save and load are timed separately and are not folded into the frame time, so `frame_us`
    stays comparable to a plain `--bench`.
  - A save or load failure prints a message and exits 1. It is a failed operation, not a
    rejected input file.
- **D-03:** `--bench` requires `--rom`. It rejects, with exit 2:
  - `--accuracycoin-page`, `--raise-after`, `--dump-frame`, `--state-at-frame`, `--hash-state`
  - `--save-dir`, `--save-interval`, `--load-state`
  - The reason is that file I/O and loaded starts pollute or complicate timing; any of these
    can be added later on request.
  - A real SIGINT or SIGTERM follows the Phase 14 stop contract. The stats print over the
    completed frames, then the stop line, and the run exits 0 (an unmet `--hash-frame` still
    gives 1, per Phase 14 D-08).
- **D-04:** Clock: a small runner-only module (e.g. `runner/clock.c` + `.h`) with
  `uint64_t clock_now_ns(void)`. It sits beside `runner/stop.c` with the same `#ifdef _WIN32`
  pattern.
  - POSIX uses `clock_gettime(CLOCK_MONOTONIC)`.
  - Windows uses `QueryPerformanceCounter` / `QueryPerformanceFrequency`, converted without
    overflow as `sec * 1000000000 + rem * 1000000000 / freq`.
  - Not `timespec_get` (C17 only guarantees `TIME_UTC`, which can step), not `clock()`, not
    `CLOCK_REALTIME`.
  - Only the call to `nesturbator_run_frame` (and, separately, the save and the load) sits
    between timestamps. Hashing, audio hashing and stdout are outside the timed region.
- **D-05:** Percentiles use nearest rank on the sorted samples (`qsort`) in integer math:
  `idx = (p_x10 * N + 999) / 1000 - 1` for p in tenths (500, 900, 990, 999), and max is the last
  element.
  - Every reported value is a measured frame time.
  - With N < 1000, p99.9 equals max, and the README says so.
  - There is no interpolation and no float.
- **D-06:** Output goes to stderr only, as stable `key=value` lines with the runner prefix.
  Times are integer nanoseconds printed as microseconds with three decimals, and fps with two
  decimals, all formatted with integer division (no `%f`):
  ```
  nesturbator-run: bench frames=600 time_ms=1234.567 fps=486.21
  nesturbator-run: bench frame_us p50=1980.214 p90=2101.700 p99=2398.003 p99.9=3011.420 max=3544.901
  nesturbator-run: bench save_us p50=41.200 p90=44.003 p99=60.100 p99.9=90.310 max=120.702
  nesturbator-run: bench load_us p50=38.904 p90=40.100 p99=52.000 p99.9=77.707 max=99.010
  ```
  - `time_ms` is the sum of the timed frame times, and `fps = frames / time`.
  - The save and load lines appear only with `--bench-state`.
  - `--help` and the README list both options, with the README noting that the timer overhead
    is included and that wall time is never gated. The README check against `--help` already
    covers this.
- **D-07:** Tests run by `cmake --workflow --preset ci`:
  - **No-hash-change test.** A CMake `-P` test runs the same short committed ROM three times:
    - `--frames K --hash-frame K --hash-audio`
    - `--bench K` with the same hash options
    - `--bench K --bench-state` with the same hash options

    It asserts exit 0 and byte-identical stdout across all three. With `--bench-state` this also
    proves a save and load every frame is invisible.
  - **stderr format test.** stderr is checked for the line shapes and key names only, never the
    numbers.
  - **Usage tests.** One exit-2 test per D-01 and D-03 conflict, plus `--bench 0`, `--bench`
    without `--rom`, and `--bench-state` without `--bench`.
  - **Percentile unit test.** A unit test of the percentile helper on fixed arrays: N=1, N=999
    (p99.9 equals max), N=1000, and known values.
  - K is small (about 120 frames) so the tests stay fast on all six platforms.

### Cachegrind job (PERF-02, criterion 2)
- **D-08:** Per-frame instructions are measured by differencing two run lengths:
  `Ir_per_frame = (Ir(360) - Ir(60)) / 300`, where each run is
  `valgrind --tool=cachegrind --cache-sim=no --cachegrind-out-file=F nesturbator-run --rom R --frames N >/dev/null`
  under `env -i` with a fixed `PATH`.
  - `Ir` is read from the `summary:` line of `F`, never from stderr.
  - Startup, the dynamic loader and ROM load cancel exactly.
  - There is no `valgrind.h` (`libretro.h` stays the only vendored file), no
    `--toggle-collect` (fragile under inlining and renames), and no runner change, so it also
    measures PRs that edit the runner.
  - Valgrind comes from apt on `ubuntu-24.04` (a system tool, recorded with its version, not a
    dependency of the code).
- **D-09:** ROMs are taken from the head checkout for both binaries, with no movie input:
  - NROM: `tests/roms/nesteroids.nes`
  - MMC1: `tests/roms/runner.nes`
  - MMC3: `tests/roms/hm/M4_P256K_C256K.nes`

  A plan task confirms each has rendering on by frame 60 (dump frame 60 and check it is not
  blank). An idle screen is acceptable, because every CPU, PPU and APU cycle runs regardless; the
  job measures a delta, not a realistic game.
- **D-10:** Builds use the `ci` preset (Release, gcc-14, as in the build job), target
  `nesturbator-run` only, so the job measures what ships.
  - Head is the `pull_request` test-merge checkout. Base is its first parent (`HEAD^1`, the
    current tip of `main`), built in `git worktree add "$RUNNER_TEMP/base" HEAD^1` with its own
    binary directory.
  - Checkout uses `fetch-depth: 2`.
  - ENGINEERING §5's "merge base" is defined as this first parent, which is exactly the PR's
    effect without merge-base drift.
  - There is no build cache.
  - If the base fails to build or run, its row prints `n/a`, and the job stays green in report
    mode.
- **D-11:** Each binary is measured per ROM. Base is measured twice (`base_a`, `base_b`, the
  per-PR repeatability control) and head once, which makes 18 valgrind runs.
  - If `base_a != base_b` on any ROM, the job prints `UNSTABLE` and exits non-zero. No allow line
    can excuse it.
- **D-12:** The job-summary table (`$GITHUB_STEP_SUMMARY`) has these columns: ROM | mapper |
  base Ir/frame | head Ir/frame | delta Ir/frame | delta %, plus a status column (`ok`,
  `ALLOWED`, `FAIL`, `n/a`).
  - The footer gives the base and head SHAs, `valgrind --version`, `gcc-14 --version` and the
    formula.
  - Only fixed ROM names, context SHAs and computed integers are written. Never PR titles,
    branch names or program output.
  - No PR comments are posted.
- **D-13:** The job is `perf` in `ci.yml`, triggered on `pull_request` only, never
  `pull_request_target`.
  - It has `permissions: contents: read`, `persist-credentials: false` and SHA-pinned `uses:`.
  - In report mode it has job-level `continue-on-error: true` and is absent from `CI required`'s
    `needs`. A comment above it says how to arm it.
  - Its timeout is a provisional `timeout-minutes: 10`, then twice the first measured cold run,
    with the run ID in a comment.
  - `tests/cmake/release_policy.cmake` and `nightly_workflow_policy.cmake` are checked and
    updated if they enumerate jobs or `needs`.

### Gate arming and allow file (PERF-02, criterion 3)
- **D-14:** The comparison logic is one script (POSIX `sh` + `awk`, or CMake `-P`, whichever
  matches `tests/cmake` precedent) used by both the job and the self-test.
  - All arithmetic is in integers on the frame-count differences
    `d = Ir(360) - Ir(60)`. FAIL when `(d_head - d_base) * 100 > d_base`, i.e. strictly above
    +1.00%; exactly +1.00% passes.
  - Improvements never fail.
  - A ROM missing on the head side fails once armed.
  - The displayed percentage is computed separately for the table only.
- **D-15:** Zero spread is proven before arming, with evidence recorded in the repo.
  - A `workflow_dispatch` control mode of the `perf` job builds the same commit twice, in two
    different directories (covering path-dependent noise), as "base" and "head".
  - It is run in at least two separate workflow runs, and every ROM must show delta exactly 0
    and `base_a == base_b`.
  - The run IDs, the runner `ImageVersion`, and the valgrind and gcc versions are recorded in
    `.planning/phases/15-speed-and-close-out/15-PERF-PROOF.md`.
  - Arming happens only in a later commit.
  - Cross-image drift needs no proof, because base and head are always built and measured in the
    same job on the same image.
- **D-16:** The gate is armed inside this phase (criterion 3 closes in v3). The arming commit:
  - removes `continue-on-error` and switches the script to gate mode;
  - adds `perf` to `CI required`'s `needs`;
  - changes the roll-up so that `perf` with result `skipped` is accepted only when the event is
    not `pull_request`. `ci.yml` also runs on push and through `release.yml`'s `workflow_call`,
    where `perf` does not run.

  A docs-only PR still runs `perf` (identical binaries, delta 0). There is no path filter.

  If the proof in D-15 shows any spread, the gate is not armed. The phase records that, and
  PERF-02 criterion 3 is reported as unmet rather than forced.
  — **Reversibility:** reversible — a one-commit revert returns to report mode.
- **D-17:** The allow file is `tests/perf/allow.txt`. `#` comments are allowed, and each line is
  `<rom> <max_percent_x100> <expires YYYY-MM-DD> <reason...>`, e.g.
  `nesteroids 350 2026-12-31 PPU timing fix, see #NN`.
  - **Match:** an entry matches by ROM name and caps that ROM's delta. The ROM passes as
    `ALLOWED` if its delta is at most the cap; a delta above the cap fails. Entries are not keyed
    by commit or PR, because squash-merging changes commits.
  - **Validation:** each of the following fails the job with its own message:
    - a malformed line;
    - a duplicate ROM;
    - an unknown ROM;
    - an empty reason;
    - an expiry more than 60 days after the check date;
    - an expired line.

    Once merged, the slowdown is in the base, so the line is dead weight. The PR that adds a line
    should be followed by its removal, and the milestone close-out sweeps any leftovers.
  - **Review:** every applied line is printed in the summary as `ALLOWED`, and the line lives in
    the PR diff and the squash commit. The owner's PR review is the review. There are no
    CODEOWNERS and no waiting step. The accepted risk is that the PR author can add a line, and
    the cap and expiry bound it.
  - The script and allow file are read from head, not checked out from base. The self-test
    (D-18) covers the script in the same PR, and the owner reviews any change to it (ladder:
    remove the need for a base checkout).
- **D-18:** The deliberate slowdown is demonstrated by a command. A ctest
  (`perf.gate_selftest` or similar) runs the comparison script on synthetic `Ir` fixtures in
  `tests/perf/fixtures/`.
  - **Pass cases:**
    - identical numbers;
    - exactly +1.00%;
    - +1.01% with a valid allow line whose cap is at least the delta (output contains
      `ALLOWED`).
  - **Fail cases:**
    - +1.01% with no allow line;
    - a cap below the delta;
    - each of the D-17 validation failures;
    - `base_a != base_b`, which gives `UNSTABLE` even with an allow line.
  - The check date is injectable so expiry cases are deterministic.

### Close-out (TUNE-10)
- **D-19:** Timeouts are measured last. After every other v3 code change has landed (TUNE-10 is
  the last plan), `nightly.yml` is dispatched on the phase branch three times. Every nightly job
  without a measured comment is set to `ceil(2 x slowest of the three)` minutes, with the run IDs
  in the comment, using whole-job duration. That includes:
  - `suite-flake` (currently a 30-minute ceiling; recent samples range from about 7 to 20
    minutes, so the old 172 s single-pass figure does not apply);
  - `state-fuzz` ("not yet measured");
  - the uncommented `nightly.yml` timeouts (lines 63, 133, 194, 332).

  The same rule applies to:
  - the uncommented `ci.yml` timeouts (lines 129, 180), from the phase PR's CI run;
  - `release.yml`'s two "initial" timeouts, from the latest completed release run. If that run
    is not usable, the value stays and the comment names why.

  `perf` gets its value the same way (D-13). A check in `tests/cmake/action_pins.cmake`, or a
  sibling hygiene test, fails any `timeout-minutes:` line without a measured-run comment, so this
  cannot drift back. The samples are recorded in `15-CLOSEOUT.md`.
- **D-20:** The Phase 14 review warnings WR-01 to WR-03 are fixed, because shipping known
  signal-handling and flaky-test debt in the closing release defeats close-out.
  - **WR-01:** keep C17 `signal()` (the Phase 14 decision stands; no `sigaction`).
    - At install, if `signal()` returns `SIG_IGN`, put `SIG_IGN` back. An inherited ignore is
      honoured, as POSIX shells expect for background jobs.
    - The handler saves and restores `errno`.
  - **WR-02:** the stop and signal tests' CTest `TIMEOUT` values become hang detectors of about
    10 s, with a comment saying they are not performance budgets.
  - **WR-03:** the README tells POSIX launchers to keep reading the runner's stdout (or redirect
    it) so a full pipe cannot delay a stop.
  - `14-REVIEW-DISPOSITION.md` marks WR-01 to WR-03 `fixed` with commits, and IN-01 to IN-04
    `deferred` (cosmetic, no behaviour change).
- **D-21:** Review means confirm and record, not hand-bump. The review goes in
  `.planning/phases/15-speed-and-close-out/15-CLOSEOUT.md`: a dated table of each pin (current
  vs latest) and each runner label (status, next retirement), with source URLs. Kept out of the
  README.
  - **Action pins:** all are latest except `actions/create-github-app-token` (pinned v3.2.0,
    v3.3.0 published 2026-10-12). Dependabot owns that bump; merge its PR if it is open when the
    phase closes.
  - **Runner labels:** all six are current.
    - The watch items are `macos-15-intel`, the last x86 image, until August 2027, and
      `macos-15`, which goes at the macOS 27 GA.
    - The `windows-11-arm` move to Visual Studio 2026 is complete, so ENGINEERING §5's stale
      "Visual Studio 2022, due to move" line is updated.
  - **AccuracyCoin:** stays at `673ef550`. Upstream has 9 newer commits (2 new tests, an OAM fix)
    and is still moving; a bump re-baselines the ROM sha256, the scoreboard and the hashes, which
    does not serve closing the milestone. A backlog item records the compare URL.
- **D-22:** "Docs describe the released code":
  - **Automated where cheap:** a test checks that every `--option` in `--help` appears in the
    README's runner section, and that every README runner option exists in `--help`. This
    extends the existing README check if one covers options; otherwise it is a new CMake `-P`
    test.
  - **Everything else:** a one-off audit of the README and `include/nesturbator.h` comments
    against the Phase 12–15 changes, recorded in `15-CLOSEOUT.md`.
  - No prose-truth checker.

### Claude's Discretion
- Exact module and file names (`runner/clock.c` vs `runner/bench.c`), and whether the
  percentile helper lives in the bench module.
- Script language for the comparison (`sh` + `awk` vs CMake `-P`), following the nearest
  `tests/cmake` precedent.
- Plan split and ordering, except that the gate is armed only after `15-PERF-PROOF.md` exists
  and TUNE-10 timeouts are measured last.

### Deferred Ideas (OUT OF SCOPE)

- Bump AccuracyCoin past `673ef550` (re-baseline the ROM hash, scoreboard and hashes). This is a
  backlog item.
- Phase 14 IN-01 to IN-04 (SIGHUP, unreachable EINTR loops, duplicated `parse_kind`, close-wait
  statics comment). Backlog.
- `--bench` with `--load-state` or `--movie`, and a stdout or `--bench-out` machine output.
  Add only on request.
- A lazy PPU schedule and other speed work (SEED-004), accepted only with identical hashes.

</deferred>
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| PERF-03 | `--bench N` prints fps and p50/p90/p99/p99.9/max frame times; `--bench-state` adds save+load cost; a test shows benchmarking changes no hash; wall time never gated | Runner map (main.c anchors), clock module recipe, nearest-rank formula + integer formatting, test design (`runner.bench.*`), README/`--help` two-way check |
| PERF-02 | Every PR: Cachegrind job, Ir per frame head vs merge base on NROM/MMC1/MMC3, table in job summary, report mode first, +1% gate armed only after zero-spread proof, allow-file line passes a deliberate slowdown | Measured zero-spread evidence and the one trap (argv[0] length), job skeleton, CMake `-P` comparison script, allow-file validation, roll-up `jq`, selftest matrix |
| TUNE-10 | Nightly suite-flake timeout about 2x a measured cold run after all v3 tests exist; pins, runner labels, AccuracyCoin pin reviewed; README and public header describe released code | Exact timeout-line inventory with line numbers, measurement commands, lint rule, pin/runner evidence, stale-doc findings |
</phase_requirements>

## Summary

All 22 decisions are implementable with what the repo and the hosted images already have; no new dependency is needed. The bench mode is a small change to the single frame loop in `runner/main.c` (loop at 1261-1331) plus two new runner-only modules (`clock.c`, `bench.c`) following the `stop.c` pattern. The Cachegrind job is a CI job plus one CMake `-P` script; I recommend CMake over `sh`+`awk` because the gate self-test must run in `cmake --workflow --preset ci` on all six platforms (including Windows/MSVC, which has no guaranteed awk), CMake `math(EXPR)` is signed 64-bit (verified to 7.5e11), and every existing test script in `tests/cmake` is CMake.

The most valuable finding is empirical. In an Ubuntu 24.04 container (valgrind 3.22.0, gcc-14.2, `ci` preset) the per-frame difference `Ir(360) - Ir(60)` was **bit-identical** across repeated runs and across two builds in different directories on all three ROMs, **provided base and head are invoked through paths of equal length**. When the invocation path length differed (4 vs 7 characters in the directory name), the delta moved by 9 to 119 instructions (about 1e-8 relative). That is far under the 1% gate but it would break D-15's "delta exactly 0" proof. The binaries themselves were byte-identical across build directories, so the fix is cheap: copy each built binary to a fixed-length path (`$RUNNER_TEMP/bin/base/nesturbator-run`, `$RUNNER_TEMP/bin/head/nesturbator-run`) and run from there under `env -i`.

TUNE-10 is mostly bookkeeping, but two traps need a planner decision: D-20 asks for ~10 s CTest `TIMEOUT` hang detectors on tests whose helpers carry 10 s + 5 s (POSIX) and 20 s + 2 s + 5 s (Windows) internal deadlines, so a 10 s CTest timeout fires first and bypasses the helper's own cleanup; and D-19's `ceil(2 x measured)` rule would set `mmc3-oracle` to about 1 minute although its network fetch has a 300 s CTest allowance (see Open Questions 1 and 2).

**Primary recommendation:** Build in this order: (1) bench mode + tests + README, (2) the CMake comparison script + selftest + fixtures, (3) the `perf` job in report mode with a `workflow_dispatch` control, (4) WR-01..WR-03, (5) record `15-PERF-PROOF.md` from two control runs and only then arm, (6) TUNE-10 last (lint, three nightly dispatches, timeouts, `15-CLOSEOUT.md`, docs audit).

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| Frame timing, percentiles, fps formatting | Runner (`runner/clock.c`, `runner/bench.c`) | — | Wall time is a host concern; the core stays integer-only and links only memory functions (CLAUDE.md rule 5) |
| Save+load cost sampling | Runner, via public `nesturbator_save_state`/`nesturbator_load_state` | Library (unchanged) | Uses only the public state API; no library change |
| Instruction counting | CI job on `ubuntu-24.04` (valgrind, system tool) | CMake `-P` script | Measures the shipped binary from outside; no `valgrind.h` in the tree |
| Gate decision, allow-file validation | CMake `-P` script `tests/cmake/perf_compare.cmake` | `ci.yml` calls it | One script for job and self-test (D-14) |
| Required-check roll-up | `ci-required` job (`jq`) | — | Existing mechanism; edited only when arming |
| Timeout/pin hygiene | `hygiene` lane test (cmake `-P`) | `15-CLOSEOUT.md` record | Follows `hygiene.action_pins` precedent |
| Docs-match-code | `ctest` cmake `-P` (`--help` vs README) | One-off audit | D-22 |

## Standard Stack

### Core
| Library / tool | Version | Purpose | Why Standard |
|----------------|---------|---------|--------------|
| C17 stdlib (`qsort`, `snprintf`, `malloc`) | — | Sample sort, formatting | Already used by `runner/main.c`; ponytail ladder step 3 |
| POSIX `clock_gettime(CLOCK_MONOTONIC)` | POSIX.1-2008 | Monotonic ns on Linux/macOS | [CITED: pubs.opengroup.org/onlinepubs/9699919799/functions/clock_getres.html] |
| Win32 `QueryPerformanceCounter`/`Frequency` | Windows XP+ | Monotonic ticks on Windows | [CITED: learn.microsoft.com/windows/win32/sysinfo/acquiring-high-resolution-time-stamps] |
| Valgrind Cachegrind | 3.22.0 (Ubuntu noble apt `1:3.22.0-0ubuntu3`) | `Ir` per run | [VERIFIED: apt in ubuntu:24.04 container this session] |
| CMake `-P` scripts | 3.25+ (image has 3.31.6) | Comparison, selftest, lints | Every script in `tests/cmake` is CMake [VERIFIED: `ls tests/cmake`] |
| `jq` | 1.7 on image | `ci-required` roll-up | Already used at ci.yml:320 |

### Supporting
| Tool | Version | Purpose | When to Use |
|------|---------|---------|-------------|
| `gh` | 2.102.0 on image; local authenticated | Dispatch runs, read job durations | TUNE-10 and PERF-PROOF evidence |
| `git worktree` | any | Build merge-base copy | `perf` job |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| CMake `-P` comparison | `sh` + `awk` | awk would not run the selftest on Windows; mawk (Ubuntu's default awk) integer printing for large values is a known risk [ASSUMED] |
| `apt-get install valgrind` | a pinned `.deb`/source build | valgrind is **not** preinstalled on the image [VERIFIED: runner-images Ubuntu2404-Readme, image 20261004.327.1, no valgrind entry]; apt is the minimal path; the version is recorded in the summary footer |

**Installation (job step):**
```bash
sudo apt-get update && sudo apt-get install -y valgrind   # pulls libc6-dbg (a hard Depends)
```
Measured install in a clean ubuntu:24.04 container: 9 s for valgrind alone [VERIFIED: container run]; expect 15 to 40 s on a hosted runner [ASSUMED]. Do not use `--no-install-recommends`; `libc6-dbg` is a `Depends`, and valgrind needs it for ld.so redirections [VERIFIED: `apt-cache show valgrind` => `Depends: libc6 (>= 2.38), libc6-dbg`].

## Package Legitimacy Audit

No npm/PyPI/crates packages are installed by this phase. The only new install is `valgrind` from the Ubuntu archive via apt (a system tool, not a code dependency; version recorded in the summary footer per D-08/D-12). The package-legitimacy seam covers npm/pypi/crates and does not apply.

| Package | Registry | Age | Downloads | Source Repo | Verdict | Disposition |
|---------|----------|-----|-----------|-------------|---------|-------------|
| valgrind | Ubuntu noble archive (apt) | decades | n/a | valgrind.org / Ubuntu | n/a (not an npm/PyPI/crates package) | Approved: system tool, `1:3.22.0-0ubuntu3` observed |

**Packages removed due to [SLOP] verdict:** none
**Packages flagged as suspicious [SUS]:** none

## Architecture Patterns

### System Architecture Diagram

```
PERF-03 (bench)                              PERF-02 (perf job, pull_request only)
argv --bench N [--bench-state]               checkout (fetch-depth 2, merge commit = head)
   |                                            |-- git worktree add $RUNNER_TEMP/base HEAD^1  (control: HEAD)
parse_options -> usage() exit 2 on conflicts    |-- cmake --preset ci ; build --target nesturbator-run  (x2 trees)
   |                                            |-- cp binaries -> $RUNNER_TEMP/bin/{base,head}/nesturbator-run  (equal-length paths)
load ROM (existing path)                        |-- apt-get install valgrind
   |                                            v
frame loop n=1..N:                            for rom in nesteroids runner M4_P256K_C256K:
  [t0] run_frame [t1]  -> frame_ns[n]            for who in base_a base_b head:
  hash/audio/stdout (untimed)                       for n in 60 360:
  if bench-state: [t2] save [t3] [t4] load [t5]       env -i PATH=... valgrind --tool=cachegrind --cache-sim=no
   |                                                     --cachegrind-out-file=F runner --rom R --frames n >/dev/null
stop signal? -> break (stats over done frames)   Ir = "summary:" line of F
   |                                            |
sort copies (qsort) -> nearest-rank p50..max    cmake -P tests/cmake/perf_compare.cmake  (reads F files, allow.txt)
   |                                               d = Ir360-Ir60 ; FAIL if (dh-db)*100 > db ; UNSTABLE if base_a!=base_b
stderr: "nesturbator-run: bench ..." lines       |-- markdown table -> $GITHUB_STEP_SUMMARY (integers + fixed names only)
stdout: hashes only (unchanged)                 `-- exit status (report mode: job continue-on-error; gate mode: in `needs`)
```

### Recommended Project Structure
```
runner/
├── clock.c / clock.h        # uint64_t nesturbator_run_clock_ns(void)  (D-04); #ifdef _WIN32 like stop.c
├── bench.c / bench.h        # sort, nearest-rank index, fixed-decimal formatting (pure, unit-testable)
├── main.c                   # --bench/--bench-state parse + timed loop + report
tests/
├── cmake/perf_compare.cmake        # comparison + allow-file validation (job and selftest)
├── cmake/runner_bench.cmake        # MODE=nohash|format
├── cmake/runner_options.cmake      # --help <-> README option two-way check (D-22)
├── cmake/timeout_comments.cmake    # hygiene lint (D-19)
├── perf/allow.txt                  # '#' comments only at first
├── perf/fixtures/*.out             # synthetic cachegrind.out files (summary: lines)
└── runner/test_bench.c             # percentile + formatting unit test (compiled with runner/bench.c)
```

### Pattern 1: Clock module (D-04)
**What:** One function, platform split by `#ifdef _WIN32`, header-isolated like `stop.c`/`stop_internal.h`.
**Verified facts:** Under the project's `-std=c17` (CMAKE_C_EXTENSIONS OFF, CMakeLists.txt:12-14), Linux glibc **does not declare `clock_gettime`** without a feature-test macro: gcc-14 `-std=c17 -Wall -Wextra -pedantic -Werror` fails with `implicit declaration of function 'clock_gettime'` [VERIFIED: ubuntu:24.04 container this session]. With `#define _POSIX_C_SOURCE 199309L` it compiles clean on Linux and macOS [VERIFIED: gcc-14 container + local clang]. `stop.c` already uses `#define _DEFAULT_SOURCE` (runner/stop.c:1), which also exposes it; prefer the narrower `_POSIX_C_SOURCE 199309L` in `clock.c`.
```c
/* runner/clock.c - macro first, before any include */
#define _POSIX_C_SOURCE 199309L
#include "clock.h"
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
uint64_t nesturbator_run_clock_ns(void)
{
    static uint64_t freq; /* fixed at boot; query once (MS Learn: "fixed at system boot ... cache the result") */
    LARGE_INTEGER c;
    uint64_t ticks, sec, rem;
    if (freq == 0u) {
        LARGE_INTEGER f;
        if (!QueryPerformanceFrequency(&f) || f.QuadPart <= 0)
            return 0u;
        freq = (uint64_t)f.QuadPart;
    }
    if (!QueryPerformanceCounter(&c))
        return 0u;
    ticks = (uint64_t)c.QuadPart;
    sec = ticks / freq;
    rem = ticks % freq; /* rem < freq, so rem * 1e9 fits in uint64 for any freq below 1.8e10 */
    return sec * UINT64_C(1000000000) + rem * UINT64_C(1000000000) / freq;
}
#else
#include <time.h>
uint64_t nesturbator_run_clock_ns(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
        return 0u;
    return (uint64_t)ts.tv_sec * UINT64_C(1000000000) + (uint64_t)ts.tv_nsec;
}
#endif
```
Microsoft's own example multiplies by 1,000,000 before dividing (`ElapsedMicroseconds.QuadPart *= 1000000`) and warns that multiplying 64-bit integers can overflow; the sec/rem split above avoids both loss and overflow. QPC frequency can be 10 MHz under a hypervisor and 3.125 MHz on some machines [CITED: learn.microsoft.com acquiring-high-resolution-time-stamps]. A return of 0 from a failed clock call must make the runner report a clock error (exit 1) or the stats are garbage; handle by checking `t1 < t0` is impossible for a monotonic clock and treating `0` start as failure.

### Pattern 2: Nearest-rank percentile and integer formatting (D-05, D-06)
Formula `idx = (p_x10 * N + 999) / 1000 - 1` with `p_x10` in {500, 900, 990, 999} equals `ceil(p*N) - 1`. Edge values I derived and checked by hand:
- N=1: every percentile index is 0 (`(500+999)/1000 - 1 = 0`).
- N=2, p50: `(1000+999)/1000 - 1 = 0` (first element, nearest rank).
- N=999: p99.9 index `(999*999+999)/1000 - 1 = 998` = last = max. N=1000: `(999000+999)/1000 - 1 = 998`, the second-to-last, so p99.9 != max from N=1000 up (matches D-05's "N < 1000 => p99.9 equals max").
- Use `uint64_t` for the product (N up to 1e6, p up to 999 => 1e9 fits uint32 but the sum does not need to). Samples 1..1000 give p50=500, p90=900, p99=990, p99.9=999, max=1000: use as the known-value unit test, plus N=999 (1..999: p99.9 = 999 = max) and N=1.
Formatting (no `%f`): ns -> `"%llu.%03llu"` of `ns / 1000` and `ns % 1000` (microseconds, 3 decimals; truncation, document it); `time_ms`: `total / 1000000` and `(total / 1000) % 1000`; fps x100 = `(frames * 100 * 1e9 + total/2) / total` (frames <= 1e6 gives <= 1e17, fits uint64), printed `"%llu.%02llu"`. Guard `total == 0` (print `fps=0.00`) so a coarse clock cannot divide by zero. Compiler flags include `-Wconversion -Wsign-conversion` (CMakeLists.txt:20): cast every width change and print `unsigned long long`.
Sort a **copy** per series with `qsort` and a comparator that uses `(a > b) - (a < b)`, not subtraction. Note `time_ms` is the **sum of unsorted** samples, computed before sorting.

### Pattern 3: Bench in the existing loop
- Parse: `--bench-state` is a standalone switch, so put it beside `--hash-audio` (runner/main.c:337-344, before `value = ...` at line 345) so it does not consume the next argument. `--bench N` is a value option next to `--frames` (main.c:411-420); on success set `o->bench = 1` and `o->frames = n`; reject `n > 1000000` with `usage()`; reject a second `--bench`.
- The final validation block (main.c:426-449) is where all D-01/D-03 exit-2 checks go; the current last check `o->frames == 0u && o->movie_path == NULL` ("--frames N is required") must keep firing for a run with neither `--frames`, `--movie` nor `--bench`. `--frames` plus `--bench` is a conflict, so record whether `--frames` was given separately (today `frames` is overwritten silently by a repeat `--frames`).
- Allocate the sample arrays (`frames`, and `save`/`load` when `--bench-state`) immediately before the loop (after main.c:1259) and store the pointers in `options` so `free_options` (main.c:230-242) frees them on every early return; failure prints `nesturbator-run: out of memory` and returns `RUNNER_EXIT_FAILURE`, as parse_options does (main.c:322-324).
- Timing: wrap only `nesturbator_run_frame(inst, &io)` (main.c:1304) with two clock reads; `--bench-state` times `nesturbator_save_state(inst, buf, size)` then `nesturbator_load_state(inst, buf, size)` after the hash/dump block and before the next iteration; buffer sized once from `nesturbator_state_size(inst)` (it is constant from a successful load) [VERIFIED: include/nesturbator.h:440-446, 499-501]. Failure of either: print a `nesturbator-run:` message and `status = RUNNER_EXIT_FAILURE` (exit 1, D-02).
- Reporting: print the bench lines **before** the stop line (main.c:1344-1346 prints `stopped by signal after frame`) so "stats, then the stop line" holds; with zero completed frames print only `bench frames=0` (an edge CONTEXT does not specify; Open Question 4).
- Accuracy-page and movie interplay: `--bench` with `--accuracycoin-page`, `--raise-after`, `--dump-frame`, `--state-at-frame`, `--hash-state`, `--save-dir`, `--save-interval`, `--load-state`, or without `--rom` => `usage()` exit 2 (D-03); `--bench` with `--frames` or `--movie` => exit 2 (D-01). `--hash-frame K` with K > N is already rejected by `check_frame_limits` (main.c:972-985).

### Pattern 4: `perf` job skeleton (report mode)
```yaml
  # Report mode (D-13). To arm: delete `continue-on-error`, add `perf` to ci-required `needs`,
  # and switch the script call to MODE=gate. See 15-PERF-PROOF.md.
  perf:
    if: github.event_name == 'pull_request' || (github.event_name == 'workflow_dispatch' && inputs.control == true)
    runs-on: ubuntu-24.04
    continue-on-error: true
    timeout-minutes: 10 # provisional; set to twice the first measured cold run (run <id>)
    permissions:
      contents: read
    steps:
      - uses: actions/checkout@3d3c42e5aac5ba805825da76410c181273ba90b1 # v7.0.1
        with:
          fetch-depth: 2
          persist-credentials: false
      - name: Toolchain
        run: |
          echo "CC=gcc-14" >> "$GITHUB_ENV"
          echo "CXX=g++-14" >> "$GITHUB_ENV"
          sudo apt-get update && sudo apt-get install -y valgrind
      - name: Build base and head
        run: |
          base=HEAD^1; [ "$GITHUB_EVENT_NAME" = workflow_dispatch ] && base=HEAD   # control: same commit twice
          git worktree add --detach "$RUNNER_TEMP/base" "$base"
          for t in "$RUNNER_TEMP/base" "$GITHUB_WORKSPACE"; do
            (cd "$t" && cmake --preset ci && cmake --build --preset ci --target nesturbator-run)
          done
          mkdir -p "$RUNNER_TEMP/bin/base" "$RUNNER_TEMP/bin/head"        # equal-length names on purpose
          cp "$RUNNER_TEMP/base/build/ci/runner/nesturbator-run" "$RUNNER_TEMP/bin/base/"
          cp "$GITHUB_WORKSPACE/build/ci/runner/nesturbator-run"  "$RUNNER_TEMP/bin/head/"
```
Then a measure step with the 18 `env -i PATH=/usr/bin:/bin valgrind ...` runs writing `$RUNNER_TEMP/perf/<who>_<rom>_<n>.out` (keep valgrind stderr in a file; print its tail only on failure), then `cmake -P tests/cmake/perf_compare.cmake ... >> "$GITHUB_STEP_SUMMARY"`. If the base build fails, skip its runs and pass the script an `n/a` marker (D-10).
- `git worktree add` on a depth-2 clone works and `HEAD^1`/`HEAD^2` both exist [VERIFIED: local experiment: `git clone --depth=2` of a repo whose tip is a merge commit; `git rev-parse HEAD^1 HEAD^2` both resolved and `git worktree add ../wt HEAD^1` succeeded]. For `pull_request`, `GITHUB_SHA` is "the SHA of the merge commit on the merge branch" [CITED: docs.github.com events-that-trigger-workflows#pull_request]. Guard against a non-merge HEAD: `[ "$(git rev-list --parents -n1 HEAD | wc -w)" = 3 ]`, else report `n/a` (the claim that GitHub always builds a true merge commit is [ASSUMED]).
- `ci.yml` has `workflow_dispatch:` with no inputs today (ci.yml:12); add `inputs: { control: { type: boolean, default: false } }`. Dispatching `ci.yml` runs **every** job (six builds, asan, retroarch-e2e on macOS), so each control run costs a full CI. Option to avoid that: gate the heavy jobs with `if: inputs.control != true`; `ci-required` is not required on a dispatch. Leave as a planner choice (Open Question 3). `inputs.control == true` typing in `if:` is [ASSUMED]; the first control run verifies it.
- Concurrency: group is `ci-<workflow>-<sha>` for non-PR events with `cancel-in-progress` false (ci.yml:26-28); dispatch the second control run only after the first finishes, or a pending run is replaced [ASSUMED from GitHub concurrency semantics].
- Title edits re-run PR CI (`types` includes `edited`, ci.yml:9) and cancel the in-progress run, so `perf` re-runs on every title edit; harmless.

### Pattern 5: Comparison script (D-14, D-17)
Inputs: `-DMODE=report|gate`, `-DDIR=<cachegrind outputs>`, `-DALLOW=tests/perf/allow.txt`, optional `-DTODAY=YYYY-MM-DD` (default `string(TIMESTAMP today "%Y-%m-%d" UTC)`), `-DOUT=<markdown file>`. Reads each `summary:` line: `file(STRINGS f l REGEX "^summary:")` then `^summary: *([0-9]+)$`; the file also has `events: Ir` [VERIFIED: container cachegrind.out head: `cmd: ...`, `events: Ir`; Valgrind manual grammar: `"summary:" ws? count (ws+ count)+` means one count per event].
Gate in integers (verified 64-bit): `d = Ir360 - Ir60`; FAIL when `(d_head - d_base) * 100 > d_base`. Largest value seen: `7570804574 * 100 = 757080457400` computes exactly in `math(EXPR)` on CMake 4.4.3 [VERIFIED: local]. Displayed percent (table only): `pct_x100 = delta * 10000 / d_base` with the sign handled separately because `math(EXPR)` truncates toward zero.
Date arithmetic for the allow file (needed for "more than 60 days after the check date"): `math(EXPR)` has no ternary, so use the non-negative-year form of Hinnant's days-from-civil with leading zeros stripped (CMake reads `08` as invalid octal). This function was run and matched Python: `1970-01-01=0`, `2026-10-12=20738`, `2026-12-11=20798`, difference 60 [VERIFIED: local cmake -P vs python `datetime`]:
```cmake
function(days_from_civil out y m d)           # y >= 1970, m and d without leading zeros
  if(m LESS_EQUAL 2)
    math(EXPR y "${y} - 1")
  endif()
  math(EXPR era "${y} / 400")
  math(EXPR yoe "${y} - ${era} * 400")
  if(m GREATER 2)
    math(EXPR mp "${m} - 3")
  else()
    math(EXPR mp "${m} + 9")
  endif()
  math(EXPR doy "(153 * ${mp} + 2) / 5 + ${d} - 1")
  math(EXPR doe "${yoe} * 365 + ${yoe} / 4 - ${yoe} / 100 + ${doy}")
  math(EXPR days "${era} * 146097 + ${doe} - 719468")
  set(${out} ${days} PARENT_SCOPE)
endfunction()
```
Note: save `CMAKE_MATCH_n` into variables before any further `string(REGEX REPLACE)` (the match variables do not survive; my first prototype failed on this).
Allow-file lines: `^[A-Za-z0-9_.-]+ [0-9]+ [0-9]{4}-[0-9]{2}-[0-9]{2} .+$`; the six D-17 failures each get a distinct message; ROM names = `nesteroids`, `runner`, `M4_P256K_C256K` (the script's own list of three; "unknown ROM" is any other name). ROM-file-to-mapper labels: nesteroids = mapper 0, runner = mapper 1, M4_P256K_C256K = mapper 4 [VERIFIED: iNES headers `4e45531a 02 01 00 00`, `4e45531a 04 02 13 08` (flags6 low nibble 1, NES 2.0 flag7 high nibble 0), `4e45531a 10 20 40 08` (flags6 low nibble 4)].

### Pattern 6: Roll-up after arming (D-16)
```yaml
    needs: [hygiene, title, build, asan, nofp, hash-equality, retroarch-e2e, perf]
    steps:
      - name: Every job succeeded
        env:
          NEEDS: ${{ toJSON(needs) }}
          EVENT: ${{ github.event_name }}
        run: >-
          echo "$NEEDS" | jq -e --arg event "$EVENT"
          'to_entries | all(.value.result == "success" or (.key == "perf" and .value.result == "skipped" and $event != "pull_request"))'
```
`github.event_name` inside a `workflow_call` workflow is the caller's event, so release.yml's `ci` job (push) skips `perf` and still passes; the event payload and `GITHUB_SHA` are the caller's [CITED: docs.github.com events-that-trigger-workflows, workflow_call] and ci.yml:23-25 already relies on it. On `workflow_dispatch` without `control`, `perf` is skipped and accepted. This must be a `jq` edit of the existing line ci.yml:320, which is the whole roll-up today (`echo "$NEEDS" | jq -e 'to_entries | all(.value.result == "success")'`). No policy test reads `ci-required` or enumerates ci.yml jobs [VERIFIED: `grep -rln "ci-required\|CI required" tests scripts` only matches `tests/hygiene/fixtures/pins_good.yml` text and workflow files; `release_policy.cmake` constrains only release.yml's `needs: [release-please, ci]` and `needs: release-please` lines (release_policy.cmake:9-12); `nightly_workflow_policy.cmake` checks only the `vectors-full`, `mmc3-oracle` and `report` jobs of nightly.yml], so no policy test needs an update for D-13, but the arming change deserves its own small test (see Validation).

### Anti-Patterns to Avoid
- Running base and head from different-length paths (shifts Ir by tens of instructions; breaks the zero-spread proof).
- Reading `Ir` from valgrind's stderr `I refs:` (comma formatted); read `summary:`.
- Writing anything from `cachegrind.out` other than integers to the summary (it contains `cmd:` with absolute paths).
- Using `--toggle-collect`, `valgrind.h`, `timespec_get`, `clock()`.
- Arming and recording the proof in the same commit.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Sorting samples | custom sort | `qsort` with a 3-way comparator | stdlib, 1e6 elements is fast |
| Monotonic time | `clock()`, `time()`, `timespec_get`, RDTSC | `clock_gettime(CLOCK_MONOTONIC)` / QPC | `CLOCK_REALTIME`/`TIME_UTC` can step; MS discourages RDTSC |
| Instruction counting | `valgrind.h` hooks, perf counters | Cachegrind `summary:` | No vendored header; works on unmodified PRs |
| 64-bit integer math in scripts | awk doubles / shell `$(( ))` on Windows | CMake `math(EXPR)` | Signed 64-bit, identical on six platforms |
| Date diff | shelling out to `date -d` | `days_from_civil` above | BSD vs GNU `date` differ; selftest needs injectable date |
| Run-time lookup of job durations | scraping logs | `gh run view <id> --json jobs --jq` | Verified below |

## Runtime State Inventory

Not a rename/refactor/migration phase. Omitted.

## Repo Map (file:line anchors for the planner)

| Decision | Where | Anchors |
|----------|-------|---------|
| Usage/`--help` text | `runner/main.c` `print_usage` | 72-102 (options block 82-96; `Exit status:` table printed via `RUNNER_EXIT_TABLE` at 97-101) |
| Option parser | `runner/main.c` `parse_options` | 304-451; `--hash-audio` switch 337-344; `--frames/--hash-frame` 411-420; conflicts 425-449 |
| Frame limit checks | `check_frame_limits` / `check_ranges` | 969-995 / 997-1032 |
| Main loop | `main` | loop 1261-1331; stop check 1264-1267; `nesturbator_run_frame` 1304; exit tail 1333-1363 (stop line 1344-1346) |
| Exit statuses | `runner/exit_status.h` | 13-20: `X(RUNNER_EXIT_DONE, 0, "done")` ... `X(RUNNER_EXIT_STATE_REJECTED, 5, "state rejected")`, `RUNNER_EXIT_RESERVED_TEXT "3 and 77 are reserved"` |
| Stop module | `runner/stop.c` | `#define _DEFAULT_SOURCE` line 1; POSIX handler 17-29; Windows 67-180 |
| Runner target | `runner/CMakeLists.txt:1` | `add_executable(nesturbator-run main.c movie.c sha256.c audio_hash.c ppm.c save.c state_file.c stop.c)` (add `clock.c bench.c`) |
| README runner section | `README.md` | `## The runner` 1009; option bullets 1031-1178; `### Stopping the runner` 1227 (POSIX paragraph 1229, Windows launcher advice ends 1241); `### Exit status` 1244 |
| README CI section | `README.md` | `## Continuous integration` 584-695 (no `perf` mention yet) |
| `runner.exit.*` tests | `tests/CMakeLists.txt:1061-1069`; script `tests/cmake/runner_exit_table.cmake` | `MODE=table|help`; the only option check is `--raise-after` (35-38, 141-145) |
| runner stop tests | `tests/CMakeLists.txt:189-224`; `tests/cmake/runner_stop.cmake` | `TIMEOUT 30 LABELS exit_status` (198) |
| signal/console timeouts | `tests/CMakeLists.txt:234` (`set(NESTURBATOR_SIGNAL_TIMEOUT 2)`, mis-indented inside `if(NOT WIN32)`), `:241`, `:271-272` (`TIMEOUT 1`) | helper deadlines `tests/runner/test_signal.c:35-37` (`READY_DEADLINE_MS 10000`, `EXIT_DEADLINE_MS 5000`, `STOP_LIMIT_MS 2000`), `tests/runner/test_console.c:23-25` (`SAVE_WAIT_MS 20000u`, `ATTACH_WAIT_MS 2000u`, `EXIT_WAIT_MS 5000u`) |
| Runner test helper | `tests/CMakeLists.txt:1-23` `nesturbator_runner_test(NAME ARGS EXIT IGNORE_STDOUT EXPECT EXPECT_ERR)` | `EXPECT_ERR` is one substring only; format checks need a cmake `-P` script |
| Direct-compile unit tests | `tests/CMakeLists.txt:971-981` (`runner.sha256`, `runner.audio_hash`) | pattern for `runner.bench` compiled with `runner/bench.c` |
| Script-policy test | `tests/cmake/script_policy.cmake` | every `tests/cmake/*.cmake` needs `cmake_minimum_required(VERSION 3.25)` as its own line (new scripts included) |
| Hygiene lane | `tests/hygiene/CMakeLists.txt:16-27` (`hygiene.action_pins`, `.bad`, `.good` with `tests/hygiene/fixtures/pins_*.yml`), `46-49` labels | place the timeout lint here |
| ci.yml | jobs: hygiene 31, title 41, build 67, asan 124, nofp 141, hash-equality 161, retroarch-e2e 178, ci-required 310 (`needs` at 313, `jq` line 320) | triggers 7-13; concurrency 26-28 |
| Required check | `.github/rulesets/main.json` | `"context": "CI required"`; squash only |
| release.yml | `ci` job `uses: ./.github/workflows/ci.yml` line 60 with `permissions: contents: read, pull-requests: read` (61-63) | timeouts 24, 69 |
| Dependabot | `.github/dependabot.yml` | `github-actions` weekly only |

### `timeout-minutes` inventory (all 16 lines)

| File:line | Value | Today's comment | Status under the lint |
|-----------|-------|-----------------|-----------------------|
| ci.yml:33 | 1 | same line: cold run 37084642541 | pass |
| ci.yml:43 | 1 | same line: run 37084642541 | pass |
| ci.yml:69 | 22 | same line: run 38295163207 | pass |
| ci.yml:129 | 14 | comment lines 126-128 above: "6m45s on PR #44 run 38295163207" | pass (comment is above, not on the line) |
| ci.yml:143 | 2 | same line: run 37084642541 | pass |
| ci.yml:164 | 1 | same line: run 37084642541 | pass |
| ci.yml:180 | 30 (`retroarch-e2e`) | none | **needs measurement** |
| ci.yml:315 | 1 | same line: run 37084642541 | pass |
| nightly.yml:63 | 16 (`vectors-full`) | comment above: cold run 37136095512 | pass |
| nightly.yml:133 | 8 (`mmc3-oracle`) | comment above says "No cold run of this lane has been measured yet" | **needs measurement** (see Open Question 2) |
| nightly.yml:194 | 3 (`rom-loader-fuzz`) | none | **needs measurement** |
| nightly.yml:238 | 3 (`state-fuzz`) | "not yet measured" | **needs measurement** |
| nightly.yml:303 | 30 (`suite-flake`) | "WR-02 ceiling until a cold run is measured" | **needs measurement** |
| nightly.yml:332 | 1 (`report`) | none | **needs measurement** |
| release.yml:24 | 30 (`release-please`) | "initial" | **needs measurement** |
| release.yml:69 | 30 (`publish`) | "initial" | **needs measurement** |

`[VERIFIED: grep -n "timeout-minutes" -B1 .github/workflows/*.yml, this session]`. Plus the new `perf` job. That is 9 existing lines to fix. D-19's "uncommented ci.yml lines 129, 180" is slightly off: line 129 already carries a measured run in the three comment lines above it; only 180 lacks one.

**Lint rule (D-19):** a `timeout-minutes:` line passes if its own trailing comment, or the contiguous `#` comment lines directly above it (up to the previous non-comment line), contain `runs? [0-9]{9,}` (a run ID is 11 digits today). With this rule exactly the nine lines above fail today; ci.yml:129 and nightly.yml:63 pass. Fixtures: `timeouts_bad.yml`, `timeouts_good.yml` in `tests/hygiene/fixtures/`, registered like `hygiene.action_pins.bad/.good` with `WILL_FAIL TRUE`.

### Measured durations available now (run IDs for the comments)

- Release: latest run with a real publish is 38093301725 (`release 0.1.9`): `publish` 73 s, `release-please` 14 s; slowest publish observed 90 s (38081193284); slowest release-please 24 s (38306108774). So `publish` => `ceil(2 x 90 s / 60)` = 3 min, `release-please` => 1 min [VERIFIED: `gh run view <id> --json jobs`, 30 runs]. Latest completed release run 38329002725: `release-please` 22 s, publish skipped; so D-19's "latest completed release run" gives 1 min for `release-please` but cannot measure `publish` from it, use 38093301725 and say so in the comment.
- Nightly main push 38329002573: `suite-flake` 719 s, `vectors-full` 145 s, `mmc3-oracle` 20 s, `rom-loader-fuzz` 73 s, `state-fuzz` 72 s, `report` 5 s [VERIFIED: gh]. Whole-run samples range 7m07 to 19m37 across Phase 14 PR runs; `suite-flake` is the long pole and varies with runner speed, hence three fresh dispatches after the last v3 test lands.
- Command (works with the authenticated `gh` here): `gh run view <id> --json jobs --jq '.jobs[] | "\(.name) \((.completedAt|fromdate)-(.startedAt|fromdate))"'`. Dispatch: `gh workflow run nightly.yml --ref gsd/phase-15-speed-and-close-out` (nightly.yml has `workflow_dispatch`, line 17).

## Common Pitfalls

### Pitfall 1: Invocation path length changes Ir (the zero-spread trap)
**What goes wrong:** Base and head are built in different directories and run by their build paths; the deltas differ by 9 to 119 instructions.
**Why:** argv[0]/AT_EXECFN length moves the initial stack layout; the Cachegrind manual says counts are sensitive to "file name lengths" [CITED: valgrind.org/docs/manual/cg-manual.html, determinism notes: highly reproducible, sensitive to executable and library sizes and file name lengths, and ASLR].
**Evidence:** containers, valgrind 3.22.0, aarch64, `ci` preset gcc-14.2.0, ROMs nesteroids/runner/M4_P256K_C256K, `Ir(360)-Ir(60)`:
- `/w/aaaa` vs `/w/bbbb` builds (same length): identical deltas 7570804574 / 7781973335 / 7450757451, and a repeat run identical.
- `/w/ccccccc` build: 7570804693 / 7781973427 / 7450757460 (+119, +92, +9).
- Binaries copied to `/t/base/nesturbator-run` and `/t/head/nesturbator-run`: identical deltas for all three ROMs, and `cmp` showed the binaries byte-identical across build directories.
- `Ir(60)` alone moved by about 3.4k when `env -i` was dropped and by 3.8k more with a 3000-byte env var, which is exactly why the job runs under `env -i` and why differencing is used (startup cancels in the delta).
**How to avoid:** copy to equal-length paths, `env -i PATH=/usr/bin:/bin`, same cwd, same ROM path for both. Evidence is aarch64 under Docker; the hosted runner is x64, so `15-PERF-PROOF.md` (two real runs) remains the arming gate [ASSUMED for x64 until that proof].
**Warning signs:** `delta != 0` in control mode with `base_a == base_b`.

### Pitfall 2: Host CPU features in libc ifuncs
Different runner hosts can pick different glibc `memcpy` variants. Base and head run in the same job on the same host, so it cancels; never compare numbers across jobs or store absolute Ir (D-21's note is right).

### Pitfall 3: `ci-required` and skipped jobs
GitHub reports a skipped job as success but the `jq` check demands `success`, so adding `perf` to `needs` without the event exception would fail every push and every release call (ci.yml:307-309 comment states the rule). Keep the exception keyed on `.key == "perf"` and the event, not on "skipped is OK".

### Pitfall 4: `continue-on-error` shows red
A failing job with job-level `continue-on-error: true` does not fail the run but the check still renders as failed in the PR checks list [ASSUMED]; `UNSTABLE` (non-zero exit) in report mode will look red even though `CI required` is green. Say so in the YAML comment so nobody "fixes" it.

### Pitfall 5: CTest timeout versus helper deadlines (WR-02)
The two timeouts at tests/CMakeLists.txt:234 (`2`) and :272 (`1`) are smaller than the helpers' own deadlines. D-20 asks for "about 10 s". At 10 s CTest still fires inside the POSIX helper's 10 s ready + 5 s exit window, so on a pathological runner CTest kills the helper before `kill_and_fail` runs, and the runner it spawned (`--frames 4294967295`) can be orphaned (14-REVIEW.md WR-02). Recommended: set the hang detector to 30 s (the value `runner.stop.*` already uses at tests/CMakeLists.txt:198 and what 14-REVIEW.md suggests), with the comment "hang detector, not a performance budget; the 2 s requirement is the helper's STOP_LIMIT_MS". If the owner wants literally 10 s, lower the helpers' `READY_DEADLINE_MS`/`EXIT_DEADLINE_MS` below it in the same change. Planner: pick one and record it (Open Question 1).

### Pitfall 6: README `--help` check fails on day one
The new two-way test will fail immediately because `--movie` and `--help` are in `--help` but have no `- \`--opt\`` bullet, and `--accuracycoin-page` and `--scoreboard` are accepted by `parse_options` (main.c:349-356) but appear in neither `--help` (print_usage 72-102) nor a README bullet. Recommendation: add the two accepted-but-unlisted options to `--help` and add bullets for `--movie`, `--help`, `--accuracycoin-page`, `--scoreboard`, `--bench`, `--bench-state`. The extractors: help options = lines matching `^  (--[a-z][a-z0-9-]*)` in `--help` stdout (continuation lines begin with 22 spaces then `[`, so they do not match) [VERIFIED by reading print_usage]; README options = lines matching ``^- `(--[a-z][a-z0-9-]*)`` between `\n## The runner\n` and `\n## The libretro core\n`. The current README bullets are `frames raise-after version rom hash-frame hash-audio dump-frame save-dir save-interval state-at-frame hash-state load-state` (12) against 14 in `--help` [VERIFIED: grep]. Put it in a new MODE of `runner_exit_table.cmake` or a new `runner_options.cmake`; a new script keeps `runner.exit.*` focused.

### Pitfall 7: README status text is stale
README.md:6-7 still says "Status: v1 shipped; milestone v2 is under way" and "Phase 5 adds parallel CI..." while the project is closing v3. This is the one concrete "docs describe released code" defect found so far; the D-22 audit should record it and fix it (also mention rewind/run-ahead, save states, `--bench`, `perf`). The header's `NESTURBATOR_BEHAVIOUR_REVISION 6` matches the README's "behaviour revision is 6" [VERIFIED: include/nesturbator.h:56 and README.md:25].

### Pitfall 8: hygiene rules for new tracked files
`scripts/hygiene.sh` fails on game-image names (`*.nes|*.sav|*.srm|*.state` not in the manifest), home paths other than `/home/runner`, and emails. Name synthetic Cachegrind fixtures `*.out` (not `*.state`/`*.sav`), keep `cmd:` lines in them generic, and use no personal data in `allow.txt` [VERIFIED: scripts/hygiene.sh:34-48].

## Code Examples

### Signal install fix (WR-01, D-20), verified
Compiled with `-std=c17 -Wall -Wextra -pedantic -Werror` and run on macOS: a plain run sets the flag (`flag=1`), and under `trap "" TERM` the inherited SIG_IGN is restored and the raise is ignored (`flag=0`) [VERIFIED: local]:
```c
#include <errno.h>
static void on_signal(int sig)
{
    int saved = errno;            /* C17 7.14.1.1: signal() may set errno when it fails */
    (void)signal(sig, SIG_DFL);   /* a second signal ends the process (D-02) */
    stop_flag = 1;
    errno = saved;
}
static void install_one(int sig)
{
    void (*prev)(int) = signal(sig, on_signal);
    if (prev == SIG_IGN)
        (void)signal(sig, SIG_IGN);   /* honour an inherited ignore */
}
void nesturbator_run_stop_install(void)
{
    install_one(SIGINT);
    install_one(SIGTERM);
}
```
Keep `#define _DEFAULT_SOURCE` (stop.c:1) so glibc gives `signal()` BSD restart semantics. The POSIX launcher advice sentence for WR-03 goes in README after line 1229: "A launcher must keep reading the runner's standard output, or redirect it, because a full pipe delays the stop until the write returns."
**Test (deterministic, no timing):** a `runner.stop.inherited_ignore` case in `runner_stop.cmake`, POSIX only, run through a one-line wrapper script (`trap "" TERM; exec "$1" ...`, as `ignwrap.sh` did in the experiment) with `--rom battery.nes --frames 5 --hash-frame 5 --raise-after 2:term`: with the fix the runner ignores the raise, finishes frame 5, prints the frame 5 line and exits 0 with no `stopped by signal` line; without the fix it stops after frame 2 and exits 1 (`not reached: --hash-frame 5`). Note `trap "" TERM` is honoured because `exec` preserves SIG_IGN. Mirror the existing `if(WIN32)` split in tests/CMakeLists.txt:200-206.

### Cachegrind measurement (D-08)
```bash
ir() { # bin rom frames  -> prints the Ir total
  env -i PATH=/usr/bin:/bin valgrind --tool=cachegrind --cache-sim=no \
    --cachegrind-out-file="$4" "$1" --rom "$2" --frames "$3" >/dev/null 2>"$4.err" || return 1
  awk '/^summary:/{print $2}' "$4"
}
```
(`awk` here only extracts one decimal; fine on Linux where the job runs. In the CMake script use `file(STRINGS ... REGEX "^summary:")`.) Measured Ir per frame (container): nesteroids 25,236,015; runner 25,939,911; M4_P256K_C256K 24,835,858 [VERIFIED: container run]. A 360-frame cachegrind run took 16.3 s and a 60-frame run 3.1 s on an 8-core aarch64 container (native 360 frames: 0.36 s, about 45x slowdown), so 18 runs are about 3 min plus apt (~20-40 s) and two builds (1.4 s configure+build in the container, longer on a 4-vCPU runner). Estimate 5 to 8 min total, so the provisional `timeout-minutes: 10` is adequate but thin; raise it on the first timeout rather than guessing [ASSUMED for hosted x64 timing].

### No-hash-change test (D-07)
`tests/cmake/runner_bench.cmake -DMODE=nohash` runs `--rom tests/roms/nesteroids.nes --frames K --hash-frame K --hash-audio`, `--bench K ...` and `--bench K --bench-state ...` (K=120), requires rc 0 and byte-identical stdout (strip `\r`). `-DMODE=format` matches stderr with regexes such as `^nesturbator-run: bench frames=[0-9]+ time_ms=[0-9]+\.[0-9]{3} fps=[0-9]+\.[0-9]{2}$` and `^nesturbator-run: bench (frame|save|load)_us p50=[0-9]+\.[0-9]{3} p90=... p99=... p99\.9=... max=...$` (keys only, never values). Usage cases through `nesturbator_runner_test(... EXIT 2 IGNORE_STDOUT EXPECT_ERR "<text>")`.

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| `cache-sim` default on | `--cache-sim=no` is the default; `Ir` only | documented default in the current manual [CITED: cg-manual] | pass it explicitly anyway; output `events: Ir`, one count in `summary:` |
| `windows-11-arm` on VS 2022 | on VS 2026 image `windows-11-vs2026-arm64` | rollout 21-30 Sept 2026 [CITED: actions/runner-images#14602] | ENGINEERING §5 line 74 is stale; confirmed in our own log: run 38329002566, job 115074198221 shows `Image: windows-11-vs2026-arm64`, `Version: 20261004.176.1`, `DevEnvDir: ...\Visual Studio\18\Enterprise\...` [VERIFIED: gh job log] |

**Deprecated/outdated:** the issue #14602 page itself is still open without a closing notice, so cite our own job log, not the issue, as the completion evidence.

## Close-out evidence gathered (for 15-CLOSEOUT.md)

| Item | Current | Latest / status | Source |
|------|---------|-----------------|--------|
| actions/checkout | v7.0.1 | v7.0.1 (2026-07-20) | [VERIFIED: `gh api repos/actions/checkout/releases/latest`] |
| actions/upload-artifact | v7.0.2 | v7.0.2 (2026-10-07) | same |
| actions/download-artifact | v8.0.2 | v8.0.2 (2026-10-07) | same |
| actions/attest | v4.2.2 | v4.2.2 (2026-08-04) | same |
| googleapis/release-please-action | v5.0.0 | v5.0.0 (2026-04-22) | same |
| actions/create-github-app-token | v3.2.0 | **v3.3.0 (2026-10-12T23:42Z)** | same; no Dependabot PR open (`gh pr list` shows only release PR #35) |
| Runner labels | six used | `ubuntu-24.04`, `ubuntu-24.04-arm`, `macos-15`, `macos-15-intel`, `windows-2025`, `windows-11-arm` all listed in the images table | [CITED: github.com/actions/runner-images README] |
| macos-15-intel | in use | "last available x86_64 image ... available until August 2027" | [CITED: actions/runner-images#13046] |
| macos-15 | in use | no retirement date given; "latest two stable versions" policy | same |
| AccuracyCoin | `673ef550db296136d52229961e7d39366116882a` (manifest.txt:18) | upstream `main` is 9 commits ahead | [VERIFIED: `gh api repos/100thCoin/AccuracyCoin/compare/673ef550...main` => ahead_by 9]; compare URL `https://github.com/100thCoin/AccuracyCoin/compare/673ef550...main` |

ENGINEERING.md lines to update with the phase: 74 (`windows-11-arm` VS note), 77 (`perf` "later milestone" row becomes real), 86 (suite-flake "timeout initially 10 minutes"), and DEC.26 context in DECISIONS.md if the merge-base definition (first parent) is recorded there.

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | The zero-spread result measured on aarch64 holds on the hosted x64 runner | Pitfall 1 | PERF-PROOF would show spread; arming is blocked by design (D-16) |
| A2 | valgrind apt install takes 15-40 s and the 18 runs fit in the 10 min provisional timeout (about 5-8 min) | Pattern 4, Code Examples | First `perf` run times out; raise the value |
| A3 | GitHub's `pull_request` merge ref is always a two-parent merge commit | Pattern 4 | `HEAD^1` guard prints `n/a`; low |
| A4 | `inputs.control == true` evaluates as a boolean in `if:` | Pattern 4 | Job never runs in control mode; first dispatch reveals it, switch to `inputs.control` truthiness or `github.event.inputs.control == 'true'` |
| A5 | Job-level `continue-on-error` job renders as a failed check yet does not fail required checks | Pitfall 4 | Cosmetic |
| A6 | Concurrent dispatches of ci.yml on one SHA replace the pending run | Pattern 4 | Dispatch control runs sequentially |
| A7 | mawk integer printing risk for the rejected awk alternative | Alternatives | None; CMake chosen anyway |
| A8 | macOS `CLOCK_MONOTONIC` resolution is fine for ~1 ms frames (prints 3 decimals of microseconds, trailing digits may be 000) | Pattern 1 | Cosmetic |

## Open Questions

1. **WR-02 timeout value (10 s vs 30 s).**
   - Known: D-20 says "about 10 s"; helpers have 10 s + 5 s (POSIX) and 20 s + 2 s + 5 s (Windows) internal deadlines; 14-REVIEW.md recommends 30 s.
   - Unclear: whether "about 10 s" is meant literally.
   - Recommendation: use 30 s (matches `runner.stop.*` at tests/CMakeLists.txt:198) with the required "hang detector, not a performance budget" comment; or lower the helper constants under 10 s in the same commit.
2. **`mmc3-oracle` timeout under the `ceil(2 x measured)` rule.** It measured 20 s whole-job, giving 1 minute, but nightly.yml:128-133 deliberately adds the 300 s fetch allowance (`mmc3.oracle.fetch` CTest TIMEOUT, tests/CMakeLists.txt:1579). Recommendation: keep the established formula (fetch allowance + 2 x measured rest, as `vectors-full` does) and put the run IDs in that comment; the lint only needs a run ID.
3. **Cost of control dispatches.** Dispatching `ci.yml` runs all jobs. Either accept two full CI runs, or add `if: inputs.control != true` to the heavy jobs. Recommend accepting the cost (two runs) unless queueing matters; fewer moving parts.
4. **`--bench` stopped before the first frame.** CONTEXT does not say what prints with zero samples. Recommend `nesturbator-run: bench frames=0` only, exit 0.
5. **`--bench` with `--hash-audio` on a stopped run** follows the Phase 14 rule: no audio hash printed on a stop, and an unmet `--hash-frame` exits 1. No new decision needed.

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| valgrind on `ubuntu-24.04` | `perf` job | not preinstalled; apt | 3.22.0 (`1:3.22.0-0ubuntu3`) | none needed; apt works [VERIFIED container] |
| gcc-14 / g++-14 | `perf` builds | yes (14.2.0 on image) | 14.2.0 | — [VERIFIED: runner-images readme] |
| Ninja, CMake, jq, gh | build, roll-up, evidence | yes (1.13.2, 3.31.6, 1.7, 2.102.0) | — | `build` job already adds `ninja-build` via apt if missing |
| valgrind on the owner's Mac | local reproduction | no (DEC.26) | — | Docker with `ubuntu:24.04` works (used in this research; Docker 29.5.2, aarch64) |
| Authenticated `gh` locally | dispatching nightly/control runs, reading durations | yes (account szTheory) | — | — |

**Missing dependencies with no fallback:** none.

## Validation Architecture

`workflow.nyquist_validation` is `false` in `.planning/config.json`, so no Nyquist matrix is required. The map below answers the orchestrator's request for which tests prove each success criterion.

| Criterion | Test (ctest name, lane) | Proves |
|-----------|--------------------------|--------|
| 1 prints fps and percentiles | `runner.bench.format` (cmake `-P`, `ci`) and `runner.bench.percentile` (unit, `ci`) | line shapes, key names, nearest-rank values for N=1, 999, 1000 |
| 1 changes no hash | `runner.bench.nohash` (cmake `-P`, `ci`) | `--frames`, `--bench`, `--bench --bench-state` produce byte-identical stdout, exit 0 |
| 1 usage conflicts | `runner.usage.bench.*` (one per D-01/D-03 conflict, `--bench 0`, no `--rom`, `--bench-state` alone, N over 1000000) via `nesturbator_runner_test` | exit 2 |
| 2 table in job summary, report mode | real PR run of `perf` plus `perf.compare.selftest` (`ci`) | summary file format from fixtures; the CI run itself is the end-to-end evidence |
| 3 gate arms only after zero spread | `perf.gate_selftest` (`ci`): pass cases identical, exactly +1.00%, +1.01% with allow line (contains `ALLOWED`); fail cases +1.01% without line, cap below delta, malformed, duplicate, unknown ROM, empty reason, expiry over 60 days, expired, `base_a != base_b` => `UNSTABLE` even with a line | the deliberate slowdown passes with a reviewed line; every failure has its own message |
| 3 roll-up | `ci.required.rollup` (small cmake `-P` that runs the `jq` expression on synthetic `needs` JSON: push with `perf` skipped passes, pull_request with `perf` skipped fails, any failure fails) | the D-16 exception |
| 3 evidence | `15-PERF-PROOF.md` with two control run IDs, image version, valgrind and gcc versions | arming precondition (artifact, not ctest) |
| 4 timeouts measured | `hygiene.timeout_comments` + `.bad` + `.good` (hygiene lane) | every `timeout-minutes` has a run ID |
| 4 docs match | `runner.options.readme` (`ci`), existing `runner.exit.table`/`.help` | `--help` and README list the same options |
| WR-01 | `runner.stop.inherited_ignore` (POSIX) | inherited SIG_IGN is honoured |
| WR-02/03 | review of `tests/CMakeLists.txt` timeouts and README sentence; `runner_exit_table.cmake` extended with the new sentence if desired | — |

Labels: reuse `exit_status` for the runner stop/usage cases; give the new bench tests a `bench` label and the gate tests a `perf` label. Quick command: `ctest --preset ci -R "runner.bench|runner.usage.bench|perf\.|runner.options"`.

### Wave 0 Gaps
- `runner/clock.[ch]`, `runner/bench.[ch]`, `tests/runner/test_bench.c`
- `tests/cmake/{perf_compare,runner_bench,runner_options,timeout_comments}.cmake`
- `tests/perf/allow.txt`, `tests/perf/fixtures/*.out`, `tests/hygiene/fixtures/timeouts_{bad,good}.yml`

## Security Domain

`security_enforcement` is `false` in `.planning/config.json`; ASVS mapping omitted. Relevant hardening already fixed by the decisions: `pull_request` only (never `pull_request_target`), `permissions: contents: read`, `persist-credentials: false`, SHA-pinned actions, summary text limited to fixed names and computed integers (GitHub summaries render Markdown, limit 1 MiB per step [CITED: docs.github.com workflow-commands, job summaries]). Threat to note: the PR author controls the comparison script and `allow.txt` (D-17 accepts this; the cap and the 60-day expiry bound it).

## Sources

### Primary (HIGH confidence)
- Repo files read this session: CONTEXT.md, `runner/main.c`, `runner/stop.c`, `runner/stop.h`, `runner/exit_status.h`, `runner/CMakeLists.txt`, `.github/workflows/{ci,nightly,release}.yml`, `.github/rulesets/main.json`, `tests/CMakeLists.txt` (anchors cited), `tests/cmake/{action_pins,runner_exit_table,runner_stop,expect_output,script_policy,nightly_workflow_policy,release_policy}.cmake`, `tests/hygiene/CMakeLists.txt`, `README.md` runner/CI sections, `include/nesturbator.h` (state API), `14-REVIEW.md`, `14-REVIEW-DISPOSITION.md`, `ENGINEERING.md` §5.
- Experiments this session: ubuntu:24.04 container with valgrind 3.22.0 and gcc-14.2.0 (Cachegrind determinism, clock macro compile, install time); local clang and cmake 4.4.3 (signal fix, date math); local shallow-clone/worktree test; `gh` queries of workflow runs, jobs, release tags and AccuracyCoin compare.
- Valgrind Cachegrind manual https://valgrind.org/docs/manual/cg-manual.html (cache-sim default off, summary/events grammar, reproducibility caveats).
- GitHub docs: events that trigger workflows (pull_request merge commit, workflow_call context), workflow commands (job summary).
- Microsoft Learn, Acquiring high-resolution time stamps.

### Secondary (MEDIUM confidence)
- actions/runner-images README and issues 13046, 14602 (labels, retirement dates, VS 2026 move).
- runner-images `Ubuntu2404-Readme.md` (image 20261004.327.1: no valgrind, gcc 14.2.0, jq 1.7, cmake 3.31.6, ninja 1.13.2).

### Tertiary (LOW confidence)
- None relied on; remaining unknowns are in the Assumptions Log.

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH - no new dependencies; versions observed directly.
- Architecture: HIGH - code anchors read and the determinism evidence measured.
- Pitfalls: HIGH for 1, 5, 6, 7 (measured or read in repo); MEDIUM for 3, 4 (GitHub semantics partly assumed).

**Research date:** 2026-10-13
**Valid until:** 2026-11-12 (runner images and action versions move weekly; re-run the pin table before the close-out commit)
