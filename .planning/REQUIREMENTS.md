# Requirements: nesturbator

**Milestone:** v9 — LibFuturo NES gameplay handoff  
**Defined:** 2026-11-03  
**Core Value:** Games behave as they do on the console, identically across platforms, from a small MIT-licensed C library that any host can embed.

## v1 Requirements

Requirements for the committed v9 scope.

### Package identity and installation

- [ ] **PKG-01**: A consumer can identify and install a published Nesturbator package built from the exact accepted current source, with consistent package/header version, ABI, behavior revision and state-format identity; the release includes the library, runner and libretro artifacts with verified checksums.
- [x] **INST-01**: A clean C consumer can configure, build and run against the installed library package after the install prefix is relocated, without resolving headers or libraries from the source or build tree; it can load a valid permitted ROM, advance frames with input, obtain video/audio, and verify that a refused replacement load preserves the active game.

### LibFuturo provider profile

- [ ] **PROF-01**: A version-bound first-route profile identifies the exact Nesturbator package/source and accurately describes its selected NTSC content and region limits, load/refusal behavior, controller sampling, video representation, audio format/cadence/buffer bounds and lifecycle; it distinguishes battery-backed game memory from machine snapshots and marks every capability without current consumer proof as unqualified.

### Interactive game scenario

- [x] **GAME-01**: One ROM already permitted by the repository manifest has a bounded, reproducible boot-to-gameplay input scenario that runs through the relocated installed package; a game-specific progress predicate demonstrates an interactive transition with visible response and non-silent audio. A title screen, input activity, audio alone or frame hash alone does not satisfy the requirement. Nesteroids is the candidate until its exact route and predicate are verified.

## Future Requirements

- Actual NES gameplay acceptance through LibFuturo's installed consumer and native host; those changes and claims belong to the LibFuturo milestone.
- PAL-specific LibFuturo profile and game-session qualification if the consumer needs European titles.
- Additional mapper families, long-tail game coverage, snapshots, and broader consumer compatibility after concrete demand and acceptance are available.
- Playstead adoption, owned by the Playstead consumer workflow.

## Out of Scope

| Feature | Reason |
|---------|--------|
| Implementing LibFuturo's installed consumer, NES native host, audio host, or game-session UX | LibFuturo owns its host and actual end-to-end acceptance; Nesturbator supplies the versioned package/profile/scenario. |
| Claiming Playstead integration or readiness | Playstead is not ready, and its adapter/readiness/input/progress acceptance is consumer-owned. |
| PAL-through-LibFuturo acceptance | Nesturbator's PAL core capability ships in v8, but the first consumer profile is NTSC; qualify PAL separately when required. |
| New public API or ABI change without a demonstrated current consumer gap | The current API already exposes region and cadence information; avoid speculative contract changes. |
| New mapper families or all-games compatibility claim | This milestone qualifies one permitted NROM title; broad mapper support remains a long-term goal. |
| Scene-accuracy fixes without a repeatable core fault and independent oracle | RM-015 remains conditional; absent its trigger, skip a standalone scene-accuracy milestone. |
| Commercial ROMs, private captures/saves, hardware tests, restarting the stopped campaign, or unbounded CI | Use manifest-permitted fixtures and the existing bounded evidence/authority policy. |

## Evidence and release policy

The [v9 coordination context](../docs/coordination/milestone-context-2026-11-03.md), [single-game playability seed](seeds/SEED-261009-zs2-single-game-playability-check.md), [asset policy](../ASSET_POLICY.md), and [automation direction](playtesting/AUTOMATION.md) define the current evidence boundary. The v8 722-test receipt is historical and does not qualify a new package. Any code behavior change must retain the project's existing deterministic, clean-room, test-and-document requirements. Release is a milestone closeout gate under [VERIFICATION-POLICY.md](VERIFICATION-POLICY.md): exact candidate checks, related PR triage, green main CI, clean project-owned release worktrees with unrelated work preserved, and verified package/source/checksum identity. This does not restart the older stopped campaign.

## Traceability

Each requirement maps to exactly one owner-approved v9 phase. Requirements and roadmap are approved.

| Requirement | Phase | Status |
|-------------|-------|--------|
| PKG-01 | Phase 28 | Pending |
| INST-01 | Phase 28 | Complete |
| PROF-01 | Phase 28 | Pending |
| GAME-01 | Phase 28 | Complete |

**Coverage:**
- v1 requirements: 4 total
- Mapped to phases: 4
- Unmapped: 0

---
*Requirements defined: 2026-11-03*
*Last updated: 2026-11-03 after owner approval of the v9 requirements and Phase 28 roadmap.*
