cmake_minimum_required(VERSION 3.25)

foreach(required IN ITEMS SOURCE_DIR WORK RUNNER ROM)
  if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
    message(FATAL_ERROR "Phase 21 hosted evidence control needs ${required}")
  endif()
endforeach()

# A complete Phase 21 candidate and its freshly captured authority pass through
# the shared validator under the current-source contract.
set(NESTURBATOR_PHASE20_EVIDENCE_LIBRARY_ONLY ON)
include("${SOURCE_DIR}/tests/cmake/phase21_hosted_evidence.cmake")

list(LENGTH NESTURBATOR_PHASE20_REQUIRED_TARGET_KEYS target_count)
if(NOT NESTURBATOR_PHASE20_PROTECTED_MINIMUM EQUAL 114 OR
   NOT target_count EQUAL 11 OR
   NOT NESTURBATOR_PHASE20_BEHAVIOUR_REVISION EQUAL 9 OR
   NOT NESTURBATOR_PHASE20_LIBRARY_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
  message(FATAL_ERROR "Phase 21 wrapper did not select the exact current-source contract")
endif()

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")
set(candidate "${WORK}/candidate-scoreboard.txt")
set(protected "${WORK}/protected-scoreboard.txt")
set(metadata "${WORK}/protected-metadata.txt")
configure_file("${SOURCE_DIR}/tests/accuracy/scoreboard.txt" "${candidate}" COPYONLY)
file(STRINGS "${candidate}" candidate_rows)
string(ASCII 9 tab)
set(protected_text "")
set(protected_count 0)
foreach(line IN LISTS candidate_rows)
  string(REPLACE "${tab}" ";" fields "${line}")
  list(LENGTH fields field_count)
  if(field_count EQUAL 5)
    list(GET fields 1 status)
    if(status STREQUAL "pass" AND protected_count LESS 114)
      list(GET fields 0 key)
      list(GET fields 2 raw)
      string(APPEND protected_text "${key}\tpass\t${raw}\t-\t-\n")
      math(EXPR protected_count "${protected_count} + 1")
    endif()
  endif()
endforeach()
if(NOT protected_count EQUAL NESTURBATOR_PHASE20_PROTECTED_MINIMUM)
  message(FATAL_ERROR "Phase 21 control needs ${NESTURBATOR_PHASE20_PROTECTED_MINIMUM} source PASS rows")
endif()
file(WRITE "${protected}" "${protected_text}")
file(SHA256 "${protected}" protected_sha)
set(base_sha "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb")
file(WRITE "${metadata}"
  "schema=1\nauthority=protected-main\nsource=canonical-baseline\nprotected_revision=${base_sha}\nscoreboard_rows=${protected_count}\nscoreboard_denominator=${protected_count}\nscoreboard_sha256=${protected_sha}\n")

phase20_validate_scoreboard("${candidate}" "${protected}" "${metadata}" "${base_sha}" error)
if(NOT error STREQUAL "")
  message(FATAL_ERROR "Phase 21 complete candidate/control sibling was rejected: ${error}")
endif()
message(STATUS "Phase 21 clean sibling accepted by shared hosted evidence contract")

# The run-specific source manifest is deliberately outside the shared collector:
# these controls exercise its missing and altered evidence diagnostics only.
set(source_manifest "${WORK}/source-modes-blobs.tsv")
set(source_identity_script "${SOURCE_DIR}/tests/cmake/phase21_source_identity.cmake")
set(identity_profile "${WORK}/source-identity-profile.json")
execute_process(COMMAND "${CMAKE_COMMAND}" --profiling-format=google-trace
  "--profiling-output=${identity_profile}" -DMODE=write "-DROOT=${SOURCE_DIR}"
  "-DMANIFEST=${source_manifest}" -P "${source_identity_script}"
  RESULT_VARIABLE manifest_write_result OUTPUT_VARIABLE manifest_write_output
  ERROR_VARIABLE manifest_write_error)
if(NOT "${manifest_write_result}" STREQUAL "0")
  message(FATAL_ERROR "Phase 21 mode/blob control could not write its fixture: ${manifest_write_output}${manifest_write_error}")
endif()
# This is a work budget, independent of host speed. The positive inventory and
# rejection controls still cover every tracked input under the same 120s cap.
file(READ "${identity_profile}" identity_profile_text)
string(REGEX MATCHALL "\"name\"[ \t\r\n]*:[ \t\r\n]*\"execute_process\""
  identity_processes "${identity_profile_text}")
list(LENGTH identity_processes identity_process_count)
if(identity_process_count GREATER 12 OR identity_process_count LESS 1)
  message(FATAL_ERROR "Phase 21 source identity process budget exceeded: ${identity_process_count}, maximum 12")
endif()
message(STATUS "Phase 21 source identity process budget: ${identity_process_count}/12")
execute_process(COMMAND "${CMAKE_COMMAND}" -DMODE=verify "-DROOT=${SOURCE_DIR}"
  "-DMANIFEST=${source_manifest}" -P "${source_identity_script}"
  RESULT_VARIABLE manifest_verify_result OUTPUT_VARIABLE manifest_verify_output
  ERROR_VARIABLE manifest_verify_error)
if(NOT "${manifest_verify_result}" STREQUAL "0")
  message(FATAL_ERROR "Phase 21 mode/blob clean sibling was rejected: ${manifest_verify_output}${manifest_verify_error}")
endif()
execute_process(COMMAND "${CMAKE_COMMAND}" -DMODE=verify "-DROOT=${SOURCE_DIR}"
  "-DMANIFEST=${WORK}/missing-source-modes-blobs.tsv" -P "${source_identity_script}"
  RESULT_VARIABLE manifest_missing_result OUTPUT_VARIABLE manifest_missing_output
  ERROR_VARIABLE manifest_missing_error)
if("${manifest_missing_result}" STREQUAL "0" OR
   NOT "${manifest_missing_output}${manifest_missing_error}" MATCHES "modes/blob manifest is missing")
  message(FATAL_ERROR "Phase 21 mode/blob missing-evidence control failed at the wrong stage: ${manifest_missing_output}${manifest_missing_error}")
endif()
file(READ "${source_manifest}" source_manifest_text)
string(ASCII 9 tab)
string(REPLACE "${tab}100644${tab}" "${tab}100755${tab}"
  source_manifest_text "${source_manifest_text}")
file(WRITE "${source_manifest}" "${source_manifest_text}")
execute_process(COMMAND "${CMAKE_COMMAND}" -DMODE=verify "-DROOT=${SOURCE_DIR}"
  "-DMANIFEST=${source_manifest}" -P "${source_identity_script}"
  RESULT_VARIABLE manifest_changed_result OUTPUT_VARIABLE manifest_changed_output
  ERROR_VARIABLE manifest_changed_error)
if("${manifest_changed_result}" STREQUAL "0" OR
   NOT "${manifest_changed_output}${manifest_changed_error}" MATCHES "manifest differs from current checkout")
  message(FATAL_ERROR "Phase 21 mode/blob changed-evidence control failed at the wrong stage: ${manifest_changed_output}${manifest_changed_error}")
endif()
message(STATUS "Phase 21 run-specific tracked mode/blob controls rejected missing and altered evidence")
execute_process(COMMAND "${CMAKE_COMMAND}" "-DSOURCE_DIR=${SOURCE_DIR}"
  "-DWORK=${WORK}/identity-semantics"
  -P "${SOURCE_DIR}/tests/cmake/phase21_source_identity_controls.cmake"
  RESULT_VARIABLE identity_result OUTPUT_VARIABLE identity_output ERROR_VARIABLE identity_error)
if(NOT "${identity_result}" STREQUAL "0")
  message(FATAL_ERROR "Phase 21 identity semantic controls failed: ${identity_output}${identity_error}")
endif()
message(STATUS "Phase 21 independent source identity semantic controls passed")

# A separate subprocess exercises the inherited content, source, inventory,
# host, performance, hash and provenance controls with Phase 21's values.
execute_process(
  COMMAND "${CMAKE_COMMAND}" -DCONTRACT=phase21 -DCASE=all
    "-DSOURCE_DIR=${SOURCE_DIR}" "-DRUNNER=${RUNNER}" "-DROM=${ROM}"
    "-DWORK=${WORK}/shared-controls"
    -P "${SOURCE_DIR}/tests/cmake/phase20_hosted_evidence_controls.cmake"
  RESULT_VARIABLE controls_result OUTPUT_VARIABLE controls_output ERROR_VARIABLE controls_error)
if(NOT "${controls_result}" STREQUAL "0")
  message(FATAL_ERROR
    "Phase 21 shared hosted evidence controls failed (${controls_result}): ${controls_output}${controls_error}")
endif()
message(STATUS "Phase 21 shared content/provenance controls passed under exact current contract")
