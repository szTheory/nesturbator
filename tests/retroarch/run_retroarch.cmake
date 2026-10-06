# retroarch.testframe (FRAME-05, D-17 to D-20): RetroArch runs the built core
# unattended, and its screenshot must equal the runner's frame exactly.
#
#   cmake -DCORE=<core module> -DINFO=<.info file> -DRUNNER=<nesturbator-run>
#         -DCOMPARE=<compare_frame> -DRA_DIR=<build>/retroarch
#         -DTEMPLATE=<test.cfg.in> -P run_retroarch.cmake
#
# Where it cannot run (not macOS, or no RetroArch) it prints a line starting
# "nesturbator-skip:" and reports itself skipped: exit 77 on CMake 3.29 and
# newer, which have cmake_language(EXIT); on 3.25 to 3.28 it exits 0 and the
# test's SKIP_REGULAR_EXPRESSION matches the line. Needs a logged-in GUI
# session, because RetroArch opens a window.
cmake_minimum_required(VERSION 3.25)

macro(skip reason)
  message("nesturbator-skip: ${reason}")
  if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.29)
    cmake_language(EXIT 77)
  endif()
  return()
endmacro()

foreach(var CORE INFO RUNNER COMPARE RA_DIR TEMPLATE)
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "run_retroarch.cmake needs -D${var}=...")
  endif()
endforeach()

# 1. The test is written for the macOS RetroArch app and sips.
if(NOT CMAKE_HOST_APPLE)
  skip("RetroArch test runs on macOS only")
endif()

# 2. RetroArch: an override from the environment, else the DMG's location.
if(DEFINED ENV{NESTURBATOR_RETROARCH})
  set(retroarch "$ENV{NESTURBATOR_RETROARCH}")
else()
  set(retroarch "/Applications/RetroArch.app/Contents/MacOS/RetroArch")
endif()
if(NOT EXISTS "${retroarch}")
  skip("RetroArch not found at ${retroarch}")
endif()

# 3. A fresh directory with the configuration and the core's .info file.
# Every directory test.cfg names exists, so RetroArch never falls back to a
# default location for a missing one.
file(REMOVE_RECURSE "${RA_DIR}")
foreach(sub assets cache cheats config content cores database/cursors database/rdb
    downloads filters/audio filters/video fonts info logs overlays playlists
    records records_config remaps runtime saves shaders shots states system
    thumbnails wallpapers autoconfig)
  file(MAKE_DIRECTORY "${RA_DIR}/${sub}")
endforeach()
configure_file("${TEMPLATE}" "${RA_DIR}/test.cfg" @ONLY)
file(COPY "${INFO}" DESTINATION "${RA_DIR}/info")

# 4. The reference: the runner's frame 5 as a P6 image.
execute_process(COMMAND "${RUNNER}" --frames 5 --dump-frame "5:${RA_DIR}/runner.ppm"
  RESULT_VARIABLE result)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "nesturbator-run exited with ${result}")
endif()

# 5. The user's RetroArch directory, read only: each entry with its size and
# modification time, relative to the directory. Built from HOME at run time,
# so no personal path is written anywhere.
set(user_dir "$ENV{HOME}/Library/Application Support/RetroArch")
function(snapshot out)
  if(NOT IS_DIRECTORY "${user_dir}")
    set(${out} "absent" PARENT_SCOPE)
    return()
  endif()
  file(GLOB_RECURSE entries LIST_DIRECTORIES true RELATIVE "${user_dir}" "${user_dir}/*")
  set(lines)
  foreach(entry IN LISTS entries)
    set(path "${user_dir}/${entry}")
    if(IS_SYMLINK "${path}")
      file(READ_SYMLINK "${path}" target)
      list(APPEND lines "${entry} -> ${target}")
    elseif(IS_DIRECTORY "${path}")
      list(APPEND lines "${entry}/")
    else()
      file(SIZE "${path}" size)
      file(TIMESTAMP "${path}" time "%Y-%m-%dT%H:%M:%S.%f" UTC)
      list(APPEND lines "${entry} ${size} ${time}")
    endif()
  endforeach()
  list(SORT lines)
  set(${out} "${lines}" PARENT_SCOPE)
endfunction()
snapshot(before)

# 6. RetroArch with only test.cfg, the core by absolute path and no content
# (the core sets SET_SUPPORT_NO_GAME). It exits after 5 frames and writes the
# core's frame to shot.png.
file(REAL_PATH "${CORE}" core)
execute_process(
  COMMAND "${retroarch}" -c "${RA_DIR}/test.cfg" -L "${core}"
    --max-frames=5 --max-frames-ss "--max-frames-ss-path=${RA_DIR}/shot.png"
  RESULT_VARIABLE ra_result
  OUTPUT_VARIABLE ra_stdout
  ERROR_VARIABLE ra_stderr
  TIMEOUT 50)
set(ra_output "stdout:\n${ra_stdout}\nstderr:\n${ra_stderr}")

# 7. Nothing in the user's directory was created, changed or removed. Checked
# before RetroArch's own result, so a failed run is still checked.
snapshot(after)
if(NOT before STREQUAL after)
  set(changes)
  foreach(line IN LISTS before)
    if(NOT line IN_LIST after)
      string(APPEND changes "\n  - ${line}")
    endif()
  endforeach()
  foreach(line IN LISTS after)
    if(NOT line IN_LIST before)
      string(APPEND changes "\n  + ${line}")
    endif()
  endforeach()
  message(FATAL_ERROR "RetroArch wrote outside the build directory:${changes}\n"
    "Add the key that names this location to tests/retroarch/test.cfg.in.\n"
    "RetroArch output:\n${ra_output}")
endif()

if(NOT ra_result EQUAL 0)
  message(FATAL_ERROR "RetroArch exited with ${ra_result}:\n${ra_output}")
endif()
if(NOT EXISTS "${RA_DIR}/shot.png")
  message(FATAL_ERROR "RetroArch wrote no screenshot at ${RA_DIR}/shot.png:\n${ra_output}")
endif()

# 8. D-20: sips colour-converts a PNG with a gAMA other than 1/2.2 or an iCCP
# chunk (01-RESEARCH, measured), so colour management is always stripped from
# a copy before converting it to BMP.
function(sips)
  execute_process(COMMAND sips ${ARGN} WORKING_DIRECTORY "${RA_DIR}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE output)
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "sips ${ARGN} exited with ${result}:\n${output}")
  endif()
endfunction()
file(COPY_FILE "${RA_DIR}/shot.png" "${RA_DIR}/shot_raw.png")
sips(--deleteColorManagementProperties shot_raw.png)
sips(-s format bmp shot_raw.png --out shot.bmp)

# 9. Exactly 256x240 and every pixel equal; compare_frame names a wrong size
# or the first differing pixel.
execute_process(COMMAND "${COMPARE}" "${RA_DIR}/runner.ppm" "${RA_DIR}/shot.bmp"
  RESULT_VARIABLE result OUTPUT_VARIABLE compare_output ERROR_VARIABLE compare_output)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "RetroArch's frame differs from the runner's:\n${compare_output}")
endif()
message(STATUS "RetroArch's frame equals the runner's frame")
