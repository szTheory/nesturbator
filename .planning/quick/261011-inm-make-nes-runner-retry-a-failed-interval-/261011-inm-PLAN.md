---
phase: quick-261011-inm
plan: 01
type: execute
wave: 1
depends_on: []
files_modified:
  - runner/main.c
  - tests/cmake/runner_save.cmake
  - tests/CMakeLists.txt
  - README.md
  - .planning/phases/08-mmc1-and-battery-saves/08-REVIEW-DISPOSITION.md
  - CMakeLists.txt
  - CMakePresets.json
  - tests/core/test_reset.c
  - .github/workflows/ci.yml
autonomous: true
requirements: [08-WR-01, AUDIT-ASAN-TIMEOUT]

estimate:
  tokens: 70000
  raw_tokens: 70000
  tasks: 3
  confidence: low

must_haves:
  truths:
    - "When an interval save flush fails, the runner tries the flush again at exit instead of skipping it (08 WR-01)"
    - "runner.save.interval_retry fails on the old code (one failed attempt reported) and passes on the new code (two)"
    - "The ci preset registers exactly the same tests with exactly the same commands as before (JSON test list identical)"
    - "Under the asan preset, runner.write_hashes(.content), runner.boot_to_play and the two vectors.registration_policy tests are not registered, core.reset sweeps fewer reset points, and non-save Holy Mapperel parity compares at frame 100"
    - "cmake --workflow --preset asan passes locally and its wall time is recorded"
    - "The asan CI job's timeout-minutes is a provisional value with a comment saying the PR's CI run supplies the measured figure"
  artifacts:
    - path: runner/main.c
      provides: "save_if_changed advances the generation only after a successful flush"
    - path: tests/cmake/runner_save.cmake
      provides: "interval_retry case"
    - path: CMakeLists.txt
      provides: "option NESTURBATOR_SHORT_RUNS, default OFF"
    - path: CMakePresets.json
      provides: "asan configure preset sets NESTURBATOR_SHORT_RUNS ON"
  key_links:
    - from: CMakePresets.json (asan configurePreset)
      to: tests/CMakeLists.txt
      via: "NESTURBATOR_SHORT_RUNS cache variable gates registration"
    - from: tests/CMakeLists.txt core.reset add_test
      to: tests/core/test_reset.c main
      via: "--short argument, passed only when NESTURBATOR_SHORT_RUNS is ON"
---

<objective>
Two independent fixes from the v2 milestone audit, as separate commits on the already
checked-out branch quick/261011-inm-save-flush-and-asan:

1. 08 WR-01: nes-runner's `--save-interval` flush consumes the save generation before it
   writes, so after a failed interval flush the exit flush sees "nothing new" and never
   retries. Fix it so the exit flush tries again, with a test.
2. The asan CI job takes about 15 minutes because of a few long tests whose full-length
   results the six `ci` legs and the hash-equality job already check. Add one CMake option,
   set only by the asan configure preset, that shortens or leaves out those tests under asan,
   keeps the `ci` preset unchanged, and sets a provisional asan `timeout-minutes`.

Purpose: close audit debt items "08 WR-01" and "CI timeouts: asan".
Output: runner fix + test, a NESTURBATOR_SHORT_RUNS option, README and ci.yml updates.
</objective>

<execution_context>
@~/.claude/gsd-core/workflows/execute-plan.md
@~/.claude/gsd-core/templates/summary.md
</execution_context>

<context>
@CLAUDE.md
@.planning/STATE.md
@.planning/v2-MILESTONE-AUDIT.md
@.planning/phases/08-mmc1-and-battery-saves/08-REVIEW.md

Facts gathered while planning (the executor need not re-derive them):

- `runner/main.c` `save_if_changed` (about line 261): sets `sv->generation = gen` before the
  memcmp and before `nesturbator_run_save_flush`. The frame loop (about line 803) runs while
  `status == 0`; an interval flush failure sets `status = 1`, the loop ends, and the exit
  flush (about line 859, "The exit flush runs whatever ended the loop") calls
  `save_if_changed` again, which currently returns 0 at once.
