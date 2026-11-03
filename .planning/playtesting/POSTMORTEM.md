# NES playtesting checkpoint and mini postmortem

Recorded 2026-10-30 after the owner asked whether more play would still change
the plan. This closes discovery's broad sweep, not a release qualification or
an approved new milestone. Detailed observations remain in the
[baseline](2026-10-30-nes-baseline.md); reusable guidance starts in
[lessons](LESSONS.md).

## Enough signal to plan

Yes. Feedback covers 22 of 26 prepared Deck titles and all six supported mapper
IDs. Many sessions had good controls, audio and general graphics. Named positive
checks include Balloon Trip's fixed HUD/stars and Mario 3's first power-up.
This is a useful ordinary-play baseline alongside CI. It says nothing about
every board variant, later level, region or full-game completion.

More generic first-level successes now have low marginal value. Several games
already report similar scrolling disturbances and crowded-scene slowdown. The
next useful work is controlled reproduction and classification, rather than
asking the owner to repeat those observations indefinitely. No further play is
required to choose a milestone. The [optional queue](DECK-QUEUE.md) gives four
unreported titles with short, concrete probes; two are enough if still fun.

## What worked and what to change

| Experience | Takeaway for the next emulator |
|---|---|
| Steam Deck → ES-DE → RetroArch supported comfortable daily play | Start with the owner's actual frontend and collection. This supplied useful feedback before deeper Playstead integration. Keep per-game core selection switchable. |
| A reference-only collection reused normal games/progress | Avoid a second test library. Preserve existing variants and metadata; distinguish shared battery RAM from core-specific snapshots. |
| Mapper breadth followed by exact scenes found more useful questions | First cover a few representative hardware families, then give scene, symptom, access route and expected effort. Record generic success separately from reaching a hard scene. |
| Display, updated launcher and input routing caused real friction | Verify the actual production launch and every active controller port. Direct-launch success missed Steam Input routing; assigning P1 alone accidentally left the same device on P2. |
| Named scenes and private captures retained evidence | Record title/revision, action and repeatability immediately. Verify the currently running game: a requested Blaster Master capture actually found DuckTales after a switch. |
| Four bounded private input/checkpoint cases replay identically | Reuse deterministic recipes for repeated inspection. Matching current RGB/PCM establishes repeatability; independent expectations must establish correctness. |
| Source/reference cards clarified quirks | Explain documented original behavior before testing. Unknown-provenance YouTube footage is comparison evidence, not a hardware oracle. |
| Repeated relaunch/reset advice cost play time | Back up valuable progress and capture the reached scene first. Relaunch only when a changed host setting needs it; reset only when the hypothesis requires a fresh boot. |
| Scene access did not generalize | Passwords/Continue are useful, but verify their actual route. No Game Genie support exists here; three Shatterhand automation attempts never reached gameplay. Stop bounded attempts rather than building an autonomous player. |

The [Muhtari/active-core research](../research/milestone-v6-discovery/SYNTHESIS.md)
converged on the same priorities: truthful player docs, actual frontend input,
separate consumer acceptance and proportional CI. Transfer those mechanisms;
derive NES hardware tests locally. Small owned helpers and justified duplication
remain preferable to a shared framework or another dependency tree.

## What remains unresolved

Treat these as an investigation queue, not confirmed core defects:

- Jurassic Park's OCEAN wobble and Crystalis's moving world seam have documented
  timing-sensitive counterparts. They are strong native-frame/IRQ investigation
  candidates. Little Nemo and MM3 have separate reported internal lines/seams.
- Shatterhand's line drifted down and wrapped. Regular VSync on/adaptive off was
  verified on relaunch; disturbances remain and the exact before/after comparison
  is inconclusive. Separate native pixels from final display output first.
- The Marble timer's native close-up already has blue edging and a small hole
  in 4. Match digits, revision, scale and reference before assuming a missing
  bitplane. Cropping outer edges cannot establish internal rendering accuracy.
