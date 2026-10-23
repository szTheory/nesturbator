# Published release byte evidence

This record binds the normal v0.1.11 publication to its immutable source and downloaded release assets. The release tag, not the current phase checkout or a staged package, is the source for archive checks.

## Release identity

| Field | Observed value |
| --- | --- |
| Tag / version | v0.1.11 / 0.1.11 |
| Release ID / URL | 422111807 / https://github.com/szTheory/nesturbator/releases/tag/v0.1.11 |
| Published | 2026-10-23T17:45:50Z |
| Draft | false |
| Direct tag object | commit e47feb3afc34baf1d611ef44153af199424f41a2 |
| Immutable source commit | e47feb3afc34baf1d611ef44153af199424f41a2 |
| Prior implementation merge | b39a0106e4cccfe9922e051d5680f1f7f84f5efa |
| Approved implementation PR head | 82e4686ece0aa8535aa39bbfc06e24d6d6b86af1 |
| Implementation PR | https://github.com/szTheory/nesturbator/pull/50 |
| Release Please PR | https://github.com/szTheory/nesturbator/pull/52 |
| Exact release workflow run | https://github.com/szTheory/nesturbator/actions/runs/39993108038 |

The remote tag-ref API was queried initially and again after asset verification; both named a direct commit object at the full SHA above. Fetching only refs/tags/v0.1.11 succeeded. The implementation merge is an ancestor of the release commit. The release PR merged from base b39a0106e4cccfe9922e051d5680f1f7f84f5efa to e47feb3afc34baf1d611ef44153af199424f41a2. The approved implementation PR head is 82e4686ece0aa8535aa39bbfc06e24d6d6b86af1 and its merge commit is b39a0106e4cccfe9922e051d5680f1f7f84f5efa.

The approved-head-to-release diff contains the expected release-please version/changelog updates in .release-please-manifest.json, CHANGELOG.md, README.md, include/nesturbator.h, libretro/nesturbator_libretro.info and version.txt. It contains no unexpected emulator implementation changes.

## Publication CI

Run 39993108038 is the completed, successful push-triggered Release workflow on e47feb3afc34baf1d611ef44153af199424f41a2. Its final requery confirms the same run ID, head SHA, push event, completed status and success conclusion.

| Job group | Result |
| --- | --- |
| Release Please, title, hygiene, pinned RetroArch e2e | success |
| Six platform builds (Linux, macOS and Windows; x64 and arm64) | all success |
| ASan; no-float Linux x64 and arm64; hash equality; CI required | all success |
| Publish | success |
| Perf | skipped for this push-triggered release run |

Native PR CI retained perf evidence: implementation head 82e4686ece0aa8535aa39bbfc06e24d6d6b86af1 passed run 39984028386, including perf; release PR head e980ea65b5d21aca30a5ce2f66f3f87a36dc753d passed run 39988911356, including perf. Both runs' six build legs, ASan, no-float, hash equality, pinned RetroArch and required CI jobs succeeded.

## Published asset inventory and checksum proof

The non-draft release lists exactly 19 distinct assets: the 18 expected component/platform ZIPs and SHA256SUMS. Asset IDs, reported sizes and GitHub API digests were unchanged on the post-download requery. The exact manifest digest is 556207df2becae6d480f8f7d7d38ec9a4565a72da906fe4bd0a73d20f9bd5589.

