# release-please's generic updater rewrites only the first version on each
# line it updates: every line inside an x-release-please-start /
# x-release-please-end block, and every line with an inline
# x-release-please-version marker. A second version on such a line keeps
# the old value after a release. This scan fails, naming file and line,
# when any such line holds two or more versions.
#
#   cmake -DSOURCE_DIR=<repository root> [-DFILES=<paths>] -P release_markers.cmake
#
# FILES is a list of paths relative to SOURCE_DIR. Without it, the scan
# covers every extra-files path in SOURCE_DIR/release-please-config.json.

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_DIR)
  message(FATAL_ERROR "release_markers: pass -DSOURCE_DIR=...")
endif()

if(NOT DEFINED FILES)
  file(READ "${SOURCE_DIR}/release-please-config.json" config)
  string(JSON count LENGTH "${config}" packages . extra-files)
  set(FILES "")
  if(count GREATER 0)
    math(EXPR last "${count} - 1")
    foreach(i RANGE ${last})
      string(JSON path GET "${config}" packages . extra-files ${i} path)
      list(APPEND FILES "${path}")
    endforeach()
  endif()
endif()

set(problems "")
foreach(path IN LISTS FILES)
  # Split on newlines into a list, keeping empty lines so numbers are exact.
  # Backslashes, semicolons and brackets would change list splitting, so
  # they are masked.
  file(READ "${SOURCE_DIR}/${path}" text)
  string(REPLACE "\\" "<backslash>" text "${text}")
  string(REPLACE ";" "<semicolon>" text "${text}")
  string(REPLACE "[" "<open>" text "${text}")
  string(REPLACE "]" "<close>" text "${text}")
  string(REPLACE "\n" ";" lines "${text}")
  set(number 0)
  set(inside FALSE)
  set(start_line 0)
  foreach(line IN LISTS lines)
    math(EXPR number "${number} + 1")
    if(line MATCHES "x-release-please-start")
      set(inside TRUE)
      set(start_line ${number})
      continue()
    endif()
    if(line MATCHES "x-release-please-end")
      set(inside FALSE)
      continue()
    endif()
    if(inside OR line MATCHES "x-release-please-version")
      string(REGEX MATCHALL "[0-9]+\\.[0-9]+\\.[0-9]+" versions "${line}")
      list(LENGTH versions n)
      if(n GREATER 1)
        string(REPLACE "<semicolon>" ";" shown "${line}")
        string(REPLACE "<open>" "[" shown "${shown}")
        string(REPLACE "<close>" "]" shown "${shown}")
        string(REPLACE "<backslash>" "\\" shown "${shown}")
        string(APPEND problems "\n${path}:${number}: ${n} versions on one line; "
          "release-please updates only the first\n  ${shown}")
      endif()
    endif()
  endforeach()
  if(inside)
    string(APPEND problems "\n${path}:${start_line}: x-release-please-start has no "
      "x-release-please-end")
  endif()
endforeach()

if(NOT problems STREQUAL "")
  message(FATAL_ERROR "release_markers:${problems}")
endif()
message(STATUS "release_markers: one version per marked line in ${FILES}")
