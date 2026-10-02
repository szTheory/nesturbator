# Stack Research

**Domain:** NES emulator core in C
**Researched:** 2026-10-02
**Confidence:** HIGH

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

---
*Details and sources: `.planning/preparation/ENGINEERING.md`*
