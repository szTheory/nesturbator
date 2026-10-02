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
