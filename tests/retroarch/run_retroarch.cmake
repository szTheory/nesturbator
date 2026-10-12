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
# Save mode (-DSAVE_ROM=<battery game.nes> -DSAVE_FRAME=<frame> [-DSAVE_SIZE=<bytes>] instead of
# -DROM and -DFRAME; SAVE_SIZE is the exact .srm size, default 32768):
# two RetroArch sessions of one battery ROM. Session 1 starts with no save and RetroArch writes
# saves/<stem>.srm at content unload; session 2 starts with that file, which the runner also
# loads as <stem>.sav. Each screenshot must equal the runner's frame for the same save state,
# and the two screenshots must differ.
#
# State mode (-DSTATE_ROM=<game.nes> -DFRAME_INDEX=<frame_index> -DSTATE_TEMPLATE=<state.cfg.in>
# instead of -DROM and -DFRAME): the core is copied into RA_DIR/cores and that copy is loaded
# with -L, because RetroArch applies the .info savestate level only to a core in its cores
# directory (13-RESEARCH flag 1). The expected run-ahead lead k_eff comes from the .info under
# test: run_ahead_frames when savestate_features is deterministic, 0 when serialized. Lanes with
# --max-frames=120: state-base shows the runner's frame 120, state-ahead (run_ahead_enabled) shows
# frame 120 + k_eff. Every lane's verbose log is kept at RA_DIR/retroarch-<lane>.log and passes
# log_check.cmake, and ra_state_outcome_checks confirms the thumbnail and secondary-instance keys.
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
if(DEFINED STATE_ROM AND NOT "${STATE_ROM}" STREQUAL "")
  set(have_state TRUE)
else()
  set(have_state FALSE)
endif()
set(mode_count 0)
foreach(flag have_game have_save have_state)
  if(${flag})
    math(EXPR mode_count "${mode_count} + 1")
  endif()
endforeach()
if(NOT mode_count EQUAL 1)
  message(FATAL_ERROR
    "run_retroarch.cmake needs exactly one of -DROM with -DFRAME, -DSAVE_ROM with -DSAVE_FRAME, or -DSTATE_ROM")
endif()
if(have_state)
  foreach(var FRAME_INDEX STATE_TEMPLATE)
    if(NOT DEFINED ${var} OR "${${var}}" STREQUAL "")
      message(FATAL_ERROR "run_retroarch.cmake's state mode needs -D${var}=...")
    endif()
  endforeach()
  set(mode_rom "${STATE_ROM}")
  set(mode_frame 120)
  set(mode_frame_name STATE_FRAME)
elseif(have_game)
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
set(save_size 32768)
if(have_save AND DEFINED SAVE_SIZE AND NOT "${SAVE_SIZE}" STREQUAL "")
  set(save_size "${SAVE_SIZE}")
endif()
if(have_save AND NOT save_size MATCHES "^[1-9][0-9]*$")
  message(FATAL_ERROR "RetroArch save test needs a positive -DSAVE_SIZE")
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
# The config file and the core every session uses; state lanes set their own before ra_session.
set(ra_cfg "${RA_DIR}/test.cfg")
set(ra_core "${core}")
if(have_state)
  # RetroArch applies the .info level only to a core it finds in its cores directory, next to
  # the .info in RA_DIR/info (13-RESEARCH flag 1, Pitfall 1), so -L names this copy.
  file(COPY_FILE "${core}" "${RA_DIR}/cores/nesturbator_libretro.dylib")
  set(ra_core "${RA_DIR}/cores/nesturbator_libretro.dylib")
endif()

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

  # State lanes set ra_max_frames (RetroArch stops earlier than the runner's frame when run-ahead
  # shows a later frame) and ra_verbose (keep the log for log_check.cmake).
  set(max_frames "${frame}")
  if(DEFINED ra_max_frames)
    set(max_frames "${ra_max_frames}")
  endif()
  set(verbose_flag)
  if(DEFINED ra_verbose)
    set(verbose_flag --verbose)
  endif()

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
      "${retroarch}" -c "${ra_cfg}" -L "${ra_core}" "${content}" ${verbose_flag}
      "--max-frames=${max_frames}" --max-frames-ss "--max-frames-ss-path=${RA_DIR}/shot-${tag}.png"
    RESULT_VARIABLE ra_result
    OUTPUT_VARIABLE ra_stdout
    ERROR_VARIABLE ra_stderr
    TIMEOUT 50)
  set(ra_output "stdout:\n${ra_stdout}\nstderr:\n${ra_stderr}")
  if(DEFINED ra_verbose)
    file(WRITE "${RA_DIR}/retroarch-${tag}.log" "${ra_stdout}${ra_stderr}")
  endif()

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

