# Phase 1: A test frame in RetroArch - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-10-02
**Phase:** 01-a-test-frame-in-retroarch
**Areas discussed:** Test frame content, Header scope now, Palette table source, Frame image & compare

Advisor mode (calibration `minimal_decisive`). Each area was researched by a separate agent with the owner's brief: fan out through every relevant role lens, weigh pros, cons, patterns and lessons from other ecosystems, run an adversarial pass, and synthesize one recommendation; prefer copy-and-paste over a new dependency.

---

## Test frame content

| Option | Description | Selected |
|--------|-------------|----------|
| Static test card | Computed 512-value chart + border + top-left corner block + distinct overscan bands; metadata proves advance; anchor pixels in the libretro test | ✓ |
| Animated card | Same plus a moving marker or frame counter | |

**User's choice:** Static test card (recommended)

---

## Header scope now

| Option | Description | Selected |
|--------|-------------|----------|
| Phase 1 surface only | version/create/destroy/get_info/get_palette/run_frame (silence at true audio rate); conventions fixed now; later phases only append | ✓ |
| Full API with stubs | Every ARCHITECTURE §7 function declared now, unbuilt ones return "unsupported" | |

**User's choice:** Phase 1 surface only (recommended)

---

## Palette table source

| Option | Description | Selected |
|--------|-------------|----------|
| Own C generator | tools/palgen from NESdev Wiki NTSC levels; checked-in table; ci regenerates and byte-compares on all 6 runners | ✓ |
| Adopt pally (MIT-0) | Commit a pally-generated table at a pinned commit; Python + colour-science toolchain | |

**User's choice:** Own C generator (recommended)
**Notes:** Researcher proposed an exported global table; synthesis changed it to a copy-out `nesturbator_get_palette` to avoid Windows DLL data imports and allow per-revision tables. Runtime integer generation and adopting FirebrandX/Smooth/Wavebeam palettes (no clear licence) were rejected.

---

## Frame image & compare

| Option | Description | Selected |
|--------|-------------|----------|
| PPM + sips + C compare | Runner writes P6 PPM; macOS sips converts RetroArch's PNG to BMP; in-repo C comparator, exact match; skip 77 without RetroArch | ✓ |
| Own PNG + own inflate | In-repo PNG writer and decoder; works on any OS; 300+ lines needing fuzzing | |

**User's choice:** PPM + sips + C compare (recommended)
**Notes:** sips behaviour was measured on the owner's Mac with scratch files: exact for plain, gAMA 1/2.2 and sRGB PNGs; colour-shifted for gAMA 1.0.

---

## Claude's Discretion

- Source layout below `src/`, the libretro test program's shape, runner option parsing within LIBRETRO-AND-RUNNER §5, the frame count passed to `--max-frames`.

## Deferred Ideas

- Palette colorimetry and pally cross-check — Phase 3
- Host `.pal` override as a display-only core option — Phase 3
- RetroArch frame-index calibration with a moving frame — Phase 3
- RetroArch test off macOS — not planned
