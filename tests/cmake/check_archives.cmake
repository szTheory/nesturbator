# Checks the release archives that `cmake --workflow --preset ci` writes:
# exactly three, named nesturbator-VERSION-COMPONENT-OS-ARCH.zip with one
# version, OS and architecture, each holding exactly its expected files at the
# zip root.
#
#   cmake -DDIR=<build dir> -P tests/cmake/check_archives.cmake
#   cmake -DPACKAGES=<zip dir> -P tests/cmake/check_archives.cmake
#
# PACKAGES defaults to ${DIR}/packages. VERSION defaults to version.txt at the
# repository root. Only zips of that version count, so CPack's staging
# directory and zips left from an earlier version are ignored.

cmake_minimum_required(VERSION 3.25)

include("${CMAKE_CURRENT_LIST_DIR}/info_claims.cmake")
set(NESTURBATOR_ARCHIVE_SOURCE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../..")
file(REAL_PATH "${NESTURBATOR_ARCHIVE_SOURCE_ROOT}" NESTURBATOR_ARCHIVE_SOURCE_ROOT)

# Read little-endian ZIP integer fields from a hex string. Byte offsets keep
# the callers aligned with the ZIP record layout and make bounds explicit.
function(nesturbator_zip_integer hex byte_offset byte_count out_value)
  string(LENGTH "${hex}" hex_length)
  math(EXPR start "${byte_offset} * 2")
  math(EXPR length "${byte_count} * 2")
  math(EXPR end "${start} + ${length}")
  if(end GREATER hex_length)
    set(${out_value} "" PARENT_SCOPE)
    return()
  endif()

  set(value_hex "")
  math(EXPR byte_index "${byte_count} - 1")
  while(byte_index GREATER_EQUAL 0)
    math(EXPR nibble_offset "${start} + ${byte_index} * 2")
    string(SUBSTRING "${hex}" ${nibble_offset} 2 byte_hex)
    set(value_hex "${value_hex}${byte_hex}")
    math(EXPR byte_index "${byte_index} - 1")
  endwhile()
  math(EXPR value "0x${value_hex}")
  set(${out_value} "${value}" PARENT_SCOPE)
endfunction()

# CMake 3.25's archive listing API is line-oriented, so it cannot distinguish
# a filename containing LF from two filenames. Validate names in the ZIP
# central directory before using that listing. CPack emits single-disk,
# non-ZIP64 ZIPs; reject other layouts rather than interpreting them loosely.
function(nesturbator_archive_inventory_check zip out_error out_member_hexes)
  set(${out_error} "" PARENT_SCOPE)
  set(${out_member_hexes} "" PARENT_SCOPE)
  if(NOT EXISTS "${zip}")
    set(${out_error} "archive inventory check is missing ${zip}" PARENT_SCOPE)
    return()
  endif()

  file(SIZE "${zip}" zip_size)
  if(zip_size LESS 22)
    set(${out_error} "archive is too short to contain a ZIP end record" PARENT_SCOPE)
    return()
  endif()

  # EOCD is at most 22 fixed bytes plus a 65535-byte comment from EOF.
  if(zip_size GREATER 65557)
    set(tail_size 65557)
  else()
    set(tail_size "${zip_size}")
  endif()
  math(EXPR tail_offset "${zip_size} - ${tail_size}")
  file(READ "${zip}" tail_hex OFFSET ${tail_offset} LIMIT ${tail_size} HEX)
  math(EXPR tail_last_candidate "${tail_size} - 22")
  math(EXPR tail_first_candidate "${tail_size} - 65557")
  if(tail_first_candidate LESS 0)
    set(tail_first_candidate 0)
  endif()

  # Count every signature whose declared comment ends exactly at EOF. This
  # rejects a second EOCD-shaped sequence embedded in an archive comment.
  set(eocd_candidates 0)
  set(eocd_offset_in_tail "")
  set(candidate ${tail_last_candidate})
  while(candidate GREATER_EQUAL tail_first_candidate)
    math(EXPR signature_offset "${candidate} * 2")
    string(SUBSTRING "${tail_hex}" ${signature_offset} 8 signature)
    if(signature STREQUAL "504b0506")
      math(EXPR comment_offset "${candidate} + 20")
      nesturbator_zip_integer("${tail_hex}" ${comment_offset} 2 comment_size)
      math(EXPR candidate_end "${candidate} + 22 + ${comment_size}")
      if(candidate_end EQUAL tail_size)
        math(EXPR eocd_candidates "${eocd_candidates} + 1")
        set(eocd_offset_in_tail ${candidate})
      endif()
    endif()
    math(EXPR candidate "${candidate} - 1")
  endwhile()
  if(NOT eocd_candidates EQUAL 1)
    set(${out_error}
      "archive has ${eocd_candidates} valid-looking ZIP end records; expected one"
      PARENT_SCOPE)
    return()
  endif()

  math(EXPR eocd_absolute "${tail_offset} + ${eocd_offset_in_tail}")
  math(EXPR field_offset "${eocd_offset_in_tail} + 4")
  nesturbator_zip_integer("${tail_hex}" ${field_offset} 2 disk_number)
  math(EXPR field_offset "${eocd_offset_in_tail} + 6")
  nesturbator_zip_integer("${tail_hex}" ${field_offset} 2 directory_disk)
  math(EXPR field_offset "${eocd_offset_in_tail} + 8")
  nesturbator_zip_integer("${tail_hex}" ${field_offset} 2 disk_entries)
  math(EXPR field_offset "${eocd_offset_in_tail} + 10")
  nesturbator_zip_integer("${tail_hex}" ${field_offset} 2 total_entries)
  math(EXPR field_offset "${eocd_offset_in_tail} + 12")
  nesturbator_zip_integer("${tail_hex}" ${field_offset} 4 directory_size)
  math(EXPR field_offset "${eocd_offset_in_tail} + 16")
  nesturbator_zip_integer("${tail_hex}" ${field_offset} 4 directory_offset)

  if(NOT disk_number EQUAL 0 OR NOT directory_disk EQUAL 0 OR
      NOT disk_entries EQUAL total_entries)
    set(${out_error} "multi-disk ZIP archives are not supported" PARENT_SCOPE)
    return()
  endif()
  if(total_entries EQUAL 65535 OR directory_size EQUAL 4294967295 OR
      directory_offset EQUAL 4294967295)
    set(${out_error} "ZIP64 archives are not supported by the package inventory check"
      PARENT_SCOPE)
    return()
  endif()
  math(EXPR directory_end "${directory_offset} + ${directory_size}")
  if(NOT directory_end EQUAL eocd_absolute OR directory_end GREATER zip_size)
    set(${out_error} "ZIP central-directory bounds do not meet the end record" PARENT_SCOPE)
    return()
  endif()

  set(cursor 0)
  set(member_index 0)
  set(member_hexes "\n")
  while(member_index LESS total_entries)
    math(EXPR fixed_end "${cursor} + 46")
    if(fixed_end GREATER directory_size)
      set(${out_error} "ZIP central directory ends inside a member header" PARENT_SCOPE)
      return()
    endif()
    math(EXPR record_offset "${directory_offset} + ${cursor}")
    file(READ "${zip}" record_hex OFFSET ${record_offset} LIMIT 46 HEX)
    string(LENGTH "${record_hex}" record_hex_length)
    if(NOT record_hex_length EQUAL 92 OR NOT record_hex MATCHES "^504b0102")
      set(${out_error} "ZIP central directory contains an invalid member header" PARENT_SCOPE)
      return()
    endif()

    nesturbator_zip_integer("${record_hex}" 20 4 compressed_size)
    nesturbator_zip_integer("${record_hex}" 24 4 uncompressed_size)
    nesturbator_zip_integer("${record_hex}" 28 2 name_size)
    nesturbator_zip_integer("${record_hex}" 30 2 extra_size)
    nesturbator_zip_integer("${record_hex}" 32 2 member_comment_size)
    nesturbator_zip_integer("${record_hex}" 34 2 member_disk)
    nesturbator_zip_integer("${record_hex}" 42 4 local_offset)
    if(compressed_size EQUAL 4294967295 OR uncompressed_size EQUAL 4294967295 OR
        local_offset EQUAL 4294967295 OR member_disk GREATER 0)
      set(${out_error} "ZIP64 or multi-disk member records are not supported" PARENT_SCOPE)
      return()
    endif()
    if(name_size EQUAL 0 OR local_offset GREATER_EQUAL zip_size)
      set(${out_error} "ZIP member has an empty name or invalid local-header offset" PARENT_SCOPE)
      return()
    endif()
    math(EXPR record_size "46 + ${name_size} + ${extra_size} + ${member_comment_size}")
    math(EXPR next_cursor "${cursor} + ${record_size}")
    if(next_cursor GREATER directory_size)
      set(${out_error} "ZIP member name, extra data or comment exceeds the central directory"
        PARENT_SCOPE)
      return()
    endif()
    math(EXPR name_offset "${record_offset} + 46")
    file(READ "${zip}" member_hex OFFSET ${name_offset} LIMIT ${name_size} HEX)
    math(EXPR member_hex_size "${name_size} * 2")
    string(LENGTH "${member_hex}" actual_member_hex_size)
    if(NOT actual_member_hex_size EQUAL member_hex_size)
      set(${out_error} "ZIP member name is truncated in the central directory" PARENT_SCOPE)
      return()
    endif()

    set(name_byte 0)
    while(name_byte LESS name_size)
      math(EXPR name_byte_offset "${name_byte} * 2")
      string(SUBSTRING "${member_hex}" ${name_byte_offset} 2 byte_hex)
      if(byte_hex MATCHES "^(0[0-9a-f]|1[0-9a-f]|7f)$")
        set(${out_error}
          "archive member name contains an ASCII control byte (name hex ${member_hex})"
          PARENT_SCOPE)
        return()
      endif()
      math(EXPR name_byte "${name_byte} + 1")
    endwhile()
    string(FIND "${member_hexes}" "\n${member_hex}\n" duplicate_index)
    if(NOT duplicate_index LESS 0)
      set(${out_error}
        "archive contains a duplicate member name (name hex ${member_hex})"
        PARENT_SCOPE)
      return()
    endif()
    string(APPEND member_hexes "${member_hex}\n")
    set(cursor ${next_cursor})
    math(EXPR member_index "${member_index} + 1")
  endwhile()

  if(NOT cursor EQUAL directory_size)
    set(${out_error} "ZIP central-directory member count and byte length disagree" PARENT_SCOPE)
    return()
  endif()
  set(${out_member_hexes} "${member_hexes}" PARENT_SCOPE)
endfunction()

function(nesturbator_archive_info_check zip source_info readme header work out_error)
  set(${out_error} "" PARENT_SCOPE)
  foreach(path IN ITEMS "${zip}" "${source_info}" "${readme}" "${header}" "${work}")
    if(path STREQUAL "")
      set(${out_error} "archive metadata check requires nonempty paths" PARENT_SCOPE)
      return()
    endif()
  endforeach()
  if(NOT EXISTS "${zip}" OR NOT EXISTS "${source_info}" OR NOT EXISTS "${readme}" OR
      NOT EXISTS "${header}")
    set(${out_error} "archive metadata check is missing an input file" PARENT_SCOPE)
    return()
  endif()

  nesturbator_archive_inventory_check("${zip}" inventory_error member_hexes)
  if(inventory_error)
    set(${out_error} "archive inventory check failed: ${inventory_error}" PARENT_SCOPE)
    return()
  endif()

  execute_process(COMMAND "${CMAKE_COMMAND}" -E tar tf "${zip}"
    OUTPUT_VARIABLE listing ERROR_VARIABLE list_err RESULT_VARIABLE list_rc)
  if(NOT list_rc EQUAL 0)
    set(${out_error} "cannot list archive: ${list_err}" PARENT_SCOPE)
    return()
  endif()
  string(REPLACE "\r" "" listing "${listing}")
  set(remaining_listing "\n${listing}\n")
  set(info_entry "\ninfo/nesturbator_libretro.info\n")
  set(info_entries 0)
  string(LENGTH "${info_entry}" info_entry_length)
  while(TRUE)
    string(FIND "${remaining_listing}" "${info_entry}" info_entry_index)
    if(info_entry_index LESS 0)
      break()
    endif()
    math(EXPR info_entries "${info_entries} + 1")
    math(EXPR info_after_index "${info_entry_index} + ${info_entry_length}")
    string(SUBSTRING "${remaining_listing}" 0 ${info_entry_index} info_before)
    string(SUBSTRING "${remaining_listing}" ${info_after_index} -1 info_after)
    set(remaining_listing "${info_before}\n${info_after}")
  endwhile()
  if(info_entries EQUAL 0)
    set(${out_error} "archive does not contain info/nesturbator_libretro.info" PARENT_SCOPE)
    return()
  endif()
  if(NOT info_entries EQUAL 1)
    set(${out_error} "archive contains ${info_entries} copies of info/nesturbator_libretro.info"
      PARENT_SCOPE)
    return()
  endif()

  file(REMOVE_RECURSE "${work}")
  file(MAKE_DIRECTORY "${work}")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E tar xf "${zip}"
      "info/nesturbator_libretro.info"
    WORKING_DIRECTORY "${work}" OUTPUT_VARIABLE extract_out ERROR_VARIABLE extract_err
    RESULT_VARIABLE extract_rc)
  if(NOT extract_rc EQUAL 0)
    file(REMOVE_RECURSE "${work}")
    set(${out_error} "cannot extract archive metadata: ${extract_err}${extract_out}" PARENT_SCOPE)
    return()
  endif()
  set(archive_info "${work}/info/nesturbator_libretro.info")
  if(NOT EXISTS "${archive_info}")
    file(REMOVE_RECURSE "${work}")
    set(${out_error} "archive metadata extraction did not create the expected file" PARENT_SCOPE)
    return()
  endif()

  file(SIZE "${source_info}" source_size)
  file(SIZE "${archive_info}" archive_size)
  if(source_size EQUAL 0 OR archive_size EQUAL 0)
    file(REMOVE_RECURSE "${work}")
    set(${out_error} "source and archived metadata must be nonempty" PARENT_SCOPE)
    return()
  endif()
  file(SHA256 "${source_info}" source_sha)
  file(SHA256 "${archive_info}" archive_sha)
  if(NOT source_size EQUAL archive_size OR NOT source_sha STREQUAL archive_sha)
    file(REMOVE_RECURSE "${work}")
    set(${out_error} "archive metadata differs from the current source .info" PARENT_SCOPE)
    return()
  endif()
  nesturbator_info_support_check("${archive_info}" "${readme}" "${header}" support_error)
  if(support_error)
    file(REMOVE_RECURSE "${work}")
    set(${out_error} "archive support check failed: ${support_error}" PARENT_SCOPE)
    return()
  endif()
  file(REMOVE_RECURSE "${work}")
