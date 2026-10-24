cmake_minimum_required(VERSION 3.25)

foreach(required_env IN ITEMS RUNNER_TEMP GITHUB_ENV)
  if(NOT DEFINED ENV{${required_env}} OR "$ENV{${required_env}}" STREQUAL "")
    message(FATAL_ERROR "${required_env} must be set to prepare the protected-main scoreboard baseline")
  endif()
endforeach()

set(runner_temp "$ENV{RUNNER_TEMP}")
set(github_env "$ENV{GITHUB_ENV}")
if(runner_temp MATCHES "[\r\n]" OR github_env MATCHES "[\r\n]")
  message(FATAL_ERROR "RUNNER_TEMP and GITHUB_ENV must not contain newlines")
endif()
if(NOT IS_DIRECTORY "${runner_temp}")
  message(FATAL_ERROR "RUNNER_TEMP is not an existing directory: ${runner_temp}")
endif()
if(NOT EXISTS "${github_env}" OR IS_DIRECTORY "${github_env}")
  message(FATAL_ERROR "GITHUB_ENV is not an existing file: ${github_env}")
endif()

if(DEFINED SCOREBOARD_REMOTE AND NOT SCOREBOARD_REMOTE STREQUAL "")
  set(scoreboard_remote "${SCOREBOARD_REMOTE}")
else()
  set(scoreboard_remote origin)
endif()

if(DEFINED SCOREBOARD_GIT_DIRECTORY AND NOT SCOREBOARD_GIT_DIRECTORY STREQUAL "")
  set(scoreboard_git_directory "${SCOREBOARD_GIT_DIRECTORY}")
else()
  set(scoreboard_git_directory "${CMAKE_CURRENT_SOURCE_DIR}")
endif()
execute_process(
  COMMAND git rev-parse --show-toplevel
  WORKING_DIRECTORY "${scoreboard_git_directory}"
  RESULT_VARIABLE git_directory_result
  OUTPUT_VARIABLE git_directory_root
  ERROR_VARIABLE git_directory_error
  OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT "${git_directory_result}" STREQUAL "0" OR git_directory_root STREQUAL "")
  message(FATAL_ERROR "Protected-main fetch directory is not a git checkout: ${git_directory_error}")
endif()

execute_process(
  COMMAND git fetch --no-tags "${scoreboard_remote}" refs/heads/main
  WORKING_DIRECTORY "${scoreboard_git_directory}"
  RESULT_VARIABLE fetch_result
  OUTPUT_VARIABLE fetch_stdout
  ERROR_VARIABLE fetch_stderr)
if(NOT "${fetch_result}" STREQUAL "0")
  message(FATAL_ERROR "Could not fetch protected refs/heads/main (${fetch_result}): ${fetch_stderr}")
endif()

# Resolve FETCH_HEAD once. Every authority file below is read by this immutable
# object ID, so a branch update cannot mix a scoreboard and identity.
execute_process(
  COMMAND git rev-parse --verify "FETCH_HEAD^{commit}"
  WORKING_DIRECTORY "${scoreboard_git_directory}"
  RESULT_VARIABLE revision_result
  OUTPUT_VARIABLE protected_revision
  ERROR_VARIABLE revision_stderr
  OUTPUT_STRIP_TRAILING_WHITESPACE)
string(LENGTH "${protected_revision}" protected_revision_length)
if(NOT "${revision_result}" STREQUAL "0" OR
   NOT protected_revision MATCHES "^[0-9a-f]+$" OR
   NOT protected_revision_length EQUAL 40)
  message(FATAL_ERROR "Could not resolve protected main to one immutable commit: ${revision_stderr}")
endif()

set(baseline_file "${runner_temp}/nesturbator-scoreboard-main.txt")
set(baseline_metadata "${runner_temp}/nesturbator-scoreboard-main-metadata.txt")
set(canonical_metadata_path tests/accuracy/baseline-metadata.txt)
set(dedicated_metadata_path tests/accuracy/scoreboard-main-metadata.txt)
set(scoreboard_path tests/accuracy/scoreboard.txt)
set(manifest_path tests/roms/manifest.txt)

function(is_lower_hex value digits output)
  string(LENGTH "${value}" value_length)
  if(value MATCHES "^[0-9a-f]+$" AND value_length EQUAL digits)
    set(${output} TRUE PARENT_SCOPE)
  else()
    set(${output} FALSE PARENT_SCOPE)
  endif()
