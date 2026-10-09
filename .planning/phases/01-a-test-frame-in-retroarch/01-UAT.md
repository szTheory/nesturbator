---
status: testing
phase: 01-a-test-frame-in-retroarch
source: [01-VERIFICATION.md]
started: 2026-10-09T11:47:58Z
updated: 2026-10-09T11:47:58Z
---

## Current Test

number: 2
name: Review clean-room provenance of the palette generator and table
expected: |
  Confirm tools/palgen/palgen.c and src/palette_ntsc.c use independently structured
  implementations based on cited hardware facts, without copied or paraphrased
  restricted decoder or emulator code.
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
expected: Confirm pull-request-controlled code cannot access the release App private key or token; release.yml mints it only for protected release events and passes it only to release-please and auto-merge.
result: [pending]

### 4. Verify docs-only and chore-only merges do not publish a release
expected: Confirm the release policy creates no release PR or release for docs/chore-only changes, while a behavior-changing release produces 18 archives plus SHA256SUMS with verified attestations.
result: [pending]

## Summary

total: 4
passed: 1
issues: 0
pending: 3
skipped: 0
blocked: 0

## Gaps

[none]
