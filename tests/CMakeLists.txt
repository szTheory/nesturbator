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
nesturbator_runner_test(NAME runner.usage.positional
  ARGS cartridge EXIT 2 IGNORE_STDOUT)

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
nesturbator_runner_test(NAME runner.usage.dump.file
  ARGS --frames 1 --dump-frame 1: EXIT 2 IGNORE_STDOUT)

# A file that cannot be created (its directory does not exist) exits 1.
nesturbator_runner_test(NAME runner.dump.unwritable
  ARGS --frames 1 --dump-frame "1:${CMAKE_BINARY_DIR}/no-such-dir/frame1.ppm"
  EXIT 1)

# CI's hash-equality job compares every game boot and DABG movie milestone
# written by write_hashes.cmake on each platform.
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

# Keep the CI cross-platform hash inventory contract executable locally.
add_test(NAME hash.inventory.self_test
  COMMAND ${CMAKE_COMMAND}
    -DSELFTEST=ON
    "-DWORK=${PROJECT_BINARY_DIR}/hash-inventory-selftest"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/hash_inventory.cmake)
set_tests_properties(hash.inventory.self_test PROPERTIES TIMEOUT 30)

# AccuracyCoin page runs are kept separate so a failure names the page whose
# RAM-derived results diverged from the committed scoreboard.
set(ACCURACYCOIN_ROM ${PROJECT_SOURCE_DIR}/tests/roms/accuracycoin.nes)
set(ACCURACYCOIN_SCOREBOARD ${CMAKE_CURRENT_SOURCE_DIR}/accuracy/scoreboard.txt)
foreach(page IN ITEMS 2 14 17)
  add_test(NAME accuracycoin.page${page}
    COMMAND ${CMAKE_COMMAND}
      "-DCMD=$<TARGET_FILE:nesturbator-run>;--accuracycoin-page;${page};--rom;${ACCURACYCOIN_ROM};--scoreboard;${ACCURACYCOIN_SCOREBOARD}"
      "-DEXPECT=accuracycoin page ${page} PASS"
      -DEXPECT_EXIT=0
      -DIGNORE_STDOUT=TRUE
      -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/expect_output.cmake)
  set_tests_properties(accuracycoin.page${page} PROPERTIES TIMEOUT 60)
endforeach()

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
nesturbator_core_test(core.apu core/test_apu.c)
nesturbator_core_test(core.synth core/test_synth.c)
target_include_directories(core.synth PRIVATE ${PROJECT_SOURCE_DIR}/src)
nesturbator_core_test(core.cartridge core/test_cartridge.c)
target_include_directories(core.cartridge PRIVATE ${PROJECT_SOURCE_DIR}/src)
# D-18: soft reset on a synthetic NROM image with a trainer.
nesturbator_core_test(core.reset core/test_reset.c)
target_include_directories(core.reset PRIVATE ${PROJECT_SOURCE_DIR}/src)
# MAP-01, D-04, D-05, D-06, D-08: cycle-stamped mapper writes, the IRQ OR, the nametable map and reset.
nesturbator_core_test(core.mapper core/test_mapper.c)
target_include_directories(core.mapper PRIVATE ${PROJECT_SOURCE_DIR}/src)
add_executable(core.controller core/test_controller.c)
target_link_libraries(core.controller PRIVATE nesturbator::nesturbator)
target_include_directories(core.controller PRIVATE ${PROJECT_SOURCE_DIR}/src)
nesturbator_warnings(core.controller)
add_test(NAME core.controller COMMAND core.controller)

# Focused CPU-visible PPU register and dot-boundary tests.
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

# Linux nightly only: the same source/entry point and sanitized library, with
# Clang's libFuzzer main and runtime instrumentation.
option(NESTURBATOR_LIBFUZZER "Build the ROM loader libFuzzer target" OFF)
if(NESTURBATOR_LIBFUZZER)
  if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "NESTURBATOR_LIBFUZZER requires Clang")
  endif()
  add_executable(fuzz.rom_loader.libfuzzer fuzz/rom_loader.c)
  target_link_libraries(fuzz.rom_loader.libfuzzer PRIVATE nesturbator::nesturbator)
  target_compile_options(fuzz.rom_loader.libfuzzer PRIVATE -fsanitize=fuzzer-no-link)
  target_link_options(fuzz.rom_loader.libfuzzer PRIVATE -fsanitize=fuzzer,address,undefined)
  nesturbator_warnings(fuzz.rom_loader.libfuzzer)
