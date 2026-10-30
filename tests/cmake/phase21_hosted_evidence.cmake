cmake_minimum_required(VERSION 3.25)

get_filename_component(NESTURBATOR_PHASE21_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
file(READ "${NESTURBATOR_PHASE21_SOURCE_DIR}/include/nesturbator.h" nesturbator_header)
file(READ "${NESTURBATOR_PHASE21_SOURCE_DIR}/version.txt" nesturbator_version_file)
string(STRIP "${nesturbator_version_file}" nesturbator_version_file)

foreach(part IN ITEMS MAJOR MINOR PATCH)
  string(REGEX MATCH "#define NESTURBATOR_VERSION_${part}[ \t]+([0-9]+)"
    version_match "${nesturbator_header}")
  if(version_match STREQUAL "")
    message(FATAL_ERROR "Phase 21 hosted contract cannot read NESTURBATOR_VERSION_${part}")
  endif()
  set(nesturbator_version_${part} "${CMAKE_MATCH_1}")
endforeach()
set(NESTURBATOR_PHASE20_LIBRARY_VERSION
  "${nesturbator_version_MAJOR}.${nesturbator_version_MINOR}.${nesturbator_version_PATCH}")
if(NOT NESTURBATOR_PHASE20_LIBRARY_VERSION STREQUAL nesturbator_version_file)
  message(FATAL_ERROR "Phase 21 hosted contract version.txt disagrees with the public version macros")
endif()
string(REGEX MATCH "#define NESTURBATOR_BEHAVIOUR_REVISION[ \t]+([0-9]+)"
  revision_match "${nesturbator_header}")
if(revision_match STREQUAL "")
  message(FATAL_ERROR "Phase 21 hosted contract cannot read NESTURBATOR_BEHAVIOUR_REVISION")
endif()
set(NESTURBATOR_PHASE20_BEHAVIOUR_REVISION "${CMAKE_MATCH_1}")

# Phase 21 selects the measured source contract through the shared Phase 20 validators.
set(NESTURBATOR_PHASE20_PROTECTED_MINIMUM 114)
set(NESTURBATOR_PHASE20_REQUIRED_TARGET_KEYS
  "accuracycoin/Interrupt flag latency"
  "accuracycoin/NMI Overlap BRK"
  "accuracycoin/NMI Overlap IRQ"
  "accuracycoin/$2002 flag timing"
  "accuracycoin/NMI Control"
  "accuracycoin/NMI Timing"
  "accuracycoin/NMI Suppression"
  "accuracycoin/NMI at VBlank end"
  "accuracycoin/NMI disabled at VBlank"
  "accuracycoin/Sprite 0 Hit behavior"
  "accuracycoin/Arbitrary Sprite zero")

# These remain the current workflow artifact names, including phase20-perf.
set(NESTURBATOR_PHASE20_PERF_ARTIFACT_PREFIX "phase20-perf")
set(NESTURBATOR_PHASE20_MARKER_ARTIFACT "retroarch-marker-progression")
set(NESTURBATOR_PHASE20_VECTORS_ARTIFACT_PREFIX "vectors-full-evidence")
set(NESTURBATOR_PHASE20_MMC3_ARTIFACT_PREFIX "mmc3-oracle-evidence")
set(NESTURBATOR_PHASE20_HASH_ARTIFACT_PREFIX "hashes")
set(NESTURBATOR_PHASE20_ACCURACYCOIN_ARTIFACT_PREFIX "accuracycoin-evidence")

include("${CMAKE_CURRENT_LIST_DIR}/phase20_hosted_evidence.cmake")
