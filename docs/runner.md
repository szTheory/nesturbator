# Runner guide

## The runner

`nesturbator-run` runs the core without a window and accepts a mapper 0, 1, 2, 3, 4 or 7 image
with `--rom FILE`. The loader validates the entire image before allocating
cartridge state. It rejects unsupported mapper, console, region, RAM and ROM
geometries, truncation, trailing bytes, and images larger than 64 MiB; the
runner prints a diagnostic on stderr and exits 1 for rejected content. It names the
reason: `nesturbator-run: four-screen cartridges are not supported`,
`nesturbator-run: MMC6 (mapper 4, submapper 1) is not supported`, or
`nesturbator-run: mapper N is not supported; supported mappers are 0, 1, 2, 3, 4 and 7`.
Any other refusal (a supported mapper with a refused size, or a file shorter than the
16-byte header) gets the generic `malformed or unsupported cartridge` line naming the same six
mappers.

```sh
nesturbator-run --frames N [--rom FILE] [--hash-frame N]... [--hash-audio] [--dump-frame N:FILE]...
                [--save-dir DIR [--save-interval N]]
                [--state-at-frame N:FILE]... [--hash-state N]... [--load-state FILE]
                [--raise-after N[:SIG[:K]]]
nesturbator-run --version
```

- `--frames N` runs N frames (N is 1 or more). One of `--frames`, `--movie` or `--bench` is
  required; without one the runner prints its usage and exits 2.
- `--bench N` runs frames 1 to N (N from 1 to 1000000) with `--rom`, times each frame, and prints
  frame-time statistics to stderr. Standard output and every hash are exactly as with
  `--frames N` (each hash line is flushed as it is printed), so `--hash-frame K` (1 to N) and `--hash-audio` work the same, and a test shows
  the output is byte-identical. Only the `nesturbator_run_frame` call sits between the two clock
  readings (a monotonic clock: `CLOCK_MONOTONIC`, or `QueryPerformanceCounter` on Windows), so the
  timer's own overhead is included in each sample. There is no warm-up: the first frames show in
  `max`. `--bench` cannot be combined with `--frames` or `--movie` and needs `--rom`; any other
  misuse (0, above 1000000, given twice) exits 2. It also refuses `--accuracycoin-page`,
  `--raise-after`, `--dump-frame`, `--state-at-frame`, `--hash-state`, `--save-dir`,
  `--save-interval` and `--load-state`, each with exit 2 and its own message, because file I/O and
  loaded starts would pollute the timing. Example stderr:

  ```
  nesturbator-run: bench frames=600 time_ms=701.367 fps=855.47
  nesturbator-run: bench frame_us p50=1163.000 p90=1235.000 p99=1339.000 p99.9=1616.000 max=1616.000
  ```

  Every value is a measured frame time in microseconds, truncated from nanoseconds, by the
  nearest-rank method with no interpolation; p99.9 equals max below 1000 frames, and `fps` is
  frames over the summed frame time. A signal stop prints the statistics over the frames
  finished, then the stop line; a stop before the first frame prints only
  `nesturbator-run: bench frames=0`. Wall time is never gated in CI: the tests check the shape of
  these lines, never a number.
- `--bench-state` needs `--bench`. After every timed frame it saves the state and loads it back,
  timing the save and the load separately from the frame, so `frame_us` stays comparable. It adds
  the `save_us` and `load_us` lines after `frame_us`. A save or load failure exits 1. Standard
  output is unchanged.
- `--raise-after N[:SIG[:K]]` is a test aid: after frame N the runner signals its own
  process K times (K defaults to 1) with SIG, so a stop lands at a known frame. SIG is `int`
  or `term` on POSIX (default `term`) and `int` or `break` on Windows (default `break`). It
  cannot be combined with `--accuracycoin-page`.
