# Bounded public runner replay and its shared output contract.
#
#   MODE=cold: generate the permitted 600-frame NMOVIE1 movie, then run it
#               twice with the public CLI and compare the complete output.
#   MODE=compare_self_test: prove the comparator accepts a complete pair.
#   MODE=compare_files: internal subprocess mode used by negative controls.

cmake_minimum_required(VERSION 3.25)

function(reproduction_fresh_root parent prefix out)
  file(MAKE_DIRECTORY "${parent}")
  while(TRUE)
    string(RANDOM LENGTH 20 ALPHABET 0123456789abcdef suffix)
    set(candidate "${parent}/${prefix}-${suffix}")
    if(NOT EXISTS "${candidate}")
      file(MAKE_DIRECTORY "${candidate}")
      set(${out} "${candidate}" PARENT_SCOPE)
      return()
    endif()
  endwhile()
endfunction()

function(reproduction_preflight_require_digest label digest)
  if("${digest}" STREQUAL "")
    message(FATAL_ERROR "reproduction unsupported: recorded ${label} SHA-256 is missing")
  endif()
  if(NOT "${digest}" MATCHES "^[0-9A-Fa-f]+$")
    message(FATAL_ERROR "reproduction unsupported: recorded ${label} SHA-256 is not hexadecimal")
  endif()
  string(LENGTH "${digest}" digest_length)
  if(NOT digest_length EQUAL 64)
    message(FATAL_ERROR "reproduction unsupported: recorded ${label} SHA-256 must be 64 hexadecimal characters")
  endif()
endfunction()

# This is procedure policy for a known, same-invocation runner checkpoint. It
# is intentionally separate from the native loader, which validates its own
# format and cartridge contents but does not pair a state with an exact build.
function(reproduction_preflight state payload_kind runner rom recorded_runner_sha256
    recorded_rom_sha256 recorded_state_sha256)
  reproduction_preflight_require_digest("runner" "${recorded_runner_sha256}")
  reproduction_preflight_require_digest("ROM" "${recorded_rom_sha256}")
  reproduction_preflight_require_digest("state" "${recorded_state_sha256}")

  if(payload_kind STREQUAL "retroarch-disk-state")
    message(FATAL_ERROR "reproduction unsupported: host container is not a runner-native payload")
  elseif(NOT payload_kind STREQUAL "runner-native-same-invocation")
    message(FATAL_ERROR "reproduction unsupported: checkpoint origin is unknown or unverified")
  endif()

  foreach(path_label IN ITEMS runner ROM state)
    if(path_label STREQUAL "runner")
      set(path "${runner}")
    elseif(path_label STREQUAL "ROM")
      set(path "${rom}")
    else()
      set(path "${state}")
    endif()
    if(NOT EXISTS "${path}" OR IS_DIRECTORY "${path}")
      message(FATAL_ERROR "reproduction unsupported: current ${path_label} file is unavailable")
    endif()
  endforeach()

  file(SHA256 "${runner}" current_runner_sha256)
  file(SHA256 "${rom}" current_rom_sha256)
  file(SHA256 "${state}" current_state_sha256)
  string(TOLOWER "${recorded_runner_sha256}" expected_runner_sha256)
  string(TOLOWER "${recorded_rom_sha256}" expected_rom_sha256)
  string(TOLOWER "${recorded_state_sha256}" expected_state_sha256)
  if(NOT current_runner_sha256 STREQUAL expected_runner_sha256)
    message(FATAL_ERROR "reproduction unsupported: recorded runner SHA-256 mismatch")
  endif()
  if(NOT current_rom_sha256 STREQUAL expected_rom_sha256)
    message(FATAL_ERROR "reproduction unsupported: recorded ROM SHA-256 mismatch")
  endif()
  if(NOT current_state_sha256 STREQUAL expected_state_sha256)
    message(FATAL_ERROR "reproduction unsupported: recorded state SHA-256 mismatch")
  endif()
  message(STATUS "runner.reproduction.preflight: supplied runner, ROM and state digests match; origin is caller-declared, not authenticated")
endfunction()

