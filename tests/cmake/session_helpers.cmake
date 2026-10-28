cmake_minimum_required(VERSION 3.25)

# Native session helpers retain the exact FNV-1a contract and closed 1 MiB cap.
function(helper_require_binary path)
  file(READ "${path}" actual HEX)
  if(NOT actual STREQUAL "010a0d7f80ff")
    message(FATAL_ERROR "session fixture bytes: expected 010a0d7f80ff, actual ${actual}")
  endif()
endfunction()
if(DEFINED CHECK_FIXTURE)
  helper_require_binary("${CHECK_FIXTURE}")
  return()
endif()

file(MAKE_DIRECTORY "${WORK}")
function(helper_expect expected_result expected_output)
  execute_process(COMMAND "${FRAME_INDEX}" ${ARGN}
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 5)
  string(STRIP "${output}" output)
  if(NOT "${result}" STREQUAL "${expected_result}" OR
     NOT "${output}" STREQUAL "${expected_output}")
    message(FATAL_ERROR "session helper ${ARGN}: status=${result} output=${output} error=${error}")
  endif()
endfunction()

file(WRITE "${WORK}/empty" "")
helper_expect(2 "" --session-hash "${WORK}/empty")
file(REMOVE "${WORK}/missing")
helper_expect(2 "" --session-hash "${WORK}/missing")
file(WRITE "${WORK}/text" "foobar")
helper_expect(0 "85944171f73967e8" --session-hash "${WORK}/text")
helper_expect(0 "" --session-binary-fixture "${WORK}/binary")
helper_require_binary("${WORK}/binary")
helper_expect(0 "112184356382ae07" --session-hash "${WORK}/binary")
# Run the actual byte gate in fresh processes: a newline conversion or altered
# high byte must fail BEFORE the hash oracle can consume a malformed fixture.
set(crlf_hex "010d0a0d7f80ff")
set(high_byte_hex "010a0d7f80fe")
foreach(mutation IN ITEMS crlf high_byte)
  helper_expect(0 "" --session-binary-fixture "${WORK}/${mutation}-mutant" "${mutation}")
  file(READ "${WORK}/${mutation}-mutant" actual HEX)
  if(NOT actual STREQUAL "${${mutation}_hex}")
    message(FATAL_ERROR "session ${mutation} mutation bytes are incorrect: ${actual}")
  endif()
  execute_process(COMMAND "${CMAKE_COMMAND}"
    "-DCHECK_FIXTURE=${WORK}/${mutation}-mutant" -P "${CMAKE_CURRENT_LIST_FILE}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 5)
  if(NOT result EQUAL 1 OR NOT error MATCHES "session fixture bytes: expected 010a0d7f80ff")
    message(FATAL_ERROR "session fixture ${mutation} mutation was not rejected by byte gate: ${result} ${output} ${error}")
  endif()
endforeach()
helper_expect(2 "" --session-binary-fixture "${WORK}/unused" invalid)
# Frozen independent vectors cover singleton, streaming-buffer neighbors and cap neighbors.
foreach(pair IN ITEMS
    "1:af63f54c86021707" "4095:abb6e3b14aef2bbf" "4096:b6f0ad4254676325"
    "4097:4259c4b56badd707" "1048575:4b86c341f1296bbf" "1048576:7f69900cc9622325")
  string(REPLACE ":" ";" fields "${pair}")
  list(GET fields 0 size)
  list(GET fields 1 hash)
  string(REPEAT "x" ${size} bytes)
  file(WRITE "${WORK}/bytes" "${bytes}")
  helper_expect(0 "${hash}" --session-hash "${WORK}/bytes")
endforeach()
string(REPEAT "x" 1048577 bytes)
file(WRITE "${WORK}/bytes" "${bytes}")
helper_expect(2 "" --session-hash "${WORK}/bytes")
helper_expect(2 "" --session-hash)
helper_expect(2 "" --session-clock extra)

execute_process(COMMAND "${FRAME_INDEX}" --session-clock RESULT_VARIABLE status
  OUTPUT_VARIABLE first OUTPUT_STRIP_TRAILING_WHITESPACE TIMEOUT 5)
if(NOT status EQUAL 0 OR NOT first MATCHES "^[1-9][0-9]*$")
  message(FATAL_ERROR "native monotonic clock unavailable")
endif()
execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 0.05)
execute_process(COMMAND "${FRAME_INDEX}" --session-clock RESULT_VARIABLE status
  OUTPUT_VARIABLE second OUTPUT_STRIP_TRAILING_WHITESPACE TIMEOUT 5)
if(NOT status EQUAL 0 OR NOT second MATCHES "^[1-9][0-9]*$" OR second LESS_EQUAL first)
  message(FATAL_ERROR "native monotonic clock did not advance")
endif()
message(STATUS "session helpers: exact binary bytes, 2 rejected mutations, FNV vectors, buffer/cap boundaries and monotonic clock passed")
