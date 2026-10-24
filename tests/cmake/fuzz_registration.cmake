cmake_minimum_required(VERSION 3.25)
foreach(required IN ITEMS CTEST BUILD_DIR EXECUTION_BINARY EXECUTION_CORPUS_DIR)
  if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
    message(FATAL_ERROR "fuzz registration guard needs ${required}")
  endif()
endforeach()

execute_process(
  COMMAND "${CTEST}" --test-dir "${BUILD_DIR}" --show-only=json-v1
  RESULT_VARIABLE result
  OUTPUT_VARIABLE inventory
  ERROR_VARIABLE error)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "could not inspect CTest inventory: ${error}")
endif()

function(require_registration inventory required out_ok)
  string(REPLACE "." "\\." pattern "${required}")
  if(inventory MATCHES "\"name\"[ \t]*:[ \t]*\"${pattern}\"")
    set(${out_ok} TRUE PARENT_SCOPE)
  else()
    set(${out_ok} FALSE PARENT_SCOPE)
  endif()
endfunction()

foreach(required fuzz.regress fuzz.state fuzz.state.regress
                 fuzz.execution.regress fuzz.execution.seeds fuzz.execution.controls
                 fuzz.execution.oracle_failure fuzz.execution.campaign_controls)
  require_registration("${inventory}" "${required}" registered)
  if(NOT registered)
    message(FATAL_ERROR "CTest does not register the required ${required} test")
  endif()
endforeach()

# Negative control: the guard must reject removal of every required execution
# replay/control registration instead of inferring coverage from job text.
foreach(required fuzz.execution.regress fuzz.execution.seeds fuzz.execution.controls
                 fuzz.execution.oracle_failure fuzz.execution.campaign_controls)
  string(REPLACE "." "\\." registration_pattern "${required}")
  string(REGEX REPLACE "\"name\"[ \t]*:[ \t]*\"${registration_pattern}\""
    "\"name\":\"__removed__\"" mutated_inventory "${inventory}")
  require_registration("${mutated_inventory}" "${required}" still_registered)
  if(still_registered)
    message(FATAL_ERROR "registration guard negative control accepted removed ${required}")
  endif()
endforeach()

if(NOT IS_DIRECTORY "${EXECUTION_CORPUS_DIR}")
  message(FATAL_ERROR "execution seed corpus directory is missing")
endif()
file(GLOB execution_seeds LIST_DIRECTORIES FALSE "${EXECUTION_CORPUS_DIR}/*")
list(LENGTH execution_seeds execution_seed_count)
if(NOT execution_seed_count EQUAL 8)
  message(FATAL_ERROR "execution seed corpus must have eight nonempty descriptors; found ${execution_seed_count}")
endif()
foreach(seed IN LISTS execution_seeds)
  file(SIZE "${seed}" seed_size)
  if(seed_size EQUAL 0)
    message(FATAL_ERROR "execution seed corpus contains an empty descriptor: ${seed}")
  endif()
endforeach()
execute_process(COMMAND "${EXECUTION_BINARY}" --check-seeds ${execution_seeds}
  RESULT_VARIABLE seeds_result OUTPUT_VARIABLE seeds_out ERROR_VARIABLE seeds_err)
if(NOT seeds_result EQUAL 0 OR NOT seeds_out MATCHES "six family, loop and JAM")
  message(FATAL_ERROR "execution seed qualification failed: ${seeds_err}${seeds_out}")
endif()

# Removing a seed from a copied fixture must fail the real seed checker.
set(negative_corpus "${BUILD_DIR}/execution-registration-negative-corpus")
file(MAKE_DIRECTORY "${negative_corpus}")
foreach(seed IN LISTS execution_seeds)
  get_filename_component(name "${seed}" NAME)
  if(NOT name STREQUAL "jam")
    file(COPY_FILE "${seed}" "${negative_corpus}/${name}" ONLY_IF_DIFFERENT)
  endif()
endforeach()
file(GLOB missing_seed_fixture LIST_DIRECTORIES FALSE "${negative_corpus}/*")
execute_process(COMMAND "${EXECUTION_BINARY}" --check-seeds ${missing_seed_fixture}
  RESULT_VARIABLE missing_seed_result OUTPUT_VARIABLE missing_seed_out ERROR_VARIABLE missing_seed_err)
if(missing_seed_result EQUAL 0)
  message(FATAL_ERROR "seed guard negative control accepted a corpus with JAM missing")
endif()
message(STATUS "fuzz registration guard passed: all required CTests and eight qualified execution descriptors are present")
