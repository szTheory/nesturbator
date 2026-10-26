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
# run-ahead, 179 + k_eff). Exact network progression uses HOSTED_MARKER=ON on generated marker
# content and the shared production verdict; it does not infer command order from picture sets.
#
# Needs a logged-in GUI session, because RetroArch opens a window.
cmake_minimum_required(VERSION 3.25)

function(hosted_write_fault_nc output real_nc cat_path)
  string(REPLACE "@CAT@" "${cat_path}" hosted_nc_source [=[
#!/bin/sh
set -u
CAT="@CAT@"
REAL_NC="@REAL_NC@"
REQUEST_FILE="${HOSTED_FAULT_STATE:?}.request.$$"
CHILD_PID=
cleanup() {
    status=$?
    trap - EXIT HUP INT TERM
    if [ -n "$CHILD_PID" ]; then
        # An interrupted mutation must not linger and send after the session fails.
        kill -KILL "$CHILD_PID" 2>/dev/null || true
        wait "$CHILD_PID" 2>/dev/null || true
        CHILD_PID=
    fi
    /bin/rm -f "$REQUEST_FILE"
    exit "$status"
}
trap cleanup EXIT
trap 'exit 129' HUP
trap 'exit 130' INT
trap 'exit 143' TERM

hosted_monotonic_ms() {
    /usr/bin/perl -MTime::HiRes=clock_gettime,CLOCK_MONOTONIC -e 'printf "%.0f\n",clock_gettime(CLOCK_MONOTONIC)*1000'
}

"$CAT" >"$REQUEST_FILE" || exit 1
payload=$("$CAT" "$REQUEST_FILE")
send_real() {
    SEND_START_MS=$(hosted_monotonic_ms) || return 1
    "$REAL_NC" "$@" <"$REQUEST_FILE" &
    CHILD_PID=$!
    if wait "$CHILD_PID"; then
        CHILD_PID=
        SEND_END_MS=$(hosted_monotonic_ms) || return 1
        return 0
    else
        send_status=$?
        # A trapped signal exits through cleanup while CHILD_PID is still set.
        # A normal nonzero wait has already reaped this exact child.
        CHILD_PID=
        SEND_END_MS=$(hosted_monotonic_ms) || return 1
        return "$send_status"
    fi
}

if [ "$payload" = FRAMEADVANCE ] &&
   { [ "${HOSTED_MARKER_FAULT:-}" = drop-advance ] ||
     [ "${HOSTED_MARKER_FAULT:-}" = double-advance ]; } &&
   /bin/mkdir "${HOSTED_FAULT_STATE:?}" 2>/dev/null; then
    if [ "$HOSTED_MARKER_FAULT" = drop-advance ]; then
        /usr/bin/printf 'fault=drop-advance\ntrigger=FRAMEADVANCE\ninjected_count=1\n' >"$HOSTED_FAULT_LOG"
        exit 0
    fi
    /usr/bin/printf 'fault=double-advance\ntrigger=FRAMEADVANCE\ninjected_count=1\n' >"$HOSTED_FAULT_LOG"
    send_real "$@" || exit $?
    first_send_start_ms=$SEND_START_MS
    first_send_end_ms=$SEND_END_MS
    # Separate the duplicate beyond several host-frame intervals so the
    # released host can process each one-shot UDP request independently.
    /usr/bin/perl -MTime::HiRes=sleep -e 'sleep 0.25' || exit $?
    second_send_ready_ms=$(hosted_monotonic_ms) || exit 1
    send_real "$@" || exit $?
    second_send_start_ms=$SEND_START_MS
    second_send_end_ms=$SEND_END_MS
    inter_send_gap_ms=$((second_send_start_ms-first_send_end_ms))
    /usr/bin/printf 'forwarded_send_count=2\nfirst_send_start_ms=%s\nfirst_send_end_ms=%s\nsecond_send_ready_ms=%s\nsecond_send_start_ms=%s\nsecond_send_end_ms=%s\ninter_send_gap_ms=%s\n' \
        "$first_send_start_ms" "$first_send_end_ms" "$second_send_ready_ms" \
        "$second_send_start_ms" "$second_send_end_ms" "$inter_send_gap_ms" >>"$HOSTED_FAULT_LOG"
    exit 0
fi
send_real "$@"
]=])
  string(REPLACE "@REAL_NC@" "${real_nc}" hosted_nc_source "${hosted_nc_source}")
  file(WRITE "${output}" "${hosted_nc_source}")
  file(CHMOD "${output}" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endfunction()

# A shell control asks for the same generated action wrapper used by the
# hosted lane, without requiring RetroArch or a GUI session.
if(DEFINED HOSTED_FAULT_WRAPPER_SELFTEST_OUTPUT)
  if(NOT DEFINED HOSTED_FAULT_WRAPPER_REAL_NC OR
      "${HOSTED_FAULT_WRAPPER_REAL_NC}" STREQUAL "")
    message(FATAL_ERROR "fault wrapper self-test needs HOSTED_FAULT_WRAPPER_REAL_NC")
  endif()
  find_program(HOSTED_FAULT_WRAPPER_CAT NAMES cat)
  if(NOT HOSTED_FAULT_WRAPPER_CAT)
    message(FATAL_ERROR "fault wrapper self-test needs cat")
  endif()
  hosted_write_fault_nc("${HOSTED_FAULT_WRAPPER_SELFTEST_OUTPUT}"
    "${HOSTED_FAULT_WRAPPER_REAL_NC}" "${HOSTED_FAULT_WRAPPER_CAT}")
  return()
endif()

# The required hosted marker job uses one root-relative entry point so it cannot
# accidentally combine a marker ROM, core, or pinned app from different builds.
if(HOSTED_MARKER)
  if(NOT CMAKE_HOST_APPLE)
    message(FATAL_ERROR "HOSTED_MARKER runs only on the pinned macOS host")
  endif()
  foreach(var BUILD_DIR HOST_ROOT)
    if(NOT DEFINED ${var} OR "${${var}}" STREQUAL "")
      message(FATAL_ERROR "HOSTED_MARKER needs -D${var}=...")
    endif()
  endforeach()
  get_filename_component(project_root "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
  # frame_index prints the state format and behaviour revision it was built with, so the
  # expected anchor line is read from the public header rather than written here.
  foreach(macro STATE_FORMAT_VERSION BEHAVIOUR_REVISION)
    file(STRINGS "${project_root}/include/nesturbator.h" macro_line
      REGEX "^#define NESTURBATOR_${macro} [0-9]+$")
    list(LENGTH macro_line macro_count)
    if(NOT macro_count EQUAL 1)
      message(FATAL_ERROR "HOSTED_MARKER: NESTURBATOR_${macro} is not defined exactly once in include/nesturbator.h")
    endif()
    string(REGEX REPLACE "^.* " "" hosted_${macro} "${macro_line}")
  endforeach()
  set(HOSTED_ANCHOR_LINE
    "native-valid format=${hosted_STATE_FORMAT_VERSION} revision=${hosted_BEHAVIOUR_REVISION} envelope=RASTATE1/MEM/pad8/END")
  cmake_path(ABSOLUTE_PATH BUILD_DIR BASE_DIRECTORY "${project_root}" NORMALIZE
    OUTPUT_VARIABLE hosted_build_root)
  cmake_path(ABSOLUTE_PATH HOST_ROOT BASE_DIRECTORY "${project_root}" NORMALIZE
    OUTPUT_VARIABLE hosted_root)
  set(HOSTED_MARKER_ROOT "${hosted_root}/marker-progression")
  if(EXISTS "${HOSTED_MARKER_ROOT}")
    message(FATAL_ERROR
      "HOSTED_MARKER evidence already exists at ${HOSTED_MARKER_ROOT}; use a fresh HOST_ROOT to preserve the first run")
  endif()
  file(MAKE_DIRECTORY "${HOSTED_MARKER_ROOT}")
  set(HOSTED_BUILD_ROOT "${hosted_build_root}")
  set(HOSTED_ROOT "${hosted_root}")
  set(CORE "${hosted_build_root}/libretro/nesturbator_libretro.dylib")
  set(INFO "${project_root}/libretro/nesturbator_libretro.info")
  set(RUNNER "${hosted_build_root}/runner/nesturbator-run")
  set(COMPARE "${hosted_build_root}/tests/retroarch/compare_frame")
  set(FRAME_INDEX "${hosted_build_root}/tests/retroarch/frame_index")
  set(STATE_ROM "${hosted_build_root}/tests/retroarch/marker.nes")
  set(TEMPLATE "${CMAKE_CURRENT_LIST_DIR}/test.cfg.in")
  set(STATE_TEMPLATE "${CMAKE_CURRENT_LIST_DIR}/state.cfg.in")
  set(SESSION_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/state_session.sh")
  set(RETROARCH "${hosted_root}/RetroArch.app/Contents/MacOS/RetroArch")
  set(VERSION_PLIST "${hosted_root}/RetroArch.app/Contents/Info.plist")
  set(ASSET_EVIDENCE "${hosted_root}/asset-evidence.txt")
  set(EXPECTED_VERSION "1.22.2")
  set(HOSTED_PIN_SHA "81b79121ba26d539064ae13b4d0419a120c3d165afbe656cf5f5412b15fdb434")
  set(RA_DIR "${HOSTED_MARKER_ROOT}/qualification")
  set(MARKER_GATE ON)
  file(WRITE "${HOSTED_MARKER_ROOT}/hosted-evidence.txt" "status=started\n")
  foreach(path CORE INFO RUNNER COMPARE FRAME_INDEX STATE_ROM TEMPLATE STATE_TEMPLATE SESSION_SCRIPT
      RETROARCH VERSION_PLIST ASSET_EVIDENCE)
    if(NOT EXISTS "${${path}}" OR IS_DIRECTORY "${${path}}")
      message(FATAL_ERROR "HOSTED_MARKER input is missing or not a file: ${path}=${${path}}")
    endif()
  endforeach()
  find_program(HOSTED_REAL_NC NAMES nc)
  if(NOT HOSTED_REAL_NC)
    message(FATAL_ERROR "HOSTED_MARKER needs the system nc used by the bounded driver")
  endif()
  find_program(HOSTED_CAT NAMES cat)
  if(NOT HOSTED_CAT)
    message(FATAL_ERROR "HOSTED_MARKER needs cat to retain the one-shot request")
  endif()
  file(READ "${ASSET_EVIDENCE}" hosted_asset_text)
  if(NOT hosted_asset_text MATCHES "expected SHA-256: ${HOSTED_PIN_SHA}" OR
      NOT hosted_asset_text MATCHES "actual SHA-256: ${HOSTED_PIN_SHA}" OR
      NOT hosted_asset_text MATCHES "RetroArch version: ${EXPECTED_VERSION}")
    message(FATAL_ERROR "HOSTED_MARKER asset evidence does not match the required pin")
  endif()
endif()

if(HOSTED_MARKER)
  function(hosted_monotonic_ms out)
    execute_process(
      COMMAND perl -MTime::HiRes=clock_gettime,CLOCK_MONOTONIC -e "printf \"%.0f\\n\",clock_gettime(CLOCK_MONOTONIC)*1000"
      RESULT_VARIABLE clock_result OUTPUT_VARIABLE clock_output ERROR_VARIABLE clock_error)
    string(STRIP "${clock_output}" clock_output)
    if(NOT clock_result EQUAL 0 OR NOT clock_output MATCHES "^[0-9]+$")
      message(FATAL_ERROR "HOSTED_MARKER could not read the monotonic timer: ${clock_error}")
    endif()
    set(${out} "${clock_output}" PARENT_SCOPE)
  endfunction()
endif()

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
if(NOT HOSTED_MARKER)
  file(REMOVE_RECURSE "${RA_DIR}")
endif()
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
  if(HOSTED_MARKER)
    hosted_monotonic_ms(qualification_start_ms)
  endif()
  execute_process(COMMAND "${FRAME_INDEX}" --marker-qualify "${ra_content}"
      "${RA_DIR}/calibration.txt" RESULT_VARIABLE qualified
    OUTPUT_VARIABLE qualification_output ERROR_VARIABLE qualification_error TIMEOUT 600)
  if(HOSTED_MARKER)
    hosted_monotonic_ms(qualification_end_ms)
    math(EXPR qualification_elapsed_ms "${qualification_end_ms} - ${qualification_start_ms}")
  endif()
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
  set(pin_sha "${HOSTED_PIN_SHA}")
  if(NOT pin_sha)
    set(pin_sha "81b79121ba26d539064ae13b4d0419a120c3d165afbe656cf5f5412b15fdb434")
  endif()
  if(NOT asset_text MATCHES "expected SHA-256: ${pin_sha}" OR
      NOT asset_text MATCHES "actual SHA-256: ${pin_sha}" OR
      NOT asset_text MATCHES "RetroArch version: 1.22.2")
    message(FATAL_ERROR "marker gate asset evidence does not match the required pin")
  endif()
  file(COPY_FILE "${ASSET_EVIDENCE}" "${RA_DIR}/asset-evidence.txt")

  if(HOSTED_MARKER)
    function(hosted_sanitize_value input out)
      set(value "${input}")
      string(REPLACE "${HOSTED_BUILD_ROOT}" "<build>" value "${value}")
      string(REPLACE "${HOSTED_ROOT}" "<retroarch>" value "${value}")
      string(REPLACE "${project_root}" "<workspace>" value "${value}")
      set(${out} "${value}" PARENT_SCOPE)
    endfunction()

    # These wrappers only inject a fault into one real local network send, or
    # into the declared origin in one real production verdict. The host, images,
    # native anchors and verdict executable remain the pinned production ones.
    set(hosted_fault_tools "${HOSTED_MARKER_ROOT}/fault-tools")
    file(MAKE_DIRECTORY "${hosted_fault_tools}")
    hosted_write_fault_nc("${hosted_fault_tools}/nc-one-shot.sh" "${HOSTED_REAL_NC}" "${HOSTED_CAT}")
    set(hosted_index_source [=[
#!/bin/sh
set -eu
if [ "${HOSTED_MARKER_FAULT:-}" = valid-wrong-origin ] &&
   [ "${1:-}" = --marker-verdict-prefix ]; then
    record=${4:?}
    boundary_count=$(/usr/bin/sed -n 's/^boundary_count=//p' "$record")
    if [ "$boundary_count" = 13 ]; then
        /bin/cp "$record" "$record.hosted-fault-original"
        /usr/bin/perl -0pi -e '
            my ($identity) = /^original_anchor_identity=([0-9a-f]{16})$/m;
            my ($frame) = /^original_anchor_frame=([0-9]+)$/m;
            die "missing valid original anchor identity" unless defined $identity;
            die "missing valid original anchor frame" unless defined $frame;
            my $changed_identity = s{(boundary\nid=13\n.*?\nanchor_identity=)[0-9a-f]{16}}{${1} . $identity}se;
            my $changed_frame = s{(boundary\nid=13\n.*?\nanchor_frame=)[0-9]+}{${1} . $frame}se;
            die "wrong-origin mutation did not affect exactly boundary 13" unless
                $changed_identity == 1 && $changed_frame == 1;
        ' "$record"
        /usr/bin/printf 'fault=valid-wrong-origin\nboundary=13\nanchor=validated-original\ninjected_count=1\n' >"${HOSTED_FAULT_LOG:?}"
    fi
fi
exec "${HOSTED_REAL_FRAME_INDEX:?}" "$@"
]=])
    file(WRITE "${hosted_fault_tools}/frame-index-wrong-origin.sh" "${hosted_index_source}")
    file(CHMOD "${hosted_fault_tools}/frame-index-wrong-origin.sh" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
      GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

    set(marker_core_sha "")
    set(marker_info_sha "")
    file(SHA256 "${CORE}" marker_core_sha)
    file(SHA256 "${INFO}" marker_info_sha)
    execute_process(COMMAND git rev-parse HEAD WORKING_DIRECTORY "${project_root}"
      RESULT_VARIABLE marker_revision_result OUTPUT_VARIABLE marker_revision OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(NOT marker_revision_result EQUAL 0)
      message(FATAL_ERROR "HOSTED_MARKER cannot bind the source commit identity")
    endif()
    file(READ "${RA_DIR}/calibration.txt" marker_calibration)
    file(APPEND "${HOSTED_MARKER_ROOT}/hosted-evidence.txt"
      "qualification=pass\nsource_commit=${marker_revision}\npinned_version=${measured_version}\n"
      "retroarch_dmg_sha256=${pin_sha}\ncontent_sha256=${supplied_marker_sha}\n"
      "core_sha256=${marker_core_sha}\ninfo_sha256=${marker_info_sha}\n"
      "frame_window=4..10000\ncalibration_record=${marker_calibration}"
      "qualification_ms=${qualification_elapsed_ms}\n")

    function(hosted_marker_session run_name fault expected_reason expected_verdict expected_boundaries expected_sends)
      set(run_root "${HOSTED_MARKER_ROOT}/${run_name}")
      if(EXISTS "${run_root}")
        message(FATAL_ERROR "HOSTED_MARKER refuses to replace first-run evidence at ${run_root}")
      endif()
      set(RA_DIR "${run_root}")
      foreach(sub assets cache cheats config content cores database/cursors database/rdb
          downloads filters/audio filters/video fonts info logs overlays playlists records
          records_config remaps runtime saves shaders shots states system thumbnails wallpapers autoconfig)
        file(MAKE_DIRECTORY "${run_root}/${sub}")
      endforeach()
      configure_file("${TEMPLATE}" "${run_root}/test.cfg" @ONLY)
      file(READ "${run_root}/test.cfg" base_config)
      file(READ "${STATE_TEMPLATE}" state_config)
      file(WRITE "${run_root}/marker-session.cfg" "${base_config}\n${state_config}")
      file(COPY "${INFO}" DESTINATION "${run_root}/info")
      file(COPY_FILE "${CORE}" "${run_root}/cores/nesturbator_libretro.dylib")
      file(COPY_FILE "${ra_content}" "${run_root}/content/marker.nes")

      set(session_frame_index "${FRAME_INDEX}")
      set(session_env "STATE_SESSION_RECORD_ROOT=${project_root}"
        "STATE_SESSION_REAL_NC=${HOSTED_REAL_NC}")
      if(NOT fault STREQUAL "")
        list(APPEND session_env
          "HOSTED_MARKER_FAULT=${fault}"
          "HOSTED_FAULT_STATE=${run_root}/fault-seen"
          "HOSTED_FAULT_LOG=${run_root}/fault-injection.txt")
        if(fault STREQUAL "drop-advance" OR fault STREQUAL "double-advance")
          list(APPEND session_env "STATE_SESSION_NC=${hosted_fault_tools}/nc-one-shot.sh")
          if(fault STREQUAL "double-advance")
            # Let the real state file publish under the virtual host before the
            # negative action; this remains inside the unchanged 30 s operation cap.
            list(APPEND session_env "STATE_SESSION_PUBLICATION_MS=10000")
          endif()
        elseif(fault STREQUAL "valid-wrong-origin")
          set(session_frame_index "${hosted_fault_tools}/frame-index-wrong-origin.sh")
          list(APPEND session_env "HOSTED_REAL_FRAME_INDEX=${FRAME_INDEX}")
        else()
          message(FATAL_ERROR "HOSTED_MARKER has no production injection for ${fault}")
        endif()
      endif()

      # Run 38633432440 on f8831c2 reached all 16 prefix boundaries and their
      # production PREFIX_OK verdict at 232.775 s, then exhausted its 240 s attempt
      # while awaiting QUIT (235.161 s elapsed, 4.824 s left). The complete verdict
      # runs after QUIT and host exit. That incomplete sample projected 300.161 s
      # for QUIT, the complete verdict and 5 s cleanup; its 320 s attempt estimate
      # was subsequently verified by exact-head run 38638052203 on e2397db: the
      # intended full path returned VERDICT_OK in 225.373 s (225.214 s attempt).
      # In that run, valid-wrong-origin used the 120 s default and failed at
      # ATTEMPT_DEADLINE after boundary 11 (97.291 s) and 32 of 40 rewind sends
      # (115.107 s, 4.885 s remaining). Its required boundary-13 ORIGIN control
      # needs the remaining rewinds, boundary-12 capture/native anchor/PREFIX_OK,
      # and one advance/capture/production verdict. Reuse the verified finite
      # 320 s attempt and 350 s outer driver envelope for this shorter path. Keep
      # the production driver's 120 s default, 30 s operation limits, exact 70
      # one-shot sends and semantic prerequisites unchanged in every mode.
      set(driver_timeout 150)
      if("${run_name}" STREQUAL "intended" OR "${run_name}" STREQUAL "fault-valid-wrong-origin")
        list(APPEND session_env "STATE_SESSION_ATTEMPT_MS=320000")
        set(driver_timeout 350)
      endif()

      hosted_monotonic_ms(run_start_ms)
      set(driver_command "${CMAKE_COMMAND}" -E env)
      list(APPEND driver_command ${session_env} sh "${SESSION_SCRIPT}"
        "${RETROARCH}" "${run_root}/marker-session.cfg"
        "${run_root}/cores/nesturbator_libretro.dylib" "${run_root}/content/marker.nes"
        "${run_root}" "${run_root}/retroarch-state-net.log" "${session_frame_index}"
        "${RA_DIR}/../qualification/calibration.txt")
      # The calibration path is stable across every attempt and stays within the
      # marker evidence root. The driver records all owned artifacts relatively.
      execute_process(COMMAND ${driver_command}
        RESULT_VARIABLE driver_result OUTPUT_VARIABLE driver_stdout ERROR_VARIABLE driver_stderr
        TIMEOUT "${driver_timeout}" WORKING_DIRECTORY "${project_root}")
      hosted_monotonic_ms(run_end_ms)
      math(EXPR driver_elapsed_ms "${run_end_ms} - ${run_start_ms}")

      hosted_sanitize_value("${driver_stdout}" safe_stdout)
      hosted_sanitize_value("${driver_stderr}" safe_stderr)
      file(WRITE "${run_root}/driver.stdout.txt" "${safe_stdout}")
      file(WRITE "${run_root}/driver.stderr.txt" "${safe_stderr}")
      if(EXISTS "${run_root}/marker-session.cfg")
        file(READ "${run_root}/marker-session.cfg" runtime_config)
        hosted_sanitize_value("${runtime_config}" safe_config)
        file(WRITE "${run_root}/marker-session.sanitized.cfg" "${safe_config}")
        file(REMOVE "${run_root}/marker-session.cfg" "${run_root}/test.cfg")
      endif()

      file(GLOB attempt_dirs LIST_DIRECTORIES TRUE "${run_root}/attempts/attempt-*")
      list(LENGTH attempt_dirs attempt_count)
      if(NOT attempt_count EQUAL 1)
        file(APPEND "${HOSTED_MARKER_ROOT}/hosted-evidence.txt"
          "${run_name}=failed result=${driver_result} reason=ATTEMPT_TREE_COUNT count=${attempt_count} elapsed_ms=${driver_elapsed_ms}\n")
        message(FATAL_ERROR "HOSTED_MARKER ${run_name} expected one retained attempt tree, found ${attempt_count}: ${safe_stderr}")
      endif()
      list(GET attempt_dirs 0 attempt_dir)
      if(NOT EXISTS "${attempt_dir}/journal.tsv" OR NOT EXISTS "${attempt_dir}/boundaries.tsv")
        message(FATAL_ERROR "HOSTED_MARKER ${run_name} failed before retaining its journal and boundaries: ${safe_stderr}")
      endif()
      file(STRINGS "${attempt_dir}/journal.tsv" journal_rows)
      list(LENGTH journal_rows journal_count)
      if(journal_count LESS 2)
        file(APPEND "${HOSTED_MARKER_ROOT}/hosted-evidence.txt"
          "${run_name}=failed result=${driver_result} reason=NO_TERMINAL_JOURNAL elapsed_ms=${driver_elapsed_ms}\n")
        message(FATAL_ERROR "HOSTED_MARKER ${run_name} retained no terminal journal row")
      endif()
      math(EXPR journal_last_index "${journal_count} - 1")
      list(GET journal_rows ${journal_last_index} journal_last_row)
      string(REPLACE "\t" ";" journal_fields "${journal_last_row}")
      list(LENGTH journal_fields journal_field_count)
      if(NOT journal_field_count EQUAL 11)
        message(FATAL_ERROR "HOSTED_MARKER ${run_name} journal row is malformed: ${journal_last_row}")
      endif()
      list(GET journal_fields 3 attempt_elapsed_ms)
      list(GET journal_fields 10 terminal_reason)
      list(REMOVE_AT journal_rows 0)
      set(actual_pause 0)
      set(actual_screenshot 0)
      set(actual_save 0)
      set(actual_advance 0)
      set(actual_load 0)
      set(actual_runahead 0)
      set(actual_rewind 0)
      set(actual_quit 0)
      set(cleanup_sends 0)
      set(driver_sends 0)
      foreach(row IN LISTS journal_rows)
        string(REPLACE "\t" ";" fields "${row}")
        list(GET fields 7 command)
        if(command STREQUAL "PAUSE_TOGGLE")
          math(EXPR actual_pause "${actual_pause} + 1")
          math(EXPR driver_sends "${driver_sends} + 1")
        elseif(command STREQUAL "SCREENSHOT")
          math(EXPR actual_screenshot "${actual_screenshot} + 1")
          math(EXPR driver_sends "${driver_sends} + 1")
        elseif(command STREQUAL "SAVE_STATE")
          math(EXPR actual_save "${actual_save} + 1")
          math(EXPR driver_sends "${driver_sends} + 1")
        elseif(command STREQUAL "FRAMEADVANCE")
          math(EXPR actual_advance "${actual_advance} + 1")
          math(EXPR driver_sends "${driver_sends} + 1")
        elseif(command STREQUAL "LOAD_STATE_SLOT 0")
          math(EXPR actual_load "${actual_load} + 1")
          math(EXPR driver_sends "${driver_sends} + 1")
        elseif(command STREQUAL "RUNAHEAD_TOGGLE")
          math(EXPR actual_runahead "${actual_runahead} + 1")
          math(EXPR driver_sends "${driver_sends} + 1")
        elseif(command STREQUAL "REWIND")
          math(EXPR actual_rewind "${actual_rewind} + 1")
          math(EXPR driver_sends "${driver_sends} + 1")
        elseif(command STREQUAL "QUIT")
          math(EXPR actual_quit "${actual_quit} + 1")
          math(EXPR driver_sends "${driver_sends} + 1")
        elseif(command STREQUAL "QUIT-at-most-once")
          math(EXPR cleanup_sends "${cleanup_sends} + 1")
        endif()
      endforeach()
      if(NOT driver_sends EQUAL expected_sends)
        message(FATAL_ERROR "HOSTED_MARKER ${run_name} expected ${expected_sends} one-shot sends, got ${driver_sends}")
      endif()
      file(STRINGS "${attempt_dir}/boundaries.tsv" boundary_rows)
      list(LENGTH boundary_rows boundary_count)
      if(fault STREQUAL "" OR fault STREQUAL "drop-advance" OR fault STREQUAL "double-advance" OR
          fault STREQUAL "valid-wrong-origin")
        set(startup_evidence "${attempt_dir}/startup-captures.tsv")
        if(NOT EXISTS "${startup_evidence}")
          message(FATAL_ERROR "HOSTED_MARKER ${run_name} has no retained startup capture evidence")
        endif()
        file(STRINGS "${startup_evidence}" startup_rows)
        list(LENGTH startup_rows startup_row_count)
        if(NOT startup_row_count EQUAL 3)
          message(FATAL_ERROR "HOSTED_MARKER ${run_name} expected two retained startup observations, got ${startup_row_count} rows")
        endif()
        list(GET startup_rows 1 startup_first)
        list(GET startup_rows 2 startup_second)
        string(REPLACE "\t" ";" startup_first_fields "${startup_first}")
        string(REPLACE "\t" ";" startup_second_fields "${startup_second}")
        list(LENGTH startup_first_fields startup_first_count)
        list(LENGTH startup_second_fields startup_second_count)
        if(NOT startup_first_count EQUAL 6 OR NOT startup_second_count EQUAL 6)
          message(FATAL_ERROR "HOSTED_MARKER ${run_name} startup observations are malformed")
        endif()
        list(GET startup_first_fields 0 startup_first_tag)
        list(GET startup_first_fields 1 startup_first_relation)
        list(GET startup_first_fields 2 startup_first_ordinal)
        list(GET startup_first_fields 4 startup_first_path)
        list(GET startup_second_fields 0 startup_second_tag)
        list(GET startup_second_fields 1 startup_second_relation)
        list(GET startup_second_fields 2 startup_second_ordinal)
        list(GET startup_second_fields 4 startup_second_path)
        if(NOT startup_first_tag STREQUAL "startup-forward-01" OR
            NOT startup_first_relation STREQUAL "baseline" OR NOT startup_first_ordinal STREQUAL "1" OR
            NOT startup_second_tag STREQUAL "startup-forward-02" OR
            NOT startup_second_relation STREQUAL "strictly-forward-from-startup-forward-01" OR
            NOT startup_second_ordinal STREQUAL "2" OR
            NOT EXISTS "${attempt_dir}/${startup_first_path}" OR NOT EXISTS "${attempt_dir}/${startup_second_path}")
          message(FATAL_ERROR "HOSTED_MARKER ${run_name} lacks two independently retained startup SCREENSHOT sends")
        endif()
      endif()
      file(SHA256 "${attempt_dir}/session.sanitized.cfg" config_sha)

      if(fault STREQUAL "")
        if(NOT driver_stdout MATCHES "sends=${driver_sends}")
          message(FATAL_ERROR "HOSTED_MARKER intended output disagrees with its journal send count: ${safe_stdout}")
        endif()
        if(NOT actual_pause EQUAL 3 OR NOT actual_screenshot EQUAL 18 OR
            NOT actual_save EQUAL 2 OR NOT actual_advance EQUAL 12 OR
            NOT actual_load EQUAL 2 OR NOT actual_runahead EQUAL 1 OR
            NOT actual_rewind EQUAL 40 OR NOT actual_quit EQUAL 1)
          message(FATAL_ERROR
            "HOSTED_MARKER intended send counts differ: pause=${actual_pause}, screenshot=${actual_screenshot}, save=${actual_save}, advance=${actual_advance}, load=${actual_load}, runahead=${actual_runahead}, rewind=${actual_rewind}, quit=${actual_quit}")
        endif()
        if(NOT "${driver_result}" STREQUAL "0" OR
            NOT driver_stdout MATCHES "progression boundaries=16 mode=complete verdict=VERDICT_OK" OR
            NOT EXISTS "${attempt_dir}/original-anchor.state" OR
            NOT EXISTS "${attempt_dir}/rewound-anchor.state" OR
            NOT EXISTS "${attempt_dir}/retroarch.sanitized.log" OR
            NOT terminal_reason STREQUAL "VERDICT_OK" OR NOT boundary_count EQUAL 16)
          message(FATAL_ERROR "HOSTED_MARKER intended sequence failed: result=${driver_result} reason=${terminal_reason} boundaries=${boundary_count} elapsed_ms=${driver_elapsed_ms}\n${safe_stdout}\n${safe_stderr}")
        endif()
        file(READ "${attempt_dir}/progression-session.rec" complete_record)
        if(NOT complete_record MATCHES "mode=complete" OR NOT complete_record MATCHES "terminal_exit=0" OR
            NOT complete_record MATCHES "boundary_count=16")
          message(FATAL_ERROR "HOSTED_MARKER intended sequence has no complete terminal evidence record")
        endif()
        file(APPEND "${HOSTED_MARKER_ROOT}/hosted-evidence.txt"
          "intended=pass\nintended_elapsed_ms=${driver_elapsed_ms}\n"
          "intended_attempt_elapsed_ms=${attempt_elapsed_ms}\nintended_boundaries=${boundary_count}\n"
          "intended_terminal_reason=${terminal_reason}\nintended_sends=${driver_sends}\ncleanup_sends=${cleanup_sends}\n"
          "startup_captures=2\nstartup_screenshot_send_ordinals=1,2\n"
          "intended_commands=PAUSE_TOGGLE:${actual_pause},SCREENSHOT:${actual_screenshot},SAVE_STATE:${actual_save},FRAMEADVANCE:${actual_advance},LOAD_STATE_SLOT_0:${actual_load},RUNAHEAD_TOGGLE:${actual_runahead},REWIND:${actual_rewind},QUIT:${actual_quit}\n"
          "intended_config_sha256=${config_sha}\n")
      else()
        if("${driver_result}" STREQUAL "0" OR NOT terminal_reason STREQUAL expected_reason OR
            NOT boundary_count EQUAL expected_boundaries OR NOT cleanup_sends EQUAL 1)
          message(FATAL_ERROR "HOSTED_MARKER ${run_name} expected ${expected_reason} at ${expected_boundaries} boundaries; got result=${driver_result} reason=${terminal_reason} boundaries=${boundary_count}\n${safe_stdout}\n${safe_stderr}")
        endif()
        if(fault STREQUAL "drop-advance" OR fault STREQUAL "double-advance")
          set(oracle_file "${attempt_dir}/oracle-prefix-3.out")
        else()
          set(oracle_file "${attempt_dir}/oracle-prefix-13.out")
        endif()
        if(NOT EXISTS "${oracle_file}")
          message(FATAL_ERROR "HOSTED_MARKER ${run_name} did not invoke the production verdict")
        endif()
        file(READ "${oracle_file}" oracle_output)
        if(NOT oracle_output MATCHES "verdict=${expected_verdict}")
          message(FATAL_ERROR "HOSTED_MARKER ${run_name} expected production ${expected_verdict}, got: ${oracle_output}")
        endif()
        if(NOT EXISTS "${run_root}/fault-injection.txt")
          message(FATAL_ERROR "HOSTED_MARKER ${run_name} never reached its requested one-shot injection")
        endif()
        file(READ "${run_root}/fault-injection.txt" injection_record)
        if(NOT injection_record MATCHES "injected_count=1")
          message(FATAL_ERROR "HOSTED_MARKER ${run_name} injection count is not exactly one")
        endif()
        if(fault STREQUAL "double-advance")
          file(STRINGS "${run_root}/fault-injection.txt" forwarded_lines
            REGEX "^forwarded_send_count=[0-9]+$")
          file(STRINGS "${run_root}/fault-injection.txt" gap_lines
            REGEX "^inter_send_gap_ms=[0-9]+$")
          list(LENGTH forwarded_lines forwarded_line_count)
          list(LENGTH gap_lines gap_line_count)
          if(NOT forwarded_line_count EQUAL 1 OR NOT gap_line_count EQUAL 1 OR
              NOT forwarded_lines MATCHES "forwarded_send_count=2")
            message(FATAL_ERROR "HOSTED_MARKER double-advance did not retain two timed one-shot forwards")
          endif()
          string(REGEX REPLACE "^[^=]*=" "" inter_send_gap_ms "${gap_lines}")
          if(inter_send_gap_ms LESS 200 OR inter_send_gap_ms GREATER 2000)
            message(FATAL_ERROR "HOSTED_MARKER double-advance send spacing is outside 200-2000 ms: ${inter_send_gap_ms}")
          endif()
        endif()
        if(fault STREQUAL "valid-wrong-origin")
          if(NOT EXISTS "${attempt_dir}/original-anchor.state" OR
              NOT EXISTS "${attempt_dir}/rewound-anchor.state" OR
              NOT EXISTS "${attempt_dir}/progression-session.rec.hosted-fault-original")
            message(FATAL_ERROR "HOSTED_MARKER wrong-origin control did not retain both native-valid anchors and its pre-injection record")
          endif()
          file(READ "${attempt_dir}/anchor-original.out" original_anchor_output)
          file(READ "${attempt_dir}/anchor-rewound.out" rewound_anchor_output)
          string(FIND "${original_anchor_output}" "${HOSTED_ANCHOR_LINE}" original_anchor_at)
          string(FIND "${rewound_anchor_output}" "${HOSTED_ANCHOR_LINE}" rewound_anchor_at)
          if(original_anchor_at EQUAL -1 OR rewound_anchor_at EQUAL -1)
            message(FATAL_ERROR "HOSTED_MARKER wrong-origin control lacked two native-valid anchors")
          endif()
        endif()
        file(APPEND "${HOSTED_MARKER_ROOT}/hosted-evidence.txt"
          "${run_name}=pass\nfault=${fault}\nproduction_verdict=${expected_verdict}\n"
          "terminal_reason=${terminal_reason}\nboundaries_before_rejection=${boundary_count}\n"
          "startup_captures=2\nstartup_screenshot_send_ordinals=1,2\n"
          "sends=${driver_sends}\ncleanup_sends=${cleanup_sends}\nelapsed_ms=${driver_elapsed_ms}\nattempt_elapsed_ms=${attempt_elapsed_ms}\n"
          "config_sha256=${config_sha}\ninjection=${injection_record}")
      endif()
      message(STATUS "HOSTED_MARKER ${run_name}: ${terminal_reason}, ${boundary_count} boundaries, ${driver_elapsed_ms} ms")
    endfunction()

    hosted_marker_session(intended "" VERDICT_OK VERDICT_OK 16 79)
    hosted_marker_session(fault-drop-advance drop-advance ADVANCE_NOT_EXACTLY_ONE PROGRESSION 3 8)
    hosted_marker_session(fault-double-advance double-advance ADVANCE_NOT_EXACTLY_ONE PROGRESSION 3 8)
    hosted_marker_session(fault-valid-wrong-origin valid-wrong-origin ORIGIN ORIGIN 13 70)
    file(APPEND "${HOSTED_MARKER_ROOT}/hosted-evidence.txt" "hosted_marker=complete\n")
    message(STATUS "HOSTED_MARKER passed: pinned qualification, exact progression and three production-verdict negatives")
    return()
  endif()
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

  # Exact network progression is proved separately on generated marker content by HOSTED_MARKER.
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