- `runner/save.c` `nesturbator_run_save_flush` prints `nesturbator-run: cannot write <path>.tmp`
  once per failed attempt when the `.tmp` cannot be opened (missing directory).
- `tests/cmake/runner_save.cmake` already has the case `unwritable` (`--save-dir
  "${WORK}/missing/dir"`, exit 1, stderr names `battery.sav.tmp`) and `interval_long`.
  Cases are registered by `runner_save_case(<case>)` in `tests/CMakeLists.txt` (about line 150).
- A transient filesystem fault cannot be produced deterministically between the interval
  flush and the exit flush (the exit flush runs immediately after the loop ends, in the same
  process), and the runner has no fault hook. The observable proof of the retry is therefore
  the number of failed attempts reported on stderr: one before the fix, two after.
- 08 IN-02 (stale `.tmp` after a failed write) is in `runner/save.c`, a different function, and
  a failed `fwrite` cannot be forced portably for a test; it stays open and out of this plan.
- Measured on the last main CI asan job (clang-18, hosted): runner.write_hashes 830 s,
  core.reset 422 s, runner.boot_to_play 262 s, vectors.registration_policy 131 s, each
  libretro.parity.m4* about 50 to 70 s. Test preset asan runs 4 tests at a time.
- Why each long test is shortened or left out under asan (decided in planning):
  - `runner.write_hashes` and `runner.write_hashes.content`: cannot be shortened. The DABG and
    nes-runner movies have a fixed length (`--frames` must equal the movie length), and the
    script asserts the full 95-key inventory and static Holy Mapperel screens. The asan job
    uploads no hashes. The same ROMs and runner paths still run under asan through
    `runner.dump.*`, `runner.dump.nesrunner.play`, `holymapperel.*.dump` and
    `runner.hash_audio.*`. Not registered when the option is ON (both tests, since `.content`
    requires the `hashes_txt` fixture and CTest would pull the setup test back in).
  - `runner.boot_to_play`: cannot be shortened. Its predicates need the whole 600-frame movie
    (play from frame 150, score progress 60 frames apart, snapshots at 300 and 600). The same
    positive movie runs through the library under asan in `runner.dump.nesrunner.play` and
    `libretro.parity.nesrunner.play`. Not registered when ON.
  - `vectors.registration_policy` and `vectors.registration_policy.self_test`: they check CTest
    metadata by configuring another tree; sanitizers add nothing, and every `ci` leg runs them.
    Not registered when ON.
  - `core.reset`: its cost is `check_reset_sentinel_frame` in `tests/core/test_reset.c`, which
    resets after every `pre` from 0 to 120 frames (about 7,300 emulated frames). Shortened:
    with `--short` the sweep steps `pre` by 20 (0, 20, ..., 120), keeping both ends.
  - `libretro.parity.<key>` for non-save Holy Mapperel entries whose N is above 100
    (m4tnrom, m4txrom, m4w8k at N = 600): shortened. Parity compares module and runner frames
    (not committed hashes), so under the option the matching `holymapperel.<key>.dump` also
    dumps frame 100 to `<key>-f100.ppm` and the parity test runs 100 frames against that
    image. The save entries (m4tkrom, m1sxrom run1/run2) keep their full length, because
    `--sav-expect` compares the battery span at the end of the runner's full run.
</context>

<tasks>