| Asset | Asset ID | Bytes | GitHub API SHA-256 |
| --- | ---: | ---: | --- |
| SHA256SUMS | 669899618 | 1956 | 556207df2becae6d480f8f7d7d38ec9a4565a72da906fe4bd0a73d20f9bd5589 |
| nesturbator-0.1.11-library-linux-arm64.zip | 669899533 | 116380 | a0a3e244e5abadaea047067af3bae0c7d0e3852e9e726ce555d27485d8aaeaa7 |
| nesturbator-0.1.11-library-linux-x64.zip | 669899539 | 114612 | 58247066cc5b2ee1cf2cf23ec956de956e856c109ffcb2e6bfcd512eafd445e2 |
| nesturbator-0.1.11-library-macos-arm64.zip | 669899536 | 101109 | 7ccf582590f921da7a3af75795226ce5ad04f8f9840c2cb16f1c9767e6db59a8 |
| nesturbator-0.1.11-library-macos-x64.zip | 669899537 | 99363 | c26091b4042e301f740cc925ea52c0ca28ceb832427068a7c022255735917874 |
| nesturbator-0.1.11-library-windows-arm64.zip | 669899531 | 145447 | 72e1f358ee1f2d2aa89ba3884c08f316591d43980e103a2762da26e0773e9c8d |
| nesturbator-0.1.11-library-windows-x64.zip | 669899550 | 122975 | f85db2e6a9adca6faa37a0e19c8cb7e7c07f281109581389b17b20db0b72ee35 |
| nesturbator-0.1.11-libretro-linux-arm64.zip | 669899556 | 97453 | c4cb1b54749047881feb218bd43c905e810d41b73f4f655c7b7d4e19fd972b93 |
| nesturbator-0.1.11-libretro-linux-x64.zip | 669899558 | 95715 | f60ae97874094b69a4237d9b17cba0125034390f1e729cbc8d7d8fda567cd02a |
| nesturbator-0.1.11-libretro-macos-arm64.zip | 669899559 | 81655 | fb9c631818d7196d9306f5357f74d281af97ad87dd27066c74e47bbb9bc1dc0f |
| nesturbator-0.1.11-libretro-macos-x64.zip | 669899566 | 79390 | 0309ec832b4eade4219a2cc055a1e1b5f53822dc56fc64d5e2f02145338f85e6 |
| nesturbator-0.1.11-libretro-windows-arm64.zip | 669899576 | 78902 | e6011af5bf696dd073b401fb92b17be0fa56616faab137f93200deb93f6af89a |
| nesturbator-0.1.11-libretro-windows-x64.zip | 669899577 | 79818 | 6cba2756660c6d524a46481d63d3a37baef7f23175265e15d178ed039e257b6e |
| nesturbator-0.1.11-runner-linux-arm64.zip | 669899590 | 111770 | 57910ba76b89990a967411a95469ea629ee3b497bbd6063e7c370699e119db3b |
| nesturbator-0.1.11-runner-linux-x64.zip | 669899595 | 110369 | 8dfdb69a70d73931fb003ab23bc3ba9ac9310a2cd47a2231563f5a6c5d0c29ff |
| nesturbator-0.1.11-runner-macos-arm64.zip | 669899606 | 92400 | 9381be4f3b9d7c13eed2372b07f89b4728a8c091b0c7b3a5ce270dd610da7943 |
| nesturbator-0.1.11-runner-macos-x64.zip | 669899603 | 90233 | da41fc571f9502dd7eaad3b3483e544230d6a9e62251f76423183bac53b359db |
| nesturbator-0.1.11-runner-windows-arm64.zip | 669899609 | 89848 | 2a8a6c9bfa3f59eda194d3352ad07ea2b1251524cab0f067b44bc39f5f01fc23 |
| nesturbator-0.1.11-runner-windows-x64.zip | 669899615 | 91281 | 6b370a08301cc270b106e951a1ab053ced647293264c16c0283f06798c7e3f8e |

All 19 actual assets were downloaded from this release. The manifest validator passed: 18 unique records, exact expected names, 64-hex SHA-256 plus two-space separators, no path-bearing names, and no extra or missing ZIPs. shasum -a 256 -c SHA256SUMS passed all 18 downloaded ZIPs. Downloaded sizes and SHA-256 digests also equal the release API values. The ignored machine-readable checksum record is build/phase16-release/checks/manifest-validation.json.

## Task 1: macOS arm64 released-byte path

An immutable git archive of e47feb3afc34baf1d611ef44153af199424f41a2 was extracted under ignored build output. Its version.txt is 0.1.11. The tagged source libretro/nesturbator_libretro.info SHA-256 is 524bb3d0dbfccea4a2ed93316fadbf2b89be1c48b927d7ec007ba3a411fe0711. The tagged README and public header parser accepts the exact released claim:

Supported NTSC MMC3: mapper 4, submappers 0/4; MMC6=no; four-screen=no.

| Platform | Library ZIP SHA-256 | Runner ZIP SHA-256 | Libretro ZIP SHA-256 | Archived info SHA-256 | Existing archive/support check | Exact-source attestation |
| --- | --- | --- | --- | --- | --- | --- |
| macOS arm64 | 7ccf582590f921da7a3af75795226ce5ad04f8f9840c2cb16f1c9767e6db59a8 | 9381be4f3b9d7c13eed2372b07f89b4728a8c091b0c7b3a5ce270dd610da7943 | fb9c631818d7196d9306f5357f74d281af97ad87dd27066c74e47bbb9bc1dc0f | 524bb3d0dbfccea4a2ed93316fadbf2b89be1c48b927d7ec007ba3a411fe0711 | pass, three archive ok records | pass, one nonempty SLSA provenance v1 result |

