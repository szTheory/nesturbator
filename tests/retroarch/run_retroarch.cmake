# Hosted RetroArch checks (FRAME-05, SAVE-05, D-17 to D-20, TUNE-05). Only the hosted
# retroarch-e2e job runs this script; it fails rather than skips, and no CTest case launches it.
#
# Game mode: RetroArch runs the built core on a pinned game, and its screenshot must equal the
# runner's frame exactly.
#   cmake -DCORE=<core module> -DINFO=<.info file> -DRUNNER=<nesturbator-run>
#         -DCOMPARE=<compare_frame> -DRA_DIR=<build>/retroarch -DTEMPLATE=<test.cfg.in>
#         -DROM=<game.nes> -DFRAME=<frame> -DRETROARCH=<app executable>
#         -DEXPECTED_VERSION=<version> -DVERSION_PLIST=<app>/Contents/Info.plist
#         -P run_retroarch.cmake
#
# Save mode (-DSAVE_ROM=<battery game.nes> -DSAVE_FRAME=<frame> instead of -DROM and -DFRAME):
# two RetroArch sessions of one battery ROM. Session 1 starts with no save and RetroArch writes
# saves/<stem>.srm at content unload; session 2 starts with that file, which the runner also
# loads as <stem>.sav. Each screenshot must equal the runner's frame for the same save state,
# and the two screenshots must differ.
#
# Needs a logged-in GUI session, because RetroArch opens a window.
cmake_minimum_required(VERSION 3.25)

foreach(var CORE INFO RUNNER COMPARE RA_DIR TEMPLATE RETROARCH
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

# 2. The pinned RetroArch app, its version, and exactly one of the two modes.
set(retroarch "${RETROARCH}")
if(NOT EXISTS "${retroarch}")
  message(FATAL_ERROR "RetroArch executable is missing at ${retroarch}")
endif()
if(DEFINED ROM AND NOT "${ROM}" STREQUAL "")
  set(have_game TRUE)
else()
  set(have_game FALSE)
endif()
if(DEFINED SAVE_ROM AND NOT "${SAVE_ROM}" STREQUAL "")
  set(have_save TRUE)
else()
  set(have_save FALSE)
endif()
if((have_game AND have_save) OR (NOT have_game AND NOT have_save))
  message(FATAL_ERROR
    "run_retroarch.cmake needs exactly one of -DROM with -DFRAME, or -DSAVE_ROM with -DSAVE_FRAME")
endif()
if(have_game)
  set(mode_rom "${ROM}")
  set(mode_frame "${FRAME}")
  set(mode_frame_name FRAME)
else()
  set(mode_rom "${SAVE_ROM}")
  set(mode_frame "${SAVE_FRAME}")
  set(mode_frame_name SAVE_FRAME)
endif()
if(NOT EXISTS "${mode_rom}" OR IS_DIRECTORY "${mode_rom}")
  message(FATAL_ERROR "RetroArch test ROM is missing: ${mode_rom}")
endif()
if(NOT mode_frame MATCHES "^[1-9][0-9]*$")
  message(FATAL_ERROR "RetroArch test needs a positive -D${mode_frame_name}")
endif()

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

# 4. Content under the build tree. The stem names the .srm and the runner's .sav.
if(have_game)
  set(stem game)
else()
  cmake_path(GET mode_rom STEM stem)
endif()
file(COPY_FILE "${mode_rom}" "${RA_DIR}/content/${stem}.nes")
set(ra_content "${RA_DIR}/content/${stem}.nes")
file(REAL_PATH "${CORE}" core)

# The user's RetroArch directory, read only: each entry with its size and
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

# D-20: sips colour-converts a PNG with a gAMA other than 1/2.2 or an iCCP
# chunk (01-RESEARCH, measured), so colour management is always stripped from
# a copy before converting it to BMP.
function(sips)
  execute_process(COMMAND sips ${ARGN} WORKING_DIRECTORY "${RA_DIR}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE output)
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "sips ${ARGN} exited with ${result}:\n${output}")
  endif()
endfunction()
# One RetroArch session of the content, checked against the runner's frame for the same save
# state. Files are named by tag (runner-<tag>.ppm, shot-<tag>.png, shot-<tag>.bmp) so that a
# later session can never read an earlier session's screenshot (research Pitfall 11).
# runner_extra is a list of extra runner arguments (--save-dir in session 2).
function(ra_session tag content frame runner_extra)
  execute_process(COMMAND "${RUNNER}" --frames "${frame}" --rom "${content}"
      --dump-frame "${frame}:${RA_DIR}/runner-${tag}.ppm" ${runner_extra}
    RESULT_VARIABLE result)
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "[${tag}] nesturbator-run exited with ${result}")
  endif()

  snapshot(before)

  # RetroArch with only test.cfg, the core by absolute path and the pinned content. It writes
  # the selected core frame to shot-<tag>.png.
  # RetroArch's macOS app creates first-run directories under its home folder even when every
  # config path is redirected. Give the child an isolated home inside RA_DIR while
  # snapshotting the real user's directory.
  set(ra_home "${RA_DIR}/home")
  file(MAKE_DIRECTORY "${ra_home}")
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env
      "HOME=${ra_home}"
      "CFFIXED_USER_HOME=${ra_home}"
      "XDG_CONFIG_HOME=${ra_home}/.config"
      "XDG_DATA_HOME=${ra_home}/.local/share"
      "XDG_CACHE_HOME=${ra_home}/.cache"
      "${retroarch}" -c "${RA_DIR}/test.cfg" -L "${core}" "${content}"
      "--max-frames=${frame}" --max-frames-ss "--max-frames-ss-path=${RA_DIR}/shot-${tag}.png"
    RESULT_VARIABLE ra_result
    OUTPUT_VARIABLE ra_stdout
    ERROR_VARIABLE ra_stderr
    TIMEOUT 50)
  set(ra_output "stdout:\n${ra_stdout}\nstderr:\n${ra_stderr}")

  # Nothing in the user's directory was created, changed or removed. Checked before
  # RetroArch's own result, so a failed run is still checked.
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
    message(FATAL_ERROR "[${tag}] RetroArch wrote outside the build directory:${changes}\n"
      "Add the key that names this location to tests/retroarch/test.cfg.in.\n"
      "RetroArch output:\n${ra_output}")
  endif()

  if(NOT ra_result EQUAL 0)
    message(FATAL_ERROR "[${tag}] RetroArch exited with ${ra_result}:\n${ra_output}")
  endif()
  if(NOT EXISTS "${RA_DIR}/shot-${tag}.png")
    message(FATAL_ERROR "[${tag}] RetroArch wrote no screenshot at ${RA_DIR}/shot-${tag}.png:\n${ra_output}")
  endif()
  file(SIZE "${RA_DIR}/shot-${tag}.png" screenshot_size)
  if(screenshot_size EQUAL 0)
    message(FATAL_ERROR "[${tag}] RetroArch wrote an empty screenshot at ${RA_DIR}/shot-${tag}.png:\n${ra_output}")
  endif()

  file(COPY_FILE "${RA_DIR}/shot-${tag}.png" "${RA_DIR}/shot-${tag}-raw.png")
  sips(--deleteColorManagementProperties shot-${tag}-raw.png)
  sips(-s format bmp shot-${tag}-raw.png --out shot-${tag}.bmp)

  # Exactly 256x240 and every pixel equal; compare_frame names a wrong size or the first
  # differing pixel.
  execute_process(COMMAND "${COMPARE}" "${RA_DIR}/runner-${tag}.ppm" "${RA_DIR}/shot-${tag}.bmp"
    RESULT_VARIABLE result OUTPUT_VARIABLE compare_output ERROR_VARIABLE compare_output)
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "[${tag}] RetroArch's frame differs from the runner's:\n${compare_output}")
  endif()
  message(STATUS "[${tag}] RetroArch screenshot: ${RA_DIR}/shot-${tag}.png (${screenshot_size} bytes)")
  message(STATUS "[${tag}] RetroArch's frame ${frame} equals the runner's frame ${frame}")
