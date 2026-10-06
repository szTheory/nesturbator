# Converts one upstream 65x02 JSON fixture to N65V with vecconv and, with RUN
# ON (the default), runs the result through the CPU with cpu.vectors.
#
#   cmake -DVECCONV=<vecconv> -DVECTORS=<cpu.vectors> -DJSON=<file.json>
#         -DOPCODE=<xx> -DFIRST=<n> -DOUT=<file.n65v> [-DRUN=ON|OFF]
#         -P vectors_fixture.cmake
#
# The test passes on this script's exit status alone: cpu.vectors must exit 0
# and print exactly "65x02/<xx>: 0 of <n> vectors failed". Carriage returns
# are removed from captured output first, so Windows text-mode output
# compares equal. Both tools' standard output is echoed as status lines.

cmake_minimum_required(VERSION 3.25)

foreach(var VECCONV JSON OPCODE FIRST OUT)
  if(NOT DEFINED ${var} OR "${${var}}" STREQUAL "")
    message(FATAL_ERROR "vectors_fixture: ${var} is not set")
  endif()
endforeach()
if(NOT DEFINED RUN)
  set(RUN ON)
endif()

execute_process(
  COMMAND ${VECCONV} ${JSON} ${OUT} ${OPCODE} --first ${FIRST}
  OUTPUT_VARIABLE conv_out
  ERROR_VARIABLE conv_err
  RESULT_VARIABLE conv_rc)
string(REPLACE "\r" "" conv_out "${conv_out}")
string(REPLACE "\r" "" conv_err "${conv_err}")
string(STRIP "${conv_out}" conv_line)
message(STATUS "${conv_line}")
if(NOT conv_rc EQUAL 0)
  message(FATAL_ERROR "vectors_fixture: vecconv exited ${conv_rc}\n${conv_err}")
endif()

if(NOT RUN)
  return()
endif()

if(NOT DEFINED VECTORS OR VECTORS STREQUAL "")
  message(FATAL_ERROR "vectors_fixture: VECTORS is not set")
endif()
execute_process(
  COMMAND ${VECTORS} ${OUT} ${OPCODE} 1 ${FIRST}
  OUTPUT_VARIABLE run_out
  ERROR_VARIABLE run_err
  RESULT_VARIABLE run_rc)
string(REPLACE "\r" "" run_out "${run_out}")
string(REPLACE "\r" "" run_err "${run_err}")
string(STRIP "${run_out}" run_line)
message(STATUS "${run_line}")
set(expected "65x02/${OPCODE}: 0 of ${FIRST} vectors failed\n")
if(NOT run_rc EQUAL 0 OR NOT run_out STREQUAL expected)
  message(FATAL_ERROR
    "vectors_fixture: cpu.vectors exited ${run_rc}\n"
    "expected stdout: [${expected}]\n"
    "actual stdout:   [${run_out}]\n"
    "stderr:\n${run_err}")
endif()
