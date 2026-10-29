cmake_minimum_required(VERSION 3.25)

foreach(required IN ITEMS SCOREBOARD GATE WORK)
  if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
    message(FATAL_ERROR "AccuracyCoin v5 target control needs ${required}")
  endif()
endforeach()
if(NOT EXISTS "${SCOREBOARD}" OR NOT EXISTS "${GATE}")
  message(FATAL_ERROR "AccuracyCoin v5 target control input is missing")
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

execute_process(COMMAND "${CMAKE_COMMAND}" "-DSCOREBOARD=${SCOREBOARD}" -P "${GATE}"
  RESULT_VARIABLE baseline_result OUTPUT_VARIABLE baseline_out ERROR_VARIABLE baseline_err)
if(NOT "${baseline_result}" STREQUAL "0")
  message(FATAL_ERROR "complete v5 target sibling failed the gate: ${baseline_err}${baseline_out}")
endif()

string(ASCII 9 TAB)
function(write_mutation name target)
  file(STRINGS "${SCOREBOARD}" scoreboard_lines)
  set(output "")
  set(found FALSE)
  foreach(line IN LISTS scoreboard_lines)
    string(REPLACE "${TAB}" ";" fields "${line}")
    list(LENGTH fields field_count)
    if(field_count GREATER 0)
      list(GET fields 0 key)
      if(key STREQUAL target)
        set(found TRUE)
        if(name STREQUAL "missing")
          continue()
        elseif(name STREQUAL "failed")
          string(REPLACE "${TAB}pass${TAB}" "${TAB}fail${TAB}" changed_line "${line}")
          if(changed_line STREQUAL line)
            message(FATAL_ERROR "v5 target control needs a passing row: ${target}")
          endif()
          string(APPEND output "${changed_line}\n")
        elseif(name STREQUAL "duplicate")
          string(APPEND output "${line}\n${line}\n")
        else()
          message(FATAL_ERROR "unknown v5 target mutation: ${name}")
        endif()
        continue()
      endif()
    endif()
    string(APPEND output "${line}\n")
  endforeach()
  if(NOT found)
    message(FATAL_ERROR "v5 target control could not locate ${target}")
  endif()
  file(WRITE "${WORK}/${name}-${target}.txt" "${output}")
  set(MUTATED_SCOREBOARD "${WORK}/${name}-${target}.txt" PARENT_SCOPE)
endfunction()

function(expect_gate_rejection name target expected_diagnostic)
  execute_process(COMMAND "${CMAKE_COMMAND}" "-DSCOREBOARD=${MUTATED_SCOREBOARD}" -P "${GATE}"
    RESULT_VARIABLE gate_result OUTPUT_VARIABLE gate_out ERROR_VARIABLE gate_err)
  set(gate_log "${gate_err}${gate_out}")
  if("${gate_result}" STREQUAL "0")
    message(FATAL_ERROR "${name} control was accepted for ${target}")
  endif()
  string(FIND "${gate_log}" "${expected_diagnostic}${target}" diagnostic_index)
  if(diagnostic_index EQUAL -1)
    message(FATAL_ERROR
      "${name} control failed without the intended target diagnostic for ${target}: ${gate_log}")
  endif()
endfunction()

file(MAKE_DIRECTORY "${WORK}")
foreach(target IN LISTS v5_targets)
  write_mutation(missing "${target}")
  expect_gate_rejection("missing" "${target}" "AccuracyCoin v5 target missing: ")
  write_mutation(failed "${target}")
  expect_gate_rejection("failed" "${target}" "AccuracyCoin v5 target failed: ")
  write_mutation(duplicate "${target}")
  expect_gate_rejection("duplicate" "${target}" "AccuracyCoin v5 target duplicate: ")
endforeach()

message(STATUS "AccuracyCoin exact-eleven v5 target controls passed")
