cmake_minimum_required(VERSION 3.25)

foreach(required IN ITEMS SOURCE_DIR WORK)
  if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
    message(FATAL_ERROR "AccuracyCoin input fingerprint controls need ${required}")
  endif()
endforeach()

get_filename_component(SOURCE_DIR "${SOURCE_DIR}" ABSOLUTE)
get_filename_component(WORK "${WORK}" ABSOLUTE)
include("${SOURCE_DIR}/tests/cmake/accuracycoin_input_fingerprint.cmake")
file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")

set(header "#define NESTURBATOR_VERSION_MAJOR 0 /* x-release-please-major */\n#define NESTURBATOR_VERSION_MINOR 1 /* x-release-please-minor */\n#define NESTURBATOR_VERSION_PATCH 11 /* x-release-please-patch */\n#define NESTURBATOR_ABI_VERSION 1\n")
file(WRITE "${WORK}/header-v11.h" "${header}")
accuracycoin_compute_input_file_hash("include/nesturbator.h"
  "${WORK}/header-v11.h" header_v11_hash)
foreach(component IN ITEMS MAJOR MINOR PATCH)
  set(new_value 1)
  if(component STREQUAL "MINOR")
    set(new_value 2)
  elseif(component STREQUAL "PATCH")
    set(new_value 12)
  endif()
  set(versioned_header "${header}")
  string(REGEX REPLACE "(#define NESTURBATOR_VERSION_${component} )[0-9]+"
    "\\1${new_value}" versioned_header "${versioned_header}")
  set(versioned_path "${WORK}/header-${component}.h")
  file(WRITE "${versioned_path}" "${versioned_header}")
  accuracycoin_compute_input_file_hash("include/nesturbator.h"
    "${versioned_path}" versioned_hash)
  if(NOT header_v11_hash STREQUAL versioned_hash)
    message(FATAL_ERROR "release ${component} version changed the AccuracyCoin input hash")
  endif()
endforeach()

set(header_abi_v2 "${header}")
string(REPLACE "NESTURBATOR_ABI_VERSION 1" "NESTURBATOR_ABI_VERSION 2"
  header_abi_v2 "${header_abi_v2}")
file(WRITE "${WORK}/header-abi-v2.h" "${header_abi_v2}")
accuracycoin_compute_input_file_hash("include/nesturbator.h"
  "${WORK}/header-abi-v2.h" header_abi_v2_hash)
if(header_v11_hash STREQUAL header_abi_v2_hash)
  message(FATAL_ERROR "non-version public header change did not change its input hash")
endif()

accuracycoin_compute_input_fingerprint("${SOURCE_DIR}" current_hash current_count)
file(STRINGS "${SOURCE_DIR}/tests/accuracy/baseline-metadata.txt" fingerprint_rows
  REGEX "^(build_inputs_sha256|build_inputs_count)=")
set(expected_hash "")
set(expected_count "")
foreach(row IN LISTS fingerprint_rows)
  if(row MATCHES "^build_inputs_sha256=([0-9a-f]+)$")
    if(NOT expected_hash STREQUAL "")
      message(FATAL_ERROR "baseline metadata repeats build_inputs_sha256")
    endif()
    set(expected_hash "${CMAKE_MATCH_1}")
  elseif(row MATCHES "^build_inputs_count=([0-9]+)$")
    if(NOT expected_count STREQUAL "")
      message(FATAL_ERROR "baseline metadata repeats build_inputs_count")
    endif()
    set(expected_count "${CMAKE_MATCH_1}")
  endif()
endforeach()
if(NOT expected_hash STREQUAL "78822adfa35ac2b0e2f562cfb0067ae08db7e76c3e6e4027fa63d7bcca400de2" OR
   NOT expected_count STREQUAL "56" OR NOT current_count EQUAL 57)
  message(FATAL_ERROR
    "historical baseline identity is not pinned: ${expected_hash}/${expected_count}, current path count ${current_count}")
endif()

if(NOT COMMAND accuracycoin_validate_input_binding)
  message(FATAL_ERROR
    "historical/current binding acceptance assertion failed: validator is not defined")
endif()
accuracycoin_validate_input_binding("${SOURCE_DIR}" binding_error)
if(binding_error)
  message(FATAL_ERROR "historical/current binding acceptance assertion failed: ${binding_error}")
endif()


