cmake_minimum_required(VERSION 3.25)

# LR-02 / D-18, D-19: the capability claims in libretro/nesturbator_libretro.info.
#
# RetroArch gates rewind on savestate_features = "serialized" and run-ahead on
# "deterministic", and a missing savestate_features with savestate = "true" acts as
# "deterministic" (13-RESEARCH, flag 1). So a claim is a promise the tests must back, and the
# promise is kept by the tree, not by commit order (a squash merge hides that).
#
# This file holds, once:
#   - the claim-to-tests table below;
#   - the .info parser;
#   - the configure-time guard, called from tests/CMakeLists.txt after every add_test;
#   - the -P check (-DINFO=path) behind CTest libretro.info_claims;
#   - the -P self-test (-DSELFTEST=ON ...) behind CTest libretro.info_claims.self_test.

# Each claimed level needs these CTests registered. The levels are cumulative.
set(NESTURBATOR_CLAIM_TESTS_serialized libretro.serialize)
set(NESTURBATOR_CLAIM_TESTS_deterministic libretro.serialize libretro.runahead core.state_runahead)

# Keys that describe the core. Any other key is a capability.
set(NESTURBATOR_INFO_METADATA_KEYS display_name authors supported_extensions corename license
  permissions display_version manufacturer categories systemname systemid database description)

# The capability keys this project sets, with the values it pins. savestate and
# savestate_features are checked separately: they are the claim.
set(NESTURBATOR_INFO_PINNED_KEYS supports_no_game libretro_saves memory_descriptors
  core_options disk_control is_experimental)
set(NESTURBATOR_INFO_PINNED_supports_no_game true)
set(NESTURBATOR_INFO_PINNED_libretro_saves true)
set(NESTURBATOR_INFO_PINNED_memory_descriptors false)
set(NESTURBATOR_INFO_PINNED_core_options false)
set(NESTURBATOR_INFO_PINNED_disk_control false)
set(NESTURBATOR_INFO_PINNED_is_experimental true)

# Reads key = "value" lines (blank lines and # comments skipped). Sets in the caller's scope
# NESTURBATOR_INFO_KEYS (the keys in file order) and NESTURBATOR_INFO_V_<key>. out_error is
# empty when the file is well formed: no empty file, no unparseable line, no duplicate key.
function(nesturbator__info_parse path out_error)
  set(keys "")
  set(${out_error} "" PARENT_SCOPE)
  if(NOT EXISTS "${path}")
    set(${out_error} "${path} does not exist" PARENT_SCOPE)
    return()
  endif()
  file(STRINGS "${path}" lines)
  foreach(line IN LISTS lines)
    if(line MATCHES "^[ \t]*$" OR line MATCHES "^[ \t]*#")
      continue()
    endif()
    if(NOT line MATCHES "^([a-z_0-9]+) = \"([^\"]*)\"[ \t]*$")
      set(${out_error} "cannot parse line: ${line}" PARENT_SCOPE)
      return()
    endif()
    set(key "${CMAKE_MATCH_1}")
    if(key IN_LIST keys)
      set(${out_error} "duplicate key ${key}" PARENT_SCOPE)
      return()
    endif()
    list(APPEND keys "${key}")
    set(NESTURBATOR_INFO_V_${key} "${CMAKE_MATCH_2}")
    set(NESTURBATOR_INFO_V_${key} "${CMAKE_MATCH_2}" PARENT_SCOPE)
  endforeach()
  if(NOT keys)
    set(${out_error} "${path} has no key = \"value\" lines" PARENT_SCOPE)
    return()
  endif()
  set(NESTURBATOR_INFO_KEYS "${keys}" PARENT_SCOPE)
endfunction()

