# Checks a battery save file: its size, and the bytes at one offset.
#
#   cmake -DFILE=<.sav> -DSIZE=<bytes> -DOFFSET=<n> -DHEX=<lowercase hex> -P check_sav.cmake
#
# Holy Mapperel writes the ASCII text SAVEDATA at offset 0x100 of its battery
# RAM, so a save the board wrote back carries 5341564544415441 there.

cmake_minimum_required(VERSION 3.25)

foreach(var IN ITEMS FILE SIZE OFFSET HEX)
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "check_sav: -D${var} is required")
  endif()
endforeach()
if(NOT EXISTS "${FILE}")
  message(FATAL_ERROR "check_sav: ${FILE} does not exist")
endif()
file(SIZE "${FILE}" actual_size)
if(NOT actual_size EQUAL SIZE)
  message(FATAL_ERROR "check_sav: ${FILE} is ${actual_size} bytes; expected ${SIZE}")
endif()
string(LENGTH "${HEX}" hex_length)
math(EXPR byte_count "${hex_length} / 2")
file(READ "${FILE}" actual_hex HEX OFFSET ${OFFSET} LIMIT ${byte_count})
if(NOT actual_hex STREQUAL HEX)
  message(FATAL_ERROR "check_sav: bytes at offset ${OFFSET} are ${actual_hex}; expected ${HEX}")
endif()
message(STATUS "check_sav: ${FILE} is ${SIZE} bytes and holds ${HEX} at offset ${OFFSET}")
