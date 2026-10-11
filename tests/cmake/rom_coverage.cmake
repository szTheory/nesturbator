# Drift guard (D-17): every committed ROM is hashed or exempt. A file in
# tests/roms/*.nes must be a game in NESTURBATOR_HASH_GAMES or listed in
# NESTURBATOR_ROM_HASH_EXEMPT; a file in tests/roms/hm/*.nes must be the file
# of a committed (not derived/) NESTURBATOR_HOLYMAPPEREL_ROMS entry. Every
# listed game and Holy Mapperel file must exist, and an empty glob or an empty
# list fails, because a guard over nothing passes for any tree. The exemptions
# and their reasons (D-14) are in hash_keys.cmake; fuzz corpus seeds are outside
# these globs and run under fuzz.regress.
#
#   cmake -DROOT=<repo> -P rom_coverage.cmake
#   cmake -DSELFTEST=ON -DWORK=<dir> -P rom_coverage.cmake
#
# SELFTEST builds scratch trees and passes only if each gets its verdict.

cmake_minimum_required(VERSION 3.25)

include("${CMAKE_CURRENT_LIST_DIR}/hash_keys.cmake")

# Sets <out> to the problems found, one per list item; empty means the tree is
# covered. games, hm_roms and exempt are the three lists, each passed quoted.
function(rom_coverage_check root games hm_roms exempt out)
  set(problems)
  if("${games}" STREQUAL "")
    list(APPEND problems "NESTURBATOR_HASH_GAMES is empty")
  endif()
  if("${hm_roms}" STREQUAL "")
    list(APPEND problems "NESTURBATOR_HOLYMAPPEREL_ROMS is empty")
  endif()

  set(game_files)
  foreach(entry IN LISTS games)
    string(REPLACE ":" ";" parts "${entry}")
    list(GET parts 1 file)
    list(APPEND game_files "${file}")
    if(NOT EXISTS "${root}/tests/roms/${file}")
      list(APPEND problems "tests/roms/${file}: listed in NESTURBATOR_HASH_GAMES but missing")
    endif()
  endforeach()

  set(hm_files)
  foreach(entry IN LISTS hm_roms)
    string(REPLACE ":" ";" parts "${entry}")
    list(GET parts 1 file)
    if(file MATCHES "^derived/")
      continue()
    endif()
    list(APPEND hm_files "${file}")
    if(NOT EXISTS "${root}/tests/roms/hm/${file}")
      list(APPEND problems "tests/roms/hm/${file}: listed in NESTURBATOR_HOLYMAPPEREL_ROMS but missing")
    endif()
  endforeach()

  file(GLOB top_roms RELATIVE "${root}/tests/roms" "${root}/tests/roms/*.nes")
  file(GLOB hm_dir_roms RELATIVE "${root}/tests/roms/hm" "${root}/tests/roms/hm/*.nes")
  if("${top_roms}" STREQUAL "")
    list(APPEND problems "tests/roms/*.nes matches no file")
  endif()
  if("${hm_dir_roms}" STREQUAL "")
    list(APPEND problems "tests/roms/hm/*.nes matches no file")
  endif()
  foreach(rom IN LISTS top_roms)
    if(NOT rom IN_LIST game_files AND NOT rom IN_LIST exempt)
      list(APPEND problems "tests/roms/${rom}: neither hashed (NESTURBATOR_HASH_GAMES) nor exempt (NESTURBATOR_ROM_HASH_EXEMPT)")
    endif()
  endforeach()
  foreach(rom IN LISTS hm_dir_roms)
    if(NOT rom IN_LIST hm_files)
      list(APPEND problems "tests/roms/hm/${rom}: not the file of any NESTURBATOR_HOLYMAPPEREL_ROMS entry")
    endif()
  endforeach()
  set(${out} "${problems}" PARENT_SCOPE)
endfunction()

