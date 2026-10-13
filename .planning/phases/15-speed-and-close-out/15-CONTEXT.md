# Phase 15: Speed and close-out - Context

**Gathered:** 2026-10-12
**Status:** Ready for planning

<domain>
## Phase Boundary

Speed becomes visible and guarded without ever gating wall time, and milestone v3 closes
(PERF-02, PERF-03, TUNE-10).

- `nesturbator-run --bench N` reports frames per second and frame-time percentiles, and
  `--bench-state` adds the cost of a save and load per frame. A `ci` test shows benchmarking
  changes no hash.
- Every pull request runs a Cachegrind job comparing instructions per frame, head against merge
  base, on committed NROM, MMC1 and MMC3 ROMs, with the table in the job summary. It starts as a
  report. It becomes a +1% per-ROM gate inside this phase, once recorded evidence shows identical
  builds have zero spread, and a deliberate slowdown passes with a reviewed line in an allow
  file.
- The milestone closes:
  - Workflow timeouts are set from measured runs.
  - Pins, runner labels and the AccuracyCoin pin are reviewed and recorded.
  - The Phase 14 review warnings are fixed.
  - The README and public header match the released code.

Not in this phase:
- Wall-time gates of any kind.
- Speed optimisation work (a lazy PPU schedule, SEED-004).
- Bumping AccuracyCoin.
- Benchmarking from `--load-state` or `--movie`.
- Phase 14 info items IN-01 to IN-04.

</domain>

<decisions>
## Implementation Decisions

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

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Requirements and scope
- `.planning/ROADMAP.md` Phase 15 — goal, success criteria 1-4, notes (TUNE-10 last, absorbs
  Backlog 999.1)
- `.planning/REQUIREMENTS.md` PERF-02, PERF-03, TUNE-10
- `.planning/seeds/SEED-004-robustness-and-speed.md` — speed must not change a hash
- `.planning/seeds/SEED-006-milestone-tune-up.md` — closed by TUNE-10

### Engineering policy
- `.planning/preparation/ENGINEERING.md` §5 — the `perf` job row, timeouts at twice a measured
  cold run, pins, `CI required`, suite-flake
- `.planning/preparation/DECISIONS.md` DEC.26 — Cachegrind instruction counts, head vs merge
  base, no wall time

### Phase 14 carry-over
- `.planning/phases/14-runner-process-contract/14-CONTEXT.md` — stop contract, exit table, stderr
  conventions
- `.planning/phases/14-runner-process-contract/14-REVIEW.md` — WR-01 to WR-03, IN-01 to IN-04
- `.planning/phases/14-runner-process-contract/14-REVIEW-DISPOSITION.md` — to update per D-20

### Code and CI touch points
- `runner/main.c`, `runner/stop.c`, `runner/exit_status.h`, `README.md` (runner sections)
- `include/nesturbator.h` — `nesturbator_run_frame`, `nesturbator_state_size`,
  `nesturbator_save_state`, `nesturbator_load_state`
- `.github/workflows/ci.yml` (`ci-required`, build job toolchain), `nightly.yml`, `release.yml`
- `tests/cmake/action_pins.cmake`, `release_policy.cmake`, `nightly_workflow_policy.cmake`,
  `runner_exit_table.cmake`

### External primary sources
- Valgrind Cachegrind manual — https://valgrind.org/docs/manual/cg-manual.html
- POSIX `clock_gettime` — https://pubs.opengroup.org/onlinepubs/9699919799/functions/clock_getres.html
- Microsoft, Acquiring high-resolution time stamps — https://learn.microsoft.com/windows/win32/sysinfo/acquiring-high-resolution-time-stamps
- GitHub Actions security hardening — https://docs.github.com/en/actions/security-for-github-actions/security-guides/security-hardening-for-github-actions
- actions/runner-images announcements — https://github.com/actions/runner-images (issues 13046, 14602, 14748)
- AccuracyCoin upstream — https://github.com/100thCoin/AccuracyCoin/compare/673ef550...main

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `runner/stop.c` / `runner/save.c`: the `#ifdef _WIN32` + `WIN32_LEAN_AND_MEAN` pattern for the
  clock module.
- `runner/main.c` option parser (`parse_count`, `usage()`): the new options and their exit-2
  conflicts.
- `tests/cmake/runner_exit_table.cmake`, `expect_output.cmake`: README/`--help` checking, and
  stdout/stderr capture for the bench tests.
- `tests/cmake/action_pins.cmake`: an existing workflow-file scanner to extend for
  `timeout-minutes` comments.
- Library state API (`nesturbator_state_size` / `save_state` / `load_state`) for
  `--bench-state`.

### Established Patterns
- CI jobs: SHA-pinned `uses:`, `persist-credentials: false`, `CC=gcc-14` on Linux, a timeout
  comment naming the measured run.
- `CI required` roll-up: `if: always()` + `jq` all `success`, so a skipped job counts as failure
  unless handled (D-16).
- Hygiene: any new tracked text file (`tests/perf/allow.txt`, fixtures) must pass
  `scripts/hygiene.sh`.

### Integration Points
- `ci.yml`: the new `perf` job, and the later edit to `ci-required`.
- `nightly.yml` / `release.yml` / `ci.yml`: timeout values and comments.
- README runner sections: bench options, the stdout-draining advice.

</code_context>

<specifics>
## Specific Ideas

- The owner's rule "another copy and paste is better than another dep" applies here:
  - no benchmark library;
  - no `valgrind.h`;
  - no perf-tracking service (Bencher, CodSpeed);
  - integer formatting by hand instead of floats.
- Published accuracy-class cores report 250–400 fps as a single average. Our `fps` is
  comparable, and the percentile line is the extra.

</specifics>

<deferred>
## Deferred Ideas

- Bump AccuracyCoin past `673ef550` (re-baseline the ROM hash, scoreboard and hashes). This is a
  backlog item.
- Phase 14 IN-01 to IN-04 (SIGHUP, unreachable EINTR loops, duplicated `parse_kind`, close-wait
  statics comment). Backlog.
- `--bench` with `--load-state` or `--movie`, and a stdout or `--bench-out` machine output.
  Add only on request.
- A lazy PPU schedule and other speed work (SEED-004), accepted only with identical hashes.

</deferred>

---

*Phase: 15-speed-and-close-out*
*Context gathered: 2026-10-12*
