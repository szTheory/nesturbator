# Checks that a failed vector does not leave its writes in RAM for the next
# one. Two INC $10 tests (opcode e6) are written as JSON, converted with
# vecconv and run with cpu.vectors:
#
# 1. $10 is not listed, so it reads as 0 and the CPU writes 0 and then 1 to
#    it; the expected A is wrong, so the test fails.
# 2. $10 is not listed again, so it must read as 0 again and end as 1.
#
# cpu.vectors must exit 1 and print "65x02/e6: 1 of 2 vectors failed". If
# test 1's write to $10 survived, test 2 would read 1 and fail too.
#
#   cmake -DVECCONV=<vecconv> -DVECTORS=<cpu.vectors> -DWORK=<dir>
#         -P vectors_stray_write.cmake

cmake_minimum_required(VERSION 3.25)

foreach(var VECCONV VECTORS WORK)
  if(NOT DEFINED ${var} OR "${${var}}" STREQUAL "")
    message(FATAL_ERROR "vectors_stray_write: ${var} is not set")
  endif()
endforeach()

file(MAKE_DIRECTORY "${WORK}")
set(json "${WORK}/e6-stray.json")
set(blob "${WORK}/e6-stray.n65v")

# INC $10 at 0x0200 (512): opcode 230 (e6), operand 16. Its five cycles read
# the opcode, the operand and $10, write the old value back, then the new one.
set(cycles "[ [512, 230, \"read\"], [513, 16, \"read\"], [16, 0, \"read\"], [16, 0, \"write\"], [16, 1, \"write\"]]")
set(code "[512, 230], [513, 16]")
file(WRITE "${json}" "[
{ \"name\": \"e6 stray\", \"initial\": { \"pc\": 512, \"s\": 253, \"a\": 0, \"x\": 0, \"y\": 0, \"p\": 36, \"ram\": [ ${code}]}, \"final\": { \"pc\": 514, \"s\": 253, \"a\": 153, \"x\": 0, \"y\": 0, \"p\": 36, \"ram\": [ ${code}]}, \"cycles\": ${cycles} },
{ \"name\": \"e6 clean\", \"initial\": { \"pc\": 512, \"s\": 253, \"a\": 0, \"x\": 0, \"y\": 0, \"p\": 36, \"ram\": [ ${code}]}, \"final\": { \"pc\": 514, \"s\": 253, \"a\": 0, \"x\": 0, \"y\": 0, \"p\": 36, \"ram\": [ ${code}, [16, 1]]}, \"cycles\": ${cycles} }
]
")

execute_process(
  COMMAND ${VECCONV} ${json} ${blob} e6 --first 2
  OUTPUT_VARIABLE conv_out
  ERROR_VARIABLE conv_err
  RESULT_VARIABLE conv_rc)
if(NOT conv_rc EQUAL 0)
  message(FATAL_ERROR "vectors_stray_write: vecconv exited ${conv_rc}\n${conv_out}${conv_err}")
endif()

execute_process(
  COMMAND ${VECTORS} ${blob} e6 1 2
  OUTPUT_VARIABLE run_out
  ERROR_VARIABLE run_err
  RESULT_VARIABLE run_rc)
string(REPLACE "\r" "" run_out "${run_out}")
set(expected "65x02/e6: 1 of 2 vectors failed\n")
if(NOT run_rc EQUAL 1 OR NOT run_out STREQUAL expected)
  message(FATAL_ERROR
    "vectors_stray_write: cpu.vectors exited ${run_rc}\n"
    "expected stdout: [${expected}]\n"
    "actual stdout:   [${run_out}]\n"
    "stderr:\n${run_err}")
endif()
message(STATUS "vectors_stray_write: the failed test's write did not reach the next test")
