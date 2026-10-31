# Release archives and process

This guide is maintained in the source repository. It is not included in the
downloaded ZIPs. For behavior tied to a particular release, read this guide
from the matching source tag or commit, for example
`https://github.com/szTheory/nesturbator/blob/<tag>/docs/releases.md`.

## Archive components

`cmake --workflow --preset ci` creates three ZIP archives for the configured
build target in `build/ci/packages/`. CPack installs one archive per component
with no enclosing top-level directory. Release targets use OS values `linux`,
`macos` and `windows`, and architecture values `x64` and `arm64`.

| Archive | Contents at the ZIP root |
|---|---|
| `nesturbator-VERSION-libretro-OS-ARCH.zip` | `cores/nesturbator_libretro.{so,dylib,dll}`, `info/nesturbator_libretro.info`, `LICENSE`, `THIRD-PARTY-NOTICES.md` |
| `nesturbator-VERSION-runner-OS-ARCH.zip` | `bin/nesturbator-run` (`.exe` on Windows), `LICENSE` |
| `nesturbator-VERSION-library-OS-ARCH.zip` | `include/nesturbator.h`, the static library and relocatable package files under `lib/`, `LICENSE` |

The install components and archive names are defined in
[`cmake/packaging.cmake`](../cmake/packaging.cmake). The source build writes
the current build-machine archives; it does not establish that all six release
targets have been built or run on their target hosts.

Run the archive inventory check against a completed source build:

```sh
cmake -DDIR=build/ci -P tests/cmake/check_archives.cmake
```

The checker requires exactly the three current-version archives, expected
component directories and exact component contents. It validates raw ZIP
member names before CMake's line-oriented listing, rejecting unsupported ZIP
layouts, semicolons, ASCII control characters and duplicate names. Each
required pattern must match exactly once; raw names must agree with the
listed names, so Unicode path aliases cannot substitute for an allowed name.
The packaged libretro metadata must match the source metadata and its checked
support claim. Existing negative controls also exercise archive metadata and
inventory rejection. Guides remain in the repository and are not added to any
archive component.

Unzip the libretro archive into RetroArch's data directory to place its core
and information file in `cores` and `info`. The player-facing installation
instructions, including the marked Apple Silicon command, remain in the
[README](../README.md).

## Apple Silicon install-line check

The README install block sets the version immediately before the download
pipeline. Keeping one version on its own marked line lets Release Please update
the tag and archive filename through the configured README marker. The guide
does not repeat a current release version.

The macOS-only build-tree check uses the exact README extraction operands
against the archive produced by the configured build:

```sh
cmake -DREADME=README.md -DPACKAGES=build/ci/packages -DOUT=build/ci/install-line -P tests/cmake/check_install_line.cmake
```

It pipes the source-built ZIP into macOS tar using the README's flags,
destination and member list, and requires exactly
`cores/nesturbator_libretro.dylib` and
`info/nesturbator_libretro.info`. It also checks that the version line matches
`version.txt` and that both the release tag and archive filename come from
the version variable. This check does not fetch a published release. It runs
on macOS, including the macOS arm64 CI leg.

The cross-platform `release.one_version_per_line` test checks every marked
Release Please file, naming any line that contains more than one version.
Its independent fixture verifies detection of a two-version marked line.

## Version markers and release policy

Release Please updates the version numbers in the public header, the
libretro information file, and the README. The README's inline marker is
owned by `release-please-config.json`; keep its `extra-files` entry because
the Apple Silicon install command derives its tag and archive filename from
that version. Do not add a second current-version literal to this guide.

The three public-header version macros are inside a narrow
`clang-format off/on` region so a numeric change such as patch 9 to 10 does
not move their inline `x-release-please-*` markers. `release.version_format`
checks the guard, markers and numeric-only mutations in CI;
`hygiene.release_version_format` formats full mutated headers with
clang-format 18. `release.one_version_per_line` scans every path configured
in `release-please-config.json`: Release Please updates only the first version
on each marked line. Its independent fixture confirms that two versions on a
marked line are rejected. The one-time `release-as` setting must also be
removed after it is used, or future release requests can reuse the pinned
version; `release.no_release_as` checks this.

Release Please reads Conventional Commit titles on `main`, opens or updates a
release pull request and auto-merges it after the required CI check passes.
The version is derived from commit titles: before 1.0, a `feat:` raises the
patch number. The pinned configuration uses the simple release type,
`bump-minor-pre-major` and `bump-patch-for-minor-pre-major`, a draft release
and immediate tag creation. Its nonbehavioral policy keeps ordinary
`docs`/`chore` commits from opening release pull requests while preserving
behavior-changing and breaking commits. `release.nonbehavioral_policy`
checks that policy and the exact CI/publish gate conditions.

The release workflow runs on a push to protected `main`. A GitHub App token
is minted only after that push and is limited to Release Please and release
pull-request auto-merge. Manually selected refs and extra private-key token
consumers fail `release.credentials_policy`. After the release pull request
merges, the workflow runs the full CI workflow on the exact release commit.
Publishing depends on that CI result.

The publish job downloads the six-platform build artifacts and requires the
exact 18 archives: library, runner and libretro components for Linux, macOS
and Windows on x64 and arm64. It writes `SHA256SUMS` in `sha256sum -a 256`
format, creates a build-provenance attestation for that checksum file,
uploads the archives and sums to the draft, and verifies each archive's
attestation. It also flips a byte in a copy and requires that copy's
attestation verification to fail. Only after these checks does it publish
the release, then verify the published tag, asset names and nonempty asset
sizes. `release.nonbehavioral_policy` checks that only the gated publish job
can make the release public.

To verify a downloaded release asset against the repository's attestation:

```sh
gh attestation verify FILE --repo szTheory/nesturbator
```

The archive and checksum checks establish package contents and provenance;
they do not establish execution on every target host. The current README's
install route and the package checks answer separate questions.

Pull-request rules are in [CONTRIBUTING.md](../CONTRIBUTING.md); security
reports go through [SECURITY.md](../SECURITY.md).
