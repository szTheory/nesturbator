---
phase: 22-install-and-protect-progress
plan: "04"
subsystem: player-documentation
tags: [retroarch, libretro, release, battery-save, save-state, ctest, cpack]
dependency-graph:
  requires:
    - phase: 22-01
      provides: canonical API and native-state documentation
    - phase: 22-02
      provides: canonical runner guide and option checks
    - phase: 22-03
      provides: release/testing guides and README claim ledger
  provides:
    - player-first-install-controls-and-progress-route
    - isolated-progress-recovery-walkthrough
    - dated-release-and-host-menu-observations
  affects: [phase-22-review, phase-22-verification]
tech-stack:
  added: []
  patterns: [source-bound-documentation, disposable-owned-fixtures, exact-artifact-identity]
key-files:
  created: [docs/progress.md]
  modified: [README.md, docs/releases.md, docs/testing.md]
key-decisions:
  - "Keep the README in player order and preserve its checked support statement and marked Apple Silicon install command."
  - "Treat module path plus digest and matching provenance as exact build identity; a displayed core name/version is preliminary, and unavailable identity stays unknown."
  - "Keep host settings, logical input ports, native snapshots, host state containers and battery data as distinct evidence and storage boundaries."
  - "Limit the walkthrough to generated or manifest-listed fixtures; keep the newest battery copy outside active test/save paths."
metrics:
  duration: "about 20 minutes (estimated; start timestamp was not retained)"
  completed: 2026-10-30
status: complete
actuals:
  tokens: 6451
  tasks: 3
  commits: 3
  plan_head_before: 7800f27ee86b61d00a88e09dbd85a279eb365b9e
  plan_head_after: d9f49de379ea7beb80f92a5acd1be766ee368091
requirements-completed: []
---

# Phase 22 Plan 04: Install and Protect Progress Summary

The README now takes players through content limits, archive selection and exact build identity, loading, host-owned controls, progress protection and troubleshooting, while a separate guide keeps disposable recovery evidence away from real saves.

## Accomplishments

- Reorganized [README.md](../../../README.md) around the player route, preserving the checked MMC3 support clause and the complete version-marked Apple Silicon install block. Added the six OS/architecture archive choices, configured core/info locations, content and no-content loading, host-port guidance, progress warning and troubleshooting.
- Added [docs/progress.md](../../../docs/progress.md) distinguishing battery RAM, native snapshots, RetroArch state containers, rewind and automatic loading. It records the current format-3 check and unenforced behavior revision accurately, gives backup/hash/rollback steps, and labels unknown cross-build SRAM and incompatible host autoload recovery.
- Expanded [docs/releases.md](../../../docs/releases.md) with loaded-build identity guidance, source/tag and artifact distinctions, platform digest tools, and unknown-on-insufficient-evidence handling. [docs/testing.md](../../../docs/testing.md) now explains the disposable fixture route and the scope of native and host observations.
- Kept each README claim represented in its maintained documentation home. The detailed migration ledger below preserves the original README's unique material across this plan and Plans 22-01 through 22-03.

## README claim-conservation ledger

| Original README claim or material | Maintained home after this plan |
|---|---|
| Core identity, deterministic/accuracy-class description, no GUI and three deliverables | README; API and behavior details in `docs/api.md` |
| No-cartridge test card, supported content and short PPU capability summary | README; detailed behavior in `docs/api.md` |
| Checked NTSC MMC3 mapper/submapper, MMC6 and four-screen statement | README, kept as the checked support clause |
| Trainer RAM, cartridge rejection and reload guarantees | `docs/api.md`; linked tests and evidence in `docs/testing.md` |
| Fixture inventory, mapper results, frame/movie hashes, AccuracyCoin pins/results, campaign bounds and protected PASS evidence | `docs/testing.md`, `tests/` and `tests/roms/manifest.txt` |
| CPU RAM inspection, frame/audio format, APU/PPU timing, mapper interface and behavior-revision contract | `docs/api.md`; test scope and limits in `docs/testing.md` |
| Build prerequisites, presets, checks, CTest groups, CI lanes and evidence limits | `docs/testing.md` |
| Hosted collection, performance allow file, provenance and threshold qualifications | `docs/testing.md` and its linked source evidence |
| Library use, battery saves, native states, status codes and color table | `docs/api.md`; practical host/save guidance in `docs/progress.md` |
| Runner options, output, movies, stop behavior and exit statuses | `docs/runner.md` |
| Archive contents, names, extraction layout, marker checks and maintainer release sequence | `docs/releases.md`; concise player selection and install remain in README |
| Libretro adapter identity, metadata, callbacks, battery span, state, JAM/audio/run-ahead and existing host evidence | `docs/api.md` for API semantics, `docs/testing.md` for checks and evidence; README links to both |
| Marked Apple Silicon install command and source-build extraction validation | Literal marked block remains in README; maintenance and extraction test in `docs/releases.md` |
| Build-tree RetroArch tryout and exact limits of local/host evidence | `docs/testing.md` |
| Original README save warning, data-type distinctions, backup and rollback guidance | README short warning and `docs/progress.md` full procedure |
| Project ownership, clean-room provenance, AI-assisted development, asset policy and MIT license | README with `PROVENANCE.md`, `ASSET_POLICY.md` and `LICENSE` links |
| ROM/test-vector licenses, hashes and regeneration steps | `docs/testing.md` and the fixture manifest |
| Nintendo non-affiliation and names used to describe the emulated system | README |
| Player installation order, physical controller assignment, launch-route caveat and troubleshooting | README; dated observation and test limits in `docs/testing.md` |

