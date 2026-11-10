# Release archives and process

For agent-led CI/CD timing review and verification automation, apply the project
[verification policy](../.planning/VERIFICATION-POLICY.md), preserving all release
validation and publication gates.

This guide is maintained in the source repository. It is not included in the
downloaded ZIPs. For behavior tied to a particular release, read this guide
from the matching source tag or commit, for example
`https://github.com/szTheory/nesturbator/blob/<tag>/docs/releases.md`.

## D-44 package handoff route

The current D-44 route keeps the historical AccuracyCoin identity
d4802809ff41 explicitly marked dirty. Its pinned 56-path historical input map
is separate from the current receipt-checker hashes; reconstructing that map
does not independently authenticate the historical dirty tree. No identity
here proves hardware behavior. The compatibility profile remains ABI 1,
behavior revision 9, state format 3 and ordinary-loader NTSC.

Local acceptance, the actual AccuracyCoin campaign and package publication
remain pending. This maintenance revision does not claim a release exists.
The source-bound one-shot instructions and their evidence rules are kept in
the D-44 workstream planning records, not in this consumer guide. The ignored
route driver is private to its prepared worktree and is neither shipped nor
available in a fresh checkout. Consumers should use the public CMake package
checks below and, when published, the package receipt.

## Historical D-42 package-only reset identity

The following D-42 material preserves its original route, base and commands.
Do not replay its one-shot commands for D-44.

This route starts from the approved NTSC source without emulator-source or
build-flag changes. The receipt enumerates the actual complete tracked input
set; it does not reuse an older path count or a capture identity. Its exclusive
route ledger permits one full local workflow, one exact-candidate required CI
and one resulting-main CI, stopping on an acceptance failure without retry.
The existing release-PR and reusable release-source gates remain required.
The ledger is an ignored local record, never a package asset. The profile is
ABI 1, behavior 9, state format 3 and ordinary-loader NTSC only. No capture,
APU optimization or additional regional capability is part of this release.

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

## Identifying a loaded build

