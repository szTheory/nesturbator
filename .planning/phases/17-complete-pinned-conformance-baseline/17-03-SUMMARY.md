---
phase: 17-complete-pinned-conformance-baseline
plan: "03"
subsystem: testing
tags: [accuracycoin, protected-baseline, pinned-rom, ci]

requires:
  - phase: 17-01
    provides: authenticated old/new ROM inventories, protected old-pin observations, source-linked migration table
  - phase: 17-02
    provides: bounded observation runner, actual report/raw formats, measured page caps
provides:
  - immutable protected-main scoreboard and metadata handoff
  - fresh old/new protected-key observation verdict in normal CI
  - production-path regression and local-git handoff controls
affects: [17-04, accuracycoin, conformance-ci]

actuals:
  tokens: 35857
  tasks: 2
  commits: 2
commits: 2
plan_head_before: 929be8147c9f58cb0ce2a869704c8dcd79892cb3
plan_head_after: 514beda30507bcb4d3bfe48f7992e5033f458b4f

tech-stack:
  added: []
  patterns:
    - immutable-revision git authority handoff
    - CTest setup fixture for fresh pinned observations
    - reason-checked production verdict mutations

key-files:
  created:
    - tests/accuracy/scoreboard-main-metadata.txt
    - tests/cmake/accuracycoin_protected.cmake
    - tests/cmake/accuracycoin_regression_controls.cmake
    - tests/cmake/accuracycoin_baseline_fetch.cmake
  modified:
    - tests/accuracy/test_scoreboard.c
    - tests/accuracy/test_conformance.c
    - tests/accuracy/migration.tsv
    - tests/CMakeLists.txt
    - tests/cmake/prepare_scoreboard_baseline.cmake
    - tests/cmake/verify_scoreboard_regression.cmake
    - runner/accuracycoin.c
    - runner/accuracycoin.h
    - include/nesturbator.h
    - README.md

key-decisions:
  - "Read protected scoreboard and identity from one fetched immutable commit; the old-main manifest fallback is permitted only from that same commit."
  - "A protected pass requires fresh raw-backed observations for both retained old controls and the candidate pin; only four exact source-reviewed mappings can admit a measured cross-pin terminal failure."
  - "Keep all CONF requirements pending until Phase 17 verification; do not activate the new canonical fixture in Plan 17-03."

patterns-established:
  - "Fetch protected branch authority once, then use the resolved commit object for every authority file."
  - "Use CTest fixture setup to remove stale evidence and create invocation-bound measurements before verdict consumers run."
  - "Regression controls must assert the production verifier's intended diagnostic, not just a generic nonzero exit."

requirements-completed: []
coverage:
  - id: D1
    description: "Fetch and validate protected scoreboard plus identity from one immutable protected-main revision, with a same-revision legacy manifest fallback."
    verification:
      - kind: integration
        ref: "accuracycoin.baseline_fetch; accuracy.scoreboard.ci_missing_baseline; accuracy.scoreboard.ci_empty_baseline_rejected"
        status: pass
    human_judgment: false
  - id: D2
    description: "Retain all old protected passes and require fresh cold old/new reports and raw evidence for every protected key."
    verification:
      - kind: integration
        ref: "accuracycoin.protected_observation; accuracycoin.protected"
        status: pass
    human_judgment: false
  - id: D3
    description: "Reject measured status, missing evidence, candidate pin, mixed revision, key-set, and migration regressions through the production verdict."
    verification:
      - kind: integration
        ref: "accuracycoin.regression_controls; accuracy.scoreboard.no_lost_pass"
        status: pass
    human_judgment: false
  - id: D4
    description: "Resolve the unclassified CONF-04 edge-probe assumption with exact source-backed per-key migration review."
    verification:
      - kind: manual_procedural
        ref: "17-03-SUMMARY.md#conf-04-manual-source-review"
        status: pass
    human_judgment: false

duration: 42min
completed: 2026-10-23
status: complete
---

# Phase 17 Plan 03: Protected Conformance Baseline Summary

**Protected AccuracyCoin scores now derive from one immutable main revision and fresh raw-backed measurements of every retained old/new key.**

