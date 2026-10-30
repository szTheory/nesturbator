cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SCOREBOARD OR "${SCOREBOARD}" STREQUAL "")
  message(FATAL_ERROR "AccuracyCoin v5 target gate needs SCOREBOARD")
endif()
if(NOT EXISTS "${SCOREBOARD}" OR IS_DIRECTORY "${SCOREBOARD}")
  message(FATAL_ERROR "AccuracyCoin v5 target scoreboard is missing: ${SCOREBOARD}")
endif()

set(v5_targets
  "accuracycoin/Interrupt flag latency"
  "accuracycoin/NMI Overlap BRK"
  "accuracycoin/NMI Overlap IRQ"
  "accuracycoin/$2002 flag timing"
  "accuracycoin/NMI Control"
  "accuracycoin/NMI Timing"
  "accuracycoin/NMI Suppression"
  "accuracycoin/NMI at VBlank end"
  "accuracycoin/NMI disabled at VBlank"
  "accuracycoin/Sprite 0 Hit behavior"
  "accuracycoin/Arbitrary Sprite zero")

set(found_targets)
string(ASCII 9 tab)
file(STRINGS "${SCOREBOARD}" scoreboard_lines)
foreach(line IN LISTS scoreboard_lines)
  string(REPLACE "${tab}" ";" fields "${line}")
  list(LENGTH fields field_count)
  if(field_count EQUAL 0)
    continue()
  endif()
  list(GET fields 0 key)
  if(NOT key IN_LIST v5_targets)
    continue()
  endif()
  if(key IN_LIST found_targets)
    message(FATAL_ERROR "AccuracyCoin v5 target duplicate: ${key}")
  endif()
  if(NOT field_count EQUAL 5)
    message(FATAL_ERROR "AccuracyCoin v5 target row is malformed: ${key}")
  endif()
  list(GET fields 1 status)
  if(NOT status STREQUAL "pass")
    message(FATAL_ERROR "AccuracyCoin v5 target failed: ${key}")
  endif()
  list(APPEND found_targets "${key}")
endforeach()

foreach(key IN LISTS v5_targets)
  if(NOT key IN_LIST found_targets)
    message(FATAL_ERROR "AccuracyCoin v5 target missing: ${key}")
  endif()
endforeach()

message(STATUS "AccuracyCoin exact-eleven v5 target gate PASS")
