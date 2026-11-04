# Phase 27 source coverage and execution map

Planning date: 2026-11-02. No configure, build or test was run during planning.
Planning status: the named PAL model and independent output oracle are now
recorded in the updated research and Plan 02. No source item remains unplanned.
Source inspection used tracked implementation/test seams and the phase-specific
research. Discovery is satisfied by that current research plus direct verification
of the existing C17/CMake, getter, runner and real-module test patterns. No new
external library, service, public API or dependency is needed. Graph context is
disabled; the assumption-delta scan reported detected=false, so no checkpoint is
created. Security enforcement is disabled in project configuration; no STRIDE
register/package installation gate is required. Estimate calibration returned
factor 1 and low confidence with zero samples.

## Multi-source coverage audit

| Source | ID | Outcome or constraint | Plan/task | Status | Grounding |
|---|---|---|---|---|---|
| GOAL | Phase 27 | Consistent PAL selection, timing and modeled colors through three deliverables with protected NTSC | 01–04 | COVERED | ROADMAP Phase 27 goal and five success criteria |
| GOAL | Success 1 | Same effective region; library/runner expose exact timing | 01.1–2, 04.1–2 | COVERED | instance.c queries; runner load; host region matrix |
| GOAL | Success 2 | Loaded-game libretro region, exact-rate conversion and 48000 Hz | 01.1–2 | COVERED | libretro.c AV and region callbacks; real module lifecycle tests |
| GOAL | Success 3 | Named modeled PAL RGB and emphasis with bounded claims | 02.1, 03.1–2, 04.1 | COVERED | palette getter, owned palgen and shared converter |
| GOAL | Success 4 | Six-mapper NTSC frame/audio/palette/callback/regression preservation | 02.1–2, 04.2–3 | COVERED | fixed NTSC table/hash blobs and existing CI inventory |
| GOAL | Success 5 | Owned/permitted cross-deliverable deterministic regional output | 04.1–3 | COVERED | existing ines builder/process/module/PPM seams and unchanged permitted-ROM tests |
| REQ | VID-01 | Deterministic named modeled 2C07 palette and emphasis | 02–04 | COVERED | requirements frontmatter in both plans |
| REQ | AV-01 | Shared resolved profile and accurate host reporting | 01, 04 | COVERED | requirements frontmatter in both plans |
| REQ | NTSC-01 | Unchanged existing output and retained regression evidence | 01–04 | COVERED | requirements frontmatter in all plans |
| RESEARCH | Loaded profile | Resolve only in loader, query separate APIs, preserve info layout | 01.1–2 | COVERED | src/cartridge.c loader; src/instance.c; include/nesturbator.h |
| RESEARCH | CLI compatibility | Opt-in parseable record, exact fractions, contradictory-preference controls | 01.1–2 | COVERED | runner/main.c parser/load/output; tests/CMakeLists.txt exact stdout helper |
| RESEARCH | AV lifecycle | Load/reload/unload/no-content/refusal/reset and no runtime reconfiguration | 01.2 | COVERED | check_region_option_lifecycle and check_region_selection_matrix |
| RESEARCH | Conversion boundary | Canonical low-nine-bit native pixels and 256x240; region selected at getter | 02.1–2, 04.1 | COVERED | src/palette.c and host/convert.c; public native video contract |
| RESEARCH | PAL model | NESdev 2C07 wiki model identified with pally v0.24.1, exact recipe, phase/colorburst interpretation, PAL emphasis and official RGB oracle | 02.1, 03.1–2 | COVERED | Updated research records recipe, source facts, nine independent sample values and full 512-entry RGB digest; Plans 02/03 fix those expectations without vendoring or network requirements |
| RESEARCH | Exhaustive palette | All 64 entries times eight emphasis masks, repeatability, bounds and independent PAL sample expectations | 02.1–2, 03.1 | COVERED | tests/core/test_palette.c existing NTSC/copy seam |
| RESEARCH | Regeneration | Preserve default NTSC generation byte-for-byte; separate PAL regeneration | 03.1 | COVERED | palgen; tests/cmake/palette_regen.cmake; palette.regen |
| RESEARCH | Six mapper controls | IDs 0,1,2,3,4,7 under both profiles; library/runner/module native/audio/RGB agreement | 04.1–2 | COVERED | core/test_mapper.c regional fixture geometry; libretro_host.c process/module harness |
| RESEARCH | NTSC evidence | Preserve table bytes, representative RGB, frame/PCM/transition hashes and callback contracts | 01.2, 02.1–2, 04.2–3 | COVERED | existing NTSC assertions and runner.write_hashes.content retained |
| RESEARCH | Ownership and limits | Integer-only core, no dependencies/GPL source, no universal/hardware claims, exact local candidate receipt | All; 04.3 | COVERED | AGENTS and verification policy; model limits remain explicit |
| CONTEXT | D-01 | Runner --info reports active resolved region and exact rates | 01.1–2 | COVERED | D-01 cited in task actions |
| CONTEXT | D-02 | Existing stdout remains unchanged without opt-in | 01.1–2 | COVERED | exact-output comparisons and tests |
| CONTEXT | D-03 | Separate queries, no duplicate info field/layout change | 01.1–2, 02.1 | COVERED | explicit public API constraint |
| CONTEXT | D-04 | One loaded instance supplies AV/region, double at edge, no rounded PAL or runtime update | 01.1–2, 04.1 | COVERED | existing production path retained; lifecycle assertions expanded |
| CONTEXT | D-05 | Instance-selected modeled PAL table, owned generator, raw native/hash preservation, fixed NTSC bytes | 02.1, 03.1–2, 04.1 | COVERED | table/getter/model/conversion tasks |
| CONTEXT | D-06 | Preserve six-mapper NTSC outputs; no silent re-pin | All; 04.2–3 | COVERED | fixed pre-phase source identity and full local workflow |

