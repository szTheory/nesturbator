# Vector fixtures

Small byte-range prefixes of the SingleStepTests 65x02 vectors, used by the
`vecconv` tests. JSON cannot carry a comment, so the attribution for each file
is here. Each file also has a line in `tests/roms/manifest.txt` with its
SHA-256.

| File | Upstream path | Bytes | Contents |
|------|---------------|-------|----------|
| `a9-first3.json` | `nes6502/v1/a9.json` | 0-983 | the first three tests of LDA immediate, ending at the third object's closing brace |
| `02-first3.json` | `nes6502/v1/02.json` | 0-1591 | the first three tests of JAM `$02` in the compact layout (`[{`), ending at the third object's closing brace |
| `a9-tail.json` | `nes6502/v1/a9.json` | 0-1083 | three whole tests of LDA immediate and the start of the fourth, cut inside its `initial` object |

- Source: https://github.com/SingleStepTests/65x02
- Pin: `2f6980a2d95757486c7bee24355c360e40e2a224`
- Fetched with `file(DOWNLOAD <raw URL at the pin> RANGE_START 0 RANGE_END <last byte> TLS_VERIFY ON)`;
  `RANGE_END` is inclusive.
- `tests/cmake/vecconv_negative.cmake` builds the malformed inputs for the
  `vecconv.reject.*` tests from `a9-first3.json` at test time; they are not
  committed.
- Licence: MIT. The upstream `LICENSE` at the pin, verbatim:

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