endfunction()

function(nesturbator_archive_contents_check zip component out_error)
  set(${out_error} "" PARENT_SCOPE)
  nesturbator_archive_inventory_check("${zip}" inventory_error member_hexes)
  if(inventory_error)
    set(${out_error} "archive inventory check failed: ${inventory_error}" PARENT_SCOPE)
    return()
  endif()
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar tf "${zip}"
    OUTPUT_VARIABLE listing ERROR_VARIABLE list_err RESULT_VARIABLE rc)
  if(NOT rc EQUAL 0)
    set(${out_error} "cannot list archive: ${list_err}" PARENT_SCOPE)
    return()
  endif()
  string(REPLACE "\r" "" listing "${listing}")
  if(listing MATCHES ";")
    set(${out_error} "archive contains a literal semicolon in an entry name" PARENT_SCOPE)
    return()
  endif()
  string(REPLACE "\n" ";" entries "${listing}")
  set(files "")
  foreach(entry IN LISTS entries)
    if(entry STREQUAL "")
      continue()
    elseif(entry MATCHES "/$")
      set(known_directory FALSE)
      foreach(re IN LISTS want_${component}_directories)
        if(entry MATCHES "${re}")
          set(known_directory TRUE)
        endif()
      endforeach()
      if(NOT known_directory)
        set(${out_error} "archive holds an unexpected or aliased directory ${entry}" PARENT_SCOPE)
        return()
      endif()
    else()
      list(APPEND files "${entry}")
    endif()
  endforeach()

  foreach(re IN LISTS want_${component})
    set(hits 0)
    foreach(f IN LISTS files)
      if(f MATCHES "${re}")
        math(EXPR hits "${hits} + 1")
      endif()
    endforeach()
    if(hits EQUAL 0)
      set(${out_error} "archive lacks an entry matching ${re}" PARENT_SCOPE)
      return()
    elseif(NOT hits EQUAL 1)
      set(${out_error}
        "archive has ${hits} entries matching ${re}; expected exactly one"
        PARENT_SCOPE)
      return()
    endif()
  endforeach()
  foreach(f IN LISTS files)
    set(known FALSE)
    foreach(re IN LISTS want_${component})
      if(f MATCHES "${re}")
        set(known TRUE)
      endif()
    endforeach()
    if(NOT known)
      set(${out_error} "archive holds an extra file ${f}" PARENT_SCOPE)
      return()
    endif()
  endforeach()
