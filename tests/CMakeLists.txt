# Runner tests: each runs nesturbator-run through expect_output.cmake, which
# checks the exact standard output and exit status.
#
#   nesturbator_runner_test(NAME <test> ARGS <arg>... EXIT <code>
#                           [IGNORE_STDOUT] [EXPECT <line>...] [EXPECT_ERR <text>])
#
# Without EXPECT and IGNORE_STDOUT the output must be empty.
function(nesturbator_runner_test)
  cmake_parse_arguments(T "IGNORE_STDOUT" "NAME;EXIT;CLEAN_DIR;EXPECT_ERR" "ARGS;EXPECT" ${ARGN})
  # A quoted add_test argument keeps its semicolons, so each list arrives
  # whole as one -D value.
  set(cmd "$<TARGET_FILE:nesturbator-run>" ${T_ARGS})
  add_test(NAME ${T_NAME}
    COMMAND ${CMAKE_COMMAND}
      "-DCMD=${cmd}"
      "-DEXPECT=${T_EXPECT}"
      "-DEXPECT_EXIT=${T_EXIT}"
      "-DIGNORE_STDOUT=${T_IGNORE_STDOUT}"
      "-DCLEAN_DIR=${T_CLEAN_DIR}"
      "-DEXPECT_ERR=${T_EXPECT_ERR}"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/expect_output.cmake")
endfunction()

# FRAME-03: the test card's hash. The expected value comes from an
# independent model of D-02 (01-RESEARCH), not from this code.
nesturbator_runner_test(NAME runner.hash
  ARGS --frames 1 --hash-frame 1
  EXIT 0
  EXPECT "frame 1 ticks 714732 sha256 b49e9be44573a4de82d845179d4389a0a6e28e934bd9ab31516a40db2c0b0453")

# D-04: frame advance shows in the metadata, not the pixels. Frame 3 has the
# same hash as frame 1 and three frames of ticks (3 x 714732).
nesturbator_runner_test(NAME runner.advance
  ARGS --frames 3 --hash-frame 1 --hash-frame 3
  EXIT 0
  EXPECT
    "frame 1 ticks 714732 sha256 b49e9be44573a4de82d845179d4389a0a6e28e934bd9ab31516a40db2c0b0453"
    "frame 3 ticks 2144196 sha256 b49e9be44573a4de82d845179d4389a0a6e28e934bd9ab31516a40db2c0b0453")

# No --hash-frame: frames run, nothing is printed.
nesturbator_runner_test(NAME runner.nohash ARGS --frames 2 EXIT 0)
nesturbator_runner_test(NAME runner.hash_audio.silent
  ARGS --frames 1 --hash-audio
  EXIT 0
  EXPECT
    "audio transitions sha256 e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"
    "audio pcm sha256 bfd6d535131a45e8a31340082147c4928edd44f2e05fe127081a1900b3cc0edc")

# SND-04: a fixed-point FFT checks the five locked APU reference periods.
add_executable(runner.spectral runner/test_spectral.c)
target_link_libraries(runner.spectral PRIVATE nesturbator::nesturbator)
target_include_directories(runner.spectral PRIVATE ${PROJECT_SOURCE_DIR}/src)
nesturbator_warnings(runner.spectral)
add_test(NAME runner.spectral COMMAND runner.spectral)
set_tests_properties(runner.spectral PROPERTIES TIMEOUT 60)

# Usage errors exit 2 (LIBRETRO-AND-RUNNER section 5).
nesturbator_runner_test(NAME runner.usage.beyond
  ARGS --frames 1 --hash-frame 2 EXIT 2 IGNORE_STDOUT)
nesturbator_runner_test(NAME runner.usage.zero
  ARGS --frames 1 --hash-frame 0 EXIT 2 IGNORE_STDOUT)
# --frames is required: a run with no arguments exits 2.
nesturbator_runner_test(NAME runner.usage.noargs
  EXIT 2 IGNORE_STDOUT)
nesturbator_runner_test(NAME runner.usage.unknown
  ARGS --bogus EXIT 2 IGNORE_STDOUT)
# A cartridge path arrives in Phase 3; until then a positional argument is a
# usage error.
# D-10: --version is handled where the option loop reaches it, so a bad option
# before it is still a usage error.
nesturbator_runner_test(NAME runner.usage.version_after_bad
  ARGS --bogus --version EXIT 2 IGNORE_STDOUT)
nesturbator_runner_test(NAME runner.usage.positional
  ARGS cartridge EXIT 2 IGNORE_STDOUT)

# RUN-03: one test per exit status. 4 and 5 are runner.save.mismatch and runner.state.reject_game
# below; --help is runner.exit.help. Options before --help are validated, as with --version.
nesturbator_runner_test(NAME runner.exit.done ARGS --frames 1 EXIT 0)
nesturbator_runner_test(NAME runner.exit.failure
  ARGS --frames 1 --rom ${CMAKE_CURRENT_BINARY_DIR}/no-such-rom.nes
  EXIT 1 EXPECT_ERR "no-such-rom.nes")
nesturbator_runner_test(NAME runner.exit.usage EXPECT_ERR "usage: nesturbator-run" EXIT 2)
nesturbator_runner_test(NAME runner.usage.help_after_bad
  ARGS --bogus --help EXIT 2 IGNORE_STDOUT)
set_tests_properties(runner.exit.done runner.exit.failure runner.exit.usage
  PROPERTIES LABELS exit_status)

# MAP-03 / D-15: a refused cartridge gets one named reason on stderr and exit 1.
set(NESTURBATOR_REJECT_SEEDS ${PROJECT_SOURCE_DIR}/tests/fuzz/corpus)
set(NESTURBATOR_MAPPER_LIST "supported mappers are 0, 1, 2, 3, 4 and 7")
nesturbator_runner_test(NAME runner.reject.four_screen
  ARGS --rom ${NESTURBATOR_REJECT_SEEDS}/mmc3-four-screen --frames 1
  EXIT 1 IGNORE_STDOUT
  EXPECT_ERR "nesturbator-run: four-screen cartridges are not supported")
nesturbator_runner_test(NAME runner.reject.mmc6
  ARGS --rom ${NESTURBATOR_REJECT_SEEDS}/mmc3-submapper-1 --frames 1
  EXIT 1 IGNORE_STDOUT
  EXPECT_ERR "nesturbator-run: MMC6 (mapper 4, submapper 1) is not supported")
nesturbator_runner_test(NAME runner.reject.mapper
  ARGS --rom ${NESTURBATOR_REJECT_SEEDS}/mapper-118 --frames 1
  EXIT 1 IGNORE_STDOUT
  EXPECT_ERR "nesturbator-run: mapper 118 is not supported; ${NESTURBATOR_MAPPER_LIST}")
nesturbator_runner_test(NAME runner.reject.generic
  ARGS --rom ${NESTURBATOR_REJECT_SEEDS}/mmc3-prg-8k --frames 1
  EXIT 1 IGNORE_STDOUT
  EXPECT_ERR "${NESTURBATOR_MAPPER_LIST}")
nesturbator_runner_test(NAME runner.reject.truncated
  ARGS --rom ${NESTURBATOR_REJECT_SEEDS}/truncated-header --frames 1
  EXIT 1 IGNORE_STDOUT
  EXPECT_ERR "${NESTURBATOR_MAPPER_LIST}")

# FRAME-03 / D-16: --dump-frame writes the test card as a 256x240 P6 image
# (15 header bytes + 256*240*3) while the hash line stays over the native
# pixels. Pixel (8,232) is native $12, whose calibrated table entry is 0x5238FF. Later
# tests read the image through the fixture testframe_ppm.
set(NESTURBATOR_TESTFRAME_PPM ${CMAKE_BINARY_DIR}/testframe/frame1.ppm)
set(dump_cmd "$<TARGET_FILE:nesturbator-run>" --frames 1
  --dump-frame "1:${NESTURBATOR_TESTFRAME_PPM}" --hash-frame 1)
add_test(NAME runner.dump
  COMMAND ${CMAKE_COMMAND}
    "-DCMD=${dump_cmd}"
    "-DEXPECT=frame 1 ticks 714732 sha256 b49e9be44573a4de82d845179d4389a0a6e28e934bd9ab31516a40db2c0b0453"
    -DEXPECT_EXIT=0
    "-DFILE=${NESTURBATOR_TESTFRAME_PPM}"
    -DSIZE=184335
    -DPIXELS=8:232:5238FF
    -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/check_ppm.cmake")
set_tests_properties(runner.dump PROPERTIES FIXTURES_SETUP testframe_ppm)

# --dump-frame usage errors exit 2: N beyond --frames, no colon, no FILE.
nesturbator_runner_test(NAME runner.usage.dump
  ARGS --dump-frame 2:x.ppm --frames 1 EXIT 2 IGNORE_STDOUT)
nesturbator_runner_test(NAME runner.usage.dump.colon
  ARGS --frames 1 --dump-frame 1 EXIT 2 IGNORE_STDOUT)
# A usage error outranks a missing ROM: the K-independent range checks run before any file is read.
nesturbator_runner_test(NAME runner.usage.beyond.badrom
  ARGS --rom no-such-file.nes --frames 1 --hash-frame 2 EXIT 2 IGNORE_STDOUT)
nesturbator_runner_test(NAME runner.usage.dump.badrom
  ARGS --rom no-such-file.nes --frames 1 --dump-frame 2:x.ppm EXIT 2 IGNORE_STDOUT)
nesturbator_runner_test(NAME runner.usage.dump.file
  ARGS --frames 1 --dump-frame 1: EXIT 2 IGNORE_STDOUT)

# A file that cannot be created (its directory does not exist) exits 1.
nesturbator_runner_test(NAME runner.dump.unwritable
  ARGS --frames 1 --dump-frame "1:${CMAKE_BINARY_DIR}/no-such-dir/frame1.ppm"
  EXIT 1)

# SAVE-02 to SAVE-04: --save-dir on synthetic cartridges. runner.save_rom writes
# them under the build tree; each runner.save.<case> runs the case in its own
# scratch directory, so the cases are safe under ctest -j.
add_executable(runner.save_rom runner/save_rom.c)
nesturbator_warnings(runner.save_rom)
target_compile_definitions(runner.save_rom PRIVATE
  $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
set(NESTURBATOR_SAVE_FIXTURE ${CMAKE_CURRENT_BINARY_DIR}/save_fixture)
file(MAKE_DIRECTORY ${NESTURBATOR_SAVE_FIXTURE})
add_test(NAME runner.save.fixture
  COMMAND runner.save_rom ${NESTURBATOR_SAVE_FIXTURE})
set_tests_properties(runner.save.fixture PROPERTIES FIXTURES_SETUP runner_save)
function(runner_save_case case)
  add_test(NAME runner.save.${case}
    COMMAND ${CMAKE_COMMAND}
      "-DRUNNER=$<TARGET_FILE:nesturbator-run>"
      "-DFIXTURE=${NESTURBATOR_SAVE_FIXTURE}"
      "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/save_work/${case}"
      "-DCASE=${case}"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/runner_save.cmake")
  set_tests_properties(runner.save.${case} PROPERTIES
    FIXTURES_REQUIRED runner_save TIMEOUT 30)
endfunction()
runner_save_case(roundtrip)
runner_save_case(fresh)
runner_save_case(mismatch)
runner_save_case(mismatch_empty)
runner_save_case(unchanged)
runner_save_case(nodir)
runner_save_case(batteryless)
runner_save_case(unwritable)
runner_save_case(interval)
runner_save_case(interval_long)
runner_save_case(interval_retry)
runner_save_case(jam)
runner_save_case(dir_at_tmp)
set_tests_properties(runner.save.interval PROPERTIES TIMEOUT 60)

# RUN-02: a signal finishes the current frame, flushes and exits 0. --raise-after signals the
# runner's own process after frame N, so each case is deterministic. The signal names differ by
# platform: int and term on POSIX, int and break on Windows.
if(NOT WIN32)
  find_program(NESTURBATOR_SH sh REQUIRED)
endif()
function(runner_stop_case case)
  add_test(NAME runner.stop.${case}
    COMMAND ${CMAKE_COMMAND}
      "-DRUNNER=$<TARGET_FILE:nesturbator-run>"
      "-DFIXTURE=${NESTURBATOR_SAVE_FIXTURE}"
      "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/stop_work/${case}"
      "-DCASE=${case}"
      "-DSH=${NESTURBATOR_SH}"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/runner_stop.cmake")
  set_tests_properties(runner.stop.${case} PROPERTIES
    FIXTURES_REQUIRED runner_save TIMEOUT 30 LABELS exit_status)
endfunction()
if(WIN32)
  runner_stop_case(frame_exact.break)
  runner_stop_case(frame_exact.int)
else()
  runner_stop_case(frame_exact.term)
  runner_stop_case(frame_exact.int)
  # D-20 WR-01: an inherited SIG_IGN for SIGTERM stays in force.
  runner_stop_case(inherited_ignore)
endif()
runner_stop_case(flush_missing_dir)
runner_stop_case(flush_dir_at_tmp)
runner_stop_case(at_zero)
runner_stop_case(nodir)
runner_stop_case(double)
runner_stop_case(double.int)
runner_stop_case(load_state_at)
runner_stop_case(load_state_below)
# D-08 to D-10: a stop that missed a requested output exits 1 and names it.
runner_stop_case(unmet_hash_frame)
runner_stop_case(unmet_hash_state)
runner_stop_case(unmet_state_at_frame)
runner_stop_case(unmet_dump_frame)
runner_stop_case(unmet_hash_audio)
runner_stop_case(unmet_movie)
runner_stop_case(unmet_order)
runner_stop_case(met_at_stop)
runner_stop_case(late)

# D-15: real SIGTERM and SIGINT from another process (never a skip: policy.no-skip).
# runner.signal prints "signal-to-exit <ms> ms" for every timed case.
if(NOT WIN32)
  add_executable(runner.signal runner/test_signal.c)
  nesturbator_warnings(runner.signal)
  add_test(NAME runner.signal.scratch
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_CURRENT_BINARY_DIR}/signal_work)
  set_tests_properties(runner.signal.scratch PROPERTIES FIXTURES_SETUP runner_signal_scratch)
  set(NESTURBATOR_SIGNAL_TIMEOUT 30)  # hang detector, not a performance budget: above the helper's 10 s ready and 5 s exit deadlines; the 2 s stop bound is the helper's STOP_LIMIT_MS (D-20, 14-REVIEW WR-02)
  function(runner_signal_case case)
    add_test(NAME runner.signal.${case}
      COMMAND runner.signal $<TARGET_FILE:nesturbator-run> ${NESTURBATOR_SAVE_FIXTURE}
        ${CMAKE_CURRENT_BINARY_DIR}/signal_work/${case} ${case})
    set_tests_properties(runner.signal.${case} PROPERTIES
      FIXTURES_REQUIRED "runner_save;runner_signal_scratch"
      RUN_SERIAL TRUE TIMEOUT ${NESTURBATOR_SIGNAL_TIMEOUT})
  endfunction()
  runner_signal_case(term)
  runner_signal_case(int)
  runner_signal_case(term_exitflush)
  runner_signal_case(second)
  runner_signal_case(load_error)
  runner_signal_case(second_in_flush)
  runner_signal_case(bench)
  runner_signal_case(bench_int)
  set_tests_properties(runner.signal.term runner.signal.int runner.signal.term_exitflush
    PROPERTIES LABELS exit_status)
endif()

# D-14: the Windows console handler called directly; no console is needed (never a skip).
if(WIN32)
  add_executable(runner.stop_win runner/test_stop_win.c ${PROJECT_SOURCE_DIR}/runner/stop.c)
  target_include_directories(runner.stop_win PRIVATE ${PROJECT_SOURCE_DIR}/runner)
  nesturbator_warnings(runner.stop_win)
  target_compile_definitions(runner.stop_win PRIVATE
    $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
  add_test(NAME runner.stop.win_handler COMMAND runner.stop_win)
  set_tests_properties(runner.stop.win_handler PROPERTIES TIMEOUT 30 LABELS exit_status)

  # D-16: a real Ctrl+Break through the runner's own console. Kept or removed by hosted evidence.
  add_executable(runner.stop_console runner/test_console.c)
  nesturbator_warnings(runner.stop_console)
  target_compile_definitions(runner.stop_console PRIVATE
    $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
  add_test(NAME runner.stop.win_console
    COMMAND runner.stop_console $<TARGET_FILE:nesturbator-run> ${NESTURBATOR_SAVE_FIXTURE}
      ${CMAKE_CURRENT_BINARY_DIR}/console_work)
  set_tests_properties(runner.stop.win_console PROPERTIES
    FIXTURES_REQUIRED runner_save TIMEOUT 30 LABELS exit_status)  # hang detector, not a performance budget: above the helper's 20 s save, 2 s attach and 5 s exit waits (D-20, 14-REVIEW WR-02)
endif()

# D-04, D-11: --raise-after usage errors exit 2 and name the option.
if(WIN32)
  set(raise_foreign_sig term)
else()
  set(raise_foreign_sig break)
endif()
nesturbator_runner_test(NAME runner.usage.raise_after.bad_sig
  ARGS --frames 2 --raise-after 1:kill EXIT 2 EXPECT_ERR "--raise-after")
nesturbator_runner_test(NAME runner.usage.raise_after.foreign_sig
  ARGS --frames 2 --raise-after 1:${raise_foreign_sig} EXIT 2 EXPECT_ERR "--raise-after")
nesturbator_runner_test(NAME runner.usage.raise_after.bad_k
  ARGS --frames 2 --raise-after 1:int:3 EXIT 2 EXPECT_ERR "--raise-after")
nesturbator_runner_test(NAME runner.usage.raise_after.twice
  ARGS --frames 2 --raise-after 1 --raise-after 1 EXIT 2 EXPECT_ERR "--raise-after")
nesturbator_runner_test(NAME runner.usage.raise_after.beyond
  ARGS --frames 2 --raise-after 3 EXIT 2 EXPECT_ERR "--raise-after")
nesturbator_runner_test(NAME runner.usage.raise_after.malformed
  ARGS --frames 2 --raise-after 1: EXIT 2 EXPECT_ERR "--raise-after")
nesturbator_runner_test(NAME runner.usage.raise_after.accuracycoin
  ARGS --accuracycoin-page 2 --rom ${PROJECT_SOURCE_DIR}/tests/roms/none.nes --scoreboard s
  --raise-after 1 EXIT 2 EXPECT_ERR "--raise-after")

# RUN-01 (D-07, D-09 to D-11): --state-at-frame and --load-state. runner.state_mutate makes the
# corrupt copies at test time (CMake cannot compute a CRC-32); no state file is committed. Each
# runner.state.<case> runs in its own scratch directory, so the cases are safe under ctest -j.
add_executable(runner.state_mutate runner/state_mutate.c ${PROJECT_SOURCE_DIR}/src/crc32.c)
target_include_directories(runner.state_mutate PRIVATE ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include)
nesturbator_warnings(runner.state_mutate)
target_compile_definitions(runner.state_mutate PRIVATE
  $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
add_executable(runner.nmi_fixture
  runner/nmi_fixture.c
  ../runner/audio_hash.c
  ../runner/sha256.c)
target_include_directories(runner.nmi_fixture PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/src
  ${PROJECT_SOURCE_DIR}/runner)
target_link_libraries(runner.nmi_fixture PRIVATE nesturbator::nesturbator)
nesturbator_warnings(runner.nmi_fixture)
target_compile_definitions(runner.nmi_fixture PRIVATE
  $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
function(runner_state_case case)
  add_test(NAME runner.state.${case}
    COMMAND ${CMAKE_COMMAND}
      "-DRUNNER=$<TARGET_FILE:nesturbator-run>"
      "-DMUTATE=$<TARGET_FILE:runner.state_mutate>"
      "-DNMI_FIXTURE=$<TARGET_FILE:runner.nmi_fixture>"
      "-DROM=${PROJECT_SOURCE_DIR}/tests/roms/dabg.nes"
      "-DOTHER_ROM=${PROJECT_SOURCE_DIR}/tests/roms/hm/M3_P32K_C32K_H.nes"
      "-DSAVE_FIXTURE=${NESTURBATOR_SAVE_FIXTURE}"
      "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/state_work/${case}"
      "-DCASE=${case}"
      "-DHASHES=${CMAKE_CURRENT_SOURCE_DIR}/runner/hashes.txt"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/runner_state.cmake")
  set_tests_properties(runner.state.${case} PROPERTIES
    FIXTURES_REQUIRED runner_save TIMEOUT 60)
endfunction()
runner_state_case(roundtrip)
runner_state_case(reject_game)
runner_state_case(reject_size)
runner_state_case(reject_checksum)
runner_state_case(reject_format)
runner_state_case(reject_invalid)
runner_state_case(missing_file)
runner_state_case(directory)
runner_state_case(idempotent)
runner_state_case(save_dir)
runner_state_case(frame0)
runner_state_case(same_k)
runner_state_case(continue_hash)
runner_state_case(nmi_roundtrip)
runner_state_case(nmi_idempotent)
runner_state_case(nmi_wrong_load)
runner_state_case(range_frames)
runner_state_case(range_hash_frame)
runner_state_case(range_state)
runner_state_case(every_frame)
set_tests_properties(runner.save.mismatch runner.save.unwritable runner.state.reject_game
  PROPERTIES LABELS exit_status)
nesturbator_runner_test(NAME runner.usage.load_state.twice
  ARGS --frames 1 --load-state a.state --load-state b.state EXIT 2 IGNORE_STDOUT)
# D-11, D-12 and Open Question 3: repetition and combination refusals exit 2.
nesturbator_runner_test(NAME runner.usage.state.twice
  ARGS --frames 5 --rom ${PROJECT_SOURCE_DIR}/tests/roms/none.nes
  --state-at-frame 2:a.state --state-at-frame 2:b.state EXIT 2 IGNORE_STDOUT)
nesturbator_runner_test(NAME runner.usage.hash_state.twice
  ARGS --frames 5 --rom ${PROJECT_SOURCE_DIR}/tests/roms/none.nes
  --hash-state 2 --hash-state 2 EXIT 2 IGNORE_STDOUT)
nesturbator_runner_test(NAME runner.usage.state.movie
  ARGS --movie m.mov --rom ${PROJECT_SOURCE_DIR}/tests/roms/none.nes
  --load-state a.state EXIT 2 IGNORE_STDOUT)
nesturbator_runner_test(NAME runner.usage.state.norom
  ARGS --frames 5 --hash-state 2 EXIT 2 IGNORE_STDOUT)

# D-18: save flag usage errors exit 2.
set(save_rom_arg ${PROJECT_SOURCE_DIR}/tests/roms/none.nes)
nesturbator_runner_test(NAME runner.usage.save_dir.norom
  ARGS --frames 1 --save-dir d EXIT 2 IGNORE_STDOUT)
nesturbator_runner_test(NAME runner.usage.save_dir.empty
  ARGS --frames 1 --rom ${save_rom_arg} --save-dir "" EXIT 2 IGNORE_STDOUT)
nesturbator_runner_test(NAME runner.usage.save_dir.twice
  ARGS --frames 1 --rom ${save_rom_arg} --save-dir a --save-dir b EXIT 2 IGNORE_STDOUT)
nesturbator_runner_test(NAME runner.usage.save_dir.accuracycoin
  ARGS --accuracycoin-page 2 --rom ${save_rom_arg} --scoreboard s --save-dir d
  EXIT 2 IGNORE_STDOUT)
nesturbator_runner_test(NAME runner.usage.save_interval.zero
  ARGS --frames 1 --rom ${save_rom_arg} --save-dir d --save-interval 0 EXIT 2 IGNORE_STDOUT)
nesturbator_runner_test(NAME runner.usage.save_interval.nodir
  ARGS --frames 1 --rom ${save_rom_arg} --save-interval 5 EXIT 2 IGNORE_STDOUT)

# D-18: the save name rule.
add_executable(runner.save_path runner/test_save.c ${PROJECT_SOURCE_DIR}/runner/save.c)
target_include_directories(runner.save_path PRIVATE ${PROJECT_SOURCE_DIR}/runner)
nesturbator_warnings(runner.save_path)
target_compile_definitions(runner.save_path PRIVATE
  $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
add_test(NAME runner.save_path COMMAND runner.save_path)

# D-06 (TUNE-07): failure injection for the atomic replace. The scratch trees
# are removed before each run so a rerun starts clean.
add_test(NAME runner.save_scratch
  COMMAND ${CMAKE_COMMAND} -E rm -rf
    ${CMAKE_CURRENT_BINARY_DIR}/save_replace_work
    ${CMAKE_CURRENT_BINARY_DIR}/save_fsize_work)
set_tests_properties(runner.save_scratch PROPERTIES FIXTURES_SETUP runner_save_scratch)
add_executable(runner.save_replace runner/test_save_replace.c ${PROJECT_SOURCE_DIR}/runner/save.c)
target_include_directories(runner.save_replace PRIVATE ${PROJECT_SOURCE_DIR}/runner)
nesturbator_warnings(runner.save_replace)
target_compile_definitions(runner.save_replace PRIVATE
  $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
add_test(NAME runner.save_replace
  COMMAND runner.save_replace ${CMAKE_CURRENT_BINARY_DIR}/save_replace_work)
set_tests_properties(runner.save_replace PROPERTIES FIXTURES_REQUIRED runner_save_scratch)
if(UNIX)
  add_executable(runner.save_fsize runner/test_save_fsize.c ${PROJECT_SOURCE_DIR}/runner/save.c)
  target_include_directories(runner.save_fsize PRIVATE ${PROJECT_SOURCE_DIR}/runner)
  nesturbator_warnings(runner.save_fsize)
  add_test(NAME runner.save_fsize
    COMMAND runner.save_fsize ${CMAKE_CURRENT_BINARY_DIR}/save_fsize_work)
  set_tests_properties(runner.save_fsize PROPERTIES FIXTURES_REQUIRED runner_save_scratch)
endif()

# Holy Mapperel (BOARD-01): each ROM in roms.cmake is run to its frame N, the
# runner dumps that frame, and holymapperel-decode reads the result code from
# the pixels. The decoder is test-only; the runner knows nothing of the ROMs.
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/hash_keys.cmake)
# A guard over an empty list passes for any tree, so an empty list stops the
# configure (D-17). Plans 06 and 07 reuse these lists for parity registration.
if(NOT NESTURBATOR_HASH_GAMES)
  message(FATAL_ERROR "NESTURBATOR_HASH_GAMES is empty (tests/cmake/hash_keys.cmake)")
endif()
if(NOT NESTURBATOR_HOLYMAPPEREL_ROMS)
  message(FATAL_ERROR "NESTURBATOR_HOLYMAPPEREL_ROMS is empty (tests/holymapperel/roms.cmake)")
endif()
set(NESTURBATOR_HM_DIR ${CMAKE_CURRENT_BINARY_DIR}/holymapperel)
file(MAKE_DIRECTORY ${NESTURBATOR_HM_DIR})
add_executable(holymapperel-decode holymapperel/decode.c)
nesturbator_warnings(holymapperel-decode)
target_compile_definitions(holymapperel-decode PRIVATE
  $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)

# Derived ROMs (D-02): header-patched copies of a committed base, built with
# the build so they exist for CTest and for CI's bare write_hashes.cmake step.
# The base's sha256 comes from its manifest line; the copies are never committed.
add_executable(holymapperel-derive holymapperel/derive.c)
nesturbator_warnings(holymapperel-derive)
target_compile_definitions(holymapperel-derive PRIVATE
  $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
set(hm_derived_outputs)
foreach(entry IN LISTS NESTURBATOR_HOLYMAPPEREL_DERIVED)
  string(REPLACE ":" ";" parts "${entry}")
  list(GET parts 0 hm_key)
  list(GET parts 1 hm_base)
  list(GET parts 2 hm_patches)
  string(REPLACE "," ";" hm_patches "${hm_patches}")
  set(hm_base_sha256 "")
  foreach(line IN LISTS NESTURBATOR_HOLYMAPPEREL_MANIFEST)
    if(line MATCHES "^tests/roms/hm/${hm_base}\t.*\t([0-9a-f]+)$")
      set(hm_base_sha256 ${CMAKE_MATCH_1})
    endif()
  endforeach()
  if(hm_base_sha256 STREQUAL "")
    message(FATAL_ERROR "no manifest line for derived base ${hm_base}")
  endif()
  set(hm_out ${NESTURBATOR_HM_DIR}/derived/${hm_key}.nes)
  add_custom_command(OUTPUT ${hm_out}
    COMMAND ${CMAKE_COMMAND} -DTOOL=$<TARGET_FILE:holymapperel-derive>
      -DBASE=${PROJECT_SOURCE_DIR}/tests/roms/hm/${hm_base}
      -DBASE_SHA256=${hm_base_sha256} -DOUT=${hm_out} "-DPATCHES=${hm_patches}"
      -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/hm_derive.cmake
    DEPENDS holymapperel-derive ${PROJECT_SOURCE_DIR}/tests/roms/hm/${hm_base}
      ${CMAKE_CURRENT_SOURCE_DIR}/cmake/hm_derive.cmake
    VERBATIM)
  list(APPEND hm_derived_outputs ${hm_out})
endforeach()
add_custom_target(holymapperel-derived ALL DEPENDS ${hm_derived_outputs})
# The derive step refuses a base whose sha256 is not the manifest's.
add_test(NAME holymapperel.derive.refuse
  COMMAND ${CMAKE_COMMAND} -DTOOL=$<TARGET_FILE:holymapperel-derive>
    -DBASE=${PROJECT_SOURCE_DIR}/tests/roms/hm/M4_P128K_CR8K.nes
    -DBASE_SHA256=0000000000000000000000000000000000000000000000000000000000000000
    -DOUT=${NESTURBATOR_HM_DIR}/derived/refused.nes "-DPATCHES=set@10=07"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/hm_derive.cmake)
set_tests_properties(holymapperel.derive.refuse PROPERTIES WILL_FAIL TRUE)

foreach(entry IN LISTS NESTURBATOR_HOLYMAPPEREL_ROMS)
  string(REPLACE ":" ";" parts "${entry}")
  list(GET parts 0 hm_key)
  list(GET parts 1 hm_file)
  if(hm_file MATCHES "^derived/")
    set(hm_rom ${NESTURBATOR_HM_DIR}/${hm_file})
  else()
    set(hm_rom ${PROJECT_SOURCE_DIR}/tests/roms/hm/${hm_file})
  endif()
  list(GET parts 2 hm_frames)
  list(GET parts 3 hm_text)
  list(LENGTH parts hm_parts)
  set(hm_save "")
  if(hm_parts GREATER 4)
    list(GET parts 4 hm_save)
  endif()
  if(hm_save STREQUAL "save")
    # D-18 / D-09: libretro.parity.<key>.run1 and .run2 round-trip the save
    # through the module. Run 1 compares the module's battery span with the
    # runner's run-1 .sav; run 2 loads that file into the span before the first
    # frame (--sav-in) and compares its frame and span with run 2's. Run 2's
    # runner dump rewrites <stem>.sav, so a parity test must never read that
    # file while run 2 can still be writing it: holymapperel.<key>.run1.sav_copy
    # takes a private copy, and run2.dump requires the copy's fixture, so
    # every reader of the run-1 bytes has them before run 2 starts.
    # SAVE-02 (D-04): the same battery ROM runs twice against one save
    # directory. Run 1 starts empty and must not show + BATTERY; it leaves a
    # 32 KiB save carrying Holy Mapperel's SAVEDATA marker at offset 0x100.
    # Run 2 loads that save and must show + BATTERY. Run 1 empties the save
    # directory itself, so it still starts empty when ctest repeats it.
    # Optional field 6 is the save span size (default 32768) and field 7 an
    # extra <offset>=<hex> check on run 1's save (the MMC3 write protect byte).
    set(hm_sav_size 32768)
    if(hm_parts GREATER 5)
      list(GET parts 5 hm_sav_size)
    endif()
    set(hm_sav_byte "")
    if(hm_parts GREATER 6)
      list(GET parts 6 hm_sav_byte)
    endif()
    get_filename_component(hm_stem ${hm_file} NAME_WE)
    set(hm_savedir ${NESTURBATOR_HM_DIR}/${hm_key}-save)
    foreach(hm_run IN ITEMS run1 run2)
      set(hm_clean "")
      if(hm_run STREQUAL "run1")
        set(hm_clean ${hm_savedir})
      endif()
      nesturbator_runner_test(NAME holymapperel.${hm_key}.${hm_run}.dump
        ARGS --rom ${hm_rom} --frames ${hm_frames}
          --save-dir ${hm_savedir}
          --dump-frame "${hm_frames}:${NESTURBATOR_HM_DIR}/${hm_key}-${hm_run}.ppm"
        EXIT 0 IGNORE_STDOUT CLEAN_DIR ${hm_clean})
      add_test(NAME holymapperel.${hm_key}.${hm_run}.decode
        COMMAND holymapperel-decode ${NESTURBATOR_HM_DIR}/${hm_key}-${hm_run}.ppm)
      add_test(NAME holymapperel.${hm_key}.${hm_run}.prgram
        COMMAND holymapperel-decode --prg-ram ${NESTURBATOR_HM_DIR}/${hm_key}-${hm_run}.ppm)
      set_tests_properties(holymapperel.${hm_key}.${hm_run}.dump PROPERTIES
        FIXTURES_SETUP holymapperel_${hm_key}_${hm_run} TIMEOUT 120)
      set_tests_properties(holymapperel.${hm_key}.${hm_run}.decode PROPERTIES
        FIXTURES_REQUIRED holymapperel_${hm_key}_${hm_run}
        PASS_REGULAR_EXPRESSION "^holymapperel: code 0000 \\(")
    endforeach()
    # Run 1: no + BATTERY after the closing quote; the save it leaves is checked.
    set_tests_properties(holymapperel.${hm_key}.run1.prgram PROPERTIES
      FIXTURES_REQUIRED holymapperel_${hm_key}_run1
      PASS_REGULAR_EXPRESSION "prg ram \"${hm_text}\"")
    add_test(NAME holymapperel.${hm_key}.run1.sav
      COMMAND ${CMAKE_COMMAND} -DFILE=${hm_savedir}/${hm_stem}.sav -DSIZE=${hm_sav_size}
        -DOFFSET=256 -DHEX=5341564544415441
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/check_sav.cmake)
    set_tests_properties(holymapperel.${hm_key}.run1.sav PROPERTIES
      FIXTURES_REQUIRED holymapperel_${hm_key}_run1
      FIXTURES_SETUP holymapperel_${hm_key}_run1_sav)
    if(NOT hm_sav_byte STREQUAL "")
      string(REPLACE "=" ";" hm_sav_byte_parts "${hm_sav_byte}")
      list(GET hm_sav_byte_parts 0 hm_byte_offset)
      list(GET hm_sav_byte_parts 1 hm_byte_hex)
      add_test(NAME holymapperel.${hm_key}.run1.sav_byte
        COMMAND ${CMAKE_COMMAND} -DFILE=${hm_savedir}/${hm_stem}.sav -DSIZE=${hm_sav_size}
          -DOFFSET=${hm_byte_offset} -DHEX=${hm_byte_hex}
          -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/check_sav.cmake)
      set_tests_properties(holymapperel.${hm_key}.run1.sav_byte PROPERTIES
        FIXTURES_REQUIRED holymapperel_${hm_key}_run1
        FIXTURES_SETUP holymapperel_${hm_key}_run1_sav_byte)
      set_tests_properties(holymapperel.${hm_key}.run2.dump PROPERTIES
        FIXTURES_REQUIRED holymapperel_${hm_key}_run1_sav_byte)
    endif()
    add_test(NAME holymapperel.${hm_key}.run1.sav_copy
      COMMAND ${CMAKE_COMMAND} -E copy ${hm_savedir}/${hm_stem}.sav
        ${NESTURBATOR_HM_DIR}/${hm_key}-run1.sav)
    set_tests_properties(holymapperel.${hm_key}.run1.sav_copy PROPERTIES
      FIXTURES_REQUIRED holymapperel_${hm_key}_run1
      FIXTURES_SETUP holymapperel_${hm_key}_run1_savcopy)
    # Run 2 starts only after run 1, its save check and the private copy.
    set_tests_properties(holymapperel.${hm_key}.run2.dump PROPERTIES
      FIXTURES_REQUIRED "holymapperel_${hm_key}_run1_sav;holymapperel_${hm_key}_run1_savcopy")
    add_test(NAME libretro.parity.${hm_key}.run1
      COMMAND libretro.host --rom-parity $<TARGET_FILE:nesturbator_libretro>
        ${hm_rom} ${hm_frames} ${NESTURBATOR_HM_DIR}/${hm_key}-run1.ppm
        --sav-expect ${NESTURBATOR_HM_DIR}/${hm_key}-run1.sav
        --testcard ${NESTURBATOR_TESTFRAME_PPM})
    set_tests_properties(libretro.parity.${hm_key}.run1 PROPERTIES
      FIXTURES_REQUIRED "holymapperel_${hm_key}_run1;holymapperel_${hm_key}_run1_savcopy;testframe_ppm"
      TIMEOUT 120)
    add_test(NAME libretro.parity.${hm_key}.run2
      COMMAND libretro.host --rom-parity $<TARGET_FILE:nesturbator_libretro>
        ${hm_rom} ${hm_frames} ${NESTURBATOR_HM_DIR}/${hm_key}-run2.ppm
        --sav-in ${NESTURBATOR_HM_DIR}/${hm_key}-run1.sav
        --sav-expect ${hm_savedir}/${hm_stem}.sav
        --testcard ${NESTURBATOR_TESTFRAME_PPM})
    set_tests_properties(libretro.parity.${hm_key}.run2 PROPERTIES
      FIXTURES_REQUIRED "holymapperel_${hm_key}_run2;holymapperel_${hm_key}_run1_savcopy;testframe_ppm"
      TIMEOUT 120)
    set_tests_properties(holymapperel.${hm_key}.run2.prgram PROPERTIES
      FIXTURES_REQUIRED holymapperel_${hm_key}_run2
      PASS_REGULAR_EXPRESSION "prg ram \"${hm_text} \\+ BATTERY\"")
  else()
    # NESTURBATOR_SHORT_RUNS: parity compares module and runner frames, not committed hashes,
    # so a run longer than 100 frames compares frame 100; the ci legs compare the full run.
    set(hm_parity_frames ${hm_frames})
    set(hm_parity_ppm ${NESTURBATOR_HM_DIR}/${hm_key}.ppm)
    set(hm_extra_dump)
    if(NESTURBATOR_SHORT_RUNS AND hm_frames GREATER 100)
      set(hm_parity_frames 100)
      set(hm_parity_ppm ${NESTURBATOR_HM_DIR}/${hm_key}-f100.ppm)
      set(hm_extra_dump --dump-frame "100:${hm_parity_ppm}")
    endif()
    nesturbator_runner_test(NAME holymapperel.${hm_key}.dump
      ARGS --rom ${hm_rom} --frames ${hm_frames}
        --dump-frame "${hm_frames}:${NESTURBATOR_HM_DIR}/${hm_key}.ppm" ${hm_extra_dump}
      EXIT 0 IGNORE_STDOUT)
    set_tests_properties(holymapperel.${hm_key}.dump PROPERTIES
      FIXTURES_SETUP holymapperel_${hm_key} TIMEOUT 120)
    # D-18: the module shows the same result screen as the runner.
    add_test(NAME libretro.parity.${hm_key}
      COMMAND libretro.host --rom-parity $<TARGET_FILE:nesturbator_libretro>
        ${hm_rom} ${hm_parity_frames} ${hm_parity_ppm}
        --testcard ${NESTURBATOR_TESTFRAME_PPM})
    set_tests_properties(libretro.parity.${hm_key} PROPERTIES
      FIXTURES_REQUIRED "holymapperel_${hm_key};testframe_ppm" TIMEOUT 120)
    add_test(NAME holymapperel.${hm_key}.decode
      COMMAND holymapperel-decode ${NESTURBATOR_HM_DIR}/${hm_key}.ppm)
    set_tests_properties(holymapperel.${hm_key}.decode PROPERTIES
      FIXTURES_REQUIRED holymapperel_${hm_key}
      PASS_REGULAR_EXPRESSION "^holymapperel: code 0000 \\(")
    add_test(NAME holymapperel.${hm_key}.prgram
      COMMAND holymapperel-decode --prg-ram ${NESTURBATOR_HM_DIR}/${hm_key}.ppm)
    set_tests_properties(holymapperel.${hm_key}.prgram PROPERTIES
      FIXTURES_REQUIRED holymapperel_${hm_key}
      PASS_REGULAR_EXPRESSION "prg ram \"${hm_text}\"")
  endif()
endforeach()

# The decoder's unit cases and font table, and the branches that must exit 2:
# the test card, a frame taken before the result screen exists, and a file that
# is not a 256x240 P6 image.
add_executable(holymapperel.test_decode holymapperel/test_decode.c)
nesturbator_warnings(holymapperel.test_decode)
target_compile_definitions(holymapperel.test_decode PRIVATE
  $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
add_test(NAME holymapperel.decode.unit COMMAND holymapperel.test_decode)
add_test(NAME holymapperel.glyphs
  COMMAND holymapperel.test_decode --glyphs ${PROJECT_SOURCE_DIR}/tests/roms/hm/M3_P32K_C32K_H.nes)
nesturbator_runner_test(NAME holymapperel.early.dump
  ARGS --rom ${PROJECT_SOURCE_DIR}/tests/roms/hm/M3_P32K_C32K_H.nes --frames 1
    --dump-frame "1:${NESTURBATOR_HM_DIR}/early.ppm"
  EXIT 0 IGNORE_STDOUT)
set_tests_properties(holymapperel.early.dump PROPERTIES FIXTURES_SETUP holymapperel_early)
file(WRITE ${NESTURBATOR_HM_DIR}/badsize.ppm "P6\n16 16\n255\n")
function(holymapperel_exit2 name file)
  add_test(NAME ${name}
    COMMAND ${CMAKE_COMMAND}
      "-DCMD=$<TARGET_FILE:holymapperel-decode>;${file}"
      -DEXPECT_EXIT=2
      -DIGNORE_STDOUT=ON
      -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/expect_output.cmake")
endfunction()
holymapperel_exit2(holymapperel.decode.testcard ${NESTURBATOR_TESTFRAME_PPM})
set_tests_properties(holymapperel.decode.testcard PROPERTIES FIXTURES_REQUIRED testframe_ppm)
holymapperel_exit2(holymapperel.decode.early ${NESTURBATOR_HM_DIR}/early.ppm)
set_tests_properties(holymapperel.decode.early PROPERTIES FIXTURES_REQUIRED holymapperel_early)
holymapperel_exit2(holymapperel.decode.badsize ${NESTURBATOR_HM_DIR}/badsize.ppm)

# CI's hash-equality job compares every game boot and DABG movie milestone
# written by write_hashes.cmake on each platform. NESTURBATOR_SHORT_RUNS (preset asan) leaves
# both tests out: the movies have a fixed length, the asan job uploads no hashes, and the same
# ROMs and runner paths run under ASan through runner.dump.*, holymapperel.*.dump and
# runner.hash_audio.*. The ci legs run them.
if(NOT NESTURBATOR_SHORT_RUNS)
add_test(NAME runner.write_hashes
  COMMAND ${CMAKE_COMMAND} -DBUILD=${PROJECT_BINARY_DIR}
    -DOUT=${CMAKE_CURRENT_BINARY_DIR}/hashes.txt
    -DMOVIE_WRITER=$<TARGET_FILE:runner.game_movie>
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/write_hashes.cmake)
# In parallel mode CTest starts tests in descending COST and a fresh build has
# no cost history. On CI run 38011892333 runner.write_hashes and the two
# vectors.registration_policy tests took up to 48 s, 107 s and 21 s of the
# macos-15-intel and asan legs, so they start first.
set_tests_properties(runner.write_hashes PROPERTIES FIXTURES_SETUP hashes_txt COST 50)
add_test(NAME runner.write_hashes.content
  COMMAND ${CMAKE_COMMAND} -E compare_files
    ${CMAKE_CURRENT_BINARY_DIR}/hashes.txt ${CMAKE_CURRENT_SOURCE_DIR}/runner/hashes.txt)
set_tests_properties(runner.write_hashes.content PROPERTIES FIXTURES_REQUIRED hashes_txt)
endif()

# D-15: the state_format row of runner/hashes.txt equals NESTURBATOR_STATE_FORMAT_VERSION.
# It reads only source files, so it runs in every preset.
add_test(NAME state.format_pin
  COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/state_format_pin.cmake)
set_tests_properties(state.format_pin PROPERTIES TIMEOUT 30)

# Keep the CI cross-platform hash inventory contract executable locally.
add_test(NAME hash.inventory.self_test
  COMMAND ${CMAKE_COMMAND}
    -DSELFTEST=ON
    "-DWORK=${PROJECT_BINARY_DIR}/hash-inventory-selftest"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/hash_inventory.cmake)
set_tests_properties(hash.inventory.self_test PROPERTIES TIMEOUT 30)

# Phase 20: offline positive and negative controls for the shared hosted
# evidence validator. The fixtures are generated only in the build tree.
add_executable(phase20_fake_gh runner/phase20_fake_gh.c)
nesturbator_warnings(phase20_fake_gh)
target_compile_definitions(phase20_fake_gh PRIVATE
  $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
add_test(NAME phase20.hosted_evidence.controls
  COMMAND ${CMAKE_COMMAND}
    -DCASE=all
    "-DSOURCE_DIR=${PROJECT_SOURCE_DIR}"
    "-DRUNNER=$<TARGET_FILE:nesturbator-run>"
    "-DROM=${PROJECT_SOURCE_DIR}/tests/roms/accuracycoin.nes"
    "-DWORK=${PROJECT_BINARY_DIR}/phase20-hosted-evidence-controls"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/phase20_hosted_evidence_controls.cmake)
set_tests_properties(phase20.hosted_evidence.controls PROPERTIES TIMEOUT 120 RUN_SERIAL TRUE)
add_test(NAME phase20.hosted_evidence.adapter
  COMMAND ${CMAKE_COMMAND}
    -DCASE=adapter
    "-DSOURCE_DIR=${PROJECT_SOURCE_DIR}"
    "-DRUNNER=$<TARGET_FILE:nesturbator-run>"
    "-DROM=${PROJECT_SOURCE_DIR}/tests/roms/accuracycoin.nes"
    "-DFAKE_GH=$<TARGET_FILE:phase20_fake_gh>"
    "-DWORK=${PROJECT_BINARY_DIR}/phase20-hosted-evidence-adapter"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/phase20_hosted_evidence_controls.cmake)
set_tests_properties(phase20.hosted_evidence.adapter PROPERTIES TIMEOUT 120 RUN_SERIAL TRUE)
add_test(NAME phase20.hosted_evidence.availability
  COMMAND ${CMAKE_COMMAND}
    -DCASE=availability
    "-DSOURCE_DIR=${PROJECT_SOURCE_DIR}"
    "-DRUNNER=$<TARGET_FILE:nesturbator-run>"
    "-DROM=${PROJECT_SOURCE_DIR}/tests/roms/accuracycoin.nes"
    "-DFAKE_GH=$<TARGET_FILE:phase20_fake_gh>"
    "-DWORK=${PROJECT_BINARY_DIR}/phase20-hosted-evidence-availability"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/phase20_hosted_evidence_controls.cmake)
set_tests_properties(phase20.hosted_evidence.availability PROPERTIES TIMEOUT 120 RUN_SERIAL TRUE)

# Phase 21: verify its acceptance contract through the shared Phase 20 collector.
add_test(NAME phase21.hosted_evidence.controls
  COMMAND ${CMAKE_COMMAND}
    "-DSOURCE_DIR=${PROJECT_SOURCE_DIR}"
    "-DRUNNER=$<TARGET_FILE:nesturbator-run>"
    "-DROM=${PROJECT_SOURCE_DIR}/tests/roms/accuracycoin.nes"
    "-DWORK=${PROJECT_BINARY_DIR}/phase21-hosted-evidence-controls"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/phase21_hosted_evidence_controls.cmake)
set_tests_properties(phase21.hosted_evidence.controls PROPERTIES TIMEOUT 120 RUN_SERIAL TRUE)

# AccuracyCoin page runs are kept separate so a failure names the page whose
# RAM-derived results diverged from the committed scoreboard.
set(ACCURACYCOIN_ROM ${PROJECT_SOURCE_DIR}/tests/roms/accuracycoin.nes)
set(ACCURACYCOIN_OLD_ROM ${PROJECT_SOURCE_DIR}/tests/roms/accuracycoin-673ef.nes)
set(ACCURACYCOIN_SCOREBOARD ${CMAKE_CURRENT_SOURCE_DIR}/accuracy/scoreboard.txt)
set(ACCURACYCOIN_OLD_SCOREBOARD ${CMAKE_CURRENT_SOURCE_DIR}/accuracy/old-673ef-scoreboard.txt)
set(ACCURACYCOIN_PROTECTED_WORK ${PROJECT_BINARY_DIR}/accuracycoin/protected-observations)
set(ACCURACYCOIN_PROTECTED_MAX_PAGES 22)
set(ACCURACYCOIN_PROTECTED_PAGE_WALL_SECONDS 30)
math(EXPR ACCURACYCOIN_PROTECTED_SETUP_TIMEOUT
  "(${ACCURACYCOIN_PROTECTED_MAX_PAGES} + 3) * ${ACCURACYCOIN_PROTECTED_PAGE_WALL_SECONDS} + 10")
foreach(page IN ITEMS 2 14 17)
  add_test(NAME accuracycoin.page${page}
    COMMAND ${CMAKE_COMMAND}
      "-DCMD=$<TARGET_FILE:nesturbator-run>;--accuracycoin-page;${page};--rom;${ACCURACYCOIN_OLD_ROM};--scoreboard;${ACCURACYCOIN_OLD_SCOREBOARD}"
      "-DEXPECT=accuracycoin page ${page} PASS"
      -DEXPECT_EXIT=0
      -DIGNORE_STDOUT=TRUE
      -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/expect_output.cmake)
  set_tests_properties(accuracycoin.page${page} PROPERTIES TIMEOUT 60)
endforeach()

# The canonical new-pin fixture exercises page-3 token expansion, start
# transition and report generation through the production runner.
set(ACCURACYCOIN_COMPARISON_ROM ${ACCURACYCOIN_ROM})
option(NESTURBATOR_ACCURACYCOIN_FULL
  "Register the measured 22-page AccuracyCoin campaign test" OFF)
add_executable(accuracycoin.conformance
  accuracy/test_conformance.c
  ../runner/accuracycoin.c
  ../runner/audio_hash.c
  ../runner/clock.c
  ../runner/sha256.c)
target_include_directories(accuracycoin.conformance PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/runner)
target_link_libraries(accuracycoin.conformance PRIVATE nesturbator::nesturbator)
target_compile_definitions(accuracycoin.conformance PRIVATE
  $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
nesturbator_warnings(accuracycoin.conformance)
set(ACCURACYCOIN_TRACER_WORK ${CMAKE_CURRENT_BINARY_DIR}/accuracycoin/page3-tracer)
add_test(NAME accuracycoin.tracer
  COMMAND ${CMAKE_COMMAND}
    "-DRUNNER=$<TARGET_FILE:nesturbator-run>"
    "-DROM=${ACCURACYCOIN_COMPARISON_ROM}"
    "-DWORK=${ACCURACYCOIN_TRACER_WORK}"
    "-DSOURCE_DIR=${PROJECT_SOURCE_DIR}"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/accuracycoin_tracer.cmake)
set_tests_properties(accuracycoin.tracer PROPERTIES TIMEOUT 90)
add_test(NAME accuracycoin.inventory
  COMMAND accuracycoin.conformance inventory
    ${ACCURACYCOIN_COMPARISON_ROM}
    ${ACCURACYCOIN_OLD_ROM})
add_test(NAME accuracycoin.classifier
  COMMAND accuracycoin.conformance classifier)
add_test(NAME accuracycoin.migration_source
  COMMAND accuracycoin.conformance migration
    ${CMAKE_CURRENT_SOURCE_DIR}/accuracy/migration.tsv
    ${CMAKE_CURRENT_SOURCE_DIR}/accuracy/old-673ef-scoreboard.txt)
add_test(NAME accuracycoin.activation
  COMMAND ${CMAKE_COMMAND}
    "-DSOURCE_DIR=${PROJECT_SOURCE_DIR}"
    "-DRUNNER=$<TARGET_FILE:nesturbator-run>"
    "-DCONFORMANCE=$<TARGET_FILE:accuracycoin.conformance>"
    "-DNEW_ROM=${ACCURACYCOIN_ROM}"
    "-DOLD_ROM=${ACCURACYCOIN_OLD_ROM}"
    "-DSCOREBOARD=${ACCURACYCOIN_SCOREBOARD}"
    "-DBASELINE_METADATA=${CMAKE_CURRENT_SOURCE_DIR}/accuracy/baseline-metadata.txt"
    "-DOLD_SCOREBOARD=${ACCURACYCOIN_OLD_SCOREBOARD}"
    "-DMIGRATION=${CMAKE_CURRENT_SOURCE_DIR}/accuracy/migration.tsv"
    "-DFULL_ENABLED=${NESTURBATOR_ACCURACYCOIN_FULL}"
    "-DFULL_REPORT=${PROJECT_BINARY_DIR}/accuracycoin/full/report.tsv"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/accuracycoin_activation.cmake)
set_tests_properties(accuracycoin.inventory accuracycoin.classifier
  accuracycoin.migration_source accuracycoin.activation PROPERTIES TIMEOUT 90)
set_tests_properties(accuracycoin.inventory accuracycoin.classifier
  accuracycoin.migration_source PROPERTIES TIMEOUT 30)
add_test(NAME accuracycoin.driver
  COMMAND ${CMAKE_COMMAND}
    "-DCONFORMANCE=$<TARGET_FILE:accuracycoin.conformance>"
    "-DRUNNER=$<TARGET_FILE:nesturbator-run>"
    "-DROM=${ACCURACYCOIN_COMPARISON_ROM}"
    "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/accuracycoin/driver"
    "-DSOURCE_DIR=${PROJECT_SOURCE_DIR}"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/accuracycoin_driver.cmake)
add_test(NAME accuracycoin.report
  COMMAND accuracycoin.conformance report
    ${ACCURACYCOIN_COMPARISON_ROM}
    ${ACCURACYCOIN_OLD_ROM}
    ${CMAKE_CURRENT_BINARY_DIR}/accuracycoin/report)
# The report command creates its work directory's missing parents, so it does
# not depend on another test having run first (ctest --schedule-random).
add_test(NAME accuracycoin.report_scratch
  COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_CURRENT_BINARY_DIR}/accuracycoin_fresh)
set_tests_properties(accuracycoin.report_scratch PROPERTIES
  FIXTURES_SETUP accuracycoin_report_fresh)
add_test(NAME accuracycoin.report_fresh
  COMMAND accuracycoin.conformance report
    ${ACCURACYCOIN_COMPARISON_ROM}
    ${ACCURACYCOIN_OLD_ROM}
    ${CMAKE_CURRENT_BINARY_DIR}/accuracycoin_fresh/nested/report)
set_tests_properties(accuracycoin.report_fresh PROPERTIES
  FIXTURES_REQUIRED accuracycoin_report_fresh)
# JAM-01: the real observe path on page 3 with one byte patched in memory only.
add_test(NAME accuracycoin.jam
  COMMAND ${CMAKE_COMMAND}
    "-DCONFORMANCE=$<TARGET_FILE:accuracycoin.conformance>"
    "-DNEW_ROM=${ACCURACYCOIN_COMPARISON_ROM}"
    "-DOLD_ROM=${ACCURACYCOIN_OLD_ROM}"
    "-DOLD_SCOREBOARD=${ACCURACYCOIN_OLD_SCOREBOARD}"
    "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/accuracycoin/jam"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/accuracycoin_jam.cmake)
set_tests_properties(accuracycoin.jam PROPERTIES TIMEOUT 60)
add_test(NAME accuracycoin.outer_failure
  COMMAND ${CMAKE_COMMAND}
    "-DRUNNER=$<TARGET_FILE:nesturbator-run>"
    "-DROM=${ACCURACYCOIN_COMPARISON_ROM}"
    "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/accuracycoin/outer-failure"
    "-DSOURCE_DIR=${PROJECT_SOURCE_DIR}"
    -DBOOT_CAP=32 -DNAVIGATION_CAP=5 -DEXECUTION_CAP=1800
    -DPAGE_TIMEOUT=1 -DCOLLECT_TIMEOUT=20
    -DTEST_TIMEOUT_PAGE=1 -DTEST_PARTIAL_FAILURE_PAGE=2
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/accuracycoin_campaign.cmake)
set_tests_properties(accuracycoin.driver accuracycoin.report
  accuracycoin.outer_failure PROPERTIES TIMEOUT 60)
if(NESTURBATOR_ACCURACYCOIN_FULL)
  add_test(NAME accuracycoin.campaign
    COMMAND ${CMAKE_COMMAND}
      "-DRUNNER=$<TARGET_FILE:nesturbator-run>"
      "-DROM=${ACCURACYCOIN_COMPARISON_ROM}"
      "-DWORK=${PROJECT_BINARY_DIR}/accuracycoin/full"
      "-DSOURCE_DIR=${PROJECT_SOURCE_DIR}"
      -DBOOT_CAP=32 -DNAVIGATION_CAP=5 -DEXECUTION_CAP=1800
      -DPAGE_TIMEOUT=15 -DCOLLECT_TIMEOUT=10
      -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/accuracycoin_campaign.cmake)
  set_tests_properties(accuracycoin.campaign PROPERTIES TIMEOUT 370)
  set_tests_properties(accuracycoin.campaign PROPERTIES FIXTURES_SETUP accuracycoin_full_report)
  set_tests_properties(accuracycoin.activation PROPERTIES FIXTURES_REQUIRED accuracycoin_full_report)
endif()

# Protected scoring is based on six fresh cold invocations: the three old
# protected pages and their new-pin semantic mappings. The verdict test cannot
# reuse evidence from a prior CTest invocation.
add_test(NAME accuracycoin.protected_observation
  COMMAND ${CMAKE_COMMAND}
    "-DRUNNER=$<TARGET_FILE:nesturbator-run>"
    "-DVERIFIER=$<TARGET_FILE:accuracy.scoreboard>"
    "-DSOURCE_DIR=${PROJECT_SOURCE_DIR}"
    "-DOLD_ROM=${ACCURACYCOIN_OLD_ROM}"
    "-DNEW_ROM=${ACCURACYCOIN_COMPARISON_ROM}"
    "-DWORK=${ACCURACYCOIN_PROTECTED_WORK}"
    "-DPAGE_WALL_SECONDS=${ACCURACYCOIN_PROTECTED_PAGE_WALL_SECONDS}"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/accuracycoin_protected.cmake)
set_tests_properties(accuracycoin.protected_observation PROPERTIES
  FIXTURES_SETUP accuracycoin_protected_observations TIMEOUT ${ACCURACYCOIN_PROTECTED_SETUP_TIMEOUT})
add_test(NAME accuracycoin.protected COMMAND accuracy.scoreboard)
set_tests_properties(accuracycoin.protected PROPERTIES
  FIXTURES_REQUIRED accuracycoin_protected_observations TIMEOUT 30)

# Phase 20: require the six measured NMI/VBlank keys by full key and status.
# On the existing full-campaign lane this check follows the fresh campaign;
# other lanes check the committed candidate scoreboard without duplicating it.
set(ACCURACYCOIN_NMI_TARGET_WORK ${PROJECT_BINARY_DIR}/accuracycoin/nmi-target-controls)
add_test(NAME accuracycoin.nmi_targets
  COMMAND ${CMAKE_COMMAND}
    "-DSCOREBOARD=${ACCURACYCOIN_SCOREBOARD}"
    "-DGATE=${CMAKE_CURRENT_SOURCE_DIR}/cmake/nmi_targets.cmake"
    "-DWORK=${ACCURACYCOIN_NMI_TARGET_WORK}"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/accuracycoin_nmi_targets_control.cmake)
set_tests_properties(accuracycoin.nmi_targets PROPERTIES TIMEOUT 15)
if(NESTURBATOR_ACCURACYCOIN_FULL)
  set_tests_properties(accuracycoin.nmi_targets PROPERTIES
    FIXTURES_REQUIRED accuracycoin_full_report)
endif()

# Phase 21: require every named interrupt, NMI/VBlank, and sprite target by full key and status.
set(ACCURACYCOIN_V5_TARGET_WORK ${PROJECT_BINARY_DIR}/accuracycoin/v5-target-controls)
add_test(NAME accuracycoin.v5_targets
  COMMAND ${CMAKE_COMMAND}
    "-DSCOREBOARD=${ACCURACYCOIN_SCOREBOARD}"
    "-DGATE=${CMAKE_CURRENT_SOURCE_DIR}/cmake/v5_targets.cmake"
    "-DWORK=${ACCURACYCOIN_V5_TARGET_WORK}"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/accuracycoin_v5_targets_control.cmake)
set_tests_properties(accuracycoin.v5_targets PROPERTIES TIMEOUT 30)
if(NESTURBATOR_ACCURACYCOIN_FULL)
  set_tests_properties(accuracycoin.v5_targets PROPERTIES
    FIXTURES_REQUIRED accuracycoin_full_report)
endif()

# Core tests: programs using tests/check.h, through the public header only.
# Each exits 0 when every check passes.
function(nesturbator_core_test name source)
  add_executable(${name} ${source})
  target_link_libraries(${name} PRIVATE nesturbator::nesturbator)
  nesturbator_warnings(${name})
  add_test(NAME ${name} COMMAND ${name} ${ARGN})
endfunction()

# D-11: version, info, create, every create status and the allocators.
nesturbator_core_test(core.api core/test_api.c)
# D-11: run_frame statuses, buffer rules, the D-09 audio period, the D-02
# boundary pixels and two independent instances.
nesturbator_core_test(core.frame core/test_frame.c)
set_tests_properties(core.frame PROPERTIES TIMEOUT 30)
nesturbator_core_test(core.apu core/test_apu.c)
nesturbator_core_test(core.synth core/test_synth.c)
target_include_directories(core.synth PRIVATE ${PROJECT_SOURCE_DIR}/src)
nesturbator_core_test(core.cartridge core/test_cartridge.c)
target_include_directories(core.cartridge PRIVATE ${PROJECT_SOURCE_DIR}/src)
# D-18: soft reset on a synthetic NROM image with a trainer.
# NESTURBATOR_SHORT_RUNS (preset asan): the reset-point sweep steps by 20 frames; ci sweeps all.
if(NESTURBATOR_SHORT_RUNS)
  nesturbator_core_test(core.reset core/test_reset.c --short)
else()
  nesturbator_core_test(core.reset core/test_reset.c)
endif()
target_include_directories(core.reset PRIVATE ${PROJECT_SOURCE_DIR}/src)
# D-02: the PPUCTRL NMI edge, one NMI per frame from a handler that rewrites $2000.
nesturbator_core_test(core.nmi_edge core/test_nmi_edge.c)
# NMI-02: source-shaped STA/INC polling sequences and guest NMI entry markers.
nesturbator_core_test(core.nmi_poll core/test_nmi_poll.c)
target_include_directories(core.nmi_poll PRIVATE ${PROJECT_SOURCE_DIR}/src)
set_tests_properties(core.nmi_poll PROPERTIES TIMEOUT 10)
# INT-01: real APU frame IRQ source, production bus capture and old-I poll cases.
nesturbator_core_test(core.irq_poll core/test_irq_poll.c)
target_include_directories(core.irq_poll PRIVATE ${PROJECT_SOURCE_DIR}/src)
set_tests_properties(core.irq_poll PROPERTIES TIMEOUT 10)
# INT-02 (D-05 to D-11): test-only bus wrappers schedule constructed input
# margins, then delegate every action to the production bus implementation.
# The alternate names keep the shipped library's bus object out of this
# executable; no callback or symbol is added to the library ABI.
add_library(interrupt_entry_production_bus OBJECT ${PROJECT_SOURCE_DIR}/src/bus.c)
nesturbator_core_flags(interrupt_entry_production_bus)
target_include_directories(interrupt_entry_production_bus PRIVATE
  ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include)
target_compile_definitions(interrupt_entry_production_bus PRIVATE
  nesturbator__bus_read=nesturbator__bus_read_production
  nesturbator__bus_write=nesturbator__bus_write_production)
add_executable(core.interrupt_entry
  core/test_interrupt_entry.c
  $<TARGET_OBJECTS:interrupt_entry_production_bus>)
target_link_libraries(core.interrupt_entry PRIVATE nesturbator::nesturbator)
target_include_directories(core.interrupt_entry PRIVATE ${PROJECT_SOURCE_DIR}/src)
nesturbator_warnings(core.interrupt_entry)
add_test(NAME core.interrupt_entry COMMAND core.interrupt_entry)
set_tests_properties(core.interrupt_entry PROPERTIES TIMEOUT 30)
# INT-02 native continuation: production entry stages are captured through a
# test-only observer that delegates to src/bus.c, then replayed through the
# public save/load API and ordinary cpu_step.
add_executable(core.interrupt_state
  core/test_interrupt_state.c
  $<TARGET_OBJECTS:interrupt_entry_production_bus>)
target_link_libraries(core.interrupt_state PRIVATE nesturbator::nesturbator)
target_include_directories(core.interrupt_state PRIVATE ${PROJECT_SOURCE_DIR}/src)
nesturbator_warnings(core.interrupt_state)
add_test(NAME core.interrupt_state COMMAND core.interrupt_state)
set_tests_properties(core.interrupt_state PROPERTIES TIMEOUT 120)
# KEEP-03 (D-16 to D-19): exact enabled-NMI state replay and omitted-history oracle.
nesturbator_core_test(core.nmi_state core/test_nmi_state.c)
target_include_directories(core.nmi_state PRIVATE ${PROJECT_SOURCE_DIR}/src)
set_tests_properties(core.nmi_state PROPERTIES TIMEOUT 30)
# MAP-01, D-04, D-05, D-06, D-08: cycle-stamped mapper writes, the IRQ OR, the nametable map and reset.
nesturbator_core_test(core.mapper core/test_mapper.c)
target_include_directories(core.mapper PRIVATE ${PROJECT_SOURCE_DIR}/src)
nesturbator_core_test(core.mapper_discrete core/test_mapper_discrete.c)
target_include_directories(core.mapper_discrete PRIVATE ${PROJECT_SOURCE_DIR}/src)
# BOARD-02 (D-10, D-14 to D-17): the MMC1 board on synthetic images.
nesturbator_core_test(core.mapper_mmc1 core/test_mapper_mmc1.c)
target_include_directories(core.mapper_mmc1 PRIVATE ${PROJECT_SOURCE_DIR}/src)
# BOARD-03 (D-04 to D-09): the MMC3 board, hook level and through the CPU.
nesturbator_core_test(core.mapper_mmc3 core/test_mapper_mmc3.c)
target_include_directories(core.mapper_mmc3 PRIVATE ${PROJECT_SOURCE_DIR}/src)
# SAVE-01 (D-06, D-11, D-12): the battery span, its address and the write counter.
nesturbator_core_test(core.save core/test_save.c)
target_include_directories(core.save PRIVATE ${PROJECT_SOURCE_DIR}/src)
# STATE-05 (D-04 step 8): a state load counts in the save generation only when the battery bytes
# change; the 600-load case, refused loads and battery-less cartridges.
nesturbator_core_test(core.state_generation core/test_state_generation.c)
target_include_directories(core.state_generation PRIVATE ${PROJECT_SOURCE_DIR}/src)
# D-16: frame-boundary invariants (no frame-scoped pointer, empty PCM ring); white box.
nesturbator_core_test(core.boundary core/test_boundary.c)
target_include_directories(core.boundary PRIVATE ${PROJECT_SOURCE_DIR}/src)
# STATE-01, STATE-02 (D-01 to D-08, D-16): save states. The dabg round trip, the header layout,
# the CRC vector and the D-04 and D-05 check orders; white box.
nesturbator_core_test(core.state core/test_state.c ${PROJECT_SOURCE_DIR}/tests/roms/dabg.nes)
target_include_directories(core.state PRIVATE ${PROJECT_SOURCE_DIR}/src)
# STATE-03 (D-08): the fork matrix. Every fork point of eight committed ROMs on six boards loads
# into an instance pre-filled with nesteroids.nes and poisoned, then continues identically; saving
# every frame changes nothing. NESTURBATOR_SHORT_RUNS (preset asan) uses forks {0, 1, 30} over 60 frames.
set(FORK_ROMS
  ${PROJECT_SOURCE_DIR}/tests/roms/nesteroids.nes
  ${PROJECT_SOURCE_DIR}/tests/roms/dabg.nes
  ${PROJECT_SOURCE_DIR}/tests/roms/runner.nes
  ${PROJECT_SOURCE_DIR}/tests/roms/hm/M1_P512K_CR8K_S32K.nes
  ${PROJECT_SOURCE_DIR}/tests/roms/hm/M2_P128K_CR8K_V.nes
  ${PROJECT_SOURCE_DIR}/tests/roms/hm/M3_P32K_C32K_H.nes
  ${PROJECT_SOURCE_DIR}/tests/roms/hm/M4_P128K_CR8K.nes
  ${PROJECT_SOURCE_DIR}/tests/roms/hm/M4_P256K_C256K.nes
  ${PROJECT_SOURCE_DIR}/tests/roms/hm/M7_P128K_CR8K.nes)
# One CTest entry per ROM plus one for the stress images, so ctest runs them in parallel and no
# single entry carries the whole matrix (it is about 140 s of CPU in ci).
add_executable(core.state_fork core/test_state_fork.c)
target_link_libraries(core.state_fork PRIVATE nesturbator::nesturbator)
nesturbator_warnings(core.state_fork)
target_include_directories(core.state_fork PRIVATE ${PROJECT_SOURCE_DIR}/src)
set(FORK_MODE)
if(NESTURBATOR_SHORT_RUNS)
  set(FORK_MODE --short)
endif()
list(GET FORK_ROMS 0 FORK_OTHER)
list(SUBLIST FORK_ROMS 1 -1 FORK_GAMES)
foreach(rom IN LISTS FORK_GAMES)
  get_filename_component(rom_name ${rom} NAME_WE)
  add_test(NAME core.state_fork.${rom_name} COMMAND core.state_fork ${FORK_MODE} ${FORK_OTHER} ${rom})
  set_tests_properties(core.state_fork.${rom_name} PROPERTIES TIMEOUT 300 COST 20)
endforeach()
add_test(NAME core.state_fork.stress COMMAND core.state_fork ${FORK_MODE} --stress ${FORK_OTHER})
set_tests_properties(core.state_fork.stress PROPERTIES TIMEOUT 300 COST 60)
# KEEP-03 (D-19): a focused NROM public-boundary fork checks every guest NMI event
# and resumes frame 0 through successive saved boundaries without replacing the board matrix.
add_test(NAME core.state_fork.nmi COMMAND core.state_fork --nmi)
set_tests_properties(core.state_fork.nmi PROPERTIES TIMEOUT 30)
# INT-02: extend the retained board-family state-fork executable with a genuine
# production-action interrupt-entry cut loaded through the native state API.
add_test(NAME core.state_fork.entry COMMAND core.state_fork --entry)
set_tests_properties(core.state_fork.entry PROPERTIES TIMEOUT 30)
# STATE-04 (D-03, D-04): all-or-nothing load. Cross-ROM, version, header_size, truncation, a flip of
# every byte, the CRC-recomputed field sweep, the at-bound table and the ppu_ticks gap, each checked
# for the D-04 status and a byte-identical instance. NESTURBATOR_SHORT_RUNS (preset asan) strides
# the cartridge-RAM flips.
if(NESTURBATOR_SHORT_RUNS)
  nesturbator_core_test(core.state_reject core/test_state_reject.c --short ${FORK_ROMS})
else()
  nesturbator_core_test(core.state_reject core/test_state_reject.c ${FORK_ROMS})
endif()
target_include_directories(core.state_reject PRIVATE ${PROJECT_SOURCE_DIR}/src)
set_tests_properties(core.state_reject PROPERTIES TIMEOUT 300 COST 60)
# LR-02 (D-12): RetroArch's second-instance run-ahead mode as a library property. Instance A runs
# the real timeline, B loads A's state each tick and runs N frames; the visible frames equal the
# one-instance model's, A equals an uninterrupted run, and the nine ROMs and two stress images
# pass. NESTURBATOR_SHORT_RUNS (preset asan) runs 30 ticks instead of 180.
if(NESTURBATOR_SHORT_RUNS)
  nesturbator_core_test(core.state_runahead core/test_state_runahead.c --short ${FORK_ROMS})
else()
  nesturbator_core_test(core.state_runahead core/test_state_runahead.c ${FORK_ROMS})
endif()
target_include_directories(core.state_runahead PRIVATE ${PROJECT_SOURCE_DIR}/src)
# TIMEOUT 600 and 30 ticks under asan, for the reason given at libretro.runahead.
set_tests_properties(core.state_runahead PROPERTIES TIMEOUT 600 COST 30)
add_executable(core.controller core/test_controller.c)
target_link_libraries(core.controller PRIVATE nesturbator::nesturbator)
target_include_directories(core.controller PRIVATE ${PROJECT_SOURCE_DIR}/src)
nesturbator_warnings(core.controller)
add_test(NAME core.controller COMMAND core.controller)

# Focused CPU-visible PPU register and dot-boundary tests.
add_executable(ppu.nmi_races ppu/test_nmi_races.c)
target_link_libraries(ppu.nmi_races PRIVATE nesturbator::nesturbator)
target_include_directories(ppu.nmi_races PRIVATE ${PROJECT_SOURCE_DIR}/src)
nesturbator_warnings(ppu.nmi_races)
add_test(NAME ppu.nmi_races COMMAND ppu.nmi_races)
set_tests_properties(ppu.nmi_races PROPERTIES TIMEOUT 10)
add_executable(ppu.registers ppu/test_registers.c)
target_link_libraries(ppu.registers PRIVATE nesturbator::nesturbator)
target_include_directories(ppu.registers PRIVATE ${PROJECT_SOURCE_DIR}/src)
nesturbator_warnings(ppu.registers)
add_test(NAME ppu.registers COMMAND ppu.registers)
add_executable(ppu.render ppu/test_render.c)
target_link_libraries(ppu.render PRIVATE nesturbator::nesturbator)
target_include_directories(ppu.render PRIVATE ${PROJECT_SOURCE_DIR}/src)
nesturbator_warnings(ppu.render)
add_test(NAME ppu.render COMMAND ppu.render)
add_executable(ppu.sprites ppu/test_sprites.c)
target_link_libraries(ppu.sprites PRIVATE nesturbator::nesturbator)
target_include_directories(ppu.sprites PRIVATE ${PROJECT_SOURCE_DIR}/src)
nesturbator_warnings(ppu.sprites)
add_test(NAME ppu.sprites COMMAND ppu.sprites)
# MAP-02, D-13: v, bus and A12 per dot.
add_executable(ppu.fetch ppu/test_fetch.c)
target_link_libraries(ppu.fetch PRIVATE nesturbator::nesturbator)
target_include_directories(ppu.fetch PRIVATE ${PROJECT_SOURCE_DIR}/src)
nesturbator_warnings(ppu.fetch)
add_test(NAME ppu.fetch COMMAND ppu.fetch)
# MAP-02, D-14, D-15: mid-frame $2005/$2006 splits against hand-derived origins.
add_executable(ppu.split_scroll ppu/test_split_scroll.c)
target_link_libraries(ppu.split_scroll PRIVATE nesturbator::nesturbator)
target_include_directories(ppu.split_scroll PRIVATE ${PROJECT_SOURCE_DIR}/src)
nesturbator_warnings(ppu.split_scroll)
add_test(NAME ppu.split_scroll COMMAND ppu.split_scroll)
# BOARD-03, D-04, D-09: the MMC3 counter clocked by real PPU A12 edges.
add_executable(ppu.mmc3 ppu/test_mmc3.c)
target_link_libraries(ppu.mmc3 PRIVATE nesturbator::nesturbator)
target_include_directories(ppu.mmc3 PRIVATE ${PROJECT_SOURCE_DIR}/src)
nesturbator_warnings(ppu.mmc3)
add_test(NAME ppu.mmc3 COMMAND ppu.mmc3)

# The same entry point is linked into the Linux libFuzzer binary and replayed
# from checked-in seeds on every CI platform. Refuse an empty corpus at configure
# time so a missing directory cannot turn this into a passing no-op.
file(GLOB NESTURBATOR_ROM_LOADER_CORPUS CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/fuzz/corpus/*")
if(NOT NESTURBATOR_ROM_LOADER_CORPUS)
  message(FATAL_ERROR "ROM loader fuzz corpus is missing or empty")
endif()
add_executable(fuzz.rom_loader fuzz/rom_loader.c)
target_link_libraries(fuzz.rom_loader PRIVATE nesturbator::nesturbator)
target_compile_definitions(fuzz.rom_loader PRIVATE NESTURBATOR_FUZZ_REPLAY=1)
nesturbator_warnings(fuzz.rom_loader)
add_test(NAME fuzz.regress
  COMMAND fuzz.rom_loader ${NESTURBATOR_ROM_LOADER_CORPUS})
set_tests_properties(fuzz.regress PROPERTIES TIMEOUT 30)

# Phase 13, D-30 to D-35: the state loader fuzz target. The seeds are generated by the harness
# (fuzz.state_loader --write-seeds) and committed; fuzz.state requires them to equal what the
# harness generates and every valid-* seed to load, fuzz.state.regress replays every committed file.
file(GLOB NESTURBATOR_STATE_CORPUS CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/fuzz/corpus_state/*")
if(NOT NESTURBATOR_STATE_CORPUS)
  message(FATAL_ERROR "state loader fuzz corpus is missing or empty")
endif()
add_executable(fuzz.state_loader fuzz/state_loader.c)
target_link_libraries(fuzz.state_loader PRIVATE nesturbator::nesturbator)
target_compile_definitions(fuzz.state_loader PRIVATE NESTURBATOR_FUZZ_REPLAY=1)
nesturbator_warnings(fuzz.state_loader)
if(MSVC)
  target_compile_definitions(fuzz.state_loader PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
add_test(NAME fuzz.state
  COMMAND fuzz.state_loader --check-seeds "${CMAKE_CURRENT_SOURCE_DIR}/fuzz/corpus_state"
    ${NESTURBATOR_STATE_CORPUS})
add_test(NAME fuzz.state.regress
  COMMAND fuzz.state_loader ${NESTURBATOR_STATE_CORPUS})
add_test(NAME fuzz.state.jam_oracle
  COMMAND fuzz.state_loader --jam-oracle-regression)
add_test(NAME fuzz.state.jam_oracle_divergence
  COMMAND ${CMAKE_COMMAND}
    "-DCMD=$<TARGET_FILE:fuzz.state_loader>;--jam-oracle-divergence"
    -DEXPECT_NONZERO=ON
    -DIGNORE_STDOUT=ON
    "-DEXPECT_ERR=two instances loading the same state produced different post-run states"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/expect_output.cmake")
set_tests_properties(fuzz.state fuzz.state.regress fuzz.state.jam_oracle
  fuzz.state.jam_oracle_divergence PROPERTIES TIMEOUT 30)

# Phase 18, Plan 01: exercise a compact descriptor through generated NROM execution,
# frame-scheduled input and a restored continuation. The corpus stores no ROM bytes.
file(GLOB NESTURBATOR_EXECUTION_CORPUS CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/fuzz/corpus_execution/*")
if(NOT NESTURBATOR_EXECUTION_CORPUS)
  message(FATAL_ERROR "execution fuzz corpus is missing or empty")
endif()
list(LENGTH NESTURBATOR_EXECUTION_CORPUS NESTURBATOR_EXECUTION_CORPUS_COUNT)
if(NOT NESTURBATOR_EXECUTION_CORPUS_COUNT EQUAL 8)
  message(FATAL_ERROR "execution fuzz corpus must contain six family, loop and JAM descriptors; found ${NESTURBATOR_EXECUTION_CORPUS_COUNT}")
endif()
set(NESTURBATOR_EXECUTION_BUDGET_FILE "${CMAKE_CURRENT_SOURCE_DIR}/fuzz/execution-budgets.txt")
if(NOT EXISTS "${NESTURBATOR_EXECUTION_BUDGET_FILE}")
  message(FATAL_ERROR "execution budget file is missing")
endif()
set(NESTURBATOR_EXECUTION_CAPS)
foreach(cap IN ITEMS max_prg_bytes max_chr_bytes max_input_bytes max_generated_bytes
    max_state_bytes max_allocation_bytes max_frames_per_phase max_frame_calls max_operations)
  file(STRINGS "${NESTURBATOR_EXECUTION_BUDGET_FILE}" cap_lines
    REGEX "^execution[.]${cap}=[0-9]+$")
  list(LENGTH cap_lines cap_line_count)
  if(NOT cap_line_count EQUAL 1)
    message(FATAL_ERROR "execution budget must define exactly one positive execution.${cap}")
  endif()
  string(REGEX REPLACE "^execution[.]${cap}=" "" cap_value "${cap_lines}")
  if(NOT cap_value MATCHES "^[1-9][0-9]*$")
    message(FATAL_ERROR "execution.${cap} must be a positive integer")
  endif()
  string(TOUPPER "${cap}" upper_cap)
  list(APPEND NESTURBATOR_EXECUTION_CAPS "NT_EXEC_${upper_cap}=${cap_value}u")
  set("NESTURBATOR_EXECUTION_${cap}" "${cap_value}")
endforeach()
file(STRINGS "${NESTURBATOR_EXECUTION_BUDGET_FILE}" replay_timeout_lines
  REGEX "^execution[.]fixed_replay_timeout_seconds=[0-9]+$")
list(LENGTH replay_timeout_lines replay_timeout_count)
if(NOT replay_timeout_count EQUAL 1)
  message(FATAL_ERROR "execution budget must define exactly one fixed replay timeout")
endif()
string(REGEX REPLACE "^execution[.]fixed_replay_timeout_seconds=" ""
  NESTURBATOR_EXECUTION_REPLAY_TIMEOUT "${replay_timeout_lines}")
if(NOT NESTURBATOR_EXECUTION_REPLAY_TIMEOUT MATCHES "^[1-9][0-9]*$")
  message(FATAL_ERROR "execution fixed replay timeout must be a positive integer")
endif()
math(EXPR expected_execution_input
  "16 + ${NESTURBATOR_EXECUTION_max_prg_bytes} + ${NESTURBATOR_EXECUTION_max_chr_bytes}")
if(NOT expected_execution_input EQUAL NESTURBATOR_EXECUTION_max_input_bytes OR
   NOT expected_execution_input EQUAL NESTURBATOR_EXECUTION_max_generated_bytes)
  message(FATAL_ERROR "execution input/generated byte caps must match the PRG + CHR + descriptor size (${expected_execution_input})")
endif()
add_executable(fuzz.execution fuzz/execution.c)
target_link_libraries(fuzz.execution PRIVATE nesturbator::nesturbator)
target_compile_definitions(fuzz.execution PRIVATE NESTURBATOR_FUZZ_REPLAY=1 ${NESTURBATOR_EXECUTION_CAPS})
nesturbator_warnings(fuzz.execution)
if(MSVC)
  target_compile_definitions(fuzz.execution PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
add_test(NAME fuzz.execution.regress
  COMMAND fuzz.execution ${NESTURBATOR_EXECUTION_CORPUS})
add_test(NAME fuzz.execution.seeds
  COMMAND fuzz.execution --check-seeds ${NESTURBATOR_EXECUTION_CORPUS})
add_executable(fuzz.execution.controls fuzz/test_execution.c)
target_include_directories(fuzz.execution.controls PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(fuzz.execution.controls PRIVATE nesturbator::nesturbator)
target_compile_definitions(fuzz.execution.controls PRIVATE NESTURBATOR_FUZZ_REPLAY=1)
nesturbator_warnings(fuzz.execution.controls)
if(MSVC)
  target_compile_definitions(fuzz.execution.controls PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
add_test(NAME fuzz.execution.controls COMMAND fuzz.execution.controls)
add_test(NAME fuzz.execution.oracle_failure
  COMMAND ${CMAKE_COMMAND}
    "-DCMD=$<TARGET_FILE:fuzz.execution>;--oracle-failure"
    -DEXPECT_EXIT=1
    -DIGNORE_STDOUT=ON
    "-DEXPECT_ERR=intentional execution oracle failure"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/expect_output.cmake")
set(NESTURBATOR_CAMPAIGN_CONTROL_ARGS)
option(NESTURBATOR_LIBFUZZER "Build the libFuzzer targets" OFF)
if(NESTURBATOR_LIBFUZZER)
  list(APPEND NESTURBATOR_CAMPAIGN_CONTROL_ARGS
    -DFUZZ_CONTROLS_REQUIRED=ON
    -DROM_LOADER_FUZZER=$<TARGET_FILE:fuzz.rom_loader.libfuzzer>
    -DSTATE_LOADER_FUZZER=$<TARGET_FILE:fuzz.state.libfuzzer>
    -DEXECUTION_FUZZER=$<TARGET_FILE:fuzz.execution.libfuzzer>
    -DROM_LOADER_REPLAY=$<TARGET_FILE:fuzz.rom_loader>
    -DSTATE_LOADER_REPLAY=$<TARGET_FILE:fuzz.state_loader>)
endif()
add_test(NAME fuzz.execution.campaign_controls
  COMMAND ${CMAKE_COMMAND}
    "-DSOURCE_DIR=${PROJECT_SOURCE_DIR}"
    "-DBUILD_DIR=${PROJECT_BINARY_DIR}"
    "-DOUTPUT_DIR=${PROJECT_BINARY_DIR}/execution-campaign-controls"
    "-DBUDGET_FILE=${NESTURBATOR_EXECUTION_BUDGET_FILE}"
    -DREPLAY_BINARY=$<TARGET_FILE:fuzz.execution>
    ${NESTURBATOR_CAMPAIGN_CONTROL_ARGS}
    -DMODE=self-test
    -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/execution_campaign.cmake")
set_tests_properties(fuzz.execution.regress fuzz.execution.seeds
  fuzz.execution.controls fuzz.execution.oracle_failure PROPERTIES
  TIMEOUT ${NESTURBATOR_EXECUTION_REPLAY_TIMEOUT})
if(NESTURBATOR_LIBFUZZER)
  set_tests_properties(fuzz.execution.campaign_controls PROPERTIES TIMEOUT 150)
else()
  set_tests_properties(fuzz.execution.campaign_controls PROPERTIES TIMEOUT 15)
endif()

# Narrow replay counters are compiled only in the matched, sanitizer-instrumented
# measurement build. The normal CI and libFuzzer targets keep their existing path.
option(NESTURBATOR_FUZZ_METRICS "Enable replay counters for the bounded fuzz comparison" OFF)
if(NESTURBATOR_FUZZ_METRICS)
  target_compile_definitions(fuzz.rom_loader PRIVATE NESTURBATOR_FUZZ_METRICS=1)
  target_compile_definitions(fuzz.state_loader PRIVATE NESTURBATOR_FUZZ_METRICS=1)
  target_compile_definitions(fuzz.execution PRIVATE NESTURBATOR_FUZZ_METRICS=1)
endif()

# Linux nightly only: the same source/entry point and sanitized library, with
# Clang's libFuzzer main and runtime instrumentation.
if(NESTURBATOR_LIBFUZZER)
  if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "NESTURBATOR_LIBFUZZER requires Clang")
  endif()
  add_executable(fuzz.rom_loader.libfuzzer fuzz/rom_loader.c)
  target_link_libraries(fuzz.rom_loader.libfuzzer PRIVATE nesturbator::nesturbator)
  target_compile_options(fuzz.rom_loader.libfuzzer PRIVATE -fsanitize=fuzzer-no-link)
  target_link_options(fuzz.rom_loader.libfuzzer PRIVATE -fsanitize=fuzzer,address,undefined)
  nesturbator_warnings(fuzz.rom_loader.libfuzzer)
  add_executable(fuzz.state.libfuzzer fuzz/state_loader.c)
  target_link_libraries(fuzz.state.libfuzzer PRIVATE nesturbator::nesturbator)
  target_compile_options(fuzz.state.libfuzzer PRIVATE -fsanitize=fuzzer-no-link)
  target_link_options(fuzz.state.libfuzzer PRIVATE -fsanitize=fuzzer,address,undefined)
  nesturbator_warnings(fuzz.state.libfuzzer)
  add_executable(fuzz.execution.libfuzzer fuzz/execution.c)
  target_link_libraries(fuzz.execution.libfuzzer PRIVATE nesturbator::nesturbator)
  target_compile_definitions(fuzz.execution.libfuzzer PRIVATE ${NESTURBATOR_EXECUTION_CAPS})
  target_compile_options(fuzz.execution.libfuzzer PRIVATE -fsanitize=fuzzer-no-link)
  target_link_options(fuzz.execution.libfuzzer PRIVATE -fsanitize=fuzzer,address,undefined)
  nesturbator_warnings(fuzz.execution.libfuzzer)
endif()

add_test(NAME fuzz.registration
  COMMAND ${CMAKE_COMMAND}
    -DCTEST=${CMAKE_CTEST_COMMAND}
    -DBUILD_DIR=${PROJECT_BINARY_DIR}
    -DEXECUTION_BINARY=$<TARGET_FILE:fuzz.execution>
    -DEXECUTION_CORPUS_DIR=${CMAKE_CURRENT_SOURCE_DIR}/fuzz/corpus_execution
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/fuzz_registration.cmake)

# D-14: nesturbator_get_palette's copy rules, calibrated sRGB samples and
# palette invariants.
nesturbator_core_test(core.palette core/test_palette.c)
add_executable(accuracy.scoreboard
  accuracy/test_scoreboard.c
  ../runner/accuracycoin.c
  ../runner/audio_hash.c
  ../runner/clock.c
  ../runner/sha256.c)
target_include_directories(accuracy.scoreboard PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/runner)
target_link_libraries(accuracy.scoreboard PRIVATE nesturbator::nesturbator)
target_compile_definitions(accuracy.scoreboard PRIVATE
  NESTURBATOR_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
  NESTURBATOR_PROTECTED_EVIDENCE_DIR="${ACCURACYCOIN_PROTECTED_WORK}"
  $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
nesturbator_warnings(accuracy.scoreboard)
add_test(NAME accuracy.scoreboard COMMAND accuracy.scoreboard --scoreboard-syntax)

# Exercise the production verdict, with a complete passing baseline and fresh
# protected-page evidence, when one candidate key is removed.
set(ACCURACYCOIN_REGRESSION_WORK ${CMAKE_CURRENT_BINARY_DIR}/accuracycoin/regression-controls)
add_test(NAME accuracy.scoreboard.no_lost_pass
  COMMAND ${CMAKE_COMMAND}
    "-DSCOREBOARD_TEST=$<TARGET_FILE:accuracy.scoreboard>"
    "-DSOURCE_DIR=${PROJECT_SOURCE_DIR}"
    "-DEVIDENCE_ROOT=${ACCURACYCOIN_PROTECTED_WORK}"
    "-DWORK=${ACCURACYCOIN_REGRESSION_WORK}/no-lost-pass"
    -DMODE=no_lost_pass
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/verify_scoreboard_regression.cmake)
set_tests_properties(accuracy.scoreboard.no_lost_pass PROPERTIES
  FIXTURES_REQUIRED accuracycoin_protected_observations TIMEOUT 30)

add_test(NAME accuracycoin.regression_controls
  COMMAND ${CMAKE_COMMAND}
    "-DCHECKER=$<TARGET_FILE:accuracy.scoreboard>"
    "-DRUNNER=$<TARGET_FILE:nesturbator-run>"
    "-DFULL_ENABLED=${NESTURBATOR_ACCURACYCOIN_FULL}"
    "-DSOURCE_DIR=${PROJECT_SOURCE_DIR}"
    "-DEVIDENCE_ROOT=${ACCURACYCOIN_PROTECTED_WORK}"
    "-DWORK=${ACCURACYCOIN_REGRESSION_WORK}/matrix"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/accuracycoin_regression_controls.cmake)
set_tests_properties(accuracycoin.regression_controls PROPERTIES
  FIXTURES_REQUIRED accuracycoin_protected_observations
  TIMEOUT ${ACCURACYCOIN_PROTECTED_SETUP_TIMEOUT})

add_test(NAME accuracycoin.baseline_fetch
  COMMAND ${CMAKE_COMMAND}
    "-DPREPARE_SCRIPT=${CMAKE_CURRENT_SOURCE_DIR}/cmake/prepare_scoreboard_baseline.cmake"
    "-DSOURCE_DIR=${PROJECT_SOURCE_DIR}"
    "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/accuracycoin/baseline-fetch-controls"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/accuracycoin_baseline_fetch.cmake)
set_tests_properties(accuracycoin.baseline_fetch PROPERTIES TIMEOUT 60)

# Exercise the CI-only handoff failures with specific reasons.
add_test(NAME accuracy.scoreboard.ci_missing_baseline
  COMMAND ${CMAKE_COMMAND}
    "-DSCOREBOARD_TEST=$<TARGET_FILE:accuracy.scoreboard>"
    "-DSOURCE_DIR=${PROJECT_SOURCE_DIR}"
    "-DEVIDENCE_ROOT=${ACCURACYCOIN_PROTECTED_WORK}"
    "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/accuracycoin/ci-missing-baseline"
    -DMODE=ci_missing_baseline
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/verify_scoreboard_regression.cmake)
set_tests_properties(accuracy.scoreboard.ci_missing_baseline PROPERTIES TIMEOUT 30)
add_test(NAME accuracy.scoreboard.ci_empty_baseline_rejected
  COMMAND ${CMAKE_COMMAND}
    "-DSCOREBOARD_TEST=$<TARGET_FILE:accuracy.scoreboard>"
    "-DSOURCE_DIR=${PROJECT_SOURCE_DIR}"
    "-DEVIDENCE_ROOT=${ACCURACYCOIN_PROTECTED_WORK}"
    "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/accuracycoin/ci-empty-baseline"
    -DMODE=ci_empty_baseline
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/verify_scoreboard_regression.cmake)
set_tests_properties(accuracy.scoreboard.ci_empty_baseline_rejected PROPERTIES TIMEOUT 30)

# D-15: the shared host conversion indexes the palette by the low 9 bits and
# honours both pitches.
add_executable(host.convert host/test_convert.c)
target_link_libraries(host.convert PRIVATE nesturbator_host)
nesturbator_warnings(host.convert)
add_test(NAME host.convert COMMAND host.convert)

# D-11: the public header alone, as C17 and as C++, warnings as errors in
# every preset. Building is the test; running only shows it linked.
add_executable(header.c header/header_c.c)
target_link_libraries(header.c PRIVATE nesturbator::nesturbator)
nesturbator_warnings(header.c)
target_compile_options(header.c PRIVATE
  $<$<C_COMPILER_ID:GNU,Clang,AppleClang>:-pedantic -Werror -Wall -Wextra>
  $<$<C_COMPILER_ID:MSVC>:/W4 /WX>)
add_test(NAME header.c COMMAND header.c)

add_executable(header.cxx header/header_cxx.cpp)
target_link_libraries(header.cxx PRIVATE nesturbator::nesturbator)
set_target_properties(header.cxx PROPERTIES
  CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
target_compile_options(header.cxx PRIVATE
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-pedantic -Werror -Wall -Wextra -Wconversion -Wsign-conversion -Wshadow>
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>)
add_test(NAME header.cxx COMMAND header.cxx)

# The runner's SHA-256 against FIPS 180-4 known answers, compiled in directly.
add_executable(runner.sha256 runner/test_sha256.c ${PROJECT_SOURCE_DIR}/runner/sha256.c)
target_include_directories(runner.sha256 PRIVATE ${PROJECT_SOURCE_DIR}/runner)
target_link_libraries(runner.sha256 PRIVATE nesturbator::nesturbator)
nesturbator_warnings(runner.sha256)
add_test(NAME runner.sha256 COMMAND runner.sha256)

# --bench (15-CONTEXT D-07): stdout and every hash equal --frames N's; stderr has the D-06 lines.
# Only shapes are checked, never a measured number.
foreach(mode nohash format)
  add_test(NAME runner.bench.${mode}
    COMMAND ${CMAKE_COMMAND} "-DRUNNER=$<TARGET_FILE:nesturbator-run>"
      -DROM=${PROJECT_SOURCE_DIR}/tests/roms/nesteroids.nes -DMODE=${mode}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/runner_bench.cmake)
  set_tests_properties(runner.bench.${mode} PROPERTIES LABELS bench)
endforeach()
add_test(NAME runner.bench.format_state
  COMMAND ${CMAKE_COMMAND} "-DRUNNER=$<TARGET_FILE:nesturbator-run>"
    -DROM=${PROJECT_SOURCE_DIR}/tests/roms/nesteroids.nes -DMODE=format -DSTATE=1
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/runner_bench.cmake)
set_tests_properties(runner.bench.format_state PROPERTIES LABELS bench)

# --bench refusals (15-CONTEXT D-01, D-02, D-03): each exits 2 with its own message.
set(bench_rom ${PROJECT_SOURCE_DIR}/tests/roms/nesteroids.nes)
nesturbator_runner_test(NAME runner.usage.bench.accuracycoin-page
  ARGS --rom ${bench_rom} --bench 2 --accuracycoin-page 2
  EXIT 2 IGNORE_STDOUT EXPECT_ERR "--bench cannot be combined with --accuracycoin-page")
set_tests_properties(runner.usage.bench.accuracycoin-page PROPERTIES LABELS exit_status)
nesturbator_runner_test(NAME runner.usage.bench.raise-after
  ARGS --rom ${bench_rom} --bench 2 --raise-after 1
  EXIT 2 IGNORE_STDOUT EXPECT_ERR "--bench cannot be combined with --raise-after")
set_tests_properties(runner.usage.bench.raise-after PROPERTIES LABELS exit_status)
nesturbator_runner_test(NAME runner.usage.bench.dump-frame
  ARGS --rom ${bench_rom} --bench 2 --dump-frame 1:x.ppm
  EXIT 2 IGNORE_STDOUT EXPECT_ERR "--bench cannot be combined with --dump-frame")
set_tests_properties(runner.usage.bench.dump-frame PROPERTIES LABELS exit_status)
nesturbator_runner_test(NAME runner.usage.bench.state-at-frame
  ARGS --rom ${bench_rom} --bench 2 --state-at-frame 1:x.sta
  EXIT 2 IGNORE_STDOUT EXPECT_ERR "--bench cannot be combined with --state-at-frame")
set_tests_properties(runner.usage.bench.state-at-frame PROPERTIES LABELS exit_status)
nesturbator_runner_test(NAME runner.usage.bench.hash-state
  ARGS --rom ${bench_rom} --bench 2 --hash-state 1
  EXIT 2 IGNORE_STDOUT EXPECT_ERR "--bench cannot be combined with --hash-state")
set_tests_properties(runner.usage.bench.hash-state PROPERTIES LABELS exit_status)
nesturbator_runner_test(NAME runner.usage.bench.save-dir
  ARGS --rom ${bench_rom} --bench 2 --save-dir d
  EXIT 2 IGNORE_STDOUT EXPECT_ERR "--bench cannot be combined with --save-dir")
set_tests_properties(runner.usage.bench.save-dir PROPERTIES LABELS exit_status)
nesturbator_runner_test(NAME runner.usage.bench.save-interval
  ARGS --rom ${bench_rom} --bench 2 --save-interval 1
  EXIT 2 IGNORE_STDOUT EXPECT_ERR "--bench cannot be combined with --save-interval")
set_tests_properties(runner.usage.bench.save-interval PROPERTIES LABELS exit_status)
nesturbator_runner_test(NAME runner.usage.bench.load-state
  ARGS --rom ${bench_rom} --bench 2 --load-state x.sta
  EXIT 2 IGNORE_STDOUT EXPECT_ERR "--bench cannot be combined with --load-state")
set_tests_properties(runner.usage.bench.load-state PROPERTIES LABELS exit_status)
nesturbator_runner_test(NAME runner.usage.bench.frames
  ARGS --rom ${bench_rom} --bench 2 --frames 2
  EXIT 2 IGNORE_STDOUT EXPECT_ERR "--bench cannot be combined with --frames or --movie")
set_tests_properties(runner.usage.bench.frames PROPERTIES LABELS exit_status)
nesturbator_runner_test(NAME runner.usage.bench.movie
  ARGS --rom ${bench_rom} --bench 2 --movie m.mov
  EXIT 2 IGNORE_STDOUT EXPECT_ERR "--bench cannot be combined with --frames or --movie")
set_tests_properties(runner.usage.bench.movie PROPERTIES LABELS exit_status)
nesturbator_runner_test(NAME runner.usage.bench.zero
  ARGS --rom ${bench_rom} --bench 0
  EXIT 2 IGNORE_STDOUT EXPECT_ERR "--bench needs one whole number")
set_tests_properties(runner.usage.bench.zero PROPERTIES LABELS exit_status)
nesturbator_runner_test(NAME runner.usage.bench.over_cap
  ARGS --rom ${bench_rom} --bench 1000001
  EXIT 2 IGNORE_STDOUT EXPECT_ERR "--bench needs one whole number")
set_tests_properties(runner.usage.bench.over_cap PROPERTIES LABELS exit_status)
nesturbator_runner_test(NAME runner.usage.bench.twice
  ARGS --rom ${bench_rom} --bench 2 --bench 2
  EXIT 2 IGNORE_STDOUT EXPECT_ERR "--bench may be specified once")
set_tests_properties(runner.usage.bench.twice PROPERTIES LABELS exit_status)
nesturbator_runner_test(NAME runner.usage.bench.no_rom
  ARGS --bench 2
  EXIT 2 IGNORE_STDOUT EXPECT_ERR "--bench needs --rom")
set_tests_properties(runner.usage.bench.no_rom PROPERTIES LABELS exit_status)
nesturbator_runner_test(NAME runner.usage.bench.state_without_bench
  ARGS --rom ${bench_rom} --frames 2 --bench-state
  EXIT 2 IGNORE_STDOUT EXPECT_ERR "--bench-state requires --bench")
set_tests_properties(runner.usage.bench.state_without_bench PROPERTIES LABELS exit_status)
nesturbator_runner_test(NAME runner.usage.bench.state_twice
  ARGS --rom ${bench_rom} --bench 2 --bench-state --bench-state
  EXIT 2 IGNORE_STDOUT EXPECT_ERR "--bench-state may be specified once")
set_tests_properties(runner.usage.bench.state_twice PROPERTIES LABELS exit_status)

# --bench percentile, formatting and zero-sample report rules (15-CONTEXT D-05, D-06, D-03).
add_executable(runner.bench_unit runner/test_bench.c ${PROJECT_SOURCE_DIR}/runner/bench.c)
target_include_directories(runner.bench_unit PRIVATE ${PROJECT_SOURCE_DIR}/runner)
target_compile_definitions(runner.bench_unit PRIVATE $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
nesturbator_warnings(runner.bench_unit)
add_test(NAME runner.bench.percentile COMMAND runner.bench_unit)
set_tests_properties(runner.bench.percentile PROPERTIES LABELS bench)

add_executable(runner.audio_hash runner/test_audio_hash.c
  ${PROJECT_SOURCE_DIR}/runner/audio_hash.c ${PROJECT_SOURCE_DIR}/runner/sha256.c)
target_include_directories(runner.audio_hash PRIVATE ${PROJECT_SOURCE_DIR}/runner)
nesturbator_warnings(runner.audio_hash)
add_test(NAME runner.audio_hash COMMAND runner.audio_hash)

# GAME-03 / D-06: an empty owned movie is valid and replays zero frames.
add_executable(runner.movie.test runner/test_movie.c)
target_link_libraries(runner.movie.test PRIVATE nesturbator::nesturbator)
target_include_directories(runner.movie.test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/runner)
target_compile_definitions(runner.movie.test PRIVATE
  $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
nesturbator_warnings(runner.movie.test)
add_test(NAME runner.movie
  COMMAND runner.movie.test $<TARGET_FILE:nesturbator-run>
    ${CMAKE_CURRENT_BINARY_DIR}/fixture.nmovie ${CMAKE_CURRENT_BINARY_DIR}/movie-a.out
    ${CMAKE_CURRENT_BINARY_DIR}/movie-b.out ${CMAKE_CURRENT_BINARY_DIR}/jam.nes)

# D-02: fixed two-port movie scripts feed the pinned DABG hash inventory.
add_executable(runner.game_movie runner/game_movie.c)
target_compile_definitions(runner.game_movie PRIVATE
  $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
nesturbator_warnings(runner.game_movie)

# PLAY-01 / D-05: the build writes the four nes-runner movies (no movie bytes
# are committed); runner.boot_to_play reads them through the nesrunner_movies
# fixture.
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/nesrunner-movies)
foreach(scenario pos none attract startonly)
  add_test(NAME runner.nesrunner_movie.${scenario}
    COMMAND runner.game_movie
      ${CMAKE_CURRENT_BINARY_DIR}/nesrunner-movies/nesrunner-${scenario}.nmovie
      nesrunner-${scenario})
  set_tests_properties(runner.nesrunner_movie.${scenario} PROPERTIES
    FIXTURES_SETUP nesrunner_movies)
endforeach()

# PLAY-01 / D-08: one program replays the movies through the library, reads
# nes-runner's RAM and judges the positive run and every control.
add_executable(runner.boot_to_play runner/test_boot_to_play.c
  ${PROJECT_SOURCE_DIR}/runner/sha256.c ${PROJECT_SOURCE_DIR}/runner/audio_hash.c
  ${PROJECT_SOURCE_DIR}/runner/movie.c)
target_link_libraries(runner.boot_to_play PRIVATE nesturbator::nesturbator)
target_include_directories(runner.boot_to_play PRIVATE ${PROJECT_SOURCE_DIR}/runner
  ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(runner.boot_to_play PRIVATE
  $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
nesturbator_warnings(runner.boot_to_play)
# About 3,000 emulated frames. ASan measured about 55 s locally, so TIMEOUT 300
# (10-RESEARCH Pitfall 8); COST 45 starts it beside runner.write_hashes.
# NESTURBATOR_SHORT_RUNS (preset asan) leaves it out: its predicates need the whole 600-frame
# movie, and runner.dump.nesrunner.play and libretro.parity.nesrunner.play run the same movie
# under ASan.
if(NOT NESTURBATOR_SHORT_RUNS)
add_test(NAME runner.boot_to_play
  COMMAND runner.boot_to_play ${PROJECT_SOURCE_DIR}/tests/roms/runner.nes
    ${CMAKE_CURRENT_BINARY_DIR}/nesrunner-movies)
set_tests_properties(runner.boot_to_play PROPERTIES
  FIXTURES_REQUIRED nesrunner_movies TIMEOUT 300 COST 45)
endif()

# version.txt equals the header's version macros.
add_test(NAME version.consistency
  COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version_consistency.cmake)

# D-12: the runner prints exactly "nesturbator-run <version.txt>", nothing on
# stderr, exit 0; --version before a bad option still succeeds.
add_test(NAME runner.version
  COMMAND ${CMAKE_COMMAND} "-DRUNNER=$<TARGET_FILE:nesturbator-run>"
    -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/runner_version.cmake)

# Every script in tests/cmake sets its policy version, so CMake reads it the
# same way on every runner.
add_test(NAME cmake.script_policy
  COMMAND ${CMAKE_COMMAND} -DDIR=${CMAKE_CURRENT_SOURCE_DIR}/cmake
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/script_policy.cmake)

# D-07: the README status table equals enum nesturbator_status, and the README keeps its Save
# states section and the rewind sentence (STATE-05).
add_test(NAME state.status_table
  COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/status_table.cmake)

# RUN-03: the exit table is the same in runner/exit_status.h, --help and the README.
foreach(mode table help)
  add_test(NAME runner.exit.${mode}
    COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
      "-DRUNNER=$<TARGET_FILE:nesturbator-run>" -DMODE=${mode}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/runner_exit_table.cmake)
  set_tests_properties(runner.exit.${mode} PROPERTIES LABELS exit_status)
endforeach()

# D-22: the options printed by --help and the README runner bullets name the same set.
add_test(NAME runner.options.readme
  COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
    "-DRUNNER=$<TARGET_FILE:nesturbator-run>"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/runner_options.cmake)

# D-01 and D-02: vector conversion and fetch stay implemented by the owned C
# parser; release configuration scripts are outside this scoped policy.
add_test(NAME vectors.source_policy
  COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/vector_source_policy.cmake)
add_test(NAME vectors.source_policy.self_test
  COMMAND ${CMAKE_COMMAND} -DSELFTEST=ON -DWORK=${CMAKE_CURRENT_BINARY_DIR}/policy
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/vector_source_policy.cmake)

# The public API guard follows the current Phase 4 contract; later intentional
# public API changes must deliberately update or retire this temporary policy.
add_test(NAME vectors.api_policy
  COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/vector_api_policy.cmake)
add_test(NAME vectors.api_policy.self_test
  COMMAND ${CMAKE_COMMAND} -DSELFTEST=ON -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/vector_api_policy.cmake)

# Tests never skip: a test that cannot run on a platform or without a host app
# is not registered there (wrap it in if()), and a check that needs a host app
# lives in a required CI job such as retroarch-e2e. This policy fails when any
# registered test could report itself skipped or is disabled.
add_test(NAME policy.no-skip
  COMMAND ${CMAKE_COMMAND}
    -DCTEST=${CMAKE_CTEST_COMMAND}
    -DBUILD_DIR=${PROJECT_BINARY_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/skip_policy.cmake)
add_test(NAME policy.no-skip.self_test
  COMMAND ${CMAKE_COMMAND} -DSELFTEST=ON
    -DCTEST=${CMAKE_CTEST_COMMAND}
    -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/no-skip
    "-DGENERATOR=${CMAKE_GENERATOR}"
    -DMAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/skip_policy.cmake)

# Exact opcode inventories come from CTest's machine-readable metadata. The
# hosted policy reads the shared full inventory; local runs configure the full
# preset as a fallback. Neither path runs its network-backed fixture.
# NESTURBATOR_SHORT_RUNS (preset asan) leaves both out: they check CTest metadata, sanitizers
# add nothing, and every ci leg runs them.
if(NOT NESTURBATOR_SHORT_RUNS)
set(NESTURBATOR_VECTORS_INVENTORY "" CACHE FILEPATH
  "Fresh full-vector CTest inventory supplied by this CI invocation")
add_test(NAME vectors.registration_policy
  COMMAND ${CMAKE_COMMAND}
    -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
    -DCI_BUILD=${PROJECT_BINARY_DIR}
    -DFULL_BUILD=${PROJECT_BINARY_DIR}/../vectors-full
    -DINVENTORY_FILE=${NESTURBATOR_VECTORS_INVENTORY}
    -DCTEST=${CMAKE_CTEST_COMMAND}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/vector_registration_policy.cmake)
set_tests_properties(vectors.registration_policy PROPERTIES COST 1000)
add_test(NAME vectors.registration_policy.self_test
  COMMAND ${CMAKE_COMMAND} -DSELFTEST=ON
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/vector_registration_policy.cmake)
set_tests_properties(vectors.registration_policy.self_test PROPERTIES COST 200)
endif()

# Run-bound full-vector evidence contains the CTest inventory, including tests
# outside the vector label. Only the 258 full-vector entries define this lane.
add_test(NAME vectors.result_policy.self_test
  COMMAND ${CMAKE_COMMAND} -DSELFTEST=ON
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/vector_result_policy.cmake)

# The post-merge evidence collector is POSIX and uses jq; exercise its
# read-only outcome classification in CI wherever those tools are available.
if(UNIX)
  find_program(NESTURBATOR_JQ_EXECUTABLE NAMES jq)
  if(NESTURBATOR_JQ_EXECUTABLE)
    add_test(NAME phase2.outcomes.self_test
      COMMAND sh "${PROJECT_SOURCE_DIR}/scripts/phase2_outcomes.sh" --self-test)
  endif()
endif()

# release-please rewrites only the first version on each marked line, so no
# marked line in its extra-files may hold two. The self-test runs the scan on
# a fixture with two versions on line 3 and passes only on that exact error.
add_test(NAME release.one_version_per_line
  COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/release_markers.cmake)
add_test(NAME release.one_version_per_line.self_test
  COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
    -DFILES=tests/release/two_versions.md
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/release_markers.cmake)
set_tests_properties(release.one_version_per_line.self_test PROPERTIES
  PASS_REGULAR_EXPRESSION "tests/release/two_versions\\.md:3: 2 versions")

# Numeric-only release updates preserve the three public-header markers and
# their narrow formatting guard, even when version components gain digits.
add_test(NAME release.version_format
  COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
    -DWORK=${CMAKE_CURRENT_BINARY_DIR}/release_version_format
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/release_version_format.cmake)

# A one-time release-as pin in release-please-config.json must not stay
# behind, or every later release reuses its version.
add_test(NAME release.no_release_as
  COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/release_config.cmake)

# Release credentials are available only to a workflow definition selected
# by a push to protected main, and the App token has exactly two consumers.
add_test(NAME release.credentials_policy
  COMMAND ${CMAKE_COMMAND} -DROOT=${PROJECT_SOURCE_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/release_credentials_policy.cmake)
add_test(NAME release.credentials_policy.selftest
  COMMAND ${CMAKE_COMMAND} -DROOT=${PROJECT_SOURCE_DIR} -DSELFTEST=ON
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/release_credentials_policy.cmake)

# The pinned release-please defaults keep non-breaking docs/chore commits from
# opening release PRs. Mutations exercise the policy checker itself.
add_test(NAME release.nonbehavioral_policy
  COMMAND ${CMAKE_COMMAND} -DROOT=${PROJECT_SOURCE_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/release_policy.cmake)
add_test(NAME release.nonbehavioral_policy.selftest
  COMMAND ${CMAKE_COMMAND} -DROOT=${PROJECT_SOURCE_DIR} -DSELFTEST=ON
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/release_policy.cmake)

# libretro.h is RetroArch v1.22.2's copy (commit
# 69a4f0ea1e8aaf442ae4858f2e7f2b31a1776576), byte for byte. .gitattributes
# keeps it LF on Windows checkouts too.
add_test(NAME libretro.vendored
  COMMAND ${CMAKE_COMMAND}
    -DFILE=${PROJECT_SOURCE_DIR}/libretro/libretro.h
    -DSHA256=bd3398d29c3763d18617087020ff56b5450a48a63123403a4b674f8e79947acb
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/vendored_sha256.cmake)

# FRAME-04: a host program loads the built module at run time, calls it as
# RetroArch does, and compares its frame with the runner's image (fixture
# testframe_ppm) and with four hard-coded pixels (D-05). The module is a
# run-time input, so the test program depends on it to build it first.
add_executable(libretro.host libretro/libretro_host.c ${PROJECT_SOURCE_DIR}/runner/movie.c)
target_include_directories(libretro.host PRIVATE ${PROJECT_SOURCE_DIR}/libretro
  ${PROJECT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/runner)
target_link_libraries(libretro.host PRIVATE nesturbator ${CMAKE_DL_LIBS})
target_compile_definitions(libretro.host PRIVATE
  $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
nesturbator_warnings(libretro.host)
add_dependencies(libretro.host nesturbator_libretro nesturbator-run)
add_test(NAME libretro.host
  COMMAND libretro.host $<TARGET_FILE:nesturbator_libretro>
    ${NESTURBATOR_TESTFRAME_PPM} ${CMAKE_CURRENT_BINARY_DIR}/nrom-tracer.ppm
    $<TARGET_FILE:nesturbator-run> ${CMAKE_CURRENT_BINARY_DIR}/nrom-tracer.nes)
set_tests_properties(libretro.host PROPERTIES FIXTURES_REQUIRED testframe_ppm)

# Pin the full-path selector boundary before authenticating fixture bytes.
add_test(NAME libretro.fixture_roles
  COMMAND libretro.host --fixture-role-selftest)
set_tests_properties(libretro.fixture_roles PROPERTIES TIMEOUT 30)

# Check copied-byte identity controls, including failures that must precede host launch.
add_test(NAME libretro.fixture_identity
  COMMAND ${CMAKE_COMMAND}
    -DSELFTEST=ON
    "-DHOST=$<TARGET_FILE:libretro.host>"
    "-DCORE=$<TARGET_FILE:nesturbator_libretro>"
    "-DSOURCE_DIR=${PROJECT_SOURCE_DIR}"
    "-DBUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}/fixture identity"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/fixture_identity.cmake)
set_tests_properties(libretro.fixture_identity PROPERTIES TIMEOUT 30)

# LR-01: --serialize drives retro_serialize_size, retro_serialize and retro_unserialize of
# the built module and checks each against the library state API. The wrapper copies and hashes
# the actual special inputs before launching the host.
set(LIBRETRO_SERIALIZE_ROMS
  ${PROJECT_SOURCE_DIR}/tests/roms/nesteroids.nes
  ${PROJECT_SOURCE_DIR}/tests/roms/dabg.nes
  ${PROJECT_SOURCE_DIR}/tests/roms/hm/M1_P128K_C128K_S8K.nes
  ${PROJECT_SOURCE_DIR}/tests/roms/hm/M2_P128K_CR8K_V.nes
  ${PROJECT_SOURCE_DIR}/tests/roms/hm/M3_P32K_C32K_H.nes)
add_test(NAME libretro.serialize
  COMMAND ${CMAKE_COMMAND}
    "-DHOST=$<TARGET_FILE:libretro.host>"
    "-DCORE=$<TARGET_FILE:nesturbator_libretro>"
    "-DMODE=serialize"
    "-DSOURCE_DIR=${PROJECT_SOURCE_DIR}"
    "-DBUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}/fixture identity"
    "-DROMS=${LIBRETRO_SERIALIZE_ROMS}"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/fixture_identity.cmake)
# The asan job on a hosted runner ran past 120 s while other tests shared the CPU.
set_tests_properties(libretro.serialize PROPERTIES TIMEOUT 600)

# LR-02: --runahead drives the module as a run-ahead host at one to three frames over two input
# scripts, against an uninterrupted run, and four broken hosts must fail the same comparison
# (D-10 to D-16). NESTURBATOR_SHORT_RUNS (preset asan) runs 30 ticks instead of 180.
set(RUNAHEAD_MODE)
if(NESTURBATOR_SHORT_RUNS)
  set(RUNAHEAD_MODE --short)
endif()
add_test(NAME libretro.runahead
  COMMAND ${CMAKE_COMMAND}
    "-DHOST=$<TARGET_FILE:libretro.host>"
    "-DCORE=$<TARGET_FILE:nesturbator_libretro>"
    "-DMODE=runahead"
    "-DSHORT=${NESTURBATOR_SHORT_RUNS}"
    -DSTRESS=ON
    "-DSOURCE_DIR=${PROJECT_SOURCE_DIR}"
    "-DBUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}/fixture identity"
    "-DROMS=${PROJECT_SOURCE_DIR}/tests/roms/nesteroids.nes;${PROJECT_SOURCE_DIR}/tests/roms/dabg.nes;${PROJECT_SOURCE_DIR}/tests/roms/hm/M1_P128K_C128K_S8K.nes;${PROJECT_SOURCE_DIR}/tests/roms/hm/M2_P128K_CR8K_V.nes;${PROJECT_SOURCE_DIR}/tests/roms/hm/M3_P32K_C32K_H.nes;${PROJECT_SOURCE_DIR}/tests/roms/hm/M4_P128K_CR8K.nes;${PROJECT_SOURCE_DIR}/tests/roms/hm/M7_P128K_CR8K.nes"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/fixture_identity.cmake)
# TIMEOUT 600: under asan on a hosted runner this test runs several times slower than locally
# (libretro.serialize needed the same ceiling). At 60 ticks a hosted asan run took 292 s and a slow
# runner passed 600 s, so asan runs 30 ticks: 52 s locally, about half the hosted time.
set_tests_properties(libretro.runahead PROPERTIES TIMEOUT 600 COST 30)

# D-18 / D-14 / D-09: --rom-parity runs a whole ROM through the libretro module
# and compares its last frame with the image the runner dumped (fixture
# rundump_<key>, so the runner is not run twice). Scope is every ROM in the hash
# inventory that needs no save file: the games at frame 180, nes-runner's boot
# at 180 and its played nesrunner-pos movie at 600, and each non-save Holy
# Mapperel ROM at its N (registered in the Holy Mapperel loop above). Each key
# is its own CTest so a failure names the ROM. The empty-list stops mean zero
# parity tests cannot pass vacuously (D-17).
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/parity)
set(NESTURBATOR_PARITY_DIR ${CMAKE_CURRENT_BINARY_DIR}/parity)
set(parity_keys)
foreach(game_entry IN LISTS NESTURBATOR_HASH_GAMES)
  string(REPLACE ":" ";" game_parts "${game_entry}")
  list(GET game_parts 0 game_key)
  list(GET game_parts 1 game_file)
  set(game_rom ${PROJECT_SOURCE_DIR}/tests/roms/${game_file})
  if(game_key STREQUAL "nesrunner")
    set(parity_key nesrunner.boot)
  else()
    set(parity_key ${game_key})
  endif()
  nesturbator_runner_test(NAME runner.dump.${parity_key}
    ARGS --rom ${game_rom} --frames 180
      --dump-frame "180:${NESTURBATOR_PARITY_DIR}/${parity_key}.ppm"
    EXIT 0 IGNORE_STDOUT)
  set_tests_properties(runner.dump.${parity_key} PROPERTIES
    FIXTURES_SETUP rundump_${parity_key})
  add_test(NAME libretro.parity.${parity_key}
    COMMAND libretro.host --rom-parity $<TARGET_FILE:nesturbator_libretro>
      ${game_rom} 180 ${NESTURBATOR_PARITY_DIR}/${parity_key}.ppm
      --testcard ${NESTURBATOR_TESTFRAME_PPM})
  set_tests_properties(libretro.parity.${parity_key} PROPERTIES
    FIXTURES_REQUIRED "rundump_${parity_key};testframe_ppm" TIMEOUT 120)
  list(APPEND parity_keys ${parity_key})
  if(game_key STREQUAL "nesrunner")
    # PLAY-01: the played movie, 600 frames, held by the runner and the module.
    set(play_movie ${CMAKE_CURRENT_BINARY_DIR}/nesrunner-movies/nesrunner-pos.nmovie)
    nesturbator_runner_test(NAME runner.dump.nesrunner.play
      ARGS --rom ${game_rom} --movie ${play_movie}
        --dump-frame "600:${NESTURBATOR_PARITY_DIR}/nesrunner.play.ppm"
      EXIT 0 IGNORE_STDOUT)
    set_tests_properties(runner.dump.nesrunner.play PROPERTIES
      FIXTURES_SETUP rundump_nesrunner.play FIXTURES_REQUIRED nesrunner_movies)
    add_test(NAME libretro.parity.nesrunner.play
      COMMAND libretro.host --rom-parity $<TARGET_FILE:nesturbator_libretro>
        ${game_rom} 600 ${NESTURBATOR_PARITY_DIR}/nesrunner.play.ppm
        --movie ${play_movie} --testcard ${NESTURBATOR_TESTFRAME_PPM})
    set_tests_properties(libretro.parity.nesrunner.play PROPERTIES
      FIXTURES_REQUIRED "rundump_nesrunner.play;nesrunner_movies;testframe_ppm" TIMEOUT 120)
    list(APPEND parity_keys nesrunner.play)
  endif()
endforeach()
if(NOT parity_keys)
  message(FATAL_ERROR "no libretro parity tests: NESTURBATOR_HASH_GAMES is empty")
endif()

# D-14: palgen regenerates src/palette_ntsc.c byte for byte, ignoring line
# endings.
add_test(NAME palette.regen
  COMMAND ${CMAKE_COMMAND}
    -DPALGEN=$<TARGET_FILE:palgen>
    -DOUT=${CMAKE_CURRENT_BINARY_DIR}/palette_ntsc.c
    -DREF=${PROJECT_SOURCE_DIR}/src/palette_ntsc.c
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/palette_regen.cmake)

# D-08, D-09, D-11: the vector program runs N65V tests through the shipped
# CPU object against the test bus in tests/cpu/vector_bus.c. It does not
# link the library, which defines the same two bus functions.
add_executable(cpu.vectors
  cpu/test_vectors.c
  cpu/vector_bus.c
  vectors/n65v.c
  $<TARGET_OBJECTS:nesturbator_cpu>)
target_include_directories(cpu.vectors PRIVATE
  ${PROJECT_SOURCE_DIR}/src
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/tests/vectors
  ${PROJECT_SOURCE_DIR}/tests/cpu)
target_compile_definitions(cpu.vectors PRIVATE $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
nesturbator_warnings(cpu.vectors)

# D-13, D-17: what the sample cannot show, on the same CPU object and test
# bus: the JSR whose push overwrites its ADH operand (65x02 issue 18), the
# jammed loop, and a JAM at 0xFFFF wrapping PC to 0x0000. It does not link
# the library.
add_executable(cpu.unit cpu/test_cpu_unit.c cpu/vector_bus.c $<TARGET_OBJECTS:nesturbator_cpu>)
target_include_directories(cpu.unit PRIVATE
  ${PROJECT_SOURCE_DIR}/src
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/tests/vectors
  ${PROJECT_SOURCE_DIR}/tests/cpu)
nesturbator_warnings(cpu.unit)
add_test(NAME cpu.unit COMMAND cpu.unit)

# D-14: nesturbator_create sets the RP2A03G machine profile, read through
# the internal header.
add_executable(core.profile core/test_profile.c)
target_link_libraries(core.profile PRIVATE nesturbator::nesturbator)
target_include_directories(core.profile PRIVATE ${PROJECT_SOURCE_DIR}/src)
nesturbator_warnings(core.profile)
add_test(NAME core.profile COMMAND core.profile)

# D-12: the library bus alone, compiled in directly: 24 ticks per call, RAM
# mirrored over 0x0000-0x1FFF, the open-bus latch for unmapped reads.
add_executable(bus.unit cpu/test_bus.c
  ${PROJECT_SOURCE_DIR}/src/bus.c
  ${PROJECT_SOURCE_DIR}/src/instance.c
  ${PROJECT_SOURCE_DIR}/src/cpu.c
  ${PROJECT_SOURCE_DIR}/src/apu.c
  ${PROJECT_SOURCE_DIR}/src/synth.c
  ${PROJECT_SOURCE_DIR}/src/cartridge.c
  ${PROJECT_SOURCE_DIR}/src/crc32.c
  ${PROJECT_SOURCE_DIR}/src/mapper_nrom.c
  ${PROJECT_SOURCE_DIR}/src/mapper_mmc1.c
  ${PROJECT_SOURCE_DIR}/src/mapper_uxrom.c
  ${PROJECT_SOURCE_DIR}/src/mapper_cnrom.c
  ${PROJECT_SOURCE_DIR}/src/mapper_axrom.c
  ${PROJECT_SOURCE_DIR}/src/mapper_mmc3.c
  ${PROJECT_SOURCE_DIR}/src/ppu.c)
target_include_directories(bus.unit PRIVATE
  ${PROJECT_SOURCE_DIR}/src
  ${PROJECT_SOURCE_DIR}/include)
nesturbator_warnings(bus.unit)
add_test(NAME bus.unit COMMAND bus.unit)

# CPU-01, D-01 and D-03: vecconv converts the first three upstream a9 tests
# (tests/vectors/fixtures) to N65V and reads them back through the shared
# reader; cpu.vectors then runs them through the CPU. The script's exit
# status decides the test, so it has no pass or fail regex.
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/vectors)
add_test(NAME vecconv.a9
  COMMAND ${CMAKE_COMMAND}
    -DVECCONV=$<TARGET_FILE:vecconv>
    -DVECTORS=$<TARGET_FILE:cpu.vectors>
    -DJSON=${CMAKE_CURRENT_SOURCE_DIR}/vectors/fixtures/a9-first3.json
    -DOPCODE=a9
    -DFIRST=3
    -DOUT=${CMAKE_CURRENT_BINARY_DIR}/vectors/a9-first3.n65v
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/vectors_fixture.cmake)

# CPU-01, D-04 and D-05: the committed sample reads through the shared reader
# as 256 chunks of 100 tests, opcodes 00 to ff, and ends after the last one.
add_executable(vectors.n65v vectors/test_n65v.c vectors/n65v.c)
target_compile_definitions(vectors.n65v PRIVATE $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
nesturbator_warnings(vectors.n65v)
add_test(NAME vectors.n65v
  COMMAND vectors.n65v ${PROJECT_SOURCE_DIR}/tests/vectors/65x02-sample.n65v)
# D-05, D-06: with no argument the same program feeds the reader crafted
# buffers (every truncation, bad magic, version 2, kind 2, opcode mismatch,
# short count, a repeated index, a trailing byte), each of which must fail
# with the byte offset at fault.
add_test(NAME vectors.n65v.crafted COMMAND vectors.n65v)

# CPU-01: cpu.vectors.<xx> runs opcode xx's 100 tests from the committed
# sample through the CPU and compares registers, RAM and every bus cycle.
# expect_output.cmake checks the exit status and the exact line, so a reader
# error after a clean count still fails; no test sets a regex property.
function(nesturbator_vector_tests)
  foreach(op IN LISTS ARGN)
    set(cmd "$<TARGET_FILE:cpu.vectors>"
      "${PROJECT_SOURCE_DIR}/tests/vectors/65x02-sample.n65v" ${op} 256 100)
    add_test(NAME cpu.vectors.${op}
      COMMAND ${CMAKE_COMMAND}
        "-DCMD=${cmd}"
        "-DEXPECT=65x02/${op}: 0 of 100 vectors failed"
        "-DEXPECT_EXIT=0"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/expect_output.cmake")
    set_tests_properties(cpu.vectors.${op} PROPERTIES LABELS vectors)
  endforeach()
endfunction()
# All 256 opcodes, 00 to ff, with no exclusion list (D-18): a chip
# difference goes through the machine profile, never a skipped vector.
set(NESTURBATOR_ALL_OPCODES)
foreach(h 0 1 2 3 4 5 6 7 8 9 a b c d e f)
  foreach(l 0 1 2 3 4 5 6 7 8 9 a b c d e f)
    list(APPEND NESTURBATOR_ALL_OPCODES ${h}${l})
  endforeach()
endforeach()
nesturbator_vector_tests(${NESTURBATOR_ALL_OPCODES})

# D-08: reject decimal vector limits that exceed their uint32 bounds instead
# of allowing arithmetic overflow to wrap them back into the accepted range.
set(vector_chunks_overflow_cmd "$<TARGET_FILE:cpu.vectors>"
  "${PROJECT_SOURCE_DIR}/tests/vectors/65x02-sample.n65v" a9 4294967552 100)
add_test(NAME cpu.vectors.usage.overflow.chunks
  COMMAND ${CMAKE_COMMAND}
    "-DCMD=${vector_chunks_overflow_cmd}"
    -DEXPECT_EXIT=2 -DIGNORE_STDOUT=ON
    -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/expect_output.cmake")
set_tests_properties(cpu.vectors.usage.overflow.chunks PROPERTIES LABELS vectors)

set(vector_per_chunk_overflow_cmd "$<TARGET_FILE:cpu.vectors>"
  "${PROJECT_SOURCE_DIR}/tests/vectors/65x02-sample.n65v" a9 256 4294977296)
add_test(NAME cpu.vectors.usage.overflow.per_chunk
  COMMAND ${CMAKE_COMMAND}
    "-DCMD=${vector_per_chunk_overflow_cmd}"
    -DEXPECT_EXIT=2 -DIGNORE_STDOUT=ON
    -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/expect_output.cmake")
set_tests_properties(cpu.vectors.usage.overflow.per_chunk PROPERTIES LABELS vectors)

# D-02, D-06: vecconv converts the compact layout of 02.json and runs its 3
# JAM tests through the CPU. The script's exit status decides the test.
add_test(NAME vecconv.02
  COMMAND ${CMAKE_COMMAND}
    -DVECCONV=$<TARGET_FILE:vecconv>
    -DVECTORS=$<TARGET_FILE:cpu.vectors>
    -DJSON=${CMAKE_CURRENT_SOURCE_DIR}/vectors/fixtures/02-first3.json
    -DOPCODE=02 -DFIRST=3
    -DOUT=${CMAKE_CURRENT_BINARY_DIR}/vectors/02-first3.n65v
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/vectors_fixture.cmake)
# A prefix that ends inside the 4th test of a9.json converts with --first 3;
# it is converted only (RUN=OFF).
add_test(NAME vecconv.a9_tail
  COMMAND ${CMAKE_COMMAND}
    -DVECCONV=$<TARGET_FILE:vecconv>
    -DJSON=${CMAKE_CURRENT_SOURCE_DIR}/vectors/fixtures/a9-tail.json
    -DOPCODE=a9 -DFIRST=3 -DRUN=OFF
    -DOUT=${CMAKE_CURRENT_BINARY_DIR}/vectors/a9-tail.n65v
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/vectors_fixture.cmake)
# A failed vector's write to an address it does not list must not reach the
# next vector: cpu.vectors zeroes all of RAM after a failure.
add_test(NAME cpu.vectors.stray-write
  COMMAND ${CMAKE_COMMAND}
    -DVECCONV=$<TARGET_FILE:vecconv>
    -DVECTORS=$<TARGET_FILE:cpu.vectors>
    -DWORK=${CMAKE_CURRENT_BINARY_DIR}/vectors/stray-write
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/vectors_stray_write.cmake)

# D-02: each malformed input makes vecconv exit 1 with a byte offset.
# vecconv_negative.cmake builds the case from a fixture and passes only on
# that result, so no test here sets a regex property or WILL_FAIL.
function(nesturbator_vecconv_reject name fixture case first)
  add_test(NAME ${name}
    COMMAND ${CMAKE_COMMAND}
      -DVECCONV=$<TARGET_FILE:vecconv>
      -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/vectors/fixtures/${fixture}
      -DCASE=${case} -DFIRST=${first}
      -DWORK=${CMAKE_CURRENT_BINARY_DIR}/vectors/${name}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/vecconv_negative.cmake)
endfunction()
foreach(case float sign exponent leading_zero addr_range byte_range kind unknown_key missing_key
    count empty_file empty_array closed_short truncated)
  nesturbator_vecconv_reject(vecconv.reject.${case} a9-first3.json ${case} 3)
endforeach()
# The a9-tail prefix holds 3 whole tests; asking for 4 must fail.
nesturbator_vecconv_reject(vecconv.a9_tail.cut a9-tail.json as_is 4)

# Every file in tests/roms/manifest.txt has five fields and the SHA-256 its
# line gives. The self-test lists a file with a wrong hash and passes only if
# the check reports it.
add_test(NAME manifest.sha256
  COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/manifest_sha256.cmake)
add_test(NAME manifest.sha256.selftest
  COMMAND ${CMAKE_COMMAND} -DSELFTEST=ON -DWORK=${CMAKE_CURRENT_BINARY_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/manifest_sha256.cmake)

# D-17: every committed ROM is hashed or exempt (hash_keys.cmake). The
# self-test builds scratch trees with an unlisted ROM, an empty hm directory
# and a missing game ROM and passes only if each is rejected.
add_test(NAME policy.rom_hash_coverage
  COMMAND ${CMAKE_COMMAND} -DROOT=${PROJECT_SOURCE_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/rom_coverage.cmake)
add_test(NAME policy.rom_hash_coverage.self_test
  COMMAND ${CMAKE_COMMAND} -DSELFTEST=ON -DWORK=${CMAKE_CURRENT_BINARY_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/rom_coverage.cmake)
set_tests_properties(policy.rom_hash_coverage policy.rom_hash_coverage.self_test
  PROPERTIES TIMEOUT 30)

# D-20: tests/vectors/pins.txt, offline: the pin line, 256 lines in path
# order, the upstream tree's total size, and the sample's commit. The
# self-test swaps two lines and passes only on the 'not sorted' report.
add_test(NAME vectors.pins
  COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/pins_check.cmake)
add_test(NAME vectors.pins.selftest
  COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
    -DSELFTEST=ON -DWORK=${CMAKE_CURRENT_BINARY_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/pins_check.cmake)
# The fetch deletes only the directory it created, and refuses a DIR that
# reaches the source tree through a symlink or a different letter case.
# Offline: the guards stop the script before any git call.
add_test(NAME vectors.fetch.guard
  COMMAND ${CMAKE_COMMAND}
    -DSCRIPT=${CMAKE_CURRENT_SOURCE_DIR}/cmake/fetch_vectors.cmake
    -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
    -DWORK=${CMAKE_CURRENT_BINARY_DIR}/fetch-guard
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/fetch_guard.cmake)
add_test(NAME vectors.fetch.transport
  COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
    -DWORK=${CMAKE_CURRENT_BINARY_DIR}/vectors-transport
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/vectors_transport.cmake)
set_tests_properties(vectors.fetch.transport PROPERTIES TIMEOUT 60)

# The blargg MMC3 fetch has the same guards, with its own directory and marker.
add_test(NAME mmc3.fetch.guard
  COMMAND ${CMAKE_COMMAND}
    -DSCRIPT=${CMAKE_CURRENT_SOURCE_DIR}/cmake/fetch_blargg_mmc3.cmake
    -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
    -DWORK=${CMAKE_CURRENT_BINARY_DIR}/mmc3-fetch-guard
    -DSUBDIR=nes-test-roms
    -DMARKER=.nesturbator-mmc3
    -DPINS=${CMAKE_CURRENT_SOURCE_DIR}/mmc3/pins.txt
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/fetch_guard.cmake)

# 09 D-11: the blargg MMC3 oracle, a test-only helper like holymapperel-decode.
# It is always built so mmc3.oracle.unit can share its header; the lane below
# runs it on fetched ROMs.
add_executable(mmc3-oracle mmc3/oracle.c)
target_link_libraries(mmc3-oracle PRIVATE nesturbator::nesturbator)
target_include_directories(mmc3-oracle PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_compile_definitions(mmc3-oracle PRIVATE $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
nesturbator_warnings(mmc3-oracle)

# 09 D-11: the oracle's judgement on synthetic memory, and the pins and result table, offline.
add_executable(mmc3.oracle.unit mmc3/test_oracle.c)
target_include_directories(mmc3.oracle.unit PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/mmc3)
nesturbator_warnings(mmc3.oracle.unit)
add_test(NAME mmc3.oracle.unit COMMAND mmc3.oracle.unit)
add_test(NAME mmc3.pins
  COMMAND ${CMAKE_COMMAND} -DMODE=pins -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/mmc3_oracle_inventory.cmake)
add_test(NAME mmc3.oracle.inventory
  COMMAND ${CMAKE_COMMAND} -DMODE=inventory -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/mmc3_oracle_inventory.cmake)

# 09 D-10: blargg's mmc3_test_2 and mmc3_irq_tests at the pin in
# tests/mmc3/pins.txt, one test per row of tests/mmc3/oracle.txt. They need the
# network, so they are registered only when NESTURBATOR_MMC3_ORACLE is ON (the
# mmc3-oracle preset); ci never fetches. Offline the fetch fails; nothing skips.
option(NESTURBATOR_MMC3_ORACLE "Register the blargg MMC3 oracle tests (network)" OFF)
set(NESTURBATOR_MMC3_DIR ${CMAKE_BINARY_DIR}/mmc3-oracle CACHE PATH
  "Where mmc3.oracle.fetch keeps the fetched blargg ROMs (not in the source tree outside build/)")
if(NESTURBATOR_MMC3_ORACLE)
  find_program(NESTURBATOR_GIT git REQUIRED)
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/mmc3/oracle.txt)
  add_test(NAME mmc3.oracle.fetch
    COMMAND ${CMAKE_COMMAND}
      -DDIR=${NESTURBATOR_MMC3_DIR}
      -DPINS=${CMAKE_CURRENT_SOURCE_DIR}/mmc3/pins.txt
      -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
      -DGIT=${NESTURBATOR_GIT}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/fetch_blargg_mmc3.cmake)
  set_tests_properties(mmc3.oracle.fetch PROPERTIES
    FIXTURES_SETUP mmc3_oracle TIMEOUT 300 LABELS mmc3-oracle)
  file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/mmc3/oracle.txt mmc3_oracle_rows)
  foreach(row IN LISTS mmc3_oracle_rows)
    string(REPLACE "\t" ";" cols "${row}")
    list(GET cols 0 key)
    if(key MATCHES "^mmc3_test_2/(.*)$")
      set(rom ${NESTURBATOR_MMC3_DIR}/nes-test-roms/mmc3_test_2/rom_singles/${CMAKE_MATCH_1}.nes)
    elseif(key MATCHES "^mmc3_irq_tests/(.*)$")
      set(rom ${NESTURBATOR_MMC3_DIR}/nes-test-roms/mmc3_irq_tests/${CMAKE_MATCH_1}.nes)
    else()
      message(FATAL_ERROR "tests/mmc3/oracle.txt: unknown suite in key ${key}")
    endif()
    add_test(NAME mmc3.oracle.${key}
      COMMAND ${CMAKE_COMMAND}
        -DORACLE=$<TARGET_FILE:mmc3-oracle>
        -DROM=${rom}
        -DKEY=${key}
        -DTABLE=${CMAKE_CURRENT_SOURCE_DIR}/mmc3/oracle.txt
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/mmc3_oracle_run.cmake)
    set_tests_properties(mmc3.oracle.${key} PROPERTIES
      FIXTURES_REQUIRED mmc3_oracle TIMEOUT 120 LABELS mmc3-oracle)
  endforeach()
endif()

# CPU-02, D-22: the full 65x02 set, fetched by git at the pin in
# tests/vectors/pins.txt. These tests need the network, so they are registered
# only when NESTURBATOR_VECTORS_FULL is ON (the vectors-full preset) and the
# ci lane never fetches. Offline the fetch fails; nothing here skips.
option(NESTURBATOR_VECTORS_FULL "Register the full 65x02 vector tests (network)" OFF)
set(NESTURBATOR_VECTORS_DIR ${CMAKE_BINARY_DIR}/vectors-full CACHE PATH
  "Where cpu.vectors-full.fetch keeps the fetched 65x02 files (not in the source tree outside build/)")
if(NESTURBATOR_VECTORS_FULL)
  find_program(NESTURBATOR_GIT git REQUIRED)
  set(vectors_full_files ${NESTURBATOR_VECTORS_DIR}/65x02-src/nes6502/v1)
  set(vectors_full_out ${CMAKE_CURRENT_BINARY_DIR}/vectors-full)
  file(MAKE_DIRECTORY ${vectors_full_out})
  add_test(NAME cpu.vectors-full.fetch
    COMMAND ${CMAKE_COMMAND}
      -DDIR=${NESTURBATOR_VECTORS_DIR}
      -DPINS=${CMAKE_CURRENT_SOURCE_DIR}/vectors/pins.txt
      -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
      -DGIT=${NESTURBATOR_GIT}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/fetch_vectors.cmake)
  # The nightly's timeout-minutes is this TIMEOUT plus twice the rest of the
  # lane's measured time; change the two together.
  set_tests_properties(cpu.vectors-full.fetch PROPERTIES
    FIXTURES_SETUP vectors_full TIMEOUT 900 LABELS vectors-full)
  # D-22: the first 100 converted tests of every fetched file equal the
  # committed sample byte for byte, the proof of where the sample came from.
  add_test(NAME cpu.vectors-full.sample-match
    COMMAND ${CMAKE_COMMAND}
      -DVECCONV=$<TARGET_FILE:vecconv>
      -DDIR=${NESTURBATOR_VECTORS_DIR}
      -DBLOB=${CMAKE_CURRENT_SOURCE_DIR}/vectors/65x02-sample.n65v
      -DWORK=${vectors_full_out}/sample-match
      -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/vectors_sample_match.cmake)
  set_tests_properties(cpu.vectors-full.sample-match PROPERTIES
    FIXTURES_REQUIRED vectors_full LABELS vectors-full)
  # Each file's 10000 tests through the CPU, 00 to ff with no skip list. The
  # script's exit status decides the test, so it sets no regex property; a
  # failure prints the failing-vector count.
  foreach(op IN LISTS NESTURBATOR_ALL_OPCODES)
    add_test(NAME cpu.vectors-full.${op}
      COMMAND ${CMAKE_COMMAND}
        -DVECCONV=$<TARGET_FILE:vecconv>
        -DVECTORS=$<TARGET_FILE:cpu.vectors>
        -DJSON=${vectors_full_files}/${op}.json
        -DOPCODE=${op}
        -DOUT=${vectors_full_out}/${op}.n65v
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/vectors_full_run.cmake)
    set_tests_properties(cpu.vectors-full.${op} PROPERTIES
      FIXTURES_REQUIRED vectors_full LABELS vectors-full)
  endforeach()
endif()

# FRAME-02: install the library component into <build>/stage, then build
# tests/consumer against it with find_package and run it. The consumer gets
# the parent's compiler, build type, C flags and link flags, each as one
# argument so an empty value or one with spaces survives: a staged library
# built with sanitizers needs the same flags to link.
add_test(NAME install.stage
  COMMAND ${CMAKE_COMMAND} --install ${PROJECT_BINARY_DIR}
    --component library --prefix ${PROJECT_BINARY_DIR}/stage
    --config $<CONFIG>)
set_tests_properties(install.stage PROPERTIES FIXTURES_SETUP staged)
get_property(nesturbator_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
set(consumer_config)
if(nesturbator_multi_config)
  set(consumer_config --build-config $<CONFIG>)
endif()
add_test(NAME install.consumer
  COMMAND ${CMAKE_CTEST_COMMAND} --build-and-test
    ${CMAKE_CURRENT_SOURCE_DIR}/consumer ${PROJECT_BINARY_DIR}/consumer
    --build-generator ${CMAKE_GENERATOR}
    ${consumer_config}
    --build-options
      -DCMAKE_PREFIX_PATH=${PROJECT_BINARY_DIR}/stage
      -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      "-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}"
      "-DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}"
    --test-command consumer)
set_tests_properties(install.consumer PROPERTIES FIXTURES_REQUIRED staged)

# A C-only project embeds the source tree with add_subdirectory, gets only the
# library and its listed nesturbator_ helpers, and runs one frame with it.
add_test(NAME embed.subdirectory
  COMMAND ${CMAKE_CTEST_COMMAND} --build-and-test
    ${CMAKE_CURRENT_SOURCE_DIR}/embed ${PROJECT_BINARY_DIR}/embed
    --build-generator ${CMAKE_GENERATOR}
    ${consumer_config}
    --build-options
      -DNESTURBATOR_SOURCE_DIR=${PROJECT_SOURCE_DIR}
      -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      "-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}"
      "-DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}"
    --test-command embed)

# FRAME-05: the RetroArch screenshot comparison and its comparator's tests.
add_subdirectory(retroarch)

# D-06, D-07: the adapter sets no serialization quirks and asks for no savestate context.
add_test(NAME libretro.no_quirks
  COMMAND ${CMAKE_COMMAND}
    -DFILE=${PROJECT_SOURCE_DIR}/libretro/libretro.c
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/libretro_no_quirks.cmake)
set_tests_properties(libretro.no_quirks PROPERTIES TIMEOUT 60)

# D-18, D-19: the .info's save-state claim and the tests that back it. The claim-to-tests table
# lives only in cmake/info_claims.cmake. The guard runs once every add_test of this directory
# has run, so configuring fails if the .info claims a level whose tests are not registered.
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/info_claims.cmake)
cmake_language(DEFER CALL nesturbator_info_claim_guard
  "${PROJECT_SOURCE_DIR}/libretro/nesturbator_libretro.info")
add_test(NAME libretro.info_claims
  COMMAND ${CMAKE_COMMAND}
    -DINFO=${PROJECT_SOURCE_DIR}/libretro/nesturbator_libretro.info
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/info_claims.cmake)
add_test(NAME libretro.info_claims.self_test
  COMMAND ${CMAKE_COMMAND} -DSELFTEST=ON
    -DWORK=${CMAKE_CURRENT_BINARY_DIR}/info-claims
    "-DGENERATOR=${CMAKE_GENERATOR}"
    -DMAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/info_claims.cmake)
add_test(NAME libretro.info_support
  COMMAND ${CMAKE_COMMAND}
    "-DINFO=${PROJECT_SOURCE_DIR}/libretro/nesturbator_libretro.info"
    "-DREADME=${PROJECT_SOURCE_DIR}/README.md"
    "-DHEADER=${PROJECT_SOURCE_DIR}/include/nesturbator.h"
    -DSUPPORT_PROBE=ON
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/info_claims.cmake)
add_test(NAME libretro.info_staged
  COMMAND ${CMAKE_COMMAND}
    -DSTAGED=ON
    "-DSOURCE_ROOT=${PROJECT_SOURCE_DIR}"
    "-DBUILD_DIR=${PROJECT_BINARY_DIR}"
    "-DWORK=${PROJECT_BINARY_DIR}/tests/info-staged"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/info_claims.cmake)
set_tests_properties(libretro.info_claims libretro.info_claims.self_test libretro.info_support
  libretro.info_staged PROPERTIES TIMEOUT 60)
add_test(NAME libretro.info_archives.self_test
  COMMAND ${CMAKE_COMMAND}
    -DSELFTEST=ON
    "-DWORK=${PROJECT_BINARY_DIR}/tests/archive-info-selftest"
    "-DSOURCE_ROOT=${PROJECT_SOURCE_DIR}"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/check_archives.cmake)
set_tests_properties(libretro.info_archives.self_test PROPERTIES TIMEOUT 60)

# D-18: perf_compare.cmake on synthetic Cachegrind files, so the +1% rule, the UNSTABLE stop
# and the deliberate-slowdown path are shown by a ci command. Each case is defined in
# cmake/perf_gate_selftest.cmake.
foreach(perf_case identical plus_100 plus_101 improvement unstable missing_head report_na
    missing_base_gate allow_ok allow_ok_boundary allow_today allow_cap_low allow_malformed
    allow_duplicate allow_unknown allow_no_reason allow_too_far allow_expired unstable_allowed
    missing_head_and_base allow_reason_semicolon allow_reason_bracket allow_bracket_then_malformed)
  add_test(NAME perf.gate_selftest.${perf_case}
    COMMAND ${CMAKE_COMMAND} -DCASE=${perf_case} -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
      -DWORK=${CMAKE_CURRENT_BINARY_DIR}/perf_selftest/${perf_case}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/perf_gate_selftest.cmake)
  set_tests_properties(perf.gate_selftest.${perf_case} PROPERTIES LABELS perf)
endforeach()

# D-09: the three ROMs the perf job measures each draw a non-blank frame 60.
add_test(NAME perf.roms_render
  COMMAND ${CMAKE_COMMAND} "-DRUNNER=$<TARGET_FILE:nesturbator-run>"
    -DSOURCE_DIR=${PROJECT_SOURCE_DIR} -DWORK=${CMAKE_CURRENT_BINARY_DIR}/perf_roms_render
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/perf_roms_render.cmake)
set_tests_properties(perf.roms_render PROPERTIES LABELS perf TIMEOUT 60)
