---
title: "Engineering: C rules, build, tests, CI and release"
summary: "Build, test, CI and release decisions for nesturbator, each with its reason and source."
read_when: "When creating or changing the build, tests, CI, repository settings or release pipeline."
updated: 2026-10-02
---

# Engineering: C rules, build, tests, CI and release

## Why this file exists
No code exists yet, so these choices are made once. Each has its reason and source: a sibling repository at a pinned commit, official documentation read on 2026-10-02, or a local experiment run that day [ENG.09].

## 1. C rules
`CMAKE_C_STANDARD 17` with `CMAKE_C_EXTENSIONS OFF` emitted `-std=c17` locally; GCC otherwise defaults to `gnu23`, and MSVC gets `/std:c17`, without VLAs, atomics or threads [ENG.09][ENG.10][ENG.11].

| Rule | Reason | Check |
|---|---|---|
| Unsigned fixed-width guest arithmetic, cast to `uint32_t` before `<<` or `*`; no signed shifts | promotion made `uint16_t * uint16_t` overflow as `int` under UBSan [ENG.09] | `asan` |
| One bus access per statement | operand order is unspecified and unwarned [ENG.09] | CPU vectors |
| Byte-wise little-endian helpers for file, state and hash data; no packed structs or bit-fields | layout, signedness and width vary by target [ENG.07] | cross-platform hashes |
| No mutable globals or statics | independent instances [ENG.07] | `nm`: no `B b D d C` symbol in the release static library [ENG.09] |

**Integer-only** covers the core target; `libretro.h` declares `double fps`, so the adapter converts at the boundary [ENG.18]. Three checks combine [ENG.09]: a text scan of `src/` and `include/` for `float`, `double` and floating literals (constant folding leaves nothing in an object); preset `nofp`, which builds the core with `-mgeneral-regs-only` (GCC and Clang, AArch64 and x86) [ENG.11][ENG.12]; and the undefined-symbol test on that library. Under the flag Apple clang still compiled local `double` arithmetic, as `__muldf3`-style calls, which the symbol test catches; without the flag those calls do not exist.

**Undefined-symbol allowlist.** A `cmake -P` script runs `nm -u` on the release static library as a CTest test; the prototype failed on `printf` and soft-float calls [ENG.09]. Allowed: `memcpy memmove memset memcmp`, which GCC requires even when freestanding [ENG.11], plus per-format compiler helpers.

| Compiler | Warnings |
|---|---|
| GCC, Clang | `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wstrict-prototypes -Wmissing-prototypes -Wvla` |
| GCC | adds `-Wduplicated-cond -Wlogical-op` |
| MSVC | `/W4` |

Presets set `CMAKE_COMPILE_WARNING_AS_ERROR`, so embedding consumers are unaffected. `asan` is `-fsanitize=address,undefined -fno-sanitize-recover=all` in one lane; a private sibling project uses two [ENG.01].

## 2. Build
CMake 3.25 is the minimum: the first release with workflow and package presets (schema 6) [ENG.10]; every runner image has 3.31.6 or newer [ENG.14].

| Preset | Content |
|---|---|
| `dev` | Debug |
| `ci` | Release, warnings fatal; tests, install, consumer build, archive |
| `ci-msvc` | `ci` built with MSVC. Ninja needs the MSVC environment loaded first, so either a setup step loads it (recommended: no Visual Studio version is named, and Windows can then use `ci` itself) or the preset names a Visual Studio generator; section 5 gives the image versions |
| `asan` | Clang with ASan and UBSan |
| `nofp`, `hygiene` | labels `abi`, `hygiene`; the floating-point scan and the symbol test run under `nofp`; a missing tool fails configure |

