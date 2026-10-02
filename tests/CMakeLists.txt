# Runner tests: each runs nesturbator-run through expect_output.cmake, which
# checks the exact standard output and exit status.
#
#   nesturbator_runner_test(NAME <test> ARGS <arg>... EXIT <code>
#                           [IGNORE_STDOUT] [EXPECT <line>...])
#
# Without EXPECT and IGNORE_STDOUT the output must be empty.
function(nesturbator_runner_test)
  cmake_parse_arguments(T "IGNORE_STDOUT" "NAME;EXIT" "ARGS;EXPECT" ${ARGN})
  # A quoted add_test argument keeps its semicolons, so each list arrives
  # whole as one -D value.
  set(cmd "$<TARGET_FILE:nesturbator-run>" ${T_ARGS})
  add_test(NAME ${T_NAME}
    COMMAND ${CMAKE_COMMAND}
      "-DCMD=${cmd}"
      "-DEXPECT=${T_EXPECT}"
      "-DEXPECT_EXIT=${T_EXIT}"
      "-DIGNORE_STDOUT=${T_IGNORE_STDOUT}"
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

# Usage errors exit 2 (LIBRETRO-AND-RUNNER section 5).
nesturbator_runner_test(NAME runner.usage.beyond
  ARGS --frames 1 --hash-frame 2 EXIT 2 IGNORE_STDOUT)
nesturbator_runner_test(NAME runner.usage.zero
  ARGS --frames 1 --hash-frame 0 EXIT 2 IGNORE_STDOUT)
nesturbator_runner_test(NAME runner.usage.unknown
  ARGS --bogus EXIT 2 IGNORE_STDOUT)
# A cartridge path arrives in Phase 3; until then a positional argument is a
# usage error.
nesturbator_runner_test(NAME runner.usage.positional
  ARGS cartridge EXIT 2 IGNORE_STDOUT)

# FRAME-03 / D-16: --dump-frame writes the test card as a 256x240 P6 image
# (15 header bytes + 256*240*3) while the hash line stays over the native
# pixels. Pixel (8,232) is native $12, whose table entry is 0x552AFF. Later
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
    -DPIXELS=8:232:552AFF
    -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/check_ppm.cmake")
set_tests_properties(runner.dump PROPERTIES FIXTURES_SETUP testframe_ppm)

# --dump-frame usage errors exit 2: N beyond --frames, no colon, no FILE.
nesturbator_runner_test(NAME runner.usage.dump
  ARGS --dump-frame 2:x.ppm --frames 1 EXIT 2 IGNORE_STDOUT)
nesturbator_runner_test(NAME runner.usage.dump.colon
  ARGS --frames 1 --dump-frame 1 EXIT 2 IGNORE_STDOUT)
nesturbator_runner_test(NAME runner.usage.dump.file
  ARGS --frames 1 --dump-frame 1: EXIT 2 IGNORE_STDOUT)

# A file that cannot be created (its directory does not exist) exits 1.
nesturbator_runner_test(NAME runner.dump.unwritable
  ARGS --frames 1 --dump-frame "1:${CMAKE_BINARY_DIR}/no-such-dir/frame1.ppm"
  EXIT 1)

# Core tests: programs using tests/check.h, through the public header only.
# Each exits 0 when every check passes.
function(nesturbator_core_test name source)
  add_executable(${name} ${source})
  target_link_libraries(${name} PRIVATE nesturbator::nesturbator)
  nesturbator_warnings(${name})
  add_test(NAME ${name} COMMAND ${name})
endfunction()

# D-11: version, info, create, every create status and the allocators.
nesturbator_core_test(core.api core/test_api.c)
# D-11: run_frame statuses, buffer rules, the D-09 audio period, the D-02
# boundary pixels and two independent instances.
nesturbator_core_test(core.frame core/test_frame.c)
# D-14: nesturbator_get_palette's copy rules and the palette invariants.
nesturbator_core_test(core.palette core/test_palette.c)

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

# version.txt equals the header's version macros.
add_test(NAME version.consistency
  COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version_consistency.cmake)

# D-14: palgen regenerates src/palette_ntsc.c byte for byte, ignoring line
# endings.
add_test(NAME palette.regen
  COMMAND ${CMAKE_COMMAND}
    -DPALGEN=$<TARGET_FILE:palgen>
    -DOUT=${CMAKE_CURRENT_BINARY_DIR}/palette_ntsc.c
    -DREF=${PROJECT_SOURCE_DIR}/src/palette_ntsc.c
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/palette_regen.cmake)
