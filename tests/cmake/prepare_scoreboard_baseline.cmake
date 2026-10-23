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

execute_process(
  COMMAND git fetch --no-tags "${scoreboard_remote}" refs/heads/main
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
  RESULT_VARIABLE revision_result
  OUTPUT_VARIABLE protected_revision
  ERROR_VARIABLE revision_stderr
  OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT "${revision_result}" STREQUAL "0" OR
   NOT protected_revision MATCHES "^[0-9a-f]+$" OR
   NOT protected_revision MATCHES "^.{40}$")
  message(FATAL_ERROR "Could not resolve protected main to one immutable commit: ${revision_stderr}")
endif()

set(baseline_file "${runner_temp}/nesturbator-scoreboard-main.txt")
set(baseline_metadata "${runner_temp}/nesturbator-scoreboard-main-metadata.txt")
set(dedicated_metadata_path tests/accuracy/scoreboard-main-metadata.txt)
set(scoreboard_path tests/accuracy/scoreboard.txt)
set(manifest_path tests/roms/manifest.txt)

function(read_protected_file relative_path output_path)
  execute_process(
    COMMAND git ls-tree "${protected_revision}" -- "${relative_path}"
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
    RESULT_VARIABLE show_result
    OUTPUT_FILE "${output_path}"
    ERROR_VARIABLE show_stderr)
  if(NOT "${show_result}" STREQUAL "0" OR
     NOT EXISTS "${output_path}" OR IS_DIRECTORY "${output_path}")
    message(FATAL_ERROR "Could not retrieve protected-main ${relative_path} (${show_result}): ${show_stderr}")
  endif()
endfunction()

read_protected_file("${scoreboard_path}" "${baseline_file}")
file(SIZE "${baseline_file}" scoreboard_size)
if(scoreboard_size EQUAL 0)
  message(FATAL_ERROR "Protected-main scoreboard authority is empty")
endif()
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
execute_process(
  COMMAND git ls-tree "${protected_revision}" -- "${dedicated_metadata_path}"
  RESULT_VARIABLE metadata_tree_result
  OUTPUT_VARIABLE metadata_tree_entry
  ERROR_VARIABLE metadata_tree_stderr)
if(NOT "${metadata_tree_result}" STREQUAL "0")
  message(FATAL_ERROR "Could not inspect protected-main metadata path (${metadata_tree_result}): ${metadata_tree_stderr}")
endif()

set(has_dedicated_metadata FALSE)
if(NOT metadata_tree_entry STREQUAL "")
  set(protected_metadata_raw "${runner_temp}/nesturbator-scoreboard-main-source-metadata.txt")
  if(NOT metadata_tree_entry MATCHES "^100[0-7][0-7][0-7] blob [0-9a-f]+\t")
    message(FATAL_ERROR "Protected-main metadata path is not a regular blob")
  endif()
  execute_process(
    COMMAND git show "${protected_revision}:${dedicated_metadata_path}"
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

if(has_dedicated_metadata)
  file(STRINGS "${protected_metadata_raw}" metadata_lines)
  set(metadata_keys)
  foreach(line IN LISTS metadata_lines)
    if(NOT line MATCHES "^([a-z_]+)=([^\r\n]+)$")
      message(FATAL_ERROR "Protected-main metadata has an invalid row: ${line}")
    endif()
    set(key "${CMAKE_MATCH_1}")
    set(value "${CMAKE_MATCH_2}")
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
  if(NOT protected_schema STREQUAL "1" OR
     NOT protected_rom_pin MATCHES "^[0-9a-f]{40}$" OR
     NOT protected_rom_sha256 MATCHES "^[0-9a-f]{64}$" OR
     NOT protected_inventory_sha256 MATCHES "^[0-9a-f]{64}$" OR
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
      if(NOT source_url STREQUAL "https://github.com/100thCoin/AccuracyCoin/tree/${rom_pin}" OR
         NOT rom_pin MATCHES "^[0-9a-f]{40}$" OR
         NOT rom_sha256 MATCHES "^[0-9a-f]{64}$" OR
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

file(APPEND "${github_env}"
  "NESTURBATOR_SCOREBOARD_BASELINE=${baseline_file}\nNESTURBATOR_SCOREBOARD_BASELINE_METADATA=${baseline_metadata}\nNESTURBATOR_SCOREBOARD_BASELINE_REVISION=${protected_revision}\n")