endfunction()

# Every file and directory in each archive must match its component allowlist.
# A Linux install may use lib64.
set(want_libretro
  "^LICENSE$"
  "^THIRD-PARTY-NOTICES\\.md$"
  "^cores/nesturbator_libretro\\.(dylib|so|dll)$"
  "^info/nesturbator_libretro\\.info$")
set(want_runner
  "^LICENSE$"
  "^bin/nesturbator-run(\\.exe)?$")
set(want_library
  "^LICENSE$"
  "^include/nesturbator\\.h$"
  "^lib(64)?/(libnesturbator\\.a|nesturbator\\.lib)$"
  "^lib(64)?/cmake/nesturbator/nesturbatorConfig\\.cmake$"
  "^lib(64)?/cmake/nesturbator/nesturbatorConfigVersion\\.cmake$"
  "^lib(64)?/cmake/nesturbator/nesturbatorTargets\\.cmake$"
  "^lib(64)?/cmake/nesturbator/nesturbatorTargets-[a-z]+\\.cmake$"
  "^lib(64)?/pkgconfig/nesturbator\\.pc$")
set(want_libretro_directories "^cores/$" "^info/$")
set(want_runner_directories "^bin/$")
set(want_library_directories
  "^include/$"
  "^lib(64)?/$"
  "^lib(64)?/cmake/$"
  "^lib(64)?/cmake/nesturbator/$"
  "^lib(64)?/pkgconfig/$")

