# nesturbator

## Workflow handoffs

- For verification, CI monitoring, shipping and release work, read and apply
  [verification and CI/CD policy](.planning/VERIFICATION-POLICY.md). Resolve
  technical checks through useful automation or bounded agent review before
  considering human UAT; use CI only where recurring detection earns its cost.
  Evaluate available CI/CD timings for safe iteration/cost improvements.

- At a GSD stopping point, end with one exact, copyable next OpenGSD command
  and a plain-language sentence explaining what it does. State whether the
  user can clear context before running it.
- Before handing off, update `.planning/STATE.md` with the actual outcome and
  next command. When a fresh session needs additional context, link a concise
  resume file there. Read STATE and its resume file before resuming work.
- Before every `$gsd-plan-phase N`, always run `$gsd-discuss-phase N` first,
  including the first phase of a new milestone. Discussion captures phase
  decisions in context for planning; do not skip directly to planning.
- Keep detailed evidence in linked artifacts. Continue authorized work within
  the current step; stop at the required phase or milestone boundary.
- Always state the recommendation explicitly before presenting alternatives
  or asking the owner to choose, and say when you are applying it.
- Release PRs may be merged and published automatically once all required CI
  checks on the current head are green. At each milestone closeout with a
  release-worthy change, the owner authorizes the normal release path: triage
  related PRs, push/open the required PR, verify its required CI and main CI,
  merge when green, publish the package, and verify its artifacts. Check that
  project-owned worktrees are clean before release; preserve unrelated dirty
  work and resolve its ownership instead of discarding it. Record the released
  version and exact source identity. This authority does not restart an
  interrupted CI or campaign run; keep that stop until separately authorized.

A NES emulator core in C. This repository ships three things: a C library, a
headless command-line runner, and a libretro adapter. RetroArch or another host
provides the window.

Project state lives in `.planning/` (PROJECT.md, REQUIREMENTS.md, ROADMAP.md,
STATE.md). Reference material with sources is indexed in
`.planning/preparation/README.md`; open the file it points to before designing
or building a subsystem.

## Rules

1. **Own code.** All emulation and test code is written here. `libretro.h` is
   the only vendored file. The core library links only the C memory functions.
2. **Three deliverables.** Library, runner, libretro adapter. No window,
   audio-device or input-device code.
3. **Clean tree.** No ROM or BIOS bytes except test ROMs listed in
   `tests/roms/manifest.txt`. No personal paths, email addresses or names.
   Commits use the GitHub noreply identity. `scripts/hygiene.sh` checks this on
   every commit and push.
4. **Clean room.** Implement from hardware documentation and from test
   behaviour. Do not open GPL or LGPL emulator source, and keep no copy of it in
   the workspace; reference emulators run only as released binaries.
5. **Deterministic core.** Integer-only. All state lives in the instance. The
   same inputs give the same frame and audio hashes on every platform.
6. **Tested and documented together.** Every behaviour change lands with a test
   run by `cmake --workflow --preset ci`, and the README and the public header's
   comments are updated in the same change. Checks are automated; none waits on
   a person.
7. **One step at a time.** Run one GSD step per command, then stop and report
   what finished and what comes next. Inside a step, proceed without asking for
   routine confirmations. Work on a phase branch; merge by pull request with a
   Conventional Commit title.
   After milestone setup, the next phase command is `$gsd-discuss-phase N`,
   followed by `$gsd-plan-phase N` after discussion completes.

8. **Rolling direction.** Before each new-milestone and complete-milestone
   step, read `.planning/PRODUCT-ROADMAP.md` alongside project state, the
   latest audit, seeds and release evidence. Within that same step, reconcile
   delivered and partial outcomes, preserve unresolved debt, refresh all four
   horizons and promotion triggers, and record why ordering changed or stayed.
   Also check LibFuturo's current roadmap, provider contract and consumer
   evidence against Nesturbator's released capabilities. Prefer a current
   checkout or a fresh owner-supplied report; never treat an older sibling plan
   or profile as the current target. If LibFuturo has no high-value
   Nesturbator gap, continue general emulator work and weave consumer checks
   into relevant mapper, accuracy or region milestones instead of inventing a
   standalone migration milestone. Record what changed and what remains
   consumer-owned in the product roadmap and
   [integration context](docs/coordination/milestone-context-2026-11-03.md).
   Present the preferred next scope and important alternatives. Commit the
   refreshed product roadmap with the step's artifacts. Only approved scope
   is Immediate/committed; future direction remains proposed. At closeout,
   archive the agreed direction with the milestone. Do not run another
   lifecycle command to perform this upkeep.

## Style

Small modules, plain control flow, fixed-width integer types. A hardware comment
says what the hardware does and cites its source. To decide how to do something,
follow the ladder at ponytail.dev: remove the need, reuse what is here, use the
standard library, use the platform, and only then write the minimum new code.

Prefer a small owned helper or justified local duplication to another
dependency. Add a dependency only when existing code, the standard library
and the platform cannot handle a demonstrated need simply; keep its tree
small and its abstraction local. The ownership and clean-room rules apply.

## Personal playtesting

At milestone boundaries or before personal host setup, read
[playtesting lessons](.planning/playtesting/LESSONS.md). Steam Deck with ES-DE
and RetroArch is a useful daily-feedback route before deeper Playstead work.
Use existing library entries and persistent save locations for personal play.
A test collection should reference those entries, with per-game core selection,
so the same games and progress remain usable for daily play. Preserve other
cores' incompatible state files in their normal core-specific storage. Record
brief user observations as scoped smoke evidence; higher-confidence device
and performance testing can follow when the owner wants it.
Before recommending play, read the
[scene-first guidance](.planning/playtesting/LESSONS.md#scene-first-checkpoint-lessons).
Give the scene, known original quirks, visible expected/failure behavior,
verified access method and likely effort; mark uncertain comparisons. Record title and exact-scene results separately as dated evidence
alongside CI; verify current cheat/state support before proposing shortcuts.
Before converting gameplay findings into CI, read the
[regression direction](.planning/playtesting/AUTOMATION.md). Prefer small owned
fixtures with independent oracles and preserve unique regression coverage.
Keep connection details, identifiers, personal paths and raw private artifacts
outside Git; record only sanitized lessons and scoped observations.

## Standing engineering authority

Task approval includes required local verification, fixes and bounded retests.
Apply an approved compatibility policy without asking again for each saved
field or version bump. Runtime alone is not an approval gate. Preserve explicit
stops, limits and no-retry rules; ask only for a materially new consequence.
The owner grants standing authority for local CMake configure, build, test and
workflow commands, including full CI and its configured acceptance campaigns;
do not ask for per-run approval. Plan-level gates that request separate owner
authorization for local CMake CI are satisfied by this standing authority.
Bound each full workflow to a 30-minute wall-clock limit, preserving stricter
per-test, fixture and resource caps already configured. After a failure,
diagnose it before rerunning; a necessary local retry within the same scope and
limit does not need another approval. The milestone-release authority above
covers its required hosted checks, pushes, PRs, merges and publication. Stop and
report if a run would exceed the limit or expand beyond that release scope.
