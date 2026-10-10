# Hosted RetroArch check (FRAME-05, D-17 to D-20, TUNE-05): RetroArch runs the
# built core on a pinned game, and its screenshot must equal the runner's frame
# exactly. Only the hosted retroarch-e2e job runs this script; it fails rather
# than skips, and no CTest case launches it.
#
#   cmake -DCORE=<core module> -DINFO=<.info file> -DRUNNER=<nesturbator-run>
#         -DCOMPARE=<compare_frame> -DRA_DIR=<build>/retroarch
#         -DTEMPLATE=<test.cfg.in> -DROM=<game.nes> -DFRAME=<frame>
#         -DRETROARCH=<app executable> -DEXPECTED_VERSION=<version>
#         -DVERSION_PLIST=<app>/Contents/Info.plist -P run_retroarch.cmake
#
# Needs a logged-in GUI session, because RetroArch opens a window.
cmake_minimum_required(VERSION 3.25)

foreach(var CORE INFO RUNNER COMPARE RA_DIR TEMPLATE RETROARCH ROM FRAME
    EXPECTED_VERSION VERSION_PLIST)
  if(NOT DEFINED ${var} OR "${${var}}" STREQUAL "")
    message(FATAL_ERROR "run_retroarch.cmake needs -D${var}=...")
  endif()
endforeach()

# 1. The script is written for the macOS RetroArch app and sips.
if(NOT CMAKE_HOST_APPLE)
  message(FATAL_ERROR
    "run_retroarch.cmake runs only on macOS (hosted retroarch-e2e job)")
endif()

# 2. The pinned RetroArch app, its version, the ROM and the frame number.
set(retroarch "${RETROARCH}")
if(NOT EXISTS "${retroarch}")
  message(FATAL_ERROR "RetroArch executable is missing at ${retroarch}")
endif()
if(NOT EXISTS "${ROM}" OR IS_DIRECTORY "${ROM}")
  message(FATAL_ERROR "RetroArch test ROM is missing: ${ROM}")
endif()
if(NOT FRAME MATCHES "^[1-9][0-9]*$")
  message(FATAL_ERROR "RetroArch game test needs a positive -DFRAME")
endif()
set(frame "${FRAME}")

if(NOT EXISTS "${VERSION_PLIST}")
  message(FATAL_ERROR "RetroArch version plist is missing: ${VERSION_PLIST}")
endif()
execute_process(COMMAND /usr/libexec/PlistBuddy -c "Print :CFBundleShortVersionString" "${VERSION_PLIST}"
  RESULT_VARIABLE version_result OUTPUT_VARIABLE measured_version ERROR_VARIABLE version_error)
string(STRIP "${measured_version}" measured_version)
if(NOT version_result EQUAL 0 OR NOT measured_version STREQUAL EXPECTED_VERSION)
  message(FATAL_ERROR "RetroArch version mismatch: expected ${EXPECTED_VERSION}, got '${measured_version}': ${version_error}")
endif()
message(STATUS "RetroArch version: ${measured_version}")

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

# 4. Keep content under the build tree and use the runner's matching frame.
file(COPY_FILE "${ROM}" "${RA_DIR}/content/game.nes")
set(ra_content "${RA_DIR}/content/game.nes")
execute_process(COMMAND "${RUNNER}" --frames "${frame}" --rom "${ra_content}"
    --dump-frame "${frame}:${RA_DIR}/runner.ppm"
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

# 6. RetroArch with only test.cfg, the core by absolute path and pinned content
# when supplied. It writes the selected core frame to shot.png.
file(REAL_PATH "${CORE}" core)
# RetroArch's macOS app creates first-run directories under its home folder
# even when every config path is redirected. Give the child an isolated home
# inside RA_DIR while snapshotting the real user's directory above.
set(ra_home "${RA_DIR}/home")
file(MAKE_DIRECTORY "${ra_home}")
execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env
    "HOME=${ra_home}"
    "CFFIXED_USER_HOME=${ra_home}"
    "XDG_CONFIG_HOME=${ra_home}/.config"
    "XDG_DATA_HOME=${ra_home}/.local/share"
    "XDG_CACHE_HOME=${ra_home}/.cache"
    "${retroarch}" -c "${RA_DIR}/test.cfg" -L "${core}" "${ra_content}"
    "--max-frames=${frame}" --max-frames-ss "--max-frames-ss-path=${RA_DIR}/shot.png"
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
file(SIZE "${RA_DIR}/shot.png" screenshot_size)
if(screenshot_size EQUAL 0)
  message(FATAL_ERROR "RetroArch wrote an empty screenshot at ${RA_DIR}/shot.png:\n${ra_output}")
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
message(STATUS "RetroArch screenshot: ${RA_DIR}/shot.png (${screenshot_size} bytes)")
message(STATUS "RetroArch's frame ${frame} equals the runner's frame ${frame}")
