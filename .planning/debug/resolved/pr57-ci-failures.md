---
status: resolved
trigger: "fix PR #57 CI failures (see the ci-efficiency-findings memory)"
created: 2026-10-26
updated: 2026-10-26
---

# Debug: PR #57 CI failures

Branch: gsd/phase-19-jam-reporting-and-sprite-0-hit. CI run 40227367491.

## Symptoms

- expected: every required check on PR #57 passes so it can squash-merge into main.
- actual: four jobs fail — hygiene, build (windows-2025 x64, ci-msvc), build (windows-11-arm arm64, ci-msvc), retroarch-e2e. "CI required" fails as a consequence.
- errors:
  1. hygiene.format: clang-format 18 violations in runner/accuracycoin.c (around lines 708-1410) and tests/accuracy/test_conformance.c (around 777-1340).
  2. Windows x64 and arm64: MSVC warning-as-error C4132 at libretro/libretro.c:49, `static const uint32_t blank[NT_PIXELS];` (const object should be initialized; likely needs `= {0}`).
  3. retroarch-e2e: "HOSTED_MARKER wrong-origin control lacked two native-valid anchors" raised at tests/retroarch/run_retroarch.cmake:729, called from hosted_marker_session at :745. Suspected cause: the 19-03 presentation-cache change. This job runs only on the hosted runner.
- timeline: introduced by phase 19 changes on this branch; main is green.
- repro:
  1. `cmake --workflow --preset hygiene` (about 31 s locally; clang-format 18 at /opt/homebrew/opt/llvm@18).
  2. MSVC only — inspect libretro/libretro.c:49; verify with `gh run view` logs of the Windows jobs.
  3. `gh run view 40227367491 --log-failed` for the retroarch-e2e job; local RetroArch reproduction if available, else reason from the log and tests/retroarch/run_retroarch.cmake.

## Notes

- Line numbers were recorded 2026-10-26; verify before acting.
- Project rules (CLAUDE.md): every behaviour change needs a test in `cmake --workflow --preset ci`; clean-room; C17; no personal paths. Do not open GPL emulator source.
- Out of scope for this session: the CI efficiency items (QUIT_GRACE variable, label split, etc.) — those go to a later /gsd-quick.

## Current Focus

bug_class: Bohrbug (round 2: both deterministic; the report failure is order-dependent but deterministic given order)
hypothesis: (4) tests/libretro/libretro_host.c:2803 `static const uint32_t black[W * H];` has no initializer -> MSVC C4132 under /WX; the only such object left in the tree. (5) accuracycoin.report passes a two-level work path (tests/accuracycoin/report) to make_directory, a single-level mkdir; the parent exists only when accuracycoin.driver/jam ran first, so random order fails.
reasoning_checkpoint:
  hypothesis: "(5) mkdir(work) fails with ENOENT because tests/accuracycoin/ does not exist when report runs before any test that creates it"
  confirming_evidence:
    - "fresh build dir, ctest -R ^accuracycoin.report$ alone: CHECK(make_directory(work)) failed at test_conformance.c:923"
    - "same build dir after mkdir -p tests/accuracycoin: passes (0.17 s)"
  falsification_test: "report still failing in a fresh dir after make_directory creates parents"
  fix_rationale: "the test program owns its work directory; creating missing parents removes the dependency on another test having run"
  blind_spots: "Windows drive prefix in the parent walk (mkdir(\"C:\") fails harmlessly; only the final mkdir result counts)"
  candidate_causes:
    - "code: single-level mkdir in test_conformance.c"
    - "config: tests/CMakeLists.txt gives a nested path with no fixture/setup creating the parent"
  and_gate: "yes - both the nested path and the single-level mkdir are needed; fixing either removes it. Fix the code side so every make_directory caller is robust."
next_action: session manager pushes 3ea2bbf..b945163 and watches both ci-msvc lanes and Nightly suite-flake on PR #57

## Evidence

- timestamp: 2026-10-26
  checked: retroarch-marker-progression artifact of run 40227367491, every anchor-*.out
  found: all six read "native-valid format=1 revision=7 envelope=RASTATE1/MEM/pad8/END"; run_retroarch.cmake:727-728 matches "revision=6"
  implication: anchors are valid; the assertion is stale. frame_index.c prints the compiled NESTURBATOR_BEHAVIOUR_REVISION, which 1cb7976 bumped to 7 on this branch. Presentation-cache hypothesis eliminated.
