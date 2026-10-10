---
phase: 01-a-test-frame-in-retroarch
plan: 11
subsystem: testing
tags: [retroarch, libretro, macos, install, release-please, github-app]

requires:
  - phase: 01-10
    provides: CI matrix, CPack archives and the release-please pipeline with an App token
provides:
  - retroarch.testframe passing on Apple Silicon (RetroArch 1.22.2, unattended, pixel-exact against runner frame 5)
  - README one-line install for RetroArch on Apple Silicon inside a release-please version block
  - tests/cmake/check_install_line.cmake and its CI step on the macOS arm64 leg
  - Public repository szTheory/nesturbator with the nesturbator-release GitHub App credential
affects: [01-12]

actuals:
  tokens: 1700
  tasks: 3
  commits: 2
plan_head_before: 62ddc9718772675c040faee57c6e568a07155d67
plan_head_after: 5951a2cf0c1b17cec05f5804b4117d98f616f580

tech-stack:
  added: [RetroArch 1.22.2 (Homebrew cask retroarch-metal, test-only host)]
  patterns:
    - "Install line is tested by parsing it out of the README and running its tar arguments on the real archive"

key-files:
  created:
    - tests/cmake/check_install_line.cmake
  modified:
    - tests/retroarch/test.cfg.in
    - README.md
    - .github/workflows/ci.yml

key-decisions:
  - "bundle_assets_extract_enable = false in the RetroArch test config: the macOS app otherwise unpacks assets.zip into the user directory on every launch"
  - "The install-line check is a CI step, not a ctest test, because the archives are written after the tests run"
  - "check_install_line.cmake picks the zip whose name carries version.txt's version, and makes OUT absolute"
  - "Owner hand-offs are scripted down to owner-only clicks (PROJECT.md Hand-offs constraint, 62ddc97)"

patterns-established:
  - "RetroArch comparison: video_gpu_screenshot=false plus --max-frames-ss gives a raw 256x240 PNG comparable pixel for pixel"

requirements-completed: [FRAME-05]

coverage:
  - id: D1
    description: "Unattended RetroArch launch on Apple Silicon whose screenshot equals the runner's frame, leaving the user's RetroArch directory unchanged"
    requirement: FRAME-05
    verification:
      - kind: e2e
        ref: "ctest --preset ci -L retroarch --output-on-failure (retroarch.testframe)"
        status: pass
    human_judgment: false
  - id: D2
    description: "README one-line install extracts exactly the core dylib and its .info file from the versioned libretro-macos-arm64 archive"
    requirement: FRAME-05
    verification:
      - kind: integration
        ref: "cmake -DREADME=README.md -DPACKAGES=build/ci/packages -DOUT=<dir> -P tests/cmake/check_install_line.cmake"
        status: pass
    human_judgment: false
  - id: D3
    description: "macOS arm64 CI leg runs the install-line check against the archive it built"
    requirement: FRAME-05
    verification:
      - kind: other
        ref: ".github/workflows/ci.yml step invoking check_install_line.cmake (runs on GitHub in plan 12)"
        status: unknown
    human_judgment: false

duration: 15min
completed: 2026-10-02
status: complete
---

# Phase 1 Plan 11: RetroArch on Apple Silicon and the One-Line Install Summary

**RetroArch 1.22.2 launches the core unattended and its raw 256x240 screenshot matches runner frame 5 pixel for pixel; the README's one-line install is checked against the real libretro-macos-arm64 archive locally and in CI**

## Performance

- **Duration:** about 15 min of execution (excluding the owner hand-off)
- **Started:** 2026-10-02T22:40:50Z
- **Completed:** 2026-10-03T00:35:29Z
- **Tasks:** 3
- **Files modified:** 4

## Accomplishments

- `retroarch.testframe` reports Passed, not Skipped, on the Apple Silicon Mac (re-confirmed 2026-10-02: 1/1 passed in 2.03 s). RESEARCH A8 is settled. The default video driver here is Vulkan through MoltenVK. With `video_gpu_screenshot=false` and `--max-frames-ss`, RetroArch writes a 256x240 PNG that equals runner frame 5 pixel for pixel.
- The README has the one-line Apple Silicon install for szTheory/nesturbator inside an `x-release-please-start-version` block.
- `check_install_line.cmake` reads the tar arguments from the README line, pipes the real archive through them, and passes only when exactly `cores/nesturbator_libretro.dylib` and `info/nesturbator_libretro.info` come out. It also checks that the URL names version.txt's version. It passes locally against v0.0.0.
- `cmake --workflow --preset ci`: 30 of 30 tests passed, and all packages were generated.
- Owner hand-off is done: RetroArch is installed via `brew install --cask retroarch-metal`. The public repository szTheory/nesturbator exists. The GitHub App `nesturbator-release` (id 5169973; Contents and Pull requests read/write; no webhook) is installed on the repository. The variable `NESTURBATOR_APP_ID` and the secret `NESTURBATOR_APP_PRIVATE_KEY` are set. All of this was scripted; the owner made two browser clicks.

## Task Commits

1. **Task 1: Owner hand-off (RetroArch, repository, release App)**: no code commit. The orchestrator checked it on 2026-10-03.
2. **Task 2: RetroArch comparison passes on Apple Silicon** - `a1f3982` (fix)
3. **Task 3: README one-line install and its check** - `5951a2c` (feat)

## Files Created/Modified

- `tests/retroarch/test.cfg.in`: adds `bundle_assets_extract_enable = "false"` so RetroArch leaves the user directory alone
- `README.md`: one-line Apple Silicon install in a release-please version block
- `tests/cmake/check_install_line.cmake`: runs the README's extraction against the real archive
- `.github/workflows/ci.yml`: install-line check step on the macOS arm64 leg

## Decisions Made

See key-decisions in the frontmatter.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] RetroArch wrote outside the build directory**
- **Found during:** Task 2
- **Issue:** The first real run failed the directory-unchanged guard. On every launch the macOS app unpacks its bundled assets.zip into `~/Library/Application Support/RetroArch/assets` (5494 files, only mtimes change).
- **Fix:** Set `bundle_assets_extract_enable = "false"` in test.cfg.in
- **Files modified:** tests/retroarch/test.cfg.in
- **Commit:** a1f3982

**2. [Rule 3 - Blocking] Install check details**
- **Found during:** Task 3
- **Issue:** More than one zip can sit in the packages directory, and a relative OUT broke the extraction directory. The archives are written after the tests run, so a ctest test could not see them.
- **Fix:** The script picks the zip by version.txt's version and makes OUT absolute. The check runs as a CI step after packaging, not as a ctest test.
- **Files modified:** tests/cmake/check_install_line.cmake, .github/workflows/ci.yml
- **Commit:** 5951a2c

### Process change

On 2026-10-02 the owner directed that every hand-off be scripted down to owner-only clicks. This is recorded as the "Hand-offs" constraint in .planning/PROJECT.md (62ddc97). Task 1 was done that way: the agent installed RetroArch and created the repository with the owner's explicit authorization, and a script created the GitHub App through the manifest flow.

## Issues Encountered

None beyond the deviations above.

## User Setup Required

None remaining. The owner completed the RetroArch install and the GitHub setup.

## Next Phase Readiness

Plan 12 can push to szTheory/nesturbator. The App credential is in place for release-please, and the CI install-line step will run for the first time on GitHub.

## Self-Check: PASSED

- FOUND: tests/cmake/check_install_line.cmake, README.md, tests/retroarch/test.cfg.in, .github/workflows/ci.yml
- FOUND: a1f3982, 5951a2c
