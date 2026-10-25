# Phase 17: Complete pinned conformance baseline - Context

**Gathered:** 2026-10-23
**Status:** Ready for planning

The user delegated all decision areas: research relevant perspectives, challenge the alternatives, synthesize coherent recommendations and follow them automatically. Three focused advisors reviewed pin/inventory migration, bounded execution, and reporting/CI, then cross-reviewed the recommendations. These are agent-selected decisions under that delegation. This discussion ran no new-ROM conformance campaign and implemented no behavior.

<domain>
## Phase Boundary

Deliver CONF-01 through CONF-04: a complete immutable AccuracyCoin inventory, an honest bounded NTSC observation, retained old-pin comparison and protection against same-pin passing-result regressions. Inventory means 146 scored tests and five separate DRAW entries at the approved pin; it does not mean every test passes or even reaches execution.

Use the existing library and headless runner. Keep the three deliverables, clean-room rules, deterministic integer-only core, dependency limit and normal phase-branch/PR delivery. This step captures context and stops before planning. No accuracy repair cluster, new hardware support, lifecycle fuzzing, host ABI or additional administrative evidence system is introduced.
</domain>

<decisions>
## Implementation Decisions

### Pin, inventory and protected-key migration

- **D-01:** Move the ROM pin, manifest hash/licence, compressed-name decoder, inventory, affected ROM-specific expectations and protected-key migration together in one reviewable change. The approved new pin is `6bee919529289c555803b612b8c9e4f7eaaeeabb`; the retained old pin is `673ef550db296136d52229961e7d39366116882a`. Independently check exact selected bytes/licence during implementation. Research streamed the new upstream asset to a digest without storing it: SHA-256 `8ac8904a3cf26fbdfa6be501e414d45886ff4478d30dee8e58fa436cf5fd5adc`. The old manifest/local fixture hash is `4fe8c2bc9abc6f4d418da47b73f62cba89fcacd950fae763097a1681d650e839`. Research observations are inputs to validation, not a substitute for it. Do not follow moving upstream HEAD.
- **D-02:** Derive the full directory from the pinned ROM rather than maintaining another address/name list. Decode both page titles and row names, including the new F0–F4 tokens, with bounded input scans and expanded output. Reject unsupported encodings, truncation, empty/oversized names, invalid pointers/fields and ambiguous or duplicate scored identities. Validate all 22 pages and the complete expected inventory, not just a matching total. Exactly five DRAW entries use the shared `$03FF` sentinel; their intentional shared address must not be rejected as duplicate scored results. Validate scored result addresses and routine pointers against the selected ROM's documented layout. Never truncate a name into a colliding key.
- **D-03:** Keep readable `accuracycoin/<decoded name>` keys scoped by pin identity. Preserve source order/page/row/address information for navigation and diagnostics; sort only the report view where useful. A result address or table ordinal is not a stable identity across pins. At these two pins all 17 protected labels occur once and retain their named result/routine symbols, so an opaque new ID registry is unnecessary. If future data makes names ambiguous, fail explicitly and review that migration rather than inventing suffixes silently.
- **D-04:** Retain the old pin's 17 passing rows, raw codes, counts and provenance as comparison evidence; record the 144-scored-plus-five-DRAW old inventory separately from the new 146-plus-five. Account explicitly for every protected key in the migration. Label continuity is not semantic equivalence: upstream APU paths, including frame-counter synchronization, changed despite retained labels. Review relevant assertion/routine changes and record unchanged, renamed, changed-meaning or removed dispositions with sources and new observations. Use ordinary reviewed migration data in the phase PR, not a bespoke approval mechanism. Cross-pin totals or old raw bytes never silently establish a new passing baseline, and changed meanings do not authorize unnamed hardware repairs.

### Bounded execution and classification

