---
phase: 01-a-test-frame-in-retroarch
plan: 10
subsystem: infra
tags: [github-actions, ci, release-please, attestation, rulesets, dependabot]

requires:
  - phase: 01-a-test-frame-in-retroarch (plans 01-09)
    provides: the ci, ci-msvc, asan, nofp and hygiene presets, check_archives.cmake, action_pins.cmake, the runner's hash output
provides:
  - ci.yml with hygiene, title, build (6 platforms), asan, nofp (2), hash-equality and the CI required roll-up
  - release.yml with release-please on a GitHub App token, CI on the release commit and an attested publish
  - .github/rulesets/main.json and .github/dependabot.yml
  - release-please-config.json and .release-please-manifest.json
  - tests/cmake/write_hashes.cmake and its runner.write_hashes tests
  - SECURITY.md, CONTRIBUTING.md, README Continuous integration and Releases sections
affects: [01-11, 01-12, phase 2 first task (release-as removal)]

actuals:
  tokens: 6900
  tasks: 2
  commits: 2
plan_head_before: df43a1ac3e594645b631de1b325778484aa3c5e2
plan_head_after: 0b40f7dcab20aa01edfd069b299469ebc1736710

tech-stack:
  added: [actions/checkout v7.0.1, actions/upload-artifact v7.0.1, actions/download-artifact v8.0.1, googleapis/release-please-action v5.0.0, actions/create-github-app-token v3.2.0, actions/attest v4.2.2]
  patterns:
    - "Every CI job runs the same cmake --workflow --preset command as local use"
    - "One required check (CI required) rolls up toJSON(needs) with jq"
    - "Every uses: pinned to a 40-hex SHA with a trailing # vX.Y.Z comment"

key-files:
  created:
    - .github/workflows/ci.yml
    - .github/workflows/release.yml
    - .github/dependabot.yml
    - .github/rulesets/main.json
    - tests/cmake/write_hashes.cmake
    - tests/runner/hashes.txt
    - release-please-config.json
    - .release-please-manifest.json
    - SECURITY.md
    - CONTRIBUTING.md
  modified:
    - tests/CMakeLists.txt
    - README.md
    - .planning/STATE.md

key-decisions:
  - "01-10: ci.yml's concurrency group includes github.workflow, so release.yml's call to ci.yml on the release commit never queues behind or is replaced by the push run on the same SHA"
  - "01-10: create-github-app-token gets vars.NESTURBATOR_APP_ID through client-id (app-id is deprecated at v3.2.0; both feed the same JWT issuer, which GitHub accepts as Client ID or App ID)"
  - "01-10: write_hashes.cmake is tested in the ci lane (runner.write_hashes + .content against tests/runner/hashes.txt), so the CI hash file format is checked locally"
  - "01-10: all six action SHAs re-resolved via gh api on 2026-10-02; each equals the RESEARCH baseline (no newer patch in the same major)"

patterns-established:
  - "Workflow step text names tokens and secrets only via env:, never inline in run:"

requirements-completed: [FRAME-01, FRAME-07]

coverage:
  - id: D1
    description: "ci.yml runs the preset lanes on six platforms behind the single CI required check, with hash equality across platforms"
    requirement: FRAME-01
    verification:
      - kind: other
        ref: "actionlint .github/workflows/ci.yml .github/workflows/release.yml"
        status: pass
      - kind: integration
        ref: "cmake --workflow --preset hygiene (hygiene.action_pins over the real workflows)"
        status: pass
    human_judgment: true
    rationale: "The workflows are linted locally, but they first run on GitHub after plan 11 creates the repository; the six-runner behaviour (images, MSVC environment, arm64 Ninja) is only shown by the first real run."
  - id: D2
    description: "write_hashes.cmake writes CR-free hashes.txt equal to the runner.advance lines"
    requirement: FRAME-01
    verification:
      - kind: integration
        ref: "tests/CMakeLists.txt#runner.write_hashes and runner.write_hashes.content (cmake --workflow --preset ci)"
        status: pass
    human_judgment: false
  - id: D3
    description: "release.yml publishes 18 attested archives with SHA256SUMS in the required order, behind CI on the release commit"
    requirement: FRAME-07
    verification:
      - kind: other
        ref: "actionlint .github/workflows/release.yml; grep -n ordering of subject-checksums < gh release upload < gh attestation verify < tampered < --draft=false"
        status: pass
    human_judgment: true
    rationale: "Release flow needs the remote, the App credential and the ruleset (plans 11 and 12); the first release settles A11, A12 and the ENGINEERING open questions."
  - id: D4
    description: "SECURITY.md, CONTRIBUTING.md, README CI and Releases sections, STATE.md release-as todo"
    requirement: FRAME-07
    verification:
      - kind: other
        ref: "grep -cE email-regex SECURITY.md == 0; grep -c release-as .planning/STATE.md >= 1; hygiene.tree"
        status: pass
    human_judgment: false

