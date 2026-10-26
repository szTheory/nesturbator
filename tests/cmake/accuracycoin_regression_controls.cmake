cmake_minimum_required(VERSION 3.25)

foreach(required_var IN ITEMS CHECKER RUNNER SOURCE_DIR EVIDENCE_ROOT WORK)
  if(NOT DEFINED ${required_var} OR "${${required_var}}" STREQUAL "")
    message(FATAL_ERROR "AccuracyCoin regression controls require ${required_var}")
  endif()
endforeach()

string(ASCII 9 tab)
string(ASCII 10 newline)
set(baseline "${SOURCE_DIR}/tests/accuracy/scoreboard-main.txt")
set(metadata "${SOURCE_DIR}/tests/accuracy/scoreboard-main-metadata.txt")
set(revision "9f9386c661f6fb8f6063bf756655713aedadae77")
set(source_scoreboard "${SOURCE_DIR}/tests/accuracy/scoreboard.txt")
set(old_rom "${SOURCE_DIR}/tests/roms/accuracycoin-673ef.nes")
set(new_rom "${SOURCE_DIR}/tests/roms/accuracycoin.nes")
set(source_manifest "${SOURCE_DIR}/tests/roms/manifest.txt")
set(source_migration "${SOURCE_DIR}/tests/accuracy/migration.tsv")
set(old_pin "673ef550db296136d52229961e7d39366116882a")
set(new_pin "6bee919529289c555803b612b8c9e4f7eaaeeabb")

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")

function(set_old_candidate_manifest root)
  file(STRINGS "${source_manifest}" source_lines)
  set(updated_manifest "")
  set(old_count 0)
  set(new_count 0)
  foreach(line IN LISTS source_lines)
    if(line MATCHES "^#")
      string(APPEND updated_manifest "${line}${newline}")
      continue()
    endif()
    string(FIND "${line}" "tests/roms/accuracycoin.nes${tab}" new_offset)
    if(new_offset EQUAL 0)
      math(EXPR new_count "${new_count} + 1")
      set(line "tests/roms/accuracycoin.nes\thttps://github.com/100thCoin/AccuracyCoin/tree/${old_pin}\t${old_pin}\tMIT\t4fe8c2bc9abc6f4d418da47b73f62cba89fcacd950fae763097a1681d650e839")
    endif()
    string(FIND "${line}" "tests/roms/accuracycoin-673ef.nes${tab}" old_offset)
    if(old_offset EQUAL 0)
      math(EXPR old_count "${old_count} + 1")
      continue()
    endif()
    string(APPEND updated_manifest "${line}${newline}")
  endforeach()
  if(NOT old_count EQUAL 1 OR NOT new_count EQUAL 1)
    message(FATAL_ERROR "old candidate manifest control did not find exactly one new canonical and old comparison row")
  endif()
  file(WRITE "${root}/manifest.txt" "${updated_manifest}")
endfunction()

macro(prepare_case name)
  set(case_root "${WORK}/${name}")
  file(MAKE_DIRECTORY "${case_root}")
  configure_file("${baseline}" "${case_root}/baseline.txt" COPYONLY)
  configure_file("${metadata}" "${case_root}/metadata.txt" COPYONLY)
  configure_file("${source_scoreboard}" "${case_root}/scoreboard.txt" COPYONLY)
  set_old_candidate_manifest("${case_root}")
  configure_file("${source_migration}" "${case_root}/migration.tsv" COPYONLY)
  file(MAKE_DIRECTORY "${case_root}/evidence")
  file(COPY "${EVIDENCE_ROOT}/old" "${EVIDENCE_ROOT}/new"
    DESTINATION "${case_root}/evidence")
  set(candidate_rom "${old_rom}")
  set(candidate_manifest "${case_root}/manifest.txt")
endmacro()

