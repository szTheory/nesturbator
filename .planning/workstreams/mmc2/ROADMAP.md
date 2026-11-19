# Roadmap: nesturbator

## Milestones

- ✅ **v10 Bounded MMC2 mapper 9** — Phase 29 is verified and archived. Its outcome and preserved limits are summarized in the [v10 handoff](V10-HANDOFF.md). No v10 public package release is claimed.
- 🚧 **v11 MMC2 mainline compatibility port** — Phase 30 carries the bounded capability onto live GitHub `main` and through the normal qualified delivery path.

## v11 — MMC2 mainline compatibility port

Deliver the already verified bounded MMC2 capability on the actual public
mainline baseline. The [requirements](REQUIREMENTS.md) form one cohesive
outcome: hosts can use the supported mapper through all three deliverables,
restore its state deterministically, and identify the compatible published
build. Implementation, compatibility, automated acceptance and delivery belong
to the same phase; splitting them would leave an unusable partial port.

The opening read-only review observed public `main` at
[`8923b66`](https://github.com/szTheory/nesturbator/commit/8923b66cca16739c5b402643798515cf5537ae22),
release 0.1.13, behavior revision 9, state format 3 and ABI 1. Local
`origin/main` was stale. Revalidate the live identity during Phase 30 discussion
and before source work. A changed base needs a recorded compatibility review;
it never permits taking the entire Phase 29 branch or silently reusing its CI.

## Phases

- [ ] **Phase 30: MMC2 mainline compatibility and delivery** — Hosts can use and restore the bounded mapper-9 profile from a qualified mainline build while existing supported behavior remains protected.

## Phase Details

### Phase 30: MMC2 mainline compatibility and delivery

**Goal**: The supported MMC2 profile is available from the live mainline through the C library, runner and libretro adapter, with explicit state compatibility and verified delivery.
**Depends on**: Completed v10 MMC2 implementation and owned fixtures; revalidated live-main source identity; existing mapper, state, CMake/CTest and release facilities.
**Requirements**: MMC2-07, MMC2-08, MMC2-09, MMC2-10, MMC2-11, MMC2-12
**Success Criteria** (what must be TRUE):
  1. The same bounded NES 2.0 mapper 9/submapper 0 profile exposes the expected PRG banks, mirroring and both CHR latch behaviors through actual pattern reads, with owned direct markers and matching exercised video/audio through the library, runner and libretro adapter. Unsupported variants retain transactional refusal.
  2. Cold and restored executions continue identically with every MMC2 selector and latch preserved and derived mappings rebuilt. The reviewed baseline advances to behavior revision 10 and state format 4 while retaining ABI 1; incompatible prior formats are refused and the public documentation states the actual boundary.
  3. Existing supported mapper behavior and protected output evidence remain intact. Necessary serialized-state hash changes have explicit reasons, and protected AccuracyCoin inputs, baselines, campaign evidence and acceptance rules are not re-anchored to make the port pass.
  4. A source-bound preflight and focused controls pass before expensive acceptance. The standard full workflow then passes on that same candidate within existing limits and generates the library, runner and libretro packages; automated verification establishes the port without avoidable human UAT.
  5. The port has a reviewed Conventional Commit PR, green required candidate checks before merge, and green resulting-main checks before publication. Release-worthy output is published through the existing release path with verified contents, checksums and exact source identity; no LibFuturo receipt or consumer acceptance is inferred.
**Plans**: TBD after Phase 30 discussion and planning.

### Required discussion and execution order

1. Run `$gsd-discuss-phase 30 --ws mmc2` before planning. Use bounded review of architecture/state compatibility, verification/release and consumer boundaries. Retain settled v10 hardware/profile decisions and their evidence tier. Recheck live-main identity and distinguish it from both the local primary branch and stale tracking refs.
2. Plan the minimal MMC2 port in a clean, isolated phase worktree pinned to that exact live base. Record how selected workstream planning records reach that worktree so `--ws mmc2` resolves the correct state. Preserve unrelated dirty checkouts and use the configured GSD isolation route. Do not resolve conflicts by taking whole files from Phase 29.
3. Compare compatibility fields, public/provider metadata, real test registrations, golden state outputs and protected AccuracyCoin bindings cheaply before consuming long acceptance. Port the existing owned behavior and fixtures; derive all compatibility changes from the target baseline. A moved base or protected mismatch must be diagnosed before execution.
4. Run focused checks, then the existing source-bound full workflow under the reviewed plan and all current resource limits. Reuse the standard workflow rather than recovery wrappers. Keep each completed result bound to its own candidate and inspect available CI timing for safe, demonstrated reductions in duplicate work.
5. Complete source/documentation review and same-candidate goal verification, then use the ordinary PR, main CI and release gates. Check project-owned worktree cleanliness, preserve unrelated dirty work, and resolve ownership before publication. Record delivery and refresh the rolling direction at milestone closeout.

### Boundaries and evidence limits

The port retains v10's exact mapper profile: NES 2.0 mapper 9/submapper 0,
128 KiB each of PRG and CHR ROM, with no declared RAM, trainer, battery or
four-screen variant. The adopted MMC2 trigger rows remain secondary-reference
evidence. No hardware, Punch-Out!!, broad mapper-9, MMC4 or commercial-title
claim follows from marker or hash parity.

The five inherited unverified-prohibition flags and v10 WR-01 remain visible
in the [v10 handoff](V10-HANDOFF.md). Scope-specific stale
wording may be corrected in the port without rewriting its historical verdict.
Phase 29 attempts 01–08, halted plans and completed CI remain terminal; do not
restart them, create attempt-09 or import their recovery scaffolding. Phase 30
has its own new-baseline acceptance scope, which downstream planning must
specify concretely before runtime work.

RM-017's provider receipt, profile/scenario and LibFuturo installed-consumer,
NES-host and gameplay gates remain separate. The current [integration
context](../../../docs/coordination/milestone-context-2026-11-03.md) records
their ownership and stops. No receipt, performance-capture, retired APU route
or interrupted campaign allowance is renewed by this milestone.

Apply the [verification policy](../../VERIFICATION-POLICY.md) and [discussion
policy](../../DISCUSSION-POLICY.md). Keep useful one-time consistency probes
local. Add a dependency or maintained CI check/lane only for a demonstrated
need and recurring value; no such addition is selected. Each full workflow
keeps the 30-minute ceiling, the configured 1500-second gate and stricter
existing per-test/resource limits. A concrete plan must name any material
new authority or risk before execution rather than treating autonomy as a
waiver.

## Progress

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 30. MMC2 mainline compatibility and delivery | 0/TBD | Not started — discussion pending | - |

**Coverage:** All six v11 requirements map exactly once to Phase 30; no
orphaned or duplicate assignments. The [traceability table](REQUIREMENTS.md#traceability)
records those mappings. The phase remains unimplemented and unverified.