- **D-05:** Prefer independent cold-loaded instances for all 22 page attempts, using the ROM's normal page-header A control. This prevents one page's hang from hiding every later page. Verify readiness, selected page/cursor and attributed start; release controls before controller-sensitive assertions. Preserve boot-time vblank/DMA pretests and startup observations. Source supports row selection and individual A execution; use that as a bounded fallback only where blocked page tails justify it and representative prerequisite/selection behavior has been qualified. Do not mandate an escalating run-all/page/test retry chain. No ROM patches, direct guest-RAM writes or test-specific saved starts to manufacture execution.
- **D-06:** Select finite boot/navigation/execution guest-frame caps from measured representative behavior, with a documented total campaign bound. Add an outer wall deadline for stalled host calls/processes using existing facilities. Record which bound stopped progress; wall time is a resource guard, not a performance or accuracy score. A page timeout must not prevent subsequent independent page attempts. An actual startup failure can leave that page's scored tests not-run with its cause; increasing caps until green is not the acceptance policy.
- **D-07:** Give every scored entry exactly one final `pass`, `fail`, `skip`, `not-run` or `timeout` classification with retained raw observations and cause. Decode `$FF` as skip before interpreting its low running bits; `$00` is not-run; low bits `01` and `10` represent pass and fail. A running marker is not terminal completion: distinguish an observed active attempt exhausting its bound from untouched later rows. Preserve protocol-valid upper variant bits (for example the existing `$41` pass); do not turn every nonzero upper code into failure or accept malformed results as variants. Validate status/raw-byte consistency against the pinned protocol without inventing a second hardware oracle.
- **D-08:** Inventory completeness, attempted execution and trustworthy terminal observations are separate claims. Retain each attempt and its origin when qualified fallback adds observations; do not cherry-pick a successful retry to hide contradictory results. An outer watchdog kill or process crash cannot supply unseen per-test raw bytes, a falsely completed report or an invented active-test identity. Preserve available diagnostics, mark unobserved rows with an honest cause, and fail the harness/infrastructure verdict. Do not call those bytes observed zeros. Valid runner-emitted guest timeouts and their downstream not-run rows remain baseline data.

### Reports and regression gates

- **D-09:** Extend the existing small owned text/TSV reporting path. Bind reports to ROM commit and full hash, inventory/schema version, tested source/behavior identity, run mode, input/start policy and budgets. Keep per-entry key, page/row, status, raw byte when observed, consumed frames and cause/attempt linkage; keep elapsed host time as diagnostic metadata. Prefer a small metadata sidecar over repeated large fields or a new serialization dependency. Exact field layout/module split is planning discretion. Store paths relative or sanitized. Retain raw result snapshots and first-failure/menu/start diagnostics through the existing build/CI artifact path; commit a compact canonical baseline and migration comparison so ordinary artifact expiry does not erase the result.
- **D-10:** The human summary reports `pass/146`, all five mutually exclusive status counts summing to 146, attempted/observed coverage and five separately listed DRAW entries. DRAW is never scored, and skip/not-run/timeout never shrink the denominator or imply a passing observation. Do not label this a cross-pin accuracy ranking, percentage of games playable or hardware proof. Retain the old-pin summary alongside it with its own denominator.
- **D-11:** Separate observation mode from strict regression enforcement. Successfully producing a complete, validated, honest baseline can exit 0 with known fail/skip/not-run/guest-timeout rows; no all-pass gate is added. Malformed inventory/results, untrusted/missing artifacts, output errors, crashes and outer-watchdog failures exit as failures. Protected regressions also fail. Keep the existing strict page command's documented failure behavior unless an explicit tested/documented interface migration replaces it. Reuse current `RUNNER_EXIT_TABLE` conventions (0 done, 1 failure, 2 usage); do not repurpose reserved 3/77 or change unrelated runner modes.
- **D-12:** Preserve CI's protected-main baseline authority. Fetch the prior scoreboard and its pin metadata from the same protected revision; do not let a candidate-only snapshot or pin-field edit authorize a reset. Same-pin prior protected passes must remain mapped and measured passing: fail a missing row, fail, skip, not-run or timeout for those keys, even if the candidate committed file still says pass. Keep raw-code changes visible; accept variant changes only under the pinned protocol with their meaning explained, rather than requiring raw equality for every valid pass. A real pin/meaning/denominator transition uses the explicit reviewed migration from D-04, never a blanket cross-pin bypass. Refresh the six-row local fallback to account for all 17 current protected passes. Establish new protected passes from actual measured evidence through the same ordinary reviewed scoreboard workflow.

