---
phase: 02-the-cpu-matches-the-public-vectors
plan: 07
subsystem: testing
tags: [cmake, ctest, git, sparse-checkout, 65x02-vectors, github-actions, nightly, provenance]

requires:
  - phase: 02-the-cpu-matches-the-public-vectors
    provides: "02-03: committed sample and manifest pin; 02-06: all 256 opcodes and NESTURBATOR_ALL_OPCODES"
provides:
  - "tests/cmake/fetch_vectors.cmake (DIR PINS SOURCE_DIR GIT [WRITE_PINS COMMIT]): sparse, blobless, depth-1 git fetch at the pin, size and SHA-256 check of 256 files"
  - "tests/vectors/pins.txt: pin line plus 256 sha256sum-layout lines with sizes, 1,081,529,097 B"
  - "tests/cmake/vectors_full_run.cmake, vectors_sample_match.cmake, pins_check.cmake"
  - "option NESTURBATOR_VECTORS_FULL, cache variable NESTURBATOR_VECTORS_DIR, fixture vectors_full, label vectors-full"
  - "tests cpu.vectors-full.fetch, .sample-match, .00 to .ff; vectors.pins and vectors.pins.selftest in ci"
  - "presets vectors-full (configure, build, test, workflow)"
  - ".github/workflows/nightly.yml: jobs vectors-full (output failed) and report (rolling issue labelled nightly)"
affects: [02-08, ci, nightly]

actuals:
  tokens: 13800
  tasks: 3
  commits: 3
plan_head_before: 83c7af504e2d8e2c9e182a830b7f248aa08ef362
plan_head_after: 774116f14fb5d7cbaf465a0de784c926d81b4844

tech-stack:
  added: []
  patterns:
    - "Network tests live behind an OFF option and their own preset, so the ci lane never fetches"
    - "A fetch fixture does nothing when every file already verifies by size and SHA-256; any mismatch re-fetches and re-verifies"
    - "Binary provenance compared in CMake with file(READ ... OFFSET LIMIT HEX), chunk by chunk"

key-files:
  created:
    - tests/cmake/fetch_vectors.cmake
    - tests/cmake/vectors_full_run.cmake
    - tests/cmake/vectors_sample_match.cmake
    - tests/cmake/pins_check.cmake
    - tests/vectors/pins.txt
    - .github/workflows/nightly.yml
  modified:
    - tests/CMakeLists.txt
    - CMakePresets.json
    - README.md
    - .planning/preparation/ENGINEERING.md
    - .planning/preparation/CONFORMANCE.md

key-decisions:
  - "fetch_vectors.cmake sets GIT_TERMINAL_PROMPT=0 besides the two low-speed variables, so an unattended fetch never waits on a credential prompt"
  - "The report job queries the open issue with '.[0].number // empty', so no open issue gives an empty string rather than 'null'"
  - "CONFORMANCE's full-set size is now about 168 MB (measured 167,549,651 B of N65V), and its sparse-checkout figure says 1,081,529,097 B on disk, about 193 MB transferred"
  - "The 1,081,529,097 total is fixed in pins_check.cmake; the README says a pin bump replaces it with the total WRITE_PINS prints"

patterns-established:
  - "Each full-file test passes on its script's exit status and the exact line '65x02/<xx>: 0 of 10000 vectors failed'; no regex property, no skip code"

requirements-completed: [CPU-02]