endif()

add_test(NAME fuzz.registration
  COMMAND ${CMAKE_COMMAND}
    -DCTEST=${CMAKE_CTEST_COMMAND}
    -DBUILD_DIR=${PROJECT_BINARY_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/fuzz_registration.cmake)

# D-14: nesturbator_get_palette's copy rules, calibrated sRGB samples and
# palette invariants.
nesturbator_core_test(core.palette core/test_palette.c)
nesturbator_core_test(accuracy.scoreboard accuracy/test_scoreboard.c)
target_compile_definitions(accuracy.scoreboard PRIVATE
  NESTURBATOR_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
  $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)

# The candidate scoreboard must fail if it drops a prior protected-main PASS.
set(ACCURACYCOIN_REGRESSION_BASELINE ${CMAKE_CURRENT_BINARY_DIR}/scoreboard-regression-baseline.txt)
file(WRITE ${ACCURACYCOIN_REGRESSION_BASELINE}
  "accuracycoin/Removed PASS\tpass\t0x01\t-\t-\n")
add_test(NAME accuracy.scoreboard.no_lost_pass
  COMMAND ${CMAKE_COMMAND}
    "-DSCOREBOARD_TEST=$<TARGET_FILE:accuracy.scoreboard>"
    "-DREGRESSION_BASELINE=${ACCURACYCOIN_REGRESSION_BASELINE}"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/verify_scoreboard_regression.cmake)

# Exercise the CI-only handoff failure and the confirmed empty first-merge case
# with isolated, build-directory fixtures rather than the local snapshot.
add_test(NAME accuracy.scoreboard.ci_missing_baseline
  COMMAND ${CMAKE_COMMAND} -E env --unset=NESTURBATOR_SCOREBOARD_BASELINE CI=true
    $<TARGET_FILE:accuracy.scoreboard>)
set_tests_properties(accuracy.scoreboard.ci_missing_baseline PROPERTIES WILL_FAIL TRUE)
set(ACCURACYCOIN_EMPTY_BASELINE ${CMAKE_CURRENT_BINARY_DIR}/scoreboard-empty-baseline.txt)
file(WRITE ${ACCURACYCOIN_EMPTY_BASELINE} "")
add_test(NAME accuracy.scoreboard.ci_empty_baseline
  COMMAND ${CMAKE_COMMAND} -E env CI=true
    "NESTURBATOR_SCOREBOARD_BASELINE=${ACCURACYCOIN_EMPTY_BASELINE}"
    $<TARGET_FILE:accuracy.scoreboard>)

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

# version.txt equals the header's version macros.
add_test(NAME version.consistency
  COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version_consistency.cmake)

# Every script in tests/cmake sets its policy version, so CMake reads it the
# same way on every runner.
add_test(NAME cmake.script_policy
  COMMAND ${CMAKE_COMMAND} -DDIR=${CMAKE_CURRENT_SOURCE_DIR}/cmake
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/script_policy.cmake)

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
# policy configures the full preset but never runs its network-backed fixture.
add_test(NAME vectors.registration_policy
  COMMAND ${CMAKE_COMMAND}
    -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
    -DCI_BUILD=${PROJECT_BINARY_DIR}
    -DFULL_BUILD=${PROJECT_BINARY_DIR}/../vectors-full
    -DCTEST=${CMAKE_CTEST_COMMAND}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/vector_registration_policy.cmake)
set_tests_properties(vectors.registration_policy PROPERTIES COST 1000)
add_test(NAME vectors.registration_policy.self_test
  COMMAND ${CMAKE_COMMAND} -DSELFTEST=ON
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/vector_registration_policy.cmake)
set_tests_properties(vectors.registration_policy.self_test PROPERTIES COST 200)

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
add_executable(libretro.host libretro/libretro_host.c)
target_include_directories(libretro.host PRIVATE ${PROJECT_SOURCE_DIR}/libretro
  ${CMAKE_CURRENT_SOURCE_DIR}/runner)
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
  ${PROJECT_SOURCE_DIR}/src/apu.c
  ${PROJECT_SOURCE_DIR}/src/synth.c
  ${PROJECT_SOURCE_DIR}/src/cartridge.c
  ${PROJECT_SOURCE_DIR}/src/mapper_nrom.c
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