### CI cost, verification and bounded scope

- **D-13:** Always put parser/classifier/inventory checks and actual protected-pass execution in normal `cmake --workflow --preset ci`, retaining existing six-platform parity, hygiene, sanitizer and perf coverage. Prefer one complete required PR run if its measured cost fits; do not automatically multiply all 22 attempts across six lanes. If even one full PR run is too expensive under existing measured-budget policy, retain the actual initial full bounded campaign and automate recurring pinned full observation in the existing nightly lane. Record that choice and the resulting detection delay. In either placement, phase acceptance needs a real complete inventory/report with diagnostics; an unexecuted scheduled configuration is insufficient. No network fetch is added to normal committed-fixture CI.
- **D-14:** Use focused discriminating controls: missing/duplicate/substituted inventory rows, malformed/unknown tokens and pointers, truncated tables, decoded-key collisions, DRAW entering the denominator, `$FF` versus running, valid pass variants, fake completion/wrong selection, held input, active timeout versus untouched tail, missing raw evidence, outer kill, missing/unfetchable baseline, same-pin lost PASS and an undisposed pin transition. Exercise production parser/classifier/verdict paths; require the intended rejection reason, not an unrelated setup crash. Reuse plain C tests, `tests/check.h` and CTest/CMake seams. Do not replicate every fault across every page or build a generic simulator.
- **D-15:** Pair behavior changes with project-required tests, README and public-header comment updates in the same change. Correct superseded preparation pin/encoding/status descriptions with clear historical/current distinctions. If ROM-bound hashes move because the fixture changed, retain old identities and explain the new expected data; do not erase unrelated determinism/state/parity checks. No new runtime or test dependency is approved. Hardware failures discovered by this baseline become named future findings, not an implicit repair obligation. Diagnose concrete harness defects within the authorized phase; no extra proof-transport framework, global GSD/config change or repeated administrative planning loop.

### Agent discretion and remaining measurements

Exact CLI spelling for inventory/observation, bounded helper/module boundaries, report schema details, permitted token expansion mechanics, cap values, artifact retention settings and CI placement remain implementation choices within these decisions. Research must verify the new ROM with the project decoder, review protected APU semantics, qualify controller/start behavior and measure runtime before locking budgets. A per-test fallback needs evidence of valid startup prerequisites before its results can serve as acceptance. No new-ROM score, campaign duration, full platform comparison or migrated protected outcome was measured during this discussion.

No Phase 17 SPEC, prior context, plans, checkpoint or blocking continuation file existed at initialization; no pending todos matched. Prior project preferences and Phase 14/15/16 contracts were carried forward rather than reopened.
</decisions>

<canonical_refs>
## Canonical References

Downstream agents must read the relevant sections before planning or implementing:

- `AGENTS.md`, `.planning/PROJECT.md`, `.planning/REQUIREMENTS.md`, `.planning/ROADMAP.md` — ownership, deliverables, CONF-01–04 and approved scope.
- `.planning/preparation/README.md` — source tiers and reference routing.
- `.planning/preparation/CONFORMANCE.md` — RAM protocol, directory, fixtures and scoreboard; historical old pin/status proposal is superseded here and by current requirements.
- `.planning/research/v4-scope/accuracy.md` — approved new pin, compression and migration evidence; preliminary milestone alternatives are not additional scope.
- `.planning/preparation/LIBRETRO-AND-RUNNER.md` §5, `.planning/milestones/v3-phases/14-runner-process-contract/14-CONTEXT.md` — process contract; current `runner/exit_status.h` and README supersede historical proposed statuses.
- `.planning/preparation/ENGINEERING.md` §§2–5, `.planning/milestones/v3-phases/15-speed-and-close-out/15-CONTEXT.md` — CTest/presets, measured CI budgets and retained perf/parity gates.
- `.planning/phases/16-host-evidence-and-roadmap/16-CONTEXT.md` — carried-forward fixture/host and roadmap boundaries. Its retired administrative mechanics are historical; do not import them into this phase.

