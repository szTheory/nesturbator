cmake_minimum_required(VERSION 3.25)

function(accuracycoin_compute_input_fingerprint source_dir out_hash out_count)
  file(GLOB_RECURSE input_files LIST_DIRECTORIES FALSE
    "${source_dir}/src/*.c" "${source_dir}/src/*.h"
    "${source_dir}/runner/*.c" "${source_dir}/runner/*.h"
    "${source_dir}/include/*.h" "${source_dir}/cmake/*.cmake")
  list(APPEND input_files
    "${source_dir}/CMakeLists.txt"
    "${source_dir}/CMakePresets.json"
    "${source_dir}/tests/CMakeLists.txt"
    "${source_dir}/tests/accuracy/test_conformance.c"
    "${source_dir}/tests/accuracy/test_scoreboard.c"
    "${source_dir}/tests/cmake/accuracycoin_campaign.cmake"
    "${source_dir}/tests/cmake/accuracycoin_activation.cmake"
    "${source_dir}/tests/cmake/accuracycoin_input_fingerprint.cmake"
    "${source_dir}/tests/cmake/accuracycoin_partial_failure.cmake")
  list(REMOVE_DUPLICATES input_files)
  list(SORT input_files)

  set(input_manifest "")
  set(input_count 0)
  foreach(input_file IN LISTS input_files)
    if(NOT EXISTS "${input_file}" OR IS_DIRECTORY "${input_file}")
      message(FATAL_ERROR "missing AccuracyCoin build input: ${input_file}")
    endif()
    file(RELATIVE_PATH relative_path "${source_dir}" "${input_file}")
    file(SHA256 "${input_file}" input_hash)
    string(APPEND input_manifest "${relative_path}\t${input_hash}\n")
    math(EXPR input_count "${input_count} + 1")
  endforeach()
  string(SHA256 fingerprint "${input_manifest}")
  set(${out_hash} "${fingerprint}" PARENT_SCOPE)
  set(${out_count} "${input_count}" PARENT_SCOPE)
endfunction()
