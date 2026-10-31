# Scoped public report and command contract for docs/reproduction.md.
#
#   MODE=check validates the report/replay/storage sections and generated CTest metadata.
#   MODE=self_test requires independent broken copies to fail for named reasons.

cmake_minimum_required(VERSION 3.25)

function(reproduction_require_contains content needle diagnostic)
  string(FIND "${content}" "${needle}" found)
  if(found EQUAL -1)
    message(FATAL_ERROR "docs.reproduction: ${diagnostic}")
  endif()
endfunction()

function(reproduction_require_test_property tests_json test_name property expected diagnostic)
  string(JSON test_count LENGTH "${tests_json}" tests)
  set(found_test FALSE)
  set(found_property FALSE)
  set(observed_values)
  foreach(test_index RANGE 0 ${test_count})
    if(test_index GREATER_EQUAL test_count)
      break()
    endif()
    string(JSON candidate_name GET "${tests_json}" tests ${test_index} name)
    if(NOT candidate_name STREQUAL test_name)
      continue()
    endif()
    set(found_test TRUE)
    string(JSON property_count LENGTH "${tests_json}" tests ${test_index} properties)
    foreach(property_index RANGE 0 ${property_count})
      if(property_index GREATER_EQUAL property_count)
        break()
      endif()
      string(JSON candidate_property GET "${tests_json}" tests ${test_index} properties ${property_index} name)
      if(NOT candidate_property STREQUAL property)
        continue()
      endif()
      set(found_property TRUE)
      string(JSON value_type TYPE "${tests_json}" tests ${test_index} properties ${property_index} value)
      if(value_type STREQUAL "ARRAY")
        string(JSON value_count LENGTH "${tests_json}" tests ${test_index} properties ${property_index} value)
        foreach(value_index RANGE 0 ${value_count})
          if(value_index GREATER_EQUAL value_count)
            break()
          endif()
          string(JSON value GET "${tests_json}" tests ${test_index} properties ${property_index} value ${value_index})
          list(APPEND observed_values "${value}")
        endforeach()
      else()
        string(JSON value GET "${tests_json}" tests ${test_index} properties ${property_index} value)
        list(APPEND observed_values "${value}")
      endif()
      break()
    endforeach()
    break()
  endforeach()

  if(NOT found_test OR NOT found_property)
    message(FATAL_ERROR "docs.reproduction: ${diagnostic}")
  endif()
  if(property STREQUAL "TIMEOUT")
    if(NOT expected IN_LIST observed_values AND NOT "${expected}.0" IN_LIST observed_values)
      message(FATAL_ERROR "docs.reproduction: ${diagnostic}")
    endif()
  elseif(NOT expected IN_LIST observed_values)
    message(FATAL_ERROR "docs.reproduction: ${diagnostic}")
  endif()
endfunction()

