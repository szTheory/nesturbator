---
phase: quick-261031-fq6
verified: 2026-10-31T15:37:09Z
status: human_needed
score: 5/5 must-haves verified
covered_files:
  - .github/workflows/ci.yml
  - .planning/quick/261031-fq6-skip-the-redundant-main-push-ci-run-for-/261031-fq6-PLAN.md
  - .planning/quick/261031-fq6-skip-the-redundant-main-push-ci-run-for-/261031-fq6-SUMMARY.md
  - README.md
  - docs/releases.md
  - docs/testing.md
  - tests/cmake/ci_release_push_policy.cmake
  - tests/hygiene/CMakeLists.txt
covered_digest: "v4:sha256:9deb93a83281cbcb4bd73a65481aa1c93656e39eb0efe624514be5153411eaec"
behavior_unverified: 0
overrides_applied: 0
decision_coverage:
  honored: 0
  total: 0
  not_honored: []
human_verification:
  - test: "After separate owner authorization, inspect a generated Release Please main-merge run in GitHub Actions."
    expected: "The direct CI jobs, including CI required, are skipped; Release runs reusable CI across six platforms, uploads archive and hash artifacts, and publishes only after its existing checks pass. Nightly remains triggered by the main push."
    why_human: "Offline CMake checks verify the source predicates and modeled cases but cannot execute GitHub's hosted workflow context, runner scheduling, artifact transfer, or attestation service."
---

# Quick Task 261031-fq6 Verification Report

