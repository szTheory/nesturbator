# Muhtari playtesting lessons for Nesturbator

Inspected: 2026-10-30. Read-only sibling inspection; no emulator, ROM/BIOS
inspection, host launch, permission change, hosted dispatch or retry performed.
Sources describe checked-out records, including local/unaccepted corrections;
their existence does not establish release or hosted acceptance. No emulator
implementation source was imported. Recommendations remain milestone proposals.

## What the owner asked Muhtari to achieve

The v3 research brief records a gamer-first shift: prove playing games through
Playstead, prioritize save/load, and inspect the actual frontend before assuming
its integration boundary. Casual successful play of two commercial titles was
recorded as observation, explicitly without a library-wide guarantee or recurring
owner UAT. [M1:3–13]

Phase 12 translates that intent into a short player README, a release-qualified
RetroArch launch contract, packaged-core visible/input/persistence evidence, and
a separate Playstead-owned integration handoff. The owner delegated routine
recommendations and requested bounded specialist breadth plus an adversarial
pass; this did not authorize automatic downstream phases. [M2:12–24; M3:28–108]

## The important evidence distinction

Muhtari's compatibility page has 201 unevaluated and three skipped pinned-list
entries, alongside owned diagnostic/title-screen evidence. Recognizing a title
or matching its startup screen does not prove gameplay. The casual commercial
reports can be useful without changing those automated ledger statuses. [M4:7–14,
28–69]

Current Phase 12 is **not delivered**. The real packaged-core held-input run
produced pixels matching idle input, despite local checks passing. Later
keyboard feasibility dispatches did not establish input: unrelated portable
controls and an Intel deadline failed before host execution. Local corrections
and deadline calibration are recorded, but native/host acceptance remains
outstanding, accepted plans remain zero, and the live allowance is exhausted.
STATE's headline predates appended summary/lesson records; read the newest
owning evidence rather than inferring completion from local commits. [M5:454–525;
M6:116–130]

## Transfer decisions

| Lesson and failure mode | Recommendation for Nesturbator | Tradeoff / limit |
|---|---|---|
| Local callback tests and screenshots do not establish actual delivered gameplay input. | For a new host-readiness claim, require an input-dependent visible transition through the actual host/core pair; distinguish missing input, wrong button/port and stuck input. | Keep exhaustive semantics in cheap native tests; one discriminating host case is sufficient if it detects the concrete fault. [M3:62–87; M6:123–127] |
| Equality can compare two equally wrong operands. Muhtari's earlier ORed fire predicate also could not distinguish buttons. | Use an owned fixture with independent expected input bits/pixels and explicit wrong-button/port controls; avoid deriving both sides from the same implementation. | A real commercial title remains valuable for player experience but is a weak deterministic oracle without a defined predicate. [M6:123] |
| A documented optional driver may be absent from the released executable. | Check capabilities of the exact packaged host before designing automation around a driver; Muhtari's JSON test-joypad path is not a ready-made reusable success. | Do not add an input framework or custom GPL host build just to rescue the design. [M6:125] |
| A held movie may release input at EOF before the screenshot. | Check the producer's complete observation window and actual bindings, including portable checker branches. | This is a risk to reproduce here, not evidence Nesturbator has the same bug. [M6:127,129] |
| A failing input launch did not retain its own log/config/schedule. | Save per-launch diagnostics on failure before investigating or rerunning; another launch's driver message cannot establish the failed launch's driver. | Keep raw commercial logs/captures local and sanitize summaries. [M6:124] |
| Source-built and packaged cores prove different claims. | Test the exact downloaded/extracted core for installation claims and retain host/core identity separately. | Nesturbator's existing host job builds and loads its build-tree module; preserve its valid source-host evidence without relabeling it as downloaded-release proof. [M3:57–68; N2:296–345] |
| A zero host exit does not establish loading or save flush. | Observe meaningful gameplay and graceful return; verify battery continuity through two fresh processes when the chosen game supports it. | Keep host exit semantics separate from the runner's documented exit table. [M3:39–56] |
| Permission, feasibility and accepted gameplay are separate. | If automation needs OS input, test availability under its actual execution identity first and use bounded process ownership; do not infer delivery from permission. | Existing exhausted budgets/stops remain; this research grants no host runtime authority. [M6:126; M5:454–468,515–525] |
| Repeated metadata repair/polling did not change acceptance. | Stop fan-out once a coherent recommendation and adversarial check exist; use one state writer and batch related local fixes before any authorized costly acceptance. | A bound should stop repetition, not waive unmet product behavior. [M6:116–122] |

## What Nesturbator already has

