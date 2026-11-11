cmake_minimum_required(VERSION 3.25)

function(accuracycoin_compute_input_file_hash relative_path input_file out_hash)
  if(relative_path STREQUAL "include/nesturbator.h")
    file(READ "${input_file}" input_contents)
    foreach(component IN ITEMS MAJOR MINOR PATCH)
      string(REGEX REPLACE "(#define NESTURBATOR_VERSION_${component} )[0-9]+"
        "\\1<release-version>" input_contents "${input_contents}")
    endforeach()
    string(SHA256 input_hash "${input_contents}")
  else()
    file(SHA256 "${input_file}" input_hash)
  endif()
  set(${out_hash} "${input_hash}" PARENT_SCOPE)
endfunction()

function(accuracycoin_input_paths source_dir out_paths)
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
  set(${out_paths} "${input_files}" PARENT_SCOPE)
endfunction()

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
    accuracycoin_compute_input_file_hash("${relative_path}" "${input_file}" input_hash)
    string(APPEND input_manifest "${relative_path}\t${input_hash}\n")
    math(EXPR input_count "${input_count} + 1")
  endforeach()
  string(SHA256 fingerprint "${input_manifest}")
  set(${out_hash} "${fingerprint}" PARENT_SCOPE)
  set(${out_count} "${input_count}" PARENT_SCOPE)
endfunction()

macro(accuracycoin_binding_fail out_error message)
  set(${out_error} "${message}" PARENT_SCOPE)
endmacro()

function(accuracycoin_binding_get json field expected_type out_value out_error)
  string(JSON actual_type ERROR_VARIABLE type_error TYPE "${json}" "${field}")
  if(type_error)
    set(${out_error} "binding ${field} is absent or malformed: ${type_error}" PARENT_SCOPE)
    return()
  endif()
  if(NOT actual_type STREQUAL expected_type)
    set(${out_error} "binding ${field} must be ${expected_type}, found ${actual_type}" PARENT_SCOPE)
    return()
  endif()
  string(JSON actual ERROR_VARIABLE value_error GET "${json}" "${field}")
  if(value_error)
    set(${out_error} "binding ${field} is malformed: ${value_error}" PARENT_SCOPE)
    return()
  endif()
  set(${out_value} "${actual}" PARENT_SCOPE)
  set(${out_error} "" PARENT_SCOPE)
endfunction()

function(accuracycoin_binding_expect json field expected_type expected out_error)
  accuracycoin_binding_get("${json}" "${field}" "${expected_type}" actual error)
  if(error)
    set(${out_error} "${error}" PARENT_SCOPE)
  elseif(NOT "${actual}" STREQUAL "${expected}")
    set(${out_error} "binding ${field} differs from its pinned value" PARENT_SCOPE)
  else()
    set(${out_error} "" PARENT_SCOPE)
  endif()
endfunction()