**Phase Goal:** Skip the redundant main-push CI run for Release Please merge commits while preserving the reusable release CI and publication gate.
**Verified:** 2026-10-31T15:37:09Z
**Status:** human_needed
**Re-verification:** No — initial verification; no earlier VERIFICATION.md was present.

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|---|---|---|
| 1 | Ordinary CI jobs skip a protected-main push with the generated Release Please release commit title. | ✓ VERIFIED | All 11 job conditions extracted from the `jobs` section are pinned by `check_policy()` in `tests/cmake/ci_release_push_policy.cmake`; the direct guard requires push, `refs/heads/main`, workflow `CI`, and a title prefix of `chore(main): release `. `ci.yml:43` and the other per-job `if` clauses implement that predicate. GitHub documents that job conditions control whether a job runs and are evaluated before matrix expansion ([workflow syntax](https://docs.github.com/en/actions/reference/workflows-and-actions/workflow-syntax#jobsjob_idif)). Hosted execution remains unobserved; see Human Verification. |
| 2 | The Release caller runs reusable CI, six-platform builds, hash checks, and archive uploads on the release commit. | ✓ VERIFIED | `.github/workflows/release.yml:58` calls `./.github/workflows/ci.yml` when `release_created` is true and is unchanged from base `c1e9c239bd188f6538271dba8252687255437f15`. `ci.yml:20` retains `workflow_call`; `ci.yml:105` lists all six platforms; `ci.yml:180` and `ci.yml:185` upload archive and hash files; `ci.yml:310` checks cross-platform hashes. The guard permits the Release caller because its `github.workflow` is `Release`; GitHub confirms a called workflow receives its caller's `github` context ([reusable workflow context](https://docs.github.com/en/actions/reference/workflows-and-actions/reusing-workflow-configurations#github-context)). The static policy pins these contracts. |
| 3 | Release publication requires successful reusable CI and the existing asset and attestation checks. | ✓ VERIFIED | `.github/workflows/release.yml:67` gives `publish` both `release-please` and `ci` dependencies and preserves the `release_created` condition. GitHub's `needs` contract requires dependencies to complete successfully unless an override condition is used ([workflow dependencies](https://docs.github.com/en/actions/reference/workflows-and-actions/workflow-syntax#jobsjob_idneeds)). The unchanged publish job verifies the exact 18 archives, writes checksums, creates attestations, verifies each archive, and rejects a modified archive before publication; the policy script requires these controls at lines 162–173. |
| 4 | Ordinary commits, all pull requests including release PRs, and manual CI retain validation; main-push Nightly remains enabled. | ✓ VERIFIED | `ci.yml` triggers are unchanged from base; the OR guard evaluates true for PRs, manual runs, and pushes without the generated release prefix. The perf guard keeps its PR/manual-control eligibility. `.github/workflows/nightly.yml` is byte-for-byte unchanged and retains its `push: branches: [main]` trigger plus generated-PR-only lane conditions. Offline event cases cover normal pushes, PR, dispatch, and other refs. |
| 5 | Focused offline policy checks establish the routing contract without full CI, campaigns, pushes, or remote dispatch. | ✓ VERIFIED | The live policy and its self-test are registered in `tests/hygiene/CMakeLists.txt:62`; the self-test requires the exact supported expression before applying an independent event table and targeted mutations. The three selected CTests passed. No full workflow, campaign, remote dispatch, push, merge, or release action was run. |

**Score:** 5/5 truths verified (0 present, behavior-unverified). The overall status is `human_needed` because hosted GitHub execution and publication have not been observed.

### Required Artifacts

| Artifact | Expected | Status | Details |
|---|---|---|---|
| `.github/workflows/ci.yml` | Caller-aware job conditions for the ordinary release merge push | ✓ VERIFIED | Exists, substantive, and used as both a directly triggered workflow and the local reusable workflow. All 11 job-level conditions are checked from the `jobs` section by the live policy. |
| `tests/cmake/ci_release_push_policy.cmake` | Static live workflow checks, event controls, and mutation rejection | ✓ VERIFIED | 553-line implementation strips comments, extracts each job condition, checks release/nightly wiring, and has bounded event classification plus mutations for every planned regression. |
| `tests/hygiene/CMakeLists.txt` | Focused policy and self-test registered in hygiene | ✓ VERIFIED | Registers both CTests with the `hygiene` label and a 5-second timeout. Both tests ran and passed. |
| `docs/releases.md` | Durable explanation of release validation and publication ownership | ✓ VERIFIED | The new “Release commit CI ownership” section describes the exception, preserved Release/Nightly responsibilities, and local evidence limits. README and testing guide also summarize the behavior. |

### Key Link Verification

| From | To | Via | Status | Details |
|---|---|---|---|---|
| `.github/workflows/ci.yml` | `.github/workflows/release.yml` | `github.workflow` distinguishes direct CI from the reusable Release caller | ✓ WIRED | Canonical query verified the pattern; official GitHub documentation says a called workflow uses the caller's `github` context. |
| `tests/hygiene/CMakeLists.txt` | `tests/cmake/ci_release_push_policy.cmake` | `hygiene.ci_release_push_policy` and `.selftest` | ✓ WIRED | Both CTests invoke the script with `ROOT`; the selected CTest run executed both. |
| `.github/workflows/release.yml` | `.github/workflows/ci.yml` | Local reusable call and publish dependency | ✓ WIRED | Canonical query verified the call and dependency. Both workflow files are unchanged from the recorded base. |

### Data-Flow Trace (Level 4)

| Artifact | Data Variable | Source | Produces Real Data | Status |
|---|---|---|---|---|
| CI workflow policy | N/A | Workflow control configuration; no dynamic application values are rendered | N/A | N/A — data-flow tracing for rendered application values does not apply. |

### Behavioral Spot-Checks

| Behavior | Command | Result | Status |
|---|---|---|---|
| Required roll-up accepts skipped `perf` only where permitted and rejects failed or skipped required dependencies | `ctest --preset hygiene --no-tests=error -R '^hygiene\.(ci_release_push_policy(\.selftest)?|ci_required_rollup)$'` | `hygiene.ci_required_rollup` passed (0.12 s) | ✓ PASS |
| Live CI, Release, and Nightly source contracts hold | Same selected CTest command | `hygiene.ci_release_push_policy` passed (0.03 s) | ✓ PASS |
| Bounded event cases and routing/publication/Nightly mutations are rejected as expected | Same selected CTest command | `hygiene.ci_release_push_policy.selftest` passed (0.58 s) | ✓ PASS |

These are offline source-policy tests, not executions of GitHub Actions. The self-test covers the direct CI release push, the Release caller under the same push payload, ordinary main pushes, a non-prefix title, absent head metadata, matching-title PR and manual runs, another ref, and a squash title with multiline release notes. It separately tests perf eligibility and checks mutations that remove guards, relax the title match, damage the required roll-up, remove `workflow_call`, weaken Release publication wiring, alter the archive path, or skip Nightly main pushes. The classifier only accepts the exact normalized expression under test; it does not claim to evaluate arbitrary GitHub expressions.

### Probe Execution

Not applicable. The plan declares no probe, and no conventional `scripts/*/tests/probe-*.sh` file was found.

### Requirements Coverage

Not applicable. This quick-task plan declares no requirement IDs and does not map to a ROADMAP phase requirement.

### Test Quality Audit

| Test File | Linked Requirement | Active | Skipped | Circular | Assertion Level | Verdict |
|---|---|---:|---:|---|---|---|
| `tests/cmake/ci_release_push_policy.cmake` | Release-push routing and preserved workflow wiring | Yes | 0 detected | No — table expectations are fixed test inputs; the script checks the live workflow text before classifying cases | Behavioral model plus mutation rejection | Adequate for the bounded offline source contract; does not establish hosted GitHub execution |
| `tests/cmake/ci_required_rollup.cmake` | Required-check failure semantics | Yes | 0 detected | No — synthetic `needs` inputs are independently supplied to the extracted jq expression | Value/behavioral assertions | Adequate for the jq roll-up; the focused CTest passed |

No disabled-test patterns were found in the new policy test or its registration. No test runs workflow jobs, downloads artifacts, or invokes a campaign.

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|---|---:|---|---|---|
| None | — | No unreferenced `TBD`, `FIXME`, `XXX`, TODO/HACK/placeholder marker, or empty implementation found in the six changed implementation/documentation files | — | No blocker detected. |

### Decision Coverage

No `CONTEXT.md` exists for this quick task. The decision-coverage query returned `skipped: true` with “No CONTEXT.md - nothing to check.”

### Human Verification Required

#### 1. Observe hosted release routing after separate authorization

**Test:** After a separately authorized generated Release Please main-merge push, inspect the GitHub Actions runs and their artifacts.
**Expected:** The direct `CI` jobs, including `CI required`, are skipped; the `Release` reusable CI call runs all six build lanes and uploads archive/hash artifacts; publication occurs only after CI, exact archive, and attestation checks pass; Nightly also starts for the main push.
**Why human:** Offline CMake checks establish the checked source and its bounded classifier cases but cannot execute GitHub's hosted workflow context, allocate runners, transfer artifacts, or call the attestation service. No remote action was authorized or run for this verification.

### Gaps Summary

No implementation gap was found: the CI job guards, perf eligibility, required-check roll-up, reusable Release call, publication dependencies/checks, Nightly trigger, tests, and documentation are present and wired. Verification remains `human_needed` until a separately authorized hosted run confirms GitHub's actual execution and artifact/publication behavior; the local static evidence does not establish those outcomes.

### Base and Scope Checks

- `git diff --exit-code c1e9c239bd188f6538271dba8252687255437f15..HEAD -- .github/workflows/release.yml .github/workflows/nightly.yml include/nesturbator.h tests/accuracy/baseline-metadata.txt` exited 0.
- `git diff --check c1e9c239bd188f6538271dba8252687255437f15..HEAD` exited 0 with no output.
- The implementation diff contains the six planned code/documentation files; the pre-existing dirty `AGENTS.md`, `.planning/config.json`, and `.planning/continue.md` were left untouched.

---
_Verified: 2026-10-31T15:37:09Z_  
_Verifier: gsd-verifier_
