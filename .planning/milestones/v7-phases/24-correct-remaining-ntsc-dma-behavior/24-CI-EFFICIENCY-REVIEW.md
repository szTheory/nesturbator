# Phase 24 developer-check review

## Verdict

The additive `dev-check` preset is suitable as a fast local feedback tier. I
found no current selection or CI-authority defect. It does not weaken or
replace the full `ci` workflow, and the documentation says full CI remains
required for acceptance. This is local feedback only; it adds no test,
campaign, host-session, remote-trigger, or acceptance authority.

## Review findings

1. **The current build/test lists are aligned.** The test filter selects 18
   executable-backed tests and two source-only checks: `state.format_pin` and
   `vectors.api_policy.self_test`. The build list contains all 18 executable
   targets. `core.state_fork.entry` builds `core.state_fork` but runs only its
   `--entry` mode; the board matrix and stress test remain outside this tier.
   The API-policy self-test also checks the current header against the pinned
   declaration stream before testing that a mutation is rejected, so it
   provides a live-source check as well as a validator self-test.

2. **No fixture closure expands the current selection.** The selected tests
   have no `FIXTURES_REQUIRED`, `FIXTURES_SETUP`, or `FIXTURES_CLEANUP`
   properties in `tests/CMakeLists.txt`. CTest does automatically add setup
   and cleanup tests for fixtures required by a regex-selected test, so that
   property should be rechecked if this list changes ([CMake fixture behavior](https://cmake.org/cmake/help/v3.25/prop_test/FIXTURES_REQUIRED.html)).
   The selected tests also have no `RESOURCE_LOCK`, `RUN_SERIAL`, or explicit
   `COST` properties. Four workers therefore bound CTest parallelism without
   bypassing a selected test's declared serialization contract. CTest uses
   cost as an ordering estimate when parallel execution is enabled; none of
   the selected tests currently needs a hand-tuned cost ([CMake `COST`](https://cmake.org/cmake/help/v3.25/prop_test/COST.html),
   [CMake `RESOURCE_LOCK`](https://cmake.org/cmake/help/v3.25/prop_test/RESOURCE_LOCK.html)).

3. **CMake 3.25 supports the preset fields used here.** The repository
   requires CMake 3.25 and the preset file uses schema version 6. CMake's 3.25
   manual marks schema version 6 and workflow presets as introduced in 3.25;
   it documents build-preset `targets`, test-preset regex `filter.include.name`,
   `execution.jobs`, and `noTestsAction: "error"` with the meanings used here
   ([CMake 3.25 `cmake-presets(7)`](https://cmake.org/cmake/help/v3.25/manual/cmake-presets.7.html)).

4. **The selected checks complement one another.** The tier spans public API
   behavior, CPU and bus behavior, core interrupt entry and polling, controller
   input, focused PPU behavior, C/C++ header compilation, and source policy
   pins. The CPU and bus tests exercise different production boundaries; the
   focused PPU checks do not replace the broad ROM or host evidence. No
   required CI suite was removed or filtered by these additions. In
   particular, this tier does not establish AccuracyCoin results, protected
   main authority, cross-platform package behavior, full state-fork coverage,
   or performance acceptance.

5. **The workflow reuses the `dev` build tree by design.** Configure and test
   presets point to the existing `dev` configuration, so incremental builds
   can reuse its cache and artifacts. Every executable in the selected test
   list is also in the selected target list, which is correct today. The
   maintenance footgun is that `noTestsAction: "error"` catches an empty
   regex result but not a partial name/target drift; with a previously built
   executable in `build/dev`, a newly selected test whose target was omitted
   could accidentally run an older binary. Keep the two lists synchronized.
   If this tier becomes a required CI gate, add a small inventory check that
   asserts the selected test names and executable targets as a set rather than
   relying on the documented count alone.

## Timing evidence and next measurement

The worker-reported run passed all 20 selected tests in 2.57 seconds of CTest
time. Its Ninja command graph was reported as 72 commands versus 228 for the
full target graph, a 68% reduction in planned commands. These are reported
measurements from that run, not a new run by this review. The CTest time is not
an end-to-end workflow duration: configure time, build wall time, cache state,
and machine load were not separated here. The 444.6-second full-CI run had
three failing tests and three ROM target failures, so it is not a passing
throughput baseline or an acceptance result.

For useful follow-up telemetry, record configure, build, and CTest wall time
separately for a clean first run and a normal incremental edit. Retain the
selected-test count and build command count alongside those timings. That
would show whether this preset improves the feedback loop without confusing
incremental speed with clean-build or full-acceptance evidence.

## Source evidence

- Preset and exact selected target/name lists: [`CMakePresets.json`](../../../CMakePresets.json)
- Developer-check scope and full-CI acceptance statement: [`docs/testing.md`](../../../docs/testing.md)
- Test registration, fixtures, and selected test modes: [`tests/CMakeLists.txt`](../../../tests/CMakeLists.txt)
- API policy live-header and mutation checks: [`tests/cmake/vector_api_policy.cmake`](../../../tests/cmake/vector_api_policy.cmake)