Do not import Muhtari's pending save-state milestone as new NES work.
Nesturbator already has save/load, rewind/run-ahead, real-host progression
controls, MMC1/MMC3 fresh-process battery tests, and owned libretro replay.
Its existing preparation instructions already call for integrated boot/control/
render/audio evidence and first-divergence diagnosis with private traces kept
local. The useful delta is the player's current installation/controls experience
and any demonstrated input or packaging gap, not another general host harness.
[N1:158–190; N2:321–405; N3:1870–2115]

The README itself reveals a real onboarding problem: it opens with a long v3
technical status, says behavior revision 8 in the overview, and puts player
installation after substantial internal detail. Muhtari's concise player-first
README is the transferable information architecture: install, load a game,
controls, limitations, then links for developers. This supports a separate docs
recommendation; it is not an emulator compatibility result. [N3:1–37,2024–2078;
M7:1–60]

## Recommended bounded NES spot-check

1. **Prepare one explicit test session.** User selects a small private folder and
   a few NES games for supported board families; start with a representative
   simple title and, if useful, one battery/MMC3 title. Keep Neo Geo/Glueyneo as a
   separate session. Record local content identity and board/region, exact core
   source/package and RetroArch app, relevant options and save directory. Do not
   run from this research or revive an old runtime window.
2. **Use the real player path.** Load the intended core, enter gameplay, exercise
   directions and both buttons, observe movement/scrolling, listen to audio,
   try pause/reset/quit and confirm orderly return. Start with defaults; use an
   isolated diagnostic profile when reproducing a fault so ambient overrides do
   not blur cause. Do not replace the player's requested exploratory play with
   an automation-only ceremony.
3. **Make feedback cheap.** Per title: booted; got into gameplay; controls;
   picture; sound/speed; save/load or battery if exercised; first bad scene/action;
   rough elapsed time; verdict good/problem/untested. Unknown stays unknown.
   The agent collects technical identity/logs. The owner supplies optional
   perception and hardware-specific feedback, not mandatory release sign-off.
4. **Turn each reproducible failure into engineering evidence.** Preserve the
   failed session, reproduce the shortest sequence, inspect a bounded first
   divergence using permitted hardware references, create the smallest owned
   regression and fix the owner module. Do not commit commercial ROM bytes,
   derived state/movie/trace/capture data or personal paths. Keep sanitized
   conclusions separate from broad compatibility claims. [N1:162–176; N4:rules3–6]
5. **Promote only demonstrated recurring gaps.** If actual host input or release
   installation is not already covered, add one bounded owned-fixture case with
   independent fault controls. Reuse C/CTest/CMake/POSIX tools; no shared sibling
   framework, new runtime dependency, blanket six-platform GUI matrix, ROM
   downloader or commercial fixture in CI. Existing deterministic checks remain
   automated. The session can reprioritize DMA, but does not itself fix those five
   failures or validate PAL.

Official host documentation confirms `-L CORE CONTENT`, explicit `--config`,
verbose logs and feature reporting. It distinguishes keyboard/mouse input
drivers from controller drivers; wrong-path diagnosis should respect that
boundary. [E1–E3]

## Sources

Sibling links are relative to this research file and intentionally contain no
personal paths. Line ranges above are inspection locations, not frozen evidence
identities; sibling work may advance during this research.

- **M1:** [Muhtari v3 brief](../../../../muhtari/.planning/research/v3.0-BRIEF.md).
- **M2:** [Phase 12 discussion](../../../../muhtari/.planning/phases/12-player-ready-delivery-and-playstead-bridge/12-DISCUSSION-LOG.md).
- **M3:** [Phase 12 context](../../../../muhtari/.planning/phases/12-player-ready-delivery-and-playstead-bridge/12-CONTEXT.md).
- **M4:** [Muhtari compatibility evidence](../../../../muhtari/docs/compatibility.md).
- **M5:** [Phase 12 latest appended execution summary](../../../../muhtari/.planning/phases/12-player-ready-delivery-and-playstead-bridge/12-01-SUMMARY.md).
- **M6:** [Muhtari reproduced lessons](../../../../muhtari/.planning/LESSONS.md).
- **M7:** [Muhtari player README](../../../../muhtari/README.md).
- **N1:** [Existing NES operating lessons](../../preparation/GSD-HANDOFF.md).
- **N2:** [Current NES hosted workflow](../../../.github/workflows/ci.yml).
- **N3:** [Current NES README](../../../README.md).
- **N4:** [NES project instructions](../../../AGENTS.md).
- **E1:** [Official RetroArch CLI guide](https://docs.libretro.com/guides/cli-intro/), read 2026-10-30.
- **E2:** [Official log guide](https://docs.libretro.com/guides/generating-retroarch-logs/), read 2026-10-30.
- **E3:** [Official input/controller driver guide](https://docs.libretro.com/guides/input-controller-drivers/), read 2026-10-30.