## Performance

- **Duration:** 42 min
- **Started:** 2026-10-23T23:16:46Z
- **Completed:** 2026-10-23T23:58:10Z
- **Tasks:** 2
- **Files modified:** 15

## Accomplishments

- Bound protected CI authority to fetched `refs/heads/main` commit `9f9386c661f6fb8f6063bf756655713aedadae77`. The scoreboard and ROM manifest are read from that exact commit object. CI rejects missing, unfetchable, empty, malformed or mismatched authority and never derives its baseline from candidate files.
- Added the matching complete 17-row old-pin local snapshot identity. The snapshot records old pin `673ef550db296136d52229961e7d39366116882a`, ROM SHA-256 `4fe8c2bc9abc6f4d418da47b73f62cba89fcacd950fae763097a1681d650e839`, inventory SHA-256 `3271803cdc63c4b4a83cf65453b817dc66ec65ed97863ad19261b56f36821c84`, and scoreboard SHA-256 `7a394b78803770cbde2ff047c9bfe0b21226d1c97e8d378e0f322b8754b7b9a4`.
- Made normal CI execute each protected page from both authenticated pins in fresh cold runs. The protected verifier checks report identity, inventory mapping, final raw samples, status classification, candidate scoreboard claims and the source-linked migration record. Same-pin protected passes must measure as pass in this invocation.
- Added production-verifier regression controls and a network-free local Git fetch fixture. Controls assert reason-specific rejection for measured fail/skip/not-run/timeout, missing report/raw evidence, candidate key substitution, candidate-only pin edits, mixed authority revisions, and missing or disguised migration dispositions.
- Updated README, runner comments and the public header comment with the fresh-evidence and protected-authority behavior. The old pin remains canonical for Plan 17-04.

## Protected Measurements and Migration Coverage

The CTest setup fixture cleared its evidence directory and performed six independent cold page runs using 32 boot frames, 5 frames per page transition, 2 start/release frames and an 1800-frame execution budget. Every retained old and mapped new observation was terminal and passing. Raw bytes and guest frames below come from the fixture's `report.tsv` records; the matching `raw.tsv` final sample is checked by the verdict.

| Protected key | Disposition | Old pin observation | New pin observation |
|---|---|---|---|
| Absolute Indexed | unchanged | `0x01 pass`, 11 frames | `0x01 pass`, 12 frames |
| CHR ROM is not writable | unchanged | `0x01 pass`, 151 frames | `0x01 pass`, 159 frames |
| Delta Modulation Channel | unchanged | `0x01 pass`, 120 frames | `0x01 pass`, 120 frames |
| Frame Counter 4-step | changed-meaning | `0x01 pass`, 120 frames | `0x01 pass`, 120 frames |
| Frame Counter 5-step | changed-meaning | `0x01 pass`, 120 frames | `0x01 pass`, 120 frames |
| Frame Counter IRQ | changed-meaning | `0x01 pass`, 120 frames | `0x01 pass`, 120 frames |
| Indirect | unchanged | `0x01 pass`, 11 frames | `0x01 pass`, 12 frames |
| Indirect, X | unchanged | `0x01 pass`, 11 frames | `0x01 pass`, 12 frames |
| Indirect, Y | unchanged | `0x01 pass`, 11 frames | `0x01 pass`, 12 frames |
| Length Counter | unchanged | `0x01 pass`, 120 frames | `0x01 pass`, 120 frames |
| Length Table | unchanged | `0x01 pass`, 120 frames | `0x01 pass`, 120 frames |
| PPU Read Buffer | unchanged | `0x41 pass`, 151 frames | `0x41 pass`, 159 frames |
| PPU Register Mirroring | unchanged | `0x01 pass`, 151 frames | `0x01 pass`, 159 frames |
| PPU Register Open Bus | changed-meaning | `0x01 pass`, 151 frames | `0x01 pass`, 159 frames |
| Palette RAM Quirks | unchanged | `0x01 pass`, 151 frames | `0x01 pass`, 159 frames |
| Relative | unchanged | `0x01 pass`, 11 frames | `0x01 pass`, 12 frames |
| Zero Page Indexed | unchanged | `0x01 pass`, 11 frames | `0x01 pass`, 12 frames |