The displayed core name and semantic version are useful for an initial check,
but they do not identify the exact source or binary. Establish the module path
used by the active launch and hash that module. When the current frontend log
records the loaded path, retain that line with the host version and launch
route. RetroArch's current CLI guide also shows explicit selection with
`retroarch -L <core-path> <content>`; a launcher such as ES-DE may choose a
different configured path. If neither the active command nor its log shows
which module was loaded, report the exact build as unknown. See Libretro's
[Starting a Game](https://docs.libretro.com/guides/starting-a-game/),
[CLI](https://docs.libretro.com/guides/cli-intro/) and
[logging](https://docs.libretro.com/guides/generating-retroarch-logs/) guides.

Keep these three identities separate:

| Build source | Identity evidence |
|---|---|
| Published release | Release tag, exact archive name, archive SHA-256 from that release's `SHA256SUMS`, and GitHub artifact attestation. Extract the module from that verified archive and compare its SHA-256 with the loaded module. |
| CI development artifact | GitHub Actions workflow run, source commit, artifact name and downloaded module digest. A CI artifact is not a published release and does not inherit its tag or release provenance. |
| Pinned local build | Exact source commit, CMake configuration/compiler and built module digest. A local version string alone does not bind the binary to a commit. |

For a published ZIP, compare its digest with the matching `SHA256SUMS` entry
and verify its attestation before treating it as a release artifact:

```sh
gh attestation verify FILE --repo szTheory/nesturbator
```

The attestation covers the archive; its checksum does not directly hash the
extracted module. Hash both the loaded module and the corresponding
`cores/nesturbator_libretro` member extracted from the same archive. Use
`shasum -a 256` on macOS, `sha256sum` on Linux, or
`Get-FileHash -Algorithm SHA256` in PowerShell. The names in RetroArch's core
list and `.info` file are not substitutes for this path-and-digest comparison.

For a CI artifact, retain the run page and source commit, then compare the
loaded module against the module from that exact downloaded artifact. For a
local build, record `git rev-parse HEAD`, the selected build configuration and
the digest of the module you actually load. If the exact path, bytes or source
identity cannot be tied together, leave the result unknown rather than
attributing current-checkout capabilities to it.

On macOS, Gatekeeper may restrict downloaded, quarantined plug-ins. Apple
documents that applications on macOS 10.15 and later can load quarantined
plug-ins only when they are notarized, or after the user explicitly approves
them. Verify the archive source and digest before acting on a system warning;
this repository guide makes no signing or notarization claim for a particular
binary. See Apple's [notarization guidance](https://developer.apple.com/documentation/security/notarizing-macos-software-before-distribution).

The release list and each release's archive assets are maintained on the
[public GitHub releases page](https://github.com/szTheory/nesturbator/releases).
Current availability is time-dependent; do not reuse an earlier lookup as a
current release observation.

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

## Release commit CI ownership

The generated `chore(main): release ...` merge push skips the redundant
ordinary `CI` workflow jobs before they allocate runners. Release invokes the
same reusable CI workflow on that exact commit, and GitHub exposes the caller
workflow name as `Release` while retaining the push event. The caller therefore
still runs the complete six-platform validation and produces its archive and
hash artifacts; its existing CI dependency and publication checks remain in
force. Release pull requests and ordinary CI events keep their validation.
Nightly continues to run on the main push, including the generated release
merge commit.

## Historical D-42 hosted release evidence

The D-42 commands below preserve the prior route and its recorded base. They
are not commands for the current D-44 handoff.

The package qualification receipt checker can read current GitHub evidence
without starting a workflow. Run it against the clean source checkout with a
tracked release evidence record. The reset source must already be frozen and
its local stage passed. Before the first candidate push/PR trigger, reserve
`MODE=reset-admit STAGE=candidate`; before protected merge, reserve
`MODE=reset-admit STAGE=main`, with the same RESET_BASE/RESET_ROUTE and the
source-bound ACCEPTANCE and RELEASE_EVIDENCE paths. Admission records intent
before a remote trigger; the following hosted commands observe the completed
run and finalize that intent. They do not launch or retry CI:

```sh
cmake -DSOURCE_DIR=. -DMODE=hosted -DSTAGE=candidate \
  -DRESET_BASE=b25146c6efaac41c58f60555f59f0fe79a90ca68 \
  -DRESET_ROUTE=build/phase28-d42/route.json \
  -DRELEASE_EVIDENCE=.planning/workstreams/libfuturo-nes-gameplay-handoff/phases/28-qualified-nes-gameplay-package/28-RESET-RELEASE-EVIDENCE.json \
  -P tests/cmake/provider_receipt.cmake
cmake -DSOURCE_DIR=. -DMODE=hosted -DSTAGE=main \
  -DRESET_BASE=b25146c6efaac41c58f60555f59f0fe79a90ca68 \
  -DRESET_ROUTE=build/phase28-d42/route.json \
  -DRELEASE_EVIDENCE=.planning/workstreams/libfuturo-nes-gameplay-handoff/phases/28-qualified-nes-gameplay-package/28-RESET-RELEASE-EVIDENCE.json \
  -P tests/cmake/provider_receipt.cmake
cmake -DSOURCE_DIR=. -DMODE=hosted -DSTAGE=release \
  -DRELEASE_EVIDENCE=.planning/workstreams/libfuturo-nes-gameplay-handoff/phases/28-qualified-nes-gameplay-package/28-RESET-RELEASE-EVIDENCE.json \
  -P tests/cmake/provider_receipt.cmake
```

The stages check the active main ruleset, exact PR heads, current test merges,
workflow identity, event, PR association, completed job results and required
rollup. Candidate and release PR test merges are recorded separately from their
PR heads and final merge commits. Resulting implementation-main CI belongs to
the candidate PR's squash merge. The Release/push caller owns full CI on its
own release-source commit; its successful reusable `ci / CI required` job and
real platform jobs are required evidence. Skipped ordinary CI jobs on a
generated release push cannot stand in for either main or release-source
execution.

`MODE=published` uses the recorded hosted evidence, verifies the observed
non-draft tag and source, downloads all release archives, checks sums and
attestations, and runs the native relocated package checks before creating
`docs/provider-ntsc-release.json`. `MODE=verify-published` checks that existing
receipt against current hosted release identity and its recorded structure; it
does not replace the receipt, download archives or repeat native gameplay.
Keep the local acceptance source, candidate PR head, candidate test merge,
resulting implementation-main commit, release PR head, release PR test merge
and release source as distinct fields even when observed commits happen to be
the same.

The offline `hygiene.ci_release_push_policy` and its `.selftest` verify this
workflow routing, the Release publication gates, and the Nightly trigger. They
establish the checked source configuration only; hosted execution and
published assets require their own GitHub run evidence.

The publish job downloads the six-platform build artifacts and requires the
exact 18 archives: library, runner and libretro components for Linux, macOS
and Windows on x64 and arm64. It writes `SHA256SUMS` in `sha256sum -a 256`
format, uses those checksums to identify the archive subjects for
build-provenance attestations,
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

## Historical D-42 published provider receipt

The following D-42 command is retained as historical evidence; use the D-44
route above for a new package handoff.

The provider profile stays candidate-only until the normal shipping step checks
the exact public release. Run the receipt driver from a clean checkout at the
commit resolved by the observed Release Please tag, with the accepted candidate
identity available in that checkout:

```sh
cmake -DSOURCE_DIR=. -DMODE=published \
  -DACCEPTANCE=.planning/workstreams/libfuturo-nes-gameplay-handoff/phases/28-qualified-nes-gameplay-package/28-RESET-IDENTITY.json \
  -DTAG=ACTUAL_RELEASE_PLEASE_TAG \
  -DWORK=build/phase28-published \
  -P tests/cmake/provider_receipt.cmake
```

Replace `ACTUAL_RELEASE_PLEASE_TAG` with the tag returned by Release Please.
The driver resolves that tag to a commit and requires `SOURCE_DIR` to be a
clean checkout at the same commit. It uses bounded, noninteractive, read-only
`gh` calls to inspect the non-draft release, download its assets, and run
`gh attestation verify FILE --repo szTheory/nesturbator --source-digest
EXACT_SOURCE_SHA --format json` on each archive. It checks that the release
contains exactly the 18 component/target ZIPs and `SHA256SUMS`, compares each
downloaded archive's bytes with its unique checksum line, and records each
verified attestation's source and subject digest. The existing archive checker
then verifies the contents and libretro metadata for each of the six OS and
architecture groups.

Published receipts use schema version 2. Their `scenario` object binds the
DABG source URL and commit to the ROM digest. Its `rights` object records SPDX
`Zlib`, the pinned commit's `LICENSE.txt` URL and source commit, the exact
license text digest, permitted uses, restrictions, and the required standalone
notice path and digest. The `distribution` object states that the ROM is absent
from component archives and that the receipt is delivered as a source-evidence
overlay. The required notice digest must equal the license text digest, and the
published and verify-published paths retrieve the pinned upstream text (60
seconds, no retry), compare its bytes with the tracked notice, and verify the
digest before they accept or write the receipt. A failed fetch or byte mismatch
blocks both paths.

Before local acceptance, the pinned upstream text can be compared with the
tracked source copy using a single bounded retrieval (60 seconds, no retry):

```sh
cmake -DSOURCE_DIR=. -DMODE=rights -DWORK=build/phase28-rights-verify -P tests/cmake/provider_receipt.cmake
```

This read-only check also verifies the manifest row, profile, gameplay guide,
and `THIRD-PARTY-NOTICES.md` identity and license terms. The source distribution
contains the exact DABG Zlib notice at
[`tests/roms/licenses/DABG-ZLIB-LICENSE.txt`](../tests/roms/licenses/DABG-ZLIB-LICENSE.txt).
The notice permits use for any purpose, including commercial use, alteration,
and redistribution; users must preserve origin and authorship, mark altered
source, and retain the unmodified notice in source distributions. Any downstream
distribution that includes the ROM must carry the same notice. This repository
does not modify downstream distributions.

The exact component archive allowlists omit both `tests/roms/dabg.nes` and the
standalone DABG notice; the notice index in the libretro archive does not place
the ROM or standalone notice there. The fixed release asset inventory also
omits `docs/provider-ntsc-release.json`, which remains only in the source
checkout as the post-publication evidence overlay. The archive checker rejects
extra files, and the published receipt still requires the exact existing asset,
checksum, attestation, CI, and native replay gates.

On a matching native target, the gate extracts the published library, runner
and libretro packages. It runs the installed consumer from a moved prefix with
a fresh build and replays the DABG gameplay and refusal controls, checks the
installed header/runtime profile, runs the published runner's `--version` and
the bounded `--frames 110 --hash-frame 110 --hash-audio` probe against the permitted ROM, and records the libretro core-member hash.
Only a successful full run writes `docs/provider-ntsc-release.json`. The
attestation authenticates archive bytes; extracted member hashes are separate
evidence. Build-tree archives, a draft release, or the previously published
v0.1.12 baseline cannot stand in for this current exact-package receipt.

This is a one-time shipping/closeout check. Phase execution prepares the
contract and checker and records publication as pending; it does not publish or
claim a release. The receipt binds the release tag, source commit, version,
ABI, behavior revision, state format, all archive/checksum/attestation
identities, exact-candidate/main/release CI run URLs and heads, and the profile,
guide and ROM digests. Later commits that only record acceptance evidence do
not change which source produced the package.

The archive and checksum checks establish package contents and provenance;
they do not establish execution on every target host. The current README's
install route and the package checks answer separate questions.

Pull-request rules are in [CONTRIBUTING.md](../CONTRIBUTING.md); security
reports go through [SECURITY.md](../SECURITY.md).