endfunction()

function(read_metadata_file path prefix)
  file(STRINGS "${path}" metadata_lines)
  set(metadata_keys)
  foreach(line IN LISTS metadata_lines)
    if(NOT line MATCHES "^([a-z][a-z0-9_]*)=(.+)$")
      message(FATAL_ERROR "Protected-main canonical metadata has an invalid row: ${line}")
    endif()
    set(key "${CMAKE_MATCH_1}")
    set(value "${CMAKE_MATCH_2}")
    if(key IN_LIST metadata_keys OR value MATCHES "=")
      message(FATAL_ERROR "Protected-main canonical metadata has a duplicate key or invalid value: ${key}")
    endif()
    list(APPEND metadata_keys "${key}")
    set("${prefix}_${key}" "${value}" PARENT_SCOPE)
  endforeach()
endfunction()

function(read_protected_file relative_path output_path)
  execute_process(
    COMMAND git ls-tree "${protected_revision}" -- "${relative_path}"
    WORKING_DIRECTORY "${scoreboard_git_directory}"
    RESULT_VARIABLE tree_result
    OUTPUT_VARIABLE tree_entry
    ERROR_VARIABLE tree_stderr)
  if(NOT "${tree_result}" STREQUAL "0")
    message(FATAL_ERROR "Could not inspect protected-main ${relative_path} (${tree_result}): ${tree_stderr}")
  endif()
  if(tree_entry STREQUAL "")
    message(FATAL_ERROR "Protected-main authority is missing ${relative_path}")
  endif()
  if(NOT tree_entry MATCHES "^100[0-7][0-7][0-7] blob [0-9a-f]+\t")
    message(FATAL_ERROR "Protected-main authority path is not a regular blob: ${relative_path}")
  endif()
  execute_process(
    COMMAND git show "${protected_revision}:${relative_path}"
    WORKING_DIRECTORY "${scoreboard_git_directory}"
    RESULT_VARIABLE show_result
    OUTPUT_FILE "${output_path}"
    ERROR_VARIABLE show_stderr)
  if(NOT "${show_result}" STREQUAL "0" OR
     NOT EXISTS "${output_path}" OR IS_DIRECTORY "${output_path}")
    message(FATAL_ERROR "Could not retrieve protected-main ${relative_path} (${show_result}): ${show_stderr}")
  endif()
endfunction()

set(source_scoreboard_file "${runner_temp}/nesturbator-scoreboard-main-source.txt")
set(canonical_metadata_raw "${runner_temp}/nesturbator-canonical-baseline-metadata.txt")
read_protected_file("${scoreboard_path}" "${source_scoreboard_file}")
file(SIZE "${source_scoreboard_file}" scoreboard_size)
if(scoreboard_size EQUAL 0)
  message(FATAL_ERROR "Protected-main scoreboard authority is empty")
endif()
execute_process(
  COMMAND git ls-tree "${protected_revision}" -- "${canonical_metadata_path}"
  WORKING_DIRECTORY "${scoreboard_git_directory}"
  RESULT_VARIABLE canonical_tree_result
  OUTPUT_VARIABLE canonical_tree_entry
  ERROR_VARIABLE canonical_tree_stderr)
if(NOT "${canonical_tree_result}" STREQUAL "0")
  message(FATAL_ERROR "Could not inspect protected-main canonical metadata (${canonical_tree_result}): ${canonical_tree_stderr}")
endif()
set(use_canonical_metadata FALSE)
if(NOT canonical_tree_entry STREQUAL "")
  if(NOT canonical_tree_entry MATCHES "^100[0-7][0-7][0-7] blob [0-9a-f]+\t")
    message(FATAL_ERROR "Protected-main canonical metadata path is not a regular blob")
  endif()
  read_protected_file("${canonical_metadata_path}" "${canonical_metadata_raw}")
  read_metadata_file("${canonical_metadata_raw}" canonical)
  if(DEFINED canonical_mode AND canonical_mode STREQUAL "campaign" AND
     DEFINED canonical_scoreboard_schema AND canonical_scoreboard_schema STREQUAL "2")
    set(use_canonical_metadata TRUE)
  endif()
endif()