All 17 migration rows link their new observation to an exact page report, attempt, row, raw byte, frame count and status. Thirteen retain unchanged source mappings. The only four `changed-meaning` rows are the three frame-counter APU tests and PPU Register Open Bus; all four new-pin measurements currently pass. A future cross-pin nonpass is allowed only as an observed terminal failure on one of those exact reviewed rows, with the old passing observation retained. It cannot admit skip, not-run or timeout.

## CONF-04 Manual Source Review

The deterministic edge probe left CONF-04 unclassified; it did not decide this policy. I reviewed the exact old/new pinned `AccuracyCoin.asm` source links recorded in `migration.tsv`. Thirteen mapped test bodies and result/routine symbols are unchanged. Three APU frame-counter tests retain their assertion boundaries but replace the old `LDA #2` / `STA $4014` cycle setup with `OAMDMAWithPage2`. PPU Register Open Bus retains its assertion cases while its initial setup changes from the `$4015` write to `OAMDMAWithPage2`. These four source facts justify the explicit `changed-meaning` rows; they do not erase the old controls or grant a blanket reset. The production verdict independently requires actual old passes, actual new observations and a terminal candidate result that matches the raw byte. The mutation controls verify same-pin regressions separately from this cross-pin review.

## Regression and Authority Controls

- `accuracycoin.regression_controls` exercises valid same-pin and old-to-new positive controls; measured fail, skip, not-run and execution timeout; missing page row and raw file; a score-key substitution that preserves row count; candidate manifest pin-only mutation; mixed protected revision; removed/disguised migration; source-supported PPU Read Buffer `0x39`; malformed PPU Read Buffer `0x05`; a measured changed-meaning terminal failure; and the same failure mislabeled `unchanged`.
- `accuracycoin.baseline_fetch` builds owned local repositories and checks exact commit/revision and scoreboard bytes for dedicated metadata, absent dedicated metadata and local-snapshot metadata fallback. It rejects missing `main`, missing/empty scoreboard and invalid protected metadata with their intended reasons.
- The existing `accuracy.scoreboard.no_lost_pass` check now removes a real retained key and asserts the production verifier's protected-key diagnostic. CI missing/empty authority tests also assert specific handoff/parser diagnostics instead of relying on generic `WILL_FAIL` results.

## Task Commits

1. **Task 1: Bind protected-main authority to actual measured protected results** — `81f6785` (`feat`)
2. **Task 2: Prove pin resets and claimed-but-unmeasured passes are rejected** — `514beda` (`test`)

The plan metadata commit is created after this summary's self-check and state updates.

## Files Created/Modified

- `tests/cmake/prepare_scoreboard_baseline.cmake` — immutable protected commit fetch, strict metadata and legacy-manifest identity validation.
- `tests/cmake/accuracycoin_protected.cmake` — fresh old/new cold observations for the protected pages.
- `tests/cmake/accuracycoin_regression_controls.cmake` and `tests/cmake/accuracycoin_baseline_fetch.cmake` — reason-specific verdict and local-Git authority controls.
- `tests/accuracy/test_scoreboard.c` — actual evidence and migration verification; specific regression diagnostics.
- `tests/accuracy/scoreboard-main.txt`, `tests/accuracy/scoreboard-main-metadata.txt`, `tests/accuracy/migration.tsv` — complete local protected snapshot identity and fresh report links.
- `tests/CMakeLists.txt`, `tests/cmake/verify_scoreboard_regression.cmake`, `tests/accuracy/test_conformance.c` — unconditional CI coverage and classifier/migration assertions.
- `runner/accuracycoin.c`, `runner/accuracycoin.h`, `include/nesturbator.h`, `README.md` — observer timestamp and paired behavior documentation.

## Decisions Made