- Crowded-scene flicker and slowdown are distinct. The NES selects at most eight
  hardware sprites per scanline; a character may use several. Stable displayed
  FPS with slower game action supports a game-side explanation but does not
  prove hardware-correct timing. The MM3 port correction did not remove the
  reported slowdown. Shatterhand's brief music interruption is also unclassified.
- Real same-core auto-state loading was observed; a controlled manual host
  save/advance/restore and fresh battery write/relaunch remain separate checks.
  State format 3 rejects earlier formats; recorded behavior revision is not a
  general compatibility guarantee. An explicit upgrade/rollback policy is needed.

The [scene cards](SCENE-PLAYLIST.md) retain sources and reference limits.
[NESdev's tricky-game notes](https://www.nesdev.org/wiki/Tricky-to-emulate_games)
document the Jurassic Park/Crystalis timing traps;
[sprite evaluation](https://www.nesdev.org/wiki/PPU_sprite_evaluation) documents
the per-line limit from timed-register observations and transistor simulation.
These community hardware references do not classify our exact captures by
themselves.

## The efficient quality loop

1. Retain a short report and checkpoint/capture without disturbing daily saves.
2. Reproduce the exact scene with identified content/core and bounded inputs.
3. Separate core indices/RGB, adapter conversion, host presentation and original
   game behavior. Use released reference binaries or hardware; never inspect
   prohibited emulator source.
4. Reduce a confirmed hardware interaction to original program/pattern bytes.
   Reuse existing C fixtures and assert independently derived timing/pixels.
   Demonstrate that a deliberately wrong control fails.
5. Land the fix with that cheap regression, applicable continuation/parity
   checks and documentation. Keep commercial media/states private, outside CI.

The leading reduction candidate is the full CPU → MMC3 IRQ handler → scroll
write → visible row boundary, reusing existing split/mapper tests. It is a
hypothesis, not a diagnosis. See [automation](AUTOMATION.md) for coverage gaps.
Keep each unique regression invariant; retire duplicate exploration only after
equivalent detection is demonstrated. Do not remove a guard merely for passing
repeatedly, add a new framework, or repeat the full matrix after docs-only edits.

## Proposed milestone sequence

1. **v6 candidate: Player-ready NES and reproducible feedback (RM-014).** Short,
   truthful README; identifiable install/core; controls and save distinctions;
   documented supported-version/backup/rollback policy; a bounded scene-report
   recipe using existing runner/host facilities. Qualify only demonstrated helper
   gaps and preserve existing executable documentation tests. Optional overscan
   presentation belongs here if selected, with full-frame diagnostic output
   preserved. No mandatory new commercial sweep or new host architecture.
2. **Scene-driven accuracy (new RM-015), conditionally next.** Start with the
   already reported OCEAN/split and host-vs-native seam questions. Bound each
   investigation; fix independently confirmed core faults with small owned
   regressions. If findings are original behavior or host-only, record that
   result and skip a standalone accuracy milestone rather than inventing work.
3. **Remaining DMA accuracy cluster.** Preserve the existing five failing rows
   and three already-passing former candidates; repair collection prerequisites
   before new qualified evidence. No visual report currently establishes a DMA
   cause. This resumes first if scene triage yields no stronger core target.

Actual Playstead integration remains a separate consumer-owned milestone when
its production launch/input/exit/progress path can be accepted. PAL is the
regional alternative. Punch-Out!! supplies named MMC2 demand, but mapper 9 is
still proposed; MMC5, expansion audio, FDS and peripherals need independent
scope. See the [rolling roadmap](../PRODUCT-ROADMAP.md) for all four horizons,
alternatives, promotion triggers and preserved audit debt.

The research/recommendations are ready. The next actor is the owner choosing
milestone scope; no more gameplay is required. v5 stays archived and Immediate
stays empty until that choice. Continue with `$gsd-new-milestone`; context may
be cleared after this checkpoint.