if(use_canonical_metadata)
  foreach(key IN ITEMS schema license rom_pin rom_sha256 inventory_schema inventory_sha256
      inventory_rows inventory_scored_rows inventory_draw_rows scoreboard_schema scoreboard_rows
      scoreboard_sha256 pass fail skip not_run timeout draw scored_total trustworthy_observation
      infrastructure_verdict old_rom_pin old_rom_sha256)
    if(NOT DEFINED canonical_${key})
      message(FATAL_ERROR "Canonical protected-main metadata is missing ${key}")
    endif()
  endforeach()
  is_lower_hex("${canonical_rom_pin}" 40 canonical_valid_rom_pin)
  is_lower_hex("${canonical_rom_sha256}" 64 canonical_valid_rom_sha)
  is_lower_hex("${canonical_inventory_sha256}" 64 canonical_valid_inventory_sha)
  is_lower_hex("${canonical_scoreboard_sha256}" 64 canonical_valid_scoreboard_sha)
  is_lower_hex("${canonical_old_rom_pin}" 40 canonical_valid_old_rom_pin)
  is_lower_hex("${canonical_old_rom_sha256}" 64 canonical_valid_old_rom_sha)
  if(NOT canonical_schema STREQUAL "1" OR NOT canonical_license STREQUAL "MIT" OR
     NOT canonical_valid_rom_pin OR NOT canonical_valid_rom_sha OR
     NOT canonical_valid_inventory_sha OR NOT canonical_valid_scoreboard_sha OR
     NOT canonical_valid_old_rom_pin OR NOT canonical_valid_old_rom_sha OR
     NOT canonical_inventory_schema STREQUAL "1" OR NOT canonical_scoreboard_schema STREQUAL "2" OR
     NOT canonical_mode STREQUAL "campaign" OR NOT canonical_trustworthy_observation STREQUAL "1" OR
     NOT canonical_infrastructure_verdict STREQUAL "complete" OR
     NOT canonical_scoreboard_rows MATCHES "^[0-9]+$" OR
     NOT canonical_scoreboard_rows STREQUAL canonical_scored_total OR
     NOT canonical_inventory_scored_rows STREQUAL canonical_scoreboard_rows OR
     NOT canonical_inventory_rows MATCHES "^[0-9]+$" OR
     NOT canonical_inventory_scored_rows MATCHES "^[0-9]+$" OR
     NOT canonical_inventory_draw_rows MATCHES "^[0-9]+$" OR
     NOT canonical_draw MATCHES "^[0-9]+$")
    message(FATAL_ERROR "Canonical protected-main metadata has inconsistent measured identity or counts")
  endif()
  foreach(key IN ITEMS pass fail skip not_run timeout)
    if(NOT canonical_${key} MATCHES "^[0-9]+$")
      message(FATAL_ERROR "Canonical protected-main metadata has invalid ${key} count")
    endif()
  endforeach()
  math(EXPR canonical_status_total
    "${canonical_pass} + ${canonical_fail} + ${canonical_skip} + ${canonical_not_run} + ${canonical_timeout}")
  math(EXPR canonical_expected_inventory_rows
    "${canonical_inventory_scored_rows} + ${canonical_inventory_draw_rows}")
  if(NOT canonical_inventory_rows EQUAL canonical_expected_inventory_rows OR
     NOT canonical_status_total EQUAL canonical_scoreboard_rows OR
     NOT canonical_inventory_draw_rows EQUAL canonical_draw OR
     NOT canonical_pass GREATER 0)
    message(FATAL_ERROR "Canonical protected-main status counts do not match the measured denominator")
  endif()

  set(manifest_file "${runner_temp}/nesturbator-canonical-baseline-manifest.txt")
  read_protected_file("${manifest_path}" "${manifest_file}")
  file(STRINGS "${manifest_file}" manifest_lines)
  set(new_manifest_matches 0)
  set(old_manifest_matches 0)
  foreach(line IN LISTS manifest_lines)
    if(line STREQUAL "" OR line MATCHES "^#")
      continue()
    endif()
    string(REPLACE "\t" ";" fields "${line}")
    list(LENGTH fields field_count)
    if(NOT field_count EQUAL 5)
      message(FATAL_ERROR "Protected-main ROM manifest has a malformed row")
    endif()
    list(GET fields 0 name)
    list(GET fields 1 source_url)
    list(GET fields 2 pin)
    list(GET fields 3 license)
    list(GET fields 4 digest)
    if(name STREQUAL "tests/roms/accuracycoin.nes")
      math(EXPR new_manifest_matches "${new_manifest_matches} + 1")
      if(NOT pin STREQUAL canonical_rom_pin OR NOT digest STREQUAL canonical_rom_sha256 OR
         NOT source_url STREQUAL "https://github.com/100thCoin/AccuracyCoin/tree/${canonical_rom_pin}" OR
         NOT license STREQUAL "MIT")
        message(FATAL_ERROR "Protected-main canonical manifest identity disagrees with its metadata")
      endif()
    elseif(name STREQUAL "tests/roms/accuracycoin-673ef.nes")
      math(EXPR old_manifest_matches "${old_manifest_matches} + 1")
      if(NOT pin STREQUAL canonical_old_rom_pin OR NOT digest STREQUAL canonical_old_rom_sha256 OR
         NOT source_url STREQUAL "https://github.com/100thCoin/AccuracyCoin/tree/${canonical_old_rom_pin}" OR
         NOT license STREQUAL "MIT")
        message(FATAL_ERROR "Protected-main retained old manifest identity disagrees with canonical metadata")
      endif()
    endif()
  endforeach()
  if(NOT new_manifest_matches EQUAL 1 OR NOT old_manifest_matches EQUAL 1)
    message(FATAL_ERROR "Protected-main canonical manifest must authenticate both AccuracyCoin pins exactly once")
  endif()

  file(SHA256 "${source_scoreboard_file}" source_scoreboard_sha256)
  if(NOT source_scoreboard_sha256 STREQUAL canonical_scoreboard_sha256)
    message(FATAL_ERROR "Canonical protected-main scoreboard does not match its metadata hash")
  endif()
  file(STRINGS "${source_scoreboard_file}" scoreboard_lines)
  set(source_scoreboard_rows 0)
  set(canonical_pass_rows 0)
  set(canonical_fail_rows 0)
  set(canonical_skip_rows 0)
  set(canonical_not_run_rows 0)
  set(canonical_timeout_rows 0)
  set(previous_key "")
  set(projected_scoreboard "")
  foreach(line IN LISTS scoreboard_lines)
    string(REPLACE "\t" ";" fields "${line}")
    list(LENGTH fields field_count)
    if(NOT field_count EQUAL 5)
      message(FATAL_ERROR "Canonical protected-main scoreboard row must have five fields: ${line}")
    endif()
    list(GET fields 0 key)
    list(GET fields 1 status)
    list(GET fields 2 raw)
    list(GET fields 3 frames)
    list(GET fields 4 hash)
    if(NOT key MATCHES "^accuracycoin/[^\t]+$" OR
       (NOT status STREQUAL "pass" AND NOT status STREQUAL "fail" AND
        NOT status STREQUAL "skip" AND NOT status STREQUAL "not-run" AND
        NOT status STREQUAL "timeout") OR
       (NOT raw STREQUAL "-" AND NOT raw MATCHES "^0x[0-9a-f][0-9a-f]$") OR
       (NOT frames STREQUAL "-" AND NOT frames MATCHES "^[0-9]+$") OR
       (NOT hash STREQUAL "-" AND NOT hash MATCHES "^[0-9a-f]+$") OR
       (NOT previous_key STREQUAL "" AND NOT previous_key STRLESS key))
      message(FATAL_ERROR "Canonical protected-main scoreboard row is invalid or unsorted: ${key}")
    endif()
    if(NOT hash STREQUAL "-")
      string(LENGTH "${hash}" hash_length)
      if(NOT hash_length EQUAL 64)
        message(FATAL_ERROR "Canonical protected-main scoreboard row has an invalid hash: ${key}")
      endif()
    endif()
    if(status STREQUAL "pass")
      if(raw STREQUAL "-" OR frames STREQUAL "-")
        message(FATAL_ERROR "Canonical protected PASS row lacks measured raw/frame evidence: ${key}")
      endif()
      math(EXPR canonical_pass_rows "${canonical_pass_rows} + 1")
      string(APPEND projected_scoreboard "${key}\tpass\t${raw}\t-\t-\n")
    elseif(status STREQUAL "fail")
      math(EXPR canonical_fail_rows "${canonical_fail_rows} + 1")
    elseif(status STREQUAL "skip")
      math(EXPR canonical_skip_rows "${canonical_skip_rows} + 1")
    elseif(status STREQUAL "not-run")
      math(EXPR canonical_not_run_rows "${canonical_not_run_rows} + 1")
    elseif(status STREQUAL "timeout")
      math(EXPR canonical_timeout_rows "${canonical_timeout_rows} + 1")
    endif()
    set(previous_key "${key}")
    math(EXPR source_scoreboard_rows "${source_scoreboard_rows} + 1")
  endforeach()
  if(NOT source_scoreboard_rows EQUAL canonical_scoreboard_rows OR
     NOT canonical_pass_rows EQUAL canonical_pass OR
     NOT canonical_fail_rows EQUAL canonical_fail OR
     NOT canonical_skip_rows EQUAL canonical_skip OR
     NOT canonical_not_run_rows EQUAL canonical_not_run OR
     NOT canonical_timeout_rows EQUAL canonical_timeout)
    message(FATAL_ERROR "Canonical protected-main scoreboard rows disagree with same-commit metadata")
  endif()
  file(WRITE "${baseline_file}" "${projected_scoreboard}")
  file(SHA256 "${baseline_file}" scoreboard_sha256)
  set(scoreboard_rows "${canonical_pass_rows}")