function(reproduction_expect_preflight_failure root case_name state payload_kind runner rom
    recorded_runner_sha256 recorded_rom_sha256 recorded_state_sha256 expected)
  execute_process(COMMAND "${CMAKE_COMMAND}"
      "-DMODE=preflight"
      "-DSTATE=${state}"
      "-DPAYLOAD_KIND=${payload_kind}"
      "-DRUNNER=${runner}"
      "-DROM=${rom}"
      "-DRECORDED_RUNNER_SHA256=${recorded_runner_sha256}"
      "-DRECORDED_ROM_SHA256=${recorded_rom_sha256}"
      "-DRECORDED_STATE_SHA256=${recorded_state_sha256}"
      -P "${CMAKE_CURRENT_LIST_FILE}"
    OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE rc TIMEOUT 10)
  if("${rc}" STREQUAL "0")
    message(FATAL_ERROR "runner.reproduction.preflight_self_test: ${case_name} was accepted")
  endif()
  string(CONCAT combined "${out}" "${err}")
  string(FIND "${combined}" "${expected}" diagnostic_pos)
  if(diagnostic_pos EQUAL -1)
    string(REGEX MATCH "reproduction unsupported:[^\n]*" observed "${combined}")
    message(FATAL_ERROR "runner.reproduction.preflight_self_test: ${case_name} expected '${expected}', observed '${observed}'")
  endif()
  message(STATUS "runner.reproduction.preflight_self_test: ${case_name} stopped before replay")
endfunction()

function(reproduction_expect_preflight_success state payload_kind runner rom
    recorded_runner_sha256 recorded_rom_sha256 recorded_state_sha256)
  execute_process(COMMAND "${CMAKE_COMMAND}"
      "-DMODE=preflight"
      "-DSTATE=${state}"
      "-DPAYLOAD_KIND=${payload_kind}"
      "-DRUNNER=${runner}"
      "-DROM=${rom}"
      "-DRECORDED_RUNNER_SHA256=${recorded_runner_sha256}"
      "-DRECORDED_ROM_SHA256=${recorded_rom_sha256}"
      "-DRECORDED_STATE_SHA256=${recorded_state_sha256}"
      -P "${CMAKE_CURRENT_LIST_FILE}"
    OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE rc TIMEOUT 10)
  if(NOT "${rc}" STREQUAL "0")
    message(FATAL_ERROR "runner.reproduction.preflight_self_test: valid public preflight mode failed (${rc})\n${out}${err}")
  endif()
  string(CONCAT combined "${out}" "${err}")
  if(NOT combined MATCHES "supplied runner, ROM and state digests match" OR
     NOT combined MATCHES "origin is caller-declared, not authenticated")
    message(FATAL_ERROR "runner.reproduction.preflight_self_test: valid public preflight mode omitted its success result")
  endif()
  message(STATUS "runner.reproduction.preflight_self_test: valid public preflight mode accepted the recorded route")
endfunction()

