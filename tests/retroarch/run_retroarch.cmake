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
# Save and load lanes: state-save (auto-save, --max-frames=150, frame 150, and the RASTATE file
# checked), state-load (auto-load, --max-frames=30, frame 179) and state-load-ahead (the same with
# run-ahead, 179 + k_eff). Then lane state-net runs -DSESSION_SCRIPT (state_session.sh): one paused
# session driven over RetroArch's network commands that saves, steps, loads, resumes, rewinds and
# steps with run-ahead, every screenshot identified by frame_index as a set of runner frames.
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

if(MARKER_GATE)
  if(NOT have_state)
    message(FATAL_ERROR "MARKER_GATE requires state mode")
  endif()
  # Bind the supplied bytes, not merely a source fixture with the same basename.
  execute_process(COMMAND "${FRAME_INDEX}" --marker-generate "${RA_DIR}/expected-marker.nes"
    RESULT_VARIABLE generated)
  execute_process(COMMAND "${FRAME_INDEX}" --marker-generate "${RA_DIR}/regenerated-marker.nes"
    RESULT_VARIABLE regenerated)
  if(NOT generated EQUAL 0 OR NOT regenerated EQUAL 0)
    message(FATAL_ERROR "marker generation failed")
  endif()
  file(SHA256 "${RA_DIR}/expected-marker.nes" expected_marker_sha)
  file(SHA256 "${RA_DIR}/regenerated-marker.nes" regenerated_marker_sha)
  file(SHA256 "${mode_rom}" supplied_marker_sha)
  file(SHA256 "${ra_content}" copied_marker_sha)
  if(NOT expected_marker_sha STREQUAL regenerated_marker_sha OR
      NOT expected_marker_sha STREQUAL supplied_marker_sha OR
      NOT expected_marker_sha STREQUAL copied_marker_sha)
    message(FATAL_ERROR "marker content SHA-256 mismatch")
  endif()
  execute_process(COMMAND "${FRAME_INDEX}" --marker-qualify "${ra_content}"
      "${RA_DIR}/calibration.txt" RESULT_VARIABLE qualified
    OUTPUT_VARIABLE qualification_output ERROR_VARIABLE qualification_error TIMEOUT 600)
  file(WRITE "${RA_DIR}/qualification.txt" "${qualification_output}${qualification_error}")
  if(NOT qualified EQUAL 0)
    message(FATAL_ERROR "marker qualification failed: ${qualification_output}${qualification_error}")
  endif()
  # The pin is an asset observation, not just an app plist's version string.
  if(NOT DEFINED ASSET_EVIDENCE)
    cmake_path(GET VERSION_PLIST PARENT_PATH asset_parent)
    cmake_path(GET asset_parent PARENT_PATH asset_parent)
    cmake_path(GET asset_parent PARENT_PATH asset_parent)
    set(ASSET_EVIDENCE "${asset_parent}/asset-evidence.txt")
  endif()
  if(NOT EXISTS "${ASSET_EVIDENCE}")
    message(FATAL_ERROR "marker gate needs the verified pinned asset-evidence.txt")
  endif()
  file(READ "${ASSET_EVIDENCE}" asset_text)
  set(pin_sha "81b79121ba26d539064ae13b4d0419a120c3d165afbe656cf5f5412b15fdb434")
  if(NOT asset_text MATCHES "expected SHA-256: ${pin_sha}" OR
      NOT asset_text MATCHES "actual SHA-256: ${pin_sha}" OR
      NOT asset_text MATCHES "RetroArch version: 1.22.2")
    message(FATAL_ERROR "marker gate asset evidence does not match the required pin")
  endif()
  file(COPY_FILE "${ASSET_EVIDENCE}" "${RA_DIR}/asset-evidence.txt")
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
  # RA_DIR/retroarch-<lane>.log, RetroArch stopping at --max-frames=max_frames and its screenshot
  # equal to the runner's frame `expected`, which frame_index also finds among frames
  # 1..expected+8. The lane's log goes through the outcome checks and the allowlisted log check.
  function(ra_state_lane lane expected max_frames)
    ra_write_lane_cfg("${RA_DIR}/${lane}.cfg" ${ARGN})
    set(ra_cfg "${RA_DIR}/${lane}.cfg")
    set(ra_verbose ON)
    set(ra_max_frames "${max_frames}")
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

  ra_state_lane(state-base 120 120)
  ra_state_outcome_checks(state-base "${RA_DIR}/retroarch-state-base.log")
  nesturbator_ra_log_check("${RA_DIR}/retroarch-state-base.log"
    "${CMAKE_CURRENT_LIST_DIR}/log_allowlist.txt" "${claim_level}" ON)
  math(EXPR ahead_frame "120 + ${k_eff}")
  ra_state_lane(state-ahead ${ahead_frame} 120 "run_ahead_enabled=true")
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

  # --- Save and load lanes (LR-03, 13-RESEARCH "Deterministic lanes") ---
  # S, the state size, from the runner.
  execute_process(COMMAND "${RUNNER}" --rom "${ra_content}" --frames 1
      --state-at-frame "1:${RA_DIR}/runner-state-size.bin"
    RESULT_VARIABLE size_result)
  if(NOT size_result EQUAL 0 OR NOT EXISTS "${RA_DIR}/runner-state-size.bin")
    message(FATAL_ERROR "nesturbator-run could not write a state of ${ra_content}")
  endif()
  file(SIZE "${RA_DIR}/runner-state-size.bin" state_size)

  # A little-endian unsigned integer from the hex digits of `bytes` (two digits per byte).
  function(ra_le_value out hex)
    string(LENGTH "${hex}" digits)
    set(value 0)
    set(pos "${digits}")
    while(pos GREATER 0)
      math(EXPR pos "${pos} - 2")
      string(SUBSTRING "${hex}" ${pos} 2 byte)
      math(EXPR value "(${value} << 8) + 0x${byte}")
    endwhile()
    set(${out} "${value}" PARENT_SCOPE)
  endfunction()

  # The state file RetroArch wrote (RASTATE container, 13-RESEARCH 3d; read from the binary's
  # output file): bytes 0-7 RASTATE and 1, 'MEM ' and the size S, the core's magic at offset 16,
  # file size in [S+8, S+32]. When `frame` is not empty, the core's frame_number (offset 32 of the
  # core state, so 48 of the file) equals it.
  function(ra_state_container lane file frame)
    file(SIZE "${file}" file_size)
    math(EXPR size_low "${state_size} + 8")
    math(EXPR size_high "${state_size} + 32")
    if(file_size LESS size_low OR file_size GREATER size_high)
      message(FATAL_ERROR "[${lane}] ${file} is ${file_size} bytes, expected ${size_low} to ${size_high}")
    endif()
    file(READ "${file}" head HEX OFFSET 0 LIMIT 24)
    string(SUBSTRING "${head}" 0 16 rastate)
    string(SUBSTRING "${head}" 16 8 mem)
    string(SUBSTRING "${head}" 24 8 size_field)
    string(SUBSTRING "${head}" 32 16 magic)
    ra_le_value(field_value "${size_field}")
    if(NOT rastate STREQUAL "5241535441544501" OR NOT mem STREQUAL "4d454d20"
        OR NOT field_value EQUAL state_size OR NOT magic STREQUAL "894e54530d0a1a0a")
      message(FATAL_ERROR "[${lane}] ${file} is not an uncompressed RASTATE of the core's ${state_size}-byte state: ${head}")
    endif()
    if(NOT "${frame}" STREQUAL "")
      file(READ "${file}" frame_hex HEX OFFSET 48 LIMIT 8)
      ra_le_value(frame_value "${frame_hex}")
      if(NOT frame_value EQUAL frame)
        message(FATAL_ERROR "[${lane}] ${file} holds frame_number ${frame_value}, expected ${frame}")
      endif()
    endif()
    message(STATUS "[${lane}] state file ${file_size} bytes, S ${state_size}, magic ok")
  endfunction()

  # Auto-save at 150, then auto-load for 30 more frames. RetroArch 1.22.2 shows runner frame 179
  # after the load (150 + 30 - 1, measured: the -1 is RetroArch's own, deterministic).
  ra_state_lane(state-save 150 150 "savestate_auto_save=true")
  file(GLOB_RECURSE auto_states "${RA_DIR}/states/*${stem}.state.auto")
  list(LENGTH auto_states auto_count)
  if(NOT auto_count EQUAL 1)
    message(FATAL_ERROR "[state-save] expected one ${stem}.state.auto under ${RA_DIR}/states, found: ${auto_states}")
  endif()
  ra_state_container(state-save "${auto_states}" 150)
  if(MARKER_GATE)
    file(COPY_FILE "${auto_states}" "${RA_DIR}/retained-origin.state")
  endif()
  ra_state_outcome_checks(state-save "${RA_DIR}/retroarch-state-save.log")
  nesturbator_ra_log_check("${RA_DIR}/retroarch-state-save.log"
    "${CMAKE_CURRENT_LIST_DIR}/log_allowlist.txt" "${claim_level}" ON)
  ra_state_lane(state-load 179 30 "savestate_auto_load=true")
  ra_state_outcome_checks(state-load "${RA_DIR}/retroarch-state-load.log")
  nesturbator_ra_log_check("${RA_DIR}/retroarch-state-load.log"
    "${CMAKE_CURRENT_LIST_DIR}/log_allowlist.txt" "${claim_level}" ON)
  math(EXPR load_ahead_frame "179 + ${k_eff}")
  ra_state_lane(state-load-ahead ${load_ahead_frame} 30 "savestate_auto_load=true"
    "run_ahead_enabled=true")
  ra_state_outcome_checks(state-load-ahead "${RA_DIR}/retroarch-state-load-ahead.log")
  nesturbator_ra_log_check("${RA_DIR}/retroarch-state-load-ahead.log"
    "${CMAKE_CURRENT_LIST_DIR}/log_allowlist.txt" "${claim_level}" ON)

  if(MARKER_GATE)
    set(marker_observations)
    foreach(lane state-base state-ahead state-save state-load state-load-ahead)
      execute_process(COMMAND "${FRAME_INDEX}" --marker-observe "${ra_content}"
          "${RA_DIR}/calibration.txt" "${RA_DIR}/shot-${lane}.bmp"
        RESULT_VARIABLE observed OUTPUT_VARIABLE observation ERROR_VARIABLE observation_error)
      if(NOT observed EQUAL 0 OR NOT observation MATCHES "marker frame=([0-9]+) guest=")
        message(FATAL_ERROR "[${lane}] unqualified marker: ${observation}${observation_error}")
      endif()
      set(marker_${lane} "${CMAKE_MATCH_1}")
      string(APPEND marker_observations "${lane}: ${observation}")
    endforeach()
    math(EXPR measured_lead "${marker_state-ahead} - ${marker_state-base}")
    if(NOT marker_state-base EQUAL 120 OR NOT marker_state-save EQUAL 150 OR
        NOT marker_state-load EQUAL 179 OR NOT marker_state-load-ahead EQUAL load_ahead_frame OR
        NOT measured_lead EQUAL k_eff)
      message(FATAL_ERROR "marker host calibration disagrees with base/save/load/display lead")
    endif()
    execute_process(COMMAND "${FRAME_INDEX}" --marker-anchor "${ra_content}"
        "${RA_DIR}/calibration.txt" "${RA_DIR}/retained-origin.state" 150
      RESULT_VARIABLE anchored OUTPUT_VARIABLE anchor_output ERROR_VARIABLE anchor_error)
    file(WRITE "${RA_DIR}/anchor.txt" "${anchor_output}${anchor_error}")
    if(NOT anchored EQUAL 0)
      message(FATAL_ERROR "marker host native anchor failed: ${anchor_output}${anchor_error}")
    endif()
    file(SHA256 "${ra_core}" marker_core_sha)
    file(SHA256 "${INFO}" marker_info_sha)
    file(SHA256 "${RA_DIR}/retained-origin.state" marker_anchor_sha)
    file(READ "${RA_DIR}/calibration.txt" marker_calibration)
    execute_process(COMMAND git rev-parse HEAD WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
      RESULT_VARIABLE revision_result OUTPUT_VARIABLE source_revision OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(NOT revision_result EQUAL 0)
      message(FATAL_ERROR "marker gate cannot bind source commit identity")
    endif()
    set(config_identities)
    foreach(lane state-base state-ahead state-save state-load state-load-ahead)
      file(SHA256 "${RA_DIR}/${lane}.cfg" config_sha)
      string(APPEND config_identities "${lane}_config_sha256=${config_sha}\n")
      file(READ "${RA_DIR}/${lane}.cfg" cfg_text)
      string(REPLACE "${RA_DIR}" "<attempt>" cfg_text "${cfg_text}")
      file(WRITE "${RA_DIR}/${lane}-sanitized.cfg" "${cfg_text}")
    endforeach()
    file(WRITE "${RA_DIR}/marker-gate-success.txt"
      "marker-gate=pass\nsource_commit=${source_revision}\nversion=${measured_version}\ncontent_sha256=${supplied_marker_sha}\n"
      "core_sha256=${marker_core_sha}\ninfo_sha256=${marker_info_sha}\n"
      "anchor_sha256=${marker_anchor_sha}\ndisplay_lead=${measured_lead}\n"
      "${marker_calibration}${config_identities}${qualification_output}${marker_observations}${anchor_output}"
      "scope=qualification; exact network progression and speculative audio are separate proofs\n")
    message(STATUS "marker-gate pass: native origin 150, interval 4..10000, measured lead ${measured_lead}")
    return()
  endif()

  # --- The network-command session (D-24, D-25 as amended) ---
  if(NOT DEFINED SESSION_SCRIPT OR NOT EXISTS "${SESSION_SCRIPT}")
    message(FATAL_ERROR "run_retroarch.cmake's state mode needs -DSESSION_SCRIPT=<state_session.sh>")
  endif()
  file(REMOVE_RECURSE "${RA_DIR}/states")
  file(MAKE_DIRECTORY "${RA_DIR}/states")
  ra_write_lane_cfg("${RA_DIR}/state-net.cfg")
  snapshot(net_before)
  execute_process(COMMAND sh "${SESSION_SCRIPT}" "${retroarch}" "${RA_DIR}/state-net.cfg"
      "${ra_core}" "${ra_content}" "${RA_DIR}" "${RA_DIR}/retroarch-state-net.log"
    RESULT_VARIABLE net_result OUTPUT_VARIABLE net_output ERROR_VARIABLE net_output
    TIMEOUT 150)
  snapshot(net_after)
  if(NOT net_before STREQUAL net_after)
    message(FATAL_ERROR "[state-net] RetroArch changed the user's RetroArch directory")
  endif()
  if(NOT net_result EQUAL 0)
    message(FATAL_ERROR "[state-net] session exited with ${net_result}:\n${net_output}")
  endif()

  # Every screenshot to a BMP (colour management stripped, as in ra_session), then every
  # frame index in 1..1200 whose RGB equals it.
  set(net_tags pause-a pause-b adv-1 adv-2 adv-3 adv-4 adv-5 load-1 load-2 load-3 m1-a m1-b
    rew-0 rew-1 rew-2 rew-3 ahead-1)
  set(net_bmps)
  foreach(tag IN LISTS net_tags)
    if(NOT EXISTS "${RA_DIR}/shot-net-${tag}.png")
      message(FATAL_ERROR "[state-net] no screenshot ${tag}")
    endif()
    file(COPY_FILE "${RA_DIR}/shot-net-${tag}.png" "${RA_DIR}/shot-net-${tag}-raw.png")
    sips(--deleteColorManagementProperties shot-net-${tag}-raw.png)
    sips(-s format bmp shot-net-${tag}-raw.png --out shot-net-${tag}.bmp)
    list(APPEND net_bmps "${RA_DIR}/shot-net-${tag}.bmp")
  endforeach()
  execute_process(COMMAND "${FRAME_INDEX}" "${mode_rom}" 1200 ${net_bmps}
    RESULT_VARIABLE index_result OUTPUT_VARIABLE index_output ERROR_VARIABLE index_error)
  if(NOT index_result EQUAL 0)
    message(FATAL_ERROR "[state-net] frame_index found no frame for a screenshot:\n${index_output}${index_error}")
  endif()
  string(REPLACE "\n" ";" index_lines "${index_output}")
  foreach(line IN LISTS index_lines)
    if(line MATCHES "^(.*)/shot-net-([a-z0-9-]+)\\.bmp ([0-9 ]+)$")
      string(REPLACE " " ";" frames "${CMAKE_MATCH_3}")
      set(set_${CMAKE_MATCH_2} "${frames}")
    endif()
  endforeach()
  set(status_text)
  foreach(tag IN LISTS net_tags)
    if("${set_${tag}}" STREQUAL "")
      message(FATAL_ERROR "[state-net] no frame set parsed for ${tag}:\n${index_output}")
    endif()
    string(APPEND status_text " ${tag}={${set_${tag}}}")
  endforeach()
  message(STATUS "[state-net] identified sets:${status_text}")

  function(ra_set_min out)
    set(best "")
    foreach(x IN LISTS ARGN)
      if("${best}" STREQUAL "" OR x LESS best)
        set(best "${x}")
      endif()
    endforeach()
    set(${out} "${best}" PARENT_SCOPE)
  endfunction()
  function(ra_set_max out)
    set(best "")
    foreach(x IN LISTS ARGN)
      if("${best}" STREQUAL "" OR x GREATER best)
        set(best "${x}")
      endif()
    endforeach()
    set(${out} "${best}" PARENT_SCOPE)
  endfunction()
  # out = the elements x of `next` for which x - 1 is in `prev`; empty is a failure.
  function(ra_chain out what prev_name next_name)
    set(result)
    foreach(x IN LISTS ${next_name})
      math(EXPR before "${x} - 1")
      if(before IN_LIST ${prev_name})
        list(APPEND result ${x})
      endif()
    endforeach()
    if("${result}" STREQUAL "")
      message(FATAL_ERROR "[state-net] ${what}: {${${next_name}}} does not follow {${${prev_name}}} by one frame")
    endif()
    set(${out} "${result}" PARENT_SCOPE)
  endfunction()

  # Pause proof: two shots 1.1 s apart are the same picture.
  file(SHA256 "${RA_DIR}/shot-net-pause-a.bmp" pause_a)
  file(SHA256 "${RA_DIR}/shot-net-pause-b.bmp" pause_b)
  if(NOT pause_a STREQUAL pause_b)
    message(FATAL_ERROR "[state-net] the screen moved while paused: pause-a {${set_pause-a}} pause-b {${set_pause-b}}")
  endif()
  set(m0 ${set_pause-a})
  # Five advances, each one frame after the last.
  set(prev m0)
  foreach(n 1 2 3 4 5)
    ra_chain(chain_${n} "advance ${n}" ${prev} set_adv-${n})
    set(prev chain_${n})
  endforeach()
  # Load slot 0 puts the screen back at M0; each advance after it is one frame on.
  set(prev m0)
  foreach(n 1 2 3)
    ra_chain(load_chain_${n} "load then advance ${n}" ${prev} set_load-${n})
    set(prev load_chain_${n})
  endforeach()
  # Resume proof: the session ran for about 3 s with run-ahead off, then paused again.
  file(SHA256 "${RA_DIR}/shot-net-m1-a.bmp" m1_a)
  file(SHA256 "${RA_DIR}/shot-net-m1-b.bmp" m1_b)
  if(NOT m1_a STREQUAL m1_b)
    message(FATAL_ERROR "[state-net] the screen moved while paused after the resume: m1-a {${set_m1-a}} m1-b {${set_m1-b}}")
  endif()
  set(m1 ${set_m1-b})
  ra_set_min(m1_min ${m1})
  ra_set_max(load3_max ${load_chain_3})
  if(NOT m1_min GREATER load3_max)
    message(FATAL_ERROR "[state-net] the resume did not run: m1 {${m1}} is not after load-3 {${load_chain_3}}")
  endif()
  # Forty paced REWINDs (D-25 step 6 as amended: the bound is 20 frames, from the frame just
  # before REWIND).
  ra_set_max(rew0_max ${set_rew-0})
  math(EXPR rew_bound "${m1_min} - 20")
  if(rew0_max GREATER rew_bound)
    message(FATAL_ERROR "[state-net] 40 REWINDs moved the screen from {${m1}} only to {${set_rew-0}}, expected at most ${rew_bound}")
  endif()
  set(prev set_rew-0)
  foreach(n 1 2 3)
    ra_chain(rew_chain_${n} "rewind then advance ${n}" ${prev} set_rew-${n})
    set(prev rew_chain_${n})
  endforeach()
  set(m3 ${rew_chain_3})
  # Run-ahead (D-25 step 7 as amended): the second save was taken at rew-0, because RetroArch
  # 1.22.2 ignores SAVE_STATE after a FRAMEADVANCE (measured), not at m3. After the toggle and the
  # load, one advance shows m + 1 + k_eff for some m in rew-0.
  set(ahead_ok FALSE)
  foreach(m IN LISTS set_rew-0)
    math(EXPR want "${m} + 1 + ${k_eff}")
    if(want IN_LIST set_ahead-1)
      set(ahead_ok TRUE)
    endif()
  endforeach()
  if(NOT ahead_ok)
    message(FATAL_ERROR "[state-net] ahead-1 {${set_ahead-1}} holds no frame m + 1 + ${k_eff} for m in rew-0 {${set_rew-0}}")
  endif()
  file(GLOB_RECURSE net_states "${RA_DIR}/states/*/${stem}.state")
  list(LENGTH net_states net_state_count)
  if(NOT net_state_count EQUAL 1)
    message(FATAL_ERROR "[state-net] expected one ${stem}.state, found: ${net_states}")
  endif()
  ra_state_container(state-net "${net_states}" "")
  ra_state_outcome_checks(state-net "${RA_DIR}/retroarch-state-net.log")
  nesturbator_ra_log_check("${RA_DIR}/retroarch-state-net.log"
    "${CMAKE_CURRENT_LIST_DIR}/log_allowlist.txt" "${claim_level}" ON)
  message(STATUS "[state-net] pause, advance, load, resume, rewind and run-ahead (lead ${k_eff}) hold")
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
