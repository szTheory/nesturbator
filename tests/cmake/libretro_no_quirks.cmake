cmake_minimum_required(VERSION 3.25)

# D-06, D-07: the adapter never sets serialization quirks and never asks the frontend for the
# savestate context. The state bytes are the library's whatever the frontend reports, so a
# quirk or a context query would be a second source of truth. -DFILE=libretro/libretro.c.
#
# The searched names are built from halves so this script's own text does not match a
# repository-wide search for them.
if(NOT DEFINED FILE)
  message(FATAL_ERROR "libretro_no_quirks: -DFILE=<path> is required")
endif()
file(READ "${FILE}" text)
string(LENGTH "${text}" length)
if(length EQUAL 0)
  message(FATAL_ERROR "libretro_no_quirks: ${FILE} is empty or unreadable")
endif()
set(prefix "RETRO_ENVIRONMENT_")
foreach(suffix SET_SERIALIZATION_QUIRKS GET_SAVESTATE_CONTEXT)
  string(FIND "${text}" "${prefix}${suffix}" at)
  if(NOT at EQUAL -1)
    message(FATAL_ERROR "libretro_no_quirks: ${FILE} names ${prefix}${suffix}")
  endif()
endforeach()
message(STATUS "libretro_no_quirks: ok")