set(binding_fixture "${WORK}/binding-fixture")
file(REMOVE_RECURSE "${binding_fixture}")
file(MAKE_DIRECTORY "${binding_fixture}")
accuracycoin_input_paths("${SOURCE_DIR}" fixture_inputs)
foreach(input_file IN LISTS fixture_inputs)
  file(RELATIVE_PATH relative_path "${SOURCE_DIR}" "${input_file}")
  get_filename_component(parent "${binding_fixture}/${relative_path}" DIRECTORY)
  file(MAKE_DIRECTORY "${parent}")
  configure_file("${input_file}" "${binding_fixture}/${relative_path}" COPYONLY)
endforeach()
foreach(relative_path IN ITEMS
    tests/accuracy/baseline-metadata.txt
    tests/accuracy/scoreboard.txt
    tests/accuracy/input-binding-v1.json)
  get_filename_component(parent "${binding_fixture}/${relative_path}" DIRECTORY)
  file(MAKE_DIRECTORY "${parent}")
  configure_file("${SOURCE_DIR}/${relative_path}" "${binding_fixture}/${relative_path}" COPYONLY)
endforeach()

function(binding_expect_rejection label expected_pattern)
  accuracycoin_validate_input_binding("${binding_fixture}" rejection)
  if(rejection STREQUAL "" OR NOT rejection MATCHES "${expected_pattern}")
    message(FATAL_ERROR "${label} binding mutation reached '${rejection}'")
  endif()
endfunction()

file(APPEND "${binding_fixture}/src/instance.c" "\n/* synthetic historical-input mutation */\n")
binding_expect_rejection("protected source input" "protected AccuracyCoin measurement input changed: src/instance.c")
configure_file("${SOURCE_DIR}/src/instance.c" "${binding_fixture}/src/instance.c" COPYONLY)

file(APPEND "${binding_fixture}/tests/accuracy/baseline-metadata.txt" "\n")
binding_expect_rejection("baseline metadata" "historical baseline metadata bytes changed")
configure_file("${SOURCE_DIR}/tests/accuracy/baseline-metadata.txt"
  "${binding_fixture}/tests/accuracy/baseline-metadata.txt" COPYONLY)

file(APPEND "${binding_fixture}/tests/accuracy/scoreboard.txt" "\n")
binding_expect_rejection("scoreboard" "historical AccuracyCoin scoreboard bytes changed")
configure_file("${SOURCE_DIR}/tests/accuracy/scoreboard.txt"
  "${binding_fixture}/tests/accuracy/scoreboard.txt" COPYONLY)

file(APPEND "${binding_fixture}/tests/cmake/accuracycoin_activation.cmake" "\n# synthetic checker mutation\n")
binding_expect_rejection("activation checker" "reviewed checker bytes differ from the binding")
configure_file("${SOURCE_DIR}/tests/cmake/accuracycoin_activation.cmake"
  "${binding_fixture}/tests/cmake/accuracycoin_activation.cmake" COPYONLY)

file(APPEND "${binding_fixture}/tests/cmake/accuracycoin_input_fingerprint.cmake" "\n# synthetic checker mutation\n")
binding_expect_rejection("fingerprint helper" "reviewed checker bytes differ from the binding")
configure_file("${SOURCE_DIR}/tests/cmake/accuracycoin_input_fingerprint.cmake"
  "${binding_fixture}/tests/cmake/accuracycoin_input_fingerprint.cmake" COPYONLY)

file(READ "${binding_fixture}/tests/accuracy/input-binding-v1.json" binding)
set(changed_cmake "cmake_minimum_required(VERSION 3.25)\n# synthetic maintenance identity mutation\n")
# Hash the persisted fixture bytes, including its requested line endings.
# Both formats must reach the pinned aggregate guard on every host.
foreach(newline_style IN ITEMS LF CRLF)
  file(CONFIGURE OUTPUT "${binding_fixture}/CMakeLists.txt"
    CONTENT "${changed_cmake}" @ONLY NEWLINE_STYLE "${newline_style}")
  file(SHA256 "${binding_fixture}/CMakeLists.txt" changed_cmake_hash)
  string(REPLACE "93de8a1181f3aca9fd7450b83c85f065e379abb63cc4c54a53fb47fda00ce67e"
    "${changed_cmake_hash}" altered_history "${binding}")
  if(altered_history STREQUAL binding)
    message(FATAL_ERROR "historical path/hash mutation control did not alter its fixture")
  endif()
  file(WRITE "${binding_fixture}/tests/accuracy/input-binding-v1.json" "${altered_history}")
  binding_expect_rejection("historical manifest hash ${newline_style}"
    "historical manifest differs from its pinned 56-input identity")
