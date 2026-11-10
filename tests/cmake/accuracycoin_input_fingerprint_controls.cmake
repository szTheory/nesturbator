cmake_minimum_required(VERSION 3.25)

foreach(required IN ITEMS SOURCE_DIR WORK)
  if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
    message(FATAL_ERROR "AccuracyCoin input fingerprint controls need ${required}")
  endif()
endforeach()

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
   NOT expected_count STREQUAL current_count)
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
string(SHA256 changed_cmake_hash "${changed_cmake}")
string(REPLACE "93de8a1181f3aca9fd7450b83c85f065e379abb63cc4c54a53fb47fda00ce67e"
  "${changed_cmake_hash}" altered_history "${binding}")
if(altered_history STREQUAL binding)
  message(FATAL_ERROR "historical path/hash mutation control did not alter its fixture")
endif()
file(WRITE "${binding_fixture}/CMakeLists.txt" "${changed_cmake}")
file(WRITE "${binding_fixture}/tests/accuracy/input-binding-v1.json" "${altered_history}")
binding_expect_rejection("historical manifest hash" "historical manifest differs from its pinned 56-input identity")
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

message(STATUS "AccuracyCoin input fingerprint controls passed")