Deferred/general property, signal and nightly mutation ideas are excluded, as
are Dendy, new boards, host integrations, hardware/calibrated color claims and
general collector changes. Prior-phase DMA/state features are preserved, not
reimplemented. The former interrupted campaign remains stopped and unknown.

## Dependencies, waves and ownership

| Plan | Needs | Creates or expands | Checkpoint | Wave |
|---|---|---|---|---|
| 27-01 | Phase 26 loaded effective-region API and current host matrix | Resolved CLI report, lifecycle regressions, reporting docs | none | 1 |
| 27-02 | 27-01 reporting/docs commits; existing palgen/getter seam | Complete owned PAL table, getter selection and initial sample/copy/lifecycle checks | none | 2 |
| 27-03 | 27-02 selected PAL table and owned generator | Exhaustive model oracle, both regeneration modes and public documentation | none | 3 |
| 27-04 | 27-03 exhaustive model/regeneration/docs, transitively 27-01/02 | Cross-deliverable six-mapper presentation checks and combined acceptance | none | 4 |

Plans share README/header/docs/tests/CMakeLists.txt and libretro host test
ownership, so they execute sequentially. Each task touches at most five files;
each plan has two or three tasks and leads with a real tracer. The implementation tracer stays end to end across generator, private table, getter and sample test; exhaustive checks and documentation are a separate sequential plan. No same-wave file
overlap exists. The loader and existing libretro production callbacks already
provide the architectural path and need no duplicated policy or new cache.

## Artifacts this phase produces

| Artifact/symbol | Owner | Reachable path |
|---|---|---|
| options.info and opt-in info record in runner/main.c | 27-01 | --info with normal frame/movie/bench execution |
| read_runner_info test helper, runtime_av_update_calls counter; expanded existing matrix/lifecycle checks | 27-01 | registered libretro.host |
| runner.info and runner.usage.info.* tests | 27-01 | tests/CMakeLists.txt CTest registration |
| src/palette_pal.inc; private static nesturbator__palette_pal[512] | 27-02 | src/palette.c include, PAL getter selection |
| palgen --pal; pal_signal_mv, pal_decode, pal_xrgb tool helpers | 27-02 | owned offline command and palette.regen.pal |
| PALETTE_MODE=pal; palette.regen.pal | 27-03 | tests/cmake/palette_regen.cmake and CTest registration |
| Expanded core.palette and host.convert assertions | 27-02/03/04 | existing CTest targets |
| build_presentation_image and check_regional_presentation_matrix | 27-04 | existing libretro.host main/harness |
| README, public-header comments, docs/runner.md, docs/api.md, docs/testing.md | 27-01/03/04 | existing canonical documentation links |
| 27-01/02/03/04-SUMMARY.md | each executor | standard execute-plan output |
| 27-ACCEPTANCE.md and 27-REVIEW-DISPOSITION.md | 27-04 | linked phase receipts and final verification |

