# Phase 30: MMC2 mainline compatibility and delivery - Context

**Gathered:** 2026-11-19
**Status:** Ready for planning

<domain>
## Phase Boundary

Port the already selected NES 2.0 mapper 9/submapper 0 MMC2 profile from the
completed v10 implementation onto the revalidated live Nesturbator mainline.
Deliver it through the C library, headless runner and libretro adapter, with
explicit state compatibility, source-bound automated acceptance, and the
normal reviewed PR and release path. This phase does not qualify silicon,
commercial titles, a LibFuturo consumer or Playstead.

</domain>

<decisions>
## Implementation Decisions

### Mainline baseline and compatibility
- Revalidate live `main` before planning source edits. The 2026-11-19 reviewed
  head is `8923b66cca16739c5b402643798515cf5537ae22`, release 0.1.13, ABI 1,
  behavior revision 9 and state format 3. If live main moves, stop source work
  and derive the delta from its actual compatibility contract.
- For that exact baseline, advance behavior revision to 10 and state format to
  4 while retaining ABI 1. Refuse incompatible prior state formats. Do not copy
  v10's revision 18 or format 7.
- Port only necessary MMC2 implementation, tests, integration and compatibility
  changes. Do not merge or cherry-pick the v10 branch wholesale.

### Bounded MMC2 behavior and state
- Support only the selected NES 2.0 mapper 9/submapper 0 profile: PRG banking,
  mirroring, both CHR latches, and trigger/non-trigger PPU reads. Preserve the
  already adopted latch model as secondary-reference evidence, not silicon
  proof. Unsupported variants keep transactional refusal.
- Exercise actual pattern reads and owned direct markers through the library,
  runner and libretro adapter. Save/load must preserve all mutable selectors
  and both latches, rebuild derived maps, and continue deterministically.
- Preserve existing mapper behavior and protected outputs. Explain only
  necessary state-hash changes; keep AccuracyCoin inputs, historical baselines,
  campaign evidence and acceptance rules unchanged.

### Automated acceptance and protected evidence
- Before expensive acceptance, run a cheap source-bound preflight for the base
  identity, allowed source/test delta, registrations, version/state fields and
  protected evidence. Run focused controls first.
- Run the existing `cmake --workflow --preset ci` on the same candidate with
  the AccuracyCoin full campaign explicitly enabled, within the approved
  1500-second plan gate, 30-minute workflow ceiling and stricter configured
  check limits. Do not re-anchor historical AccuracyCoin metadata or pass
  mismatches by changing protected rules.
- Preserve the current target campaign's historical input manifest and report
  pins. If its source-closure checker must admit MMC2 files, make the smallest
  exact allowlist change and bind it to the current-candidate campaign
  fingerprint; do not weaken historic protection.

### Delivery and evidence claims
- Use a reviewed Conventional Commit PR. Require green exact-candidate checks
  before merge and green resulting-main checks before publication. Verify
  published contents, checksums and source identity.
- Keep the separate RM-017 publisher receipt and LibFuturo consumer/host/gameplay
  gates blocked on their own evidence. This MMC2 port does not satisfy them.
- Do not restart Phase 29 CI or terminal attempts, add attempt-09, import its
  recovery scaffolding, add a new dependency or CI lane, run a performance
  capture, expand hardware/title scope, or claim broad mapper-9, MMC4 or
  Punch-Out!! compatibility.

### the agent's Discretion
- Choose the smallest source changes and test decomposition that fit the
  existing C, mapper, state, runner, libretro and CMake patterns. Keep useful
  one-off checks local unless recurring independent value is demonstrated.
- Keep formatting, commit organization and routine implementation details
  consistent with repository conventions and the reviewed release path.

</decisions>

<canonical_refs>
## Canonical References

- `.planning/workstreams/mmc2/STATE.md` — selected current state, compatibility
  baseline, preserved stops and authorization.
- `.planning/workstreams/mmc2/V11-HANDOFF.md` — exact-main route, phase contract
  and fresh source-bound limits.
- `.planning/workstreams/mmc2/V10-HANDOFF.md` — completed MMC2 behavior, evidence
  tier and transfer audit; do not reuse its incompatible version values.
