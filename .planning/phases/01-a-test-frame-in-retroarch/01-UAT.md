---
status: testing
phase: 01-a-test-frame-in-retroarch
source: [01-VERIFICATION.md]
started: 2026-10-03T01:45:00Z
updated: 2026-10-03T01:45:00Z
---

## Current Test

number: 1
name: Merge draft PR #1 into main (owner consent; the only manual action)
expected: |
  The Release workflow starts on the push to main. After that, these commands
  prove the release half of criterion 5 and the install half of criterion 3,
  with no further owner action:
  `gh release view v0.1.0` (18 zips plus SHA256SUMS, published),
  `shasum -c SHA256SUMS` on the downloaded files, the README install URL
  returning 200, the README install line extracting into a scratch directory,
  and `ctest --preset ci -L retroarch`.
awaiting: user response

## Tests

### 1. Merge draft PR #1 into main (owner consent; the only manual action)
expected: The Release workflow starts; the automated post-merge checks in 01-VERIFICATION.md `behavior_unverified_items` pass.
result: [pending]

## Summary

total: 1
passed: 0
issues: 0
pending: 1
skipped: 0
blocked: 0

## Gaps