function(reproduction_check doc_path)
  foreach(var SOURCE_DIR BUILD_DIR CTEST)
    if(NOT DEFINED ${var} OR "${${var}}" STREQUAL "")
      message(FATAL_ERROR "docs.reproduction: -D${var}=... is required")
    endif()
  endforeach()
  if(NOT EXISTS "${doc_path}")
    message(FATAL_ERROR "docs.reproduction: report document is missing")
  endif()

  file(READ "${doc_path}" document)
  string(FIND "${document}" "## Report a scene" report_start)
  string(FIND "${document}" "## Permitted fixture example" example_start)
  string(FIND "${document}" "## Cold movie replay" cold_start)
  string(FIND "${document}" "## Native checkpoint continuation" checkpoint_start)
  string(FIND "${document}" "## Before checkpoint replay" preflight_start)
  string(FIND "${document}" "## Scratch storage and bounds" scratch_start)
  string(FIND "${document}" "## Procedure result and runner exit status" result_start)
  string(FIND "${document}" "## Choose the next evidence source" triage_start)
  string(FIND "${document}" "## Evidence limits" evidence_limits_start)
  if(report_start EQUAL -1 OR example_start LESS report_start OR cold_start LESS example_start OR
     checkpoint_start LESS cold_start OR preflight_start LESS checkpoint_start OR
     scratch_start LESS preflight_start OR result_start LESS scratch_start OR
     triage_start LESS result_start OR evidence_limits_start LESS triage_start)
    message(FATAL_ERROR "docs.reproduction: report, fixture example and cold replay sections are out of order")
  endif()
  math(EXPR report_length "${example_start} - ${report_start}")
  string(SUBSTRING "${document}" ${report_start} ${report_length} report_section)
  math(EXPR cold_length "${checkpoint_start} - ${cold_start}")
  math(EXPR checkpoint_length "${preflight_start} - ${checkpoint_start}")
  math(EXPR preflight_length "${scratch_start} - ${preflight_start}")
  math(EXPR scratch_length "${result_start} - ${scratch_start}")
  math(EXPR result_length "${triage_start} - ${result_start}")
  math(EXPR triage_length "${evidence_limits_start} - ${triage_start}")
  string(SUBSTRING "${document}" ${cold_start} ${cold_length} cold_section)
  string(SUBSTRING "${document}" ${checkpoint_start} ${checkpoint_length} checkpoint_section)
  string(SUBSTRING "${document}" ${preflight_start} ${preflight_length} preflight_section)
  string(SUBSTRING "${document}" ${scratch_start} ${scratch_length} scratch_section)
  string(SUBSTRING "${document}" ${result_start} ${result_length} result_section)
  string(SUBSTRING "${document}" ${triage_start} ${triage_length} triage_section)
  string(SUBSTRING "${document}" ${evidence_limits_start} -1 evidence_limits_section)

  set(report_fields
    "Report revision/date:"
    "Title and region:"
    "Content revision or digest:"
    "Loaded core and exact build identity:"
    "Host and host version:"
    "Launch route and relevant settings:"
    "Exact scene or point reached:"
    "Access method and whether it was verified:"
    "Start conditions (cold boot, checkpoint, save, or other):"
    "Inputs in order, including port for each input:"
    "Expected behavior and its source/confidence:"
    "Observed behavior:"
    "Repeatability (attempts and outcomes):"
    "Conditions not tried:"
    "Evidence limits or unknowns:"
    "Attachments withheld from public report:")
  foreach(field IN LISTS report_fields)
    reproduction_require_contains("${report_section}" "${field}" "report field missing: ${field}")
  endforeach()

  foreach(value unknown withheld "not tried" unavailable)
    reproduction_require_contains("${report_section}" "${value}" "uncertainty value missing: ${value}")
  endforeach()
  reproduction_require_contains("${report_section}" "title-level observation" "title-level and exact-scene evidence are not distinguished")
  reproduction_require_contains("${report_section}" "expected behavior comes" "expected-behavior provenance is missing")
  reproduction_require_contains("${report_section}" "how confident that source is" "expected-behavior confidence is missing")
  reproduction_require_contains("${report_section}" "Do not publish commercial game media" "privacy claim missing: commercial game media")
  reproduction_require_contains("${report_section}" "saves" "privacy claim missing: saves")
  reproduction_require_contains("${report_section}" "native checkpoints" "privacy claim missing: native checkpoints")
  reproduction_require_contains("${report_section}" "raw private" "privacy claim missing: raw private logs or captures")
  reproduction_require_contains("${report_section}" "personal paths" "privacy claim missing: personal paths")
  reproduction_require_contains("${report_section}" "personal identifiers" "privacy claim missing: personal identifiers")

  foreach(required_heading "## Permitted fixture example" "## Cold movie replay")
    reproduction_require_contains("${document}" "${required_heading}" "required heading missing: ${required_heading}")
  endforeach()
  reproduction_require_contains("${cold_section}" "two-port NMOVIE1" "two-port NMOVIE1 validation is not documented")
  reproduction_require_contains("${cold_section}" "--frames" "unsupported movie/frame combination is not documented")
  reproduction_require_contains("${cold_section}" "--load-state" "unsupported movie/state combination is not documented")
  reproduction_require_contains("${cold_section}" "Host recording" "host recording limitation is not documented")
  reproduction_require_contains("${cold_section}" "foreign movie formats" "foreign movie conversion limitation is not documented")
  reproduction_require_contains("${cold_section}" "hardware-correctness evidence" "repeatability is overstated as correctness")

  foreach(required IN ITEMS
      "runner.reproduction.preflight"
      "runner.reproduction.preflight_self_test"
      "runner.state.nmi_roundtrip"
      "runner.state.nmi_idempotent"
      "runner.state.nmi_wrong_load"
      "neutral-input"
      "no movie"
      "K=1"
      "N=4"
      "2–4; it is not four additional frames."
      "transition and PCM suffix"
      "guest NMI event sequence"
      "stale-load and"
      "omitted-load guest-marker failures")
    reproduction_require_contains("${checkpoint_section}" "${required}" "native checkpoint route missing: ${required}")
  endforeach()
  foreach(required IN ITEMS
      "64 hexadecimal characters"
      "MODE=preflight"
      "STATE"
      "PAYLOAD_KIND"
      "runner-native-same-invocation"
      "RECORDED_RUNNER_SHA256"
      "RECORDED_ROM_SHA256"
      "RECORDED_STATE_SHA256"
      "declared origin label"
      "reproduction unsupported:"
      "host container stop"
      "not hostile-file authentication"
      "Native format version 3 and behavior revision 9 are separate values."
      "Successful preflight does not")
    reproduction_require_contains("${preflight_section}" "${required}" "checkpoint preflight policy missing: ${required}")
  endforeach()
  reproduction_require_contains("${preflight_section}" "does not enforce the exact executing build" "loader build-enforcement limit missing")
  reproduction_require_contains("${result_section}" "reproduction unsupported:" "procedure unsupported result is missing")
  reproduction_require_contains("${result_section}" "not a runner exit code" "procedure result is conflated with runner exits")
  foreach(exit_code IN ITEMS 0 1 2 4 5)
    reproduction_require_contains("${result_section}" "Runner exit ${exit_code}" "runner exit contract is missing ${exit_code}")
  endforeach()
  reproduction_require_contains("${result_section}" "Runner exits 3 and 77" "reserved runner exits are missing")
  reproduction_require_contains("${result_section}" "runner-native state cannot be renamed into a RetroArch disk state" "host/native container boundary is missing")
  reproduction_require_contains("${result_section}" "--movie" "movie-plus-state refusal is missing")

  foreach(required IN ITEMS
      "| Comparison | Supported inference | Remaining unknown | Next bounded evidence source |"
      "Scene, access route, content/build identity and inputs"
      "title-level observation, not scene acceptance"
      "Repeated native frame, transition and PCM windows"
      "repeatability for those native outputs"
      "Native pixels versus adapter RGB/callback output"
      "host.convert` covers palette indexing and source/output pitch"
      "libretro.host` compares selected video and PCM callback results"
      "These tests do not provide a display or audio device."
      "Frontend input, presentation, screenshot or audio timing"
      "dropped or doubled presentation"
      "Expected original-game behavior or hardware correctness"
      "deliberately wrong behavior control"
      "released reference-emulator binary with its version identified"
      "https://docs.libretro.com/development/cores/developing-cores/"
      "https://docs.libretro.com/guides/optimal-vsync/")
    reproduction_require_contains("${triage_section}" "${required}"
      "triage branch or evidence layer is missing: ${required}")
  endforeach()
  reproduction_require_contains("${triage_section}"
    "Use `runner.reproduction.cold` for two cold 600-frame executions or `runner.state.nmi_roundtrip` for its neutral K=1 through N=4 suffix."
    "native next-evidence missing")
  reproduction_require_contains("${triage_section}"
    "Unaligned screenshots are qualitative and cannot be equated to native hashes."
    "screenshot/native hash conflation")
  reproduction_require_contains("${triage_section}"
    "pacing, resampling, synchronization and physical devices remain frontend/host questions."
    "host timing attribution")
  reproduction_require_contains("${evidence_limits_section}" "Repeatability"
    "repeatability limit is missing")
  reproduction_require_contains("${evidence_limits_section}" "Fixture expectations"
    "fixture expectation limit is missing")
  reproduction_require_contains("${evidence_limits_section}" "Hardware correctness"
    "independent hardware-correctness limit is missing")
  reproduction_require_contains("${evidence_limits_section}" "does not show that the output is correct."
    "hash correctness conflation")
  reproduction_require_contains("${evidence_limits_section}" "guest marker"
    "narrow fixture-marker inference limit is missing")
  reproduction_require_contains("${evidence_limits_section}" "derived owned fixture"
    "independent fixture-oracle requirement is missing")
  reproduction_require_contains("${evidence_limits_section}" "without inspecting its source"
    "reference-emulator clean-room boundary is missing")
  reproduction_require_contains("${triage_section}" "GPL/LGPL emulator source"
    "prohibited emulator-source boundary is missing")
  reproduction_require_contains("${evidence_limits_section}" "These reports are not\ndiagnoses or fixes."
    "commercial status relabeled")
  reproduction_require_contains("${evidence_limits_section}"
    "[playtesting postmortem](../.planning/playtesting/POSTMORTEM.md#what-remains-unresolved)"
    "dated commercial evidence link is missing")
  if(NOT EXISTS "${SOURCE_DIR}/.planning/playtesting/POSTMORTEM.md")
    message(FATAL_ERROR "docs.reproduction: dated commercial evidence target is missing")
  endif()
  reproduction_require_contains("${evidence_limits_section}" "keep the result unclassified."
    "unclassified outcome missing")
  foreach(commercial_case IN ITEMS "Jurassic Park's OCEAN" "Crystalis's moving world seam"
      "Little Nemo" "Mega Man 3" "Shatterhand's drift, slowdown and music interruption"
      "Marble Madness timer comparison")
    reproduction_require_contains("${evidence_limits_section}" "${commercial_case}"
      "commercial observation is missing or relabeled: ${commercial_case}")
  endforeach()

  foreach(required IN ITEMS "mktemp -d" "[guid]::NewGuid()")
    reproduction_require_contains("${cold_section}" "${required}" "fresh POSIX/PowerShell scratch root is missing: ${required}")
  endforeach()
  foreach(required IN ITEMS
      "one new scratch root and build directory per invocation"
      "fresh nonce directory"
      "captured stdout/stderr"
      "Two separate CTest processes sharing one build tree are unsupported."
      "No example imports daily battery data."
      "600 movie frames per run; 2 cold runs"
      "runner.reproduction.preflight`: 60 s"
      "runner.reproduction.preflight_self_test`: 60 s"
      "runner.state.nmi_roundtrip`: 60 s"
      "runner.state.nmi_idempotent`: 60 s"
      "runner.state.nmi_wrong_load`: 60 s"
      "runner.save.fixture`: 10 s"
      "runner.boot_to_play`: 300 s"
      "runner.nesrunner_movie.*`: 10 s"
      "stop line"
      "partial")
    reproduction_require_contains("${scratch_section}" "${required}" "scratch/bounds policy missing: ${required}")
  endforeach()
  reproduction_require_contains("${scratch_section}" "`runner.reproduction.cold`: 180 s" "cold replay timeout bound missing")
  reproduction_require_contains("${scratch_section}" "outside every active example path" "newest battery backup must stay outside active paths")

  file(READ "${SOURCE_DIR}/README.md" readme)
  reproduction_require_contains("${readme}" "## Troubleshooting" "README troubleshooting heading missing")
  reproduction_require_contains("${readme}"
    "## Troubleshooting\n\n[Report a scene or reproduce a permitted fixture](docs/reproduction.md)"
    "README troubleshooting does not link directly to the report")
  foreach(path IN ITEMS
      "docs/runner.md"
      "docs/testing.md"
      "docs/progress.md"
      "tests/runner/game_movie.c")
    if(NOT EXISTS "${SOURCE_DIR}/${path}")
      message(FATAL_ERROR "docs.reproduction: linked file missing: ${path}")
    endif()
  endforeach()
  file(READ "${SOURCE_DIR}/docs/runner.md" runner_guide)
  file(READ "${SOURCE_DIR}/docs/testing.md" testing_guide)
  file(READ "${SOURCE_DIR}/docs/progress.md" progress_guide)
  file(READ "${SOURCE_DIR}/tests/runner/game_movie.c" movie_writer)
  reproduction_require_contains("${runner_guide}" "## Input movies" "runner guide section link is stale")
  reproduction_require_contains("${runner_guide}" "For the sanitized report form, bounded replay examples and next-evidence\ntriage, see the [scene reproduction guide](reproduction.md)."
    "runner guide does not link directly to scene reproduction")
  reproduction_require_contains("${testing_guide}" "# Testing and engineering" "testing guide heading is stale")
  reproduction_require_contains("${progress_guide}" "# Protecting emulator progress" "progress guide heading is stale")
  reproduction_require_contains("${testing_guide}" "[scene reproduction guide](reproduction.md)" "testing guide does not link to scene reproduction")
  reproduction_require_contains("${progress_guide}" "[scene reproduction guide](reproduction.md)" "progress guide does not link to scene reproduction")
  reproduction_require_contains("${movie_writer}" "nesrunner-pos" "owned movie scenario is missing")
  reproduction_require_contains("${cold_section}" "[runner guide](runner.md)" "runner guide link is stale")
  reproduction_require_contains("${cold_section}" "[testing guide](testing.md)" "testing guide link is stale")
  reproduction_require_contains("${cold_section}" "[progress guide](progress.md)" "progress guide link is stale")
  reproduction_require_contains("${document}" "../tests/runner/game_movie.c" "movie generator source link is stale")

  # Both shell examples must name the same source-build targets, and CTest's
  # generated command metadata must contain their corresponding executables.
  string(REGEX MATCHALL "cmake --build [^\n\r]*--target [^\n\r]*" build_commands "${cold_section}")
  list(LENGTH build_commands build_command_count)
  if(build_command_count LESS 2)
    message(FATAL_ERROR "docs.reproduction: POSIX and PowerShell build commands are required")
  endif()
  set(documented_targets)
  foreach(command IN LISTS build_commands)
    string(REGEX REPLACE "^.*--target[ \t]+" "" targets_text "${command}")
    string(REPLACE "\"" "" targets_text "${targets_text}")
    string(STRIP "${targets_text}" targets_text)
    string(REPLACE " " ";" targets "${targets_text}")
    list(APPEND documented_targets ${targets})
  endforeach()
  list(REMOVE_DUPLICATES documented_targets)

  string(REGEX MATCH "-R[ \t]+'[^']+'" selector_arg "${cold_section}")
  if(selector_arg STREQUAL "")
    message(FATAL_ERROR "docs.reproduction: bounded CTest selection is missing")
  endif()
  string(REGEX REPLACE "^-R[ \t]+'" "" test_regex "${selector_arg}")
  string(REGEX REPLACE "'$" "" test_regex "${test_regex}")
  string(LENGTH "${test_regex}" regex_length)
  if(regex_length LESS 5)
    message(FATAL_ERROR "docs.reproduction: bounded CTest selection is malformed")
  endif()
  string(SUBSTRING "${test_regex}" 0 2 regex_prefix)
  math(EXPR test_names_length "${regex_length} - 4")
  string(SUBSTRING "${test_regex}" 2 ${test_names_length} test_names_text)
  math(EXPR regex_suffix_start "${regex_length} - 2")
  string(SUBSTRING "${test_regex}" ${regex_suffix_start} 2 regex_suffix)
  if(NOT regex_prefix STREQUAL "^(" OR NOT regex_suffix STREQUAL ")$")
    message(FATAL_ERROR "docs.reproduction: bounded CTest selection must be an anchored name list")
  endif()
  string(REPLACE "|" ";" documented_tests "${test_names_text}")

  execute_process(COMMAND "${CTEST}" --test-dir "${BUILD_DIR}" --show-only=json-v1
    OUTPUT_VARIABLE all_tests_json ERROR_VARIABLE ctest_err RESULT_VARIABLE ctest_rc TIMEOUT 20)
  if(NOT "${ctest_rc}" STREQUAL "0")
    message(FATAL_ERROR "docs.reproduction: generated CTest metadata could not be read")
  endif()
  string(JSON all_test_count LENGTH "${all_tests_json}" tests)
  set(generated_tests)
  foreach(index RANGE 0 ${all_test_count})
    if(index GREATER_EQUAL all_test_count)
      break()
    endif()
    string(JSON test_name GET "${all_tests_json}" tests ${index} name)
    list(APPEND generated_tests "${test_name}")
  endforeach()
  foreach(test_name IN LISTS documented_tests)
    if(NOT test_name IN_LIST generated_tests)
      message(FATAL_ERROR "docs.reproduction: documented CTest test is not generated: ${test_name}")
    endif()
  endforeach()
  foreach(required_test IN ITEMS
      runner.reproduction.cold
      runner.reproduction.compare_self_test
      runner.reproduction.preflight
      runner.reproduction.preflight_self_test
      runner.boot_to_play
      runner.state.nmi_roundtrip
      runner.state.nmi_idempotent
      runner.state.nmi_wrong_load
      docs.links
      docs.links.self_test
      docs.reproduction
      docs.reproduction.self_test
      cmake.script_policy)
    if(NOT required_test IN_LIST documented_tests)
      message(FATAL_ERROR "docs.reproduction: documented CTest selection omits ${required_test}")
    endif()
  endforeach()

  execute_process(COMMAND "${CTEST}" --test-dir "${BUILD_DIR}" --show-only=json-v1 -R "${test_regex}"
    OUTPUT_VARIABLE selected_tests_json ERROR_VARIABLE selected_err RESULT_VARIABLE selected_rc TIMEOUT 20)
  if(NOT "${selected_rc}" STREQUAL "0")
    message(FATAL_ERROR "docs.reproduction: documented CTest selection could not be evaluated")
  endif()
  string(JSON selected_count LENGTH "${selected_tests_json}" tests)
  if(selected_count LESS 1)
    message(FATAL_ERROR "docs.reproduction: documented CTest selection matches no tests")
  endif()

  reproduction_require_test_property("${selected_tests_json}" "runner.reproduction.cold" "TIMEOUT" "180"
    "generated cold replay timeout is not 180 seconds")
  reproduction_require_test_property("${selected_tests_json}" "runner.reproduction.compare_self_test" "TIMEOUT" "60"
    "generated movie comparison self-test timeout is not 60 seconds")
  reproduction_require_test_property("${selected_tests_json}" "runner.reproduction.preflight" "TIMEOUT" "60"
    "generated checkpoint preflight timeout is not 60 seconds")
  reproduction_require_test_property("${selected_tests_json}" "runner.reproduction.preflight_self_test" "TIMEOUT" "60"
    "generated preflight self-test timeout is not 60 seconds")
  reproduction_require_test_property("${selected_tests_json}" "docs.reproduction" "TIMEOUT" "60"
    "generated reproduction document-check timeout is not 60 seconds")
  reproduction_require_test_property("${selected_tests_json}" "docs.reproduction.self_test" "TIMEOUT" "60"
    "generated reproduction document self-test timeout is not 60 seconds")
  reproduction_require_test_property("${selected_tests_json}" "runner.boot_to_play" "TIMEOUT" "300"
    "generated played-input control timeout is not 300 seconds")
  reproduction_require_test_property("${selected_tests_json}" "runner.save.fixture" "TIMEOUT" "10"
    "required save fixture setup has no 10-second timeout")
  reproduction_require_test_property("${selected_tests_json}" "runner.save.fixture" "FIXTURES_SETUP" "runner_save"
    "required save fixture setup is not registered")
  reproduction_require_test_property("${selected_tests_json}" "runner.boot_to_play" "FIXTURES_REQUIRED" "nesrunner_movies"
    "played-input control is missing its movie fixtures")
  foreach(case IN ITEMS nmi_roundtrip nmi_idempotent nmi_wrong_load)
    reproduction_require_test_property("${selected_tests_json}" "runner.state.${case}" "TIMEOUT" "60"
      "generated runner.state.${case} timeout is not 60 seconds")
    reproduction_require_test_property("${selected_tests_json}" "runner.state.${case}" "FIXTURES_REQUIRED" "runner_save"
      "generated runner.state.${case} save fixture dependency is missing")
    reproduction_require_test_property("${selected_tests_json}" "runner.state.${case}" "DEPENDS" "runner.save.fixture"
      "generated runner.state.${case} setup test dependency is missing")
  endforeach()
  foreach(scenario IN ITEMS pos none attract startonly)
    reproduction_require_test_property("${selected_tests_json}" "runner.nesrunner_movie.${scenario}" "TIMEOUT" "10"
      "required runner.nesrunner_movie.${scenario} setup timeout is not 10 seconds")
    reproduction_require_test_property("${selected_tests_json}" "runner.nesrunner_movie.${scenario}" "FIXTURES_SETUP" "nesrunner_movies"
      "required runner.nesrunner_movie.${scenario} fixture is not registered")
  endforeach()

  file(READ "${SOURCE_DIR}/tests/cmake/reproduction.cmake" replay_wrapper)
  file(READ "${SOURCE_DIR}/tests/CMakeLists.txt" test_registration)
  reproduction_require_contains("${replay_wrapper}" "reproduction_preflight(" "example preflight implementation is missing")
  reproduction_require_contains("${replay_wrapper}" "elseif(MODE STREQUAL \"preflight\")" "public preflight mode is missing")
  reproduction_require_contains("${replay_wrapper}" "TIMEOUT 20" "frame-1 runner child deadline is missing")
  reproduction_require_contains("${replay_wrapper}" "TIMEOUT 10" "fixture and negative-control child deadlines are missing")
  reproduction_require_contains("${replay_wrapper}" "nesturbator-run: stopped" "interrupted child output is not rejected")
  reproduction_require_contains("${test_registration}" "-DMODE=preflight_fixture" "positive preflight CTest mode is stale")
  reproduction_require_contains("${test_registration}" "-DMODE=preflight_self_test" "preflight negative-control CTest mode is stale")

  foreach(target IN LISTS documented_targets)
    string(FIND "${all_tests_json}" "/${target}" target_position)
    if(target_position EQUAL -1)
      message(FATAL_ERROR "docs.reproduction: documented build target is not generated: ${target}")
    endif()
  endforeach()
  foreach(required_target IN ITEMS nesturbator-run runner.game_movie runner.nmi_fixture
      runner.boot_to_play runner.save_rom runner.state_mutate)
    if(NOT required_target IN_LIST documented_targets)
      message(FATAL_ERROR "docs.reproduction: documented build command omits ${required_target}")
    endif()
  endforeach()

  message(STATUS "docs.reproduction: report fields, source links, build targets and bounded CTest selection are current")