function(reproduction_parse_output output label frames_out transitions_out pcm_out)
  # Line endings are the only normalization; record contents remain exact.
  string(REPLACE "\r\n" "\n" normalized "${output}")
  string(REPLACE "\r" "\n" normalized "${normalized}")
  string(REPLACE "\n" ";" lines "${normalized}")
  set(frame_records)
  set(expected_frame 1)
  set(transitions "")
  set(pcm "")

  foreach(line IN LISTS lines)
    if(line MATCHES "^frame ")
      if(NOT line MATCHES "^frame ([0-9]+) ticks ([0-9]+) sha256 ([0-9a-f]+)$")
        message(FATAL_ERROR "reproduction ${label}: malformed native frame record")
      endif()
      set(frame_number "${CMAKE_MATCH_1}")
      set(frame_ticks "${CMAKE_MATCH_2}")
      set(frame_hash "${CMAKE_MATCH_3}")
      string(LENGTH "${frame_hash}" frame_hash_length)
      if(NOT frame_hash_length EQUAL 64)
        message(FATAL_ERROR "reproduction ${label}: frame ${frame_number} hash is not 64 lowercase hex characters")
      endif()
      if(NOT frame_number EQUAL expected_frame)
        message(FATAL_ERROR "reproduction ${label}: expected frame ${expected_frame}, found ${frame_number}")
      endif()
      list(APPEND frame_records "frame ${frame_number} ticks ${frame_ticks} sha256 ${frame_hash}")
      math(EXPR expected_frame "${expected_frame} + 1")
    elseif(line MATCHES "^audio transitions ")
      if(NOT transitions STREQUAL "")
        message(FATAL_ERROR "reproduction ${label}: duplicate audio transitions hash")
      endif()
      if(NOT line MATCHES "^audio transitions sha256 ([0-9a-f]+)$")
        message(FATAL_ERROR "reproduction ${label}: malformed audio transitions hash")
      endif()
      set(transitions "${CMAKE_MATCH_1}")
      string(LENGTH "${transitions}" transitions_length)
      if(NOT transitions_length EQUAL 64)
        message(FATAL_ERROR "reproduction ${label}: audio transitions hash is not 64 lowercase hex characters")
      endif()
    elseif(line MATCHES "^audio pcm ")
      if(NOT pcm STREQUAL "")
        message(FATAL_ERROR "reproduction ${label}: duplicate audio PCM hash")
      endif()
      if(NOT line MATCHES "^audio pcm sha256 ([0-9a-f]+)$")
        message(FATAL_ERROR "reproduction ${label}: malformed audio PCM hash")
      endif()
      set(pcm "${CMAKE_MATCH_1}")
      string(LENGTH "${pcm}" pcm_length)
      if(NOT pcm_length EQUAL 64)
        message(FATAL_ERROR "reproduction ${label}: audio PCM hash is not 64 lowercase hex characters")
      endif()
    elseif(line MATCHES "^audio ")
      message(FATAL_ERROR "reproduction ${label}: unexpected audio record")
    endif()
  endforeach()

  if(NOT expected_frame EQUAL 601)
    math(EXPR frame_count "${expected_frame} - 1")
    message(FATAL_ERROR "reproduction ${label}: found ${frame_count} of 600 ordered native frame records")
  endif()
  if(transitions STREQUAL "")
    message(FATAL_ERROR "reproduction ${label}: audio transitions hash missing")
  endif()
  if(pcm STREQUAL "")
    message(FATAL_ERROR "reproduction ${label}: audio PCM hash missing")
  endif()

  string(JOIN "\n" frame_text ${frame_records})
  set(${frames_out} "${frame_text}" PARENT_SCOPE)
  set(${transitions_out} "${transitions}" PARENT_SCOPE)
  set(${pcm_out} "${pcm}" PARENT_SCOPE)
endfunction()

function(reproduction_compare_outputs left right label)
  reproduction_parse_output("${left}" "${label} first output" left_frames left_transitions left_pcm)
  reproduction_parse_output("${right}" "${label} second output" right_frames right_transitions right_pcm)
  if(NOT "${left_frames}" STREQUAL "${right_frames}")
    message(FATAL_ERROR "reproduction ${label}: ordered native frame records differ")
  endif()
  if(NOT "${left_transitions}" STREQUAL "${right_transitions}")
    message(FATAL_ERROR "reproduction ${label}: audio transitions hashes differ")
  endif()
  if(NOT "${left_pcm}" STREQUAL "${right_pcm}")
    message(FATAL_ERROR "reproduction ${label}: audio PCM hashes differ")
  endif()
endfunction()

function(reproduction_synthetic_output seed out)
  set(lines "")
  foreach(frame RANGE 1 600)
    math(EXPR ticks "${frame} * 29780")
    string(SHA256 frame_hash "${seed}-frame-${frame}")
    string(APPEND lines "frame ${frame} ticks ${ticks} sha256 ${frame_hash}\n")
  endforeach()
  string(SHA256 transitions "${seed}-transitions")
  string(SHA256 pcm "${seed}-pcm")
  string(APPEND lines "audio transitions sha256 ${transitions}\naudio pcm sha256 ${pcm}\n")
  set(${out} "${lines}" PARENT_SCOPE)
endfunction()

# This second writer deliberately uses a separate implementation from
# reproduction_synthetic_output: the positive contract control is not two
# copies of one already-formatted string.
function(reproduction_synthetic_reference_output out)
  set(lines "")
  set(frame 0)
  while(frame LESS 600)
    math(EXPR frame "${frame} + 1")
    math(EXPR ticks "${frame} * 29780")
    string(SHA256 digest "independent-control-a-frame-${frame}")
    string(APPEND lines "frame ${frame} ticks ${ticks} sha256 ${digest}\n")
  endwhile()
  string(SHA256 transition_digest "independent-control-a-transitions")
  string(SHA256 pcm_digest "independent-control-a-pcm")
  string(APPEND lines "audio transitions sha256 ${transition_digest}\naudio pcm sha256 ${pcm_digest}\n")
  set(${out} "${lines}" PARENT_SCOPE)