**Entrypoint: `cmake --workflow --preset <lane>`, no `scripts/ci.sh`.** One command ran configure, build and test locally, and `cmake --list-presets=workflow` lists the lanes [ENG.09]. A `scripts/ci.sh`, as in that sibling project, needs a 218-line parity script to keep its lane array equal to presets and matrix, and bash on Windows [ENG.01]. The cost is one command per lane and only configure, build, test and package steps [ENG.10], so hygiene checks are CTest tests. A script wins if a lane needs two configurations in one command.

Naming follows the sibling's convention [ENG.07]: target `nesturbator`, alias `nesturbator::nesturbator`, `NESTURBATOR_` options, `nesturbator_` symbols, a relocatable CMake package, `nesturbator.pc` and an `install.consumer` test. The runner is `nesturbator-run`; the adapter is module `nesturbator_libretro` beside `nesturbator_libretro.info` [ENG.18][ENG.19].

`.clang-format` is checked in `hygiene` with clang-format 18 from the Ubuntu image; none is installed locally [ENG.14].

## 3. Tests without a framework dependency
Tests are plain C executables under CTest with an in-repo `tests/check.h` of comparison macros. Vendoring Unity as the sibling did (2.7.0) loses: `libretro.h` is the only vendored file, and CTest already gives selection, parallelism, timeouts and JUnit output. A need for mocking would reopen it [ENG.07].

| Label | Tests | Notes |
|---|---|---|
| `vectors` | `cpu.vectors.00` to `.ff` | the committed sample of SingleStepTests `nes6502/v1` (MIT), with per-cycle bus checks; the full 1.08 GB set runs nightly [ENG.17][ENG.19] |
| `rom` | `rom.<suite>.<name>` | committed test ROMs run by `nesturbator-run` to their own verdict |
| `framehash` | `framehash.<rom>.<script>` | SHA-256 frame and audio hashes |
| `vectors-full` | the full 65x02 set, fetched at its pin | nightly; outside the `ci` preset |
| `retroarch` | `retroarch.testframe` | launches RetroArch unattended; reports skipped where RetroArch is absent |

Expected results are the scoreboard file CONFORMANCE.md defines; a test fails when its result is worse than recorded [ENG.19]. Test presets set `noTestsAction: error` [ENG.09].

## 4. Fuzzing and property-style tests
Targets use `LLVMFuzzerTestOneInput`: the ROM loader first; the save-state loader and a bounded run of arbitrary ROM bytes in later milestones (PERF-01). libFuzzer runs under Clang 18 on Linux x64 in the nightly workflow [ENG.12]. Apple clang has no libFuzzer runtime, so every other leg links a standalone `main` replaying the committed corpus as `fuzz.regress.<target>` [ENG.09]. Property tests, also later, use an in-repo integer PRNG and a printed seed: N then M frames equals N+M; save, load, continue equals uninterrupted [ENG.07].

## 5. CI design
`ci.yml` runs on `pull_request`, `push` to `main`, `workflow_dispatch` and `workflow_call`, using committed files only. A scheduled `nightly.yml`, outside the required check, runs the full vectors, fetched suites and fuzzing [ENG.19].

| Job | Runner | Content |
|---|---|---|
| `hygiene`, `title` | `ubuntu-24.04` | sections 7 and 6 |
| `build` | `ubuntu-24.04`, `ubuntu-24.04-arm` (GCC 14); `macos-15`, `macos-15-intel` (Apple clang 17); `windows-2025` (Visual Studio 2026, 18.10), `windows-11-arm` (Visual Studio 2022, 17.14, due to move to 2026) [ENG.13][ENG.14][ENG.21] | `ci` or `ci-msvc`; uploads archive and `hashes.txt` |
| `asan`, `nofp` | `ubuntu-24.04`; `nofp` also `-arm` | Clang 18; GCC 14 |
| `hash-equality` | `ubuntu-24.04` | needs exactly the expected hash files, byte-identical |
| `perf` (later milestone, PERF-02) | `ubuntu-24.04` | head and merge-base under Cachegrind; `Ir` reported [ENG.12] |

