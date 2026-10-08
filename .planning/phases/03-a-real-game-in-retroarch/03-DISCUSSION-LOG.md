# Phase 03: A real game in RetroArch - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in `03-CONTEXT.md` — this log preserves the alternatives considered.

**Date:** 2026-10-08
**Phase:** 03-a-real-game-in-retroarch
**Areas discussed:** NROM scope and fixtures, pixel calibration, controller ports and movies, automated verification, dependencies and owner hand-off

The owner asked for a broad, multi-role review through diminishing returns and authorized automatic acceptance of the synthesized recommendations. No follow-up question was needed. The recommendations below were assessed from product, architecture, CPU/PPU timing, graphics, security/licensing, test, CI/release, and maintenance perspectives.

---

## NROM scope and licensed fixtures

| Option | Description | Selected |
|--------|-------------|----------|
| One minimal ROM | Less fixture maintenance, but weak coverage of CHR RAM and multiplayer behavior. | |
| Three complementary NROM games | Slightly more fixture bytes and hashes; covers varied rendering, CHR RAM, and two-player input while staying on mapper 0. | ✓ |
| Broader mapper/game set | More user coverage, but adds mapper scope that belongs after the NROM phase. | |

**User's choice:** Automatically accept the recommendation: pin Nesteroids (MIT), Double Action Blaster Guys v2 (zlib), and RHDE v0.07 (all-permissive) and record each binary's source, pin, license, and checksum in the ROM manifest.
**Notes:** DABG is the two-player case; RHDE is the CHR-RAM case. Security and legal review favored only explicitly redistributable games, no hidden ROM assets, bounded parsing, and no emulator-source dependency. Added fixture maintenance is offset by distinct recurring test value.

---

## Pixel calibration

| Option | Description | Selected |
|--------|-------------|----------|
| Keep the Phase 1 direct decode | Lowest change cost; leaves the already deferred display calibration unresolved. | |
| Add one fixed, documented NTSC-to-sRGB calibration to the offline generator | Better real-game presentation and a stable cross-platform test baseline; no runtime floating point or new dependency. The palette is a documented default, not a claim that one look suits every display or title. | ✓ |
| Add selectable palettes or filters | More player control, but adds UI/configuration surface and makes screenshot evidence less comparable. | |

**User's choice:** Automatically accept the recommendation to close Phase 1's palette deferral with one deterministic generated calibration; keep core-native pixel hashes independent of display RGB.
**Notes:** Graphics review favored explicit source assumptions and exact generated-table tests. Architecture and test review favored a single reproducible default and no palette branch in the emulated state. Per-game settings and filters remain deferred.

---

## Controller ports and movie replay

| Option | Description | Selected |
|--------|-------------|----------|
| Implement only player one | Meets a basic playable slice with less interface work. | |
| Implement both standard NES ports | Exercises the actual two-port hardware and enables deterministic two-player cases without expansion-device scope. | ✓ |
| Add Four Score or other expansion devices | Broader compatibility, but outside the phase boundary. | |

**User's choice:** Automatically accept standard port 1 and port 2 with per-frame scripted input and deterministic `--movie FILE` replay.
**Notes:** Product review favored standard controls and an optional DualSense through RetroArch. Test and security review favored injected input, a small owned replay format, and no input library or physical-device requirement. Exact encoding remains a planning detail.

---

## Automated verification

| Option | Description | Selected |
|--------|-------------|----------|
| Depend on a developer's local RetroArch install and allow missing installs to skip | Convenient locally but leaves the key end-to-end path unverified on pull requests. | |
| Require one checksum-pinned RetroArch release-binary lane on macOS CI | Verifies game loading and the screenshot seam on every change; normal six-platform hash and host checks stay in place. | ✓ |
| Build a graphical stack or add a new test framework | Adds dependencies and maintenance without improving the project-owned seam tests enough to justify it. | |

**User's choice:** Automatically accept the required macOS CI lane, plus scripted ROM hashes, movie replay, libretro seam checks, malformed-loader tests, fuzz corpus regressions, nightly libFuzzer, and AccuracyCoin scoreboard tests.
**Notes:** DevOps review recommended fail-closed behavior in the CI lane when the pinned RetroArch binary or screenshot is missing, while preserving optional local skip behavior. The lane uses the released binary only. Owner UAT is not a gate for results these checks can establish.

---

## Cross-cutting lens synthesis

- **Product:** The phase should result in an actual playable NROM path in RetroArch, with no audio or extra mapper promises.
- **Architecture and timing:** The new PPU and cartridge path must honor documented bus/frame behavior and preserve deterministic boundaries with the existing CPU.
- **Graphics:** Keep native pixel hashes separate from the one calibrated display palette; compare exact output in the RetroArch seam.
- **Security and licensing:** Reject hostile files before allocation, fuzz the parser, manifest every committed ROM, and use no emulator source.
- **DevOps and tests:** Run recurring checks in CI when they prevent regressions; reserve nightly for high-cost fuzzing; collect evidence by commit/run identity.
- **Maintenance:** Reuse current CTest, runner, adapter, screenshot comparator, and offline generator; add no runtime or CI framework dependency.

## the agent's Discretion

- Internal PPU layout and exact module split.
- Movie serialization details and stable CLI diagnostics.
- Which pinned game supplies the mandatory RetroArch screenshot, based on a stable frame.
- Cache strategy for the pinned RetroArch artifact, provided the CI check remains required and checksum-verified.

## Deferred Ideas

- Audio and sound hashes: Phase 4.
- Other mapper families, PAL/Dendy, expansion controllers, save states, battery RAM, palette choices, and filters: future phases or roadmap work.
- Phase 2 LXA profile, JAM result, and instruction/frame-boundary questions: investigate during Phase 3 research only if they affect its stated requirements.