function(run_verdict expected_reason)
  execute_process(
    COMMAND "${CHECKER}" --verify
      "${case_root}/baseline.txt" "${case_root}/metadata.txt" "${revision}"
      "${case_root}/scoreboard.txt" "${candidate_rom}" "${candidate_manifest}"
      "${case_root}/migration.tsv" "${case_root}/evidence"
    RESULT_VARIABLE verdict_result
    OUTPUT_VARIABLE verdict_stdout
    ERROR_VARIABLE verdict_stderr)
  set(verdict_output "${verdict_stderr}${verdict_stdout}")
  if(expected_reason STREQUAL "PASS")
    if(NOT "${verdict_result}" STREQUAL "0" OR
       NOT verdict_output MATCHES "protected AccuracyCoin verdict PASS")
      message(FATAL_ERROR "valid protected control failed (${verdict_result}): ${verdict_output}")
    endif()
    if(DEFINED EXPECT_PROTECTED_COUNT AND
       NOT verdict_output MATCHES "${EXPECT_PROTECTED_COUNT} protected PASS keys")
      message(FATAL_ERROR
        "valid protected control did not confirm ${EXPECT_PROTECTED_COUNT} protected PASS keys: ${verdict_output}")
    endif()
  else()
    if("${verdict_result}" STREQUAL "0")
      message(FATAL_ERROR "protected regression control was accepted; expected '${expected_reason}'")
    endif()
    string(FIND "${verdict_output}" "${expected_reason}" expected_offset)
    if(expected_offset EQUAL -1)
      message(FATAL_ERROR
        "protected regression control failed for the wrong reason (${verdict_result}); expected '${expected_reason}': ${verdict_output}")
    endif()
  endif()
endfunction()

function(replace_once path old_text new_text)
  file(READ "${path}" contents)
  string(FIND "${contents}" "${old_text}" offset)
  if(offset EQUAL -1)
    message(FATAL_ERROR "control mutation did not find its input in ${path}: ${old_text}")
  endif()
  string(REPLACE "${old_text}" "${new_text}" updated "${contents}")
  file(WRITE "${path}" "${updated}")
endfunction()

function(create_next_main_baseline root)
  file(STRINGS "${source_scoreboard}" scoreboard_lines)
  set(pass_rows "")
  set(pass_count 0)
  foreach(line IN LISTS scoreboard_lines)
    string(REPLACE "${tab}" ";" fields "${line}")
    list(LENGTH fields field_count)
    if(NOT field_count EQUAL 5)
      message(FATAL_ERROR "next-main control found a malformed canonical scoreboard row")
    endif()
    list(GET fields 1 status)
    if(status STREQUAL "pass")
      string(APPEND pass_rows "${line}${newline}")
      math(EXPR pass_count "${pass_count} + 1")
    endif()
  endforeach()
  if(NOT pass_count EQUAL 110)
    message(FATAL_ERROR "next-main control requires the canonical 110 PASS set, found ${pass_count}")
  endif()
  file(WRITE "${root}/baseline.txt" "${pass_rows}")
  file(SHA256 "${root}/baseline.txt" baseline_sha256)

  file(STRINGS "${SOURCE_DIR}/tests/accuracy/baseline-metadata.txt" metadata_lines
    REGEX "^(rom_pin|rom_sha256|inventory_sha256|inventory_rows|inventory_scored_rows|inventory_draw_rows)=")
  foreach(line IN LISTS metadata_lines)
    string(REPLACE "=" ";" fields "${line}")
    list(GET fields 0 key)
    list(GET fields 1 value)
    set("next_${key}" "${value}")
  endforeach()
  if(NOT next_rom_pin STREQUAL new_pin OR
     NOT next_rom_sha256 STREQUAL
       "8ac8904a3cf26fbdfa6be501e414d45886ff4478d30dee8e58fa436cf5fd5adc" OR
     NOT next_inventory_rows STREQUAL "151" OR
     NOT next_inventory_scored_rows STREQUAL "146" OR
     NOT next_inventory_sha256 STREQUAL
       "7d022800d01ad81159109bf7457af94ab6f80e4a682fce716ec476f7c454a4a5")
    message(FATAL_ERROR "next-main control metadata does not describe the canonical new pin")
  endif()
  file(WRITE "${root}/metadata.txt"
    "schema=1${newline}authority=local-snapshot${newline}source=next-main-production-control${newline}protected_revision=${revision}${newline}rom_pin=${next_rom_pin}${newline}rom_sha256=${next_rom_sha256}${newline}inventory_schema=1${newline}inventory_sha256=${next_inventory_sha256}${newline}inventory_rows=${next_inventory_rows}${newline}inventory_scored_rows=${next_inventory_scored_rows}${newline}inventory_draw_rows=5${newline}scoreboard_schema=1${newline}scoreboard_rows=${pass_count}${newline}scoreboard_denominator=${pass_count}${newline}scoreboard_sha256=${baseline_sha256}${newline}")
