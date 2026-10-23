# Technology Stack: v4 Measured NTSC accuracy and robustness

Researched and approved 2026-10-13. No dependency additions. Existing versions are defined by the repository's presets, pins and workflows; v4 does not upgrade tooling incidentally.

## Existing stack to reuse

- Owned C17 library, headless runner and libretro adapter; only `libretro.h` is vendored. Integer-only core, per-instance state and memory-function-only core linkage remain constraints.
- CMake/CTest workflow presets, owned `check.h` helpers, portable libretro host tests, required hosted RetroArch job and six-platform hash equality.
- Existing Clang libFuzzer, ASan/UBSan and corpus-replay infrastructure; add one small owned accepted-program target rather than a new fuzz engine or dependency framework.
- Existing SHA/pin/manifest and scoreboard machinery, plus an owned tested decoder for AccuracyCoin's compressed names.
- Markdown and git for product direction, with a project-instruction refresh procedure. No service, parser package or extra CI job is needed.

## New integration needs

The immutable AccuracyCoin candidate is `6bee919529289c555803b612b8c9e4f7eaaeeabb`: 146 scored tests and five DRAW entries. Recheck the exact artifact licence/hash and result encoding during Phase 17. Migrate the parser and protected keys alongside the pin; do not fetch mutable main at test time.

RetroArch uses UDP commands. Probe capabilities of the pinned released binary before relying on documented status replies. Confirm safely retried requests and never blindly replay FRAMEADVANCE or PAUSE_TOGGLE.

## Installation and exclusions

No installation step, new runtime dependency, new host ABI, GUI/device layer or generic abstraction is proposed. Existing tooling expresses the work. Small owned helpers or justified duplication are preferable to expanding the dependency tree.

## Sources

[AccuracyCoin pinned README](https://github.com/100thCoin/AccuracyCoin/blob/6bee919529289c555803b612b8c9e4f7eaaeeabb/README.md), [LLVM libFuzzer](https://llvm.org/docs/LibFuzzer.html), [Clang UBSan](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html), [RetroArch NCI](https://docs.libretro.com/development/retroarch/network-control-interface/). Accessed 2026-10-13. Detailed repository observations are in [accuracy](v4-scope/accuracy.md) and [reliability](v4-scope/reliability.md).