endfunction()

function(reproduction_expect_failure root case_name left right expected)
  set(left_file "${root}/${case_name}-left.log")
  set(right_file "${root}/${case_name}-right.log")
  file(WRITE "${left_file}" "${left}")
  file(WRITE "${right_file}" "${right}")
  execute_process(COMMAND "${CMAKE_COMMAND}"
      "-DMODE=compare_files"
      "-DLEFT_FILE=${left_file}"
      "-DRIGHT_FILE=${right_file}"
      "-DCASE_NAME=${case_name}"
      -P "${CMAKE_CURRENT_LIST_FILE}"
    OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE rc TIMEOUT 10)
  if("${rc}" STREQUAL "0")
    message(FATAL_ERROR "reproduction compare_self_test: ${case_name} was accepted")
  endif()
  string(CONCAT combined "${out}" "${err}")
  string(FIND "${combined}" "${expected}" diagnostic_pos)
  if(diagnostic_pos EQUAL -1)
    string(REGEX MATCH "reproduction [^\n]*" observed "${combined}")
    message(FATAL_ERROR "reproduction compare_self_test: ${case_name} expected '${expected}', observed '${observed}'")
  endif()
  message(STATUS "runner.reproduction.compare_self_test: ${case_name} rejected for the expected reason")
endfunction()

if(NOT DEFINED MODE OR MODE STREQUAL "")
  message(FATAL_ERROR "reproduction: -DMODE=cold|compare_self_test|preflight|preflight_fixture|preflight_self_test is required")
elseif(MODE STREQUAL "cold")
  foreach(var RUNNER MOVIE_WRITER ROM WORK)
    if(NOT DEFINED ${var} OR "${${var}}" STREQUAL "")
      message(FATAL_ERROR "reproduction cold: -D${var}=... is required")
    endif()
  endforeach()

  reproduction_fresh_root("${WORK}" "cold" run_root)
  set(movie "${run_root}/nesrunner-pos.nmovie")
  execute_process(COMMAND "${MOVIE_WRITER}" "${movie}" nesrunner-pos
    OUTPUT_VARIABLE writer_out ERROR_VARIABLE writer_err RESULT_VARIABLE writer_rc
    TIMEOUT 10)
  file(WRITE "${run_root}/movie-writer.stdout" "${writer_out}")
  file(WRITE "${run_root}/movie-writer.stderr" "${writer_err}")
  if(NOT "${writer_rc}" STREQUAL "0")
    message(FATAL_ERROR "reproduction cold: permitted NMOVIE1 generation failed (status ${writer_rc})")
  endif()

  foreach(run IN ITEMS 1 2)
    execute_process(COMMAND "${RUNNER}" --rom "${ROM}" --movie "${movie}" --hash-audio
      OUTPUT_VARIABLE run_out ERROR_VARIABLE run_err RESULT_VARIABLE run_rc TIMEOUT 60)
    file(WRITE "${run_root}/run-${run}.stdout" "${run_out}")
    file(WRITE "${run_root}/run-${run}.stderr" "${run_err}")
    if(NOT "${run_rc}" STREQUAL "0")
      message(FATAL_ERROR "reproduction cold: runner execution ${run} failed or exceeded 60 seconds (status ${run_rc})")
    endif()
    if(run_err MATCHES "nesturbator-run: stopped")
      message(FATAL_ERROR "reproduction cold: runner reported an interrupted stop; this is not a complete replay")
    endif()
    set(run_${run}_output "${run_out}")
  endforeach()

  reproduction_parse_output("${run_1_output}" "cold replay 1" frames_one transitions_one pcm_one)
  reproduction_parse_output("${run_2_output}" "cold replay 2" frames_two transitions_two pcm_two)
  reproduction_compare_outputs("${run_1_output}" "${run_2_output}" "cold replay")
  string(SHA256 frame_records_sha256 "${frames_one}")
  message(STATUS "runner.reproduction.cold: 600 ordered native frame records, transition hashes and PCM hashes match")
  message(STATUS "runner.reproduction.cold: native_frame_records_sha256=${frame_records_sha256}")
  message(STATUS "runner.reproduction.cold: audio_transitions_sha256=${transitions_one}")
  message(STATUS "runner.reproduction.cold: audio_pcm_sha256=${pcm_one}")