- timestamp: 2026-10-26
  checked: libretro/libretro.c:49
  found: `static const uint32_t blank[NT_PIXELS];` with no initializer (zero by static storage, but MSVC warns C4132 and /WX makes it an error)
  implication: add `= {0}`; behaviour unchanged.
- timestamp: 2026-10-26
  checked: clang-format 18.1.8 --dry-run --Werror over all tracked C sources
  found: violations in runner/accuracycoin.c (12), tests/accuracy/test_conformance.c (9), tests/libretro/libretro_host.c (38), tests/runahead_model.h (2) - more files than the debug note listed
  implication: reformat all four with clang-format -i; diffs are line rewraps and comment alignment only.
- timestamp: 2026-10-26
  checked: new guard tests/cmake/retroarch_revision_literals.cmake against HEAD's run_retroarch.cmake and the fixed one
  found: HEAD copy fails (two revision=6 lines named); fixed copy passes
  implication: the class (hosted-only literal goes stale) is now caught by the ci preset.
- timestamp: 2026-10-26
  checked: the new header-derived anchor match run against the downloaded wrong-origin anchor-*.out files and a synthetic revision=6 line
  found: derived line "native-valid format=1 revision=7 envelope=RASTATE1/MEM/pad8/END"; both real anchors match, the stale line does not
  implication: the hosted wrong-origin check will accept the real anchors and still reject a mismatched revision.
- timestamp: 2026-10-26
  checked: first local ci preset after the fixes
  found: accuracycoin.activation failed: baseline build_inputs_sha256 dc9c8a9e... vs current. runner/accuracycoin.c, tests/accuracy/test_conformance.c and tests/CMakeLists.txt are AccuracyCoin campaign inputs (accuracycoin_input_fingerprint.cmake)
  implication: formatting fix forces a baseline re-record; moved the new test registration to tests/retroarch/CMakeLists.txt to keep tests/CMakeLists.txt out of the change. Full campaign rerun (build/pr57-full, FULL=ON): report.tsv sha b73c7b25... identical to the pinned campaign_report_sha256; only build_inputs_sha256, host_elapsed_guest_ns, source_revision and source_dirty_reason updated.

- timestamp: 2026-10-26 (round 2)
  checked: compiler sweep - every target built with Apple clang 21 and -Wdefault-const-init-var -Wdefault-const-init-var-unsafe -Wdefault-const-init-field (99 translation units); plus a regex sweep of all 136 tracked .c/.h files for const non-pointer objects declared without initializer (covers Windows/fuzz-only code clang did not compile)
  found: exactly one hit in both sweeps: tests/libretro/libretro_host.c:2803 `static const uint32_t black[W * H];`. (An earlier grep -v 'libretro/libretro.h' had silently dropped libretro_host.c because '.' matched '_'.)
  implication: one-line `= {0}` fix closes the MSVC C4132 class; clang 21 flags the same construct, so adding -Wdefault-const-init-var where supported catches it locally under the ci preset.
- timestamp: 2026-10-26 (round 2)
  checked: fresh build dir, ctest -R '^accuracycoin\.report$' alone, then again after mkdir -p tests/accuracycoin
  found: fails at test_conformance.c:923 CHECK(make_directory(work)); passes once the parent exists
  implication: order dependency confirmed; accuracycoin.driver and accuracycoin.jam create tests/accuracycoin/ via file(MAKE_DIRECTORY), which hides it in the fixed ci order.
- timestamp: 2026-10-26 (round 2)
  checked: ctest --show-only=json-v1 - every direct-executable (non cmake -P) test arg under the build dir that is nested and absent at configure; ran each whose parent dir is missing after configure (8 runner.signal.*, libretro.parity.m1sxrom.run2, libretro.parity.m4tkrom.run2, 3 retroarch.session.*) in isolation from removed dirs
  found: all pass in isolation (fixtures create their dirs); accuracycoin.report is the only test with the nested-work-dir dependency. cmake -P tests use recursive file(MAKE_DIRECTORY).
  implication: fix scope is test_conformance.c make_directory only.

