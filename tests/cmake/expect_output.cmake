# Runs a command and checks its exact standard output and exit status.
#
#   cmake -DCMD=<program;arg;...> -DEXPECT=<line;line;...> -DEXPECT_EXIT=<code>
#         [-DIGNORE_STDOUT=ON] [-DEXPECT_ERR=<text>] [-DCLEAN_DIR=<directory>] -P expect_output.cmake
#
# EXPECT_ERR, when set and non-empty, must appear as a substring of standard error.
#
# CLEAN_DIR, when set, is emptied (removed and created again) before the
# command runs, so a test that writes there can be repeated.
#
# EXPECT lists the expected output lines; each line ends with a newline, and
# an empty EXPECT means no output at all. With IGNORE_STDOUT=ON only the exit
# status is checked (usage errors). Carriage returns are removed from
# the output before comparing, so Windows text-mode output compares equal.

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED CMD OR CMD STREQUAL "")
  message(FATAL_ERROR "expect_output: CMD is not set")
endif()
if(NOT DEFINED EXPECT_EXIT)
  set(EXPECT_EXIT 0)
endif()
if(DEFINED CLEAN_DIR AND NOT CLEAN_DIR STREQUAL "")
  file(REMOVE_RECURSE "${CLEAN_DIR}")
  file(MAKE_DIRECTORY "${CLEAN_DIR}")
endif()

execute_process(
  COMMAND ${CMD}
  OUTPUT_VARIABLE actual
  ERROR_VARIABLE actual_err
  RESULT_VARIABLE actual_exit)
string(REPLACE "\r" "" actual "${actual}")

set(expected "")
foreach(line IN LISTS EXPECT)
  string(APPEND expected "${line}\n")
endforeach()

set(failed FALSE)
if(NOT "${actual_exit}" STREQUAL "${EXPECT_EXIT}")
  set(failed TRUE)
endif()
if(NOT IGNORE_STDOUT AND NOT "${actual}" STREQUAL "${expected}")
  set(failed TRUE)
endif()
if(DEFINED EXPECT_ERR AND NOT EXPECT_ERR STREQUAL "")
  string(REPLACE "\r" "" err_text "${actual_err}")
  string(FIND "${err_text}" "${EXPECT_ERR}" err_pos)
  if(err_pos EQUAL -1)
    set(failed TRUE)
  endif()
endif()
if(failed)
  message(FATAL_ERROR
    "expect_output: mismatch\n"
    "command: ${CMD}\n"
    "expected exit: ${EXPECT_EXIT}\n"
    "actual exit:   ${actual_exit}\n"
    "expected stdout:\n[${expected}]\n"
    "actual stdout:\n[${actual}]\n"
    "expected stderr to contain: [${EXPECT_ERR}]\n"
    "stderr:\n${actual_err}")
endif()
