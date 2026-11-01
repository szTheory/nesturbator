# Phase 20: NMI line and VBlank flag - Discussion Log

> **Audit trail only.** Do not use as input to planning, research or execution
> agents. Decisions are captured in CONTEXT.md; this log preserves alternatives.

**Date:** 2026-10-26
**Phase:** 20-nmi-line-and-vblank-flag
**Areas discussed:** Race-test evidence, CPU/PPU alignment, save-state race
coverage, change attribution.

## Owner direction and decision method

The initial selection offered four areas. The owner chose all, asking for
breadth and depth through relevant stakeholder roles, reputable primary
sources, pros/cons, footguns and an adversarial pass, then coherent
recommendations adopted automatically. The owner repeated the preference
for small, flat dependency trees: "another copy and paste is better than
another dep". This authorized synthesis and adoption rather than another
four-question interview per area. No individual recommendation below is
misrepresented as a separately answered user question.

Three read-only specialist reviews examined race evidence, alignment/event
ordering, and save-state feasibility. The orchestrator examined regression
provenance and delivery, challenged categorical conclusions, and reconciled
the recommendations. No emulation implementation or runtime conformance
probe was performed in this discussion.

## Race-test evidence

| Option | Pros | Cons / failure mode | Decision |
|--------|------|---------------------|----------|
| Direct PPU helper tests only | Small, precise flag-transition diagnostics | Bypasses M2 phases, real decoding and instruction polling | Useful first layer only |
| AccuracyCoin only | Independent pinned integration target | Error codes hide bad offsets; permitted alignment shifts obscure some defects | Required integration layer, insufficient alone |
| Dot tables plus bus/program reproducers plus campaign | Independent expectations, precise localization, end-to-end coverage | More fixture work; duplicated or self-derived assertions must be avoided | Adopted |

**Adopted:** D-01 through D-07. Test both `$2002` subtests, both VBlank
boundaries, flag versus NMI suppression, enable/disable/re-enable, unchanged
enable rewrites, and NMI Control's STA/RMW distinction. Pin source-derived
expectations with named sample coordinates. Reuse owned fixtures. Separate
actual failing reproducers from passing guards. Select controls for collapsed
sample timing, off-by-one flags, erased captured edges, missing RMW writes
and early handler entry.

**Role synthesis:** Hardware accuracy requires explicit edge coordinates;
test architecture requires an external oracle and diagnostic layers; CI
requires bounded cases; API stewardship requires public-path evidence;
security/provenance requires owned code and licensed test inputs. Adding a
generic property/mutation framework would not improve the specific oracle.

**Source corrections:** Pinned assembly explicitly distinguishes VBlank's
M2-rise sample from sprite flags' M2-fall sample. The NMI-at-end test uses
its actual two sequences, not the earlier research's claimed PLA-versus-NOP
comparison. AccuracyCoin defines test intent; it does not certify every
answer against measured hardware. Source-tolerated entries and shifts must
be retained exactly, not generalized into arbitrary tolerance.

## CPU/PPU alignment

| Option | Pros | Cons / failure mode | Decision |
|--------|------|---------------------|----------|
| Add a global alignment offset first | Small patch; fixed value is deterministic | Cannot express two sample times; broad hash drift can mask wrong ordering | Conditional only after a residual phase defect is demonstrated |
| Keep two-edge cycle skeleton with explicit event samples | Reuses timeline and preserves cycle accounting | Same-edge set/clear/sample ordering needs source-backed probes | Adopted starting approach |
| Generalize all register scheduling or rewrite CPU per cycle | Broad expressive power | Wider mapper/APU/state/performance surface and scope growth | No speculative rewrite; minimum local staging only if demonstrated necessary |

**Adopted:** D-08 through D-13. Separate physical line, detector history,
pending edge and instruction decision. Keep split status sampling distinct
from CPU NMI sampling. Preserve a captured request after the line deasserts;
suppress a pulse that never reaches the detector. Research the documented
M2-fall candidate and same-edge ordering before fixing details in code.

**Role synthesis:** CPU/PPU engineering needs one causally ordered timeline;
determinism forbids random or per-title alignment; performance argues for
small staging within the existing bus cycle; maintainability favors explicit
state over unrelated latches; host consumers should not acquire a new
alignment setting. Legal alternate whole-table alignments do not justify
per-entry expected-value choices.

**Open evidence:** Current startup/reset phase against complete pinned
tables; ordering when read-clear, flag transition and detector coincide.
The existing research contradicts itself on rise/fall sampling and a
mandatory phase constant. Those details were not silently adopted.

## Save-state race coverage