endfunction()

if(have_game)
  ra_session(game "${ra_content}" "${mode_frame}" "")
  return()
endif()

# Save mode (D-20). RetroArch writes the .srm at content unload; the settings in test.cfg.in
# keep every other save path off, so a stray file here is a failure, not a pass.
function(check_one_srm context)
  file(GLOB found "${RA_DIR}/saves/*")
  list(LENGTH found count)
  if(NOT count EQUAL 1)
    message(FATAL_ERROR "${context}: expected exactly one file in ${RA_DIR}/saves, found ${count}: ${found}")
  endif()
  if(NOT found STREQUAL "${RA_DIR}/saves/${stem}.srm")
    message(FATAL_ERROR "${context}: expected ${RA_DIR}/saves/${stem}.srm, found ${found}")
  endif()
  file(SIZE "${RA_DIR}/saves/${stem}.srm" srm_size)
  if(NOT srm_size EQUAL 32768)
    message(FATAL_ERROR "${context}: ${stem}.srm is ${srm_size} bytes, expected 32768")
  endif()
endfunction()

file(GLOB before_saves "${RA_DIR}/saves/*")
if(before_saves)
  message(FATAL_ERROR "saves/ is not empty before session 1: ${before_saves}")
endif()

ra_session(s1 "${ra_content}" "${mode_frame}" "")
check_one_srm("after session 1")
# SAVEDATA at 0x100 is the pattern the SXROM test ROM writes in its first run.
file(READ "${RA_DIR}/saves/${stem}.srm" marker HEX OFFSET 256 LIMIT 8)
if(NOT marker STREQUAL "5341564544415441")
  message(FATAL_ERROR "session 1's ${stem}.srm has ${marker} at 0x100, expected 5341564544415441 (SAVEDATA)")
endif()

# Session 2: RetroArch loads the .srm into the span after retro_load_game; the runner reads the
# same bytes as <stem>.sav.
file(MAKE_DIRECTORY "${RA_DIR}/runner-saves")
file(COPY_FILE "${RA_DIR}/saves/${stem}.srm" "${RA_DIR}/runner-saves/${stem}.sav")
ra_session(s2 "${ra_content}" "${mode_frame}" "--save-dir;${RA_DIR}/runner-saves")
file(SHA256 "${RA_DIR}/shot-s1.bmp" sha_s1)
file(SHA256 "${RA_DIR}/shot-s2.bmp" sha_s2)
if(sha_s1 STREQUAL sha_s2)
  message(FATAL_ERROR "session 1 and session 2 screenshots are identical (${sha_s1}); the save did not change the screen")
endif()
check_one_srm("after session 2")
message(STATUS "RetroArch battery save round trip: ${stem}.srm written, loaded back, frames equal the runner's")