endfunction()

macro(prepare_next_main_case name)
  prepare_case(${name})
  configure_file("${next_main_root}/baseline.txt" "${case_root}/baseline.txt" COPYONLY)
  configure_file("${next_main_root}/metadata.txt" "${case_root}/metadata.txt" COPYONLY)
  configure_file("${next_main_root}/scoreboard.txt" "${case_root}/scoreboard.txt" COPYONLY)
  configure_file("${next_main_root}/manifest.txt" "${case_root}/manifest.txt" COPYONLY)
  configure_file("${next_main_root}/migration.tsv" "${case_root}/migration.tsv" COPYONLY)
  file(REMOVE_RECURSE "${case_root}/evidence")
  file(MAKE_DIRECTORY "${case_root}/evidence")
  file(COPY "${next_main_root}/evidence/old" "${next_main_root}/evidence/new"
    DESTINATION "${case_root}/evidence")
  set(candidate_rom "${new_rom}")
  set(candidate_manifest "${case_root}/manifest.txt")
endmacro()

function(mutate_observation root pin page row key status raw cause)
  if(page LESS 10)
    set(page_directory "0${page}")
  else()
    set(page_directory "${page}")
  endif()
  set(report_path "${root}/evidence/${pin}/page-${page_directory}/report.tsv")
  set(raw_path "${root}/evidence/${pin}/page-${page_directory}/raw.tsv")
  file(STRINGS "${report_path}" report_lines)
  set(updated_report "")
  set(found_report 0)
  set(target_prefix "${key}${tab}${page}${tab}${row}${tab}")
  foreach(line IN LISTS report_lines)
    string(FIND "${line}" "${target_prefix}" prefix_offset)
    if(prefix_offset EQUAL 0)
      string(REPLACE "${tab}" ";" fields "${line}")
      list(LENGTH fields field_count)
      if(NOT field_count EQUAL 9)
        message(FATAL_ERROR "control report row has unexpected field count for ${key}")
      endif()
      list(GET fields 6 frames)
      if(status STREQUAL "timeout")
        set(frames 1800)
      endif()
      list(REMOVE_AT fields 3)
      list(INSERT fields 3 "${status}")
      list(REMOVE_AT fields 5)
      list(INSERT fields 5 "${raw}")
      list(REMOVE_AT fields 6)
      list(INSERT fields 6 "${frames}")
      list(REMOVE_AT fields 7)
      list(INSERT fields 7 "${cause}")
      list(JOIN fields "${tab}" updated_line)
      set(line "${updated_line}")
      math(EXPR found_report "${found_report} + 1")
    endif()
    string(APPEND updated_report "${line}${newline}")
  endforeach()
  if(NOT found_report EQUAL 1)
    message(FATAL_ERROR "control report row was missing or duplicated for ${key}")
  endif()
  file(WRITE "${report_path}" "${updated_report}")

  file(STRINGS "${raw_path}" raw_lines)
  set(updated_raw "")
  set(found_raw 0)
  set(raw_prefix "final${tab}${page}${tab}${row}${tab}${key}${tab}")
  foreach(line IN LISTS raw_lines)
    string(FIND "${line}" "${raw_prefix}" prefix_offset)
    if(prefix_offset EQUAL 0)
      string(REPLACE "${tab}" ";" fields "${line}")
      list(LENGTH fields field_count)
      if(NOT field_count EQUAL 7)
        message(FATAL_ERROR "control raw row has unexpected field count for ${key}")
      endif()
      list(REMOVE_AT fields 6)
      list(INSERT fields 6 "${raw}")
      list(JOIN fields "${tab}" updated_line)
      set(line "${updated_line}")
      math(EXPR found_raw "${found_raw} + 1")
    endif()
    string(APPEND updated_raw "${line}${newline}")
  endforeach()
  if(NOT found_raw EQUAL 1)
    message(FATAL_ERROR "control final raw sample was missing or duplicated for ${key}")
  endif()
  file(WRITE "${raw_path}" "${updated_raw}")
endfunction()

