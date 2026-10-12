# D-15: the state_format row in tests/runner/hashes.txt must equal
# NESTURBATOR_STATE_FORMAT_VERSION in include/nesturbator.h, so changing the
# state layout cannot go unpinned. Reads source files only.
#
#   cmake -DSOURCE_DIR=<repo> -P state_format_pin.cmake

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
  message(FATAL_ERROR "state_format_pin: -DSOURCE_DIR=<repo> is required")
endif()

file(STRINGS "${SOURCE_DIR}/include/nesturbator.h" macro_lines
  REGEX "^#define NESTURBATOR_STATE_FORMAT_VERSION [0-9]+$")
file(STRINGS "${SOURCE_DIR}/tests/runner/hashes.txt" row_lines REGEX "^state_format [0-9]+$")
list(LENGTH macro_lines macro_count)
list(LENGTH row_lines row_count)
if(NOT macro_count EQUAL 1)
  message(FATAL_ERROR "state_format_pin: NESTURBATOR_STATE_FORMAT_VERSION is not defined exactly once in include/nesturbator.h")
endif()
if(NOT row_count EQUAL 1)
  message(FATAL_ERROR "state_format_pin: tests/runner/hashes.txt has ${row_count} state_format rows; expected 1")
endif()
string(REGEX REPLACE "^.* ([0-9]+)$" "\\1" macro "${macro_lines}")
string(REGEX REPLACE "^.* ([0-9]+)$" "\\1" pinned "${row_lines}")
if(NOT macro STREQUAL pinned)
  message(FATAL_ERROR "state_format_pin: hashes.txt pins state_format ${pinned} but NESTURBATOR_STATE_FORMAT_VERSION is ${macro}; repin the state hashes with the new format")
endif()
message(STATUS "state format ${macro} is pinned")