# Parses the file and returns the level it claims in out_level: none, serialized or
# deterministic. out_error is empty when the savestate claim is well formed:
#   savestate = "false" with no savestate_features, or
#   savestate = "true" with savestate_features serialized or deterministic.
# "basic" is never claimed, and a missing savestate_features with "true" is an error because
# RetroArch would read it as "deterministic".
function(nesturbator_info_level path out_level out_error)
  set(${out_level} "none" PARENT_SCOPE)
  nesturbator__info_parse("${path}" error)
  if(error)
    set(${out_error} "${error}" PARENT_SCOPE)
    return()
  endif()
  set(${out_error} "" PARENT_SCOPE)
  if(NOT "savestate" IN_LIST NESTURBATOR_INFO_KEYS)
    set(${out_error} "no savestate key" PARENT_SCOPE)
    return()
  endif()
  if(NESTURBATOR_INFO_V_savestate STREQUAL "false")
    if("savestate_features" IN_LIST NESTURBATOR_INFO_KEYS)
      set(${out_error} "savestate_features is set but savestate is \"false\"" PARENT_SCOPE)
    endif()
    return()
  endif()
  if(NOT NESTURBATOR_INFO_V_savestate STREQUAL "true")
    set(${out_error} "savestate is \"${NESTURBATOR_INFO_V_savestate}\", not true or false"
      PARENT_SCOPE)
    return()
  endif()
  if(NOT "savestate_features" IN_LIST NESTURBATOR_INFO_KEYS)
    set(${out_error}
      "savestate = \"true\" without savestate_features (RetroArch reads that as deterministic)"
      PARENT_SCOPE)
    return()
  endif()
  if(NOT NESTURBATOR_INFO_V_savestate_features MATCHES "^(serialized|deterministic)$")
    set(${out_error}
      "savestate_features is \"${NESTURBATOR_INFO_V_savestate_features}\", not serialized or deterministic"
      PARENT_SCOPE)
    return()
  endif()
  set(${out_level} "${NESTURBATOR_INFO_V_savestate_features}" PARENT_SCOPE)
endfunction()

# D-19: the exact capability key set and the pinned values, on top of a well formed claim.
# Compares key sets, so the order of the lines does not matter.
function(nesturbator_info_check path out_error)
  nesturbator_info_level("${path}" level error)
  if(error)
    set(${out_error} "${error}" PARENT_SCOPE)
    return()
  endif()
  set(${out_error} "" PARENT_SCOPE)
  nesturbator__info_parse("${path}" error)
  set(expected ${NESTURBATOR_INFO_PINNED_KEYS} savestate)
  if(NOT level STREQUAL "none")
    list(APPEND expected savestate_features)
  endif()
  foreach(key IN LISTS NESTURBATOR_INFO_KEYS)
    if(NOT key IN_LIST NESTURBATOR_INFO_METADATA_KEYS AND NOT key IN_LIST expected)
      set(${out_error} "unknown capability key ${key}" PARENT_SCOPE)
      return()
    endif()
  endforeach()
  foreach(key IN LISTS expected)
    if(NOT key IN_LIST NESTURBATOR_INFO_KEYS)
      set(${out_error} "missing capability key ${key}" PARENT_SCOPE)
      return()
    endif()
  endforeach()
  foreach(key IN LISTS NESTURBATOR_INFO_PINNED_KEYS)
    if(NOT NESTURBATOR_INFO_V_${key} STREQUAL NESTURBATOR_INFO_PINNED_${key})
      set(${out_error}
        "${key} is \"${NESTURBATOR_INFO_V_${key}}\", expected \"${NESTURBATOR_INFO_PINNED_${key}}\""
        PARENT_SCOPE)
      return()
    endif()
  endforeach()
endfunction()