else()
  configure_file("${source_scoreboard_file}" "${baseline_file}" COPYONLY)
  file(STRINGS "${baseline_file}" scoreboard_lines)
  set(scoreboard_rows 0)
  set(previous_key "")
  foreach(line IN LISTS scoreboard_lines)
    if(NOT line MATCHES "^(accuracycoin/[^\t]+)\t(pass|fail|unavailable|unsupported)\t0x[0-9a-f][0-9a-f]\t-\t-$")
      message(FATAL_ERROR "Protected-main scoreboard has an invalid row: ${line}")
    endif()
    set(key "${CMAKE_MATCH_1}")
    if(NOT previous_key STREQUAL "" AND NOT previous_key STRLESS key)
      message(FATAL_ERROR "Protected-main scoreboard rows are duplicated or unsorted at ${key}")
    endif()
    set(previous_key "${key}")
    math(EXPR scoreboard_rows "${scoreboard_rows} + 1")
  endforeach()
  if(scoreboard_rows EQUAL 0)
    message(FATAL_ERROR "Protected-main scoreboard authority has no rows")
  endif()
  file(SHA256 "${baseline_file}" scoreboard_sha256)
endif()

if(NOT use_canonical_metadata)
  execute_process(
    COMMAND git ls-tree "${protected_revision}" -- "${dedicated_metadata_path}"
    WORKING_DIRECTORY "${scoreboard_git_directory}"
    RESULT_VARIABLE metadata_tree_result
    OUTPUT_VARIABLE metadata_tree_entry
    ERROR_VARIABLE metadata_tree_stderr)
  if(NOT "${metadata_tree_result}" STREQUAL "0")
    message(FATAL_ERROR "Could not inspect protected-main metadata (${metadata_tree_result}): ${metadata_tree_stderr}")
  endif()
