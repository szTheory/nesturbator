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

`libretro.h` (MIT) is the only file this project vendors. Its upstream, commit
and SHA-256 are recorded here when the file is added.

| File | Upstream | Commit | SHA-256 |
|---|---|---|---|
| none yet | | | |

## Test data

A committed test ROM or vector file carries an explicit licence that allows
redistribution, and is listed in `tests/roms/manifest.txt` with its source, pin,
licence and SHA-256. [ASSET_POLICY.md](ASSET_POLICY.md) says what never enters.

## What this file does not do

It keeps the project's own licence clean. It is not legal advice, and it is not
a defence against a rights holder's claim; the asset policy handles that side
by keeping game content out of the repository entirely.
