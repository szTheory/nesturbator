# Roadmap: nesturbator

## Milestones

- ✅ **v1 NROM games with sound** — Phases 1–4.2 (shipped 2026-10-09) — [archive](milestones/v1-ROADMAP.md)
- ✅ **v2 Most of the library plays** — Phases 5–10 (shipped 2026-10-11) — [archive](milestones/v2-ROADMAP.md)
- ✅ **v3 Save states and rewind** — Phases 11–15 (shipped 2026-10-13) — [archive](milestones/v3-ROADMAP.md)
- ✅ **v4 Measured NTSC accuracy and robustness** — Phases 16–18 (shipped 2026-10-24) — [archive](milestones/v4-ROADMAP.md)
- ✅ **v5 NMI, VBlank and sprite-0 timing** — Phases 19–21 (planning milestone shipped 2026-10-30; package publication later completed) — [archive](milestones/v5-ROADMAP.md)
- ✅ **v6 Player-ready NES and reproducible feedback** — Phases 22–23 (planning milestone shipped 2026-10-31; no package release) — [archive](milestones/v6-ROADMAP.md)
- ✅ **v7 Remaining DMA accuracy failures** — Phase 24 (planning milestone shipped 2026-11-01; no package release) — [archive](milestones/v7-ROADMAP.md)
- ✅ **v8 PAL region support** — Phases 25–27.1 (planning milestone shipped 2026-11-03; no package release or tag) — [archive](milestones/v8-ROADMAP.md)
- 🚧 **v9 LibFuturo NES gameplay handoff** — Phase 28 (requirements and roadmap approved 2026-11-03)

Planning milestone completion remains separate from package publication. Release PR #56 later published v0.1.12; v0.1.12 remains the latest package until v9's release is observed. Milestone tags are disabled. See [PROJECT.md](PROJECT.md) and [PRODUCT-ROADMAP.md](PRODUCT-ROADMAP.md) for current delivery and future direction.

## v9 — LibFuturo NES gameplay handoff

An integrator should receive one current, identified Nesturbator release, a truthful initial NTSC provider profile, and a reproducible licensed NROM gameplay scenario through the installed public C package. These outcomes share an exact source/package identity, so one phase owns the full handoff. Splitting package publication from its profile and gameplay proof would create an unnecessary second candidate and requalification boundary. Actual LibFuturo installed-consumer, native-host and game-session acceptance remain LibFuturo-owned.

The owner approved the four [requirements](REQUIREMENTS.md) and the single-phase roadmap on 2026-11-03. Phase 28 implementation, final review and local acceptance are complete for candidate `33628da`; exact-candidate CI, resulting-main CI and the published receipt remain pending. Granularity is coarse.

## Phases

- [ ] **Phase 28: Qualified NES gameplay package** — Deliver a released current package with relocated-consumer proof, a version-bound NTSC profile, and one reproducible interactive game scenario.

## Phase Details

### Phase 28: Qualified NES gameplay package

**Goal**: A LibFuturo integrator can install an identified current Nesturbator release, replay one permitted NTSC game into interactive play through its public C API, and use a provider profile bound to that exact package.
**Depends on**: v8 archived source and existing package, public API and permitted-ROM facilities; no LibFuturo host completion is required.
**Requirements**: PKG-01, INST-01, PROF-01, GAME-01
**Success Criteria** (what must be TRUE):
  1. An integrator can obtain the published library, runner and libretro artifacts, verify their checksums, and identify one consistent accepted source, package version, ABI, behavior revision and state format. A locally generated archive or release candidate alone does not satisfy publication.
  2. A clean C consumer can configure, build and run using only a relocated install prefix, load permitted content, advance frames with input, and receive video and audio; a refused replacement load leaves the active game usable. Its result does not depend on headers or libraries from the source or build tree.
  3. One manifest-permitted NTSC NROM title follows a bounded, reproducible input recipe from boot to a declared gameplay transition with visible response and non-silent audio. A game-specific progress predicate establishes interactive progress; a title picture, input activity, audio or a frame hash alone cannot satisfy it. Nesteroids remains a candidate until its access route and predicate are verified.
  4. An integrator can use the version-bound profile to determine the tested package/source, selected NTSC content and region limits, load/refusal policy, controller sampling, pixel interpretation, audio format/cadence/buffer bounds and lifecycle. Battery-backed game memory and machine snapshots are described separately, and capabilities without current consumer proof remain explicitly unqualified.
**Plans**: 3/4 plans complete; Phase 28 acceptance remains pending hosted CI and the published receipt.

Plans:
- [ ] 28-04-PLAN.md
- [x] 28-01-PLAN.md
- [x] 28-02-PLAN.md
- [x] 28-03-PLAN.md

**Wave 1**
- [x] [28-01-PLAN.md](phases/28-qualified-nes-gameplay-package/28-01-PLAN.md) — Relocated installed consumer and bounded gameplay oracle (2 tasks)

**Wave 2** *(after Wave 1)*
- [x] [28-02-PLAN.md](phases/28-qualified-nes-gameplay-package/28-02-PLAN.md) — NTSC provider profile and offline receipt checks (2 tasks)

**Wave 3** *(after Wave 2)*
- [x] [28-03-PLAN.md](phases/28-qualified-nes-gameplay-package/28-03-PLAN.md) — Release lineage, candidate acceptance and shipping handoff (3 tasks)

### Phase 28 evidence and release boundary

Discuss Phase 28 must settle the exact game access route and independent basis for its progress predicate, the current LibFuturo handoff contract, and the package-line reconciliation before planning implementation. Reuse the existing installed-package and gameplay facilities where they detect the required boundary; add only the missing owned checks with useful recurring detection. Prefer a small local helper to another dependency.

The first route is NTSC-specific. Preserve the core's shipped region behavior and document metadata precedence; an NTSC preference must not be presented as an override of explicit PAL metadata. A new public API requires a demonstrated consumer gap. The handoff does not establish PAL-through-LibFuturo, LibFuturo host playback, Playstead, physical-device pacing, hardware accuracy, snapshots or broad mapper/title compatibility.

Freeze and qualify the final source/package/profile/scenario identity before publication. Publication is Phase 28's final acceptance gate and is checked again at milestone closeout under [VERIFICATION-POLICY.md](VERIFICATION-POLICY.md): bounded accepted local workflow, related PR triage, exact-candidate required CI, green resulting main CI, clean project-owned release worktrees with unrelated work preserved, and verified released artifacts. Normal release mechanics may use their own workflow commands; PKG-01 remains pending until the published-package receipt exists. Keep the older interrupted CI/AccuracyCoin campaign stopped and unknown. Historical v8 results do not qualify this release candidate.

See the [coordination context](../docs/coordination/milestone-context-2026-11-03.md), [playability seed](seeds/SEED-261009-zs2-single-game-playability-check.md), [asset policy](../ASSET_POLICY.md) and [automation direction](playtesting/AUTOMATION.md) for evidence limits. A concrete core fault discovered during qualification needs independently supported, bounded scope; it does not automatically promote standalone scene-accuracy or mapper work.

## Progress

Execution order: Phase 28. All three plans have completed local execution and handoff validation. The phase remains In Progress until exact-candidate CI, resulting-main CI and published-package acceptance are verified; PKG-01 remains pending.

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 28. Qualified NES gameplay package | 3/4 | In Progress | — |

## Backlog disposition

2026-10-13: Backlog 999.2, “Bump AccuracyCoin past 673ef55,” was promoted into [Phase 17](milestones/v4-ROADMAP.md#phase-17-complete-pinned-conformance-baseline) and CONF-01 through CONF-04. No duplicate active backlog remains.
