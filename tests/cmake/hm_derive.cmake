# Builds one derived Holy Mapperel ROM from a committed base, refusing a base
# whose SHA-256 is not the manifest's. The copy is made at build time, never
# committed, and keeps the base's size.
#
#   cmake -DTOOL=<holymapperel-derive> -DBASE=<rom> -DBASE_SHA256=<hex>
#     -DOUT=<file> -DPATCHES=<patch;patch> -P hm_derive.cmake

cmake_minimum_required(VERSION 3.25)

foreach(var IN ITEMS TOOL BASE BASE_SHA256 OUT PATCHES)
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "hm_derive: -D${var} is required")
  endif()
endforeach()
file(SHA256 "${BASE}" actual_sha256)
if(NOT actual_sha256 STREQUAL BASE_SHA256)
  message(FATAL_ERROR "hm_derive: ${BASE} has sha256 ${actual_sha256}; the manifest says ${BASE_SHA256}")
endif()
get_filename_component(out_dir "${OUT}" DIRECTORY)
file(MAKE_DIRECTORY "${out_dir}")
execute_process(COMMAND "${TOOL}" "${BASE}" "${OUT}" ${PATCHES}
  RESULT_VARIABLE rc ERROR_VARIABLE errors)
if(NOT rc EQUAL 0)
  file(REMOVE "${OUT}")
  message(FATAL_ERROR "hm_derive: tool exited with ${rc}: ${errors}")
endif()
file(SIZE "${BASE}" base_size)
file(SIZE "${OUT}" out_size)
if(NOT out_size EQUAL base_size)
  file(REMOVE "${OUT}")
  message(FATAL_ERROR "hm_derive: ${OUT} is ${out_size} bytes; the base is ${base_size}")
endif()
message(STATUS "hm_derive: wrote ${OUT}")