function(accuracycoin_binding_meta_value source_dir name out_value out_error)
  set(metadata_file "${source_dir}/tests/accuracy/baseline-metadata.txt")
  if(NOT EXISTS "${metadata_file}" OR IS_DIRECTORY "${metadata_file}" OR IS_SYMLINK "${metadata_file}")
    set(${out_error} "historical baseline metadata is missing or not a regular file" PARENT_SCOPE)
    return()
  endif()
  file(READ "${metadata_file}" metadata_text)
  string(ASCII 10 newline)
  string(FIND "${metadata_text}" "${name}=" key_index)
  if(key_index LESS 0)
    set(${out_error} "historical baseline metadata is missing ${name}" PARENT_SCOPE)
    return()
  endif()
  if(key_index GREATER 0)
    math(EXPR previous_index "${key_index} - 1")
    string(SUBSTRING "${metadata_text}" ${previous_index} 1 previous_character)
    if(NOT previous_character STREQUAL newline)
      set(${out_error} "historical baseline metadata is missing ${name}" PARENT_SCOPE)
      return()
    endif()
  endif()
  string(SUBSTRING "${metadata_text}" ${key_index} -1 field_tail)
  string(FIND "${field_tail}" "${newline}" line_end)
  if(line_end LESS 0)
    string(LENGTH "${field_tail}" line_end)
  endif()
  math(EXPR after_line_index "${key_index} + ${line_end}")
  string(SUBSTRING "${metadata_text}" ${after_line_index} -1 remaining_text)
  string(FIND "${remaining_text}" "${newline}${name}=" duplicate_index)
  if(NOT duplicate_index LESS 0)
    set(${out_error} "historical baseline metadata repeats ${name}" PARENT_SCOPE)
    return()
  endif()
  string(LENGTH "${name}" name_length)
  math(EXPR value_start "${name_length} + 1")
  math(EXPR value_length "${line_end} - ${value_start}")
  string(SUBSTRING "${field_tail}" ${value_start} ${value_length} value)
  set(${out_value} "${value}" PARENT_SCOPE)
  set(${out_error} "" PARENT_SCOPE)
endfunction()