elseif(MODE STREQUAL "compare_self_test")
  if(NOT DEFINED WORK OR "${WORK}" STREQUAL "")
    message(FATAL_ERROR "reproduction compare_self_test: -DWORK=... is required")
  endif()
  reproduction_fresh_root("${WORK}" "compare-self-test" run_root)
  reproduction_synthetic_output("independent-control-a" control_a)
  reproduction_synthetic_reference_output(control_b)
  reproduction_compare_outputs("${control_a}" "${control_b}" "synthetic positive control")

  string(SHA256 final_hash "independent-control-a-frame-600")
  string(SHA256 changed_hash "changed-final-frame")
  set(changed_frame "${control_b}")
  string(REPLACE "sha256 ${final_hash}\naudio transitions" "sha256 ${changed_hash}\naudio transitions"
    changed_frame "${changed_frame}")
  reproduction_expect_failure("${run_root}" "changed-final-frame" "${control_a}" "${changed_frame}"
    "ordered native frame records differ")

  string(SHA256 transition_hash "independent-control-a-transitions")
  string(SHA256 changed_transition_hash "changed-transition-hash")
  set(changed_transition "${control_b}")
  string(REPLACE "audio transitions sha256 ${transition_hash}"
    "audio transitions sha256 ${changed_transition_hash}" changed_transition "${changed_transition}")
  reproduction_expect_failure("${run_root}" "changed-transition-hash" "${control_a}" "${changed_transition}"
    "audio transitions hashes differ")

  string(SHA256 pcm_hash "independent-control-a-pcm")
  string(SHA256 changed_pcm_hash "changed-pcm-hash")
  set(changed_pcm "${control_b}")
  string(REPLACE "audio pcm sha256 ${pcm_hash}" "audio pcm sha256 ${changed_pcm_hash}"
    changed_pcm "${changed_pcm}")
  reproduction_expect_failure("${run_root}" "changed-pcm-hash" "${control_a}" "${changed_pcm}"
    "audio PCM hashes differ")

  set(missing_audio "${control_b}")
  string(REPLACE "audio transitions sha256 ${transition_hash}\n" "" missing_audio "${missing_audio}")
  reproduction_expect_failure("${run_root}" "missing-audio-line" "${control_a}" "${missing_audio}"
    "transitions hash")

  string(SHA256 hash_599 "independent-control-a-frame-599")
  set(duplicate_frame "${control_b}")
  string(REPLACE "frame 600 ticks 17868000 sha256 ${final_hash}\naudio transitions"
    "frame 600 ticks 17868000 sha256 ${final_hash}\nframe 600 ticks 17868000 sha256 ${final_hash}\naudio transitions"
    duplicate_frame "${duplicate_frame}")
  reproduction_expect_failure("${run_root}" "duplicate-frame" "${control_a}" "${duplicate_frame}"
    "expected frame 601, found 600")

  set(reordered_frames "${control_b}")
  string(REPLACE "frame 599 ticks 17838220 sha256 ${hash_599}\nframe 600 ticks 17868000 sha256 ${final_hash}"
    "frame 600 ticks 17868000 sha256 ${final_hash}\nframe 599 ticks 17838220 sha256 ${hash_599}"
    reordered_frames "${reordered_frames}")
  reproduction_expect_failure("${run_root}" "reordered-frames" "${control_a}" "${reordered_frames}"
    "expected frame 599, found 600")

  set(truncated "${control_b}")
  string(REPLACE "frame 600 ticks 17868000 sha256 ${final_hash}\n" "" truncated "${truncated}")
  reproduction_expect_failure("${run_root}" "truncated-frame-stream" "${control_a}" "${truncated}"
    "found 599")
  message(STATUS "runner.reproduction.compare_self_test: independently generated 600-record positive pair accepted")
elseif(MODE STREQUAL "compare_files")
  foreach(var LEFT_FILE RIGHT_FILE)
    if(NOT DEFINED ${var} OR "${${var}}" STREQUAL "")
      message(FATAL_ERROR "reproduction compare_files: -D${var}=... is required")
    endif()
  endforeach()
  file(READ "${LEFT_FILE}" left)
  file(READ "${RIGHT_FILE}" right)
  reproduction_compare_outputs("${left}" "${right}" "${CASE_NAME}")
  message(STATUS "reproduction ${CASE_NAME}: outputs match")