function(remove_report_row root pin page row key)
  if(page LESS 10)
    set(page_directory "0${page}")
  else()
    set(page_directory "${page}")
  endif()
  set(report_path "${root}/evidence/${pin}/page-${page_directory}/report.tsv")
  file(STRINGS "${report_path}" report_lines)
  set(updated_report "")
  set(found 0)
  set(target_prefix "${key}${tab}${page}${tab}${row}${tab}")
  foreach(line IN LISTS report_lines)
    string(FIND "${line}" "${target_prefix}" prefix_offset)
    if(prefix_offset EQUAL 0)
      math(EXPR found "${found} + 1")
    else()
      string(APPEND updated_report "${line}${newline}")
    endif()
  endforeach()
  if(NOT found EQUAL 1)
    message(FATAL_ERROR "control could not remove exactly one report row for ${key}")
  endif()
  file(WRITE "${report_path}" "${updated_report}")
endfunction()

function(mutate_final_raw_row root pin page mode row key duplicate_row duplicate_key)
  if(page LESS 10)
    set(page_directory "0${page}")
  else()
    set(page_directory "${page}")
  endif()
  set(raw_path "${root}/evidence/${pin}/page-${page_directory}/raw.tsv")
  file(STRINGS "${raw_path}" raw_lines)
  if(mode STREQUAL "crlf")
    set(updated_raw "")
    foreach(line IN LISTS raw_lines)
      string(APPEND updated_raw "${line}${newline}")
    endforeach()
    string(REPLACE "${newline}" "\r${newline}" updated_raw "${updated_raw}")
    file(WRITE "${raw_path}" "${updated_raw}")
    return()
  endif()

  set(target_prefix "final${tab}${page}${tab}${row}${tab}${key}${tab}")
  set(duplicate_prefix "final${tab}${page}${tab}${duplicate_row}${tab}${duplicate_key}${tab}")
  set(target_found 0)
  set(duplicate_found 0)
  set(duplicate_line "")
  foreach(line IN LISTS raw_lines)
    string(FIND "${line}" "${target_prefix}" target_offset)
    string(FIND "${line}" "${duplicate_prefix}" duplicate_offset)
    if(target_offset EQUAL 0)
      math(EXPR target_found "${target_found} + 1")
    endif()
    if(duplicate_offset EQUAL 0)
      math(EXPR duplicate_found "${duplicate_found} + 1")
      set(duplicate_line "${line}")
    endif()
  endforeach()
  if(NOT target_found EQUAL 1)
    message(FATAL_ERROR "control could not find exactly one final raw sample for ${key}")
  endif()
  if(mode STREQUAL "duplicate" AND NOT duplicate_found EQUAL 1)
    message(FATAL_ERROR "control could not find exactly one duplicate sample for ${duplicate_key}")
  endif()

  set(updated_raw "")
  foreach(line IN LISTS raw_lines)
    string(FIND "${line}" "${target_prefix}" target_offset)
    string(FIND "${line}" "${duplicate_prefix}" duplicate_offset)
    if(target_offset EQUAL 0 AND (mode STREQUAL "remove" OR mode STREQUAL "duplicate"))
      continue()
    elseif(target_offset EQUAL 0 AND mode STREQUAL "malformed")
      string(APPEND updated_raw "final${tab}${page}${tab}${row}${tab}${key}${newline}")
    elseif(mode STREQUAL "duplicate" AND duplicate_offset EQUAL 0)
      string(APPEND updated_raw "${duplicate_line}${newline}${duplicate_line}${newline}")
    else()
      string(APPEND updated_raw "${line}${newline}")
    endif()
  endforeach()
  file(WRITE "${raw_path}" "${updated_raw}")
endfunction()

function(set_new_candidate_manifest root)
  file(STRINGS "${source_manifest}" source_lines)
  set(updated_manifest "")
  set(old_count 0)
  set(new_count 0)
  foreach(line IN LISTS source_lines)
    if(line MATCHES "^#")
      string(APPEND updated_manifest "${line}${newline}")
      continue()
    endif()
    string(FIND "${line}" "tests/roms/accuracycoin-673ef.nes${tab}" old_offset)
    if(old_offset EQUAL 0)
      math(EXPR old_count "${old_count} + 1")
      continue()
    endif()
    string(FIND "${line}" "tests/roms/accuracycoin.nes${tab}" new_offset)
    if(new_offset EQUAL 0)
      math(EXPR new_count "${new_count} + 1")
      string(APPEND updated_manifest "${line}${newline}")
    else()
      string(APPEND updated_manifest "${line}${newline}")
    endif()
  endforeach()
  if(NOT old_count EQUAL 1 OR NOT new_count EQUAL 1)
    message(FATAL_ERROR "new candidate manifest control did not find exactly one old and new ROM row")
  endif()
  file(WRITE "${root}/manifest.txt" "${updated_manifest}")
  set(candidate_rom "${new_rom}" PARENT_SCOPE)
  set(candidate_manifest "${root}/manifest.txt" PARENT_SCOPE)