Six legs run per pull request because cross-platform identity is a product claim; three, with the rest on `main`, is the alternative if measured queueing dominates. The Windows images are changing: `windows-2025` has Visual Studio 2026 and no 2022, and `windows-11-arm` is scheduled to follow [ENG.21]. A preset that names the `Visual Studio 17 2022` generator does not configure on the new image, and the `Visual Studio 18 2026` generator needs CMake 4.2 [ENG.10].

- **Required check.** `CI required` has `if: always()` and passes only when `jq` finds every `result` in `toJSON(needs)` equal to `success`, because GitHub reports a skipped job as "Success" and skips a job whose dependency failed [ENG.03][ENG.13].
- **No trigger-level path filter.** A workflow skipped by `paths` leaves its checks "Pending" and blocks merging [ENG.13].
- **Concurrency.** `cancel-in-progress` is true only for pull requests, so each `main` commit keeps a result; `release.yml` has its own group, because a called workflow sees the caller's context [ENG.03][ENG.04].
- **Timeouts** are twice a measured cold duration, with the run recorded in a comment; the default is 360 minutes [ENG.03][ENG.13].
- **Pins.** Every `uses:` is a full SHA (threadline uses tags): the setting `sha_pinning_required` rejects anything else [ENG.13]; a `hygiene` test reads every `uses:` key, since scrypath's pattern matches only `- uses:` and sees 50 of its 60 lines [ENG.05]; Dependabot covers `github-actions` alone [ENG.01].
- **Caches.** No compiler cache at first (ccache is on no image); a later one is keyed by preset, runner label and compiler version, not `runner.os` [ENG.03]. The nightly vector cache is keyed by upstream commit; entries unused for 7 days expire [ENG.13].

## 6. Release
| Key | Value | Reason |
|---|---|---|
| `release-type` | `simple` | maintains `CHANGELOG.md`, `version.txt` |
| `extra-files` | the public header | version macros marked `x-release-please-major`, `-minor`, `-patch`, `-version` |
| `bump-minor-pre-major`, `bump-patch-for-minor-pre-major` | both `true` | as scrypath; `false` sends a breaking 0.x change to 1.0.0, contrary to the skill's note [ENG.05][ENG.06] |
| `draft`, `force-tag-creation` | both `true` | drafts are untagged until published; the second key (17.2.0) tags at once |
| manifest; `release-as` | `0.0.0`; `0.1.0` once | the skill's first-release traps [ENG.06] |

Keys are from release-please's own files [ENG.15]; action v5.0.0 bundles 17.6.0 [ENG.16]. CMake reads `version.txt`; any other tracked version string carries a marker, since lattice_stripe needed an extra job for strings release-please could not write [ENG.02].

**Token: a GitHub App installation token.** With `GITHUB_TOKEN`, runs on the release pull request wait for approval; GitHub's remedy is an App token or a personal access token [ENG.13]. With an App, third-party actions hold only a short-lived token revoked at job end [ENG.16]. The fine-grained token of lattice_stripe and threadline is the proven alternative if an App is unwanted [ENG.02]. Explicit dispatch needs no credential but doubled CI in threadline [ENG.03].

**Flow.** Native auto-merge (`gh pr merge --auto --squash`) merges the release pull request once `CI required` passes on its head; lattice_stripe's 239-line merger is the alternative [ENG.02]. On `release_created` the run calls `ci.yml` and publishing `needs` it, so success on the exact commit is structural [ENG.04]. Publishing requires the exact expected archive set, writes `SHA256SUMS`, attests it with `actions/attest`, uploads to the draft, runs `gh attestation verify`, confirms a byte-flipped copy fails, then publishes: GitHub's order for immutable releases [ENG.04][ENG.13][ENG.16].

**Artifacts.** Per target (Linux, macOS and Windows, each x64 and arm64): `nesturbator-VERSION-libretro-OS-ARCH.zip` holding `cores/`, `info/` and `LICENSE`, plus library and runner archives [ENG.19]. macOS binaries carry only the linker's ad-hoc signature; keepling is the reference for signing and notarisation [ENG.08][ENG.09].

