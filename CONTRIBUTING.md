# Contributing

Thank you for helping. nesturbator is a small, MIT-licensed NES core, and the
rules below keep it that way. [AGENTS.md](AGENTS.md) is the short list;
[PROVENANCE.md](PROVENANCE.md) and [ASSET_POLICY.md](ASSET_POLICY.md) explain
them.

## Clean room

- Write emulation code from hardware documentation and from what test ROMs
  show, and cite the source in a comment.
- Do not open the source code of GPL or LGPL emulators, and keep no copy of
  it in your workspace. Reference emulators may run only as released binaries,
  to compare results.
- Do not copy, port, translate or paraphrase another emulator's code,
  comments, names, tables or structure, whatever its licence, and whether you
  or an AI assistant does the rewriting.
- `libretro/libretro.h` is the only vendored file. The core library links
  only the C memory functions.
- No ROM, BIOS or save data enters the repository, except licensed test ROMs
  listed in `tests/roms/manifest.txt`.

## Set up

Turn on the hooks once per clone. They run `scripts/hygiene.sh` on every
commit and push:

```sh
git config core.hooksPath .githooks
```

Commit with your GitHub noreply address (find it under GitHub's email
settings), and keep personal paths, email addresses and names out of tracked
files. The hooks and CI reject them.

```sh
git config user.email <id>+<handle>@users.noreply.github.com
```

## Build and test

```sh
cmake --workflow --preset ci       # build, every test, the release archives
cmake --workflow --preset asan     # the tests under AddressSanitizer and UBSan
cmake --workflow --preset nofp     # the core with no floating-point registers
cmake --workflow --preset hygiene  # tree contents, action pins, formatting
```

CI runs the same commands on Linux, macOS and Windows; see the README.

Every behaviour change lands with a test that `cmake --workflow --preset ci`
runs, and with the README and the comments in `include/nesturbator.h` updated
in the same change. Checks are automated; none waits on a person.

## Pull requests

- The title is a Conventional Commit, such as `feat(ppu): sprite zero hit` or
  `fix: reject a truncated header`. Types: feat, fix, docs, style, refactor,
  perf, test, build, ci, chore, revert. The `title` job checks it.
- Pull requests are squash-merged, and the title becomes the commit on
  `main`. release-please reads those titles to choose the next version and
  write the changelog, so `feat` and `fix` titles publish a release.
- If an AI assistant helped, say so in the description: which parts it wrote
  and what it was given to work from.
