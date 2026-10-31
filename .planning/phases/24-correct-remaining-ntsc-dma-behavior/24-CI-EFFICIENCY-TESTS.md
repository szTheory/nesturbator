# Phase 24 CI efficiency: test topology and developer feedback

## Scope and evidence

This is read-only analysis for the current test topology. No tests, workflows,
ROMs, campaigns, builds, or CI triggers were run for this note. The authorized
full local CI was active while this research was in progress, so its live
`build/ci/Testing/Temporary` files were not used as duration evidence.

The latest stable local measurement found is from the 2026-10-26 CI efficiency
task: 675/675 tests passed in 356.11 seconds of CTest time; the focused session
matrix took 284.61 seconds and took 275.22 seconds when included in that full
run. This predates Phase 24 and is not a current-suite forecast. The present
Phase 24 acceptance inventory has 707 tests with `NESTURBATOR_ACCURACYCOIN_FULL`
enabled in the local `ci` cache. The combined current runtime remains
unqualified; the 39-minute window in `24-ACCEPTANCE.md` is a resource ceiling,
not a measured completion time.

Evidence sources: `CMakePresets.json`, `tests/CMakeLists.txt`,
`tests/retroarch/CMakeLists.txt`, `tests/cmake/accuracycoin_campaign.cmake`,
`tests/cmake/accuracycoin_protected.cmake`,
`.github/workflows/ci.yml`, `24-CI-SELECTION.tsv`,
`24-ACCEPTANCE.md`, `.planning/research/2026-10-27-CI-EFFICIENCY.md`, and
`.planning/quick/261026-i8z-reduce-ci-overhead-using-the-recovered-c/`.

## Prioritized recommendations

### 1. Add a named fast local feedback tier; keep full CI as the acceptance gate

`dev` currently builds and tests the broad suite with four CTest jobs. There is
no named local tier for the owned emulator-unit closure. Add a small preset or
documented command for the directly affected unit tests and their build
targets; for this phase that means `bus.unit`, `core.apu`, `core.controller`,
and the relevant `core.state_fork` entry. Keep `cmake --workflow --preset ci`
unchanged as the required behavior-change acceptance run.

The fast tier should select exact names, require at least one test, and record
the resolved test list before execution. Do not make it a broad exclusion such
as “everything except campaign/session/perf”: exclusions can silently omit a
new required check. Do not select `accuracycoin.activation` by name under the
FULL configuration without inspecting fixture closure; CTest will add its
campaign setup. The focused Phase 24 unit closure was run successfully during
implementation, but its wall time was not recorded. A seconds-level saving is
plausible, not measured. The older full local run spent 356.11 seconds in
CTest, with 284.61 seconds in the session matrix; these values only establish
that a focused entry point could shorten iteration, not its exact saving.

### 2. Preserve fixture reuse and fresh evidence; run shared consumers together