endif()
set(has_dedicated_metadata FALSE)
if(NOT use_canonical_metadata AND NOT metadata_tree_entry STREQUAL "")
  set(protected_metadata_raw "${runner_temp}/nesturbator-scoreboard-main-source-metadata.txt")
  if(NOT metadata_tree_entry MATCHES "^100[0-7][0-7][0-7] blob [0-9a-f]+\t")
    message(FATAL_ERROR "Protected-main metadata path is not a regular blob")
  endif()
  execute_process(
    COMMAND git show "${protected_revision}:${dedicated_metadata_path}"
    WORKING_DIRECTORY "${scoreboard_git_directory}"
    RESULT_VARIABLE metadata_show_result
    OUTPUT_FILE "${protected_metadata_raw}"
    ERROR_VARIABLE metadata_show_stderr)
  if(NOT "${metadata_show_result}" STREQUAL "0")
    message(FATAL_ERROR "Could not retrieve protected-main metadata (${metadata_show_result}): ${metadata_show_stderr}")
  endif()
  file(STRINGS "${protected_metadata_raw}" metadata_authority_lines REGEX "^authority=")
  list(LENGTH metadata_authority_lines metadata_authority_count)
  if(NOT metadata_authority_count EQUAL 1)
    message(FATAL_ERROR "Protected-main metadata must declare exactly one authority")
  endif()
  if(metadata_authority_lines STREQUAL "authority=protected-main")
    set(has_dedicated_metadata TRUE)
  elseif(NOT metadata_authority_lines STREQUAL "authority=local-snapshot")
    message(FATAL_ERROR "Protected-main metadata declares an unsupported authority")
  endif()