# D-15: inspect only the localized support fact clause, leaving surrounding prose free to change.
function(nesturbator_info__support_text text source out_error)
  set(${out_error} "" PARENT_SCOPE)
  string(REGEX MATCHALL "Supported NTSC " support_prefixes "${text}")
  list(LENGTH support_prefixes support_count)
  if(support_count GREATER 1)
    set(${out_error} "${source}: duplicate localized MMC3 support clause" PARENT_SCOPE)
    return()
  endif()
  string(FIND "${text}" "Supported NTSC " start)
  if(start LESS 0)
    set(${out_error} "${source}: missing localized MMC3 support clause" PARENT_SCOPE)
    return()
  endif()
  string(SUBSTRING "${text}" ${start} -1 tail)
  string(FIND "${tail}" "." period)
  if(period LESS 0)
    set(${out_error} "${source}: malformed localized MMC3 support clause" PARENT_SCOPE)
    return()
  endif()
  string(SUBSTRING "${tail}" 0 ${period} clause)
  string(REPLACE ";" ";" facts "${clause}")
  list(LENGTH facts fact_count)
  list(GET facts 0 profile)
  string(STRIP "${profile}" profile)
  set(normalized_facts "${profile}")
  foreach(fact IN LISTS facts)
    string(STRIP "${fact}" fact)
    if(NOT fact STREQUAL profile)
      list(APPEND normalized_facts "${fact}")
    endif()
  endforeach()
  if(NOT profile MATCHES "^Supported ([^ ]+) ([^:]+): mapper ([0-9]+), submappers ([^ ]+)$")
    set(${out_error} "${source}: malformed MMC3 profile fact" PARENT_SCOPE)
    return()
  endif()
  set(region "${CMAKE_MATCH_1}")
  set(board "${CMAKE_MATCH_2}")
  set(mapper "${CMAKE_MATCH_3}")
  set(submappers "${CMAKE_MATCH_4}")
  if(NOT region STREQUAL "NTSC" OR NOT board STREQUAL "MMC3")
    set(${out_error}
      "${source}: supported board is ${region} ${board}; expected NTSC MMC3"
      PARENT_SCOPE)
    return()
  endif()
  if(NOT mapper STREQUAL "4")
    set(${out_error} "${source}: mapper is ${mapper}; expected 4 for MMC3" PARENT_SCOPE)
    return()
  endif()
  if(NOT submappers STREQUAL "0/4")
    set(${out_error} "${source}: submappers are ${submappers}; expected 0/4" PARENT_SCOPE)
    return()
  endif()
  if(NOT "MMC6=no" IN_LIST normalized_facts)
    if("MMC6=yes" IN_LIST normalized_facts)
      set(${out_error} "${source}: MMC6 must be excluded (found MMC6=yes)" PARENT_SCOPE)
    else()
      set(${out_error} "${source}: missing MMC6=no exclusion" PARENT_SCOPE)
    endif()
    return()
  endif()
  if(NOT "four-screen=no" IN_LIST normalized_facts)
    if("four-screen=yes" IN_LIST normalized_facts)
      set(${out_error} "${source}: four-screen must be excluded (found four-screen=yes)"
        PARENT_SCOPE)
    else()
      set(${out_error} "${source}: missing four-screen=no exclusion" PARENT_SCOPE)
    endif()
    return()
  endif()
  if(NOT fact_count EQUAL 3)
    set(${out_error} "${source}: unexpected fact in MMC3 support clause" PARENT_SCOPE)
  endif()
endfunction()

function(nesturbator_info_support_check info_path readme_path header_path out_error)
  set(${out_error} "" PARENT_SCOPE)
  nesturbator_info_check("${info_path}" error)
  if(error)
    set(${out_error} "${info_path}: ${error}" PARENT_SCOPE)
    return()
  endif()
  nesturbator__info_parse("${info_path}" error)
  if(NOT "description" IN_LIST NESTURBATOR_INFO_KEYS)
    set(${out_error} "${info_path}: missing description" PARENT_SCOPE)
    return()
  endif()
  nesturbator_info__support_text("${NESTURBATOR_INFO_V_description}" "${info_path}" error)
  if(error)
    set(${out_error} "${error}" PARENT_SCOPE)
    return()
  endif()
  foreach(path IN ITEMS "${readme_path}" "${header_path}")
    if(NOT EXISTS "${path}")
      set(${out_error} "${path} does not exist" PARENT_SCOPE)
      return()
    endif()
    file(READ "${path}" text)
    nesturbator_info__support_text("${text}" "${path}" error)
    if(error)
      set(${out_error} "${error}" PARENT_SCOPE)
      return()
    endif()
  endforeach()
endfunction()

# D-18: configure fails when the claimed level's proving tests are not registered. Call it
# with cmake_language(DEFER) after every add_test of the directory.
function(nesturbator_info_claim_guard path)
  nesturbator_info_level("${path}" level error)
  if(error)
    message(FATAL_ERROR "${path}: ${error}")
  endif()
  if(level STREQUAL "none")
    return()
  endif()
  foreach(name IN LISTS NESTURBATOR_CLAIM_TESTS_${level})
    if(NOT TEST ${name})
      message(FATAL_ERROR
        "${path} claims savestate level ${level} but its proving test ${name} is not registered")
    endif()
  endforeach()
endfunction()

