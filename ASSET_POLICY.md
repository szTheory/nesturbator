# Asset policy

What may and may not be in this repository, its history, its CI artifacts and
its releases.

## Never

- Bytes of a commercial game, in any amount or form: ROM images, patches that
  embed them, disassemblies, extracted graphics, sound or text.
- BIOS or firmware images, including the Famicom Disk System BIOS.
- Battery saves, save states, screenshots, recordings, traces or logs made
  while running a commercial game.
- A header or checksum database copied from another project. None publishes a
  licence.
- Nintendo's logos, artwork or packaging.
- Test ROMs whose authors state no licence, or a copyleft or non-commercial
  one. When a test needs one, it is fetched at a pinned commit and checked
  against a recorded SHA-256; it is not committed.

Owning a cartridge does not change this list. A player's own dump stays on the
player's machine.

## Allowed

- Test ROMs and test data whose licence permits redistribution, each listed in
  `tests/roms/manifest.txt` with its source, pin, licence and SHA-256.
- Single facts about a game or a board: a mapper number, a board name, a size,
  the hash of a file. A bulk table of such facts copied from a database is not
  allowed (see above).
- Images and recordings made from the committed test ROMs or from the core's
  own test frame.

## Local testing with your own games

Local test runs can be pointed at a private folder through `.env.local`, which
git ignores. What those runs produce (hashes, logs, screenshots) stays on that
machine.

## Names

"NES" and "Famicom" are used only to say what the core emulates. The project is
not affiliated with or endorsed by Nintendo.

## How this is checked

`scripts/hygiene.sh` rejects ROM and save file extensions and the iNES and FDS
file signatures unless the manifest lists the file. It runs on every commit and
push through the hooks in `.githooks/`, and in CI.

## If something gets in

Remove it and rewrite the branch before pushing; the push hook scans every
commit that is about to leave. If it was already pushed, rewrite the history,
force-push, and ask GitHub support to drop cached copies.
