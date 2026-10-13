# Requirements: nesturbator

**Milestone:** v4 Measured NTSC accuracy and robustness
**Defined and scope approved:** 2026-10-13
**Core Value:** Games behave as they do on the console, identically on every platform, from a small MIT-licensed C library that any host can embed.

## v4 Requirements

The user approved these requirements with the v4 proposal on 2026-10-13. The new categories do not collide with archived requirement IDs.

### Host evidence

- [ ] **EVID-01**: The real-host progression check rejects dropped, doubled or wrong-load sequences using distinguishable frames or absolute state anchors, including negative controls for identical-image ambiguity.
- [ ] **EVID-02**: Network-driven checks confirm bounded completion of safely retried requests and fail/restart unconfirmed non-idempotent actions; no blind FRAMEADVANCE or PAUSE_TOGGLE retry.
- [ ] **EVID-03**: Fixture selection uses exact content identity/basenames, and adversarial parent-directory names cannot change the selected path.
- [ ] **EVID-04**: Released libretro metadata describes the implemented MMC3 support truthfully and agrees with documented supported profiles.

### Conformance

- [ ] **CONF-01**: The AccuracyCoin pin, licence/hash, compressed-name decoder and protected-key migration are validated together, retaining the old-pin comparison.
- [ ] **CONF-02**: The runner inventories every scored test and DRAW entry at that immutable pin; missing or duplicate entries and malformed result tables fail validation.
- [ ] **CONF-03**: A bounded automated run gives each scored test an explicit pass/fail/skip/not-run/timeout classification with retained results and diagnostics; DRAW is excluded from the score denominator.
- [ ] **CONF-04**: Previously protected passing keys remain mapped and accounted for; same-pin pass regressions fail, while changed upstream meanings or denominator changes require an explicit reviewed disposition.

### Execution robustness

- [ ] **ROB-01**: A small owned fuzz/replay target executes bounded accepted profiles for all six supported board families with generated program/CHR bytes and controlled input.
- [ ] **ROB-02**: The target exercises reset, save/load and unload/reload, checking instance independence and deterministic continuation through existing API contracts.
- [ ] **ROB-03**: Normal CI replays a fixed permitted/generated corpus; finite nightly sanitizer campaigns retain deterministic crash/timeout reproducers with measured time, memory and execution budgets.
- [ ] **ROB-04**: Guest loops and JAM have tested progress/stop behavior, including the adapter's observable halt policy; any reproduced library/host contract violation receives a minimized regression and correction or explicit rescoping.

### Product direction

- [ ] **DIR-01**: The repository has a product roadmap with Immediate/Near/Mid/Long horizons, stable outcome IDs, evidence and promotion triggers linked from the project instructions and preparation handoff.
- [ ] **DIR-02**: New-milestone and complete-milestone instructions require the agent to reconcile and refresh that roadmap within the invoked step, preserving delivered seed evidence and unresolved debt.


## Acceptance boundaries

A complete inventory and honest bounded observation are required; an all-pass score is not. At the proposed immutable AccuracyCoin pin, report 146 scored tests separately from five DRAW entries. Preserve the 17 existing protected results and old-pin comparison. Explicit not-run/timeout classifications must include the cause and raw diagnostics rather than pretending execution occurred.

The execution target begins with a short measured prototype proving useful additional program-execution reach beyond loader/state fuzz. All six supported board families must reach execution. Select fixed replay and sanitizer budgets from throughput/coverage measurements. Finite evidence does not prove universal safety or promise a discovered bug count. Confirmed reproducible safety failures receive minimized regressions and correction or an explicit scope amendment; they are not silently waived.

Every behavior change carries the project-required test, README/public-header documentation, deterministic/state/parity evidence as relevant, and CI/perf gates. No new dependency is approved. DIR-01/DIR-02 artifacts are initialized in this step; Phase 16 verifies and ships the operating contract alongside its host-evidence result.

## Future Requirements

These are direction, not committed v4 scope. [PRODUCT-ROADMAP.md](PRODUCT-ROADMAP.md) is the ordering and promotion authority.

- **ACC-01**: Improve pinned NTSC accuracy through named, game-relevant failure clusters selected from the full baseline (RM-004). The historical all-pass aspiration is preserved in archived requirements; no universal passing score is promised here.
- **ACC-02**: PAL and Dendy run at their own timing, scoped independently with regional acceptance evidence (RM-005/RM-006).
- **STATE-06**: A movie starts from a saved state, when a concrete host/replay need justifies it (RM-008).
- **RUN-04**: Runner preflight/expectation reporting serves a named launcher contract; interface and exit statuses are scoped before implementation (RM-008).
- **LIB-03**: A committed MMC3 real-game hash from Stallar, once the exact code/art provenance is verified and the fixture adds useful evidence.
- **LIB-04**: Selected additional board profiles, four-screen VRAM or MMC6, chosen individually by demand and validation readiness (RM-007/RM-011).
- **HW-01**: Selected expansion audio, FDS and additional peripherals, split into independently promotable outcomes (RM-010/RM-012/RM-013).

**Historical disposition:** PERF-01's remaining accepted-program execution portion is promoted into ROB-01 through ROB-03; existing loader/state fuzz is credited. PERF-02/PERF-03 already shipped and remain regression gates. Backlog 999.2 is promoted into CONF-01 through CONF-04 / RM-001; do not leave a duplicate active backlog item.

## Out of Scope

| Feature | Reason |
|---|---|
| PAL/Dendy implementation and new board families | Separate outcomes after measurement; v4 deepens evidence for currently supported NTSC content. |
| Expansion audio, MMC5, FDS and additional peripherals | Separate hardware and host contracts, sources and lawful fixtures are needed. |
| An obligation to pass every AccuracyCoin test or repair unnamed hardware failures | Baseline first; exact repair scope follows measured results. |
| Achievements, netplay, a new host ABI and broader real-RetroArch OS coverage | Require a concrete consumer or measured incremental value; portable existing host coverage stays credited. |
| Arbitrary old-debt/cosmetic refactor sweep | Tune-up is limited to the three named Phase 13 weaknesses, truthful MMC3 metadata and roadmap upkeep. |
| Leaderboard submission | Independent outward-facing choice, unnecessary to establish the baseline. |
| GUI/window/audio-device/input-device code, bundled BIOS/database and GPL/LGPL emulator source | Existing deliverable, ownership, licence and clean-room boundaries remain in force. |
| New dependency/framework, calendar commitments or promised package versions | Existing owned code/tools suffice; product direction uses readiness horizons. |

## Traceability

Every approved v4 requirement maps to exactly one phase in [ROADMAP.md](ROADMAP.md).

| Requirement | Phase | Status |
|---|---|---|
| EVID-01 | Phase 16 | Pending |
| EVID-02 | Phase 16 | Pending |
| EVID-03 | Phase 16 | Pending |
| EVID-04 | Phase 16 | Pending |
| CONF-01 | Phase 17 | Pending |
| CONF-02 | Phase 17 | Pending |
| CONF-03 | Phase 17 | Pending |
| CONF-04 | Phase 17 | Pending |
| ROB-01 | Phase 18 | Pending |
| ROB-02 | Phase 18 | Pending |
| ROB-03 | Phase 18 | Pending |
| ROB-04 | Phase 18 | Pending |
| DIR-01 | Phase 16 | Pending |
| DIR-02 | Phase 16 | Pending |

**Coverage:**
- v4 requirements: 14 total
- Mapped to phases: 14
- Unmapped: 0
- Duplicate mappings: 0

---
*Requirements defined and approved: 2026-10-13*
*Last updated: 2026-10-13 at v4 initialization*