- `--movie FILE` replays a validated two-port movie (see [Input movies](#input-movies)) instead of running `--frames N`,
  and prints a SHA-256 line for every frame.
- `--help` prints the usage and the exit statuses to stdout and exits 0.
- `--accuracycoin-page N` runs a strict AccuracyCoin page (2, 14 or 17) for
  1800 frames and compares its RAM results to the scoreboard. It needs `--rom`
  and `--scoreboard` and installs no stop handlers.
- `--scoreboard FILE` names the scoreboard that `--accuracycoin-page` compares against; it is
  an error without `--accuracycoin-page`.
- `--accuracycoin-inventory` prints the complete verified old or new pinned
  directory as TSV. It needs `--rom` and no other run mode.
- `--accuracycoin-observe PAGE` runs one selected page from a cold boot within
  `--frames N`, writing checked bounded evidence under `--accuracycoin-output DIR`.
  A real guest timeout, a guest JAM in the execution stage or a known
  failing/skipped result can still produce a trustworthy observation with exit 0
  (a JAM in an earlier stage writes no report and exits 1); bad inventory, missing output or a
  result code outside the pinned protocol exits 1. This mode installs no stop
  handlers and cannot be combined with scoreboards, movies, saves, state files,
  hashes or frame dumps.
- `--accuracycoin-output DIR` names the existing directory for observation
  evidence (`report.tsv`, `raw.tsv`, `diagnostics.tsv`, and `metadata.txt`); it
  is an error without `--accuracycoin-observe`.
- `--accuracycoin-collect DIR` validates launcher outcomes and all 22 page
  outputs, then emits campaign inventory, attempts, raw observations, final
  classifications, a count summary and provenance metadata. It needs `--rom`
  and cannot be combined with another run mode.
- `--accuracycoin-boot-cap N` sets the positive cold-boot guest-frame bound for
  `--accuracycoin-observe`. The CMake campaign value is in
  [`tests/accuracy/budgets.txt`](../tests/accuracy/budgets.txt).
- `--accuracycoin-navigation-cap N` sets the positive guest-frame bound for each
  page-navigation step in `--accuracycoin-observe`. The CMake campaign value is
  in [`tests/accuracy/budgets.txt`](../tests/accuracy/budgets.txt).
- `--accuracycoin-execution-cap N` sets the positive post-release guest-frame
  bound for `--accuracycoin-observe`. The CMake campaign value is in
  [`tests/accuracy/budgets.txt`](../tests/accuracy/budgets.txt).
- `--version` prints one line to stdout, `nesturbator-run X.Y.Z`, formatted at
  run time from `nesturbator_get_version`, and exits 0. It needs no other
  option and nothing else is printed. It is handled where the option loop
  reaches it, so options before it are still checked:
  `nesturbator-run --version --bogus` prints the version, while
  `nesturbator-run --bogus --version` is a usage error with exit 2. It prints
  no ABI or behaviour revision.
- `--rom FILE` loads a bounded iNES 1.0 or NES 2.0 image. Accepted
  geometry is 8 KiB CHR ROM or declared 8 KiB CHR RAM with 16 or 32 KiB PRG
  for mapper 0 (NROM), or PRG in 16 KiB banks up to 4 MiB for mapper 2
  (UxROM, submappers 0 to 2), or 16 or 32 KiB PRG with 8, 16 or 32 KiB CHR ROM
  for mapper 3 (CNROM, submappers 0 to 2), or 32 to 256 KiB PRG in 32 KiB banks
  with 8 KiB declared CHR RAM for mapper 7 (AxROM, submappers 0 to 2), or the
  mapper 1 and mapper 4 shapes below; optional trainers are included in the validated
  file length. UxROM writes to `$8000-$FFFF` select the bank at `$8000`; the
  last bank stays at `$C000`. Submappers 0 and 2 AND the written value with
  the ROM byte under the write (submapper 0 is the project default), and
  submapper 1 takes it raw. CNROM writes to `$8000-$FFFF` select the 8 KiB
  CHR bank with the same bus-conflict rule (submappers 0 and 2 AND, submapper
  1 raw); writes to CHR ROM are ignored. AxROM writes to `$8000-$FFFF` select the
  32 KiB PRG bank (bits 0 to 2) and the single-screen nametable page (bit 4);
  the header mirroring bit is ignored, the reset vector comes from bank 0 at power-on (a soft reset
  keeps the bank register, so it then comes from the selected bank), and
  only submapper 2 ANDs the value with the ROM byte under the write (submapper
  0 has no bus conflict).
- Mapper 1 (MMC1) is accepted with submapper 0, or submapper 5 with 32 KiB
  PRG. PRG is a power of two from 32 to 512 KiB, and 512 KiB only with 8 KiB
  CHR. CHR is 8 to 128 KiB of ROM or 8 KiB of RAM. PRG RAM is 0, 8, 16 or 32
  KiB in total (work plus battery), and more than 8 KiB only with 8 KiB CHR.
  The battery bit must be set exactly when battery RAM is declared. CHR NVRAM,
  other submappers, mapper 155 and a 24 KiB or 64 KiB total are refused.
  Mappers 0, 2, 3 and 7 still refuse the battery bit and every RAM size.
  The board is the MMC1B chip: five writes to `$8000-$FFFF` fill the serial
  port, and a write on the CPU cycle right after another write is ignored
  unless its bit 7 is set, which always resets the port (this is what makes
  `INC $8000` shift one bit, not two). PRG RAM is enabled at power-on and bit 4
  of the PRG register disables it. The SNROM, SOROM, SUROM and SXROM variants
  follow from the header sizes (hardware: NESdev Wiki "MMC1" and "SxROM").
  `nesturbator_reset` keeps the MMC1 registers and the battery RAM, because
  the cartridge sees no reset line.
- Mapper 4 (MMC3) is accepted with submapper 0 (the Sharp revision, also used
  for every iNES 1 image) or submapper 4 (the NEC revision). PRG is a power of
  two from 16 to 512 KiB, CHR is a power of two from 8 to 256 KiB of ROM or
  8 KiB of RAM, and PRG RAM is none, 8 KiB of work RAM or 8 KiB of battery
  RAM (the battery bit must match). Refused: MMC6 (submapper 1), submappers
  2, 3, 5 and 6 to 15, four-screen images, CHR NVRAM, other RAM sizes, and
  mappers 118, 119, 206 and 249. Registers decode by `addr & $E001`: `$8000`
  selects a bank register, the PRG mode and the CHR inversion, `$8001`
  writes it, `$A000` picks the mirroring (bit 0 clear is vertical), and
  `$A001` enables (bit 7) and write-protects (bit 6) the RAM at
  `$6000-$7FFF`, which reads as open bus when disabled. At power-on the RAM
  is enabled and writable. Banks wrap modulo the file size, so 16 KiB of PRG
  repeats. `nesturbator_reset` keeps the registers and the battery RAM
  (hardware: NESdev Wiki "MMC3").
- `--hash-frame N` prints a line after frame N has run. N must be between 1
  and the `--frames` value. The option can be repeated.
- `--hash-audio` prints one hash for all mixed-level transitions and one for
  all signed 16-bit PCM samples emitted by the requested run. It also works
  with `--movie` and AccuracyCoin page mode; a no-cartridge run hashes an empty
  transition stream and its silent PCM bytes.
- `--dump-frame N:FILE` writes frame N to FILE as a binary PPM (P6), 256x240,
  in the RGB of the colour table. N follows the `--hash-frame` rules, and the
  option can be repeated. The image is converted by [`host/convert.c`](../host/convert.c), the same
  loop the libretro adapter uses, so both show the same colours.
- `--save-dir DIR` keeps the battery RAM of a mapper 1 cartridge in
  `DIR/<name>.sav`. It needs `--rom` and cannot be combined with
  `--accuracycoin-page`. `<name>` is the last component of the `--rom` path
  (split at `/` or `\` on every platform) without its last extension, so
  `roms/game.v1.nes` uses `DIR/game.v1.sav`. After the cartridge loads and
  before the first frame the runner reads that file into the battery RAM; with
  no file the RAM starts zeroed. When the run ends, including after a JAM, it
  writes the RAM back if the CPU wrote to it and the bytes changed. The write
  goes to `DIR/<name>.sav.tmp`, is synced (with `F_FULLFSYNC` on macOS), and is renamed over the save, so the
  save is the old one or the new one and never torn. A failed write removes its
  `.tmp`, prints `nesturbator-run: cannot save <path> (<reason>); the previous
  save is kept`, and the runner keeps the newest bytes in memory for the next
  attempt. A kill during a write can leave a stale `.tmp` (a regular file or a
  symlink, even a dangling one), which the next write clears without touching a
  link's target; a directory at that name is never removed. There is no directory
  sync, so a power cut just after the rename may still show the previous save. A run that writes
  the same bytes leaves the file and its modification time alone. A `.sav` is
  the RAM's raw bytes with no header, the same bytes as RetroArch's `.srm`.
  The [battery-save API](api.md#battery-saves) documents the library payload
  and span lifetime. A
  cartridge without battery RAM reads and writes nothing, and without
  `--save-dir` no save file is read or written anywhere. A save
  failure, or a `DIR` that does not exist, exits 1. A `.sav` whose length is
  not the battery RAM's size (a 0-byte file included) is never padded,
  truncated or rewritten: the runner prints
  `<path> is <a> bytes; the cartridge's battery RAM is <b> bytes`, runs no
  frames and exits 4. The runner takes no lock: two runners on one directory,
  or two ROMs with the same name, share one save file, and the size check
  catches a clash of different sizes.
- `--save-interval N` (N of 1 or more, needs `--save-dir`) also writes a
  changed save after every Nth frame, counted from 1, so a runner stopped
  after a flush leaves the RAM as it was then. A failed interval write ends
  the run with exit 1, and the runner tries the write once more before it
  exits. With `--frames 3
  --save-interval 5` no interval write falls inside the run, and the write at
  exit still happens. `--save-interval 0`, a repeated or empty `--save-dir`,
  and `--save-dir` without `--rom` exit 2.

- `--state-at-frame N:FILE` (repeatable; N is 0 or more) writes the save state
  of the machine after frame N to FILE, split at the first colon. N of 0 is
  the state at power-on, or the loaded state itself when `--load-state` is
  given. The file is written through the same atomic replace as a `.sav`, so
  FILE holds the old file or one complete new one; the runner writes no other
  path and leaves no `.tmp` after a successful write. Running the same command
  twice replaces the file with identical bytes. Two runners writing one path
  at the same time are not supported. A write failure exits 1. The same N
  given twice exits 2. The payload and compatibility contract are documented in
  the [save-state API](api.md#save-states).
- `--hash-state N` (repeatable; N is 0 or more) prints one line after frame N,
  and for the loaded frame before the first frame runs:
  `state <N> ticks <T> size <S> sha256 <64 lowercase hex digits>`. The hash is the SHA-256 of
  the bytes `nesturbator_save_state` writes, `S` is their length and `T` the
  emulated time after frame N (0 at power-on). Per frame the order is the
  `frame` line, then the `state` line, then the dump files. A run that saves at
  60 and a run that loads that state print the same `state 120` line. The same
  N given twice exits 2.
- `--load-state FILE` (once; a second one exits 2) resumes from a save state
  after the cartridge, and the battery save when `--save-dir` is given, are
  loaded. The file is the native payload written by `nesturbator_save_state`,
  not a host-specific state container; see the [save-state API](api.md#save-states).
  Frame numbers stay absolute from power-on: a state saved after frame
  K (read from the state's header) makes the run continue at frame K + 1 up to
  `--frames`, and `--hash-frame` and `--state-at-frame` use the same numbers,
  so a run that saves at 60 and a run that loads that state print the same
  line for frame 120. A rejected state exits 5 and prints one line,
  `nesturbator-run: cannot load state <path> (<word>)`, where the word is
  `another format version`, `wrong game`, `wrong length`, `damaged` or
  `invalid contents`, for `NESTURBATOR_ERR_STATE_FORMAT`, `_CARTRIDGE`, `_SIZE`,
  `_CHECKSUM` and `_INVALID`. A file that is missing, is a directory or cannot
  be read exits 1 with `nesturbator-run: cannot read state <path> (<reason>)`.
  With `--save-dir`, a state whose battery bytes differ from the loaded `.sav`
  counts as a change, so the write at exit puts the state's battery bytes in
  the `.sav`.

  After a load at frame K, `--frames N` is the last frame to run (an end frame,
  not a count of further frames), so the run computes frames K + 1 to N. The
  ranges are checked after the load and before any frame runs, and a breach
  exits 2 with a message that names K: `--frames` must be at least K;
  `--hash-frame` and `--dump-frame` need K < N <= `--frames`;
  `--state-at-frame` and `--hash-state` need K <= N <= `--frames`. Without a
  load K is 0. `--movie` cannot be combined with `--load-state` (exit 2), and
  any of `--state-at-frame`, `--load-state` and `--hash-state` without `--rom`,
  or with `--accuracycoin-page`, exits 2.

### Stopping the runner

On SIGTERM or SIGINT the runner finishes the current frame, writes the battery save and exits 0, or 1 if the save could not be written. It prints one stop line on standard error and adds nothing to standard output. The stop is seen between frames, also before the first frame when the request arrives while the cartridge loads; a load error keeps its own exit status. The save goes through the same temp-and-rename write as any other, so it is never torn. A second signal at any time, including during the flush, ends the runner at once. A signal the runner inherited as ignored stays ignored: a runner started as a shell background job (`cmd &` in a non-interactive shell) or under `nohup` does not stop on SIGINT or SIGTERM, so a launcher that needs the stop starts it with those signals at their default. The `runner.signal.*` and `runner.stop.*` tests need the same, and fail or run to their timeout when ctest itself is started with SIGINT or SIGTERM ignored. A launcher on Linux or macOS must keep reading the runner's standard output, or redirect it to a file, because a full pipe holds the runner in a write and delays the stop until the write returns.

On Linux and macOS the `runner.signal.*` tests send real SIGTERM and SIGINT from another process and require exit 0 within 2 s with the whole save and no temp file, and a second SIGTERM to end a runner that is blocked writing to a full pipe. They also show that a signal during loading leaves a load error's status in charge, and that a second SIGTERM ends a runner held inside its exit flush.

A stopped run exits 1 when something requested was not produced: a `--hash-frame`, `--hash-state`, `--state-at-frame` or `--dump-frame` beyond the last finished frame, `--hash-audio`, or a movie that did not reach its end. The line after the stop line names the first such option on the command line, for example `nesturbator-run: not reached: --hash-frame 6`, and standard output gains nothing. A stopped run prints no audio hash, because it would cover a truncated run. A `--frames` count not reached is not a failure, so a launcher that runs open-ended (`--frames 4294967295 --save-dir DIR`, no hash or dump options) and stops it gets 0. A signal that arrives after the last frame changes nothing: the run is complete, exits 0 and prints its audio hash.

On Windows the runner handles Ctrl+C, Ctrl+Break and console close the same way. SIGTERM is not a Windows mechanism. A forced termination (TerminateProcess, taskkill /F, power loss) cannot be handled; the last committed .sav survives it. Logoff and shutdown are not handled. `--accuracycoin-page` installs no stop handlers. On Windows `runner.stop.win_handler` calls the console handler directly (Ctrl+C and Ctrl+Break handled once and declined after, logoff and shutdown declined, a close event held until the exit sequence is done or its cap passes), and `runner.stop.win_console` sends a real Ctrl+Break through the runner's own console and requires exit 0 with a whole save and no temp file.

A Windows launcher stops the runner with a console event, in one of two cases.

- The launcher shares the runner's console. It starts the runner with `CREATE_NEW_PROCESS_GROUP` (and without `CREATE_NEW_CONSOLE`, `DETACHED_PROCESS` or `CREATE_NO_WINDOW`) and calls `GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, pid)`, the runner's process id being its group id, so only the runner's group gets the event.
- The launcher has no console or a different one. It starts the runner with `CREATE_NEW_CONSOLE`, detaches from its own console (`FreeConsole`), attaches to the runner's (`AttachConsole` with the runner's process id), registers its own handler with `SetConsoleCtrlHandler`, and only then sends `GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, 0)`. Group 0 reaches every process on that console, the launcher included. A launcher that sends the event to group 0 first registers its own handler that returns TRUE for CTRL_BREAK_EVENT, after attaching and before sending, or the event ends the launcher too. Attaching resets the handler list, so the handler goes in after it.

Never send CTRL_C_EVENT to a process group. No event reaches a runner started with `DETACHED_PROCESS` or `CREATE_NO_WINDOW`. The launcher must read or discard the runner's standard output, because the runner flushes it before the save.

### Exit status

`nesturbator-run --help` prints the usage text and this table to standard output and
exits 0. A bad command line prints the same text to standard error and exits 2.

| Status | Meaning | When |
|---|---|---|
| 0 | done | The run finished, or `--version` or `--help` printed its text. |
| 1 | failure | A file cannot be read or written, a save fails, a stop whose save could not be written, a stopped run that missed a requested output, or a frame fails or hits a JAM. |
| 2 | usage error | The command line is wrong. |
| 4 | .sav size mismatch | The `.sav` file's length differs from the cartridge's battery RAM. |
| 5 | state rejected | `--load-state` refused the file. |

3 and 77 are reserved; the runner never returns them.

## Save and state examples

For a mapper 1 cartridge with battery RAM, create the save directory first:

```sh
mkdir -p saves
nesturbator-run --rom game.nes --frames 600 --save-dir saves --save-interval 120
```

Capture a native save state after frame 60, then resume it to the same absolute
end frame. Both runs report the same state hash for frame 120:

```sh
nesturbator-run --rom game.nes --frames 120 --state-at-frame 60:slot.state --hash-state 120
nesturbator-run --rom game.nes --frames 120 --load-state slot.state --hash-state 120
```

## Input movies

The runner can replay an owned, versioned two-port input movie:

```sh
nesturbator-run --movie input.nmovie --rom game.nes
```

The binary movie begins with the eight bytes `NMOVIE1` and a zero byte, then
little-endian `uint32_t` version `1` and frame count. Each frame stores two
little-endian `uint16_t` button masks, port 0 then port 1; values must fit the
eight standard NES button bits. The count is limited to 1,000,000 frames, and
the file must contain exactly the declared records. Empty movies are valid and
run zero frames. Invalid, truncated, extra, unsupported-version, or oversized
movies fail before a frame runs. Replay prints one native-pixel SHA-256 line
per frame in ascending frame order; a JAM stop reports its frame and exits
nonzero.

For the sanitized report form, bounded replay examples and next-evidence
triage, see the [scene reproduction guide](reproduction.md).

## Output examples

Each hashed frame prints one line:

```
frame <N> ticks <ticks> sha256 <64 lowercase hex digits>
```

`ticks` counts emulated time in half master-clock periods, 714732 per NTSC
frame. The hash is SHA-256 over the 256x240 native pixels, each a 16-bit
value written low byte first, row by row from the top row. The same inputs
give the same hash on every platform. For the test card:

```
$ nesturbator-run --frames 1 --hash-frame 1
frame 1 ticks 714732 sha256 b49e9be44573a4de82d845179d4389a0a6e28e934bd9ab31516a40db2c0b0453
```

Audio hashes can be checked without an audio device:

```
$ nesturbator-run --frames 1 --hash-audio
audio transitions sha256 e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
audio pcm sha256 bfd6d535131a45e8a31340082147c4928edd44f2e05fe127081a1900b3cc0edc
```

The hash is always over the native pixels, never over a dumped image:

```
$ nesturbator-run --frames 1 --dump-frame 1:frame1.ppm --hash-frame 1
frame 1 ticks 714732 sha256 b49e9be44573a4de82d845179d4389a0a6e28e934bd9ab31516a40db2c0b0453
```

writes `frame1.ppm` (184335 bytes) and prints the same line as before.

## Related runner checks

The save writer is tested under failure in
[`runner_save.cmake`](../tests/cmake/runner_save.cmake). `runner.save_replace` runs the shared
replace helper against a directory at the save path, a missing parent, a
regular file as a parent, an empty and a non-empty directory at `.tmp`, a stale
`.tmp`, and (Linux and macOS) a read-only directory and a symlink (also a dangling one) at `.tmp`;
each leaves the old save intact and no `.tmp` behind. `runner.save_fsize`
(Linux and macOS) repeats that under a file size limit.
`runner.save.dir_at_tmp` runs the runner itself with a directory at
`battery.sav.tmp` and expects exit 1, the `cannot save` message and the
directory left alone. Runner-level state and stop behavior remains covered by
[`runner_state.cmake`](../tests/cmake/runner_state.cmake) and
[`runner_stop.cmake`](../tests/cmake/runner_stop.cmake); save behavior is
registered in [`tests/CMakeLists.txt`](../tests/CMakeLists.txt).
