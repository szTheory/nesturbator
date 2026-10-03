# Provenance

How the code in this repository is written and what may enter it. This file was
written on 2026-10-02, before any emulator code existed, so it covers the whole
history.

## Licence

Everything written here is MIT (see [LICENSE](LICENSE)). Nothing enters that the
project lacks the right to offer under that licence.

## Facts and expression

- **Facts may enter.** What the hardware does (addresses, register behaviour,
  timings, clock ratios, the result a test expects) is recorded in this
  project's own words, with a citation to where it was read or measured.
- **Another emulator's expression may not.** Code, comments, identifier names,
  table layouts and file structure are not copied, ported, translated or
  paraphrased from any other emulator, whatever its licence, and whether a
  person or an AI assistant does the rewriting.
- **Copyleft emulator source stays closed.** The source code of GPL and LGPL
  emulators is not opened, and no copy of it is kept in the workspace. They run
  only as released binaries, to compare results. Their documentation, release
  notes and issue trackers may be read.
- **Permissive emulator source** may be read to learn how a behaviour is
  modelled. The behaviour is then written from hardware documentation and
  shown by a test.

## Sources

Hardware behaviour comes from the NESdev Wiki, chip-level write-ups made from
die photographs, published papers, and the source and documentation of public
test ROMs. The files under [`.planning/preparation/`](.planning/preparation/)
list each source with its revision or commit.

## AI assistance

This project is written with AI coding assistants, directed and reviewed by its
maintainer.

- An assistant writes code from hardware documentation, this repository's
  files and test results. It is not asked to port, translate or reproduce
  another emulator's code.
- During preparation, assistants read permissively licensed emulators to
  compare designs, and read RetroArch's help text, configuration and platform
  files for facts about how it loads cores. The Sources tables under
  `.planning/preparation/` list each one. No code was taken from them; the
  libretro adapter is written against `libretro.h`, which is MIT.
- A constant, table or name that the cited sources do not explain is treated as
  suspect: it is re-derived from a cited source, or removed, before it merges.
- Commits and pull requests made with an assistant say so, in a co-author
  trailer or in the description.

## Vendored files

`libretro.h` (MIT) is the only file this project vendors. It is never edited;
ctest `libretro.vendored` checks its SHA-256, and its licence notice is in
[THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md).

| File | Upstream | Commit | SHA-256 |
|---|---|---|---|
| `libretro/libretro.h` | https://github.com/libretro/RetroArch (`libretro-common/include/libretro.h`) | 69a4f0ea1e8aaf442ae4858f2e7f2b31a1776576 (v1.22.2) | bd3398d29c3763d18617087020ff56b5450a48a63123403a4b674f8e79947acb |

## Test data

A committed test ROM or vector file carries an explicit licence that allows
redistribution, and is listed in `tests/roms/manifest.txt` with its source, pin,
licence and SHA-256. [ASSET_POLICY.md](ASSET_POLICY.md) says what never enters.

The CPU test vectors are not vendored code. They are MIT test data from the
SingleStepTests 65x02 project, at
https://github.com/SingleStepTests/65x02 commit
2f6980a2d95757486c7bee24355c360e40e2a224. The sample is converted from the
upstream JSON by `tools/vecconv`, and the fixtures are byte-range prefixes of
it. Their licence notice is in [THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md);
ctest `manifest.sha256` checks each SHA-256.

| File | Upstream | Commit | SHA-256 |
|---|---|---|---|
| `tests/vectors/65x02-sample.n65v` | https://github.com/SingleStepTests/65x02 (`nes6502/v1/00.json` to `ff.json`, first 100 tests each) | 2f6980a2d95757486c7bee24355c360e40e2a224 | 0c318cec0bd4031396964ca9a0de0daf25c2651c28f34a0d0616a9294430e109 |
| `tests/vectors/fixtures/a9-first3.json` | https://github.com/SingleStepTests/65x02 (`nes6502/v1/a9.json`, bytes 0-983) | 2f6980a2d95757486c7bee24355c360e40e2a224 | 3195bc1b6d17fb002f8d0a4f8f5b99077d42d62f79e35161d9de1f7d4bf1d83b |
| `tests/vectors/fixtures/02-first3.json` | https://github.com/SingleStepTests/65x02 (`nes6502/v1/02.json`, bytes 0-1591) | 2f6980a2d95757486c7bee24355c360e40e2a224 | 2738dad0c19bc0e4dc4d3a131a7cd3768a1493df9ec665b0e738fa397fbbb58f |
| `tests/vectors/fixtures/a9-tail.json` | https://github.com/SingleStepTests/65x02 (`nes6502/v1/a9.json`, bytes 0-1083) | 2f6980a2d95757486c7bee24355c360e40e2a224 | 6d60b42a8fca08c75e7e3ea364085be67ea786c9d15e279e6fe9e4790ab42ddc |

## What this file does not do

It keeps the project's own licence clean. It is not legal advice, and it is not
a defence against a rights holder's claim; the asset policy handles that side
by keeping game content out of the repository entirely.
