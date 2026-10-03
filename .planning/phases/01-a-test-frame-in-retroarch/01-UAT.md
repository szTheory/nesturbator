---
status: complete
phase: 01-a-test-frame-in-retroarch
source: [01-VERIFICATION.md]
started: 2026-10-03T01:45:00Z
updated: 2026-10-03T13:20:27Z
---

## Current Test

[testing complete]

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

## Summary

total: 1
passed: 1
issues: 0
pending: 0
skipped: 0
blocked: 0

## Gaps

[none]
