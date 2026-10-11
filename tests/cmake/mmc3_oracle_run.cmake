# Runs one blargg MMC3 ROM through mmc3-oracle and compares its exit status
# with the key's row in tests/mmc3/oracle.txt (09 D-10, D-11).
#
#   cmake -DORACLE=<mmc3-oracle> -DROM=<file.nes> -DKEY=<key> -DTABLE=<oracle.txt>
#         -P mmc3_oracle_run.cmake
#
# A row is key<TAB>status<TAB>code<TAB>frames<TAB>hash. A pass row expects
# exit 0x00 and an unsupported row expects exactly its recorded code, so a
# row that no longer fails is reported and must be edited. The protocol is
# 6000 for mmc3_test_2/* and f8 for mmc3_irq_tests/*.

# Script mode starts with no policies set; pin them as the other scripts do.
cmake_minimum_required(VERSION 3.25)

foreach(var ORACLE ROM KEY TABLE)
  if(NOT DEFINED ${var} OR "${${var}}" STREQUAL "")
    message(FATAL_ERROR "mmc3_oracle_run: ${var} is not set")
  endif()
endforeach()

file(STRINGS "${TABLE}" rows)
set(row "")
foreach(line IN LISTS rows)
  string(REPLACE "\t" ";" cols "${line}")
  list(GET cols 0 key)
  if(key STREQUAL KEY)
    set(row "${cols}")
  endif()
endforeach()
if(row STREQUAL "")
  message(FATAL_ERROR "mmc3_oracle_run: no row for ${KEY} in ${TABLE}")
endif()
list(GET row 1 status)
list(GET row 2 want_code)
list(GET row 3 frames)
math(EXPR want "${want_code}")
if(KEY MATCHES "^mmc3_test_2/")
  set(protocol 6000)
elseif(KEY MATCHES "^mmc3_irq_tests/")
  set(protocol f8)
else()
  message(FATAL_ERROR "mmc3_oracle_run: unknown suite in key ${KEY}")
endif()

execute_process(
  COMMAND "${ORACLE}" --rom "${ROM}" --frames ${frames} --protocol ${protocol}
  OUTPUT_VARIABLE out
  ERROR_VARIABLE err
  RESULT_VARIABLE rc)
string(REPLACE "\r" "" out "${out}")
string(STRIP "${out}" out)
message(STATUS "${out}")
if(NOT rc MATCHES "^[0-9]+$")
  message(FATAL_ERROR "mmc3_oracle_run: ${KEY}: mmc3-oracle did not exit normally: ${rc}\n${err}")
endif()
if(status STREQUAL "unsupported" AND rc EQUAL 0)
  message(FATAL_ERROR "mmc3_oracle_run: ${KEY}: unsupported row passed; edit tests/mmc3/oracle.txt")
endif()
if(NOT rc EQUAL want)
  message(FATAL_ERROR "mmc3_oracle_run: ${KEY}: exit ${rc}, row expects ${want} (${status})\n${err}")
endif()