coverage:
  - id: D1
    description: "The full 65x02 set, fetched by git at the pin and checked against pins.txt, matches the CPU on all 2,560,000 tests (256 cpu.vectors-full.<xx> tests)"
    requirement: CPU-02
    verification:
      - kind: integration
        ref: "cmake --workflow --preset vectors-full (258 of 258, cold from an empty build dir, 105 s)"
        status: pass
    human_judgment: false
  - id: D2
    description: "The first 100 converted tests of every fetched file equal the committed sample byte for byte (provenance proof)"
    requirement: CPU-02
    verification:
      - kind: integration
        ref: "ctest --preset vectors-full -R sample-match; a scratch blob with one byte changed failed 'chunk 84 differs at blob offset 896032'"
        status: pass
    human_judgment: false
  - id: D3
    description: "A missing, altered or mismatched file re-fetches or fails with the path and both hashes; no vectors-full test can skip"
    requirement: CPU-02
    verification:
      - kind: integration
        ref: "one byte of a9.json changed -> fetch re-fetched and passed (97 s); a pins copy with a9's hash zeroed -> exit 1 naming nes6502/v1/a9.json with expected and actual sha256"
        status: pass
      - kind: other
        ref: "grep -c 'SKIP_RETURN_CODE|RETURN_CODE 77|EXIT 77' over the four files: 0 each; --show-only=json-v1 has 0 PASS_REGULAR_EXPRESSION"
        status: pass
    human_judgment: false
  - id: D4
    description: "Offline ci check of pins.txt: format, 256 lines in order, size sum, commit equal to the sample's manifest pin"
    requirement: CPU-02
    verification:
      - kind: integration
        ref: "ctest --preset ci -R '^vectors\\.pins' (2 of 2); scratch copies with a wrong size and a wrong commit each failed with the exact reason"
        status: pass
    human_judgment: false
  - id: D5
    description: "nightly.yml defines the scheduled full run, pinned, with no cache or secrets"
    requirement: CPU-02
    verification:
      - kind: other
        ref: "actionlint .github/workflows/nightly.yml (clean); cmake --workflow --preset hygiene (6 of 6, hygiene.action_pins)"
        status: pass
    human_judgment: false
  - id: D6
    description: "The report job opens, edits and closes the rolling nightly issue on scheduled runs"
    requirement: CPU-02
    verification: []
    human_judgment: true
    rationale: "It runs only on schedule, so it is first exercised on GitHub after the merge; the key-parsing pipeline was checked locally on a sample LastTestsFailed.log"
  - id: D7
    description: "ENGINEERING section 5 and CONFORMANCE corrected; README describes the lane, the nightly, NESTURBATOR_VECTORS_DIR and a pin bump"
    verification:
      - kind: other
        ref: "grep counts: 'keyed by upstream commit' 0; '54,433|one per line' 0; '1,678,114' 1; 'vectors-full' in README 9"
        status: pass
    human_judgment: false

duration: 11min
completed: 2026-10-03
status: complete
---

# Phase 2 Plan 07: Full 65x02 vector set at its pin, locally and nightly Summary

**`cmake --workflow --preset vectors-full` fetches upstream `nes6502/v1` at `2f6980a2` by sparse, blobless, depth-1 git, checks all 256 files against `tests/vectors/pins.txt`, and passes all 2,560,000 tests plus a byte-for-byte proof that the committed sample is their first 100 per opcode. `nightly.yml` runs it every night with no cache and keeps one rolling issue on failure.**

## Performance

- **Duration:** about 11 min
- **Started:** 2026-10-03T15:50:51Z
- **Completed:** 2026-10-03T16:02:00Z
- **Tasks:** 3
- **Files modified:** 11

## Accomplishments

- `fetch_vectors.cmake` refuses a DIR inside the source tree outside `build/`. It does nothing when all 256 files verify; otherwise it replaces `DIR/src` with `git init`, `remote add`, `sparse-checkout set --cone nes6502/v1`, `fetch --depth 1 --filter=blob:none`, `checkout FETCH_HEAD`, and requires HEAD to equal the pin. It then removes `.git` and checks every file's size and SHA-256. A missing, extra or altered file fails with its path and both hashes.
- `pins.txt` was generated by WRITE_PINS at the pin. It has 257 lines and the sizes sum to exactly 1,081,529,097 B, matching the git tree figure in 02-RESEARCH.
- `cpu.vectors-full.00` to `.ff` each convert one whole file (vecconv without `--first`, so exactly 10000 tests) and require `65x02/<xx>: 0 of 10000 vectors failed`. All 256 pass.
- `cpu.vectors-full.sample-match` shows that the 256 `--first 100` chunks join to the committed 1,678,114 B sample exactly.
- `vectors.pins` and `vectors.pins.selftest` run offline in `ci`. The check catches swapped lines (`not sorted`), a wrong size sum and a wrong commit.
- `nightly.yml`: cron `17 4 * * *`, dispatch, and pull requests on its own paths; `permissions: {}`; checkout at the ci.yml SHA with `persist-credentials: false`; a `Failed keys` step that maps `LastTestsFailed.log` to `65x02/<xx>`, `fetch` and `sample-match`; a `report` job limited to `issues: write` on scheduled runs.
- **Local wall time of the full `vectors-full` workflow:** 105 s cold from an empty build directory (configure, build, about 97 s of fetch, then 258 tests), and 18 s warm with nothing to fetch. The 256 full-file tests take about 0.03 s each on this machine.

## Task Commits

1. **Task 1 (tracer): fetch at the pin, pins.txt, a9 end to end, presets**: `fc52a22` (feat)
2. **Task 2: all 256 full files, sample-match, offline pins check**: `d5a1fc1` (feat)
3. **Task 3: nightly.yml, ENGINEERING and CONFORMANCE revisions, README**: `774116f` (ci)

## Files Created/Modified