# ---- script mode ----
if(NOT CMAKE_SCRIPT_MODE_FILE STREQUAL CMAKE_CURRENT_LIST_FILE)
  return()
endif()

if(STAGED)
  foreach(required SOURCE_ROOT BUILD_DIR WORK)
    if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
      message(FATAL_ERROR "info staged: -D${required}=<path> is required")
    endif()
  endforeach()
  cmake_path(ABSOLUTE_PATH BUILD_DIR NORMALIZE OUTPUT_VARIABLE build_dir)
  cmake_path(ABSOLUTE_PATH WORK NORMALIZE OUTPUT_VARIABLE work_dir)
  if(NOT EXISTS "${build_dir}/CMakeCache.txt")
    message(FATAL_ERROR "info staged: ${build_dir} is not a configured build directory")
  endif()
  if(NOT work_dir STREQUAL "${build_dir}/tests/info-staged")
    message(FATAL_ERROR "info staged: WORK must be the isolated build-owned tests/info-staged directory")
  endif()
  set(source_info "${SOURCE_ROOT}/libretro/nesturbator_libretro.info")
  set(readme "${SOURCE_ROOT}/README.md")
  set(header "${SOURCE_ROOT}/include/nesturbator.h")
  foreach(path IN ITEMS "${source_info}" "${readme}" "${header}")
    if(NOT EXISTS "${path}")
      message(FATAL_ERROR "info staged: missing current input ${path}")
    endif()
  endforeach()
  file(REMOVE_RECURSE "${work_dir}")
  file(MAKE_DIRECTORY "${work_dir}")
  execute_process(COMMAND "${CMAKE_COMMAND}" --install "${build_dir}" --component libretro
      --prefix "${work_dir}/root"
    RESULT_VARIABLE install_rc OUTPUT_VARIABLE install_out ERROR_VARIABLE install_err)
  if(NOT install_rc EQUAL 0)
    message(FATAL_ERROR "info staged: component install failed (${install_rc}): ${install_out}${install_err}")
  endif()
  set(staged_info "${work_dir}/root/info/nesturbator_libretro.info")
  if(NOT EXISTS "${staged_info}")
    message(FATAL_ERROR "info staged: component install did not create ${staged_info}")
  endif()
  file(SIZE "${source_info}" source_size)
  file(SIZE "${staged_info}" staged_size)
  if(source_size EQUAL 0 OR staged_size EQUAL 0)
    message(FATAL_ERROR "info staged: source and installed metadata must be nonempty")
  endif()
  file(SHA256 "${source_info}" source_sha)
  file(SHA256 "${staged_info}" staged_sha)
  if(NOT source_sha STREQUAL staged_sha)
    message(FATAL_ERROR "info staged: installed metadata differs from the current source .info")
  endif()
  nesturbator_info_support_check("${staged_info}" "${readme}" "${header}" error)
  if(error)
    message(FATAL_ERROR "info staged: ${error}")
  endif()
  message(STATUS "info staged: current component metadata matches source and support facts")
  return()
endif()

if(SUPPORT_PROBE)
  foreach(required INFO README HEADER)
    if(NOT DEFINED ${required})
      message(FATAL_ERROR "info support: -D${required}=<path> is required")
    endif()
  endforeach()
  nesturbator_info_support_check("${INFO}" "${README}" "${HEADER}" error)
  if(error)
    message(FATAL_ERROR "info support: ${error}")
  endif()
  message(STATUS "info support: localized MMC3 support clauses present")
  return()
endif()

if(NOT SELFTEST)
  if(NOT DEFINED INFO)
    message(FATAL_ERROR "info_claims: -DINFO=<path> is required")
  endif()
  nesturbator_info_check("${INFO}" error)
  if(error)
    message(FATAL_ERROR "info_claims: ${INFO}: ${error}")
  endif()
  nesturbator_info_level("${INFO}" level error)
  message(STATUS "info_claims: ok, savestate level ${level}")
  return()
endif()

foreach(required WORK GENERATOR MAKE_PROGRAM)
  if(NOT DEFINED ${required})
    message(FATAL_ERROR "info_claims self-test: -D${required} is required")
  endif()
endforeach()
file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")