endfunction()

function(change_migration_reference root key old_raw old_status new_raw new_status frames)
  set(migration_path "${root}/migration.tsv")
  file(STRINGS "${migration_path}" migration_lines)
  set(updated_migration "")
  set(found 0)
  set(target_prefix "${old_pin}${tab}${key}${tab}")
  foreach(line IN LISTS migration_lines)
    string(FIND "${line}" "${target_prefix}" prefix_offset)
    if(prefix_offset EQUAL 0)
      math(EXPR found "${found} + 1")
      string(REPLACE "raw=${old_raw};frames=${frames};status=${old_status}"
        "raw=${new_raw};frames=${frames};status=${new_status}" updated_line "${line}")
      if(updated_line STREQUAL line)
        message(FATAL_ERROR "migration evidence control did not update ${key}'s source link")
      endif()
      set(line "${updated_line}")
    endif()
    string(APPEND updated_migration "${line}${newline}")
  endforeach()
  if(NOT found EQUAL 1)
    message(FATAL_ERROR "migration control could not find exactly one row for ${key}")
  endif()
  file(WRITE "${migration_path}" "${updated_migration}")
endfunction()

function(set_migration_disposition root key disposition)
  set(migration_path "${root}/migration.tsv")
  file(STRINGS "${migration_path}" migration_lines)
  set(updated_migration "")
  set(found 0)
  set(target_prefix "${old_pin}${tab}${key}${tab}")
  foreach(line IN LISTS migration_lines)
    string(FIND "${line}" "${target_prefix}" prefix_offset)
    if(prefix_offset EQUAL 0)
      math(EXPR found "${found} + 1")
      string(REPLACE "${tab}changed-meaning${tab}" "${tab}${disposition}${tab}"
        updated_line "${line}")
      set(line "${updated_line}")
    endif()
    string(APPEND updated_migration "${line}${newline}")
  endforeach()
  if(NOT found EQUAL 1)
    message(FATAL_ERROR "migration disposition control could not find exactly one row for ${key}")
  endif()
  file(WRITE "${migration_path}" "${updated_migration}")
endfunction()

function(remove_migration_row root key)
  set(migration_path "${root}/migration.tsv")
  file(STRINGS "${migration_path}" migration_lines)
  set(updated_migration "")
  set(found 0)
  set(target_prefix "${old_pin}${tab}${key}${tab}")
  foreach(line IN LISTS migration_lines)
    string(FIND "${line}" "${target_prefix}" prefix_offset)
    if(prefix_offset EQUAL 0)
      math(EXPR found "${found} + 1")
    else()
      string(APPEND updated_migration "${line}${newline}")
    endif()
  endforeach()
  if(NOT found EQUAL 1)
    message(FATAL_ERROR "migration control could not remove exactly one row for ${key}")
  endif()
  file(WRITE "${migration_path}" "${updated_migration}")
endfunction()

# Positive same-pin control reaches the complete production verdict path.
prepare_case(valid_same_pin)
run_verdict("PASS")

# A candidate assertion remains PASS while measured protected evidence changes.
foreach(status_case IN ITEMS
    "fail|fail|0x02|terminal-fail"
    "skip|skip|0xff|upstream-skip"
    "not_run|not-run|0x00|unstarted"
    "timeout|timeout|0x03|execution-cap-active-test")
  string(REPLACE "|" ";" status_fields "${status_case}")
  list(GET status_fields 0 case_name)
  list(GET status_fields 1 status)
  list(GET status_fields 2 raw)
  list(GET status_fields 3 cause)
  prepare_case("measured_${case_name}")
  mutate_observation("${case_root}" old 2 1 "accuracycoin/Absolute Indexed"
    "${status}" "${raw}" "${cause}")
  run_verdict("retained old protected key lacks an actual passing observation: ${status}")
endforeach()

prepare_case(missing_report_key)
remove_report_row("${case_root}" old 2 1 "accuracycoin/Absolute Indexed")
run_verdict("protected page report does not cover the full page inventory")

