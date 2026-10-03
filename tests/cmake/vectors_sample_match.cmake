# The provenance proof for the committed sample (D-22): the first 100
# converted tests of every fetched full file equal the committed
# tests/vectors/65x02-sample.n65v byte for byte, chunk by chunk.
#
#   cmake -DVECCONV=<vecconv> -DDIR=<NESTURBATOR_VECTORS_DIR>
#         -DBLOB=<65x02-sample.n65v> -DWORK=<dir> -P vectors_sample_match.cmake
#
# For xx from 00 to ff, vecconv converts DIR/src/nes6502/v1/<xx>.json with
# --first 100, and the chunk must equal the blob's bytes at the running
# offset. At the end the offset must equal the blob's size. The first
# difference fails with the chunk and the blob offset.

# Script mode starts with no policies set; pin them as the other scripts do.
cmake_minimum_required(VERSION 3.25)

foreach(var VECCONV DIR BLOB WORK)
  if(NOT DEFINED ${var} OR "${${var}}" STREQUAL "")
    message(FATAL_ERROR "vectors_sample_match: ${var} is not set")
  endif()
endforeach()

file(MAKE_DIRECTORY "${WORK}")
file(SIZE "${BLOB}" blob_size)
set(o 0)
foreach(h 0 1 2 3 4 5 6 7 8 9 a b c d e f)
  foreach(l 0 1 2 3 4 5 6 7 8 9 a b c d e f)
    set(xx ${h}${l})
    set(chunk "${WORK}/${xx}.first100.n65v")
    execute_process(
      COMMAND ${VECCONV} "${DIR}/src/nes6502/v1/${xx}.json" "${chunk}" ${xx} --first 100
      OUTPUT_VARIABLE conv_out
      ERROR_VARIABLE conv_err
      RESULT_VARIABLE conv_rc)
    if(NOT conv_rc EQUAL 0)
      message(FATAL_ERROR "sample-match: vecconv ${xx} exited ${conv_rc}\n${conv_out}${conv_err}")
    endif()
    file(SIZE "${chunk}" n)
    math(EXPR end "${o} + ${n}")
    if(end GREATER blob_size)
      message(FATAL_ERROR "sample-match: chunk ${xx} differs at blob offset ${o} (it runs past the blob's ${blob_size} bytes)")
    endif()
    file(READ "${chunk}" got HEX)
    file(READ "${BLOB}" want OFFSET ${o} LIMIT ${n} HEX)
    if(NOT got STREQUAL want)
      message(FATAL_ERROR "sample-match: chunk ${xx} differs at blob offset ${o}")
    endif()
    set(o ${end})
  endforeach()
endforeach()
if(NOT o EQUAL blob_size)
  message(FATAL_ERROR "sample-match: 256 chunks end at offset ${o}, the blob holds ${blob_size} bytes")
endif()
message(STATUS "sample-match: 256 chunks equal the committed sample, ${o} bytes")