duration: 6min
completed: 2026-10-02
status: complete
---

# Phase 1 Plan 10: CI and release pipeline Summary

**Six-platform GitHub Actions CI that runs the local preset commands behind one `CI required` check, plus a release-please flow on a GitHub App token that publishes 18 attested archives and SHA256SUMS only after CI passes on the release commit and a tampered copy fails verification**

## Performance

- **Duration:** about 6 min
- **Started:** 2026-10-02T21:37:49Z
- **Completed:** 2026-10-02T21:43:45Z
- **Tasks:** 2
- **Files modified:** 13

## Accomplishments

- `ci.yml`: `hygiene`, `title` (live PR title through `gh pr view --repo`), `build` on ubuntu-24.04, ubuntu-24.04-arm, macos-15, macos-15-intel, windows-2025 and windows-11-arm, `asan` (Clang 18), `nofp` (GCC 14, x64 and arm64), `hash-equality` (exactly six byte-identical `hashes.txt`) and `CI required` (`if: always()`, jq over `toJSON(needs)`). No path filters, no `pull_request_target`, cancellation only for pull requests. Both Linux arm64 jobs install Ninja when it is missing.
- `release.yml`: mints the App token from `vars.NESTURBATOR_APP_ID` and `secrets.NESTURBATOR_APP_PRIVATE_KEY`, runs release-please, auto-merges the release PR, calls `ci.yml` when `release_created`, then publishes in the fixed order: download, exactly 18 names, `SHA256SUMS`, attest, upload to the draft, verify each zip, require a byte-flipped copy to fail, un-draft. Concurrency group `release`, never cancelled.
- `release-please-config.json` (simple, pre-major bump flags, draft + force-tag-creation, one-time `release-as` 0.1.0, `include-v-in-tag`, generic updater on the header, README and `.info`) and the manifest at 0.0.0. Every key checked against release-please v17.6.0's schema.
- `rulesets/main.json` (deletion, non-fast-forward, linear history, squash only with 0 approvals, `CI required` from integration 15368, no bypass) and `dependabot.yml` (github-actions, weekly, prefix `ci`).
- `write_hashes.cmake`, with ctest `runner.write_hashes` and `runner.write_hashes.content`.
- SECURITY.md (private vulnerability reporting, no address), CONTRIBUTING.md, README "Continuous integration" and "Releases" sections.
- **STATE.md Pending Todo:** Phase 2's first task, after v0.1.0 is published, is to remove the one-time `release-as` pin with `jq 'del(.packages["."]["release-as"])' release-please-config.json > tmp && mv tmp release-please-config.json` and confirm it with `jq -e '.packages["."] | has("release-as") | not' release-please-config.json` (both commands tested on a scratch copy).

## Task Commits

1. **Task 1: ci.yml, dependabot, ruleset, write_hashes, README CI section** - `924a099` (ci)
2. **Task 2: release.yml, release-please config, SECURITY.md, CONTRIBUTING.md, README Releases, STATE.md todo** - `0b40f7d` (ci)

## Files Created/Modified

- `.github/workflows/ci.yml` - six-platform CI with the CI required roll-up
- `.github/workflows/release.yml` - release-please, CI on the release commit, attested publish
- `.github/dependabot.yml` - weekly action updates
- `.github/rulesets/main.json` - branch rules as code (applied by plan 12)
- `tests/cmake/write_hashes.cmake` - CR-free runner hashes for hash equality
- `tests/runner/hashes.txt` - expected file for `runner.write_hashes.content`
- `tests/CMakeLists.txt` - registers the two write_hashes tests
- `release-please-config.json`, `.release-please-manifest.json` - release configuration
- `SECURITY.md`, `CONTRIBUTING.md` - legal and contribution files (ENGINEERING section 7)
- `README.md` - Continuous integration and Releases sections, write_hashes tests
- `.planning/STATE.md` - the release-as removal todo

## Decisions Made

