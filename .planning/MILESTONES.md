# Milestones

## v1 NROM games with sound (Shipped: 2026-10-09)

**Delivered:** NROM games render, take controller input and play with sound in RetroArch, with frame and audio hashes identical on Linux, macOS and Windows on x64 and arm64.

**Phases completed:** 6 phases (01, 02, 03, 04, 04.1, 04.2), 44 plans, 73 tasks
**Timeline:** 2026-10-02 → 2026-10-10 (8 days), releases v0.1.0 through v0.1.6
**Code:** 13,642 lines of C (5,964 in the core library), excluding the vendored `libretro.h`
**Requirements:** 19/19 v1 requirements satisfied ([audit](milestones/v1-MILESTONE-AUDIT.md))

**Key accomplishments:**
- A C17 library, headless runner and libretro core build, test and release on six platforms behind one required CI check; merging a behaviour change publishes 18 attested archives and `SHA256SUMS` with no manual step.
- The 6502 matches the public 65x02 vectors on all 256 opcodes on every bus cycle: a committed sample runs in CI, and the full 2,560,000-test set runs nightly at its pinned commit.
- A bounded mapper-0 iNES/NES 2.0 loader, a dot-timed PPU with sprites and OAM DMA, and two controller ports run three open-licence games whose frame hashes match byte for byte on all six platforms and in a pinned RetroArch screenshot check.
- Deterministic movie replay, AccuracyCoin results read from emulated RAM against a protected scoreboard, and loader fuzzing (corpus replay in CI, libFuzzer nightly).
- An integer APU and band-limited synthesiser produce mono PCM whose transition and sample hashes match on every platform; five reference tones stay below -80 dB of non-harmonic content, and libretro delivers equal stereo through both the batch and single-sample callbacks.

**Closeout:** override closeout. All six phase verifications passed, but GSD marked them stale because 03-15, 04.1, 04.2 and release version bumps later changed covered files. Those changes carry their own passing verifications, `main` CI passed at `5c082c1`, and the milestone audit covers them.

Known verification overrides: 9 newly acknowledged, 0 carried forward from a prior close (see STATE.md Deferred Items)

**Deferred tech debt:**
- `tests/cmake/release_policy.cmake` matches the publish gate by substring; an appended `|| always()` could evade it.
- No trainer-bearing fixture runs the full runner or libretro host path.
- The local `retroarch.testframe` and `retroarch.game` tests self-skip on the owner's Mac because RetroArch does not start there; only the hosted `retroarch-e2e` job gives live screenshot evidence.
- `retro_reset()` does nothing for a loaded game (outside v1 requirements).

---
