# Regenerates the committed vector sample tests/vectors/65x02-sample.n65v
# from the SingleStepTests 65x02 vectors at one upstream commit (D-07). For
# each opcode 00 to ff it downloads the first 64 KiB of nes6502/v1/<xx>.json,
# converts its first 100 tests with vecconv, and then joins the 256 chunks in
# opcode order. It prints the size, the SHA-256 and the line for
# tests/roms/manifest.txt. About 16 MB is downloaded; a pin bump is its own
# change (CONFORMANCE).
#
#   cmake -DCOMMIT=<40-hex sha> -DVECCONV=<vecconv> -DWORK=<dir> -DOUT=<blob>
#         -P vectors_regen.cmake
#
# COMMIT has no default. The prefixes and chunks are kept in WORK.
cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED COMMIT OR NOT COMMIT MATCHES "^[0-9a-f]+$")
  message(FATAL_ERROR "COMMIT must be the upstream commit, 40 lowercase hex digits")
endif()
string(LENGTH "${COMMIT}" n)
if(NOT n EQUAL 40)
  message(FATAL_ERROR "COMMIT must be the upstream commit, 40 lowercase hex digits")
endif()
foreach(var VECCONV WORK OUT)
  if(NOT ${var})
    message(FATAL_ERROR "${var} is required")
  endif()
endforeach()

file(MAKE_DIRECTORY "${WORK}")
set(base "https://raw.githubusercontent.com/SingleStepTests/65x02/${COMMIT}/nes6502/v1")
set(chunks)
# Two lowercase hex digits per opcode; math(OUTPUT_FORMAT HEXADECIMAL) would
# give 0x0, unpadded.
foreach(h 0 1 2 3 4 5 6 7 8 9 a b c d e f)
  foreach(l 0 1 2 3 4 5 6 7 8 9 a b c d e f)
    set(xx "${h}${l}")
    set(prefix "${WORK}/${xx}.prefix.json")
    # RANGE_END is inclusive: bytes 0 to 65535. TLS_VERIFY is explicit because
    # CMake verifies by default only from 3.31.
    file(DOWNLOAD "${base}/${xx}.json" "${prefix}"
      RANGE_START 0 RANGE_END 65535 TLS_VERIFY ON STATUS st)
    list(GET st 0 code)
    if(NOT code EQUAL 0)
      message(FATAL_ERROR "${xx}.json: download failed: ${st}")
    endif()
    execute_process(
      COMMAND "${VECCONV}" "${prefix}" "${WORK}/${xx}.n65v" "${xx}" --first 100
      RESULT_VARIABLE rc OUTPUT_QUIET)
    if(NOT rc EQUAL 0)
      message(FATAL_ERROR "${xx}.json: vecconv exited with ${rc}")
    endif()
    list(APPEND chunks "${WORK}/${xx}.n65v")
  endforeach()
endforeach()

execute_process(COMMAND "${CMAKE_COMMAND}" -E cat ${chunks}
  OUTPUT_FILE "${OUT}" RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
  message(FATAL_ERROR "joining the chunks into ${OUT} failed")
endif()

file(SIZE "${OUT}" size)
file(SHA256 "${OUT}" sha)
message(STATUS "${OUT}: ${size} bytes, sha256 ${sha}")
message(STATUS "manifest line:")
message("tests/vectors/65x02-sample.n65v\thttps://github.com/SingleStepTests/65x02\t${COMMIT}\tMIT\t${sha}")
