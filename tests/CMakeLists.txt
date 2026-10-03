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
# --frames is required: a run with no arguments exits 2.
nesturbator_runner_test(NAME runner.usage.noargs
  EXIT 2 IGNORE_STDOUT)
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

# CI's hash-equality job compares the file write_hashes.cmake writes on each
# platform. It must be the runner.advance lines with LF endings only, byte for
# byte tests/runner/hashes.txt.
add_test(NAME runner.write_hashes
  COMMAND ${CMAKE_COMMAND} -DBUILD=${PROJECT_BINARY_DIR}
    -DOUT=${CMAKE_CURRENT_BINARY_DIR}/hashes.txt
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/write_hashes.cmake)
set_tests_properties(runner.write_hashes PROPERTIES FIXTURES_SETUP hashes_txt)
add_test(NAME runner.write_hashes.content
  COMMAND ${CMAKE_COMMAND} -E compare_files
    ${CMAKE_CURRENT_BINARY_DIR}/hashes.txt ${CMAKE_CURRENT_SOURCE_DIR}/runner/hashes.txt)
set_tests_properties(runner.write_hashes.content PROPERTIES FIXTURES_REQUIRED hashes_txt)

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
add_executable(libretro.host libretro/libretro_host.c)
target_include_directories(libretro.host PRIVATE ${PROJECT_SOURCE_DIR}/libretro)
target_link_libraries(libretro.host PRIVATE ${CMAKE_DL_LIBS})
target_compile_definitions(libretro.host PRIVATE
  $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
nesturbator_warnings(libretro.host)
add_dependencies(libretro.host nesturbator_libretro)
add_test(NAME libretro.host
  COMMAND libretro.host $<TARGET_FILE:nesturbator_libretro> ${NESTURBATOR_TESTFRAME_PPM})
set_tests_properties(libretro.host PROPERTIES FIXTURES_REQUIRED testframe_ppm)

# D-14: palgen regenerates src/palette_ntsc.c byte for byte, ignoring line
# endings.
add_test(NAME palette.regen
  COMMAND ${CMAKE_COMMAND}
    -DPALGEN=$<TARGET_FILE:palgen>
    -DOUT=${CMAKE_CURRENT_BINARY_DIR}/palette_ntsc.c
    -DREF=${PROJECT_SOURCE_DIR}/src/palette_ntsc.c
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/palette_regen.cmake)

# CPU-01, D-01 and D-03: vecconv converts the first three upstream a9 tests
# (tests/vectors/fixtures) to N65V and reads them back through the shared
# reader.
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/vectors)
add_test(NAME vecconv.a9
  COMMAND vecconv ${CMAKE_CURRENT_SOURCE_DIR}/vectors/fixtures/a9-first3.json
    ${CMAKE_CURRENT_BINARY_DIR}/vectors/a9-first3.n65v a9 --first 3)
set_tests_properties(vecconv.a9 PROPERTIES
  PASS_REGULAR_EXPRESSION "vecconv: a9: 3 tests written and re-read")

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
# library, and runs one frame with it.
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