- Use the exact fetched commit object for all remote protected authority. The retained pre-metadata main revision derives its old pin identity only from that same revision's canonical manifest row.
- Require fresh old and new protected evidence before accepting any migration. A row's candidate status and raw byte must match the current measured observation.
- Keep CONF-01 through CONF-04 pending until Phase 17's final verification; Plan 17-04 owns canonical fixture activation and CI placement.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Validate CMake pin/hash lengths without unsupported interval matching**
- **Found during:** Task 2 (local protected-main fetch controls)
- **Issue:** The local Git fixture showed CMake's regex handling rejected a valid immutable SHA even though it contained the required lowercase hex characters.
- **Fix:** Check lowercase-hex syntax and exact string length separately for revisions, ROM hashes and inventory hashes.
- **Files modified:** `tests/cmake/prepare_scoreboard_baseline.cmake`
- **Verification:** `accuracycoin.baseline_fetch`; full CI
- **Committed in:** `514beda`

**2. [Rule 1 - Bug] Parse dedicated protected metadata strictly by key/value fields**
- **Found during:** Task 2 (dedicated metadata positive control)
- **Issue:** A CMake line regex rejected valid `rom_sha256` metadata from the local repository fixture.
- **Fix:** Parse one `=` separator, reject empty/unknown/duplicate keys and validate required identity fields before exporting the handoff.
- **Files modified:** `tests/cmake/prepare_scoreboard_baseline.cmake`
- **Verification:** `accuracycoin.baseline_fetch` dedicated metadata and invalid metadata cases; full CI
- **Committed in:** `514beda`

**3. [Rule 1 - Bug] Preserve exact candidate manifest mismatch diagnostics**
- **Found during:** Task 2 (candidate pin-only regression control)
- **Issue:** Candidate manifest validation set a specific error, but a combined condition replaced it with a generic pin identity message.
- **Fix:** Return the manifest validator's reason directly so the negative control proves the intended pin/ROM mismatch rejection.
- **Files modified:** `tests/accuracy/test_scoreboard.c`
- **Verification:** `accuracycoin.regression_controls`; full CI
- **Committed in:** `514beda`

**Total deviations:** 3 auto-fixed bugs (Rule 1)
**Impact on plan:** Each fix made the specified fetch/verdict controls enforce and report the intended contract; no scope expansion.

## Issues Encountered

- The first Task 2 full workflow passed 660/661; the unrelated serial `retroarch.session.controls` test reported a transient driver trace write error. Its standalone retry passed 1/1 in 274.08 seconds; the subsequent complete workflow passed all 661 tests. No RetroArch code was changed.
- The final Task 2 workflow passed 661/661 tests in 376.32 seconds and generated the library, runner and libretro packages. Task 1's full workflow passed 659/659 tests in 383.84 seconds and also generated all three packages.

## Verification

- `cmake --workflow --preset ci` after Task 1 — passed, 659/659 tests; all four workflow steps completed.
- `ctest --preset ci -R '^(accuracycoin\.(protected|protected_observation|regression_controls|baseline_fetch)|accuracy\.scoreboard.*)$' --no-tests=error --output-on-failure` — passed, 8/8 tests in 1.33 seconds.
- `ctest --preset ci -R '^retroarch\.session\.controls$' --no-tests=error --output-on-failure` — passed, 1/1 in 274.08 seconds after the transient failure noted above.
- Final `cmake --workflow --preset ci` — passed, 661/661 tests in 376.32 seconds; library, runner and libretro packages generated.
- `git diff --check` passed. The only remaining working-tree change is the preexisting `.planning/config.json` edit, unstaged and untouched by this plan.

## User Setup Required

None — no external service or manual configuration is required.

## Next Phase Readiness

Plan 17-04 can evaluate canonical pin activation and measured CI placement using the protected authority and fresh-observation gate delivered here. The old canonical fixture stays active until that plan. Phase-level CONF requirements remain pending until the complete Phase 17 goal-backward verification.

## Self-Check: PASSED

The summary and its key implementation artifacts exist; task commits `81f6785` and `514beda` are ancestors of HEAD. The focused protected set passed 8/8, the final full workflow passed 661/661, and `git diff --check` is clean. The only preexisting working-tree edit remains `.planning/config.json`, unstaged and untouched.

---
*Phase: 17-complete-pinned-conformance-baseline*
*Completed: 2026-10-23*
