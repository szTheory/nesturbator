# Reproduce the qualified NTSC scenario

Use the installed public C package and the fixed permitted DABG image and
recipe in [nrom-gameplay.md](nrom-gameplay.md). Verify its manifest digest
before running. The scenario uses the ordinary loader and two frame-boundary
logical input masks. It requires source-mapped game progress, visible native
pixel response and non-silent PCM; hashes alone cannot satisfy it.

Configure and build the source, then run the moved-prefix scenario:

```sh
cmake --preset ci
cmake --build --preset ci --parallel 4
ctest --test-dir build/ci -R '^install.gameplay$' --output-on-failure --timeout 60
```

The test prints the owned scratch directory containing `gameplay.json`,
`no-input.json`, `start-only.json` and package provenance. Check the runtime
contract with its actual fresh report path:

```sh
cmake -DSOURCE_DIR=. -DMODE=contract \
  -DRUNTIME_REPORT=build/ci/qualified-consumer-ACTUAL/gameplay.json \
  -P tests/cmake/provider_profile.cmake
```

Replace `qualified-consumer-ACTUAL` with the directory printed by the test.

The checked [profile](provider-ntsc.md) remains candidate-only until the
published receipt binds source, version, archive digests and attestations.
Provider qualification does not establish LibFuturo's separate relocated
consumer, native host or gameplay acceptance, physical pacing, hardware
correctness or Playstead behavior.