<task type="auto" tdd="true">
  <name>Task 1: Retry a failed interval save flush at exit (08 WR-01)</name>
  <files>tests/cmake/runner_save.cmake, tests/CMakeLists.txt, runner/main.c, README.md, .planning/phases/08-mmc1-and-battery-saves/08-REVIEW-DISPOSITION.md</files>
  <behavior>
    - New case `interval_retry`: `--rom "${battery}" --frames 3 --save-dir "${WORK}/missing/dir" --save-interval 1` exits 1, and stderr contains `battery.sav.tmp` exactly twice: the interval attempt after frame 1 and the retry at exit.
    - On the current code the same case reports it once, so the test fails before the fix (RED).
    - Existing runner.save.* cases (unwritable, interval, interval_long, unchanged, jam, ...) still pass.
  </behavior>
  <action>
    RED: in tests/cmake/runner_save.cmake add an `elseif(CASE STREQUAL "interval_retry")` branch beside `unwritable`, with a one-line comment saying a failed interval flush must be tried again at exit (08 WR-01). Run the runner with the arguments in the behavior block through `run_runner`, `expect_exit(1)`, then count matches of `battery\.sav\.tmp` in `run_err` with `string(REGEX MATCHALL ...)` and `list(LENGTH ...)`; FATAL_ERROR with the stderr text unless the count is 2. Register it in tests/CMakeLists.txt with `runner_save_case(interval_retry)` after `runner_save_case(interval_long)`. Build with the dev preset and confirm runner.save.interval_retry fails with a count of 1. Commit the failing test: `test(runner): a failed interval save flush is tried again at exit`.

    GREEN: in runner/main.c `save_if_changed`, apply the 08-REVIEW WR-01 fix: when the span equals the shadow, record the generation and return 0; otherwise call `nesturbator_run_save_flush` and record the generation only when it returns 0, returning 1 on failure without touching `sv->generation`. Update the function's comment to say a failed write leaves the generation unrecorded so the next look (the exit flush) writes again. In the file's top comment block (the paragraph starting "With --save-dir DIR", about line 20) add one sentence: a write that fails at an interval is tried once more at exit. In README.md, in the `--save-interval N` bullet (about line 922), add the same fact in plain words (a failed interval write ends the run with exit 1 and the runner tries the write once more before it exits). Do not touch runner/save.c (08 IN-02 stays open, see context).

    In .planning/phases/08-mmc1-and-battery-saves/08-REVIEW-DISPOSITION.md set the WR-01 row's Disposition to `fixed` and append to its Source cell "; fixed in quick 261011-inm (runner.save.interval_retry)"; lower the `open:` front-matter count from 5 to 4.

    Run the whole dev suite, then commit: `fix(runner): retry a failed interval save flush at exit`.
  </action>
  <verify>
    <automated>cmake --preset dev >/dev/null && cmake --build --preset dev && ctest --preset dev -R '^runner\.(save|usage)\.'</automated>
  </verify>
  <done>runner.save.interval_retry passes and was seen failing first; all runner.save.* and runner.usage.* tests pass; `cmake --workflow --preset dev` passes; README and the runner's header comment state the retry; two commits exist (test, then fix).</done>
</task>

