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
  message(FATAL_ERROR "reproduction: -DMODE=cold|compare_self_test is required")
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
else()
  message(FATAL_ERROR "reproduction: unknown mode '${MODE}'")
endif()
