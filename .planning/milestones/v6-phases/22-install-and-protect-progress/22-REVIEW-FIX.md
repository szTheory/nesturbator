---
phase: 22
status: fixed
findings_fixed: 3
findings_skipped: 0
---

# Phase 22 review fixes

## Fixed Issues

### WR-01: Apple Silicon install command assumes RetroArch’s default directories

README now identifies the literal command as using the default macOS data
directory and directs custom-directory users to the configured-directory manual
route. The release-marked block is unchanged. The install-line extraction check
passed against the local package after this clarification.

### WR-02: Release guide names the wrong attestation subject

The guide now says the checksum entries identify the archive subjects for
build-provenance attestations. This matches `subject-checksums` and the per-ZIP
verification loop in `.github/workflows/release.yml`. This correction does not
claim a successful local Sigstore verification.

### WR-03: Status-table checker does not enforce the canonical API guide path

The checker normalizes its source/document inputs and requires the API guide
under that source root, matching the existing runner-consumer pattern. A new
isolated control passes the otherwise-correct stale README as DOC. It failed
before the guard with `accepted noncanonical stale README path`, then passed
after the guard rejected that path for its specific diagnostic. Existing
missing-guide, row, heading and warning controls also passed.

## Validation

After the corrections, direct production status-table and self-test invocations,
README link and link self-test invocations, and the macOS install-line extraction
check passed. These used independent scratch roots while the full CI workflow
was running; no competing CTest run wrote its result directory. No runtime
code, public header, package layout or version marker changed.
