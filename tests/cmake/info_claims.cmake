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

if(SUPPORT_PROBE)
  foreach(required INFO README HEADER)
    if(NOT DEFINED ${required})
      message(FATAL_ERROR "info support: -D${required}=<path> is required")
    endif()
  endforeach()
  nesturbator__info_parse("${INFO}" error)
  if(error)
    message(FATAL_ERROR "info support: ${INFO}: ${error}")
  endif()
  if(NOT "description" IN_LIST NESTURBATOR_INFO_KEYS)
    message(FATAL_ERROR "info support: ${INFO}: missing description")
  endif()
  set(clause "Supported NTSC MMC3: mapper 4, submappers 0/4; MMC6=no; four-screen=no.")
  set(sources "${NESTURBATOR_INFO_V_description}" "${README}" "${HEADER}")
  set(paths "${INFO}" "${README}" "${HEADER}")
  list(LENGTH sources source_count)
  math(EXPR last_source "${source_count} - 1")
  foreach(index RANGE ${last_source})
    list(GET sources ${index} source)
    list(GET paths ${index} path)
    if(index GREATER 0)
      if(NOT EXISTS "${source}")
        message(FATAL_ERROR "info support: ${source} does not exist")
      endif()
      file(READ "${source}" source)
    endif()
    string(FIND "${source}" "${clause}" clause_at)
    if(clause_at LESS 0)
      message(FATAL_ERROR "info support: ${path} lacks the localized MMC3 support clause")
    endif()
  endforeach()
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
