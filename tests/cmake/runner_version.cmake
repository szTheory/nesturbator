# D-12: nesturbator-run --version prints exactly "nesturbator-run <version.txt>"
# and a newline on stdout, nothing on stderr, and exits 0. This extends the chain
# version.txt = header macros = runtime value = runner output.
#
#   cmake -DRUNNER=<nesturbator-run> -DSOURCE_DIR=<repository root> -P runner_version.cmake

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED RUNNER OR NOT DEFINED SOURCE_DIR)
  message(FATAL_ERROR "runner_version: RUNNER and SOURCE_DIR are required")
endif()
file(STRINGS "${SOURCE_DIR}/version.txt" text_version LIMIT_COUNT 1)
set(expected "nesturbator-run ${text_version}\n")

foreach(extra "" "--bogus")
  execute_process(COMMAND "${RUNNER}" --version ${extra}
    RESULT_VARIABLE status OUTPUT_VARIABLE out ERROR_VARIABLE err)
  string(REPLACE "\r" "" out "${out}")
  string(REPLACE "\r" "" err "${err}")
  if(NOT status EQUAL 0)
    message(FATAL_ERROR "--version ${extra} exited ${status}, expected 0")
  endif()
  if(NOT out STREQUAL expected)
    message(FATAL_ERROR "--version ${extra} stdout is '${out}', expected '${expected}'")
  endif()
  if(NOT err STREQUAL "")
    message(FATAL_ERROR "--version ${extra} wrote to stderr: '${err}'")
  endif()
endforeach()
message(STATUS "runner --version ${text_version}")
