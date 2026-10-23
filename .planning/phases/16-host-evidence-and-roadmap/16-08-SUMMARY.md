---
phase: 16-host-evidence-and-roadmap
plan: "08"
subsystem: release-evidence
tags: [release, packaging, checksum, attestation, mmc3]
dependency_graph:
  requires:
    - 16-07 normal v0.1.11 publication
  provides:
    - Immutable published-byte evidence for EVID-04
  affects:
    - Phase 16 independent goal verification
tech_stack:
  added: []
  patterns:
    - GitHub CLI release and attestation reads
    - Existing archive checker run against an immutable tagged-source snapshot
key_files:
  created:
    - .planning/phases/16-host-evidence-and-roadmap/16-RELEASE-EVIDENCE.md
    - .planning/phases/16-host-evidence-and-roadmap/16-08-SUMMARY.md
  modified:
    - .planning/STATE.md
    - .planning/ROADMAP.md
decisions:
  - Use the non-draft v0.1.11 release and its exact peeled tag commit as the source for all checks.
  - Reuse the existing archive inventory and support parser without adding code or changing release assets.
metrics:
  duration: 18m
  completed: 2026-10-23
commits: 2
plan_head_before: ce80825230c01c2859de52beee2fc18d481cd346
plan_head_after: 468f687204b9498746e538a3650155cfc9862f5d
actuals:
  tokens: 3790
  tasks: 2
  commits: 2
status: complete
---

# Phase 16 Plan 08: Published Release Byte Verification Summary

Bound the v0.1.11 publication to its peeled source commit, exact push CI, all 18 downloaded ZIP checksums and attestations, and six tagged MMC3 metadata files.

## Tasks Completed

1. Proved the macOS arm64 library, runner and libretro ZIP trio against the immutable v0.1.11 source snapshot. The existing archive checker passed, and the downloaded libretro ZIP passed exact-source attestation verification.
2. Expanded the checks to all six platform trios. All 18 ZIPs matched SHA256SUMS and the release API digests, all 18 per-asset attestations verified, and all six embedded libretro metadata files matched the tagged source and passed the existing support parser.

The release is ID 422111807, tag v0.1.11, full commit e47feb3afc34baf1d611ef44153af199424f41a2. Release workflow run 39993108038 completed successfully on that exact push commit; all six build legs, ASan, both no-float legs, hash equality, pinned RetroArch, required CI and publish succeeded. Perf was skipped for the push run; native PR CI runs 39984028386 and 39988911356 both passed perf. PR #50 merged head 82e4686ece0aa8535aa39bbfc06e24d6d6b86af1 at b39a0106e4cccfe9922e051d5680f1f7f84f5efa, and Release Please PR #52 merged to the release commit.

All six released info entries have SHA-256 524bb3d0dbfccea4a2ed93316fadbf2b89be1c48b927d7ec007ba3a411fe0711, equal to the immutable tagged source. The released support claim is “Supported NTSC MMC3: mapper 4, submappers 0/4; MMC6=no; four-screen=no.” Full asset identities, hashes, verification commands and per-platform results are in 16-RELEASE-EVIDENCE.md.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking issue] Redirected the Sigstore TUF cache to ignored task-local output**
- **Found during:** Task 1
- **Issue:** The default user cache was not writable in the sandbox, so gh could not initialize its Sigstore verifiers.
- **Fix:** Confirmed the failure with gh attestation trusted-root and set XDG_CACHE_HOME to build/phase16-release/cache for the verification commands. Trusted-root retrieval and all 18 exact-source checks then passed with the planned repository, workflow, source digest and predicate constraints unchanged.
- **Files modified:** None tracked beyond the planned release evidence.
- **Commit:** e53ca609db1decc570b0a134db5e8c3ff9831764

## Verification

- Release ID, publication timestamp, all 19 asset identities and direct tag commit stayed stable on final requery.
- SHA256SUMS contained exactly 18 expected records; all 18 downloaded archive bytes passed shasum and matched GitHub API sizes/digests.
- Six isolated snapshot checker calls passed with exactly three expected archive records each.
- All six embedded metadata hashes matched the tagged source hash and passed the tagged README/header support parser.
- All 18 attestation results were nonempty SLSA provenance v1 results. Each requested subject digest matched its downloaded bytes; source repository, full release source digest and signer workflow matched the required identity.
- The unrelated Nightly run 39993108154 completed with vectors-full failed and suite-flake passed. This is recorded separately from the required successful Release run.
- No source, test, release asset, verification fingerprint or requirement status was changed. The normal independent Phase 16 verifier still owns the phase verdict and EVID-04 requirement update.

## Auth Gates

None.

## Known Stubs

None.

## Threat Flags

None. This plan adds only evidence about existing immutable release artifacts.

## Self-Check: PASSED

- Summary and release evidence files exist.
- Both task commits are ancestors of HEAD.
- The persisted plan ledger measures two task commits from ce80825230c01c2859de52beee2fc18d481cd346 through 468f687204b9498746e538a3650155cfc9862f5d.
