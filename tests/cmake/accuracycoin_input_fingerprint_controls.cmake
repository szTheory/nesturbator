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
if(NOT expected_hash STREQUAL current_hash OR NOT expected_count STREQUAL current_count)
  message(FATAL_ERROR
    "baseline input identity is stale: ${expected_hash}/${expected_count}, current ${current_hash}/${current_count}")
endif()

message(STATUS "AccuracyCoin input fingerprint controls passed")
