cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SCOREBOARD OR "${SCOREBOARD}" STREQUAL "")
  message(FATAL_ERROR "AccuracyCoin NMI target gate needs SCOREBOARD")
endif()
if(NOT EXISTS "${SCOREBOARD}" OR IS_DIRECTORY "${SCOREBOARD}")
  message(FATAL_ERROR "AccuracyCoin NMI target scoreboard is missing: ${SCOREBOARD}")
endif()

set(nmi_targets
  "accuracycoin/$2002 flag timing"
  "accuracycoin/NMI Timing"
  "accuracycoin/NMI Suppression"
  "accuracycoin/NMI Control"
  "accuracycoin/NMI at VBlank end"
  "accuracycoin/NMI disabled at VBlank")
set(found_targets)
string(ASCII 9 tab)
file(STRINGS "${SCOREBOARD}" scoreboard_lines)
foreach(line IN LISTS scoreboard_lines)
  string(REPLACE "${tab}" ";" fields "${line}")
  list(LENGTH fields field_count)
  if(field_count LESS 2)
    continue()
  endif()
  list(GET fields 0 key)
  if(NOT key IN_LIST nmi_targets)
    continue()
  endif()
  if(key IN_LIST found_targets)
    message(FATAL_ERROR "AccuracyCoin NMI target appears more than once: ${key}")
  endif()
  if(NOT field_count EQUAL 5)
    message(FATAL_ERROR "AccuracyCoin NMI target row is malformed: ${key}")
  endif()
  list(GET fields 1 status)
  if(NOT status STREQUAL "pass")
    message(FATAL_ERROR "AccuracyCoin NMI target must pass: ${key} (status=${status})")
  endif()
  list(APPEND found_targets "${key}")
endforeach()

foreach(key IN LISTS nmi_targets)
  if(NOT key IN_LIST found_targets)
    message(FATAL_ERROR "AccuracyCoin NMI target is missing: ${key}")
  endif()
endforeach()

message(STATUS "AccuracyCoin exact-six NMI target gate PASS")