endif()

if(use_canonical_metadata)
  set(metadata_source "canonical-baseline")
  set(rom_pin "${canonical_rom_pin}")
  set(rom_sha256 "${canonical_rom_sha256}")
  set(inventory_schema "${canonical_inventory_schema}")
  set(inventory_sha256 "${canonical_inventory_sha256}")
  set(inventory_rows "${canonical_inventory_rows}")
  set(inventory_scored_rows "${canonical_inventory_scored_rows}")
  set(inventory_draw_rows "${canonical_inventory_draw_rows}")
  set(scoreboard_schema 1)
  set(scoreboard_denominator "${canonical_pass_rows}")
  set(canonical_source_scoreboard_hash "${source_scoreboard_sha256}")
  set(canonical_source_scoreboard_rows "${source_scoreboard_rows}")
  set(canonical_source_pass_rows "${canonical_pass_rows}")
elseif(has_dedicated_metadata)
  file(STRINGS "${protected_metadata_raw}" metadata_lines)
  set(metadata_keys)
  set(allowed_metadata_keys schema authority source protected_revision rom_pin rom_sha256
    inventory_schema inventory_sha256 inventory_rows inventory_scored_rows inventory_draw_rows
    scoreboard_schema scoreboard_rows scoreboard_denominator scoreboard_sha256)
  foreach(line IN LISTS metadata_lines)
    string(FIND "${line}" "=" separator)
    if(separator LESS 1)
      message(FATAL_ERROR "Protected-main metadata has an invalid row: ${line}")
    endif()
    string(SUBSTRING "${line}" 0 ${separator} key)
    math(EXPR value_start "${separator} + 1")
    string(SUBSTRING "${line}" ${value_start} -1 value)
    if(value STREQUAL "" OR NOT key IN_LIST allowed_metadata_keys OR
       value MATCHES "=")
      message(FATAL_ERROR "Protected-main metadata has an invalid row: ${line}")
    endif()
    if(key IN_LIST metadata_keys)
      message(FATAL_ERROR "Protected-main metadata has duplicate key: ${key}")
    endif()
    list(APPEND metadata_keys "${key}")
    set("protected_${key}" "${value}")
  endforeach()
  foreach(key IN ITEMS schema rom_pin rom_sha256 inventory_schema
                       inventory_sha256 inventory_rows inventory_scored_rows inventory_draw_rows
                       scoreboard_schema scoreboard_rows scoreboard_denominator scoreboard_sha256)
    if(NOT DEFINED protected_${key})
      message(FATAL_ERROR "Protected-main metadata is missing ${key}")
    endif()
  endforeach()
  if(DEFINED protected_protected_revision AND
     NOT protected_protected_revision STREQUAL protected_revision)
    message(FATAL_ERROR "Protected-main metadata declares a different protected revision")
  endif()
  is_lower_hex("${protected_rom_pin}" 40 valid_rom_pin)
  is_lower_hex("${protected_rom_sha256}" 64 valid_rom_sha)
  is_lower_hex("${protected_inventory_sha256}" 64 valid_inventory_sha)
  if(NOT protected_schema STREQUAL "1" OR
     NOT valid_rom_pin OR
     NOT valid_rom_sha OR
     NOT valid_inventory_sha OR
     NOT protected_inventory_schema STREQUAL "1" OR
     NOT protected_scoreboard_schema STREQUAL "1" OR
     NOT protected_scoreboard_rows STREQUAL "${scoreboard_rows}" OR
     NOT protected_scoreboard_denominator STREQUAL "${scoreboard_rows}" OR
     NOT protected_scoreboard_sha256 STREQUAL scoreboard_sha256)
    message(FATAL_ERROR "Protected-main metadata identity does not match the fetched revision or scoreboard")
  endif()
  set(metadata_source "protected-metadata")
  set(rom_pin "${protected_rom_pin}")
  set(rom_sha256 "${protected_rom_sha256}")
  set(inventory_schema "${protected_inventory_schema}")
  set(inventory_sha256 "${protected_inventory_sha256}")
  set(inventory_rows "${protected_inventory_rows}")
  set(inventory_scored_rows "${protected_inventory_scored_rows}")
  set(inventory_draw_rows "${protected_inventory_draw_rows}")
  set(scoreboard_schema "${protected_scoreboard_schema}")
  set(scoreboard_denominator "${protected_scoreboard_denominator}")