elseif(MODE STREQUAL "preflight_fixture")
  foreach(var RUNNER NMI_FIXTURE WORK)
    if(NOT DEFINED ${var} OR "${${var}}" STREQUAL "")
      message(FATAL_ERROR "reproduction preflight_fixture: -D${var}=... is required")
    endif()
  endforeach()
  reproduction_fresh_root("${WORK}" "preflight" run_root)
  set(rom "${run_root}/nmi.nes")
  set(state "${run_root}/frame1.state")
  execute_process(COMMAND "${NMI_FIXTURE}" write "${rom}"
    OUTPUT_VARIABLE fixture_out ERROR_VARIABLE fixture_err RESULT_VARIABLE fixture_rc TIMEOUT 10)
  file(WRITE "${run_root}/fixture.stdout" "${fixture_out}")
  file(WRITE "${run_root}/fixture.stderr" "${fixture_err}")
  if(NOT "${fixture_rc}" STREQUAL "0" OR NOT EXISTS "${rom}")
    message(FATAL_ERROR "runner.reproduction.preflight: NMI fixture generation failed or exceeded 10 seconds (status ${fixture_rc})")
  endif()
  execute_process(COMMAND "${RUNNER}" --rom "${rom}" --frames 1
      --state-at-frame "1:${state}" --hash-state 1
    OUTPUT_VARIABLE runner_out ERROR_VARIABLE runner_err RESULT_VARIABLE runner_rc TIMEOUT 20)
  file(WRITE "${run_root}/runner.stdout" "${runner_out}")
  file(WRITE "${run_root}/runner.stderr" "${runner_err}")
  if(NOT "${runner_rc}" STREQUAL "0" OR NOT EXISTS "${state}")
    message(FATAL_ERROR "runner.reproduction.preflight: runner did not create the frame-1 checkpoint within 20 seconds (status ${runner_rc})")
  endif()
  if(runner_err MATCHES "nesturbator-run: stopped")
    message(FATAL_ERROR "runner.reproduction.preflight: runner reported an interrupted stop before checkpoint creation completed")
  endif()
  if(NOT runner_out MATCHES "state 1 ticks [0-9]+ size [1-9][0-9]* sha256 [0-9a-f]+")
    message(FATAL_ERROR "runner.reproduction.preflight: required frame-1 state identity output is missing")
  endif()
  # Record all three identities at creation in this invocation, then check
  # those records with the same function exercised by the negative controls.
  file(SHA256 "${RUNNER}" recorded_runner_sha256)
  file(SHA256 "${rom}" recorded_rom_sha256)
  file(SHA256 "${state}" recorded_state_sha256)
  reproduction_preflight("${state}" "runner-native-same-invocation" "${RUNNER}" "${rom}"
    "${recorded_runner_sha256}" "${recorded_rom_sha256}" "${recorded_state_sha256}")
  message(STATUS "runner.reproduction.preflight: fixture created this native checkpoint with the identified runner in this invocation")