- `.planning/workstreams/mmc2/ROADMAP.md` and
  `.planning/workstreams/mmc2/REQUIREMENTS.md` — Phase 30 scope and acceptance.
- `.planning/VERIFICATION-POLICY.md` and
  `.planning/DISCUSSION-POLICY.md` — evidence, CI, release and re-entry rules.
- `.planning/preparation/README.md` — index to owned hardware references.
- `src/mapper.h`, `src/cartridge.c`, `src/ppu.c`, `src/state.h` and
  `src/state.c` — live mainline mapper dispatch, PPU hook, state layout and
  derived-map reconstruction patterns.
- `tests/CMakeLists.txt`, `tests/core/test_mapper.c`,
  `tests/core/test_state.c`, `tests/runner/hashes.txt`,
  `tests/libretro/libretro_host.c` and `tests/cmake/accuracycoin_input_fingerprint.cmake`
  — existing test registration, state/parity, output pins and protected source
  closure.
- `CMakeLists.txt`, `include/nesturbator.h`, `README.md` and
  `docs/provider-ntsc.json` — build integration and public compatibility
  documentation.
- Exact target commit:
  <https://github.com/szTheory/nesturbator/commit/8923b66cca16739c5b402643798515cf5537ae22>
- Current LibFuturo source tree:
  <https://github.com/szTheory/libfuturo/tree/fa1908a9831354cb325f9480be0559d7c5dca687>
- Current LibFuturo seam map:
  <https://github.com/szTheory/libfuturo/blob/fa1908a9831354cb325f9480be0559d7c5dca687/docs/nesturbator-seams.md>
- CMake 3.25 workflow and cache-variable reference:
  <https://cmake.org/cmake/help/v3.25/manual/cmake.1.html>

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `src/mapper.h` already defines mapper registers, bank-page tables, state
  hooks and the reserved watch-gated PPU-read hook for this board family.
- `src/cartridge.c` owns mapper selection and initialization; `src/ppu.c`
  owns pattern reads and can call a board hook only when the watch bit is set.
- Existing MMC1/MMC3 modules, mapper fixtures, state primitives, CTest
  registration and library/runner/libretro harnesses provide local patterns.
- Existing AccuracyCoin fingerprint and protection checks are the guards for
  source closure and immutable historical evidence.

### Established Patterns
- Mapper state contains fixed-width serializable registers only. Page tables
  are derived and rebuilt after loads. Hooks receive time/address explicitly
  and mutate mapper state only.
- Public compatibility is reported separately as ABI, behavior revision and
  state format. Serialized state format changes require refusal and hash-pin
  updates to be explicit.
- Tests use owned marker fixtures and stable output hashes; integration passes
  through all three shipped interfaces.

### Integration Points
- Add only the mapper and tests required by MMC2 to `src`, root `CMakeLists.txt`,
  and `tests`; route PPU pattern reads through the reserved hook without
  disturbing watch-gated behavior for existing boards.
- Update compatibility checks, state tests and only the affected expected
  hashes, then update the README and public-header comments in the same change.
- Extend the AccuracyCoin source-closure allowance narrowly if required, while
  retaining its historical metadata and report pins.

</code_context>

<specifics>
## Specific Ideas

The phase's observable proof is the selected marker's PRG/CHR/mirroring behavior
and latch-triggered versus non-triggered pattern reads, plus equal exercised
video/PCM results and save/load continuation across library, runner and
libretro. A software oracle supports the adopted latch model; hashes establish
repeatability and parity, not silicon correctness. The live LibFuturo README
still describes only its synthetic-v1 source proof, its seam map remains pinned
to revision 9/state format 3 with a create/destroy probe, and the fresh
2026-11-19 owner report still says the publisher receipt is missing.

</specifics>

<deferred>
## Deferred Ideas

Other mapper families or MMC2 variants, MMC4, commercial title claims, hardware
or silicon validation, performance work, and LibFuturo/Playstead consumer,
host, gameplay or receipt acceptance remain outside this phase. No owner
decision is unresolved within the approved Phase 30 scope.

</deferred>