**Rulesets as code.** `.github/rulesets/main.json` pins the check to the Actions app and requires linear history, squash merges, zero approvals and no bypass; a scheduled workflow compares live rules with the file [ENG.03].

**Title lint.** Job `title` matches the live title from `gh pr view` against lattice_stripe's type list, so a re-run passes after an edit [ENG.02]; listening to the `edited` activity type starts that re-run when the title changes [ENG.13]. `squash_merge_commit_title` is `PR_TITLE`, so the linted title is the commit subject [ENG.13]. `pull_request_target` is not used: the private sibling project bans it, and a second workflow cannot join the single check [ENG.01].

## 7. Repository hygiene and legal files
The tree already holds `LICENSE`, `README.md`, `.gitignore`, `AGENTS.md`, `scripts/hygiene.sh` and `.githooks/` [ENG.20]. `PROVENANCE.md` and `ASSET_POLICY.md` join the first commit; `SECURITY.md` (private vulnerability reporting), `CONTRIBUTING.md` and `THIRD-PARTY-NOTICES.md` arrive in the first phase.

`PROVENANCE.md` predates the code because a policy cannot vouch for earlier history [ENG.01]. Modelled on the private sibling project's, it expands `AGENTS.md` rule 4: hardware facts may enter, restated with a citation; another emulator's code, comments, names and structure may not, even paraphrased by an assistant; assistants are prompted from specifications; their involvement is recorded in the pull-request description; each vendored file lists upstream, commit and SHA-256. `ASSET_POLICY.md` expands rule 3 and states that hashes and mapper numbers are facts, not content.

`scripts/hygiene.sh` is the forbidden-content scan: POSIX shell over git that rejects home-directory paths, email addresses, ROM and save extensions and iNES or FDS magic, excepting `tests/roms/manifest.txt`. Hooks run its `--staged` and `--history` modes; the `hygiene` job runs `--tree` [ENG.20]. It gains one rule from that project's scanner: a tracked file git treats as binary fails unless the manifest lists it, which also stops save states, raw dumps and screenshots [ENG.01]. The library needs no secrets; local automation reads a git-ignored `.env.local`, and CI uses Actions secrets [ENG.07].

## 8. Practices carried over
| Practice | Proven in | Here; disagreements |
|---|---|---|
| One roll-up required check | threadline, lattice_stripe: `.github/workflows/ci.yml` | exifcleaner-node requires 19 contexts and needed no-op matrix legs |
| No trigger path filter | threadline: `.github/workflows/ci.yml`; exifcleaner-node: `docs/ci-budget.md` | lattice_stripe has `paths-ignore` on push |
| Cancel only superseded pull-request runs | threadline: `.github/workflows/ci.yml` | the private sibling project and lattice_stripe cancel on every event; exifcleaner-node never |
| Release calls CI, attests, rejects a tampered copy | exifcleaner-node: `.github/workflows/release.yml` | lattice_stripe, threadline poll for CI; scrypath releases with `GITHUB_TOKEN` |

## 9. What the sibling core's first day left missing
The sibling Neo Geo core at `6d24c3f` [ENG.07]: CMake minimum 3.20 against a planned 3.24; no presets; no warning flags and no `-std` flag in its build files; no `.github/`, remote or release configuration; no `src/`, `include/`, product library target or install rules; a default configure builds nothing and registers 0 tests; CTest needs Python 3; Unity 2.7.0 and a third-party CPU are vendored; no `.clang-format`, `SECURITY.md` or policy files.

## Open questions
- ELF and COFF helper symbols, and whether const pointer tables are `d` in position-independent ELF: first Linux and Windows `abi` run.
- GCC and MSVC behaviour for the warning sets and `-mgeneral-regs-only`: first CI run with a floating fixture.
- Whether App-enabled auto-merge emits the `push` that starts `release.yml`: first release pull request; `gh workflow run` is the fallback.
- Whether `draft` with `force-tag-creation` works in the bundled 17.6.0: first release.

