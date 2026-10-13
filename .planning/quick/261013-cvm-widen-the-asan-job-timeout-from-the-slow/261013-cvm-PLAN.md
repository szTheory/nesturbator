---
phase: quick-261013-cvm
plan: 01
type: execute
wave: 1
depends_on: []
files_modified:
  - .github/workflows/ci.yml
  - .github/workflows/nightly.yml
  - .github/workflows/release.yml
  - .planning/preparation/ENGINEERING.md
  - tests/cmake/timeout_comments.cmake
  - .planning/phases/15-speed-and-close-out/15-REVIEW-FIX.md
  - .planning/phases/15-speed-and-close-out/15-REVIEW-DISPOSITION.md
autonomous: true
requirements: [WR-04]

estimate:
  tokens: 60000
  raw_tokens: 60000
  tasks: 3
  confidence: low

must_haves:
  truths:
    - "Every timeout-minutes line in ci.yml, nightly.yml and release.yml is at least twice the slowest sampled successful run of that job, rounded up to whole minutes, and its comment names that run ID and the sample count"
    - "The asan job has timeout-minutes 26 (756 s in run 38341228969, n=27), so the 730 s test time in run 38398904814 fits"
    - "hygiene, title, hash-equality, ci-required, release-please, publish and the nightly report job have timeout-minutes of at least 5 (the network-bound floor from WR-04)"
    - "ENGINEERING section 5 states the slowest-of-many-samples rule and the 5-minute network-bound floor, matching the workflow comments"
    - "cmake --workflow --preset ci (including hygiene.timeout_comments) and scripts/hygiene.sh pass"
    - "The pushed branch gsd/phase-15-speed-and-close-out shows green hosted checks on PR #46, or the failing check is reported"
  artifacts:
    - path: .github/workflows/ci.yml
      provides: "asan 26, build 23, retroarch-e2e 12, perf 10, hygiene/title/hash-equality/ci-required 5"
    - path: .github/workflows/nightly.yml
      provides: "vectors-full 21, mmc3-oracle 7, state-fuzz 4, suite-flake 40, report 5"
    - path: .github/workflows/release.yml
      provides: "release-please 5, publish 5"
    - path: .planning/preparation/ENGINEERING.md
      provides: "Section 5 Timeouts bullet with sample rule and floor"
  key_links:
    - from: .github/workflows/*.yml timeout comments
      to: tests/cmake/timeout_comments.cmake
      via: "comment on or directly above each timeout-minutes line contains `run <9+ digits>` or `runs <9+ digits>`"
    - from: .github/workflows/nightly.yml vectors-full / mmc3-oracle
      to: tests/CMakeLists.txt fetch TIMEOUT (900 s / 300 s)
      via: "timeout = fetch TIMEOUT + twice the whole sampled job"
---

<objective>
Fix the PR #46 hosted CI failure (asan killed at its 12-minute timeout in run 38398904814 after all 628 tests passed in 730 s) and close review finding WR-04, by setting every workflow job timeout from the slowest of the sampled run history instead of a single sample, and by giving short network-bound jobs a 5-minute floor.

Purpose: a timeout kill of a required job blocks merging, and a kill of release-please or publish can leave a release half made. Hosted run time is bimodal (asan ~325 s or 570-756 s), so one fast sample is not a measurement of the tail.
Output: edited workflow timeouts and comments, ENGINEERING section 5 rule text, lint wording, WR-04 recorded as fixed, branch pushed, hosted CI green on PR #46.

Stay on branch gsd/phase-15-speed-and-close-out. Do not create a branch.
</objective>

<execution_context>
@~/.claude/gsd-core/workflows/execute-plan.md
@~/.claude/gsd-core/templates/summary.md
</execution_context>

<context>
@.planning/STATE.md
@CLAUDE.md
@.planning/phases/15-speed-and-close-out/15-REVIEW.md
@.github/workflows/ci.yml
@.github/workflows/nightly.yml
@.github/workflows/release.yml
@tests/cmake/timeout_comments.cmake

Lint contract (tests/cmake/timeout_comments.cmake, test `hygiene.timeout_comments`): a `timeout-minutes:` line passes when its trailing `#` comment, or the contiguous YAML comment lines directly above it, contain `run <9 or more digits>` or `runs <9 or more digits>`. Any non-comment line between the comment block and `timeout-minutes:` breaks the association. Use the above-line comment form already used for asan, perf and the nightly jobs.

Rule text lives at .planning/preparation/ENGINEERING.md line 84 (section 5, "**Timeouts** are twice a measured cold duration ...") and line 86 (suite-flake "timeout twice a measured cold run").

The fetch lanes follow tests/CMakeLists.txt line 1726: "The nightly's timeout-minutes is this TIMEOUT plus twice the rest of the lane's measured time". cpu.vectors-full.fetch TIMEOUT is 900 s; mmc3.oracle.fetch TIMEOUT is 300 s. The new samples are whole-job times, so twice the whole job is used for "the rest" (conservative; no fetch subtraction).

Sample source for every comment: the last 40 completed runs of each workflow, successful jobs only, started_at to completed_at, collected 2026-10-13 with `gh api repos/{owner}/{repo}/actions/runs/{id}/jobs`.

Target table (current -> new; 2x = twice the slowest sample, rounded up to whole minutes; "floor" = 5-minute network-bound floor per WR-04). Never lower a value.

ci.yml
| job | n | slowest | run | 2x | current | new | reason |
| hygiene | 34 | 58 s | 38288411183 | 2 | 1 | 5 | floor (checkout + scans) |
| title | 37 | 8 s | 38345355652 | 1 | 1 | 5 | floor (gh/API) |
| build (matrix) | 34/30/33/34/35/34 | 679 s macos-15-intel | 38345355652 | 23 | 22 | 23 | 2x |
| asan | 27 | 756 s | 38341228969 | 26 | 12 | 26 | 2x; also cite cancelled run 38398904814 (~743 s, tests done at 730 s) |
| nofp (matrix) | 35/35 | 43 s ubuntu-24.04 | 38342366027 | 2 | 2 | 2 | unchanged value, comment refreshed |
| hash-equality | 28 | 9 s | 38398904814 | 1 | 1 | 5 | floor (artifact download) |
| retroarch-e2e | 33 | 346 s | 38359109704 | 12 | 11 | 12 | 2x |
| perf | 13 | 286 s | 38398904814 | 10 | 9 | 10 | 2x |
| ci-required | 23 | 4 s | 38359109704 | 1 | 1 | 5 | floor (rollup of needs) |

nightly.yml
| vectors-full | 36 | 166 s | 38341228998 | 900 + 332 = 1232 s | 19 | 21 | fetch TIMEOUT + 2x whole job |
| mmc3-oracle | 39 | 41 s | 38398904800 | 300 + 82 = 382 s | 6 | 7 | fetch TIMEOUT + 2x whole job |
| rom-loader-fuzz | 40 | 85 s | 38347599457 | 3 | 3 | 3 | unchanged value, comment refreshed |
| state-fuzz | 37 | 99 s | 38289608338 | 4 | 3 | 4 | 2x |
| suite-flake | 39 | 1173 s | 38324782531 | 40 | 38 | 40 | 2x |
| report | 3 | 5 s | 38329002573 | 1 | 1 | 5 | floor (gh only) |

release.yml
| release-please | 39 | 24 s | 38306108774 | 1 | 1 | 5 | floor (release API; a kill can half-create a release) |
| publish | 9 | 90 s | 38081193284 | 3 | 3 | 5 | floor (release job, uploads assets; WR-04 names release jobs) |

The release.yml `ci / *` jobs call ci.yml and carry no timeout of their own. The nightly vectors-full network failure in run 38397148750 was a git/curl "Operation too slow" stall at 62.5 s, not a timeout kill; it is not a timeout input.
</context>

<tasks>

<task type="tracer">
  <name>Task 1: Tracer - asan timeout from the sample history, rule text, lint green</name>
  <files>.github/workflows/ci.yml, .planning/preparation/ENGINEERING.md, tests/cmake/timeout_comments.cmake</files>
  <action>
Wire the one failing path end to end: workflow value, its comment, the rule it cites, and the lint that checks it.

1. In .github/workflows/ci.yml asan job (around line 132-134): keep the existing first comment line about the reduced suite; replace the "Twice the measured hosted run: 340 s in run 38346573845 ..." line with a comment that says the timeout is twice the slowest of 27 sampled successful runs, 756 s in run 38341228969, rounded up to 26 minutes, ENGINEERING section 5, and add a second comment line noting that run 38398904814 was killed at the old 12-minute limit after its tests passed in 730 s. Set `timeout-minutes: 26`. The comment lines must sit directly above the `timeout-minutes:` line with no non-comment line between.

2. In .planning/preparation/ENGINEERING.md section 5, rewrite the **Timeouts** bullet (line 84) so it states: a timeout is twice the slowest successful run of that job among the sampled runs (the last 40 completed runs of the workflow, or all of them when there are fewer), rounded up to whole minutes; the comment names the slowest run ID and the sample count; a lane with a network fetch adds the fetch test's TIMEOUT to twice the rest; a short network-bound job (checkout, `gh` or another network API, or a `needs` rollup: hygiene, title, hash-equality, CI required, release-please, publish, the nightly report) gets a 5-minute floor, because runner start-up and network tails dominate it and a kill blocks merging or leaves a release half made; the default stays 360 minutes; keep the [ENG.03][ENG.13] citations. Also change the suite-flake sentence on line 86 from "timeout twice a measured cold run" to "timeout twice the slowest sampled run". Plain short sentences, house style.

3. In tests/cmake/timeout_comments.cmake, change only the wording "twice a measured cold run" in the header comment and in the per-offender STATUS message to "twice the slowest sampled run". Do not change the regex or logic.
  </action>
  <verify>
    <automated>cmake -DROOT=.github/workflows -P tests/cmake/timeout_comments.cmake && grep -n 'timeout-minutes: 26' .github/workflows/ci.yml && grep -c '5-minute floor' .planning/preparation/ENGINEERING.md</automated>
  </verify>
  <done>asan is 26 minutes with a lint-passing comment naming run 38341228969 and n=27; ENGINEERING section 5 states the sampled-slowest rule and the 5-minute floor; the lint passes over .github/workflows.</done>
</task>

<task type="auto">
  <name>Task 2: Apply the table to every other job, record WR-04 fixed, run the suite, commit</name>
  <files>.github/workflows/ci.yml, .github/workflows/nightly.yml, .github/workflows/release.yml, .planning/phases/15-speed-and-close-out/15-REVIEW-FIX.md, .planning/phases/15-speed-and-close-out/15-REVIEW-DISPOSITION.md</files>
  <action>
1. For every remaining row of the target table in the context section, set the `timeout-minutes` value to the "new" column and rewrite its comment. Use one comment form: for 2x rows, "Twice the slowest of N sampled successful runs: S s in run ID, rounded up to M minutes, ENGINEERING section 5." For floor rows, "5-minute floor for a network-bound job, ENGINEERING section 5; twice the slowest of N sampled successful runs (S s in run ID) is M minutes." For the build and nofp matrix jobs, name the runner of the slowest sample and give the per-runner sample count range. For vectors-full and mmc3-oracle, keep the existing explanation that the fetch has its own allowance (900 s / 300 s TIMEOUT in tests/CMakeLists.txt) and replace the old three-run arithmetic with: twice the slowest whole job of N sampled runs, S s in run ID, so TIMEOUT + 2S seconds, rounded up to M minutes. For publish, drop the old two-line note about runs that skipped publish (the new sample set has 9 runs where it ran). Convert single-line trailing comments (hygiene, title, build, nofp, hash-equality, ci-required) to the above-line form so all three files read the same. Leave nofp and rom-loader-fuzz values unchanged; only their comments change. Do not lower any value.

2. Record WR-04 as fixed in .planning/phases/15-speed-and-close-out/15-REVIEW-FIX.md: move the WR-04 section from skipped into the fixed issues list with a short note (sampled last 40 runs per workflow, every timeout now twice the slowest sample with run ID and count, 5-minute floor for network-bound jobs, rule written into ENGINEERING section 5), and update the frontmatter and summary counts (fixed 4, skipped 0, status all_fixed). In 15-REVIEW-DISPOSITION.md set WR-04 `disposition: fixed` in the frontmatter and the table. Keep `open: 3` (IN-01 to IN-03 are unchanged).

3. Run `cmake --workflow --preset ci` and `scripts/hygiene.sh`. Both must pass. If a test fails, fix the cause (most likely a comment block separated from its `timeout-minutes:` line); do not weaken the lint.

4. Commit the workflow, ENGINEERING and lint changes as `ci: set job timeouts from sampled run history` with a body that names the asan kill in run 38398904814 and WR-04, then commit the two review files as `docs(15): record WR-04 fix`. End each message with the line `Co-Authored-By: Claude Opus 5.5 (noreply trailer)`. No personal paths in any file or message.
  </action>
  <verify>
    <automated>cmake --workflow --preset ci && scripts/hygiene.sh && git status --porcelain | wc -l</automated>
  </verify>
  <done>All 17 timeout lines match the table and carry run IDs and sample counts; the ci workflow preset and hygiene script pass; WR-04 is recorded fixed; two commits exist on gsd/phase-15-speed-and-close-out and the tree is clean.</done>
</task>

<task type="auto">
  <name>Task 3: Push and confirm hosted CI on PR #46</name>
  <files>none (git push only)</files>
  <action>
Push gsd/phase-15-speed-and-close-out to origin (plain push, no force). Then wait for the hosted run on PR #46: run `gh pr checks 46 --watch --interval 60` in the background (the run takes up to about 25 minutes, so a single foreground call would exceed the 10-minute tool limit), or poll `gh run list --branch gsd/phase-15-speed-and-close-out --limit 3` until the newest ci.yml run is completed. If every check passes, record the run ID. If any check fails, fetch `gh run view <id> --log-failed`, report the failing job and the cause, and stop; do not re-run or retry blindly. release.yml and nightly.yml do not run on a pull request, so their new values are checked only by hygiene.timeout_comments; say so in the summary.
  </action>
  <verify>
    <automated>gh pr checks 46</automated>
  </verify>
  <done>PR #46 checks are all green with the run ID recorded, or the failing check and its log cause are reported.</done>
</task>

</tasks>

<threat_model>
## Trust Boundaries

| Boundary | Description |
|----------|-------------|
| workflow -> hosted runner | timeout-minutes bounds how long a job may hold a runner and block a merge or release |

## STRIDE Threat Register

| Threat ID | Category | Component | Severity | Disposition | Mitigation Plan |
|-----------|----------|-----------|----------|-------------|-----------------|
| T-q261013-01 | Denial of Service | release.yml release-please / publish | medium | mitigate | 5-minute floor so a network tail does not kill a release mid-creation |
| T-q261013-02 | Denial of Service | ci.yml required jobs | medium | mitigate | timeouts from the slowest of many samples so a slow hosted runner does not block merging |
| T-q261013-03 | Information Disclosure | comments and commit messages | low | mitigate | scripts/hygiene.sh run before commit; no personal paths |
| T-q261013-SC | Tampering | package installs | low | accept | no package installs in this plan |
</threat_model>

<verification>
- `cmake -DROOT=.github/workflows -P tests/cmake/timeout_comments.cmake` reports 17 timeout-minutes lines carrying a run ID.
- `cmake --workflow --preset ci` passes; `scripts/hygiene.sh` exits 0.
- `gh pr checks 46` shows every check passing.
</verification>

<success_criteria>
- asan timeout 26 minutes; PR #46 hosted CI green.
- Every workflow timeout is at least twice its slowest sampled run, its comment names that run and the sample count, and network-bound jobs have a 5-minute floor.
- ENGINEERING section 5 states the same rule; WR-04 recorded fixed.
</success_criteria>

<output>
Create `.planning/quick/261013-cvm-widen-the-asan-job-timeout-from-the-slow/261013-cvm-SUMMARY.md` when done
</output>
