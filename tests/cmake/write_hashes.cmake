# Writes a sorted inventory of native frame hashes for every licensed game
# ROM plus the two-port DABG input scripts. The hash is over native pixels,
# independent of the display palette.
#
#   cmake -DBUILD=<build dir> -DOUT=<file> -DMOVIE_WRITER=<exe> -P write_hashes.cmake

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED BUILD OR NOT DEFINED OUT OR NOT DEFINED MOVIE_WRITER)
  message(FATAL_ERROR "write_hashes: pass -DBUILD=<build dir> -DOUT=<file> -DMOVIE_WRITER=<exe>")
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
if(NOT EXISTS "${MOVIE_WRITER}")
  message(FATAL_ERROR "write_hashes: DABG movie writer is missing at ${MOVIE_WRITER}")
endif()

set(source_dir "${CMAKE_CURRENT_LIST_DIR}/../..")
file(STRINGS "${source_dir}/tests/roms/manifest.txt" manifest_lines)
set(required_manifest
  "tests/roms/dabg.nes\thttps://github.com/NovaSquirrel/DABG/tree/5ecc60b6af3f726851bfeb1c2555388c5953e0c0\t5ecc60b6af3f726851bfeb1c2555388c5953e0c0\tZlib\teca79b9d0b546e96c1bc73099e239539fb9c7773ff10a431bb3a9cd6763208ca"
  "tests/roms/nesteroids.nes\thttps://github.com/battlelinegames/nesteroids/tree/ae65d78048af68f3e735c49d77a7f37e976a7afe\tae65d78048af68f3e735c49d77a7f37e976a7afe\tMIT\ta30dcfb12c1447dc121619c93b4083d2b83c7405a883ed490c3a59e52ce74f36"
  "tests/roms/rhde.nes\thttps://github.com/pinobatch/rhde-nes/tree/71b18aeb54fefa16373cb3ad2996a2aff90181b5\t71b18aeb54fefa16373cb3ad2996a2aff90181b5\tFSFAP\tb2c4748a5b3651e393572046daf126213653b58b55472cdfdf4b863834dd0241")
foreach(required IN LISTS required_manifest)
  if(NOT required IN_LIST manifest_lines)
    string(REGEX REPLACE "\t.*$" "" missing_path "${required}")
    message(FATAL_ERROR "write_hashes: manifest provenance missing or changed for ${missing_path}")
  endif()
endforeach()

# RHDE's iNES header declares zero CHR-ROM banks; its pinned README says it
# expects 8 KiB CHR RAM. Its native render hashes exercise the decompressed
# CHR data written at startup and subsequently fetched by the PPU.
file(READ "${source_dir}/tests/roms/rhde.nes" rhde_chr_count OFFSET 5 LIMIT 1 HEX)
if(NOT rhde_chr_count STREQUAL "00")
  message(FATAL_ERROR "write_hashes: RHDE byte 5 must declare CHR RAM (got ${rhde_chr_count})")
endif()

set(output_lines)
set(frame_args --hash-frame 1 --hash-frame 30 --hash-frame 60 --hash-frame 120 --hash-frame 180)
foreach(game IN ITEMS nesteroids dabg rhde)
  set(rom "${source_dir}/tests/roms/${game}.nes")
  execute_process(
    COMMAND "${runner}" --frames 180 --rom "${rom}" ${frame_args}
    OUTPUT_VARIABLE hashes
    ERROR_VARIABLE errors
    RESULT_VARIABLE rc)
  if(NOT rc EQUAL 0)
    message(FATAL_ERROR "write_hashes: ${game} boot run exited with ${rc}: ${errors}")
  endif()
  string(REPLACE "\r" "" hashes "${hashes}")
  string(REPLACE "\n" ";" hash_lines "${hashes}")
  foreach(line IN LISTS hash_lines)
    if(line MATCHES "^frame (1|30|60|120|180) ")
      list(APPEND output_lines "${game}/boot/${line}")
    endif()
  endforeach()
endforeach()

set(movie_dir "${BUILD}/game-movies")
file(MAKE_DIRECTORY "${movie_dir}")
foreach(port IN ITEMS p0 p1 both)
  set(movie "${movie_dir}/dabg-${port}.nmovie")
  execute_process(COMMAND "${MOVIE_WRITER}" "${movie}" "${port}"
    RESULT_VARIABLE rc ERROR_VARIABLE errors)
  if(NOT rc EQUAL 0 OR NOT EXISTS "${movie}")
    message(FATAL_ERROR "write_hashes: could not create DABG ${port} input script: ${errors}")
  endif()
  execute_process(
    COMMAND "${runner}" --movie "${movie}" --rom "${source_dir}/tests/roms/dabg.nes"
    OUTPUT_VARIABLE hashes ERROR_VARIABLE errors RESULT_VARIABLE rc)
  if(NOT rc EQUAL 0)
    message(FATAL_ERROR "write_hashes: DABG ${port} replay exited with ${rc}: ${errors}")
  endif()
  string(REPLACE "\r" "" hashes "${hashes}")
  string(REPLACE "\n" ";" hash_lines "${hashes}")
  foreach(line IN LISTS hash_lines)
    if(line MATCHES "^frame (1|30|60|120|180) ")
      list(APPEND output_lines "dabg/${port}/${line}")
    endif()
  endforeach()
endforeach()

list(SORT output_lines)
list(JOIN output_lines "\n" hashes)
file(CONFIGURE OUTPUT "${OUT}" CONTENT "@hashes@" @ONLY NEWLINE_STYLE LF)
message(STATUS "write_hashes: wrote sorted game inventory to ${OUT}")