set(meta "display_name = \"x\"\ncorename = \"x\"\n")
set(caps "supports_no_game = \"true\"\nlibretro_saves = \"true\"\nmemory_descriptors = \"false\"\ncore_options = \"false\"\ndisk_control = \"false\"\nis_experimental = \"true\"\n")

# Writes the file and requires nesturbator_info_level (or _check) to report an error that
# contains wanted.
function(write_info name content)
  file(WRITE "${WORK}/${name}.info" "${content}")
endfunction()

function(expect_level_error name content wanted)
  write_info(${name} "${content}")
  nesturbator_info_level("${WORK}/${name}.info" level error)
  if(NOT error)
    message(FATAL_ERROR "info_claims self-test: accepted ${name}")
  endif()
  if(NOT error MATCHES "${wanted}")
    message(FATAL_ERROR "info_claims self-test: ${name} rejected for the wrong reason: ${error}")
  endif()
endfunction()

function(expect_check_error name content wanted)
  write_info(${name} "${content}")
  nesturbator_info_check("${WORK}/${name}.info" error)
  if(NOT error)
    message(FATAL_ERROR "info_claims self-test: accepted ${name}")
  endif()
  if(NOT error MATCHES "${wanted}")
    message(FATAL_ERROR "info_claims self-test: ${name} rejected for the wrong reason: ${error}")
  endif()
endfunction()

function(expect_level name content wanted_level)
  write_info(${name} "${content}")
  nesturbator_info_check("${WORK}/${name}.info" error)
  if(error)
    message(FATAL_ERROR "info_claims self-test: rejected ${name}: ${error}")
  endif()
  nesturbator_info_level("${WORK}/${name}.info" level error)
  if(NOT level STREQUAL wanted_level)
    message(FATAL_ERROR "info_claims self-test: ${name} is level ${level}, expected ${wanted_level}")
  endif()
endfunction()

# Parser failures (LR-02 empty).
expect_level_error(empty "" "no key")
expect_level_error(comments_only "# nothing\n\n" "no key")
expect_level_error(no_savestate "${meta}${caps}" "no savestate key")
expect_level_error(true_without_features "${meta}${caps}savestate = \"true\"\n"
  "without savestate_features")
expect_level_error(duplicate "${meta}${caps}savestate = \"false\"\nsavestate = \"false\"\n"
  "duplicate key savestate")
expect_level_error(garbage "${meta}${caps}savestate = false\n" "cannot parse")
expect_level_error(basic "${meta}${caps}savestate = \"true\"\nsavestate_features = \"basic\"\n"
  "not serialized or deterministic")
expect_level_error(features_with_false
  "${meta}${caps}savestate = \"false\"\nsavestate_features = \"serialized\"\n" "is \"false\"")
expect_level_error(maybe "${meta}${caps}savestate = \"maybe\"\n" "not true or false")
expect_level_error(missing_file_content_stub "x" "cannot parse")

# Check failures: unknown capability key, a changed pinned value, a missing key.
expect_check_error(unknown_key "${meta}${caps}cheats = \"true\"\nsavestate = \"false\"\n"
  "unknown capability key cheats")
expect_check_error(changed_pin
  "${meta}supports_no_game = \"true\"\nlibretro_saves = \"true\"\nmemory_descriptors = \"false\"\ncore_options = \"true\"\ndisk_control = \"false\"\nis_experimental = \"true\"\nsavestate = \"false\"\n"
  "core_options is \"true\"")
expect_check_error(missing_pin
  "${meta}supports_no_game = \"true\"\nlibretro_saves = \"true\"\nmemory_descriptors = \"false\"\ncore_options = \"false\"\nis_experimental = \"true\"\nsavestate = \"false\"\n"
  "missing capability key disk_control")

# Accepted files, and the same lines in another order reach the same level (LR-02 ordering).
expect_level(unclaimed "${meta}${caps}savestate = \"false\"\n" none)
expect_level(serialized "${meta}${caps}savestate = \"true\"\nsavestate_features = \"serialized\"\n"
  serialized)
expect_level(deterministic
  "${meta}${caps}savestate = \"true\"\nsavestate_features = \"deterministic\"\n" deterministic)