The full AccuracyCoin campaign is one `FIXTURES_SETUP` for
`accuracycoin.activation`, `accuracycoin.nmi_targets`, and
`accuracycoin.v5_targets` when FULL is enabled. The protected observation is a
separate fixture used by protected-pass and regression controls. CTest runs a
fixture setup once for all consumers in one selected test set, and automatically
adds required setup tests to regex-selected subsets ([CTest 3.25 fixture
semantics](https://cmake.org/cmake/help/v3.25/prop_test/FIXTURES_REQUIRED.html)).
Keep consumers sharing each fixture in one CTest invocation. Splitting those
consumers into separate invocations reruns the cold setup each time. The
campaign has a 370-second CTest timeout; the protected setup has a 760-second
timeout. These are upper bounds, not expected savings; no repeated-invocation
timing was measured.

Keep each setup fresh. `accuracycoin_protected.cmake` removes its work tree
before collecting old/new-pin observations, `accuracycoin.report_scratch`
removes the report fixture, and the campaign clears the report and per-page
outputs it consumes. Do not replace these checks with a cached report, suppress
fixture setup with `-FS`/`-FA`, or treat a stale passing artifact as current
evidence. The campaign writes under a stable ignored build path, so archive
prior evidence before a new run when it must be preserved, as
`24-ACCEPTANCE.md` requires.

The standalone old-pin `accuracycoin.page2/14/17` tests overlap in page and ROM
with the protected observation, but use the strict page command and committed
old-pin scoreboard, while protected observation records cold raw/report
evidence for the protected-key verifier. Keep both unless a review proves the
strict path remains independently asserted; no safe duration saving is
measured for removing them.

### 3. Keep deadline-sensitive tests isolated; tune scheduling from clean data

The CI test preset uses four jobs. The RetroArch session source-root,
readiness, and control tests are `RUN_SERIAL`; the controls test has a 900-second
hang cap and documents that concurrent load can inflate its measured timing
delta. The prior efficiency investigation measured the local matrix at 284.61
seconds and explicitly rejected removing `RUN_SERIAL` as an easy speedup.
Retain those isolation and per-test limits. A `RESOURCE_LOCK` only excludes
tests that share that lock; it does not isolate a timing-sensitive test from
all unrelated CTest work ([CTest 3.25 `RUN_SERIAL`](https://cmake.org/cmake/help/v3.25/prop_test/RUN_SERIAL.html),
[`RESOURCE_LOCK`](https://cmake.org/cmake/help/v3.25/prop_test/RESOURCE_LOCK.html)).

After the current run finishes, use its attributable CTest durations plus a
clean-tree repeat to decide whether to seed `COST` for long, parallel-safe tests
so they start early on fresh CI trees. CTest learns weighted cost history in a
reused build directory, but clean hosted jobs start without that history; tests
without explicit cost initially have cost zero ([CTest 3.25 `COST`](https://cmake.org/cmake/help/v3.25/prop_test/COST.html)).
`runner.write_hashes` already seeds cost 50 based on measured hosted runs. Do
not raise the four-job level or add costs based only on timeout caps. No current
Phase 24 scheduling saving has been measured.

### 4. Retain correctness gates; improve machine-readable timing evidence

The fast local tier is for iteration only. Keep full CI's deterministic hash
generation and content comparison, six-platform hash equality, the full
AccuracyCoin report and every existing fixture consumer. Keep the hosted cold
protected-main baseline preparation and evidence-derived protected-key check;
the local fallback scoreboard is not that authority. Keep the independent
trace/raw-value DMA oracles, the +1.00% performance threshold and
`tests/perf/allow.txt` policy. A hash comparison alone does not replace an
independent behavior oracle, and local hashes or a local campaign do not prove
cross-platform equality or protected-main acceptance.

For later scheduling work, retain the existing uploaded `LastTest.log` and add
the CTest cost file and resolved JSON inventory to the same run-scoped evidence
bundle. CI currently uploads `LastTest.log` with AccuracyCoin evidence, while
the `ci` test preset does not emit JUnit and `CTestCostData.txt` remains in the
build tree. The inventory plus source/run identity would distinguish changed
selection from changed timing; the cost file would show CTest's learned data.
This adds no test or weaker gate. The likely benefit is better diagnosis and
future scheduling decisions, not a measured runtime reduction. Preserve the
current behavior if the required artifact cannot be produced; missing
telemetry must not become a skipped test.

## No-go shortcuts

- Do not drop the AccuracyCoin campaign, protected observations, RetroArch
  session matrix, hash checks, native-state/hash inventory, or performance gate
  from required coverage to shorten the full workflow.
- Do not promote checkout scoreboard fallback as cold protected-main
  authority, derive expected values from the implementation under test, alter
  accepted DMA raw values, or weaken the +1.00% threshold.
- Do not add test retries for real campaign/session failures. CTest's
  `--repeat until-pass` retries failures and can hide a first failing
  observation; it is not an acceptance shortcut.
- Do not use CMake 4.5's [`ctest --out-of-date`](https://cmake.org/cmake/help/latest/manual/ctest.1.html#cmdoption-ctest-out-of-date)
  as a local shortcut: the project supports CMake 3.25, and an incremental
  selection is not a substitute for the required full run.

## Next measurement

When the authorized current CI completes, record the actual workflow and CTest
durations, selected test count, skips, and campaign/session outcomes with its
source identity. A clean repeat would require separate authority; compare such
a run before assigning new CTest costs or claiming a full-suite saving. This
analysis does not authorize another workflow or a hosted trigger.
