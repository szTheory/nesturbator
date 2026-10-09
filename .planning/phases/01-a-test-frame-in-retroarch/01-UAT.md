---
status: testing
phase: 01-a-test-frame-in-retroarch
source: [01-VERIFICATION.md]
started: 2026-10-09T11:47:58Z
updated: 2026-10-09T12:44:00Z
---

## Current Test

number: 2
name: Review clean-room provenance of the palette generator and table
expected: |
  Confirm tools/palgen/palgen.c and src/palette_ntsc.c use independently structured
  implementations based on cited hardware facts, without copied or paraphrased
  restricted decoder or emulator code. This remains the sole owner judgment:
  deterministic tests can check source/table consistency and citations, but no
  repository check can prove what material an author read or rule out an
  unrecorded paraphrase.
awaiting: user response

## Tests

### 1. Merge draft PR #1 into main (owner consent; the only manual action)
expected: The Release workflow starts; the automated post-merge checks in 01-VERIFICATION.md `behavior_unverified_items` pass.
result: pass
source: automated
evidence: |
  PR #1 merged 2026-10-03 (owner consent). Release PR #2 merged by
  app/nesturbator-release with no person acting; Release run 37125205144 succeeded.
  `gh release view v0.1.0`: isDraft false, 18 zips plus SHA256SUMS.
  `shasum -a 256 -c SHA256SUMS` on the downloaded release: exit 0, 18 lines OK.
  The README install URL for v0.1.0 returns HTTP 200. The README block, run with HOME
  set to a scratch directory, extracted cores/nesturbator_libretro.dylib
  (byte-identical to the release zip) and info/nesturbator_libretro.info.
  `cmake --workflow --preset ci` exit 0; `ctest --preset ci -L retroarch`:
  retroarch.testframe Passed.
  Note: PR #3 (fix: bump every version in the README install line) was needed
  before the release so that the line named v0.1.0 everywhere.

### 2. Review clean-room provenance of the palette generator and table
expected: Confirm tools/palgen/palgen.c and src/palette_ntsc.c use independently structured implementations based on cited hardware facts, without copied or paraphrased restricted decoder or emulator code.
result: [pending]

### 3. Review release credential isolation enforcement
expected: |
  Pull-request-controlled or manually selected branch code cannot access the
  release App key or token; release.yml runs only on pushes to main and passes
  the token only to release-please and auto-merge.
result: pass
source: automated
evidence: |
  `cmake --workflow --preset ci` passed `release.credentials_policy` and its
  mutation self-test. The test requires the sole event to be push-to-main, the
  private-key reference to appear once inside the release job, and exactly two
  App-token consumers. Its mutation self-test rejects workflow_dispatch and an
  extra private-key consumer. `actionlint .github/workflows/release.yml` exited
  0. Removed workflow_dispatch because GitHub allows dispatching against a
  selected branch or tag.

### 4. Verify docs-only and chore-only merges do not publish a release
expected: |
  Non-breaking docs-only and chore-only commits do not create a release; a
  behavior-changing release publishes 18 archives plus SHA256SUMS with
  verified attestations.
result: pass
source: automated
evidence: |
  `cmake --workflow --preset ci` passed `release.nonbehavioral_policy` and its
  mutation self-test. The check pins release-please-action v5.0.0 (bundled
  release-please 17.6.0), requires the `simple` strategy with no custom
  changelog sections, verifies that non-breaking docs/chore types are filtered,
  and that feature/fix/perf/revert and breaking commits remain releaseable.
  It also requires the publish job to depend on successful CI and a created
  release. The current hosted v0.1.4 release run succeeded with 18 archives
  plus SHA256SUMS (19 assets); the release workflow verifies attestations.

## Summary

total: 4
passed: 3
issues: 0
pending: 1
skipped: 0
blocked: 0

## Gaps

[none]