endforeach()
configure_file("${SOURCE_DIR}/CMakeLists.txt" "${binding_fixture}/CMakeLists.txt" COPYONLY)
configure_file("${SOURCE_DIR}/tests/accuracy/input-binding-v1.json"
  "${binding_fixture}/tests/accuracy/input-binding-v1.json" COPYONLY)

file(READ "${binding_fixture}/tests/accuracy/input-binding-v1.json" binding)
string(JSON altered_binding SET "${binding}" caller_exemptions "[\"src/instance.c\"]")
file(WRITE "${binding_fixture}/tests/accuracy/input-binding-v1.json" "${altered_binding}\n")
binding_expect_rejection("caller-selected exemption" "binding has missing or unknown fields")

configure_file("${SOURCE_DIR}/tests/accuracy/input-binding-v1.json"
  "${binding_fixture}/tests/accuracy/input-binding-v1.json" COPYONLY)
file(READ "${binding_fixture}/tests/accuracy/input-binding-v1.json" binding)
string(JSON missing_binding REMOVE "${binding}" historical_input_count)
file(WRITE "${binding_fixture}/tests/accuracy/input-binding-v1.json" "${missing_binding}\n")
binding_expect_rejection("missing binding field" "binding has missing or unknown fields")

configure_file("${SOURCE_DIR}/tests/accuracy/input-binding-v1.json"
  "${binding_fixture}/tests/accuracy/input-binding-v1.json" COPYONLY)
file(READ "${binding_fixture}/tests/accuracy/input-binding-v1.json" binding)
string(REGEX REPLACE "\"schema_version\"[ \t\r\n]*:[ \t\r\n]*1"
  "\"schema_version\":1,\"schema_version\":1" duplicate_binding "${binding}")
if(duplicate_binding STREQUAL binding)
  message(FATAL_ERROR "duplicate binding field control did not alter its fixture")
endif()
file(WRITE "${binding_fixture}/tests/accuracy/input-binding-v1.json" "${duplicate_binding}\n")
binding_expect_rejection("duplicate binding field" "binding")

configure_file("${SOURCE_DIR}/tests/accuracy/input-binding-v1.json"
  "${binding_fixture}/tests/accuracy/input-binding-v1.json" COPYONLY)
file(READ "${binding_fixture}/tests/accuracy/input-binding-v1.json" binding)
string(JSON mistyped_binding SET "${binding}" schema_version "\"one\"")
file(WRITE "${binding_fixture}/tests/accuracy/input-binding-v1.json" "${mistyped_binding}\n")
binding_expect_rejection("mistyped binding field" "binding schema_version must be NUMBER, found STRING")

configure_file("${SOURCE_DIR}/tests/accuracy/input-binding-v1.json"
  "${binding_fixture}/tests/accuracy/input-binding-v1.json" COPYONLY)
foreach(path IN ITEMS src/mapper_mmc2.c src/cartridge.c tests/CMakeLists.txt)
  file(APPEND "${binding_fixture}/${path}" "\n# changed\n")
  binding_expect_rejection("reviewed current bytes" "input changed|registration CMake bytes")
  configure_file("${SOURCE_DIR}/${path}" "${binding_fixture}/${path}" COPYONLY)
endforeach()
file(WRITE "${binding_fixture}/src/unapproved.c" "/* extra input */\n")
binding_expect_rejection("extra path" "closure differs")
file(REMOVE "${binding_fixture}/src/unapproved.c")
file(REMOVE "${binding_fixture}/src/mapper_mmc2.c")
binding_expect_rejection("missing mapper" "closure differs")
configure_file("${SOURCE_DIR}/src/mapper_mmc2.c" "${binding_fixture}/src/mapper_mmc2.c" COPYONLY)