else()
  # The pre-Phase-17 protected revision has no dedicated metadata yet. Parse
  # its exact manifest row from this same commit; do not read the candidate tree.
  set(manifest_file "${runner_temp}/nesturbator-scoreboard-main-manifest.txt")
  read_protected_file("${manifest_path}" "${manifest_file}")
  file(STRINGS "${manifest_file}" manifest_lines)
  set(matches 0)
  foreach(line IN LISTS manifest_lines)
    if(line STREQUAL "" OR line MATCHES "^#")
      continue()
    endif()
    string(REPLACE "\t" ";" fields "${line}")
    list(LENGTH fields field_count)
    if(field_count LESS 1)
      continue()
    endif()
    list(GET fields 0 manifest_file_name)
    if(manifest_file_name STREQUAL "tests/roms/accuracycoin.nes")
      math(EXPR matches "${matches} + 1")
      if(NOT field_count EQUAL 5)
        message(FATAL_ERROR "Protected-main AccuracyCoin manifest row must have exactly five fields")
      endif()
      list(GET fields 1 source_url)
      list(GET fields 2 rom_pin)
      list(GET fields 3 license)
      list(GET fields 4 rom_sha256)
      is_lower_hex("${rom_pin}" 40 valid_manifest_pin)
      is_lower_hex("${rom_sha256}" 64 valid_manifest_sha)
      if(NOT source_url STREQUAL "https://github.com/100thCoin/AccuracyCoin/tree/${rom_pin}" OR
         NOT valid_manifest_pin OR
         NOT valid_manifest_sha OR
         NOT license STREQUAL "MIT")
        message(FATAL_ERROR "Protected-main AccuracyCoin manifest row has invalid identity")
      endif()
    endif()
  endforeach()
  if(NOT matches EQUAL 1)
    message(FATAL_ERROR "Protected-main manifest must contain exactly one AccuracyCoin old-pin row")
  endif()
  if(NOT rom_pin STREQUAL "673ef550db296136d52229961e7d39366116882a" OR
     NOT rom_sha256 STREQUAL "4fe8c2bc9abc6f4d418da47b73f62cba89fcacd950fae763097a1681d650e839")
    message(FATAL_ERROR "Legacy protected manifest identity is not the retained old AccuracyCoin pin")
  endif()
  set(metadata_source "legacy-manifest")
  set(inventory_schema 1)
  set(inventory_sha256 "3271803cdc63c4b4a83cf65453b817dc66ec65ed97863ad19261b56f36821c84")
  set(inventory_rows 149)
  set(inventory_scored_rows 144)
  set(inventory_draw_rows 5)
  set(scoreboard_schema 1)
  set(scoreboard_denominator "${scoreboard_rows}")
endif()

file(WRITE "${baseline_metadata}"
  "schema=1\nauthority=protected-main\nsource=${metadata_source}\nprotected_revision=${protected_revision}\nrom_pin=${rom_pin}\nrom_sha256=${rom_sha256}\ninventory_schema=${inventory_schema}\ninventory_sha256=${inventory_sha256}\ninventory_rows=${inventory_rows}\ninventory_scored_rows=${inventory_scored_rows}\ninventory_draw_rows=${inventory_draw_rows}\nscoreboard_schema=${scoreboard_schema}\nscoreboard_rows=${scoreboard_rows}\nscoreboard_denominator=${scoreboard_denominator}\nscoreboard_sha256=${scoreboard_sha256}\n")
if(use_canonical_metadata)
  file(APPEND "${baseline_metadata}"
    "canonical_scoreboard_schema=2\ncanonical_scoreboard_rows=${canonical_source_scoreboard_rows}\ncanonical_pass_rows=${canonical_source_pass_rows}\ncanonical_scoreboard_sha256=${canonical_source_scoreboard_hash}\n")
endif()

file(APPEND "${github_env}"
  "NESTURBATOR_SCOREBOARD_BASELINE=${baseline_file}\nNESTURBATOR_SCOREBOARD_BASELINE_METADATA=${baseline_metadata}\nNESTURBATOR_SCOREBOARD_BASELINE_REVISION=${protected_revision}\n")