## Dated release and host-menu observations

Read-only refresh on 2026-10-30 found GitHub release `v0.1.11`, non-draft and published 2026-10-23. The release listed the expected 18 library/runner/libretro archives (three components across Linux, macOS and Windows and x64/arm64) plus `SHA256SUMS`. The public macOS arm64 libretro archive digest, `fb9c631818d7196d9306f5357f74d281af97ad87dd27066c74e47bbb9bc1dc0f`, matched its published checksum. GitHub listed two attestations for that digest; local `gh attestation verify` could not initialize a Sigstore verifier, so cryptographic verification is not claimed.

The current official RetroArch/Libretro guides were checked read-only: [Installing Cores](https://docs.libretro.com/guides/download-cores/) describes `cores/`, the matching info file, configured Core Info and platform module extensions; [Input and Controls](https://docs.libretro.com/guides/input-and-controls/) describes Device Index selection for connected host devices; [Directory Configuration](https://docs.libretro.com/guides/change-directories/) identifies configurable save/state directories; [Starting a Game](https://docs.libretro.com/guides/starting-a-game/) describes selecting a core/content or starting without content; [Quick Menu](https://docs.libretro.com/guides/quick-menu/) covers host state controls; and [Generating Logs](https://docs.libretro.com/guides/generating-logs/) covers host log settings. The Installing Cores page showed an update date of 2026-10-22 and Directory Configuration 2026-07-25 at inspection. Labels and unused-device choices vary with host, driver and version. Apple's [notarization guidance](https://developer.apple.com/documentation/security/notarizing-macos-software-before-distribution) documents possible macOS quarantine restrictions; it does not establish whether this project's binary is signed or notarized.

## Verification

- Task 1 target build passed. Its six-test CTest selection passed 7/7 including fixture setup. The documented scratch recipe was run in a unique temporary tree: configure/build passed, the first selection passed 2/2, the refusal/roundtrip selection passed 4/4, and the separately backed-up generated battery digest remained unchanged. After adding the explicitly isolated scratch `config` directory and PowerShell setup note, the tracer build and seven-test selection were rerun and passed before further expansion.
- Task 2's exact release/docs selection passed 6/6.
- Task 3 `cmake --preset ci` and `cmake --build --preset ci` passed. Its focused selection passed 18/18. CPack created the three configured macOS arm64 archives; `tests/cmake/check_archives.cmake` accepted all three, and `tests/cmake/check_install_line.cmake` confirmed the README extraction line against the source-built libretro archive.
- `git diff --check` passed. Repository commit hooks passed for all three task commits. No runtime code, package layout, dependency, test registration or public symbol changed.

## Scope and evidence limits

No full CI workflow or conformance campaign was run; the parent phase executor will run full CI after the serial plans. This local CPack run establishes only the configured macOS arm64 archives, not all six release targets or target-host runtime behavior. No RetroArch host/device operation, user sign-off, personal content, actual save, other-core state or publication was performed. Existing logical-port tests do not cover every physical button or reconnect path. Native state refusal/generation tests do not establish incompatible host-container recovery, and cross-build SRAM reuse remains conditional and otherwise unknown. Phase 22 review and verification remain the parent executor's responsibility; READY requirements are intentionally pending.

## Deviations from Plan

None. The final documentation commit also made the walkthrough's scratch `config` directory explicit; this completes the planned isolation detail. The tracer checks passed again after that correction.

## Authentication Gates

None.

## User Setup Required

None.

## Next Plan Readiness

Plan 22-04 is complete. Phase 22 is **awaiting the parent executor's review/verifier and full CI**; it is not marked complete here, and Phase 23 has not started. Next command: `$gsd-verify-work 22`. It reviews the completed Phase 22 evidence and checks the phase goal. Context may be cleared before running it; read [STATE.md](../../STATE.md), [continue.md](../../continue.md), the phase context and this summary first.

## Self-Check: PASSED

- All four plan documentation files and this summary exist.
- Task commits 070c2d5, 5abb684 and d9f49de are ancestors of HEAD.
- Added documentation diff scan found no TODO/FIXME/placeholder stubs.

## Task Commits

1. **Task 1: progress walkthrough** - `070c2d5` (docs/test)
2. **Task 2: archive and identity guidance** - `5abb684` (docs/test)
3. **Task 3: controls and troubleshooting** - `d9f49de` (docs/test)
