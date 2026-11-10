# Reproduce the qualified NTSC scenario

Use the installed public C package and the fixed permitted DABG image and
recipe in [nrom-gameplay.md](nrom-gameplay.md). Verify its manifest digest
before running. The scenario uses the ordinary loader and two frame-boundary
logical input masks. It requires source-mapped game progress, visible native
pixel response and non-silent PCM; hashes alone cannot satisfy it.

After a configured build, `ctest --test-dir build/ci -R '^install.gameplay$'
runs the moved-prefix scenario and prints the owned scratch directory that
contains `gameplay.json`, `no-input.json`, `start-only.json` and provenance.
To check its source/runtime contract, use `cmake -DSOURCE_DIR=.
-DMODE=contract -DRUNTIME_REPORT=<fresh-gameplay.json>
-P tests/cmake/provider_profile.cmake` with the actual printed report path.

The checked [profile](provider-ntsc.md) remains candidate-only until the
published receipt binds source, version, archive digests and attestations.
Provider qualification does not establish LibFuturo's separate relocated
consumer, native host or gameplay acceptance, physical pacing, hardware
correctness or Playstead behavior.