## Sources
| ID | Tier | Source | Pin or access date | Supports |
|---|---|---|---|---|
| ENG.01 | T1 | A private sibling C++ project: `scripts/`, `CMakePresets.json`, `.github/`, `PROVENANCE.md`, `ASSET_POLICY.md`, `tools/forbidden-content-scan/` | a9859b6 | CI, policies, scan |
| ENG.02 | T1 | lattice_stripe: `.github/workflows/`, `release-please-config.json` | 66379aeb | release, title |
| ENG.03 | T1 | threadline: `.github/`, `bin/verify-repo-hygiene`, `bin/verify-branch-protection` | 7966fb49 | CI, ruleset |
| ENG.04 | T1 | exifcleaner-node: `.github/workflows/`, `docs/ci-budget.md` | 41ea275 | matrix, release |
| ENG.05 | T1 | scrypath: `.github/workflows/`, `release-please-config.json` | 11ab1c9 | pins, bump flags |
| ENG.06 | T1 | bootstrap-elixir-hex-lib skill: `SKILL.md` | 2026-10-02 | release traps |
| ENG.07 | T1 | The sibling Neo Geo core (unpublished): `CMakeLists.txt`, `experiments/cpu/`, `third_party/unity/`, `.planning/preparation/`, git log | 6d24c3f | state, conventions |
| ENG.08 | T1 | keepling: `.planning/phases/` | 274bfe0 | signing |
| ENG.09 | T1 | Local runs: Apple clang 21, CMake 4.4.3 | 2026-10-02 | experiments |
| ENG.10 | T1 | https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html; CMake 4.4.3 manuals | 2026-10-02 | presets |
| ENG.11 | T1 | https://gcc.gnu.org/onlinedocs/gcc/ (AArch64-Options, x86-Options, Standards); https://learn.microsoft.com/cpp/build/reference/std-specify-language-standard-version | 2026-10-02 | compilers |
| ENG.12 | T1 | llvm-project `clang/include/clang/Options/Options.td` (fe1de64cb3c2); https://llvm.org/docs/LibFuzzer.html; https://valgrind.org/docs/manual/cg-manual.html | 2026-10-02 | tools |
| ENG.13 | T1 | github/docs (https://docs.github.com): runners, `GITHUB_TOKEN`, checks, workflow syntax, caching, security, releases, REST | 87cce24ea55d | GitHub |
| ENG.14 | T1 | actions/runner-images: `images/` readmes | 6d942e630479 | tool versions |
| ENG.15 | T1 | googleapis/release-please: `docs/`, `schemas/config.json`, `src/` | edce3d805ef3 | configuration |
| ENG.16 | T1 | Actions: release-please-action v5.0.0, create-github-app-token v3.2.0, attest v4.2.2; `gh attestation verify --help` | 2026-10-02 | actions |
| ENG.17 | T1 | SingleStepTests/65x02: `nes6502/` | 2f6980a2d957 | vectors |
| ENG.18 | T1 | RetroArch `libretro-common/include/libretro.h`, the copy to vendor; libretro-core-info `00_example_libretro.info` | v1.22.2; 5a74858ab2f7 | header, `.info` |
| ENG.19 | T2 | This directory: `CONFORMANCE.md`, `LIBRETRO-AND-RUNNER.md`, `ARCHITECTURE.md` | 2026-10-02 | tiers, runner, artifacts |
| ENG.20 | T1 | nesturbator working tree: `scripts/hygiene.sh`, `.githooks/`, `.gitignore`, `AGENTS.md` | 2026-10-02 | tree |
| ENG.21 | T1 | actions/runner-images: issues 14017 and 14602; `images/windows/Windows2025-VS2026-Readme.md` | 2026-10-02 | Windows images |
