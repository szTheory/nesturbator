<!-- GSD:project-start source:PROJECT.md -->

## Project

**nesturbator**

nesturbator is a NES emulator core written in C. It ships as a library other programs embed, a headless command-line runner for automation, and a libretro adapter so that hosts such as RetroArch can play games with it. It is for people who build emulator hosts and want an accurate, permissively licensed NES core, and for the players who use those hosts.

**Core Value:** Games behave as they do on the console, identically on every platform, from a small MIT-licensed C library that any host can embed.

### Constraints

- **Deliverables**: a C library, a headless runner and a libretro adapter only — the host owns windows, audio devices and input devices.
- **Language**: C17 without extensions; the core links only the C memory functions — it has to embed anywhere.
- **Ownership**: all emulation and test code is written here; `libretro.h` is the only vendored file — owned code over dependencies.
- **Determinism**: integer-only core with all state in the instance — the same inputs give identical frame and audio hashes on every platform.
- **Legal**: no ROM or BIOS bytes beyond licensed test ROMs; nothing taken from GPL or LGPL emulators — the repository is public and MIT.
- **Privacy**: no personal paths, addresses or names in tracked files or release artifacts — the repository is public.
- **Verification**: every behaviour is shown by a command: `cmake --workflow --preset ci` for the main suite, the other presets and the nightly jobs for the rest — nothing waits on a person.
- **Delivery**: the owner creates each phase's branch; it merges by pull request into a green `main` and is released automatically — each phase ends in something to download and run.

<!-- GSD:project-end -->

<!-- GSD:stack-start source:research/STACK.md -->

## Technology Stack

## Recommended Stack

| Technology | Version | Purpose |
|------------|---------|---------|
| C | C17, extensions off | Core, runner, libretro adapter |
| CMake | 3.25 or newer | Build and workflow presets: `dev`, `ci` (`ci-msvc` on Windows), `asan`, `nofp`, `hygiene` |
| Ninja, CTest | current | Build tool; test runner with an in-repo `check.h` |
| `libretro.h` | pinned commit | The only vendored file |
| GitHub Actions | six hosted runners | Linux, macOS and Windows on x64 and arm64 |
| release-please | action v5 | Version, changelog and release from Conventional Commit titles |
| libFuzzer | Linux, nightly | Fuzzing the ROM loader |

## Commands

| Command | Does |
|---------|------|
| `cmake --workflow --preset ci` | Builds everything and runs every test |
| `cmake --workflow --preset asan` | Runs the same tests under the sanitizers |
<!-- GSD:stack-end -->

<!-- GSD:conventions-start source:CONVENTIONS.md -->

## Conventions

Conventions not yet established. Will populate as patterns emerge during development.
<!-- GSD:conventions-end -->

<!-- GSD:architecture-start source:ARCHITECTURE.md -->

## Architecture

Architecture not yet mapped. Follow existing patterns found in the codebase.
<!-- GSD:architecture-end -->

<!-- GSD:skills-start source:skills/ -->

## Project Skills

No project skills found. Add skills to any of: `.claude/skills/`, `.agents/skills/`, `.cursor/skills/`, `.github/skills/`, or `.codex/skills/` with a `SKILL.md` index file.
<!-- GSD:skills-end -->

<!-- GSD:workflow-start source:GSD defaults -->

## GSD Workflow Enforcement

Before using Edit, Write, or other file-changing tools, start work through a GSD command so planning artifacts and execution context stay in sync.

Use these entry points:
- `/gsd-quick` for small fixes, doc updates, and ad-hoc tasks
- `/gsd-debug` for investigation and bug fixing
- `/gsd-execute-phase` for planned phase work

Do not make direct repo edits outside a GSD workflow unless the user explicitly asks to bypass it.
<!-- GSD:workflow-end -->

<!-- GSD:profile-start -->

## Developer Profile

> Profile not yet configured. Run `/gsd-profile-user` to generate your developer profile.
> This section is managed by `generate-claude-profile` -- do not edit manually.
<!-- GSD:profile-end -->
