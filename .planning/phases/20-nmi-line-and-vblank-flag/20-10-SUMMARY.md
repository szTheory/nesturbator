---
phase: 20-nmi-line-and-vblank-flag
plan: "10"
subsystem: hosted-evidence
tags: [github-actions, conformance, determinism, performance, source-identity]
requires:
  - phase: 20-12
    provides: Fresh source-bound successful CI/Nightly availability
provides:
  - Validated real hosted bundle for candidate fe70d61 and tested merge ef6d9e5
  - Six identical complete inventories and per-lane 108-key protected PASS retention
  - Clean campaign, real performance, Nightly conformance and RetroArch confirmation
affects: [phase-20-verification, KEEP-02, KEEP-03]
actuals:
  tokens: 4200
  tasks: 2
  commits: 2
tech-stack:
  added: []
  patterns: [Immutable hosted bundle validated against equivalent evidence-only commits]
key-files:
  created:
    - .planning/phases/20-nmi-line-and-vblank-flag/20-10-SUMMARY.md
  modified:
    - .planning/phases/20-nmi-line-and-vblank-flag/20-EVIDENCE.md
    - tests/accuracy/measurements.md
key-decisions:
  - "Keep candidate, tested merge, protected base and evidence-record commits distinct."
  - "Collect from a clean equivalent checkout to avoid ignored local fixture paths; revalidate against the working checkout."
patterns-established:
  - "Use an absolute SOURCE_REF and a scratch GitHub CLI cache without changing effective source inputs."
requirements-completed: []
duration: 15min
completed: 2026-10-28
status: complete
---

# Phase 20 Plan 10: Real hosted content acceptance

**Real CI/Nightly artifact content passes the unchanged production validators for all required platform, protected-PASS, campaign, performance, conformance and host outcomes.**

## Accomplishments

- Live read-only precondition passed before collection/document mutation: exact successful CI `40700458574/1`, Nightly `40700458462/1`, PR #59 mapping, selected jobs, unexpired artifacts and 417 effective inputs.
- Candidate `fe70d619d6a3cf7b80ed1867ff9faf4c7c9053b1`, actual tested merge `ef6d9e5ec2c94ddfd431c1d7162f620722a145df`, protected base `ab9c2e6eb822c06f0dfb0f08a0ca42ec541358be` and current effective digest `a08da046cedeecbdd28227cc53d54e3d3e7dff2a93ee88aab9ead67da47bf61c` are bound by raw metadata and full content checks.
- Six 114-row native inventories are byte-identical; every lane freshly retains all 108 protected PASS keys. Clean full campaign: 114 PASS / 32 FAIL, six exact NMI targets passing, no skip/not-run/timeout, five DRAW.
- Actual base/head performance passes +1% with zero-spread controls: NROM +0.51%, MMC1 -10.10%, MMC3 -5.85%. Nightly vectors-full 258/258 and MMC3 13/13 pass. Real RetroArch progression reaches 16 boundaries, with all fault controls rejecting as intended.

## Task Commits

1. **Collect and record accepted source-bound hosted content** — `e50e825`.
2. **Publish clean hosted measurement confirmation** — `140fa5a`.

## Verification

- Production `MODE=collect` exited 0 with `hosted phase20 evidence verified` on a clean local clone of evidence-record base `445e7e5cb6c266721d355c5bb927c4be7cb5eff8`.
- The identical immutable bundle was copied to `build/phase20-hosted-evidence/`; production `MODE=validate` exited 0 against the current working checkout before Task 1 and again after its commit and the Task 2 measurement edit.
- Manifest SHA-256: `55857090681bef15f44671c8a0ef5a39fddda2c23b479b45d4802871e83a2386`; verdict SHA-256: `84867fe8bfc1292323e0f0cc88e9b906b1b66eb947dbd9290f934b1943c68f22`; 114-file evidence manifest SHA-256: `715e1b27b157ab117934bb3d4766ee9fd603e0b981eee442631e0a0a91130e54`.
- Full run/attempt/job/artifact IDs, raw ZIP digests, campaign provenance and performance measurements are recorded in `20-EVIDENCE.md` and `tests/accuracy/measurements.md`.
- Task 1 and Task 2 commits alter only the established evidence exclusions. The immutable hosted source identity does not move with those documentation commits.

## Deviations and Issues

The documented relative `SOURCE_REF=.` failed during the collector's candidate-scoreboard copy. An absolute path in the populated working checkout then encountered ignored build fixture filenames during recursive scanning. No task commit or success ledger edit was made for either failed attempt. Collection succeeded from a clean byte-equivalent local clone with an absolute source path and sibling scratch root, followed by validation against the working checkout. GitHub CLI caching was redirected to build scratch because the default cache was outside writable roots. Failed attempts and raw diagnostics remain scratch-only. No collector/source change, hosted dispatch, push, PR update or acceptance-policy relaxation occurred.

## Decisions and Remaining Limits

Only the specified current-source hosted evidence debt is resolved. Dirty local campaign metadata, causal old/new hashes, constructed/executed cutpoint distinctions, historical failures, unrelated debt and exact physical paired-chip tie uncertainty remain intact. The judgment-tier clean-room prohibition stays flagged without an invented manual approval gate. All four requirements and Phase 20 closure remain owned by independent phase re-verification; this plan marks none complete.

## Self-Check: PASSED

- Both target documents exist and both task commits are reachable on the phase branch.
- Real collection and repeated current-checkout validation pass; fixture provenance was not used.
- Effective source remains the tested 417-file digest; native pins, thresholds and runtime are unchanged.
- The pre-existing `.planning/config.json` edit remains unstaged and unchanged.