- I re-resolved all six action SHAs with `gh api`, dereferencing annotated tags. Each matches the RESEARCH baseline. I read the inputs and outputs from `action.yml` at the pinned SHA. release-please declares no outputs there, so I took `release_created`, `tag_name`, `prs_created` and `pr` (a JSON object with `number`) from its `src/index.ts`.
- `client-id` rather than the deprecated `app-id` (see key-decisions). Plan 11 can store either the App's Client ID or its App ID in `NESTURBATOR_APP_ID`.
- In the publish job, `VERSION` is computed from `TAG` in the first script step (`${TAG#v}`) and exported through `GITHUB_ENV`, because Actions expressions have no prefix-strip function.
- `CI required` passes `toJSON(needs)` through an `env:` variable instead of a single-quoted `echo`, so no quote in the JSON can break the shell.
- Every checkout uses `persist-credentials: false`.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] ci.yml concurrency group would make the release's CI queue behind (or be replaced by) the push run on the same commit**
- **Found during:** Task 1
- **Issue:** A called workflow sees the caller's context. With group `ci-<pr number || sha>`, two runs share one group: `release.yml`'s call to `ci.yml` on the release commit and the push-triggered CI on that same commit. With cancel-in-progress false, the second run waits. A third run on that SHA, such as a manual dispatch, would replace the pending release CI and skip the publish.
- **Fix:** The group is `ci-${{ github.workflow }}-${{ github.event.pull_request.number || github.sha }}`. Pull requests still supersede their own runs, and only for pull_request events.
- **Files modified:** .github/workflows/ci.yml
- **Committed in:** 924a099

**2. [Rule 2 - Missing critical] Test for write_hashes.cmake (CLAUDE.md rule 6)**
- **Found during:** Task 1
- **Issue:** The plan added a CI script with no test in the `ci` lane.
- **Fix:** `runner.write_hashes` writes the file and `runner.write_hashes.content` compares it byte for byte with `tests/runner/hashes.txt`, the `runner.advance` lines. The README describes both.
- **Files modified:** tests/CMakeLists.txt, tests/runner/hashes.txt, README.md
- **Committed in:** 924a099

**3. [Rule 1 - Bug] shellcheck findings in release.yml**
- **Found during:** Task 2 (actionlint runs shellcheck)
- **Issue:** `ls` parsing, an unguarded `*.zip` glob and a variable in a `printf` format.
- **Fix:** A `dotglob nullglob` array, `sha256sum -- *.zip`, and `printf '%b' "\\0NNN"` for the byte flip. Locally, I ran the flip on bytes 0, 7, 92, 200 and 255, and the 18-name check on a complete set and on a set with one archive missing and one extra file.
- **Files modified:** .github/workflows/release.yml
- **Committed in:** 0b40f7d

---

**Total deviations:** 3 auto-fixed (2 bugs, 1 missing test)
**Impact on plan:** None changes scope. Each makes the pipeline more correct or brings it in line with CLAUDE.md.

## Issues Encountered

None.

## User Setup Required

None in this plan. Plan 11's owner checkpoint creates the repository, the GitHub App (`NESTURBATOR_APP_ID` variable, `NESTURBATOR_APP_PRIVATE_KEY` secret), auto-merge, squash-only with `PR_TITLE` titles, private vulnerability reporting and immutable releases. Plan 12 applies `.github/rulesets/main.json`.

## Verification

- `actionlint .github/workflows/ci.yml .github/workflows/release.yml`: clean, with shellcheck.
- `cmake --workflow --preset ci`: 30/30 passed (`retroarch.testframe` skipped, RetroArch not installed). `asan`: 29/29. `nofp`: 5/5. `hygiene`: 5/5 with clang-format 18 (llvm@18), including `hygiene.action_pins` over the real workflows (7 `uses:` keys in ci.yml).
- `cmake -DBUILD=build/ci -DOUT=build/ci/hashes.txt -P tests/cmake/write_hashes.cmake` writes exactly the frame 1 and frame 3 lines with hash b49e9be4...0453 and no `\r`.
- All acceptance criteria of both tasks were run and pass: 6 runner labels, 2 `command -v ninja`, ruleset jq index, release.yml step order (lines 105 < 107 < 111 < 117 < 132), one `cancel-in-progress: false`, no `pull_request_target`, no private key in ci.yml, no address in SECURITY.md.

## Next Phase Readiness

- Ready for plan 01-11 (owner checkpoint: repository, App, settings, account handle in the README install line).
- Carried assumptions: A11 (generic updater on Markdown and `.info` markers), A12 (auto-merge waits on the ruleset check), whether an App-token auto-merge starts `release.yml` (the fallback is `gh workflow run release.yml`, which is available through `workflow_dispatch`), and draft with force-tag-creation in release-please 17.6.0. The first release settles all four.

## Self-Check: PASSED

- All 10 created files exist; commits 924a099 and 0b40f7d exist and both end with the Co-Authored-By trailer.

---
*Phase: 01-a-test-frame-in-retroarch*
*Completed: 2026-10-02*
