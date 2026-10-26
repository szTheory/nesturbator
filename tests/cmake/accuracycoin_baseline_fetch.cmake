cmake_minimum_required(VERSION 3.25)

foreach(required_var IN ITEMS PREPARE_SCRIPT SOURCE_DIR WORK)
  if(NOT DEFINED ${required_var} OR "${${required_var}}" STREQUAL "")
    message(FATAL_ERROR "protected baseline fetch controls require ${required_var}")
  endif()
endforeach()

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")
string(ASCII 10 metadata_newline)
string(ASCII 9 manifest_tab)

function(run_git working_directory description)
  execute_process(
    COMMAND git ${ARGN}
    WORKING_DIRECTORY "${working_directory}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error)
  if(NOT "${result}" STREQUAL "0")
    message(FATAL_ERROR "Could not prepare ${description} (${result}): ${error}${output}")
  endif()
endfunction()

function(create_fixture name scoreboard_mode metadata_mode has_main)
  set(fixture_root "${WORK}/${name}")
  set(source_repo "${fixture_root}/source")
  set(consumer_repo "${fixture_root}/consumer")
  file(MAKE_DIRECTORY "${source_repo}/tests/accuracy" "${source_repo}/tests/roms"
    "${consumer_repo}")
  run_git("${fixture_root}" "local protected repository"
    init -q --initial-branch=main "${source_repo}")
  run_git("${fixture_root}" "local fetch consumer repository"
    init -q --initial-branch=main "${consumer_repo}")

  if(scoreboard_mode STREQUAL "canonical")
    configure_file("${SOURCE_DIR}/tests/accuracy/scoreboard.txt"
      "${source_repo}/tests/accuracy/scoreboard.txt" COPYONLY)
    if(name STREQUAL "canonical_hash_mismatch")
      set(original_row "accuracycoin/$03   SLO indirect,X${manifest_tab}pass${manifest_tab}0x01${manifest_tab}13${manifest_tab}-")
      set(mutated_row "accuracycoin/$03   SLO indirect,X${manifest_tab}pass${manifest_tab}0x01${manifest_tab}14${manifest_tab}-")
      file(READ "${source_repo}/tests/accuracy/scoreboard.txt" canonical_scoreboard)
      string(FIND "${canonical_scoreboard}" "${original_row}" original_offset)
      if(original_offset EQUAL -1)
        message(FATAL_ERROR "canonical hash-mismatch control could not find its measured pass row")
      endif()
      string(REPLACE "${original_row}" "${mutated_row}" canonical_scoreboard "${canonical_scoreboard}")
      file(WRITE "${source_repo}/tests/accuracy/scoreboard.txt" "${canonical_scoreboard}")
    endif()
  elseif(scoreboard_mode STREQUAL "present")
    configure_file("${SOURCE_DIR}/tests/accuracy/scoreboard-main.txt"
      "${source_repo}/tests/accuracy/scoreboard.txt" COPYONLY)
  elseif(scoreboard_mode STREQUAL "empty")
    file(WRITE "${source_repo}/tests/accuracy/scoreboard.txt" "")
  elseif(NOT scoreboard_mode STREQUAL "missing")
    message(FATAL_ERROR "unknown scoreboard fixture mode: ${scoreboard_mode}")
  endif()
  configure_file("${SOURCE_DIR}/tests/roms/manifest.txt"
    "${source_repo}/tests/roms/manifest.txt" COPYONLY)
  # Model protected main before pin activation: its canonical filename still
  # identified the old bytes. The legacy fixture must not copy the candidate
  # identity forward merely because the pathname has since been reused.
  if(NOT scoreboard_mode STREQUAL "canonical")
    file(STRINGS "${source_repo}/tests/roms/manifest.txt" manifest_lines)
    set(legacy_manifest "")
    set(legacy_manifest_rows 0)
    foreach(line IN LISTS manifest_lines)
      string(FIND "${line}" "tests/roms/accuracycoin.nes${manifest_tab}" canonical_offset)
      if(canonical_offset EQUAL 0)
        math(EXPR legacy_manifest_rows "${legacy_manifest_rows} + 1")
        set(line "tests/roms/accuracycoin.nes\thttps://github.com/100thCoin/AccuracyCoin/tree/673ef550db296136d52229961e7d39366116882a\t673ef550db296136d52229961e7d39366116882a\tMIT\t4fe8c2bc9abc6f4d418da47b73f62cba89fcacd950fae763097a1681d650e839")
      endif()
      string(APPEND legacy_manifest "${line}${metadata_newline}")
    endforeach()
    if(NOT legacy_manifest_rows EQUAL 1)
      message(FATAL_ERROR "legacy protected fixture did not find exactly one canonical AccuracyCoin row")
    endif()
    file(WRITE "${source_repo}/tests/roms/manifest.txt" "${legacy_manifest}")
  endif()

  if(metadata_mode STREQUAL "canonical")
    configure_file("${SOURCE_DIR}/tests/accuracy/baseline-metadata.txt"
      "${source_repo}/tests/accuracy/baseline-metadata.txt" COPYONLY)
  elseif(metadata_mode STREQUAL "local_snapshot")
    configure_file("${SOURCE_DIR}/tests/accuracy/scoreboard-main-metadata.txt"
      "${source_repo}/tests/accuracy/scoreboard-main-metadata.txt" COPYONLY)
  elseif(metadata_mode STREQUAL "protected")
    file(READ "${SOURCE_DIR}/tests/accuracy/scoreboard-main-metadata.txt" metadata_text)
    string(REPLACE "authority=local-snapshot" "authority=protected-main"
      protected_metadata "${metadata_text}")
    set(old_revision_row "protected_revision=9f9386c661f6fb8f6063bf756655713aedadae77${metadata_newline}")
    string(REPLACE "${old_revision_row}" "" protected_metadata "${protected_metadata}")
    file(WRITE "${source_repo}/tests/accuracy/scoreboard-main-metadata.txt"
      "${protected_metadata}")
  elseif(metadata_mode STREQUAL "invalid")
    file(WRITE "${source_repo}/tests/accuracy/scoreboard-main-metadata.txt"
      "authority=protected-main${metadata_newline}")
  elseif(NOT metadata_mode STREQUAL "none")
    message(FATAL_ERROR "unknown metadata fixture mode: ${metadata_mode}")
  endif()

  if(has_main)
    run_git("${source_repo}" "fixture author name" config user.name "Fixture")
    run_git("${source_repo}" "fixture author identity" config user.email "fixture")
    run_git("${source_repo}" "fixture manifest" add tests/roms/manifest.txt)
    if(EXISTS "${source_repo}/tests/accuracy/scoreboard.txt")
      run_git("${source_repo}" "fixture scoreboard" add tests/accuracy/scoreboard.txt)
    endif()
    if(EXISTS "${source_repo}/tests/accuracy/scoreboard-main-metadata.txt")
      run_git("${source_repo}" "fixture metadata" add tests/accuracy/scoreboard-main-metadata.txt)
    endif()
    if(EXISTS "${source_repo}/tests/accuracy/baseline-metadata.txt")
      run_git("${source_repo}" "canonical fixture metadata" add tests/accuracy/baseline-metadata.txt)
    endif()
    run_git("${source_repo}" "protected fixture commit" commit -q -m "protected fixture")
  endif()

  set(fixture_source_repo "${source_repo}" PARENT_SCOPE)
  set(fixture_consumer_repo "${consumer_repo}" PARENT_SCOPE)
endfunction()

function(run_fetch expected_reason)
  set(runner_temp "${fixture_root}/runner-temp")
  set(github_env "${fixture_root}/github-env.txt")
  file(MAKE_DIRECTORY "${runner_temp}")
  file(WRITE "${github_env}" "")
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env
      "RUNNER_TEMP=${runner_temp}" "GITHUB_ENV=${github_env}"
      "${CMAKE_COMMAND}"
        "-DSCOREBOARD_REMOTE=${fixture_source_repo}"
        "-DSCOREBOARD_GIT_DIRECTORY=${fixture_consumer_repo}"
        -P "${PREPARE_SCRIPT}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error)
  set(fetch_output "${error}${output}")
  if(expected_reason STREQUAL "PASS")
    if(NOT "${result}" STREQUAL "0")
      message(FATAL_ERROR "local protected-main fetch control failed (${result}): ${fetch_output}")
    endif()
    execute_process(
      COMMAND git rev-parse --verify "HEAD^{commit}"
      WORKING_DIRECTORY "${fixture_source_repo}"
      RESULT_VARIABLE revision_result
      OUTPUT_VARIABLE expected_revision
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(NOT "${revision_result}" STREQUAL "0")
      message(FATAL_ERROR "could not resolve the local protected fixture commit")
    endif()
    file(READ "${github_env}" exported)
    string(FIND "${exported}" "NESTURBATOR_SCOREBOARD_BASELINE_REVISION=${expected_revision}${metadata_newline}" revision_offset)
    if(revision_offset EQUAL -1)
      message(FATAL_ERROR "fetcher did not export the exact resolved protected revision: ${exported}")
    endif()
    string(FIND "${exported}" "NESTURBATOR_SCOREBOARD_BASELINE=" scoreboard_offset)
    if(scoreboard_offset EQUAL -1)
      message(FATAL_ERROR "fetcher did not export the scoreboard path: ${exported}")
    endif()
    string(FIND "${exported}" "NESTURBATOR_SCOREBOARD_BASELINE_METADATA=" metadata_offset)
    if(metadata_offset EQUAL -1)
      message(FATAL_ERROR "fetcher did not export the metadata path: ${exported}")
    endif()
    file(STRINGS "${github_env}" env_lines)
    set(exported_scoreboard "")
    set(exported_metadata "")
    foreach(line IN LISTS env_lines)
      if(line MATCHES "^NESTURBATOR_SCOREBOARD_BASELINE=(.+)$")
        set(exported_scoreboard "${CMAKE_MATCH_1}")
      elseif(line MATCHES "^NESTURBATOR_SCOREBOARD_BASELINE_METADATA=(.+)$")
        set(exported_metadata "${CMAKE_MATCH_1}")
      endif()
    endforeach()
    if(exported_scoreboard STREQUAL "" OR exported_metadata STREQUAL "")
      message(FATAL_ERROR "fetcher did not export both authority paths")
    endif()
    file(SHA256 "${fixture_source_repo}/tests/accuracy/scoreboard.txt" source_sha)
    file(SHA256 "${exported_scoreboard}" fetched_sha)
    if(DEFINED EXPECTED_SOURCE AND EXPECTED_SOURCE STREQUAL "canonical-baseline")
      file(STRINGS "${fixture_source_repo}/tests/accuracy/scoreboard.txt" source_rows)
      set(expected_projection "")
      foreach(line IN LISTS source_rows)
        string(REPLACE "\t" ";" fields "${line}")
        list(GET fields 0 key)
        list(GET fields 1 status)
        if(status STREQUAL "pass")
          list(GET fields 2 raw)
          string(APPEND expected_projection "${key}\tpass\t${raw}\t-\t-\n")
        endif()
      endforeach()
      file(WRITE "${fixture_root}/expected-pass-scoreboard.txt" "${expected_projection}")
      file(SHA256 "${fixture_root}/expected-pass-scoreboard.txt" expected_projection_sha)
      if(NOT expected_projection_sha STREQUAL fetched_sha)
        message(FATAL_ERROR "fetcher did not project exactly the canonical source's PASS keys")
      endif()
      file(STRINGS "${exported_scoreboard}" projected_rows)
      list(LENGTH projected_rows projected_count)
      if(NOT DEFINED EXPECTED_BASELINE_ROWS OR NOT projected_count EQUAL EXPECTED_BASELINE_ROWS)
        message(FATAL_ERROR "canonical protected baseline exported ${projected_count} PASS rows, expected ${EXPECTED_BASELINE_ROWS}")
      endif()
    elseif(NOT source_sha STREQUAL fetched_sha)
      message(FATAL_ERROR "fetched scoreboard bytes differ from the same protected commit")
    endif()
    file(READ "${exported_metadata}" metadata_contents)
    string(FIND "${metadata_contents}" "protected_revision=${expected_revision}${metadata_newline}" embedded_revision_offset)
    if(embedded_revision_offset EQUAL -1)
      message(FATAL_ERROR "fetched metadata does not pin the same protected commit")
    endif()
    if(DEFINED EXPECTED_SOURCE)
      string(FIND "${metadata_contents}" "source=${EXPECTED_SOURCE}${metadata_newline}" source_offset)
      if(source_offset EQUAL -1)
        message(FATAL_ERROR "fetched identity came from the wrong source (${EXPECTED_SOURCE}): ${metadata_contents}")
      endif()
    endif()
    if(DEFINED EXPECTED_PIN)
      string(FIND "${metadata_contents}" "rom_pin=${EXPECTED_PIN}${metadata_newline}" pin_offset)
      if(pin_offset EQUAL -1)
        message(FATAL_ERROR "fetcher selected the wrong canonical pin; expected ${EXPECTED_PIN}: ${metadata_contents}")
      endif()
    endif()
    if(DEFINED EXPECTED_SOURCE AND EXPECTED_SOURCE STREQUAL "canonical-baseline")
      file(SHA256 "${fixture_source_repo}/tests/accuracy/scoreboard.txt" canonical_source_sha)
      foreach(expected_row IN ITEMS
          "canonical_scoreboard_schema=2"
          "canonical_scoreboard_rows=146"
          "canonical_pass_rows=110"
          "canonical_scoreboard_sha256=${canonical_source_sha}"
          "inventory_rows=151"
          "inventory_scored_rows=146"
          "inventory_draw_rows=5"
          "scoreboard_schema=1"
          "scoreboard_rows=110"
          "scoreboard_denominator=110")
        string(FIND "${metadata_contents}" "${expected_row}${metadata_newline}" expected_offset)
        if(expected_offset EQUAL -1)
          message(FATAL_ERROR "canonical fetch metadata omits expected row '${expected_row}': ${metadata_contents}")
        endif()
      endforeach()
    endif()
  else()
    if("${result}" STREQUAL "0")
      message(FATAL_ERROR "invalid protected-main input was accepted; expected '${expected_reason}'")
    endif()
    string(FIND "${fetch_output}" "${expected_reason}" expected_offset)
    if(expected_offset EQUAL -1)
      message(FATAL_ERROR "protected-main fetch failed for the wrong reason (${result}); expected '${expected_reason}': ${fetch_output}")
    endif()
  endif()
endfunction()

create_fixture(canonical_current canonical canonical TRUE)
set(fixture_root "${WORK}/canonical_current")
set(EXPECTED_SOURCE canonical-baseline)
set(EXPECTED_PIN 6bee919529289c555803b612b8c9e4f7eaaeeabb)
set(EXPECTED_BASELINE_ROWS 110)
run_fetch("PASS")
unset(EXPECTED_PIN)
unset(EXPECTED_BASELINE_ROWS)

create_fixture(canonical_hash_mismatch canonical canonical TRUE)
set(fixture_root "${WORK}/canonical_hash_mismatch")
run_fetch("Canonical protected-main scoreboard does not match its metadata hash")

create_fixture(valid_legacy present none TRUE)
set(fixture_root "${WORK}/valid_legacy")
set(EXPECTED_SOURCE legacy-manifest)
run_fetch("PASS")

create_fixture(local_snapshot_metadata present local_snapshot TRUE)
set(fixture_root "${WORK}/local_snapshot_metadata")
set(EXPECTED_SOURCE legacy-manifest)
run_fetch("PASS")

create_fixture(valid_protected_metadata present protected TRUE)
set(fixture_root "${WORK}/valid_protected_metadata")
set(EXPECTED_SOURCE protected-metadata)
run_fetch("PASS")

create_fixture(missing_main present none FALSE)
set(fixture_root "${WORK}/missing_main")
run_fetch("Could not fetch protected refs/heads/main")

create_fixture(missing_scoreboard missing none TRUE)
set(fixture_root "${WORK}/missing_scoreboard")
run_fetch("Protected-main authority is missing tests/accuracy/scoreboard.txt")

create_fixture(empty_scoreboard empty none TRUE)
set(fixture_root "${WORK}/empty_scoreboard")
run_fetch("Protected-main scoreboard authority is empty")

create_fixture(invalid_metadata present invalid TRUE)
set(fixture_root "${WORK}/invalid_metadata")
run_fetch("Protected-main metadata is missing schema")
