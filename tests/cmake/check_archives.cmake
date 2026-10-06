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

# Every file each archive must hold; nothing else may be in it. Directory
# entries are ignored. A Linux install may use lib64.
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

  execute_process(COMMAND ${CMAKE_COMMAND} -E tar tf "${zip}"
    OUTPUT_VARIABLE listing RESULT_VARIABLE rc)
  if(NOT rc EQUAL 0)
    message(FATAL_ERROR "check_archives: cannot list ${name}")
  endif()
  string(REPLACE "\r" "" listing "${listing}")
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
      message(FATAL_ERROR "check_archives: ${name} lacks an entry matching ${re}")
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
      message(FATAL_ERROR "check_archives: ${name} holds an extra file ${f}")
    endif()
  endforeach()
  message(STATUS "check_archives: ${name} ok")
endforeach()

list(REMOVE_DUPLICATES components)
list(LENGTH components distinct)
if(NOT distinct EQUAL 3)
  message(FATAL_ERROR "check_archives: components ${components}, expected library, runner and libretro")
endif()