<task type="auto">
  <name>Task 2: NESTURBATOR_SHORT_RUNS shortens the long tests under asan only</name>
  <files>CMakeLists.txt, CMakePresets.json, tests/CMakeLists.txt, tests/core/test_reset.c, README.md</files>
  <action>
    Before any edit, capture the ci test list for the byte-for-byte check: `cmake --preset ci >/dev/null && ctest --test-dir build/ci --show-only=json-v1 > <scratchpad>/ci-before.json` (use the session scratchpad, not the repo).

    CMakeLists.txt: next to `option(NESTURBATOR_NOFP ...)` add `option(NESTURBATOR_SHORT_RUNS "..." OFF)` with a two-line comment: preset asan sets it; it shortens or leaves out the long tests whose full-length results the ci legs and the hash-equality job already check, so ASan exercises the same code paths in less time.

    CMakePresets.json: add `"NESTURBATOR_SHORT_RUNS": "ON"` to the asan configure preset's cacheVariables only.

    tests/CMakeLists.txt, each change guarded by `if(NOT NESTURBATOR_SHORT_RUNS)` / `if(NESTURBATOR_SHORT_RUNS)` with a one-line comment giving the reason from the context section:
    - Wrap the `runner.write_hashes` add_test, its set_tests_properties and `runner.write_hashes.content` (about lines 425-439) so neither is registered when ON.
    - Wrap the `add_test(NAME runner.boot_to_play ...)` and its properties (about line 693) the same way; keep the add_executable so it still compiles under asan.
    - Wrap `vectors.registration_policy` and `vectors.registration_policy.self_test` (about line 747) the same way.
    - core.reset (about line 483): it is registered by the `nesturbator_core_test(name source)` helper, whose add_test takes no arguments. Let the helper forward extra arguments: change its add_test to `COMMAND ${name} ${ARGN}`, and call `nesturbator_core_test(core.reset core/test_reset.c --short)` when ON and the existing call when OFF. With OFF every helper-registered test keeps exactly its current command (ARGN is empty), which the JSON comparison below confirms.
    - Holy Mapperel non-save branch (the `else()` branch about line 373): define `NESTURBATOR_SHORT_PARITY_FRAMES` = 100 near the top of the loop. When ON and `hm_frames` is greater than 100, add `--dump-frame "100:${NESTURBATOR_HM_DIR}/${hm_key}-f100.ppm"` to the `holymapperel.${hm_key}.dump` ARGS and register `libretro.parity.${hm_key}` with 100 frames and that `-f100.ppm` image; otherwise register it exactly as today. Leave the save branch untouched.
    - Update the COST comment above runner.write_hashes (about line 429) only if it now reads wrongly; keep it short.

    tests/core/test_reset.c: change `main(void)` to `main(int argc, char **argv)`; accept exactly one optional argument `--short` (any other argument: print a usage line to stderr and return 2). Pass a step to `check_reset_sentinel_frame` (1 normally, 20 with `--short`) and step `pre` by it; keep the upper bound 120 and the second loop (`pres` 0, 1, 60) unchanged. Add a comment that the short sweep keeps both ends and is used by the asan preset, whose ci legs run the full sweep. Run clang-format on the file if `scripts/hygiene.sh` or the format check requires it.

    README.md: in the lane table row for `asan` (about line 217), say it runs the same tests built with ASan and UBSan, except that NESTURBATOR_SHORT_RUNS leaves `runner.write_hashes`, `runner.boot_to_play` and the two `vectors.registration_policy` tests to the ci lanes, sweeps fewer reset points in `core.reset`, and compares the non-save Holy Mapperel parity runs longer than 100 frames at frame 100. In the paragraph about the long tests' COST (about line 564), add that the asan lane does not register them.

    Byte-for-byte check: `cmake --preset ci >/dev/null && ctest --test-dir build/ci --show-only=json-v1 > <scratchpad>/ci-after.json && cmp <scratchpad>/ci-before.json <scratchpad>/ci-after.json` must report no difference.

    Measure: run `time cmake --workflow --preset asan` from the checkout root and record the wall time, the test count, and the ten slowest tests from `build/asan/Testing/Temporary/LastTest.log` or CTestCostData.txt in the SUMMARY. If a policy or drift-guard test fails because a test it expects is not registered under the option, fix it by guarding that expectation with the same option (never by weakening the ci behaviour), and say so in the SUMMARY.

    Run `cmake --workflow --preset ci`, then commit: `test: shorten the long tests under the asan preset`.
  </action>
  <verify>
    <automated>cmake --preset asan >/dev/null && ! ctest --test-dir build/asan -N | grep -E 'runner\.write_hashes|runner\.boot_to_play|vectors\.registration_policy' && ctest --test-dir build/asan -N | grep -q 'core\.reset' && cmake --workflow --preset asan</automated>
  </verify>
  <done>The ci JSON test list is identical before and after (cmp silent); `ctest --test-dir build/asan -N` lists none of the left-out tests; `cmake --workflow --preset asan` and `cmake --workflow --preset ci` both pass; the local asan wall time and slowest tests are recorded for the SUMMARY; README describes the asan lane's short runs.</done>
