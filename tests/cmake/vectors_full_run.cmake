# Converts one full upstream 65x02 file to N65V and runs all of it through
# the CPU (CPU-02).
#
#   cmake -DVECCONV=<vecconv> -DVECTORS=<cpu.vectors> -DJSON=<xx.json>
#         -DOPCODE=<xx> -DOUT=<xx.n65v> -P vectors_full_run.cmake
#
# vecconv runs without --first, so the file must hold exactly 10000 tests.
# The test passes on this script's exit status alone: both tools must exit 0
# and cpu.vectors must print exactly "65x02/<xx>: 0 of 10000 vectors failed".
# Carriage returns are removed from captured output first, as in
# expect_output.cmake. Both tools' output is printed, so a failing test shows
# its failing-vector count.

# Script mode starts with no policies set; pin them as the other scripts do.
cmake_minimum_required(VERSION 3.25)

foreach(var VECCONV VECTORS JSON OPCODE OUT)
  if(NOT DEFINED ${var} OR "${${var}}" STREQUAL "")
    message(FATAL_ERROR "vectors_full_run: ${var} is not set")
  endif()
endforeach()

execute_process(
  COMMAND ${VECCONV} ${JSON} ${OUT} ${OPCODE}
  OUTPUT_VARIABLE conv_out
  ERROR_VARIABLE conv_err
  RESULT_VARIABLE conv_rc)
string(REPLACE "\r" "" conv_out "${conv_out}")
string(REPLACE "\r" "" conv_err "${conv_err}")
string(STRIP "${conv_out}" conv_line)
message(STATUS "${conv_line}")
if(NOT conv_rc EQUAL 0)
  message(FATAL_ERROR "vectors_full_run: vecconv exited ${conv_rc}\n${conv_err}")
endif()

execute_process(
  COMMAND ${VECTORS} ${OUT} ${OPCODE} 1 10000
  OUTPUT_VARIABLE run_out
  ERROR_VARIABLE run_err
  RESULT_VARIABLE run_rc)
string(REPLACE "\r" "" run_out "${run_out}")
string(REPLACE "\r" "" run_err "${run_err}")
string(STRIP "${run_out}" run_line)
message(STATUS "${run_line}")
set(expected "65x02/${OPCODE}: 0 of 10000 vectors failed\n")
if(NOT run_rc EQUAL 0 OR NOT run_out STREQUAL expected)
  message(FATAL_ERROR
    "vectors_full_run: cpu.vectors exited ${run_rc}\n"
    "expected stdout: [${expected}]\n"
    "actual stdout:   [${run_out}]\n"
    "stderr:\n${run_err}")
endif()