| Option | Pros | Cons / failure mode | Decision |
|--------|------|---------------------|----------|
| Frame save/load replay only | Matches public API and existing host matrix | Does not establish an exact-dot snapshot; two paths can share a core bug | Required public evidence, not sufficient by itself |
| Internal valid cutpoint state-forks plus exact race tests and public replay | Exercises production serialization and historical latches without a new API | Cutpoint reachability and resumability must be demonstrated | Adopted |
| Arbitrary-cycle pause/resume API and CPU state machine | Could expose any desired instant | Requires serialized in-flight execution and broadens product/API scope | Outside Phase 20 |

**Adopted:** D-14 through D-19. Keep format 2 and revision 8, serialize
history that cannot be derived, validate legal transition states, reject
format 1 without mutation, and compare restored machine/guest results with
uninterrupted continuation. Reuse poisoned loads and wrong-host controls,
adding a focused omitted-history control. Preserve board diversity and
instance independence without repeating identical PPU sweeps on every board.

**Role synthesis:** API stewardship requires honest frame-boundary claims;
emulation accuracy requires exact flag/edge proof; security requires complete
validation before apply and unchanged-state rejection; test architecture
requires meaningful restore controls; CI cost favors a small NROM race
fixture alongside the existing board/host matrix.

**Feasibility guard:** The public API has no arbitrary-dot or mid-instruction
yield. A byte snapshot taken while a C instruction call is in flight is not
automatically resumable. Exact-dot internal cutpoints must be justified;
nearby frame saves cannot be renamed exact-dot saves. Any actual remaining
acceptance mismatch must be recorded before claiming KEEP-03, not hidden
behind a new debugger API or an unannounced weakening of the roadmap.

**Adversarial correction:** The first specialist conclusion that exact-dot
instruction-boundary snapshots were impossible was challenged and withdrawn.
CPU cycles span three PPU dots, but scanlines contain 341 dots and render-off
frames 89342, so event alignment changes across frames. From `(261,0)`, set
ordinal 261207 and clear ordinal 89343 are divisible by three. Selected event
dots can therefore be reached at completed instruction boundaries if the guest
schedule accounts for reset and instruction lengths. This arithmetic is a
candidate for the owned fixture, not measured execution evidence. A frame
return may also coincide with a suitable clear event; verify it rather than
assuming. Private stepping must maintain synthesizer/sample history before
claiming audio continuation. The recommendation remains valid internal
cutpoint proof plus separate public frame replay, without API expansion.

## Change attribution and CI

| Option | Pros | Cons / failure mode | Decision |
|--------|------|---------------------|----------|
| One large behavior diff and blanket final re-pin | Less intermediate bookkeeping | Cannot distinguish ordering fixes from regressions | Rejected |
| One PR with causal green commits and paired tests/evidence | Reviewable source of each moved result; preserves delivery contract | Requires deliberate reproducers and provenance refreshes | Adopted |
| Separate phase PR per internal mechanism | Independent release points | Unnecessary lifecycle overhead for coupled latches/polls | Not required |

**Adopted:** D-20 through D-24. Protect all 108 current PASS rows rather
than only the v5 opening's 101. Use focused tests for iteration and full
campaigns for causal/final evidence. Preserve existing six-platform checks,
caps, pin, provenance, sanitizer/performance rules and CI placement. Explain
each observed frame/audio/state hash move. Keep the dirty local baseline
qualified until actual clean hosted evidence is available.

**Role synthesis:** Product needs the six named outcomes and preserved play;
review/release needs attributable results and coherent state/version changes;
DevOps needs bounded runs and truthful source identity; audio/rendering needs
causal explanations for changed hashes; security/ownership needs no new
dependency or unlicensed input. A full campaign on every lane or iteration
would add cost without a distinct oracle.

## Agent discretion

Helper/field names, exact local staging, fixture details, diagnostic format,
bounded sweep size and commit subdivision may be selected during planning.
Source contradictions, same-edge ordering, alignment and valid state
cutpoints are explicit research tasks, not guessed constants or claims of
already passing tests. No further independent decision justified expanding
the phase into UI, 3D, other languages, a scheduler or a test-tool project.

## Deferred ideas

- IRQ I-flag latency and BRK/IRQ hijack remain Phase 21.
- Proposed owned-property helper/shrinking, runner-signal work and nightly
  mutation-tool pilot remain the three existing pending todos.
- DMA/OAM/open-bus clusters, PAL, extra chip profiles, arbitrary-cycle debug
  APIs and fetch-only blargg lane expansion remain separate scope.
- No planning or execution lifecycle command was invoked by this discussion.

## Sources

The full canonical reference list is in `20-CONTEXT.md`. Specialist online
checks used the exact pinned AccuracyCoin assembly/README (test-intent
primary sources), NESdev's hardware explanations, Visual6502-derived
transistor analysis and official Libretro run-ahead guidance. Repository
code and the current scoreboard were read directly. No GPL/LGPL emulator
source, new ROM or external implementation was opened or incorporated.
