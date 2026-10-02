# nesturbator

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

## Style

Small modules, plain control flow, fixed-width integer types. A hardware comment
says what the hardware does and cites its source. To decide how to do something,
follow the ladder at ponytail.dev: remove the need, reuse what is here, use the
standard library, use the platform, and only then write the minimum new code.