</task>

<task type="auto">
  <name>Task 3: Provisional asan timeout from the shortened run</name>
  <files>.github/workflows/ci.yml</files>
  <action>
    In .github/workflows/ci.yml, asan job (about line 126), replace `timeout-minutes: 20` and its comment with `timeout-minutes: 15` and a shorter comment: provisional; NESTURBATOR_SHORT_RUNS (asan preset) dropped the long tests that made the job about 15 minutes (runner.write_hashes 830 s, core.reset 422 s, runner.boot_to_play 262 s, vectors.registration_policy 131 s on the last main run); the local shortened asan workflow took <measured wall time from Task 2>; set this to twice the measured hosted run from this PR's CI (ENGINEERING section 5). 15 is chosen so the first hosted run of the shortened job cannot be cut off, while still below the old 20; the orchestrator replaces it with twice the measured PR run before merge. Commit separately: `ci: provisional asan timeout after shortening the long tests`.
  </action>
  <verify>
    <automated>grep -n -A1 '^  asan:' .github/workflows/ci.yml && grep -E '^\s+timeout-minutes: 15 # provisional' .github/workflows/ci.yml && cmake --workflow --preset hygiene</automated>
  </verify>
  <done>ci.yml asan job has `timeout-minutes: 15` with a provisional comment naming the local measurement and the rule (twice the measured hosted run); hygiene lane passes; separate commit.</done>
</task>

</tasks>

<threat_model>
## Trust Boundaries

| Boundary | Description |
|----------|-------------|
| runner -> filesystem | The runner writes `<dir>/<name>.sav` through `<name>.sav.tmp` |
| CI config -> test coverage | A preset option decides which tests run under sanitizers |

## STRIDE Threat Register

| Threat ID | Category | Component | Severity | Disposition | Mitigation Plan |
|-----------|----------|-----------|----------|-------------|-----------------|
| T-q261011inm-01 | Tampering | runner/main.c save_if_changed | medium | mitigate | Generation recorded only after a successful flush, so battery data is not silently dropped after a transient failure; runner.save.interval_retry guards it |
| T-q261011inm-02 | Denial of service | runner exit flush | low | accept | A persistent fault now costs one extra failed write attempt at exit; the run already exits 1 |
| T-q261011inm-03 | Repudiation | NESTURBATOR_SHORT_RUNS coverage | medium | mitigate | The option is OFF by default and set only by the asan preset; the ci JSON test list is compared byte for byte before and after; README states exactly which tests asan leaves to the ci lanes |
| T-q261011inm-SC | Tampering | npm/pip/cargo installs | low | accept | No package installs in this plan |
</threat_model>

<verification>
- `cmake --workflow --preset dev` and `cmake --workflow --preset ci` pass.
- `cmake --workflow --preset asan` passes locally; wall time recorded.
- ci test list JSON identical before and after Task 2.
- `cmake --workflow --preset hygiene` passes.
- Three or four commits, Conventional Commit messages, flush fix and CI change in separate commits.
</verification>

<success_criteria>
- A failed interval flush is retried at exit, shown by runner.save.interval_retry (two failed attempts reported).
- The asan lane no longer registers runner.write_hashes(.content), runner.boot_to_play or the vectors.registration_policy pair, runs core.reset with a 20-frame sweep step, and compares m4tnrom, m4txrom and m4w8k parity at frame 100; the ci lane is unchanged.
- asan `timeout-minutes` is a provisional 15 with a comment; the PR's hosted run supplies the measured figure for the final value.
- README and the runner's header comment describe both changes.
</success_criteria>

<output>
Create `.planning/quick/261011-inm-make-nes-runner-retry-a-failed-interval-/261011-inm-SUMMARY.md` when done. Include the local asan wall time, the slowest asan tests after the change, and a note that the orchestrator sets the final asan timeout to twice the PR's measured hosted run.
</output>
