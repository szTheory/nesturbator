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
    if(NOT entry STREQUAL "" AND NOT entry MATCHES "/$")
      list(APPEND files "${entry}")
    endif()
  endforeach()

  foreach(re IN LISTS want_${component})
    set(hit FALSE)
    foreach(f IN LISTS files)
      if(f MATCHES "${re}")
        set(hit TRUE)
      endif()
    endforeach()
    if(NOT hit)
      set(${out_error} "archive lacks an entry matching ${re}" PARENT_SCOPE)
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

# Every file each archive must hold; nothing else may be in it. Directory entries are ignored.
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
  message(STATUS
    "check_archives self-test: absent, stale, current and semicolon-path metadata controls passed")
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