function(accuracycoin_validate_input_binding current_source out_error)
  set(${out_error} "" PARENT_SCOPE)
  get_filename_component(current_source_real "${current_source}" REALPATH)
  if(NOT IS_DIRECTORY "${current_source_real}")
    accuracycoin_binding_fail("${out_error}" "binding source is not a directory")
    return()
  endif()

  set(binding_file "${current_source_real}/tests/accuracy/input-binding-v1.json")
  if(NOT EXISTS "${binding_file}" OR IS_DIRECTORY "${binding_file}" OR IS_SYMLINK "${binding_file}")
    accuracycoin_binding_fail("${out_error}" "versioned historical/current binding is missing or not a regular file")
    return()
  endif()
  file(READ "${binding_file}" binding_text)
  string(JSON root_type ERROR_VARIABLE root_error TYPE "${binding_text}")
  if(root_error OR NOT root_type STREQUAL "OBJECT")
    accuracycoin_binding_fail("${out_error}" "historical/current binding must be a JSON object")
    return()
  endif()

  set(binding_keys
    schema_version source_revision source_dirty source_dirty_reason
    baseline_metadata_sha256 historical_input_sha256 historical_input_manifest historical_input_count
    campaign_report_sha256 scoreboard_sha256 original_tests_cmake_sha256
    current_tests_cmake_sha256 allowed_registration_names
    accuracycoin_activation_sha256 accuracycoin_input_fingerprint_sha256)
  list(LENGTH binding_keys expected_key_count)
  string(JSON actual_key_count ERROR_VARIABLE key_count_error LENGTH "${binding_text}")
  if(key_count_error OR NOT actual_key_count EQUAL expected_key_count)
    accuracycoin_binding_fail("${out_error}" "binding has missing or unknown fields")
    return()
  endif()
  foreach(field IN LISTS binding_keys)
    string(REGEX MATCHALL "\"${field}\"[ \t\r\n]*:" occurrences "${binding_text}")
    list(LENGTH occurrences occurrence_count)
    if(NOT occurrence_count EQUAL 1)
      accuracycoin_binding_fail("${out_error}" "binding field ${field} is missing or duplicated")
      return()
    endif()
  endforeach()
  math(EXPR last_key_index "${actual_key_count} - 1")
  foreach(index RANGE 0 ${last_key_index})
    string(JSON field ERROR_VARIABLE field_error MEMBER "${binding_text}" ${index})
    if(field_error OR NOT field IN_LIST binding_keys)
      accuracycoin_binding_fail("${out_error}" "binding contains an unknown or malformed field")
      return()
    endif()
  endforeach()

  set(expectations
    "schema_version|NUMBER|1"
    "source_revision|STRING|d4802809ff41"
    "source_dirty|NUMBER|1"
    "baseline_metadata_sha256|STRING|e48e15e7203c0eef96281cb545f0f487f9b716168570918a5ba8d80209c0e65b"
    "historical_input_sha256|STRING|78822adfa35ac2b0e2f562cfb0067ae08db7e76c3e6e4027fa63d7bcca400de2"
    "historical_input_count|NUMBER|56"
    "campaign_report_sha256|STRING|56ebd01adb97b3916fde36f8739602284d951a1fd00d84285bb4ef441fb463a3"
    "scoreboard_sha256|STRING|059d7c7558bd9f4785ab384c589f04e16f330ef3ba477393a8492b59e086dbbf"
    "original_tests_cmake_sha256|STRING|c656956571e96aedbb75ffec6fa47a1035faa5a45767734f1c89888919ab06e7"
    "current_tests_cmake_sha256|STRING|a4179a215765f1b783e7a408f55d2eb9f9264c0cb315215c9da09e0336624ea2")
  foreach(item IN LISTS expectations)
    string(REPLACE "|" ";" parts "${item}")
    list(GET parts 0 field)
    list(GET parts 1 expected_type)
    list(GET parts 2 expected_value)
    accuracycoin_binding_expect("${binding_text}" "${field}" "${expected_type}" "${expected_value}" field_error)
    if(field_error)
      accuracycoin_binding_fail("${out_error}" "${field_error}")
      return()
    endif()
  endforeach()

  accuracycoin_binding_get("${binding_text}" historical_input_manifest STRING
    recorded_history_manifest manifest_error)
  if(manifest_error)
    accuracycoin_binding_fail("${out_error}" "${manifest_error}")
    return()
  endif()

  set(expected_reason "Quick task 261029-sdj measured after explicit CI configuration at d4802809ff41 with a dirty working tree. CI inventory/hash reuse and paired public-header comments are present; the campaign retains 119 PASS and 27 FAIL with unchanged report digest. Earlier hosted acceptance keeps its original source identity.")
  accuracycoin_binding_expect("${binding_text}" source_dirty_reason STRING "${expected_reason}" field_error)
  if(field_error)
    accuracycoin_binding_fail("${out_error}" "${field_error}")
    return()
  endif()

  string(JSON registration_type ERROR_VARIABLE registration_error
    TYPE "${binding_text}" allowed_registration_names)
  string(JSON registration_count ERROR_VARIABLE registration_count_error
    LENGTH "${binding_text}" allowed_registration_names)
  set(expected_registrations
    install.gameplay provider.profile provider.profile.controls provider.receipt.controls)
  if(registration_error OR registration_count_error OR
     NOT registration_type STREQUAL "ARRAY" OR NOT registration_count EQUAL 4)
    accuracycoin_binding_fail("${out_error}" "binding registration names must be the exact four-entry array")
    return()
  endif()
  foreach(index RANGE 0 3)
    string(JSON registration_item_type ERROR_VARIABLE item_type_error
      TYPE "${binding_text}" allowed_registration_names ${index})
    string(JSON registration_item ERROR_VARIABLE item_error
      GET "${binding_text}" allowed_registration_names ${index})
    list(GET expected_registrations ${index} expected_registration)
    if(item_type_error OR item_error OR
       NOT registration_item_type STREQUAL "STRING" OR
       NOT registration_item STREQUAL expected_registration)
      accuracycoin_binding_fail("${out_error}" "binding registration names differ from the exact reviewed delta")
      return()
    endif()
  endforeach()

  set(metadata_file "${current_source_real}/tests/accuracy/baseline-metadata.txt")
  file(SHA256 "${metadata_file}" metadata_sha)
  if(NOT metadata_sha STREQUAL "e48e15e7203c0eef96281cb545f0f487f9b716168570918a5ba8d80209c0e65b")
    accuracycoin_binding_fail("${out_error}" "historical baseline metadata bytes changed")
    return()
  endif()
  foreach(pair IN ITEMS
      "source_revision|d4802809ff41"
      "source_dirty|1"
      "build_inputs_sha256|78822adfa35ac2b0e2f562cfb0067ae08db7e76c3e6e4027fa63d7bcca400de2"
      "build_inputs_count|56"
      "campaign_report_sha256|56ebd01adb97b3916fde36f8739602284d951a1fd00d84285bb4ef441fb463a3"
      "scoreboard_sha256|059d7c7558bd9f4785ab384c589f04e16f330ef3ba477393a8492b59e086dbbf")
    string(REPLACE "|" ";" fields "${pair}")
    list(GET fields 0 name)
    list(GET fields 1 expected_value)
    accuracycoin_binding_meta_value("${current_source_real}" "${name}" actual_value meta_error)
    if(meta_error OR NOT actual_value STREQUAL expected_value)
      accuracycoin_binding_fail("${out_error}" "historical metadata ${name} changed or is malformed: ${meta_error}")
      return()
    endif()
  endforeach()

  accuracycoin_binding_meta_value("${current_source_real}" source_dirty_reason actual_reason reason_error)
  if(reason_error OR NOT actual_reason STREQUAL expected_reason)
    accuracycoin_binding_fail("${out_error}" "historical metadata source_dirty_reason changed or is malformed: ${reason_error}")
    return()
  endif()

  set(scoreboard_file "${current_source_real}/tests/accuracy/scoreboard.txt")
  if(NOT EXISTS "${scoreboard_file}" OR IS_DIRECTORY "${scoreboard_file}" OR IS_SYMLINK "${scoreboard_file}")
    accuracycoin_binding_fail("${out_error}" "historical AccuracyCoin scoreboard is missing or not a regular file")
    return()
  endif()
  file(SHA256 "${scoreboard_file}" scoreboard_sha)
  if(NOT scoreboard_sha STREQUAL "059d7c7558bd9f4785ab384c589f04e16f330ef3ba477393a8492b59e086dbbf")
    accuracycoin_binding_fail("${out_error}" "historical AccuracyCoin scoreboard bytes changed")
    return()
  endif()

  set(current_cmake_file "${current_source_real}/tests/CMakeLists.txt")
  file(SHA256 "${current_cmake_file}" actual_current_cmake_sha)
  if(NOT actual_current_cmake_sha STREQUAL "a4179a215765f1b783e7a408f55d2eb9f9264c0cb315215c9da09e0336624ea2")
    accuracycoin_binding_fail("${out_error}" "current registration CMake bytes differ from the exact four-entry delta")
    return()
  endif()

  set(checker_hashes
    "accuracycoin_activation_sha256|tests/cmake/accuracycoin_activation.cmake"
    "accuracycoin_input_fingerprint_sha256|tests/cmake/accuracycoin_input_fingerprint.cmake")
  foreach(pair IN LISTS checker_hashes)
    string(REPLACE "|" ";" fields "${pair}")
    list(GET fields 0 binding_field)
    list(GET fields 1 relative_path)
    accuracycoin_binding_get("${binding_text}" "${binding_field}" STRING recorded_hash hash_error)
    if(hash_error)
      accuracycoin_binding_fail("${out_error}" "${hash_error}")
      return()
    endif()
    file(SHA256 "${current_source_real}/${relative_path}" actual_hash)
    if(NOT recorded_hash STREQUAL actual_hash)
      accuracycoin_binding_fail("${out_error}" "reviewed checker bytes differ from the binding: ${relative_path}")
      return()
    endif()
  endforeach()

  accuracycoin_input_paths("${current_source_real}" input_files)
  list(LENGTH input_files input_count)
  if(NOT input_count EQUAL 56)
    accuracycoin_binding_fail("${out_error}" "current protected input closure changed from 56 paths")
    return()
  endif()
  accuracycoin_compute_input_fingerprint("${current_source_real}" current_fingerprint current_count)
  if(NOT current_count EQUAL 56)
    accuracycoin_binding_fail("${out_error}" "current protected input count is malformed")
    return()
  endif()

  set(historical_manifest "")
  set(maintenance_paths
    tests/CMakeLists.txt
    tests/cmake/accuracycoin_activation.cmake
    tests/cmake/accuracycoin_input_fingerprint.cmake)
  string(ASCII 9 tab)
  string(ASCII 10 newline)
  string(LENGTH "${recorded_history_manifest}" history_manifest_length)
  if(history_manifest_length LESS 1)
    accuracycoin_binding_fail("${out_error}" "historical manifest is empty")
    return()
  endif()
  math(EXPR final_character_index "${history_manifest_length} - 1")
  string(SUBSTRING "${recorded_history_manifest}" ${final_character_index} 1 final_character)
  if(NOT final_character STREQUAL newline)
    accuracycoin_binding_fail("${out_error}" "historical manifest must end with one newline")
    return()
  endif()
  math(EXPR history_rows_length "${history_manifest_length} - 1")
  string(SUBSTRING "${recorded_history_manifest}" 0 ${history_rows_length} manifest_without_final_newline)
  string(REPLACE "${newline}" ";" historical_rows "${manifest_without_final_newline}")
  list(LENGTH historical_rows historical_row_count)
  if(NOT historical_row_count EQUAL 56)
    accuracycoin_binding_fail("${out_error}" "historical manifest must contain exactly 56 rows")
    return()
  endif()
  set(history_index 0)
  foreach(input_file IN LISTS input_files)
    if(NOT EXISTS "${input_file}" OR IS_DIRECTORY "${input_file}" OR IS_SYMLINK "${input_file}")
      accuracycoin_binding_fail("${out_error}" "current protected input is missing or not regular: ${input_file}")
      return()
    endif()
    file(RELATIVE_PATH relative_path "${current_source_real}" "${input_file}")
    list(GET historical_rows ${history_index} historical_row)
    string(FIND "${historical_row}" "${tab}" separator)
    if(separator LESS 1)
      accuracycoin_binding_fail("${out_error}" "historical manifest row ${history_index} is malformed")
      return()
    endif()
    string(SUBSTRING "${historical_row}" 0 ${separator} historical_path)
    math(EXPR hash_offset "${separator} + 1")
    string(SUBSTRING "${historical_row}" ${hash_offset} -1 historical_hash)
    string(LENGTH "${historical_hash}" historical_hash_length)
    if(NOT historical_path STREQUAL relative_path OR NOT historical_hash_length EQUAL 64 OR
       NOT historical_hash MATCHES "^[0-9a-f]+$")
      accuracycoin_binding_fail("${out_error}" "historical manifest paths/order/hash are invalid at row ${history_index}")
      return()
    endif()
    string(APPEND historical_manifest "${historical_path}\t${historical_hash}\n")
    accuracycoin_compute_input_file_hash("${relative_path}" "${input_file}" current_hash)
    if(NOT relative_path IN_LIST maintenance_paths AND NOT current_hash STREQUAL historical_hash)
      accuracycoin_binding_fail("${out_error}" "protected AccuracyCoin measurement input changed: ${relative_path}")
      return()
    endif()
    math(EXPR history_index "${history_index} + 1")
  endforeach()
  if(NOT historical_manifest STREQUAL recorded_history_manifest)
    accuracycoin_binding_fail("${out_error}" "recorded historical path/hash manifest is not canonical")
    return()
  endif()
  string(SHA256 reconstructed_history_sha "${historical_manifest}")
  if(NOT reconstructed_history_sha STREQUAL "78822adfa35ac2b0e2f562cfb0067ae08db7e76c3e6e4027fa63d7bcca400de2")
    accuracycoin_binding_fail("${out_error}" "historical manifest differs from its pinned 56-input identity")
    return()
  endif()
  set(${out_error} "" PARENT_SCOPE)
endfunction()