if(SELFTEST)
  foreach(required WORK SOURCE_ROOT)
    if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
      message(FATAL_ERROR "check_archives self-test: -D${required}=<path> is required")
    endif()
  endforeach()
  cmake_path(ABSOLUTE_PATH WORK NORMALIZE OUTPUT_VARIABLE selftest_work)
  cmake_path(ABSOLUTE_PATH SOURCE_ROOT NORMALIZE OUTPUT_VARIABLE source_root)
  set(source_info "${source_root}/libretro/nesturbator_libretro.info")
  set(readme "${source_root}/README.md")
  set(header "${source_root}/include/nesturbator.h")
  foreach(path IN ITEMS "${source_info}" "${readme}" "${header}")
    if(NOT EXISTS "${path}")
      message(FATAL_ERROR "check_archives self-test: missing current input ${path}")
    endif()
  endforeach()
  file(REMOVE_RECURSE "${selftest_work}")
  file(MAKE_DIRECTORY "${selftest_work}")
  file(READ "${source_info}" current_info)
  string(REPLACE "battery saves." "battery saves (stale)." stale_info "${current_info}")
  if(stale_info STREQUAL current_info)
    message(FATAL_ERROR "check_archives self-test: could not create a stale metadata control")
  endif()

  function(create_archive_control name info_file)
    set(root "${selftest_work}/${name}-root")
    file(MAKE_DIRECTORY "${root}/info")
    file(WRITE "${root}/LICENSE" "synthetic archive control\n")
    if(NOT info_file STREQUAL "")
      file(COPY_FILE "${info_file}" "${root}/info/nesturbator_libretro.info")
      file(ARCHIVE_CREATE OUTPUT "${selftest_work}/${name}.zip"
        PATHS LICENSE info/nesturbator_libretro.info FORMAT zip WORKING_DIRECTORY "${root}")
    else()
      file(ARCHIVE_CREATE OUTPUT "${selftest_work}/${name}.zip"
        PATHS LICENSE FORMAT zip WORKING_DIRECTORY "${root}")
    endif()
  endfunction()

  create_archive_control(absent "")
  nesturbator_archive_info_check("${selftest_work}/absent.zip" "${source_info}" "${readme}"
    "${header}" "${selftest_work}/extract-absent" error)
  if(NOT error STREQUAL "archive does not contain info/nesturbator_libretro.info")
    message(FATAL_ERROR "check_archives self-test: absent metadata failed for the wrong reason: ${error}")
  endif()

  set(stale_info_path "${selftest_work}/stale.info")
  file(WRITE "${stale_info_path}" "${stale_info}")
  create_archive_control(stale "${stale_info_path}")
  nesturbator_archive_info_check("${selftest_work}/stale.zip" "${source_info}" "${readme}"
    "${header}" "${selftest_work}/extract-stale" error)
  if(NOT error STREQUAL "archive metadata differs from the current source .info")
    message(FATAL_ERROR "check_archives self-test: stale metadata failed for the wrong reason: ${error}")
  endif()

  create_archive_control(current "${source_info}")
  nesturbator_archive_info_check("${selftest_work}/current.zip" "${source_info}" "${readme}"
    "${header}" "${selftest_work}/extract-current" error)
  if(error)
    message(FATAL_ERROR "check_archives self-test: current metadata rejected: ${error}")
  endif()

  set(semicolon_root "${selftest_work}/semicolon-root")
  file(MAKE_DIRECTORY "${semicolon_root}/cores" "${semicolon_root}/info")
  file(WRITE "${semicolon_root}/LICENSE;THIRD-PARTY-NOTICES.md" "combined allowed names\n")
  file(WRITE "${semicolon_root}/cores/nesturbator_libretro.so" "synthetic core\n")
  file(COPY_FILE "${source_info}" "${semicolon_root}/info/nesturbator_libretro.info")
  file(ARCHIVE_CREATE OUTPUT "${selftest_work}/semicolon.zip"
    PATHS "LICENSE;THIRD-PARTY-NOTICES.md" "cores/nesturbator_libretro.so"
      "info/nesturbator_libretro.info"
    FORMAT zip WORKING_DIRECTORY "${semicolon_root}")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E tar tf "${selftest_work}/semicolon.zip"
    OUTPUT_VARIABLE semicolon_listing RESULT_VARIABLE semicolon_list_rc)
  if(NOT semicolon_list_rc EQUAL 0 OR
      NOT semicolon_listing MATCHES "LICENSE;THIRD-PARTY-NOTICES\\.md" OR
      NOT semicolon_listing MATCHES "info/nesturbator_libretro\\.info")
    message(FATAL_ERROR
      "check_archives self-test: synthetic semicolon archive lacks its combined path or valid metadata: ${semicolon_listing}")
  endif()
  nesturbator_archive_info_check("${selftest_work}/semicolon.zip" "${source_info}" "${readme}"
    "${header}" "${selftest_work}/extract-semicolon" error)
  if(error)
    message(FATAL_ERROR "check_archives self-test: valid metadata control failed: ${error}")
  endif()
  nesturbator_archive_contents_check("${selftest_work}/semicolon.zip" libretro error)
  if(NOT error STREQUAL "archive contains a literal semicolon in an entry name")
    message(FATAL_ERROR
      "check_archives self-test: semicolon allowlist control failed for the wrong reason: ${error}")
  endif()

  string(ASCII 10 line_feed)
  set(newline_root "${selftest_work}/newline-root")
  file(MAKE_DIRECTORY "${newline_root}/LICENSE${line_feed}bin")
  file(WRITE "${newline_root}/LICENSE${line_feed}bin/nesturbator-run" "synthetic runner\n")
  file(ARCHIVE_CREATE OUTPUT "${selftest_work}/newline.zip"
    PATHS "LICENSE${line_feed}bin/nesturbator-run" FORMAT zip
    WORKING_DIRECTORY "${newline_root}")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E tar tf "${selftest_work}/newline.zip"
    OUTPUT_VARIABLE newline_listing RESULT_VARIABLE newline_list_rc)
  if(NOT newline_list_rc EQUAL 0 OR
      NOT newline_listing STREQUAL "LICENSE${line_feed}bin/nesturbator-run${line_feed}")
    message(FATAL_ERROR
      "check_archives self-test: synthetic newline member did not produce the split listing: ${newline_listing}")
  endif()
  nesturbator_archive_contents_check("${selftest_work}/newline.zip" runner error)
  if(NOT error MATCHES "archive member name contains an ASCII control byte")
    message(FATAL_ERROR "check_archives self-test: newline member was accepted or failed for the wrong reason: ${error}")
  endif()

  set(duplicate_root "${selftest_work}/duplicate-root")
  file(MAKE_DIRECTORY "${duplicate_root}/bin")
  file(WRITE "${duplicate_root}/LICENSE" "synthetic license\n")
  file(WRITE "${duplicate_root}/bin/nesturbator-run" "synthetic runner\n")
  file(ARCHIVE_CREATE OUTPUT "${selftest_work}/duplicate.zip"
    PATHS LICENSE LICENSE bin/nesturbator-run FORMAT zip WORKING_DIRECTORY "${duplicate_root}")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E tar tf "${selftest_work}/duplicate.zip"
    OUTPUT_VARIABLE duplicate_listing RESULT_VARIABLE duplicate_list_rc)
  if(NOT duplicate_list_rc EQUAL 0 OR
      NOT duplicate_listing STREQUAL "LICENSE${line_feed}LICENSE${line_feed}bin/nesturbator-run${line_feed}")
    message(FATAL_ERROR
      "check_archives self-test: duplicate archive fixture was not preserved: ${duplicate_listing}")
  endif()
  nesturbator_archive_contents_check("${selftest_work}/duplicate.zip" runner error)
  if(NOT error MATCHES "archive contains a duplicate member name")
    message(FATAL_ERROR "check_archives self-test: duplicate LICENSE member was accepted or failed for the wrong reason: ${error}")
  endif()

  set(duplicate_directory_root "${selftest_work}/duplicate-directory-root")
  file(MAKE_DIRECTORY "${duplicate_directory_root}/bin")
  file(WRITE "${duplicate_directory_root}/LICENSE" "synthetic license\n")
  file(WRITE "${duplicate_directory_root}/bin/nesturbator-run" "synthetic runner\n")
  file(ARCHIVE_CREATE OUTPUT "${selftest_work}/duplicate-directory.zip"
    PATHS LICENSE bin bin FORMAT zip WORKING_DIRECTORY "${duplicate_directory_root}")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E tar tf "${selftest_work}/duplicate-directory.zip"
    OUTPUT_VARIABLE duplicate_directory_listing RESULT_VARIABLE duplicate_directory_list_rc)
  if(NOT duplicate_directory_list_rc EQUAL 0)
    message(FATAL_ERROR
      "check_archives self-test: could not list duplicate-directory fixture: ${duplicate_directory_listing}")
  endif()
  set(remaining_directory_listing "\n${duplicate_directory_listing}")
  set(directory_entry "\nbin/\n")
  set(directory_entry_count 0)
  string(LENGTH "${directory_entry}" directory_entry_length)
  while(TRUE)
    string(FIND "${remaining_directory_listing}" "${directory_entry}" directory_entry_index)
    if(directory_entry_index LESS 0)
      break()
    endif()
    math(EXPR directory_entry_count "${directory_entry_count} + 1")
    math(EXPR after_directory_index "${directory_entry_index} + ${directory_entry_length}")
    string(SUBSTRING "${remaining_directory_listing}" 0 ${directory_entry_index} before_directory)
    string(SUBSTRING "${remaining_directory_listing}" ${after_directory_index} -1 after_directory)
    set(remaining_directory_listing "${before_directory}\n${after_directory}")
  endwhile()
  if(NOT directory_entry_count EQUAL 2)
    message(FATAL_ERROR
      "check_archives self-test: duplicate directory fixture was not preserved: ${duplicate_directory_listing}")
  endif()
  nesturbator_archive_contents_check("${selftest_work}/duplicate-directory.zip" runner error)
  if(NOT error MATCHES "archive contains a duplicate member name")
    message(FATAL_ERROR "check_archives self-test: duplicate directory member was accepted or failed for the wrong reason: ${error}")
  endif()

  set(multiple_match_root "${selftest_work}/multiple-match-root")
  file(MAKE_DIRECTORY "${multiple_match_root}/bin")
  file(WRITE "${multiple_match_root}/LICENSE" "synthetic license\n")
  file(WRITE "${multiple_match_root}/bin/nesturbator-run" "synthetic runner\n")
  file(WRITE "${multiple_match_root}/bin/nesturbator-run.exe" "synthetic runner\n")
  file(ARCHIVE_CREATE OUTPUT "${selftest_work}/multiple-match.zip"
    PATHS LICENSE bin/nesturbator-run bin/nesturbator-run.exe FORMAT zip
    WORKING_DIRECTORY "${multiple_match_root}")
  nesturbator_archive_contents_check("${selftest_work}/multiple-match.zip" runner error)
  if(NOT error MATCHES "expected exactly one")
    message(FATAL_ERROR
      "check_archives self-test: multiple expected-pattern matches were accepted or failed for the wrong reason: ${error}")
  endif()

  message(STATUS
    "check_archives self-test: absent, stale, current, semicolon, newline, duplicate and multiple-match controls passed")
  return()
