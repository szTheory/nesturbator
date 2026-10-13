cmake_minimum_required(VERSION 3.25)

# PERF-02 (15-CONTEXT D-18): run tests/cmake/perf_compare.cmake on synthetic Cachegrind files.
#   cmake -DCASE=<name> -DSOURCE_DIR=<repo> -DWORK=<scratch dir> -P perf_gate_selftest.cmake
# Every side has Ir(60) = 1000000 and Ir(360) = 1000000 + d. The default d is 3000000. A case
# lists overrides (DS: who:rom:d), missing sides (MISSING: who:rom; who base drops both bases),
# the allow text it writes (ALLOW_TEXT), the mode, whether it must exit zero or non-zero
# (EXIT: 0 or 1), substrings the output must hold (EXPECT) and must not hold (FORBID).

foreach(var CASE SOURCE_DIR WORK)
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "${var} is required")
  endif()
endforeach()

set(cases "")
macro(define_case name)
  list(APPEND cases ${name})
  cmake_parse_arguments(C_${name} "" "MODE;EXIT;ALLOW_TEXT" "DS;MISSING;EXPECT;FORBID" ${ARGN})
endmacro()

set(row_ok_nest "| nesteroids | 0 | 10000 | 10000 | +0 | +0.00% | ok |")
set(row_ok_run "| runner | 1 | 10000 | 10000 | +0 | +0.00% | ok |")
set(row_ok_m4 "| M4_P256K_C256K | 4 | 10000 | 10000 | +0 | +0.00% | ok |")

define_case(identical EXIT 0 EXPECT "${row_ok_nest}" "${row_ok_run}" "${row_ok_m4}")
define_case(plus_100 EXIT 0 DS head:nesteroids:3030000
  EXPECT "| nesteroids | 0 | 10000 | 10100 | +100 | +1.00% | ok |")
define_case(plus_101 EXIT 1 DS head:nesteroids:3030300 EXPECT "exceeds +1.00%")
define_case(improvement EXIT 0 DS head:nesteroids:2000000
  EXPECT "| nesteroids | 0 | 10000 | 6666 | -3333 | -33.33% | ok |")
define_case(unstable EXIT 1 DS base_b:runner:3000001 EXPECT "UNSTABLE")
define_case(missing_head EXIT 1 MISSING head:M4_P256K_C256K EXPECT "missing head for M4_P256K_C256K")
define_case(report_na EXIT 0 MODE report MISSING head:M4_P256K_C256K
  EXPECT "| M4_P256K_C256K | 4 | 10000 | n/a | n/a | n/a | n/a |")
define_case(missing_base_gate EXIT 0 MISSING base:nesteroids base:runner base:M4_P256K_C256K
  EXPECT "| nesteroids | 0 | n/a | 10000 | n/a | n/a | n/a |"
         "| runner | 1 | n/a | 10000 | n/a | n/a | n/a |"
         "| M4_P256K_C256K | 4 | n/a | 10000 | n/a | n/a | n/a |")

# D-17: the allow file. Line 1 is a comment naming the case, so the line under test is line 2.
macro(define_allow_case name line exit)
  define_case(${name} EXIT ${exit} DS head:nesteroids:3030300
    ALLOW_TEXT "# ${name}\n${line}\n" ${ARGN})
endmacro()
define_allow_case(allow_ok "nesteroids 150 2026-11-30 deliberate slowdown fixture" 0
  EXPECT "| nesteroids | 0 | 10000 | 10101 | +101 | +1.01% | ALLOWED |"
    "ALLOWED nesteroids cap 1.50% expires 2026-11-30 (tests/perf/allow.txt line 2)"
  FORBID "deliberate")
define_allow_case(allow_ok_boundary "nesteroids 150 2026-12-11 sixty days" 0
  EXPECT "| ALLOWED |")
define_allow_case(allow_today "nesteroids 150 2026-10-12 expires today" 0
  EXPECT "| ALLOWED |")
define_allow_case(allow_cap_low "nesteroids 100 2026-11-30 cap below delta" 1
  EXPECT "exceeds its allow cap")
define_allow_case(allow_malformed "nesteroids 1.5 2026-11-30 bad cap" 1
  EXPECT "malformed allow line")
define_case(allow_duplicate EXIT 1 DS head:nesteroids:3030300
  ALLOW_TEXT "# allow_duplicate\nnesteroids 150 2026-11-30 one\nnesteroids 150 2026-11-30 two\n"
  EXPECT "duplicate allow line for nesteroids")