Primary external test-suite references (MIT test source, not reference-emulator implementation):

- [New pinned README](https://github.com/100thCoin/AccuracyCoin/blob/6bee919529289c555803b612b8c9e4f7eaaeeabb/README.md), [assembly](https://github.com/100thCoin/AccuracyCoin/blob/6bee919529289c555803b612b8c9e4f7eaaeeabb/AccuracyCoin.asm), [licence](https://github.com/100thCoin/AccuracyCoin/blob/6bee919529289c555803b612b8c9e4f7eaaeeabb/LICENSE) — counts, token table, menu controls, boot prerequisites and result protocol.
- [Old pinned README](https://github.com/100thCoin/AccuracyCoin/blob/673ef550db296136d52229961e7d39366116882a/README.md), [assembly](https://github.com/100thCoin/AccuracyCoin/blob/673ef550db296136d52229961e7d39366116882a/AccuracyCoin.asm), [old-to-new comparison](https://github.com/100thCoin/AccuracyCoin/compare/673ef550db296136d52229961e7d39366116882a...6bee919529289c555803b612b8c9e4f7eaaeeabb) — retained comparison and semantic dispositions.
</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets

- `runner/main.c` — current directory parser, RAM observation/frame helpers and page controller driver. It accepts only pages 2/14/17, copies compressed bytes without decoding, limits storage to 32 rows per page and treats required nonpasses as command failure. Extend/extract its relevant helpers without changing ordinary run modes.
- `runner/sha256.c`, `runner/clock.c`, `runner/exit_status.h` — existing digest, runner-only timing and process-status facilities. The core remains independent of host timing/I/O.
- `tests/accuracy/scoreboard.txt` — 17 current passing keys; `tests/accuracy/scoreboard-main.txt` — six-row local fallback, not the complete protected set.
- `tests/accuracy/test_scoreboard.c`, `tests/cmake/prepare_scoreboard_baseline.cmake` — sorted text validation and protected-main fetch; presently lack pin-bound comparison. CI baseline fetch occurs before candidate-controlled CTest configuration.
- `tests/roms/manifest.txt`, `tests/cmake/manifest_sha256.cmake`, `tests/check.h` — permitted fixture/hash admission and owned test assertions.

### Established Patterns and Integration Points

- `tests/CMakeLists.txt` registers separate AccuracyCoin pages and scoreboard mutation checks. Add recurring checks to the same CTest workflow and retain current coverage.
- `.github/workflows/ci.yml`, `.github/workflows/nightly.yml` already prepare protected baseline data and retain ordinary CI results. Reuse them; choose full-observation tier from measurements.
- `README.md`, `include/nesturbator.h`, `runner/main.c` help/header comments and ROM-specific hash expectations must agree with the final behavior. The public library ABI and state format need no change for a runner-only baseline.
</code_context>

<specifics>
## Specific Ideas

Prefer a small owned helper or justified local duplication over another dependency. Relevant lenses were test-suite semantics and hardware evidence, C/API portability, reproducibility, CI/release cost, provenance/security, maintainability and honest consumer-facing claims. GUI/graphics-framework or distributed-system comparisons add no value to this headless measurement phase. Source evidence supports the recommendations; runtime qualification remains implementation work.
</specifics>

<deferred>
## Deferred Ideas

- Named NTSC hardware repair clusters selected from the baseline; no all-pass obligation.
- Accepted-program execution/lifecycle fuzzing and JAM policy work remain Phase 18.
- PAL/Dendy, new boards, expansion audio, FDS, external leaderboard submission and reference-emulator integration remain separate scope.
- Existing nightly vectors-full failure remains separate unresolved debt; this discussion neither repairs nor waives it.
</deferred>

---
*Phase: 17-complete-pinned-conformance-baseline*
*Context gathered: 2026-10-23*
