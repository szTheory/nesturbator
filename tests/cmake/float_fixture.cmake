# Shows that the no-float checks can fail. tests/abi/float_fixture.c does a
# double multiply; the build target nesturbator_float_fixture compiles it with
# the core's -mgeneral-regs-only. GCC rejects floating point under that flag;
# Clang compiles it into soft-float calls such as __muldf3, which the
# undefined-symbol check must then reject. The test fails if the fixture
# builds and passes that check.
#
#   cmake -DBIN=<build dir> -DNM=<nm> -DLIB=<fixture library> -DAPPLE=<bool>
#         -DSCRIPTS=<tests/cmake> -P float_fixture.cmake

cmake_minimum_required(VERSION 3.25)

execute_process(
  COMMAND "${CMAKE_COMMAND}" --build "${BIN}" --target nesturbator_float_fixture
  RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)
if(NOT rc EQUAL 0)
  # The failure must be the fixture's own compile, not some other error.
  if(NOT "${out}${err}" MATCHES "float_fixture\\.c")
    message(FATAL_ERROR "fixture build failed for another reason:\n${out}${err}")
  endif()
  message(STATUS "the compiler rejected floating point in the fixture:\n${out}${err}")
  return()
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" "-DNM=${NM}" "-DLIB=${LIB}" "-DAPPLE=${APPLE}"
    -P "${SCRIPTS}/undefined_symbols.cmake"
  RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)
if(rc EQUAL 0)
  message(FATAL_ERROR "the fixture built and passed the undefined-symbol check:\n${out}${err}")
endif()
message(STATUS "the undefined-symbol check rejected the fixture:\n${out}${err}")
