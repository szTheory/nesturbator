# Checks the README's one-line RetroArch install for Apple Silicon against
# the libretro archive a build writes. The tar arguments come from the README
# line itself, and the zip goes to tar on standard input, as curl's output
# does in the line.
#
#   cmake -DREADME=<README.md> -DPACKAGES=<zip dir> -DOUT=<scratch dir>
#         -P tests/cmake/check_install_line.cmake
#
# macOS only: the line relies on macOS's tar (bsdtar), which reads a zip from
# standard input. VERSION defaults to version.txt at the repository root.
# The line's download half (the release URL) is not fetched here.

foreach(var README PACKAGES OUT)
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "check_install_line: pass -D${var}=...")
  endif()
endforeach()
if(NOT CMAKE_HOST_APPLE)
  message(FATAL_ERROR "check_install_line: runs on macOS only, whose tar reads a zip on stdin")
endif()
get_filename_component(OUT "${OUT}" ABSOLUTE)
if(NOT DEFINED VERSION)
  file(READ "${CMAKE_CURRENT_LIST_DIR}/../../version.txt" VERSION)
  string(STRIP "${VERSION}" VERSION)
endif()

# 1. Exactly one README line pipes the macOS arm64 libretro zip into tar.
file(STRINGS "${README}" lines REGEX "-libretro-macos-arm64\\.zip \\| tar ")
list(LENGTH lines count)
if(NOT count EQUAL 1)
  message(FATAL_ERROR "check_install_line: ${count} lines in ${README} contain "
    "'-libretro-macos-arm64.zip | tar ', expected 1")
endif()
set(line "${lines}")

# 2. The URL names version.txt's version in both the tag and the file name.
string(REPLACE "." "\\." version_re "${VERSION}")
if(NOT line MATCHES "/releases/download/v${version_re}/nesturbator-${version_re}-libretro-macos-arm64\\.zip ")
  message(FATAL_ERROR "check_install_line: the install line does not name "
    "v${VERSION}/nesturbator-${VERSION}-libretro-macos-arm64.zip (version.txt):\n${line}")
endif()

# 3. After "| tar ": <flags> -C <directory> <members>. The directory is the
# RetroArch path with its spaces backslash-escaped; it is replaced by OUT.
string(FIND "${line}" "| tar " at)
math(EXPR at "${at} + 6")
string(SUBSTRING "${line}" ${at} -1 tar_args)
string(STRIP "${tar_args}" tar_args)
if(NOT tar_args MATCHES "^(.+) -C ((\\\\.|[^ \\\\])+) ([^ ].*)$")
  message(FATAL_ERROR "check_install_line: tar arguments are not "
    "'<flags> -C <directory> <members>': ${tar_args}")
endif()
set(flags_text "${CMAKE_MATCH_1}")
set(members_text "${CMAKE_MATCH_4}")
separate_arguments(flags UNIX_COMMAND "${flags_text}")
separate_arguments(members UNIX_COMMAND "${members_text}")

# 4. The zip of this version, through the line's tar on standard input.
set(zip "${PACKAGES}/nesturbator-${VERSION}-libretro-macos-arm64.zip")
if(NOT EXISTS "${zip}")
  message(FATAL_ERROR "check_install_line: no ${zip}; run cmake --workflow --preset ci first")
endif()
file(REMOVE_RECURSE "${OUT}")
file(MAKE_DIRECTORY "${OUT}")
execute_process(
  COMMAND cat "${zip}"
  COMMAND tar ${flags} -C "${OUT}" ${members}
  RESULTS_VARIABLE results
  OUTPUT_VARIABLE output
  ERROR_VARIABLE output)
foreach(rc IN LISTS results)
  if(NOT rc EQUAL 0)
    message(FATAL_ERROR "check_install_line: cat | tar ${flags} -C <OUT> ${members} "
      "exited with ${results}:\n${output}")
  endif()
endforeach()

# 5. Exactly the core and its information file, nothing else.
foreach(want cores/nesturbator_libretro.dylib info/nesturbator_libretro.info)
  if(NOT EXISTS "${OUT}/${want}")
    message(FATAL_ERROR "check_install_line: the install line did not extract ${want}")
  endif()
endforeach()
file(GLOB top LIST_DIRECTORIES true RELATIVE "${OUT}" "${OUT}/*")
list(SORT top)
if(NOT top STREQUAL "cores;info")
  message(FATAL_ERROR "check_install_line: the install line extracted ${top}, expected cores and info only")
endif()
file(GLOB_RECURSE extracted LIST_DIRECTORIES false RELATIVE "${OUT}" "${OUT}/*")
list(SORT extracted)
if(NOT extracted STREQUAL "cores/nesturbator_libretro.dylib;info/nesturbator_libretro.info")
  message(FATAL_ERROR "check_install_line: the install line extracted ${extracted}, "
    "expected cores/nesturbator_libretro.dylib and info/nesturbator_libretro.info only")
endif()
message(STATUS "check_install_line: ${tar_args} extracts the core and its .info file from v${VERSION}")