if(have_state)
  # State mode (D-26, D-27, D-23): the claim-level lanes. Game and battery modes below are
  # unchanged (D-29).

  # The expected run-ahead lead from the .info under test: run_ahead_frames when the .info
  # claims deterministic, 0 when it claims serialized. An .info that claims neither is refused.
  file(STRINGS "${INFO}" feature_lines REGEX "^savestate_features *= *\"")
  string(REGEX REPLACE "^savestate_features *= *\"([^\"]*)\".*$" "\\1" claim_level
    "${feature_lines}")
  file(STRINGS "${STATE_TEMPLATE}" ahead_lines REGEX "^run_ahead_frames *= *\"[0-9]+\"")
  string(REGEX REPLACE "^run_ahead_frames *= *\"([0-9]+)\".*$" "\\1" lead_frames "${ahead_lines}")
  if(claim_level STREQUAL "deterministic")
    set(k_eff "${lead_frames}")
  elseif(claim_level STREQUAL "serialized")
    set(k_eff 0)
  else()
    message(FATAL_ERROR
      "${INFO} claims savestate_features '${claim_level}'; the state mode needs serialized or deterministic")
  endif()
  message(STATUS "State mode: the .info claims '${claim_level}', expected run-ahead lead ${k_eff}")

  # A lane config: test.cfg, then state.cfg.in, then each override ("key=value") replacing the
  # existing `key = "value"` line. A key that ends up on two lines is an error, because
  # RetroArch would take one of them without saying which.
  include("${CMAKE_CURRENT_LIST_DIR}/log_check.cmake")
  function(ra_write_lane_cfg out)
    file(READ "${RA_DIR}/test.cfg" base_cfg)
    file(READ "${STATE_TEMPLATE}" state_cfg)
    set(cfg "${base_cfg}\n${state_cfg}")
    foreach(override IN LISTS ARGN)
      string(REGEX MATCH "^([a-z0-9_]+)=(.*)$" matched "${override}")
      set(key "${CMAKE_MATCH_1}")
      set(value "${CMAKE_MATCH_2}")
      if(key STREQUAL "")
        message(FATAL_ERROR "ra_write_lane_cfg: bad override '${override}'")
      endif()
      string(REGEX MATCHALL "(^|\n)${key} = \"[^\"]*\"" present "${cfg}")
      list(LENGTH present present_count)
      if(NOT present_count EQUAL 1)
        message(FATAL_ERROR "ra_write_lane_cfg: ${key} appears ${present_count} times, expected 1")
      endif()
      string(REGEX REPLACE "(^|\n)${key} = \"[^\"]*\"" "\\1${key} = \"${value}\"" cfg "${cfg}")
    endforeach()
    string(REGEX MATCHALL "(^|\n)[a-z0-9_]+ = " key_lines "${cfg}")
    list(LENGTH key_lines key_count)
    list(REMOVE_DUPLICATES key_lines)
    list(LENGTH key_lines unique_count)
    if(NOT key_count EQUAL unique_count)
      message(FATAL_ERROR "ra_write_lane_cfg: a key appears on two lines of ${out}")
    endif()
    file(WRITE "${out}" "${cfg}")
  endfunction()

  # D-26: the outcomes that confirm the two keys that otherwise have none.
  function(ra_state_outcome_checks lane log)
    # savestate_thumbnail_enable = "false": no thumbnail was written beside any state.
    file(GLOB_RECURSE thumbnails "${RA_DIR}/states/*.png")
    if(thumbnails)
      message(FATAL_ERROR "[${lane}] savestate_thumbnail_enable = false, but thumbnails exist: ${thumbnails}")
    endif()
    # run_ahead_secondary_instance = "false": a second core instance could be loaded from a copy
    # of the core library, so a library outside RA_DIR/cores is a failure. Measured on 1.22.2
    # with the key true: no copy appears on macOS, so the log check below is the effective one
    # here. The .info under RA_DIR/info is not a library and is not matched.
    file(GLOB_RECURSE core_copies
      "${RA_DIR}/*nesturbator_libretro.dylib"
      "${RA_DIR}/*nesturbator_libretro.so"
      "${RA_DIR}/*nesturbator_libretro.dll")
    foreach(copy IN LISTS core_copies)
      cmake_path(IS_PREFIX RA_DIR_cores "${copy}" NORMALIZE inside_cores)
      if(NOT inside_cores)
        message(FATAL_ERROR "[${lane}] run_ahead_secondary_instance = false, but a core copy exists outside ${RA_DIR}/cores: ${copy}")
      endif()
    endforeach()
    # Measured on RetroArch 1.22.2 (macOS), lane state-ahead with .info deterministic: with
    # run_ahead_secondary_instance = "true" the log has no line that is absent with the key
    # false and no core library copy outside RA_DIR/cores, but the second instance repeats the
    # core's environment calls: `[INFO] [Environ] SET_SUPPORT_NO_GAME: yes.` appears 4 times, with
    # the key false it appears 2 times in every lane, state-base and state-ahead alike. A count
    # above 2 is therefore the marker.
    file(STRINGS "${log}" no_game_lines REGEX "^\\[INFO\\] \\[Environ\\] SET_SUPPORT_NO_GAME")
    list(LENGTH no_game_lines no_game_count)
    if(no_game_count GREATER 2)
      message(FATAL_ERROR "[${lane}] run_ahead_secondary_instance = false, but ${log} shows the core set up ${no_game_count} times (a second instance), expected 2")
    endif()
  endfunction()
  set(RA_DIR_cores "${RA_DIR}/cores")

  # One lane: its own config (overrides are "key=value" items), a verbose log kept at
  # RA_DIR/retroarch-<lane>.log, RetroArch stopping at --max-frames=120 and its screenshot equal
  # to the runner's frame `expected`, which frame_index also finds among frames 1..expected+8.
  function(ra_state_lane lane expected)
    ra_write_lane_cfg("${RA_DIR}/${lane}.cfg" ${ARGN})
    set(ra_cfg "${RA_DIR}/${lane}.cfg")
    set(ra_verbose ON)
    set(ra_max_frames 120)
    ra_session(${lane} "${ra_content}" "${expected}" "")
    math(EXPR index_limit "${expected} + 8")
    execute_process(COMMAND "${FRAME_INDEX}" "${mode_rom}" "${index_limit}"
        "${RA_DIR}/shot-${lane}.bmp"
      RESULT_VARIABLE index_result OUTPUT_VARIABLE index_output ERROR_VARIABLE index_output)
    string(STRIP "${index_output}" index_output)
    if(NOT index_result EQUAL 0 OR NOT index_output MATCHES " ${expected}( |$)")
      message(FATAL_ERROR "[${lane}] frame_index did not find the runner's frame ${expected}: ${index_output}")
    endif()
    message(STATUS "[${lane}] frame_index: ${index_output}")
  endfunction()

  ra_state_lane(state-base 120)
  ra_state_outcome_checks(state-base "${RA_DIR}/retroarch-state-base.log")
  nesturbator_ra_log_check("${RA_DIR}/retroarch-state-base.log"
    "${CMAKE_CURRENT_LIST_DIR}/log_allowlist.txt" "${claim_level}" ON)
  math(EXPR ahead_frame "120 + ${k_eff}")
  ra_state_lane(state-ahead ${ahead_frame} "run_ahead_enabled=true")
  ra_state_outcome_checks(state-ahead "${RA_DIR}/retroarch-state-ahead.log")
  nesturbator_ra_log_check("${RA_DIR}/retroarch-state-ahead.log"
    "${CMAKE_CURRENT_LIST_DIR}/log_allowlist.txt" "${claim_level}" ON)
  if(k_eff GREATER 0)
    # The lane must tell the levels apart: the runner's frames 120 and 120 + k_eff differ.
    file(SHA256 "${RA_DIR}/runner-state-base.ppm" base_sha)
    file(SHA256 "${RA_DIR}/runner-state-ahead.ppm" ahead_sha)
    if(base_sha STREQUAL ahead_sha)
      message(FATAL_ERROR "runner frames 120 and ${ahead_frame} are identical; the lane cannot show run-ahead")
    endif()
  endif()
  message(STATUS "RetroArch applied the .info level '${claim_level}': state-ahead showed frame ${ahead_frame}")
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
  if(NOT srm_size EQUAL save_size)
    message(FATAL_ERROR "${context}: ${stem}.srm is ${srm_size} bytes, expected ${save_size}")
  endif()
endfunction()

file(GLOB before_saves "${RA_DIR}/saves/*")
if(before_saves)
  message(FATAL_ERROR "saves/ is not empty before session 1: ${before_saves}")
endif()

ra_session(s1 "${ra_content}" "${mode_frame}" "")
check_one_srm("after session 1")
# Every Holy Mapperel battery ROM writes SAVEDATA at 0x100 on its first run (the shared run1.sav
# check in tests/CMakeLists.txt).
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