# The reviewed current map and registration array admit neither omissions nor
# reordering. Restoring the real binding makes each mutation independent.
file(READ "${SOURCE_DIR}/tests/accuracy/input-binding-v1.json" original_binding)
string(JSON altered_binding REMOVE "${original_binding}" current_input_hashes "src/mapper_mmc2.c")
file(WRITE "${binding_fixture}/tests/accuracy/input-binding-v1.json" "${altered_binding}")
binding_expect_rejection("missing current hash" "exact seven reviewed paths")
string(JSON altered_binding SET "${original_binding}" current_input_hashes "src/mapper_mmc2.c" "\"bad\"")
file(WRITE "${binding_fixture}/tests/accuracy/input-binding-v1.json" "${altered_binding}")
binding_expect_rejection("malformed current hash" "current input hash.*malformed")
string(JSON altered_binding SET "${original_binding}" allowed_registration_names 0 "\"provider.profile\"")
file(WRITE "${binding_fixture}/tests/accuracy/input-binding-v1.json" "${altered_binding}")
binding_expect_rejection("reordered registrations" "registration names differ")
configure_file("${SOURCE_DIR}/tests/accuracy/input-binding-v1.json"
  "${binding_fixture}/tests/accuracy/input-binding-v1.json" COPYONLY)

foreach(escaped_key IN ITEMS "schema_version" "src/mapper_mmc2.c")
  if(escaped_key STREQUAL "schema_version")
    set(escaped_spelling "\\u0073chema_version")
    set(duplicate_value "1")
  else()
    set(escaped_spelling "\\u0073rc/mapper_mmc2.c")
    string(JSON duplicate_value GET "${original_binding}" current_input_hashes "${escaped_key}")
    set(duplicate_value "\"${duplicate_value}\"")
  endif()
  string(REPLACE "\"${escaped_key}\":" "\"${escaped_spelling}\":${duplicate_value},\"${escaped_key}\":" escaped_binding "${original_binding}")
  if(escaped_binding STREQUAL original_binding)
    message(FATAL_ERROR "escaped member control did not alter fixture")
  endif()
  file(WRITE "${binding_fixture}/tests/accuracy/input-binding-v1.json" "${escaped_binding}")
  binding_expect_rejection("escaped duplicate member" "member names must not contain escapes")
endforeach()
configure_file("${SOURCE_DIR}/tests/accuracy/input-binding-v1.json"
  "${binding_fixture}/tests/accuracy/input-binding-v1.json" COPYONLY)

# Owned metadata/report exercise freshness without invoking campaign activation.
file(WRITE "${WORK}/report.tsv" "owned freshness fixture\n")
file(SHA256 "${WORK}/report.tsv" report_hash)
accuracycoin_compute_input_fingerprint("${binding_fixture}" fixture_hash fixture_count)
set(fresh "build_inputs_sha256=${fixture_hash}\nbuild_inputs_count=${fixture_count}\ncampaign_report_sha256=${report_hash}\n")
function(freshness_expect label metadata should_pass)
  file(WRITE "${WORK}/metadata.txt" "${metadata}")
  accuracycoin_validate_campaign_metadata("${binding_fixture}" "${WORK}/metadata.txt"
    "${WORK}/report.tsv" "${report_hash}" error)
  if(should_pass AND error OR NOT should_pass AND NOT error)
    message(FATAL_ERROR "${label}: unexpected freshness verdict '${error}'")
  endif()
endfunction()
freshness_expect("fresh metadata" "${fresh}" TRUE)
freshness_expect("absent fields" "" FALSE)
freshness_expect("malformed row" "malformed metadata\n${fresh}" FALSE)
freshness_expect("duplicate hash" "${fresh}build_inputs_sha256=${fixture_hash}\n" FALSE)
string(REPLACE "build_inputs_count=${fixture_count}" "build_inputs_count=bogus" malformed "${fresh}")
freshness_expect("malformed count" "${malformed}" FALSE)
string(REPLACE "build_inputs_count=${fixture_count}" "build_inputs_count=56" stale "${fresh}")
freshness_expect("stale count" "${stale}" FALSE)
file(APPEND "${binding_fixture}/src/mapper_mmc2.c" "\n/* post-run mutation */\n")
freshness_expect("post-run source mutation" "${fresh}" FALSE)
configure_file("${SOURCE_DIR}/src/mapper_mmc2.c" "${binding_fixture}/src/mapper_mmc2.c" COPYONLY)
file(APPEND "${WORK}/report.tsv" "changed\n")
freshness_expect("altered report" "${fresh}" FALSE)
message(STATUS "AccuracyCoin input fingerprint controls passed: current closure and fresh campaign negatives")