endfunction()

function(reproduction_expect_mutation root name mutated expected)
  set(mutated_doc "${root}/${name}.md")
  file(WRITE "${mutated_doc}" "${mutated}")
  execute_process(COMMAND "${CMAKE_COMMAND}"
      "-DMODE=check"
      "-DSOURCE_DIR=${SOURCE_DIR}"
      "-DBUILD_DIR=${BUILD_DIR}"
      "-DCTEST=${CTEST}"
      "-DDOC=${mutated_doc}"
      -P "${CMAKE_CURRENT_LIST_FILE}"
    OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE rc TIMEOUT 20)
  if("${rc}" STREQUAL "0")
    message(FATAL_ERROR "docs.reproduction self-test: ${name} mutation was accepted")
  endif()
  string(CONCAT combined "${out}" "${err}")
  string(FIND "${combined}" "${expected}" diagnostic_pos)
  if(diagnostic_pos EQUAL -1)
    string(REGEX MATCH "docs\.reproduction[^\n]*" observed "${combined}")
    message(FATAL_ERROR "docs.reproduction self-test: ${name} expected '${expected}', observed '${observed}'")
  endif()
  message(STATUS "docs.reproduction.self_test: ${name} rejected for the expected reason")
endfunction()

if(NOT DEFINED MODE OR MODE STREQUAL "")
  message(FATAL_ERROR "docs.reproduction: -DMODE=check|self_test is required")
