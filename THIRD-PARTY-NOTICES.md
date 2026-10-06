# Third-party notices

nesturbator is MIT-licensed (see [LICENSE](LICENSE)). Two things in this
repository come from elsewhere: the vendored `libretro/libretro.h`, and the
CPU test data under `tests/vectors/`.

## libretro.h

- File: `libretro/libretro.h`, unmodified
- Upstream: https://github.com/libretro/RetroArch, `libretro-common/include/libretro.h`
- Commit: 69a4f0ea1e8aaf442ae4858f2e7f2b31a1776576 (tag v1.22.2)
- SHA-256: bd3398d29c3763d18617087020ff56b5450a48a63123403a4b674f8e79947acb

Its licence, as stated in the file:

```
The following license statement only applies to this libretro API header (libretro.h).

Copyright (C) 2010-2024 The RetroArch team

Permission is hereby granted, free of charge,
to any person obtaining a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
```

## SingleStepTests 65x02

CPU test vectors (test data, not code), converted to N65V by `tools/vecconv`
or cut as byte-range prefixes of the upstream JSON.

- Upstream: https://github.com/SingleStepTests/65x02, `nes6502/v1/`
- Commit: 2f6980a2d95757486c7bee24355c360e40e2a224

| File | What it is | SHA-256 |
|---|---|---|
| `tests/vectors/65x02-sample.n65v` | the first 100 tests of each of the 256 files, as N65V | 0c318cec0bd4031396964ca9a0de0daf25c2651c28f34a0d0616a9294430e109 |
| `tests/vectors/fixtures/a9-first3.json` | a byte-range prefix of one file, see `tests/vectors/fixtures/README.md` | 3195bc1b6d17fb002f8d0a4f8f5b99077d42d62f79e35161d9de1f7d4bf1d83b |
| `tests/vectors/fixtures/02-first3.json` | a byte-range prefix of one file, see `tests/vectors/fixtures/README.md` | 2738dad0c19bc0e4dc4d3a131a7cd3768a1493df9ec665b0e738fa397fbbb58f |
| `tests/vectors/fixtures/a9-tail.json` | a byte-range prefix of one file, see `tests/vectors/fixtures/README.md` | 6d60b42a8fca08c75e7e3ea364085be67ea786c9d15e279e6fe9e4790ab42ddc |

Its licence, the upstream `LICENSE` at that commit:

```
MIT License

Copyright (c) 2024 Thomas Harte et al

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