expect_level(reordered
  "savestate_features = \"deterministic\"\nis_experimental = \"true\"\ndisk_control = \"false\"\ncore_options = \"false\"\nmemory_descriptors = \"false\"\nlibretro_saves = \"true\"\nsavestate = \"true\"\nsupports_no_game = \"true\"\ncorename = \"x\"\ndisplay_name = \"x\"\n"
  deterministic)

# D-15: every control begins with the complete current metadata and documentation inputs.
set(source_root "${CMAKE_CURRENT_LIST_DIR}/../..")
file(READ "${source_root}/libretro/nesturbator_libretro.info" valid_info)
file(READ "${source_root}/README.md" valid_readme)
file(READ "${source_root}/include/nesturbator.h" valid_header)
nesturbator_info_support_check("${source_root}/libretro/nesturbator_libretro.info"
  "${source_root}/README.md" "${source_root}/include/nesturbator.h" error)
if(error)
  message(FATAL_ERROR "info_claims self-test: current MMC3 support facts failed: ${error}")
endif()

function(expect_support_error name info_content readme_content header_content wanted)
  set(info_path "${WORK}/${name}.info")
  set(readme_path "${WORK}/${name}-README.md")
  set(header_path "${WORK}/${name}-header.h")
  file(WRITE "${info_path}" "${info_content}")
  file(WRITE "${readme_path}" "${readme_content}")
  file(WRITE "${header_path}" "${header_content}")
  nesturbator_info_support_check("${info_path}" "${readme_path}" "${header_path}" error)
  if(NOT error)
    message(FATAL_ERROR "info_claims self-test: accepted support mutant ${name}")
  endif()
  if(NOT error MATCHES "${wanted}")
    message(FATAL_ERROR "info_claims self-test: ${name} failed for the wrong reason: ${error}")
  endif()
endfunction()

function(expect_support_ok name info_content readme_content header_content)
  set(info_path "${WORK}/${name}.info")
  set(readme_path "${WORK}/${name}-README.md")
  set(header_path "${WORK}/${name}-header.h")
  file(WRITE "${info_path}" "${info_content}")
  file(WRITE "${readme_path}" "${readme_content}")
  file(WRITE "${header_path}" "${header_content}")
  nesturbator_info_support_check("${info_path}" "${readme_path}" "${header_path}" error)
  if(error)
    message(FATAL_ERROR "info_claims self-test: rejected harmless support prose ${name}: ${error}")
  endif()
endfunction()

set(support_clause "Supported NTSC MMC3: mapper 4, submappers 0/4; MMC6=no; four-screen=no.")
set(contradictory_clause
  "Supported NTSC MMC3: mapper 4, submappers 0/4; MMC6=yes; four-screen=no.")
string(REPLACE "${support_clause}" "" no_support_info "${valid_info}")
expect_support_error(missing_positive_clause "${no_support_info}" "${valid_readme}"
  "${valid_header}" "missing localized MMC3 support clause")
string(REPLACE "NTSC MMC3:" "NTSC MMC2:" unsupported_board "${valid_info}")
expect_support_error(unsupported_board "${unsupported_board}" "${valid_readme}" "${valid_header}"
  "expected NTSC MMC3")
string(REPLACE "mapper 4, submappers" "mapper 5, submappers" unsupported_mapper "${valid_info}")
expect_support_error(unsupported_mapper "${unsupported_mapper}" "${valid_readme}"
  "${valid_header}" "mapper is 5")
string(REPLACE "submappers 0/4" "submappers 0/4/5" broad_submappers "${valid_info}")
expect_support_error(broader_submappers "${broad_submappers}" "${valid_readme}"
  "${valid_header}" "submappers are 0/4/5")
string(REPLACE "MMC6=no; " "" no_mmc6_exclusion "${valid_info}")
expect_support_error(no_mmc6_exclusion "${no_mmc6_exclusion}" "${valid_readme}"
  "${valid_header}" "missing MMC6=no exclusion")
string(REPLACE "; four-screen=no" "" no_four_screen_exclusion "${valid_info}")
expect_support_error(no_four_screen_exclusion "${no_four_screen_exclusion}" "${valid_readme}"
  "${valid_header}" "missing four-screen=no exclusion")
string(REPLACE "mapper 4, submappers" "mapper 5, submappers" bad_readme "${valid_readme}")
expect_support_error(contradictory_readme "${valid_info}" "${bad_readme}" "${valid_header}"
  "README.md: mapper is 5")