elseif(MODE STREQUAL "check")
  if(NOT DEFINED DOC OR "${DOC}" STREQUAL "")
    message(FATAL_ERROR "docs.reproduction: -DDOC=... is required")
  endif()
  reproduction_check("${DOC}")
elseif(MODE STREQUAL "self_test")
  foreach(var DOC WORK)
    if(NOT DEFINED ${var} OR "${${var}}" STREQUAL "")
      message(FATAL_ERROR "docs.reproduction self-test: -D${var}=... is required")
    endif()
  endforeach()
  reproduction_check("${DOC}")
  file(READ "${DOC}" original)
  string(RANDOM LENGTH 20 ALPHABET 0123456789abcdef suffix)
  set(root "${WORK}/contract-${suffix}")
  file(MAKE_DIRECTORY "${root}")

  set(field_mutation "${original}")
  string(REPLACE "Title and region:\n" "" field_mutation "${field_mutation}")
  reproduction_expect_mutation("${root}" "missing-report-field" "${field_mutation}" "report field missing: Title and region")

  set(test_mutation "${original}")
  string(REPLACE "runner.reproduction.cold|runner.reproduction.compare_self_test"
    "runner.reproduction.not_cold|runner.reproduction.compare_self_test" test_mutation "${test_mutation}")
  reproduction_expect_mutation("${root}" "stale-test-name" "${test_mutation}" "not generated")

  set(target_mutation "${original}")
  string(REPLACE "runner.game_movie" "runner.game_movie_missing" target_mutation "${target_mutation}")
  reproduction_expect_mutation("${root}" "stale-target-path" "${target_mutation}" "build target is not generated")

  set(safety_mutation "${original}")
  string(REPLACE "Do not publish commercial game media" "Publish commercial game media" safety_mutation "${safety_mutation}")
  reproduction_expect_mutation("${root}" "missing-privacy-claim" "${safety_mutation}" "privacy claim missing")

  set(bounds_mutation "${original}")
  string(REPLACE "`runner.reproduction.cold`: 180 s" "`runner.reproduction.cold`: unbounded" bounds_mutation "${bounds_mutation}")
  reproduction_expect_mutation("${root}" "missing-replay-bound" "${bounds_mutation}"
    "cold replay timeout bound missing")

  set(storage_mutation "${original}")
  string(REPLACE "outside every active example path" "inside every active example path" storage_mutation "${storage_mutation}")
  reproduction_expect_mutation("${root}" "daily-storage-in-example" "${storage_mutation}"
    "newest battery backup must stay outside active paths")

  set(container_mutation "${original}")
  string(REPLACE "runner-native state cannot be renamed into a RetroArch disk state"
    "runner-native state is a RetroArch disk state" container_mutation "${container_mutation}")
  reproduction_expect_mutation("${root}" "host-native-container-conflation" "${container_mutation}"
    "host/native container boundary is missing")

  set(loader_mutation "${original}")
  string(REPLACE "does not enforce the exact executing build" "enforces the exact executing build" loader_mutation "${loader_mutation}")
  reproduction_expect_mutation("${root}" "false-loader-build-enforcement" "${loader_mutation}"
    "loader build-enforcement limit missing")

  set(next_evidence_mutation "${original}")
  string(REPLACE
    "Use `runner.reproduction.cold` for two cold 600-frame executions or `runner.state.nmi_roundtrip` for its neutral K=1 through N=4 suffix."
    ""
    next_evidence_mutation "${next_evidence_mutation}")
  reproduction_expect_mutation("${root}" "missing-next-evidence-branch" "${next_evidence_mutation}"
    "native next-evidence missing")

  set(hash_hardware_mutation "${original}")
  string(REPLACE "does not show that the output is correct."
    "shows that the output is hardware-correct." hash_hardware_mutation "${hash_hardware_mutation}")
  reproduction_expect_mutation("${root}" "hash-as-hardware-oracle" "${hash_hardware_mutation}"
    "hash correctness conflation")

  set(screenshot_hash_mutation "${original}")
  string(REPLACE "Unaligned screenshots are qualitative and cannot be equated to native hashes."
    "Screenshots are native hashes." screenshot_hash_mutation "${screenshot_hash_mutation}")
  reproduction_expect_mutation("${root}" "screenshot-as-native-hash" "${screenshot_hash_mutation}"
    "screenshot/native hash conflation")

  set(adapter_pacing_mutation "${original}")
  string(REPLACE "pacing, resampling, synchronization and physical devices remain frontend/host questions."
    "pacing is established by adapter parity, and resampling and physical devices remain frontend/host questions."
    adapter_pacing_mutation "${adapter_pacing_mutation}")
  reproduction_expect_mutation("${root}" "adapter-clears-host-pacing" "${adapter_pacing_mutation}"
    "host timing attribution")

  set(commercial_fixed_mutation "${original}")
  string(REPLACE "These reports are not\ndiagnoses or fixes."
    "These reports are fixed." commercial_fixed_mutation "${commercial_fixed_mutation}")
  reproduction_expect_mutation("${root}" "unclassified-commercial-labeled-fixed" "${commercial_fixed_mutation}"
    "commercial status relabeled")

  message(STATUS "docs.reproduction.self_test: thirteen independent safety and evidence mutations rejected")
else()
  message(FATAL_ERROR "docs.reproduction: unknown mode '${MODE}'")
endif()