The existing snapshot tests/cmake/check_archives.cmake ran with PACKAGES=build/phase16-release/platforms/macos-arm64 and VERSION=0.1.11; exit 0, with the library, libretro and runner archives each reported ok. Its existing checker verifies ZIP inventory, safely extracts info/nesturbator_libretro.info, requires byte equality with the immutable source and applies tests/cmake/info_claims.cmake to the tagged README/header. The downloaded libretro archive's embedded info digest independently equals the source digest above.

The attestation command passed with repository szTheory/nesturbator, signer workflow szTheory/nesturbator/.github/workflows/release.yml, source digest e47feb3afc34baf1d611ef44153af199424f41a2 and the default SLSA provenance v1 predicate. Its verified result names the requested asset with the exact downloaded SHA-256 and has the expected release-workflow certificate identity.

The first verifier attempts reported that no Sigstore verifier could be initialized. Diagnosis found the CLI's default TUF cache was not writable in the sandbox. Trusted-root retrieval and attestation verification then succeeded with only XDG_CACHE_HOME redirected to ignored build/phase16-release/cache. Repository, workflow, source-digest and predicate constraints remained unchanged. No global CLI/configuration change was made.

The previously published v0.1.10 archive has SHA-256 9226ddce3e95c4bb0a2b13d910e34c225f67c0a02e24ce550b315e551235142d; it was not used for any v0.1.11 check.

Task 1 records the macOS arm64 release path. Task 2 below expands the same source-bound checks to all six platforms and all 18 downloaded ZIPs.

## Task 2: six-platform package and attestation matrix

Each platform directory contains exactly its downloaded library, runner and libretro ZIPs, byte-compared with the original download. The immutable release snapshot's existing check_archives.cmake ran separately in each directory. Each call exited 0 and returned exactly three expected archive-ok records. The six libretro metadata entries were extracted from the downloaded archives; each SHA-256 equals the tagged source info digest above. The same checker applied info_claims.cmake to the immutable source README and public header and accepted the supported MMC3 profile and both exclusions on every platform.

| Platform | Library ZIP SHA-256 | Runner ZIP SHA-256 | Libretro ZIP SHA-256 | Tagged info SHA-256 | Archive/support checker |
| --- | --- | --- | --- | --- | --- |
| linux-x64 | 58247066cc5b2ee1cf2cf23ec956de956e856c109ffcb2e6bfcd512eafd445e2 | 8dfdb69a70d73931fb003ab23bc3ba9ac9310a2cd47a2231563f5a6c5d0c29ff | f60ae97874094b69a4237d9b17cba0125034390f1e729cbc8d7d8fda567cd02a | 524bb3d0dbfccea4a2ed93316fadbf2b89be1c48b927d7ec007ba3a411fe0711 | pass, 3/3 archives |
| linux-arm64 | a0a3e244e5abadaea047067af3bae0c7d0e3852e9e726ce555d27485d8aaeaa7 | 57910ba76b89990a967411a95469ea629ee3b497bbd6063e7c370699e119db3b | c4cb1b54749047881feb218bd43c905e810d41b73f4f655c7b7d4e19fd972b93 | 524bb3d0dbfccea4a2ed93316fadbf2b89be1c48b927d7ec007ba3a411fe0711 | pass, 3/3 archives |
| macos-x64 | c26091b4042e301f740cc925ea52c0ca28ceb832427068a7c022255735917874 | da41fc571f9502dd7eaad3b3483e544230d6a9e62251f76423183bac53b359db | 0309ec832b4eade4219a2cc055a1e1b5f53822dc56fc64d5e2f02145338f85e6 | 524bb3d0dbfccea4a2ed93316fadbf2b89be1c48b927d7ec007ba3a411fe0711 | pass, 3/3 archives |
| macos-arm64 | 7ccf582590f921da7a3af75795226ce5ad04f8f9840c2cb16f1c9767e6db59a8 | 9381be4f3b9d7c13eed2372b07f89b4728a8c091b0c7b3a5ce270dd610da7943 | fb9c631818d7196d9306f5357f74d281af97ad87dd27066c74e47bbb9bc1dc0f | 524bb3d0dbfccea4a2ed93316fadbf2b89be1c48b927d7ec007ba3a411fe0711 | pass, 3/3 archives |
| windows-x64 | f85db2e6a9adca6faa37a0e19c8cb7e7c07f281109581389b17b20db0b72ee35 | 6b370a08301cc270b106e951a1ab053ced647293264c16c0283f06798c7e3f8e | 6cba2756660c6d524a46481d63d3a37baef7f23175265e15d178ed039e257b6e | 524bb3d0dbfccea4a2ed93316fadbf2b89be1c48b927d7ec007ba3a411fe0711 | pass, 3/3 archives |
| windows-arm64 | 72e1f358ee1f2d2aa89ba3884c08f316591d43980e103a2762da26e0773e9c8d | 2a8a6c9bfa3f59eda194d3352ad07ea2b1251524cab0f067b44bc39f5f01fc23 | e6011af5bf696dd073b401fb92b17be0fa56616faab137f93200deb93f6af89a | 524bb3d0dbfccea4a2ed93316fadbf2b89be1c48b927d7ec007ba3a411fe0711 | pass, 3/3 archives |