prepare_case(missing_raw_evidence)
file(REMOVE "${case_root}/evidence/old/page-02/raw.tsv")
run_verdict("cannot read raw observation evidence")

prepare_case(crlf_final_raw_evidence)
mutate_final_raw_row("${case_root}" old 2 crlf 0 "" 0 "")
run_verdict("PASS")

prepare_case(missing_final_raw_row)
mutate_final_raw_row("${case_root}" old 2 remove 2 "accuracycoin/Zero Page Indexed" 0 "")
run_verdict("final raw evidence is incomplete")

prepare_case(duplicate_final_raw_row_replaces_missing)
mutate_final_raw_row("${case_root}" old 2 duplicate 2 "accuracycoin/Zero Page Indexed"
  1 "accuracycoin/Absolute Indexed")
run_verdict("duplicate final raw evidence key")

prepare_case(current_pin_duplicate_final_raw_row_replaces_missing)
set_new_candidate_manifest("${case_root}")
mutate_final_raw_row("${case_root}" new 2 duplicate 2 "accuracycoin/Zero Page Indexed"
  1 "accuracycoin/Absolute Indexed")
run_verdict("duplicate final raw evidence key")

prepare_case(malformed_final_raw_row)
mutate_final_raw_row("${case_root}" old 2 malformed 2 "accuracycoin/Zero Page Indexed" 0 "")
run_verdict("final raw evidence row is malformed")

prepare_case(key_substitution_preserves_total)
replace_once("${case_root}/scoreboard.txt"
  "accuracycoin/Absolute Indexed\tpass\t0x01\t12\t-\n"
  "accuracycoin/Absolute Index\tpass\t0x01\t12\t-\n")
run_verdict("candidate scoreboard lost a protected key")

prepare_case(candidate_pin_field_only)
replace_once("${case_root}/manifest.txt"
  "https://github.com/100thCoin/AccuracyCoin/tree/${old_pin}\t${old_pin}\tMIT"
  "https://github.com/100thCoin/AccuracyCoin/tree/${old_pin}\t${new_pin}\tMIT")
run_verdict("candidate manifest identity does not match candidate ROM bytes")

prepare_case(mixed_protected_revision)
replace_once("${case_root}/metadata.txt" "protected_revision=${revision}"
  "protected_revision=0000000000000000000000000000000000000000")
run_verdict("protected metadata revision differs from fetched authority revision")

prepare_case(migration_removed)
remove_migration_row("${case_root}" "accuracycoin/Frame Counter 4-step")
run_verdict("protected migration table must cover all 17 retained keys")

prepare_case(migration_disguised)
set_migration_disposition("${case_root}" "accuracycoin/Frame Counter 4-step" unreviewed)
run_verdict("migration disposition is unrecognized")

# A supported raw pass variant remains accepted when its evidence link and
# candidate row expose the measured byte. The malformed neighboring variant fails.
prepare_case(source_supported_raw_variant)
set_new_candidate_manifest("${case_root}")
mutate_observation("${case_root}" new 17 4 "accuracycoin/PPU Read Buffer"
  pass 0x39 source-defined-pass-variant)
replace_once("${case_root}/scoreboard.txt"
  "accuracycoin/PPU Read Buffer\tpass\t0x41\t150\t-\n"
  "accuracycoin/PPU Read Buffer\tpass\t0x39\t150\t-\n")
change_migration_reference("${case_root}" "accuracycoin/PPU Read Buffer"
  0x41 pass 0x39 pass 150)
run_verdict("PASS")

prepare_case(malformed_raw_variant)
set_new_candidate_manifest("${case_root}")
mutate_observation("${case_root}" new 17 4 "accuracycoin/PPU Read Buffer"
  pass 0x05 unsupported-pass-variant)
replace_once("${case_root}/scoreboard.txt"
  "accuracycoin/PPU Read Buffer\tpass\t0x41\t150\t-\n"
  "accuracycoin/PPU Read Buffer\tpass\t0x05\t150\t-\n")
change_migration_reference("${case_root}" "accuracycoin/PPU Read Buffer"
  0x41 pass 0x05 pass 150)
run_verdict("protected page report status does not match its raw byte")

# Complete measured cross-pin migration is a positive control. The changed-
# meaning branch also permits only a measured terminal fail for its source-reviewed key.
prepare_case(valid_cross_pin)
set_new_candidate_manifest("${case_root}")
run_verdict("PASS")

