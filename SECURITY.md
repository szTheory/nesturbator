# Security policy

## Reporting a vulnerability

Report a vulnerability privately through GitHub's private vulnerability
reporting: open the repository's **Security** tab and choose **Report a
vulnerability**. Please do not open a public issue or pull request for it.

The report reaches the maintainer only. Include what you found, how to
reproduce it (a command line, an input file described or attached), and the
version or commit you used.

## Scope

- The C library: `include/nesturbator.h` and everything under `src/`.
- The headless runner, `nesturbator-run`.
- The libretro core, `nesturbator_libretro`, and its `.info` file.
- The release archives and their `SHA256SUMS` and build-provenance
  attestation.

A malformed game image or input that makes the library, the runner or the
libretro core read or write out of bounds, crash or hang is in scope. Problems
in a frontend such as RetroArch belong to that frontend's project.

## Fixes

A fix is released as a new version through the normal release process, and the
changelog (`CHANGELOG.md` and the GitHub release notes) says what it fixes.
Only the latest release receives fixes.