endif()

if(NOT DEFINED PACKAGES)
  if(NOT DEFINED DIR)
    message(FATAL_ERROR "check_archives: pass -DDIR=<build dir> or -DPACKAGES=<zip dir>")
  endif()
  set(PACKAGES "${DIR}/packages")
endif()
if(NOT DEFINED VERSION)
  file(READ "${CMAKE_CURRENT_LIST_DIR}/../../version.txt" VERSION)
  string(STRIP "${VERSION}" VERSION)
endif()

file(GLOB zips LIST_DIRECTORIES false "${PACKAGES}/nesturbator-${VERSION}-*.zip")
list(LENGTH zips count)
if(NOT count EQUAL 3)
  message(FATAL_ERROR "check_archives: ${count} archives of version ${VERSION} in ${PACKAGES}, expected 3: ${zips}")
endif()

set(name_re "^nesturbator-([0-9]+\\.[0-9]+\\.[0-9]+)-(library|runner|libretro)-(linux|macos|windows)-(x64|arm64)\\.zip$")
set(platform "")
set(components "")
foreach(zip IN LISTS zips)
  get_filename_component(name "${zip}" NAME)
  if(NOT name MATCHES "${name_re}")
    message(FATAL_ERROR "check_archives: unexpected archive name ${name}")
  endif()
  set(component "${CMAKE_MATCH_2}")
  set(this_platform "${CMAKE_MATCH_1}-${CMAKE_MATCH_3}-${CMAKE_MATCH_4}")
  if(platform STREQUAL "")
    set(platform "${this_platform}")
  elseif(NOT platform STREQUAL this_platform)
    message(FATAL_ERROR "check_archives: ${name} differs in version, OS or architecture from ${platform}")
  endif()
  list(APPEND components ${component})

  nesturbator_archive_contents_check("${zip}" "${component}" archive_error)
  if(archive_error)
    message(FATAL_ERROR "check_archives: ${name}: ${archive_error}")
  endif()
  if(component STREQUAL "libretro")
    string(RANDOM LENGTH 8 ALPHABET 0123456789abcdef archive_nonce)
    set(archive_work "${CMAKE_CURRENT_BINARY_DIR}/.nesturbator-archive-info-${archive_nonce}")
    nesturbator_archive_info_check("${zip}"
      "${NESTURBATOR_ARCHIVE_SOURCE_ROOT}/libretro/nesturbator_libretro.info"
      "${NESTURBATOR_ARCHIVE_SOURCE_ROOT}/README.md"
      "${NESTURBATOR_ARCHIVE_SOURCE_ROOT}/include/nesturbator.h"
      "${archive_work}" archive_error)
    if(archive_error)
      message(FATAL_ERROR "check_archives: ${name}: ${archive_error}")
    endif()
    file(REMOVE_RECURSE "${archive_work}")
  endif()
  message(STATUS "check_archives: ${name} ok")
endforeach()

list(REMOVE_DUPLICATES components)
list(LENGTH components distinct)
if(NOT distinct EQUAL 3)
  message(FATAL_ERROR "check_archives: components ${components}, expected library, runner and libretro")
endif()
