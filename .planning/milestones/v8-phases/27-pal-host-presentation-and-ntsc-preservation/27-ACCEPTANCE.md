# Phase 27 Final Acceptance

## Candidate and source identity

- Code candidate: commit `ae2959caaa70fb690689a087eef58df988a63f2d`, tree `e85d7f8f01a8904c86428a89a121992b67ea3da`.
- Library version `0.1.11`, behavior revision `17`, ABI `1`, native state format `6`.
- The source-bound report records a dirty checkout (`source_dirty=1`). Its reason is recorded in `tests/accuracy/baseline-metadata.txt`; the exact 56-input helper digest identifies the measured build inputs.
- Before and after the provenance-only refresh, the strict input set remained 56 files with SHA-256 `40dca65130e677063633bda42059b9d057e08815c313574e77bf45e5b4654e85`.
- The active metadata binds the final candidate to the retained report. `accuracycoin.activation` and the stale-source controls passed in the final workflow.

## AccuracyCoin evidence

The configured `accuracycoin.campaign` fixture completed during the final
workflow. Its 146 scored rows, 22/22 launched and attributed pages, and 22/22
completed pages are recorded at
`build/phase27-acceptance/20261102T201327Z/accuracycoin/full/`. The campaign
report SHA-256 is
`f3b5bbf7ddcf4d18deabda6d875b441330dc71c97b8aca5fdcc38bf471f98f6c`; the
workflow's current report has the same digest. Counts remain 126 pass, 20
fail, 5 inventory-only draws, and zero skipped, not-run, or timed-out rows.
The canonical scoreboard SHA-256 remains
`9ec5d50d291d2713f5281851f642270498a4e8b0dab273b8b72cd19b66ff111b`.

This is an NTSC conformance inventory. It is not PAL or hardware accuracy
evidence. The older interrupted campaign remains stopped and unknown; the
Plan 27-01 capture remains intermediate and is not used as final acceptance.
The preserved historical receipt in `tests/accuracy/measurements.md` remains
byte-for-byte intact.

## Full local workflow

Command: `cmake --workflow --preset ci` with the configured full
AccuracyCoin fixture enabled. It ran on the frozen code candidate above.

- Attempt 1 completed 716/722 checks and failed five hosted-evidence metadata
  readers plus one `accuracycoin.inventory` timeout. The metadata parser
  rejects an equals sign in a property value; the dirty-checkout explanation
  contained `source_dirty=1` and was corrected to plain prose. The inventory
  binary's first invocation stalled during dyld startup without opening its
  input files; its focused rerun passed in 0.02 seconds. The four affected
  hosted-evidence controls and `accuracycoin.baseline_fetch` also passed after
  the metadata correction. Logs and the first attempt's CTest records are
  retained under `build/phase27-acceptance/20261102T201327Z/`.
- Attempt 2 passed all 722/722 CTest checks in 517.90 seconds, including the
  campaign, activation and stale-source checks, and created all three packages:
  library, runner and libretro (`0.1.11`, macOS arm64). The serial
  `retroarch.session.controls` test passed in 271.01 seconds.
- The two attempts and diagnosis/retry remained within the original 30-minute
  outer workflow window and all configured per-test and fixture limits.

After the acceptance and review documents were written, a focused
`ctest --test-dir build/ci -R '^accuracycoin\\.(activation|input_fingerprint)$'`
recheck passed. CTest included its configured campaign fixture (4.87 seconds)
before activation; the regenerated current report remained byte-identical to
the retained report at the hash above. This revalidates that the documentation
closeout did not alter any of the 56 build inputs.

The complete retry log is
`build/phase27-acceptance/20261102T201327Z/workflow-attempt-2.log`; the first
attempt log is
`build/phase27-acceptance/20261102T201327Z/workflow.log`.

## NTSC preservation

The pre-phase comparison against `ad24e05247f4ebe5cb7efb8212b632c2f8cf4f8f`
reported no changes in `src/palette_ntsc.c`, `tests/runner/hashes.txt`, or
`tests/accuracy/scoreboard.txt`. The full workflow also passed the retained
NTSC regression and scoreboard checks. No NTSC baseline was re-pinned.

## Scope limits

The deterministic PAL RGB bytes identify the named NESdev 2C07 software model;
the palette generator formats pinned published RGB values rather than
independently implementing the analog decoder. This limitation is documented
in the public API, README and testing guide. The acceptance does not claim
silicon color capture, universal display calibration, physical host pacing, or
game compatibility on a hardware host. The descriptor-less fallback rows and
transparency prohibitions carried from the prior milestone remain flagged.

