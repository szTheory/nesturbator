# One malformed input for vecconv (D-02): builds the case file in WORK from a
# committed fixture, runs vecconv on it, and passes only if vecconv exits 1
# with a byte offset ("offset <n>") on standard error. Exit 0, exit 2, a crash,
# or exit 1 without an offset fails, so the script's exit status alone decides
# the test.
#
#   cmake -DVECCONV=<vecconv> -DFIXTURE=<json> -DCASE=<name> -DWORK=<dir>
#         [-DFIRST=<n>] -P vecconv_negative.cmake
#
# FIRST defaults to 3. The cases, each an edit of the fixture's first test
# unless it says otherwise:
#
#   float        "s": 215     becomes "s": 1.5
#   sign         "a": 22      becomes "a": -1
#   exponent     "x": 214     becomes "x": 1e3
#   leading_zero "y": 9       becomes "y": 09
#   addr_range   "pc": 33710  becomes "pc": 65536
#   byte_range   "y": 9       becomes "y": 256
#   kind         the first "read" becomes "fetch"
#   unknown_key  "p" becomes "q"
#   missing_key  "y": 9 is removed
#   count        the first cycle list gets 256 entries
#   empty_file   a 0-byte file
#   empty_array  [] without --first
#   closed_short the fixture's three tests and a closing ] without --first
#   truncated    the fixture cut inside its 3rd test, with --first 3
#   as_is        the fixture unchanged (for a fixture whose cut ends early)

cmake_minimum_required(VERSION 3.25)

foreach(var VECCONV FIXTURE CASE WORK)
  if(NOT DEFINED ${var} OR "${${var}}" STREQUAL "")
    message(FATAL_ERROR "vecconv_negative: ${var} is not set")
  endif()
endforeach()
if(NOT DEFINED FIRST OR FIRST STREQUAL "")
  set(FIRST 3)
endif()

file(READ "${FIXTURE}" fixture)
set(text "${fixture}")
set(use_first ON)

# Replaces the first occurrence of FROM in text; fails if there is none.
macro(replace_first from to)
  string(FIND "${text}" "${from}" at)
  if(at LESS 0)
    message(FATAL_ERROR "vecconv_negative: ${CASE}: '${from}' is not in ${FIXTURE}")
  endif()
  string(LENGTH "${from}" from_len)
  string(SUBSTRING "${text}" 0 ${at} head)
  math(EXPR rest_at "${at} + ${from_len}")
  string(SUBSTRING "${text}" ${rest_at} -1 tail)
  set(text "${head}${to}${tail}")
endmacro()

if(CASE STREQUAL "float")
  replace_first("\"s\": 215" "\"s\": 1.5")
elseif(CASE STREQUAL "sign")
  replace_first("\"a\": 22" "\"a\": -1")
elseif(CASE STREQUAL "exponent")
  replace_first("\"x\": 214" "\"x\": 1e3")
elseif(CASE STREQUAL "leading_zero")
  replace_first("\"y\": 9" "\"y\": 09")
elseif(CASE STREQUAL "addr_range")
  replace_first("\"pc\": 33710" "\"pc\": 65536")
elseif(CASE STREQUAL "byte_range")
  replace_first("\"y\": 9" "\"y\": 256")
elseif(CASE STREQUAL "kind")
  replace_first("\"read\"" "\"fetch\"")
elseif(CASE STREQUAL "unknown_key")
  replace_first("\"p\": " "\"q\": ")
elseif(CASE STREQUAL "missing_key")
  replace_first("\"y\": 9, " "")
elseif(CASE STREQUAL "count")
  # The opcode fetch, then 255 operand reads: 256 entries, one over the u8
  # count N65V allows.
  set(list "[[33710, 169, \"read\"]")
  foreach(i RANGE 1 255)
    string(APPEND list ", [33711, 195, \"read\"]")
  endforeach()
  replace_first("[ [33710, 169, \"read\"], [33711, 195, \"read\"]]" "${list}]")
elseif(CASE STREQUAL "empty_file")
  set(text "")
elseif(CASE STREQUAL "empty_array")
  set(text "[]\n")
  set(use_first OFF)
elseif(CASE STREQUAL "closed_short")
  set(text "${fixture}\n]\n")
  set(use_first OFF)
elseif(CASE STREQUAL "truncated")
  # Find the 3rd test's opening and cut 40 bytes into it.
  set(at -1)
  foreach(i 1 2 3)
    math(EXPR from "${at} + 1")
    string(SUBSTRING "${text}" ${from} -1 rest)
    string(FIND "${rest}" "{ \"name\"" found)
    if(found LESS 0)
      message(FATAL_ERROR "vecconv_negative: ${CASE}: ${FIXTURE} has fewer than 3 tests")
    endif()
    math(EXPR at "${from} + ${found}")
  endforeach()
  math(EXPR cut "${at} + 40")
  string(SUBSTRING "${text}" 0 ${cut} text)
elseif(CASE STREQUAL "as_is")
else()
  message(FATAL_ERROR "vecconv_negative: unknown CASE ${CASE}")
endif()

if(NOT CASE STREQUAL "as_is" AND text STREQUAL fixture)
  message(FATAL_ERROR "vecconv_negative: ${CASE}: the edit left the fixture unchanged")
endif()

file(MAKE_DIRECTORY "${WORK}")
set(in "${WORK}/${CASE}.json")
set(out "${WORK}/${CASE}.n65v")
file(WRITE "${in}" "${text}")
file(REMOVE "${out}")
set(cmd "${VECCONV}" "${in}" "${out}" a9)
if(use_first)
  list(APPEND cmd --first ${FIRST})
endif()
execute_process(COMMAND ${cmd}
  OUTPUT_VARIABLE conv_out ERROR_VARIABLE conv_err RESULT_VARIABLE rc)
string(REPLACE "\r" "" conv_err "${conv_err}")
string(STRIP "${conv_err}" conv_err)
message(STATUS "vecconv exited ${rc}: ${conv_err}")
if(NOT rc STREQUAL "1")
  message(FATAL_ERROR "vecconv_negative: ${CASE}: vecconv exited ${rc}, expected 1")
endif()
if(NOT conv_err MATCHES "offset [0-9]+")
  message(FATAL_ERROR "vecconv_negative: ${CASE}: no byte offset on standard error")
endif()