Attestation verification ran once for each of the 18 actual ZIP files with the same repository, signer workflow, source commit and default SLSA provenance v1 policy. All 18 commands exited 0 and returned a nonempty verified JSON result. Each result contains the requested filename and its SHA-256 computed from the downloaded bytes; each verified certificate identifies source repository https://github.com/szTheory/nesturbator, source digest e47feb3afc34baf1d611ef44153af199424f41a2, and signer workflow szTheory/nesturbator/.github/workflows/release.yml on refs/heads/main. The final check found 18 distinct ZIP identities, six libretro rows, no missing result, and no subject digest mismatch.

The complete local result set remains ignored under build/phase16-release: six logs named checks/archive-<platform>.log, the 18 per-asset checks/<asset>.verified.json responses, manifest-validation.json, downloaded assets and the immutable source snapshot. The downloaded binary assets and local verification output are not committed.

Final requery passed after the matrix: the published release is still non-draft v0.1.11 with the same publication time and all 19 asset identities; the remote tag remains the direct commit e47feb3afc34baf1d611ef44153af199424f41a2; Release run 39993108038 remains the successful push run on that exact SHA.

All planned released-byte conditions are now evidenced for v0.1.11: 19 published assets, 18 checksum-matched downloaded ZIPs, 18 exact-source attestations and six metadata-equal archive trios. This evidence is ready for the normal independent phase verification; this record does not preempt that verifier's verdict.

## Separate Nightly result

The targeted Nightly run 39993108154 completed with an overall failure on the same commit. Its vectors-full job failed and suite-flake succeeded. This is a separate Nightly result, not the successful required Release workflow, and is not represented as a release or universal pass.

## Commands and retained local evidence

Read-only release identity and transport commands:

    gh api repos/szTheory/nesturbator/releases/422111807
    gh api repos/szTheory/nesturbator/git/ref/tags/v0.1.11
    git fetch --no-tags origin refs/tags/v0.1.11
    git merge-base --is-ancestor b39a0106e4cccfe9922e051d5680f1f7f84f5efa e47feb3afc34baf1d611ef44153af199424f41a2
    gh run view 39993108038 --repo szTheory/nesturbator
    gh release download v0.1.11 --repo szTheory/nesturbator --dir build/phase16-release/assets
    shasum -a 256 -c SHA256SUMS

Task 1 platform and attestation commands, both exit 0:

    cmake -DPACKAGES=build/phase16-release/platforms/macos-arm64 -DVERSION=0.1.11 -P build/phase16-release/source/tests/cmake/check_archives.cmake
    XDG_CACHE_HOME="$(git rev-parse --show-toplevel)/build/phase16-release/cache" gh attestation verify build/phase16-release/assets/nesturbator-0.1.11-libretro-macos-arm64.zip --repo szTheory/nesturbator --signer-workflow szTheory/nesturbator/.github/workflows/release.yml --source-digest e47feb3afc34baf1d611ef44153af199424f41a2 --format json

Task 2 ran the same archive command separately for linux-x64, linux-arm64, macos-x64, macos-arm64, windows-x64 and windows-arm64, each with its matching build/phase16-release/platforms/<platform> directory; all six exited 0. The attestation command above ran for each of the 18 ZIP paths with the same XDG_CACHE_HOME and identity arguments; all 18 exited 0.

Actual downloaded ZIPs, attestation JSON, source snapshot and checker logs remain ignored under build/phase16-release. No release assets, version, source code, verification fingerprint or phase requirements were changed.
