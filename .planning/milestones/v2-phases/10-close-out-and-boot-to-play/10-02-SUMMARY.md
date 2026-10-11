---
phase: 10-close-out-and-boot-to-play
plan: 02
subsystem: test-assets
tags: [rom, manifest, notices, nes-runner]
requires: ["10-01"]
provides:
  - "tests/roms/runner.nes, nes-runner v1, byte-for-byte"
  - "manifest row and credits-text note"
  - "nes-runner section in THIRD-PARTY-NOTICES.md"
affects: [10-03, 10-04, 10-05, 10-06, 10-07]
tech-stack:
  added: []
  patterns: ["ROM committed once from release asset after sha256 check"]
key-files:
  created: [tests/roms/runner.nes]
  modified: [tests/roms/manifest.txt, THIRD-PARTY-NOTICES.md]
key-decisions:
  - "Manifest row placed after the rhde.nes row, with a # comment line above it about credits-screen text"
status: complete
commits: 2
plan_head_before: c22f9268cad457a268b22863edd55b25c53ed196
plan_head_after: 096a071da40ccd51789fdcd819378ff60938d429
actuals:
  tokens: 6000
  tasks: 2
  commits: 2
duration: 10 min
completed: 2026-10-11
---

# Phase 10 Plan 02: nes-runner ROM intake Summary

nes-runner v1 (81,936 bytes, sha256 1a67bfab...4384) is committed byte-for-byte with its manifest line and a MIT notice, and the runner plays it for 180 frames with exit 0.

## Tasks

| Task | Commit | Result |
|------|--------|--------|
| 1 Tracer: ROM + manifest | 9c9a642 | Size and sha256 matched before copy; `manifest.sha256` and selftest pass; `nesturbator-run` prints `frame 180 ticks 128661072 sha256 d407ddd6...` |
| 2 Notices | 096a071 | `## nes-runner` section: upstream, v1, commit 47537be, SHA-256, MIT text, provenance (LICENSE.md in e2d70cf, 2019-06-28, three days after v1 tag), credits-text sentence, driver attribution; intro sentence updated |

## Driver attribution evidence

Confirmed from the nes-runner tree at commit 47537be: `sound/driver.s`, lines 2 to 4: "The NSF music driver for FamiTracker / Version 2.11 / By jsr". The notice says "FamiTracker NSF music driver, version 2.11, by jsr". The email address on line 4 of that file was deliberately not copied. Nothing else was attributed. Only the game repository was read, no emulator source.

## Deviations from Plan

None - plan executed as written.

## Verification

`cmake --workflow --preset hygiene` passed after each task; `cmake --workflow --preset ci` passed at the end.

## Known Stubs

None.

## Threat Flags

None.

## Self-Check: PASSED

runner.nes, manifest.txt and THIRD-PARTY-NOTICES.md present; commits 9c9a642 and 096a071 are ancestors of HEAD.