- `tests/cmake/fetch_vectors.cmake`: test-time fetch and verification; WRITE_PINS mode
- `tests/cmake/vectors_full_run.cmake`: one full file through vecconv and cpu.vectors
- `tests/cmake/vectors_sample_match.cmake`: first 100 per file against the committed blob
- `tests/cmake/pins_check.cmake`: the offline pins check and its self-test
- `tests/vectors/pins.txt`: pin line plus 256 hash and size lines
- `tests/CMakeLists.txt`: `vectors.pins*` (always) and the `vectors-full` block (option ON only)
- `CMakePresets.json`: `vectors-full` configure, build, test and workflow presets
- `.github/workflows/nightly.yml`: nightly run and rolling issue
- `.planning/preparation/ENGINEERING.md`: section 5 "Caches" (D-21)
- `.planning/preparation/CONFORMANCE.md`: "The 65x02 vectors" corrections (D-02, D-04)
- `README.md`: Building, Checks table, test descriptions, Continuous integration, pin bump

## Decisions Made

- `GIT_TERMINAL_PROMPT=0` is set with the two low-speed variables, so the fetch can never wait for input.
- The open-issue query appends `// empty`, so a repository with no open nightly issue gives an empty string, not `null`.
- The CONFORMANCE corrections carried over from 02-01 and 02-03 are in: "a JSON array; 25 files use a compact layout" (not "one per line"), 54,565 B over all 256 files (not 54,433), the 8-byte chunk header, 65.55 B per test, and 1,678,114 B for the sample (not D-04's 1,677,602 B). The full-set figures were updated from measurement: about 168 MB of N65V (167,549,651 B), and 1,081,529,097 B on disk from about 193 MB transferred.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 2 - Docs with behaviour] README updated in every task**
- **Found during:** Tasks 1 and 2
- **Issue:** The plan puts the README in Task 3, but CLAUDE.md rule 6 requires docs in the same change as the behaviour.
- **Fix:** Task 1 added the lane to Building and Checks and described the fetch. Task 2 described the 256 tests, sample-match and `vectors.pins`. Task 3 added the nightly, `NESTURBATOR_VECTORS_DIR` and the pin bump.
- **Committed in:** fc52a22, d5a1fc1, 774116f

**2. [Rule 2 - Robustness] `GIT_TERMINAL_PROMPT=0` and `// empty` in the issue query**
- **Found during:** Tasks 1 and 3
- **Issue:** An unattended git could block on a prompt, and `gh -q '.[0].number'` on an empty list can print `null`, which would then be treated as an issue number.
- **Fix:** As described under Decisions.
- **Committed in:** fc52a22, 774116f

**3. [Rule 1 - Accuracy] CONFORMANCE full-set numbers updated with the planned corrections**
- **Found during:** Task 3
- **Issue:** Once the per-test size became 65.55 B, the old "162 MB for the full set" and "fetches 1.08 GB" no longer held.
- **Fix:** Replaced them with the measured 167,549,651 B (about 168 MB) and "1,081,529,097 B on disk, about 193 MB transferred".
- **Committed in:** 774116f

---

**Total deviations:** 3 auto-fixed (2 rule 2, 1 rule 1). **Impact:** no change in scope.

## Issues Encountered

None. The full set matched on the first run.

## Verification

- `cmake --workflow --preset vectors-full`: 258 of 258, both warm and cold. `wc -l < tests/vectors/pins.txt` gives 257.
- A second run prints `256 files verify at 2f6980a2...; nothing fetched`. `ctest --preset ci -N | grep -c vectors-full` gives 0.
- With one byte of `a9.json` changed, `ctest --preset vectors-full -R fetch` reported `1 of 256 files do not verify`, fetched again and passed.
- `ctest --preset vectors-full -N | grep -c 'cpu.vectors-full\.'` gives 258. The json-v1 PASS_REGULAR_EXPRESSION count is 0, and the skip-code grep gives 0 for each of the four files.
- `ctest --preset ci -R '^vectors\.pins'`: 2 of 2.
- `actionlint` is clean. In `nightly.yml`, the `actions/cache|secrets.` count is 0 and the cron count is 1.
- Workflows: `ci` 0 (317 tests, `vectors.pins` included), `asan` 0 (316), `nofp` 0 (5), `hygiene` 0 (6, `hygiene.action_pins` included).
- Tracer gate (interactive, end-of-phase, automated verify only): after fc52a22 the `vectors-full` workflow ran again and passed before Task 2 started.

## User Setup Required

None.

## Next Phase Readiness

- Plan 08 pushes the branch and opens the pull request. Because the PR touches `nightly.yml` and `tests/vectors/**`, the nightly runs on it. Plan 08 then replaces `timeout-minutes: 30` with twice the measured cold run on GitHub.
- The `report` job is first exercised by a scheduled run after the merge (backstop).

---
*Phase: 02-the-cpu-matches-the-public-vectors*
*Completed: 2026-10-03*

## Self-Check: PASSED