No public API symbol, native machine state field, ABI layout, hardware profile,
runtime dependency or new shipped deliverable is created. Default NTSC source
blob is 26ff4966e7a4be2f1ebf0b30bec125ddbe11c87e; existing runner inventory
blob is c5736d543bff79da4a8ab995fb47decf042373ba. The pre-phase commit used by
the preservation command is ad24e05247f4ebe5cb7efb8212b632c2f8cf4f8f. That
read-only preservation check passes before implementation by design; it guards
an invariant, not feature completion. No grep/count gate is used as behavior
acceptance.

## Resolved palette source clarification

The updated phase research preserves the selected NESdev 2C07 wiki palette
model, pally v0.24.1, and records its exact official command, RC hue-shift
criterion, delay-line decoding, phase-7 colorburst and line-alternating V. The
source's complete output is 1536 bytes of RGB triples, with SHA-256
`cb052e4ef8c738c19c2304804e3851d5afd2ba8bf68ed810cabd69de0fe9e9e5`.
The research records nine independent first-group RGB sample values, copied as
expectations into Plan 02. The official output remains outside the repository;
implementation/tests use owned generation code, fixed sample constants and the
full-table digest, with no network fetch or pally dependency. The PAL mapping
is native PPUMASK bit 5 green, bit 6 red, bit 7 blue; native bits stay canonical.
The recorded model output is an independent software-model oracle, not a
hardware color capture or universal calibration claim. No model substitution
or new owner decision is required.

## Flagged spec-less edge assumptions

No SPEC/Edge Coverage section was supplied. These probe rows remain unresolved
and flagged-unverified. Focused tests cover concrete phase contracts but do not
silently classify, dismiss or resolve these generic probes.

| Requirement | Category | Probe | Status | Verification | Resolution | Reason |
|---|---|---|---|---|---|---|
| VID-01 | unclassified | unclassified — review manually | unresolved | null | null | null |
| AV-01 | unclassified | unclassified — review manually | unresolved | null | null | null |
| NTSC-01 | adjacency | When two things are exactly equal or just touch, do they merge, collide, or separate? | unresolved | null | null | null |
| NTSC-01 | empty | What is the result for empty, single-element, or null input? | unresolved | null | null | null |
| NTSC-01 | ordering | When elements compare equal, is output order specified and stable? | unresolved | null | null | null |

## Prohibition fallback: recall then precision

No supplied Prohibitions section or wired descriptor exists. Recall considered
incorrect preference reporting, rounded PAL rates, changed stdout, stale table
selection, lost emphasis, changed NTSC hashes, off-by-one copy bounds, global
state leakage, dependency additions, unsupported boards, misleading color or
hardware claims, overstated parity evidence, hidden regression re-pinning and
misrepresented historical campaign/authority.

Precision drops routine engineering/scope correctness items into the ordinary
task checks: rates, output syntax, selected table, native/copy bounds, shared
state and dependencies/boards. It keeps these bespoke transparency items as
descriptor-less, flagged-unverified prohibitions (status unresolved,
verification/resolution/reason null):

1. VID-01: MUST NOT present the named modeled PAL palette, its regeneration or
   cross-deliverable RGB equality as universally calibrated or direct hardware
   color proof.
2. AV-01: MUST NOT present matching exact regional reports or deterministic
   callback outputs as proof of physical host pacing, device behavior or PAL
   silicon correctness.
3. NTSC-01: MUST NOT hide unexplained NTSC output drift by silently re-pinning
   baselines, or represent the stopped historical campaign's unknown result as
   accepted evidence or permission to restart it.

Plan 04 review must retain these flags in its disposition and acceptance
records; passing local automation does not remove them. No check_* descriptor
is fabricated. Source-bound technical review can assess claims while preserving
the explicit evidence limit; no redundant owner acknowledgment is required.

## Execution limits and verification cost

Focused checks reuse existing targets and process/module seams. The only new
maintained registrations are cheap exact-output/parser cases and PAL table
regeneration, both recurring failure boundaries with independent expectations.
The final unfiltered `cmake --workflow --preset ci` is required during execution,
under the standing 30-minute outer limit and all tighter configured caps. A
failure must be diagnosed before a bounded necessary retry. Existing unique
fixture closure, NTSC inventory and callback/session coverage stays intact.
Review available timing evidence without adding cache/concurrency/dependencies
or general CI cleanup. Hosted actions, pushes, PRs, merges, releases, device
tests and stopped-campaign restart are outside this phase authority.