- timestamp: 2026-10-26 (round 2)
  checked: guard and regression tests
  found: -Wdefault-const-init-var detected on Apple clang 21; with the old `black` line, libretro.host fails to build under CMAKE_COMPILE_WARNING_AS_ERROR, passes with `= {0}`. accuracycoin.report_fresh fails with the old make_directory (test_conformance.c:923), passes with the fix.
  implication: both classes are now caught by the local ci preset.
- timestamp: 2026-10-26 (round 2)
  checked: full AccuracyCoin campaign rerun in build/pr57-full (FULL=ON) on the final tree
  found: report.tsv sha256 b73c7b25... equals campaign_report_sha256; new build_inputs_sha256 f87a1dc2..., host_elapsed_guest_ns 4728137000
  implication: baseline re-recorded the same way as round 1; only the four non-result fields change.

## Eliminated

- hypothesis: the 19-03 presentation-cache change broke the HOSTED_MARKER wrong-origin control
  evidence: the retained anchors are native-valid and print revision=7; only the expected string (revision=6) was wrong
  timestamp: 2026-10-26

## Resolution

root_cause: round 1 - (1) phase-19 edits left four C files not formatted to clang-format 18; (2) libretro/libretro.c:49 `static const uint32_t blank[NT_PIXELS];` without an initializer (MSVC C4132 under /WX); (3) 1cb7976 bumped NESTURBATOR_BEHAVIOUR_REVISION 6->7 but tests/retroarch/run_retroarch.cmake hardcoded "revision=6" in the hosted-only wrong-origin check. Round 2 - (4) tests/libretro/libretro_host.c:2803 `static const uint32_t black[W * H];`, the same C4132 class and the only remaining instance; (5) accuracycoin.report's work dir is two levels deep and make_directory was a single-level mkdir, so the test depended on accuracycoin.driver/jam having created tests/accuracycoin/ first (AND-gate: nested path + non-recursive mkdir); pre-existing, exposed by the nightly --schedule-random run.
fix: (1) clang-format -i; (2) `= {0}`; (3) header-derived anchor line + retroarch.revision_literals guard; (4) `= {0}` plus -Wdefault-const-init-var in nesturbator_warnings when check_c_compiler_flag accepts it; (5) make_directory creates missing parents; new ci tests accuracycoin.report_scratch (FIXTURES_SETUP) + accuracycoin.report_fresh on a removed nested tree; README documents both; AccuracyCoin baseline metadata re-recorded.
oracle_type: specified (exact anchor line; report command must succeed on a fresh nested tree; compiler diagnostic)
verification: final tree - `cmake --workflow --preset ci` 673/673 pass (incl. accuracycoin.report_fresh, accuracycoin.activation); `cmake --workflow --preset hygiene` 13/13 pass; fresh build dir: accuracycoin.report and report_fresh pass from a removed tests/accuracycoin; `ctest --preset ci --repeat until-fail:3 --schedule-random` from a removed tests/accuracycoin: 673/673 pass, each test 3 times (1124 s); guard red/green checked; full campaign report byte-identical. MSVC verified only by CI after push.
commits: 7857a4e style, b4bb6cf fix(libretro) C4132, 1d060bb test(retroarch), 3ea2bbf fix(tests) C4132 + warning guard, b945163 fix(accuracy) report parents + baseline
files_changed: [libretro/libretro.c, runner/accuracycoin.c, tests/accuracy/test_conformance.c, tests/libretro/libretro_host.c, tests/runahead_model.h, tests/accuracy/baseline-metadata.txt, tests/retroarch/run_retroarch.cmake, tests/retroarch/CMakeLists.txt, tests/cmake/retroarch_revision_literals.cmake, README.md, CMakeLists.txt, tests/CMakeLists.txt]

## CI confirmation

- 2026-10-26: after push of b945163, CI run 40346475858 is fully green (CI required, hygiene, both ci-msvc lanes, retroarch-e2e, hash-equality). Nightly run 40346475804: suite-flake passes; vectors-full fails twice at cpu.vectors-full.fetch with a stalled upstream git transfer (curl 28, bad pack header) from SingleStepTests/65x02. It passed on the previous push (run 40336124755) and no fetch code changed, so this is external and outside this session.
