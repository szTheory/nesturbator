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

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED BUILD OR NOT DEFINED OUT)
  message(FATAL_ERROR "write_hashes: pass -DBUILD=<build dir> -DOUT=<file>")
endif()

# The cross-platform inventory must fail closed when any approved game is
# missing from the manifest. Keep the exact paths here so an empty or partial
# ROM set can never produce a passing, vacuous hash artifact.
file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/../roms/manifest.txt" manifest_lines)
foreach(required_rom IN ITEMS nesteroids dabg rhde)
  set(found_rom FALSE)
  foreach(line IN LISTS manifest_lines)
    if(line MATCHES "^tests/roms/${required_rom}\\.nes\\t")
      set(found_rom TRUE)
    endif()
  endforeach()
  if(NOT found_rom)
    message(FATAL_ERROR "write_hashes: required game ROM ${required_rom}.nes is missing from tests/roms/manifest.txt")
  endif()
endforeach()

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
# file(WRITE) gave a file that differed from the LF reference on both
# Windows runners (CI run 37084293594); file(CONFIGURE) is told the line
# ending, LF, so every platform writes the same bytes. It ends the content
# with a newline of its own, so the runner's last one is removed first.
string(REGEX REPLACE "\n$" "" hashes "${hashes}")
file(CONFIGURE OUTPUT "${OUT}" CONTENT "@hashes@" @ONLY NEWLINE_STYLE LF)
message(STATUS "write_hashes: wrote ${OUT}")