if(SELFTEST)
  if(NOT DEFINED WORK)
    message(FATAL_ERROR "rom_coverage.cmake self-test requires -DWORK=<temporary directory>")
  endif()

  # Builds a scratch tree holding exactly the listed ROMs and exempt files.
  function(rom_coverage_scratch dir)
    file(REMOVE_RECURSE "${dir}")
    file(MAKE_DIRECTORY "${dir}/tests/roms/hm")
    foreach(entry IN LISTS NESTURBATOR_HASH_GAMES)
      string(REPLACE ":" ";" parts "${entry}")
      list(GET parts 1 file)
      file(WRITE "${dir}/tests/roms/${file}" "x")
    endforeach()
    foreach(file IN LISTS NESTURBATOR_ROM_HASH_EXEMPT)
      file(WRITE "${dir}/tests/roms/${file}" "x")
    endforeach()
    foreach(entry IN LISTS NESTURBATOR_HOLYMAPPEREL_ROMS)
      string(REPLACE ":" ";" parts "${entry}")
      list(GET parts 1 file)
      if(NOT file MATCHES "^derived/")
        file(WRITE "${dir}/tests/roms/hm/${file}" "x")
      endif()
    endforeach()
  endfunction()

  # expect_verdict(<case> <root> <games> <hm> <substring>): an empty substring
  # expects a pass; otherwise the problems must name it.
  function(expect_verdict case root games hm_roms substring)
    rom_coverage_check("${root}" "${games}" "${hm_roms}" "${NESTURBATOR_ROM_HASH_EXEMPT}" problems)
    if(substring STREQUAL "")
      if(NOT "${problems}" STREQUAL "")
        message(FATAL_ERROR "self-test ${case}: a covered tree was rejected: ${problems}")
      endif()
    else()
      if("${problems}" STREQUAL "")
        message(FATAL_ERROR "self-test ${case}: the tree was accepted")
      endif()
      string(FIND "${problems}" "${substring}" at)
      if(at EQUAL -1)
        message(FATAL_ERROR "self-test ${case}: the report does not name ${substring}: ${problems}")
      endif()
    endif()
  endfunction()

  set(tree "${WORK}/rom-coverage-selftest")
  rom_coverage_scratch("${tree}")
  expect_verdict(covered "${tree}" "${NESTURBATOR_HASH_GAMES}" "${NESTURBATOR_HOLYMAPPEREL_ROMS}" "")

  file(WRITE "${tree}/tests/roms/unlisted.nes" "x")
  expect_verdict(unlisted_top "${tree}" "${NESTURBATOR_HASH_GAMES}" "${NESTURBATOR_HOLYMAPPEREL_ROMS}" "tests/roms/unlisted.nes")

  rom_coverage_scratch("${tree}")
  file(WRITE "${tree}/tests/roms/hm/Unlisted_HM.nes" "x")
  expect_verdict(unlisted_hm "${tree}" "${NESTURBATOR_HASH_GAMES}" "${NESTURBATOR_HOLYMAPPEREL_ROMS}" "tests/roms/hm/Unlisted_HM.nes")

  rom_coverage_scratch("${tree}")
  file(REMOVE_RECURSE "${tree}/tests/roms/hm")
  file(MAKE_DIRECTORY "${tree}/tests/roms/hm")
  expect_verdict(empty_hm "${tree}" "${NESTURBATOR_HASH_GAMES}" "${NESTURBATOR_HOLYMAPPEREL_ROMS}" "tests/roms/hm/*.nes matches no file")

  rom_coverage_scratch("${tree}")
  list(GET NESTURBATOR_HASH_GAMES 0 first_game)
  string(REPLACE ":" ";" first_parts "${first_game}")
  list(GET first_parts 1 first_file)
  file(REMOVE "${tree}/tests/roms/${first_file}")
  expect_verdict(missing_game "${tree}" "${NESTURBATOR_HASH_GAMES}" "${NESTURBATOR_HOLYMAPPEREL_ROMS}" "tests/roms/${first_file}")

  rom_coverage_scratch("${tree}")
  expect_verdict(empty_games "${tree}" "" "${NESTURBATOR_HOLYMAPPEREL_ROMS}" "NESTURBATOR_HASH_GAMES is empty")
  expect_verdict(empty_hm_list "${tree}" "${NESTURBATOR_HASH_GAMES}" "" "NESTURBATOR_HOLYMAPPEREL_ROMS is empty")

  file(REMOVE_RECURSE "${tree}")
  message(STATUS "rom coverage guard self-test passed")
  return()
endif()

if(NOT DEFINED ROOT)
  message(FATAL_ERROR "rom_coverage.cmake requires -DROOT=<repo>")
endif()
rom_coverage_check("${ROOT}" "${NESTURBATOR_HASH_GAMES}" "${NESTURBATOR_HOLYMAPPEREL_ROMS}" "${NESTURBATOR_ROM_HASH_EXEMPT}" problems)
if(NOT "${problems}" STREQUAL "")
  list(JOIN problems "\n  " text)
  message(FATAL_ERROR "rom coverage check failed:\n  ${text}")
endif()
message(STATUS "every committed ROM is hashed or exempt")