define_allow_case(allow_unknown "zelda 150 2026-11-30 not ours" 1
  EXPECT "unknown ROM in allow line: zelda")
define_allow_case(allow_no_reason "nesteroids 150 2026-11-30" 1
  EXPECT "allow line has no reason")
define_allow_case(allow_too_far "nesteroids 150 2026-12-12 too far" 1
  EXPECT "expires more than 60 days after 2026-10-12")
define_allow_case(allow_expired "nesteroids 150 2026-10-11 old" 1
  EXPECT "allow line expired")
define_case(unstable_allowed EXIT 1 DS base_b:nesteroids:3000001 head:nesteroids:3030300
  ALLOW_TEXT "# unstable_allowed\nnesteroids 150 2026-11-30 reason\n"
  EXPECT "UNSTABLE")

# The names of all cases, for tests/CMakeLists.txt.
if(CASE STREQUAL "--list")
  message("${cases}")
  return()
endif()

list(FIND cases "${CASE}" at)
if(at EQUAL -1)
  message(FATAL_ERROR "unknown case '${CASE}'")
endif()
set(p C_${CASE})

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")
file(READ "${SOURCE_DIR}/tests/perf/fixtures/cachegrind.out.in" template)
set(roms nesteroids runner M4_P256K_C256K)
foreach(who base_a base_b head)
  foreach(rom IN LISTS roms)
    set(d 3000000)
    foreach(entry IN LISTS ${p}_DS)
      string(REGEX MATCH "^([a-z_]+):([A-Za-z0-9_]+):([0-9]+)$" m "${entry}")
      if(m AND CMAKE_MATCH_1 STREQUAL who AND CMAKE_MATCH_2 STREQUAL rom)
        set(d ${CMAKE_MATCH_3})
      endif()
    endforeach()
    set(missing FALSE)
    foreach(entry IN LISTS ${p}_MISSING)
      if(entry STREQUAL "${who}:${rom}" OR (entry STREQUAL "base:${rom}" AND who MATCHES "^base_"))
        set(missing TRUE)
      endif()
    endforeach()
    if(missing)
      continue()
    endif()
    foreach(n 60 360)
      if(n EQUAL 60)
        set(ir 1000000)
      else()
        math(EXPR ir "1000000 + ${d}")
      endif()
      string(REPLACE "@IR@" "${ir}" text "${template}")
      string(REPLACE "@FRAMES@" "${n}" text "${text}")
      file(WRITE "${WORK}/${who}_${rom}_${n}.out" "${text}")
    endforeach()
  endforeach()
endforeach()

set(allow "${SOURCE_DIR}/tests/perf/allow.txt")
if(DEFINED ${p}_ALLOW_TEXT)
  set(allow "${WORK}/allow.txt")
  file(WRITE "${allow}" "${${p}_ALLOW_TEXT}")
endif()
set(mode gate)
if(DEFINED ${p}_MODE)
  set(mode ${${p}_MODE})
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} -DMODE=${mode} -DDIR=${WORK} -DALLOW=${allow}
    -DLABEL=tests/perf/allow.txt -DOUT=${WORK}/summary.md -DTODAY=2026-10-12
    -P ${SOURCE_DIR}/tests/cmake/perf_compare.cmake
  RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)
set(summary "")
if(EXISTS "${WORK}/summary.md")
  file(READ "${WORK}/summary.md" summary)
endif()
set(all "${out}${err}${summary}")

if(${p}_EXIT EQUAL 0 AND NOT rc EQUAL 0)
  message(FATAL_ERROR "${CASE}: expected exit 0, got ${rc}\n${all}")
endif()
if(NOT ${p}_EXIT EQUAL 0 AND rc EQUAL 0)
  message(FATAL_ERROR "${CASE}: expected a non-zero exit, got 0\n${all}")
endif()
foreach(needle IN LISTS ${p}_EXPECT)
  string(FIND "${all}" "${needle}" at)
  if(at EQUAL -1)
    message(FATAL_ERROR "${CASE}: expected substring '${needle}' not found\n${all}")
  endif()
endforeach()
foreach(needle IN LISTS ${p}_FORBID)
  string(FIND "${all}" "${needle}" at)
  if(NOT at EQUAL -1)
    message(FATAL_ERROR "${CASE}: forbidden substring '${needle}' found\n${all}")
  endif()
endforeach()
message(STATUS "${CASE}: ok")
