# Writes the runner's hash lines for frames 1 and 3 to a file, so CI can
# compare them byte for byte across platforms (the hash-equality job,
# ENGINEERING section 5).
#
#   cmake -DBUILD=<build dir> -DOUT=<file> -P write_hashes.cmake
#
# The runner is looked for at ${BUILD}/runner/nesturbator-run, then with
# .exe, where every preset's single-configuration Ninja build puts it.
# Carriage returns are removed before writing, because the Windows C runtime
# writes "\r\n" on text-mode standard output.

if(NOT DEFINED BUILD OR NOT DEFINED OUT)
  message(FATAL_ERROR "write_hashes: pass -DBUILD=<build dir> -DOUT=<file>")
endif()

set(tried "${BUILD}/runner/nesturbator-run" "${BUILD}/runner/nesturbator-run.exe")
set(runner "")
foreach(candidate IN LISTS tried)
  if(EXISTS "${candidate}" AND NOT IS_DIRECTORY "${candidate}")
    set(runner "${candidate}")
    break()
  endif()
endforeach()
if(runner STREQUAL "")
  list(JOIN tried " and " names)
  message(FATAL_ERROR "write_hashes: no runner at ${names}")
endif()

execute_process(
  COMMAND "${runner}" --frames 3 --hash-frame 1 --hash-frame 3
  OUTPUT_VARIABLE hashes
  ERROR_VARIABLE errors
  RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
  message(FATAL_ERROR "write_hashes: ${runner} exited with ${rc}: ${errors}")
endif()
string(REPLACE "\r" "" hashes "${hashes}")
file(WRITE "${OUT}" "${hashes}")
message(STATUS "write_hashes: wrote ${OUT}")