elseif(MODE STREQUAL "preflight_self_test")
  if(NOT DEFINED WORK OR "${WORK}" STREQUAL "")
    message(FATAL_ERROR "runner.reproduction.preflight_self_test: -DWORK=... is required")
  endif()
  reproduction_fresh_root("${WORK}" "preflight-self-test" run_root)
  set(runner "${run_root}/runner.bin")
  set(rom "${run_root}/rom.nes")
  set(state "${run_root}/frame1.state")
  file(WRITE "${runner}" "synthetic exact runner identity\n")
  file(WRITE "${rom}" "synthetic exact content identity\n")
  file(WRITE "${state}" "synthetic native state identity\n")
  file(SHA256 "${runner}" recorded_runner_sha256)
  file(SHA256 "${rom}" recorded_rom_sha256)
  file(SHA256 "${state}" recorded_state_sha256)
  reproduction_preflight("${state}" "runner-native-same-invocation" "${runner}" "${rom}"
    "${recorded_runner_sha256}" "${recorded_rom_sha256}" "${recorded_state_sha256}")
  reproduction_expect_preflight_success("${state}" "runner-native-same-invocation" "${runner}" "${rom}"
    "${recorded_runner_sha256}" "${recorded_rom_sha256}" "${recorded_state_sha256}")

  string(SUBSTRING "${recorded_runner_sha256}" 0 1 first_hex)
  string(SUBSTRING "${recorded_runner_sha256}" 1 -1 remaining_hex)
  if(first_hex STREQUAL "0")
    set(changed_first_hex "1")
  else()
    set(changed_first_hex "0")
  endif()
  set(changed_runner_sha256 "${changed_first_hex}${remaining_hex}")
  reproduction_expect_preflight_failure("${run_root}" "changed-runner-identity" "${state}"
    "runner-native-same-invocation" "${runner}" "${rom}" "${changed_runner_sha256}"
    "${recorded_rom_sha256}" "${recorded_state_sha256}" "recorded runner SHA-256 mismatch")

  string(SUBSTRING "${recorded_rom_sha256}" 0 1 first_hex)
  string(SUBSTRING "${recorded_rom_sha256}" 1 -1 remaining_hex)
  if(first_hex STREQUAL "0")
    set(changed_first_hex "1")
  else()
    set(changed_first_hex "0")
  endif()
  set(changed_rom_sha256 "${changed_first_hex}${remaining_hex}")
  reproduction_expect_preflight_failure("${run_root}" "changed-content-identity" "${state}"
    "runner-native-same-invocation" "${runner}" "${rom}" "${recorded_runner_sha256}"
    "${changed_rom_sha256}" "${recorded_state_sha256}" "recorded ROM SHA-256 mismatch")

  string(SUBSTRING "${recorded_state_sha256}" 0 1 first_hex)
  string(SUBSTRING "${recorded_state_sha256}" 1 -1 remaining_hex)
  if(first_hex STREQUAL "0")
    set(changed_first_hex "1")
  else()
    set(changed_first_hex "0")
  endif()
  set(changed_state_sha256 "${changed_first_hex}${remaining_hex}")
  reproduction_expect_preflight_failure("${run_root}" "changed-state-identity" "${state}"
    "runner-native-same-invocation" "${runner}" "${rom}" "${recorded_runner_sha256}"
    "${recorded_rom_sha256}" "${changed_state_sha256}" "recorded state SHA-256 mismatch")

  reproduction_expect_preflight_failure("${run_root}" "missing-recorded-identity" "${state}"
    "runner-native-same-invocation" "${runner}" "${rom}" "${recorded_runner_sha256}"
    "${recorded_rom_sha256}" "" "recorded state SHA-256 is missing")
  reproduction_expect_preflight_failure("${run_root}" "unknown-origin" "${state}" "unknown"
    "${runner}" "${rom}" "${recorded_runner_sha256}" "${recorded_rom_sha256}"
    "${recorded_state_sha256}" "checkpoint origin is unknown or unverified")
  reproduction_expect_preflight_failure("${run_root}" "host-container" "${state}" "retroarch-disk-state"
    "${runner}" "${rom}" "${recorded_runner_sha256}" "${recorded_rom_sha256}"
    "${recorded_state_sha256}" "host container is not a runner-native payload")

  file(WRITE "${state}" "changed after the recorded identity\n")
  reproduction_expect_preflight_failure("${run_root}" "changed-state-bytes" "${state}"
    "runner-native-same-invocation" "${runner}" "${rom}" "${recorded_runner_sha256}"
    "${recorded_rom_sha256}" "${recorded_state_sha256}" "recorded state SHA-256 mismatch")
  message(STATUS "runner.reproduction.preflight_self_test: valid identity accepted; seven independent unsupported cases rejected before replay")
elseif(MODE STREQUAL "preflight")
  foreach(var STATE PAYLOAD_KIND RUNNER ROM RECORDED_RUNNER_SHA256 RECORDED_ROM_SHA256 RECORDED_STATE_SHA256)
    if(NOT DEFINED ${var})
      message(FATAL_ERROR "runner.reproduction.preflight: -D${var}=... is required")
    endif()
  endforeach()
  reproduction_preflight("${STATE}" "${PAYLOAD_KIND}" "${RUNNER}" "${ROM}"
    "${RECORDED_RUNNER_SHA256}" "${RECORDED_ROM_SHA256}" "${RECORDED_STATE_SHA256}")
else()
  message(FATAL_ERROR "reproduction: unknown mode '${MODE}'")
endif()