prepare_case(reviewed_changed_meaning_failure)
set_new_candidate_manifest("${case_root}")
mutate_observation("${case_root}" new 14 4 "accuracycoin/Frame Counter 4-step"
  fail 0x02 terminal-fail)
replace_once("${case_root}/scoreboard.txt"
  "accuracycoin/Frame Counter 4-step\tpass\t0x01\t120\t-\n"
  "accuracycoin/Frame Counter 4-step\tfail\t0x02\t120\t-\n")
change_migration_reference("${case_root}" "accuracycoin/Frame Counter 4-step"
  0x01 pass 0x02 fail 120)
run_verdict("PASS")

prepare_case(undisposed_meaning_failure)
set_new_candidate_manifest("${case_root}")
mutate_observation("${case_root}" new 14 4 "accuracycoin/Frame Counter 4-step"
  fail 0x02 terminal-fail)
replace_once("${case_root}/scoreboard.txt"
  "accuracycoin/Frame Counter 4-step\tpass\t0x01\t120\t-\n"
  "accuracycoin/Frame Counter 4-step\tfail\t0x02\t120\t-\n")
change_migration_reference("${case_root}" "accuracycoin/Frame Counter 4-step"
  0x01 pass 0x02 fail 120)
set_migration_disposition("${case_root}" "accuracycoin/Frame Counter 4-step" unchanged)
run_verdict("unchanged cross-pin key lacks a measured passing observation")

if(FULL_ENABLED)
  # Model the first run after main adopts the new pin. Generate a complete
  # 110-PASS baseline, then let the production page planner and cold observer
  # create this invocation's evidence for every selected page.
  prepare_case(next_main_positive)
  create_next_main_baseline("${case_root}")
  set_new_candidate_manifest("${case_root}")
  execute_process(
    COMMAND "${CMAKE_COMMAND}"
      "-DRUNNER=${RUNNER}"
      "-DVERIFIER=${CHECKER}"
      "-DSOURCE_DIR=${SOURCE_DIR}"
      "-DOLD_ROM=${old_rom}"
      "-DNEW_ROM=${new_rom}"
      "-DWORK=${case_root}/evidence"
      "-DBASELINE=${case_root}/baseline.txt"
      "-DBASELINE_METADATA=${case_root}/metadata.txt"
      "-DMIGRATION=${case_root}/migration.tsv"
      -DPAGE_WALL_SECONDS=30
      -P "${SOURCE_DIR}/tests/cmake/accuracycoin_protected.cmake"
    RESULT_VARIABLE observe_result
    OUTPUT_VARIABLE observe_stdout
    ERROR_VARIABLE observe_stderr)
  if(NOT "${observe_result}" STREQUAL "0")
    message(FATAL_ERROR
      "next-main control could not freshly observe its protected PASS pages (${observe_result}): ${observe_stderr}${observe_stdout}")
  endif()
  set(next_main_root "${case_root}")
  set(EXPECT_PROTECTED_COUNT 110)
  run_verdict("PASS")
  unset(EXPECT_PROTECTED_COUNT)

  # A PASS outside the old 2/14/17 set is now protected, and the same-pin
  # production verdict must reject its measured terminal failure.
  prepare_next_main_case(next_main_outside_old_pages_fails)
  mutate_observation("${case_root}" new 1 1 "accuracycoin/ROM is not writable"
    fail 0x02 terminal-fail)
  replace_once("${case_root}/scoreboard.txt"
    "accuracycoin/ROM is not writable\tpass\t0x01\t51\t-\n"
    "accuracycoin/ROM is not writable\tfail\t0x02\t51\t-\n")
  run_verdict("same-pin protected pass is not measured passing now")

  # A formerly changed-meaning key receives no same-pin failure exemption.
  prepare_next_main_case(next_main_changed_meaning_fails)
  mutate_observation("${case_root}" new 14 3 "accuracycoin/Frame Counter IRQ"
    fail 0x02 terminal-fail)
  replace_once("${case_root}/scoreboard.txt"
    "accuracycoin/Frame Counter IRQ\tpass\t0x01\t120\t-\n"
    "accuracycoin/Frame Counter IRQ\tfail\t0x02\t120\t-\n")
  change_migration_reference("${case_root}" "accuracycoin/Frame Counter IRQ"
    0x01 pass 0x02 fail 120)
  run_verdict("same-pin protected pass is not measured passing now")
endif()