string(REPLACE "MMC6=no" "MMC6=yes" bad_header "${valid_header}")
expect_support_error(contradictory_header "${valid_info}" "${valid_readme}" "${bad_header}"
  "header.h: MMC6 must be excluded")
string(REPLACE "${support_clause}" "${support_clause} ${contradictory_clause}"
  contradictory_metadata "${valid_info}")
expect_support_error(later_metadata_contradiction "${contradictory_metadata}" "${valid_readme}"
  "${valid_header}" "duplicate localized MMC3 support clause")
string(REPLACE "${support_clause}" "${support_clause}\n${contradictory_clause}"
  later_readme_contradiction "${valid_readme}")
expect_support_error(later_readme_contradiction "${valid_info}" "${later_readme_contradiction}"
  "${valid_header}" "duplicate localized MMC3 support clause")
string(REPLACE "${support_clause}" "${support_clause}\n * ${contradictory_clause}"
  later_header_contradiction "${valid_header}")
expect_support_error(later_header_contradiction "${valid_info}" "${valid_readme}"
  "${later_header_contradiction}" "duplicate localized MMC3 support clause")
set(duplicate_description "${valid_info}description = \"duplicate\"\n")
expect_support_error(duplicate_description "${duplicate_description}" "${valid_readme}"
  "${valid_header}" "duplicate key description")
string(REPLACE "Accuracy-focused" "Accurate" harmless_info "${valid_info}")
set(harmless_readme "${valid_readme}\nSurrounding product prose may change.\n")
set(harmless_header "${valid_header}\nSurrounding product prose may change.\n")
expect_support_ok(harmless_surrounding_prose "${harmless_info}" "${harmless_readme}"
  "${harmless_header}")
message(STATUS "info_claims self-test: MMC3 support controls passed (13 cases)")

# The configure-time guard in a scratch project: all the tests registered configures, and with
# one of the claim's tests omitted it fails and names the level and the test.
function(configure_guard name info omit)
  execute_process(COMMAND "${CMAKE_COMMAND}"
    -S "${CMAKE_CURRENT_LIST_DIR}/claim_guard" -B "${WORK}/${name}"
    -G "${GENERATOR}" "-DCMAKE_MAKE_PROGRAM=${MAKE_PROGRAM}"
    "-DINFO=${info}" "-DOMIT=${omit}"
    RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)
  set(guard_rc ${rc} PARENT_SCOPE)
  set(guard_text "${out}\n${err}" PARENT_SCOPE)
endfunction()

function(expect_guard_ok name info omit)
  configure_guard(${name} "${info}" "${omit}")
  if(NOT guard_rc EQUAL 0)
    message(FATAL_ERROR "info_claims self-test: guard rejected ${name}:\n${guard_text}")
  endif()
endfunction()

function(expect_guard_fail name info omit level)
  configure_guard(${name} "${info}" "${omit}")
  if(guard_rc EQUAL 0)
    message(FATAL_ERROR "info_claims self-test: guard accepted ${name}")
  endif()
  # CMake wraps long error text; compare it with the whitespace collapsed.
  string(REGEX REPLACE "[ \t\r\n]+" " " flat "${guard_text}")
  if(NOT flat MATCHES "claims savestate level ${level}" OR
      NOT flat MATCHES "${omit} is not registered")
    message(FATAL_ERROR "info_claims self-test: guard failed ${name} without naming the level and test:\n${guard_text}")
  endif()
endfunction()

expect_guard_ok(guard_none "${WORK}/unclaimed.info" "")
expect_guard_ok(guard_none_omit "${WORK}/unclaimed.info" "libretro.serialize")
expect_guard_ok(guard_serialized "${WORK}/serialized.info" "libretro.runahead")
expect_guard_ok(guard_deterministic "${WORK}/deterministic.info" "")
expect_guard_fail(guard_serialized_missing "${WORK}/serialized.info" "libretro.serialize"
  serialized)
foreach(omitted IN LISTS NESTURBATOR_CLAIM_TESTS_deterministic)
  expect_guard_fail(guard_deterministic_missing_${omitted} "${WORK}/deterministic.info"
    "${omitted}" deterministic)
endforeach()
message(STATUS "info_claims self-test: ok")
