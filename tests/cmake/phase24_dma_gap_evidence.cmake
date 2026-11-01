cmake_minimum_required(VERSION 3.25)

foreach(required MODE SOURCE_REF ROOT)
  if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
    message(FATAL_ERROR "phase24 DMA gap evidence requires -D${required}=<value>")
  endif()
endforeach()

get_filename_component(source_dir "${SOURCE_REF}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}/../..")
get_filename_component(root "${ROOT}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}")
file(REAL_PATH "${source_dir}" source_dir)
string(FIND "${root}" "${source_dir}/build/" build_root_position)
if(NOT build_root_position EQUAL 0)
  message(FATAL_ERROR "ROOT must be inside the ignored build/ directory: ${root}")
endif()

execute_process(
  COMMAND git check-ignore -q "${root}"
  WORKING_DIRECTORY "${source_dir}"
  RESULT_VARIABLE ignore_result)
if(NOT "${ignore_result}" STREQUAL "0")
  message(FATAL_ERROR "ROOT is not ignored by Git: ${root}")
endif()

set(NESTURBATOR_PHASE20_REPOSITORY "szTheory/nesturbator")
set(NESTURBATOR_PHASE20_PLATFORMS
  linux-x64 linux-arm64 macos-x64 macos-arm64 windows-x64 windows-arm64)
set(NESTURBATOR_PHASE20_BEHAVIOUR_REVISION 14)
set(NESTURBATOR_PHASE20_LIBRARY_VERSION "0.1.11")
set(NESTURBATOR_PHASE20_PERF_ARTIFACT_PREFIX "phase20-perf")
set(NESTURBATOR_PHASE20_MARKER_ARTIFACT "retroarch-marker-progression")
set(NESTURBATOR_PHASE20_HASH_ARTIFACT_PREFIX "hashes")
set(NESTURBATOR_PHASE20_ACCURACYCOIN_ARTIFACT_PREFIX "accuracycoin-evidence")
set(NESTURBATOR_PHASE24_REQUIRED_TARGET_KEYS
  "accuracycoin/DMA Landing on Write"
  "accuracycoin/DMC DMA + OAM DMA"
  "accuracycoin/DMC DMA Bus Conflicts"
  "accuracycoin/Explicit DMA Abort"
  "accuracycoin/Implicit DMA Abort"
  "accuracycoin/DMA + $4015 Read"
  "accuracycoin/Misaligned OAM DMA"
  "accuracycoin/INC $4014")
set(NESTURBATOR_PHASE20_REQUIRED_TARGET_KEYS ${NESTURBATOR_PHASE24_REQUIRED_TARGET_KEYS})
set(NESTURBATOR_PHASE20_EVIDENCE_LIBRARY_ONLY ON)
include("${CMAKE_CURRENT_LIST_DIR}/phase20_hosted_evidence.cmake")
string(TIMESTAMP _PHASE24_NOW "%Y-%m-%dT%H:%M:%SZ" UTC)
set(PHASE24_POST_REQUEST_PATHS
  .planning/STATE.md
  .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-ACCEPTANCE.md
  .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-EXECUTION-RESUME.md
  .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-HOSTED-EVIDENCE.md
  .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-05-SUMMARY.md)

function(phase24_expect_rejection label expected_pattern actual_error)
  if("${actual_error}" STREQUAL "" OR NOT "${actual_error}" MATCHES "${expected_pattern}")
    message(FATAL_ERROR "${label} control failed: expected '${expected_pattern}', got '${actual_error}'")
  endif()
  message(STATUS "control rejected ${label}: ${actual_error}")
endfunction()

function(phase24_git out_value)
  execute_process(
    COMMAND git ${ARGN}
    WORKING_DIRECTORY "${source_dir}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE value
    ERROR_VARIABLE error
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(NOT "${result}" STREQUAL "0")
    message(FATAL_ERROR "git ${ARGN} failed (${result}): ${error}")
  endif()
  set(${out_value} "${value}" PARENT_SCOPE)
endfunction()

function(phase24_lower_sha value digits out_ok)
  string(LENGTH "${value}" value_length)
  if("${value}" MATCHES "^[0-9a-f]+$" AND value_length EQUAL digits)
    set(${out_ok} TRUE PARENT_SCOPE)
  else()
    set(${out_ok} FALSE PARENT_SCOPE)
  endif()
endfunction()

function(phase24_control_files root)
  file(REMOVE_RECURSE "${root}")
  file(MAKE_DIRECTORY "${root}")
  set(base_sha "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa")
  set(protected_rows
    "accuracycoin/DMA Landing on Write\tpass\t0x01\t-\t-\n"
    "accuracycoin/DMC DMA + OAM DMA\tpass\t0x01\t-\t-\n"
    "accuracycoin/DMC DMA Bus Conflicts\tpass\t0x01\t-\t-\n")
  set(candidate_rows)
  foreach(key IN LISTS NESTURBATOR_PHASE24_REQUIRED_TARGET_KEYS)
    list(APPEND candidate_rows "${key}\tpass\t0x01\t46\t-")
  endforeach()
  list(SORT candidate_rows)
  list(JOIN candidate_rows "\n" candidate_text)
  list(JOIN protected_rows "" protected_text)
  file(WRITE "${root}/candidate-scoreboard.tsv" "${candidate_text}\n")
  file(WRITE "${root}/protected-scoreboard.txt" "${protected_text}")
  file(SHA256 "${root}/protected-scoreboard.txt" protected_sha)
  file(WRITE "${root}/protected-metadata.txt"
    "fixture=control-only\nauthority=protected-main\nprotected_revision=${base_sha}\n"
    "scoreboard_sha256=${protected_sha}\nscoreboard_rows=3\nscoreboard_denominator=3\n")
  set(${root}_base_sha "${base_sha}" PARENT_SCOPE)
endfunction()

function(phase24_run_hash_controls work_dir)
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -DSELFTEST=ON "-DWORK=${work_dir}"
      -P "${CMAKE_CURRENT_LIST_DIR}/hash_inventory.cmake"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE stdout
    ERROR_VARIABLE stderr)
  set(output "${stdout}${stderr}")
  if(NOT "${result}" STREQUAL "0" OR
     NOT output MATCHES "all-six missing-key control rejected" OR
     NOT output MATCHES "single-platform missing-key control rejected")
    message(FATAL_ERROR "hash inventory controls did not prove missing-key rejection: ${output}")
  endif()

  file(MAKE_DIRECTORY "${work_dir}/missing-platform")
  execute_process(
    COMMAND "${CMAKE_COMMAND}" "-DROOT=${work_dir}/missing-platform"
      -P "${CMAKE_CURRENT_LIST_DIR}/hash_inventory.cmake"
    RESULT_VARIABLE missing_result
    OUTPUT_VARIABLE missing_stdout
    ERROR_VARIABLE missing_stderr)
  set(missing_output "${missing_stdout}${missing_stderr}")
  if("${missing_result}" STREQUAL "0" OR
     NOT missing_output MATCHES "expected 6 platform artifacts")
    message(FATAL_ERROR "missing-platform control failed: ${missing_output}")
  endif()
  message(STATUS "control rejected absent platform artifact: expected 6 platform artifacts")
endfunction()

function(phase24_write_perf_identity path run_id attempt candidate tested base)
  file(WRITE "${path}"
    "run_id=${run_id}\nrun_attempt=${attempt}\nbase_sha=${base}\nhead_sha=${tested}\n"
    "candidate_head_sha=${candidate}\nvalgrind_version=valgrind 3.21.0\ngcc_version=gcc 14.2.0\n")
endfunction()

function(phase24_make_artifact_control_fixture fixture_root tamper)
  file(REMOVE_RECURSE "${fixture_root}")
  file(MAKE_DIRECTORY "${fixture_root}/provenance/artifact-zips" "${fixture_root}/artifacts")
  set(specs)
  foreach(platform IN LISTS NESTURBATOR_PHASE20_PLATFORMS)
    string(REPLACE "-" ";" parts "${platform}")
    list(GET parts 0 os)
    list(GET parts 1 arch)
    list(APPEND specs
      "${NESTURBATOR_PHASE20_HASH_ARTIFACT_PREFIX}-${os}-${arch}|hashes-${platform}/hashes.txt"
      "${NESTURBATOR_PHASE20_ACCURACYCOIN_ARTIFACT_PREFIX}-${os}-${arch}-12345-1|build-${platform}/accuracycoin/payload.txt")
  endforeach()
  list(APPEND specs
    "${NESTURBATOR_PHASE20_PERF_ARTIFACT_PREFIX}-12345-1|perf/payload.txt"
    "vector-inventory|vector-inventory/payload.txt"
    "${NESTURBATOR_PHASE20_MARKER_ARTIFACT}|retroarch/payload.txt"
    "retroarch-e2e-frames|retroarch-e2e-frames/payload.txt")
  string(TIMESTAMP created "%Y-%m-%dT%H:%M:%SZ" UTC)
  set(expires "9999-12-31T23:59:59Z")
  set(artifact_rows "")
  set(file_rows "")
  set(artifact_json "{\"artifacts\":[")
  set(id 90001)
  foreach(spec IN LISTS specs)
    string(REPLACE "|" ";" parts "${spec}")
    list(GET parts 0 name)
    list(GET parts 1 relative)
    set(seed "${fixture_root}/seed-${id}")
    file(MAKE_DIRECTORY "${seed}" "${fixture_root}/provenance/artifact-zips")
    file(WRITE "${seed}/payload.txt" "control artifact ${id}\n")
    set(zip "${fixture_root}/provenance/artifact-zips/${id}.zip")
    file(ARCHIVE_CREATE OUTPUT "${zip}" PATHS payload.txt WORKING_DIRECTORY "${seed}" FORMAT zip)
    file(SHA256 "${zip}" zip_sha)
    file(SHA256 "${seed}/payload.txt" payload_sha)
    get_filename_component(normalized_parent "${fixture_root}/artifacts/${relative}" DIRECTORY)
    file(MAKE_DIRECTORY "${normalized_parent}")
    configure_file("${seed}/payload.txt" "${fixture_root}/artifacts/${relative}" COPYONLY)
    string(APPEND artifact_rows "${name}|${id}|sha256:${zip_sha}|${created}|${expires}\n")
    string(APPEND file_rows "${id}|payload.txt|${relative}|${payload_sha}\n")
    if(NOT artifact_json STREQUAL "{\"artifacts\":[")
      string(APPEND artifact_json ",")
    endif()
    string(APPEND artifact_json
      "{\"id\":${id},\"name\":\"${name}\",\"digest\":\"sha256:${zip_sha}\","
      "\"expired\":false,\"created_at\":\"${created}\",\"expires_at\":\"${expires}\","
      "\"workflow_run\":{\"id\":12345,\"run_attempt\":1}}")
    math(EXPR id "${id} + 1")
  endforeach()
  string(APPEND artifact_json "]}")
  file(WRITE "${fixture_root}/provenance/artifact-map.tsv" "${artifact_rows}")
  file(WRITE "${fixture_root}/provenance/artifact-files.tsv" "${file_rows}")
  file(WRITE "${fixture_root}/provenance/artifacts.json" "${artifact_json}")
  if(tamper)
    file(WRITE "${fixture_root}/artifacts/hashes-linux-x64/hashes.txt" "tampered\n")
  endif()
endfunction()

function(phase24_controls)
  if(NOT DEFINED BUS OR NOT EXISTS "${BUS}" OR IS_DIRECTORY "${BUS}")
    message(FATAL_ERROR "MODE=controls requires -DBUS=<built bus.unit executable>")
  endif()
  file(REMOVE_RECURSE "${root}/controls")
  file(MAKE_DIRECTORY "${root}/controls")

  execute_process(
    COMMAND "${BUS}" --dma-trace overflow-control
    RESULT_VARIABLE overflow_result
    OUTPUT_VARIABLE overflow_stdout
    ERROR_VARIABLE overflow_stderr)
  set(overflow_output "${overflow_stdout}${overflow_stderr}")
  string(REGEX MATCHALL "DMA trace overflow: retained window exceeded capacity of 32768 events"
    overflow_diagnostics "${overflow_output}")
  list(LENGTH overflow_diagnostics overflow_diagnostic_count)
  if("${overflow_result}" STREQUAL "0" OR NOT overflow_diagnostic_count EQUAL 1)
    message(FATAL_ERROR "forced-overflow control did not fail once with the explicit diagnostic: ${overflow_output}")
  endif()
  message(STATUS "control rejected a forced retained-trace overflow with one explicit diagnostic")

  set(archive_fixture "${root}/controls/artifact-map")
  phase24_make_artifact_control_fixture("${archive_fixture}/valid" FALSE)
  file(READ "${archive_fixture}/valid/provenance/artifacts.json" artifact_json)
  phase24_validate_artifact_map("${archive_fixture}/valid" 12345 1 "${artifact_json}" error)
  if(NOT error STREQUAL "")
    message(FATAL_ERROR "valid ZIP-bound artifact control was rejected: ${error}")
  endif()
  phase24_make_artifact_control_fixture("${archive_fixture}/tampered" TRUE)
  file(READ "${archive_fixture}/tampered/provenance/artifacts.json" artifact_json)
  phase24_validate_artifact_map("${archive_fixture}/tampered" 12345 1 "${artifact_json}" error)
  phase24_expect_rejection("normalized artifact bytes detached from immutable ZIP"
    "normalized file digest mismatch" "${error}")

  set(fixtures "${root}/controls/scoreboard")
  phase24_control_files("${fixtures}")
  set(NESTURBATOR_PHASE20_PROTECTED_MINIMUM 3)
  phase20_validate_scoreboard("${fixtures}/candidate-scoreboard.tsv"
    "${fixtures}/protected-scoreboard.txt" "${fixtures}/protected-metadata.txt"
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" error)
  if(NOT error STREQUAL "")
    message(FATAL_ERROR "valid control-only protected-key fixture was rejected: ${error}")
  endif()

  file(READ "${fixtures}/protected-metadata.txt" metadata)
  string(REPLACE "authority=protected-main" "authority=checkout-fallback" metadata "${metadata}")
  file(WRITE "${fixtures}/protected-metadata.txt" "${metadata}")
  phase20_validate_scoreboard("${fixtures}/candidate-scoreboard.tsv"
    "${fixtures}/protected-scoreboard.txt" "${fixtures}/protected-metadata.txt"
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" error)
  phase24_expect_rejection("checkout fallback authority" "identity mismatch" "${error}")

  phase24_control_files("${fixtures}")
  file(READ "${fixtures}/protected-metadata.txt" metadata)
  string(REPLACE "protected_revision=aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
    "protected_revision=bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb" metadata "${metadata}")
  file(WRITE "${fixtures}/protected-metadata.txt" "${metadata}")
  phase20_validate_scoreboard("${fixtures}/candidate-scoreboard.tsv"
    "${fixtures}/protected-scoreboard.txt" "${fixtures}/protected-metadata.txt"
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" error)
  phase24_expect_rejection("stale protected-main revision" "identity mismatch" "${error}")

  phase24_control_files("${fixtures}")
  file(STRINGS "${fixtures}/candidate-scoreboard.tsv" candidate_rows)
  list(REMOVE_ITEM candidate_rows
    "accuracycoin/DMC DMA + OAM DMA\tpass\t0x01\t46\t-")
  list(JOIN candidate_rows "\n" candidate_text)
  file(WRITE "${fixtures}/candidate-scoreboard.tsv" "${candidate_text}\n")
  phase20_validate_scoreboard("${fixtures}/candidate-scoreboard.tsv"
    "${fixtures}/protected-scoreboard.txt" "${fixtures}/protected-metadata.txt"
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" error)
  phase24_expect_rejection("lost protected-main PASS key" "protected key lost" "${error}")

  phase24_control_files("${fixtures}")
  file(STRINGS "${fixtures}/candidate-scoreboard.tsv" candidate_rows)
  list(REMOVE_ITEM candidate_rows
    "accuracycoin/INC $4014\tpass\t0x01\t46\t-")
  list(JOIN candidate_rows "\n" candidate_text)
  file(WRITE "${fixtures}/candidate-scoreboard.tsv" "${candidate_text}\n")
  phase20_validate_scoreboard("${fixtures}/candidate-scoreboard.tsv"
    "${fixtures}/protected-scoreboard.txt" "${fixtures}/protected-metadata.txt"
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" error)
  phase24_expect_rejection("missing Phase 24 DMA target key" "required target key" "${error}")

  set(perf_root "${root}/controls/stale-attempt")
  set(perf_dir "${perf_root}/artifacts/perf")
  file(MAKE_DIRECTORY "${perf_dir}")
  file(WRITE "${perf_dir}/summary.md" "control-only performance summary\n")
  phase24_write_perf_identity("${perf_dir}/identity.txt" 8123 2
    "cccccccccccccccccccccccccccccccccccccccc"
    "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa")
  phase20_validate_perf("${perf_root}" "${source_dir}" 8123 3
    "cccccccccccccccccccccccccccccccccccccccc"
    "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" error)
  phase24_expect_rejection("stale CI attempt identity" "identity does not bind" "${error}")

  phase24_write_perf_identity("${perf_dir}/identity.txt" 8123 2
    "cccccccccccccccccccccccccccccccccccccccc"
    "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa")
  phase20_validate_perf("${perf_root}" "${source_dir}" 8123 2
    "cccccccccccccccccccccccccccccccccccccccc"
    "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" error)
  phase24_expect_rejection("incomplete raw performance measurements" "raw measurement is missing" "${error}")

  phase24_run_hash_controls("${root}/controls/hash-inventory")
  message(STATUS "Phase 24 DMA gap evidence controls passed; all fixtures are synthetic control-only data")
endfunction()

function(phase24_json_string value out_value)
  string(REPLACE "\\" "\\\\" escaped "${value}")
  string(REPLACE "\"" "\\\"" escaped "${escaped}")
  string(REPLACE "\r" "\\r" escaped "${escaped}")
  string(REPLACE "\n" "\\n" escaped "${escaped}")
  set(${out_value} "\"${escaped}\"" PARENT_SCOPE)
endfunction()

function(phase24_expect_command command_label)
  execute_process(
    COMMAND ${ARGN}
    WORKING_DIRECTORY "${source_dir}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE stdout
    ERROR_VARIABLE stderr)
  if(NOT "${result}" STREQUAL "0")
    message(FATAL_ERROR "${command_label} failed (${result}): ${stdout}${stderr}")
  endif()
  string(STRIP "${stdout}" stdout)
  set(${command_label}_OUTPUT "${stdout}" PARENT_SCOPE)
endfunction()

function(phase24_snapshot out_head out_tree out_index out_worktree_digest out_dirty_paths_file)
  set(write_artifacts TRUE)
  if(ARGC GREATER 5)
    set(write_artifacts "${ARGV5}")
  endif()
  phase24_git(head rev-parse --verify HEAD^{commit})
  phase24_git(tree rev-parse --verify HEAD^{tree})
  phase24_git(branch branch --show-current)
  if(NOT branch MATCHES "^[A-Za-z0-9._/-]+$" OR branch STREQUAL "")
    message(FATAL_ERROR "current branch has an unsafe or empty name")
  endif()
  phase24_git(index_tree write-tree)
  execute_process(
    COMMAND git diff --cached --binary HEAD
    WORKING_DIRECTORY "${source_dir}"
    RESULT_VARIABLE index_diff_result
    OUTPUT_VARIABLE index_diff
    ERROR_VARIABLE index_diff_error)
  if(NOT "${index_diff_result}" STREQUAL "0")
    message(FATAL_ERROR "could not fingerprint the staged index: ${index_diff_error}")
  endif()
  string(SHA256 index_diff_sha "${index_diff}")
  execute_process(
    COMMAND git diff --binary HEAD
    WORKING_DIRECTORY "${source_dir}"
    RESULT_VARIABLE worktree_diff_result
    OUTPUT_VARIABLE worktree_diff
    ERROR_VARIABLE worktree_diff_error)
  if(NOT "${worktree_diff_result}" STREQUAL "0")
    message(FATAL_ERROR "could not fingerprint the tracked worktree: ${worktree_diff_error}")
  endif()
  string(SHA256 worktree_diff_sha "${worktree_diff}")
  phase24_git(status status --porcelain=v1 --untracked-files=all)
  phase24_git(untracked ls-files --others --exclude-standard)
  set(untracked_manifest "")
  string(REPLACE "\n" ";" untracked_paths "${untracked}")
  foreach(path IN LISTS untracked_paths)
    if(path STREQUAL "")
      continue()
    endif()
    if(path MATCHES "(^/|(^|/)\\.\\.(/|$)|[\r\n])")
      message(FATAL_ERROR "unsafe untracked worktree path in candidate snapshot")
    endif()
    if(NOT EXISTS "${source_dir}/${path}" OR IS_DIRECTORY "${source_dir}/${path}" OR
       IS_SYMLINK "${source_dir}/${path}")
      message(FATAL_ERROR "untracked path is missing or unsafe: ${path}")
    endif()
    file(SHA256 "${source_dir}/${path}" path_sha)
    string(APPEND untracked_manifest "${path}\t${path_sha}\n")
  endforeach()
  string(SHA256 untracked_sha "${untracked_manifest}")
  string(SHA256 worktree_digest "${status}\n${index_diff_sha}\n${worktree_diff_sha}\n${untracked_sha}\n")
  set(dirty_paths_file "${root}/worktree-status.txt")
  if(write_artifacts)
    file(WRITE "${dirty_paths_file}" "${status}\n")
    file(WRITE "${root}/untracked-worktree-files.tsv" "${untracked_manifest}")
    file(WRITE "${root}/worktree-fingerprint.txt"
      "index_tree=${index_tree}\nindex_diff_sha256=${index_diff_sha}\n"
      "tracked_worktree_diff_sha256=${worktree_diff_sha}\nuntracked_manifest_sha256=${untracked_sha}\n"
      "worktree_sha256=${worktree_digest}\n")
  endif()
  set(${out_head} "${head}" PARENT_SCOPE)
  set(${out_tree} "${tree}" PARENT_SCOPE)
  set(${out_index} "${index_tree}" PARENT_SCOPE)
  set(${out_worktree_digest} "${worktree_digest}" PARENT_SCOPE)
  set(${out_dirty_paths_file} "${dirty_paths_file}" PARENT_SCOPE)
endfunction()

function(phase24_post_request_guard out_guard)
  set(excluded_pathspecs)
  foreach(path IN LISTS PHASE24_POST_REQUEST_PATHS)
    list(APPEND excluded_pathspecs ":(exclude)${path}")
  endforeach()
  phase24_git(stable_index_diff diff --cached --binary HEAD -- . ${excluded_pathspecs})
  phase24_git(stable_worktree_diff diff --binary HEAD -- . ${excluded_pathspecs})
  phase24_git(untracked ls-files --others --exclude-standard)
  string(SHA256 stable_index_sha "${stable_index_diff}")
  string(SHA256 stable_worktree_sha "${stable_worktree_diff}")
  string(REPLACE "\r\n" "\n" untracked "${untracked}")
  string(REPLACE "\n" ";" untracked_paths "${untracked}")
  set(untracked_manifest "")
  foreach(path IN LISTS untracked_paths)
    if(path STREQUAL "" OR path IN_LIST PHASE24_POST_REQUEST_PATHS)
      continue()
    endif()
    if(NOT EXISTS "${source_dir}/${path}" OR IS_DIRECTORY "${source_dir}/${path}" OR
       IS_SYMLINK "${source_dir}/${path}")
      message(FATAL_ERROR "untracked path is missing or unsafe while computing request guard: ${path}")
    endif()
    file(SHA256 "${source_dir}/${path}" path_sha)
    string(APPEND untracked_manifest "${path}\t${path_sha}\n")
  endforeach()
  string(SHA256 stable_untracked_sha "${untracked_manifest}")
  string(SHA256 guard "${stable_index_sha}\n${stable_worktree_sha}\n${stable_untracked_sha}\n")
  set(${out_guard} "${guard}" PARENT_SCOPE)
endfunction()

function(phase24_prepare)
  file(MAKE_DIRECTORY "${root}")
  foreach(file_name IN ITEMS request.json request.md request.sha256 pr-body.md pr-metadata.json)
    if(EXISTS "${root}/${file_name}")
      message(FATAL_ERROR "prepared request already exists at ${root}/${file_name}; preserve it and use a fresh ROOT")
    endif()
  endforeach()

  phase24_git(repo_root rev-parse --show-toplevel)
  if(NOT repo_root STREQUAL source_dir)
    message(FATAL_ERROR "SOURCE_REF does not resolve to the Git repository root")
  endif()
  phase24_git(remote_url remote get-url origin)
  string(TOLOWER "${remote_url}" remote_url_lower)
  if(NOT remote_url_lower MATCHES "github\\.com[:/]s[z]theory/nesturbator(\\.git)?$")
    message(FATAL_ERROR "origin is not the authorized repository szTheory/nesturbator")
  endif()

  phase24_snapshot(candidate_sha candidate_tree index_tree worktree_sha dirty_paths_file)
  phase24_post_request_guard(post_request_guard)
  file(RELATIVE_PATH root_relative "${source_dir}" "${root}")
  set(pr_body_relative "${root_relative}/pr-body.md")
  phase24_lower_sha("${candidate_sha}" 40 valid_candidate)
  phase24_lower_sha("${candidate_tree}" 40 valid_candidate_tree)
  phase24_lower_sha("${index_tree}" 40 valid_index)
  if(NOT valid_candidate OR NOT valid_candidate_tree OR NOT valid_index)
    message(FATAL_ERROR "candidate, committed tree, or staged index identity is malformed")
  endif()
  phase24_git(branch branch --show-current)

  phase24_expect_command(main_sha_command gh api "repos/${NESTURBATOR_PHASE20_REPOSITORY}/branches/main" --jq .commit.sha)
  set(main_sha "${main_sha_command_OUTPUT}")
  phase24_lower_sha("${main_sha}" 40 valid_main)
  if(NOT valid_main)
    message(FATAL_ERROR "read-only GitHub main branch metadata returned an invalid SHA")
  endif()
  phase24_expect_command(ls_remote git ls-remote --heads origin "refs/heads/${branch}")
  set(remote_branch_sha "")
  if(NOT ls_remote_OUTPUT STREQUAL "")
    if(NOT ls_remote_OUTPUT MATCHES "^([0-9a-f]+)[ \t]+(.+)$")
      message(FATAL_ERROR "read-only remote branch metadata is malformed: ${ls_remote_OUTPUT}")
    endif()
    set(remote_ref "${CMAKE_MATCH_2}")
    phase24_lower_sha("${CMAKE_MATCH_1}" 40 valid_remote_branch)
    if(NOT valid_remote_branch OR NOT remote_ref STREQUAL "refs/heads/${branch}")
      message(FATAL_ERROR "read-only remote branch metadata is malformed: ${ls_remote_OUTPUT}")
    endif()
    set(remote_branch_sha "${CMAKE_MATCH_1}")
  endif()

  phase24_expect_command(pr_list_command gh pr list --repo "${NESTURBATOR_PHASE20_REPOSITORY}"
    --state all --head "szTheory:${branch}" --limit 100
    --json number,state,url,title,baseRefName,headRefName,headRefOid,baseRefOid,headRepositoryOwner)
  set(pr_list "${pr_list_command_OUTPUT}")
  string(JSON pr_count ERROR_VARIABLE pr_error LENGTH "${pr_list}")
  if(NOT pr_error STREQUAL "NOTFOUND" OR NOT pr_count MATCHES "^[0-9]+$")
    message(FATAL_ERROR "read-only PR metadata is not a JSON array: ${pr_error}")
  endif()
  if(pr_count GREATER 1)
    message(FATAL_ERROR "multiple pull requests use the candidate branch; prepare a scoped request manually")
  endif()
  set(existing_pr_number "")
  set(existing_pr_url "")
  set(existing_pr_head_sha "")
  set(existing_pr_base_sha "")
  set(existing_pr_state "none")
  if(pr_count EQUAL 1)
    string(JSON existing_pr_number GET "${pr_list}" 0 number)
    string(JSON existing_pr_state GET "${pr_list}" 0 state)
    string(JSON existing_pr_url GET "${pr_list}" 0 url)
    string(JSON existing_pr_base_ref GET "${pr_list}" 0 baseRefName)
    string(JSON existing_pr_head_ref GET "${pr_list}" 0 headRefName)
    string(JSON existing_pr_head_sha GET "${pr_list}" 0 headRefOid)
    string(JSON existing_pr_base_sha GET "${pr_list}" 0 baseRefOid)
    string(JSON existing_pr_owner GET "${pr_list}" 0 headRepositoryOwner login)
    if(NOT existing_pr_state STREQUAL "OPEN" OR
       NOT existing_pr_base_ref STREQUAL "main" OR
       NOT existing_pr_head_ref STREQUAL branch OR
       NOT existing_pr_owner STREQUAL "szTheory")
      message(FATAL_ERROR "existing pull request differs from the proposed main/branch request; no remote action is prepared")
    endif()
    if(NOT existing_pr_base_sha STREQUAL main_sha)
      message(FATAL_ERROR "existing PR base is stale against current main; the concrete request needs a new review")
    endif()
  endif()
  if(NOT remote_branch_sha STREQUAL "" AND NOT remote_branch_sha STREQUAL candidate_sha)
    execute_process(
      COMMAND git merge-base --is-ancestor "${remote_branch_sha}" "${candidate_sha}"
      WORKING_DIRECTORY "${source_dir}"
      RESULT_VARIABLE ancestor_result
      ERROR_VARIABLE ancestor_error)
    if(NOT "${ancestor_result}" STREQUAL "0")
      message(FATAL_ERROR "remote branch does not fast-forward to this candidate; no push request is prepared")
    endif()
  endif()
  if(NOT existing_pr_head_sha STREQUAL "" AND
     NOT remote_branch_sha STREQUAL "" AND
     NOT existing_pr_head_sha STREQUAL remote_branch_sha)
    message(FATAL_ERROR "open PR head and remote branch disagree; no push request is prepared")
  endif()

  file(READ "${source_dir}/.github/workflows/ci.yml" workflow_text)
  foreach(required_text IN ITEMS
      "pull_request:" "opened, synchronize, reopened, edited"
      "name: CI required" "vector-inventory:" "session-controls:" "asan:" "nofp:"
      "hash-equality:" "retroarch-e2e:" "perf:" "timeout-minutes: 39"
      "timeout-minutes: 15" "NESTURBATOR_ACCURACYCOIN_FULL=ON"
      "tests/cmake/prepare_scoreboard_baseline.cmake"
      [=[accuracycoin-evidence-${{ matrix.os }}-${{ matrix.arch }}-${{ github.run_id }}-${{ github.run_attempt }}]=]
      [=[hashes-${{ matrix.os }}-${{ matrix.arch }}]=]
      [=[phase20-perf-${{ github.run_id }}-${{ github.run_attempt }}]=]
      "- { runner: ubuntu-24.04, preset: ci, workflow: ci-build, os: linux, arch: x64 }"
      "- { runner: ubuntu-24.04-arm, preset: ci, workflow: ci-build, os: linux, arch: arm64 }"
      "- { runner: macos-15, preset: ci, workflow: ci-build, os: macos, arch: arm64 }"
      "- { runner: macos-15-intel, preset: ci, workflow: ci-build, os: macos, arch: x64 }"
      "- { runner: windows-2025, preset: ci-msvc, os: windows, arch: x64 }"
      "- { runner: windows-11-arm, preset: ci-msvc, os: windows, arch: arm64 }")
    string(FIND "${workflow_text}" "${required_text}" required_position)
    if(required_position EQUAL -1)
      message(FATAL_ERROR "current CI workflow no longer contains the planned selection or side effect: ${required_text}")
    endif()
  endforeach()
  file(SHA256 "${source_dir}/.github/workflows/ci.yml" workflow_sha)
  file(SHA256 "${source_dir}/tests/cmake/prepare_scoreboard_baseline.cmake" baseline_script_sha)

  set(build_platforms "linux-x64;linux-arm64;macos-x64;macos-arm64;windows-x64;windows-arm64")
  set(job_selection "hygiene;title;build:linux-x64;build:linux-arm64;build:macos-x64;build:macos-arm64;build:windows-x64;build:windows-arm64;vector-inventory;session-controls:ubuntu-24.04;session-controls:macos-15;asan;nofp:ubuntu-24.04;nofp:ubuntu-24.04-arm;hash-equality;retroarch-e2e;perf;CI required")
  set(artifact_selection "hashes-OS-ARCH;accuracycoin-evidence-OS-ARCH-RUN-ATTEMPT;phase20-perf-RUN-ATTEMPT;vector-inventory;retroarch-marker-progression;retroarch-e2e-frames")
  if(existing_pr_number STREQUAL "")
    set(event "opened")
    set(proposed_commands
      "git push origin HEAD:refs/heads/${branch}\ngh pr create --repo szTheory/nesturbator --base main --head ${branch} --title \"fix(dma): correct remaining NTSC DMA behavior\" --body-file build/phase24-gap/pr-body.md")
  else()
    set(event "synchronize")
    set(proposed_commands
      "git push origin HEAD:refs/heads/${branch}\nExisting PR: ${existing_pr_url}")
  endif()

  file(WRITE "${root}/pr-metadata.json" "${pr_list}\n")
  file(SHA256 "${root}/pr-metadata.json" pr_metadata_sha)
  file(SHA256 "${dirty_paths_file}" dirty_paths_sha)
  string(SHA256 proposed_commands_sha "${proposed_commands}")
  phase24_json_string("${branch}" branch_json)
  phase24_json_string("${candidate_sha}" candidate_json)
  phase24_json_string("${candidate_tree}" candidate_tree_json)
  phase24_json_string("${main_sha}" base_json)
  phase24_json_string("${index_tree}" index_json)
  phase24_json_string("${worktree_sha}" worktree_json)
  phase24_json_string("${post_request_guard}" post_request_guard_json)
  phase24_json_string("${workflow_sha}" workflow_json)
  phase24_json_string("${baseline_script_sha}" baseline_json)
  phase24_json_string("${pr_metadata_sha}" pr_metadata_json)
  phase24_json_string("${remote_branch_sha}" remote_branch_json)
  phase24_json_string("${proposed_commands}" proposed_commands_json)
  phase24_json_string("${job_selection}" job_selection_json)
  if(existing_pr_number STREQUAL "")
    set(pr_number_json "null")
    set(pr_url_json "null")
  else()
    set(pr_number_json "${existing_pr_number}")
    phase24_json_string("${existing_pr_url}" pr_url_json)
  endif()
  file(WRITE "${root}/request.json"
    "{\n  \"schema\": 1,\n  \"repository\": \"szTheory/nesturbator\",\n"
    "  \"candidate_head_sha\": ${candidate_json},\n  \"candidate_tree_sha\": ${candidate_tree_json},\n"
    "  \"branch\": ${branch_json},\n  \"base_branch\": \"main\",\n  \"base_sha\": ${base_json},\n"
    "  \"existing_pr_number\": ${pr_number_json},\n  \"existing_pr_url\": ${pr_url_json},\n"
    "  \"event\": \"${event}\",\n  \"expected_workflow\": \"CI\",\n"
    "  \"index_tree_sha\": ${index_json},\n  \"worktree_sha256\": ${worktree_json},\n"
    "  \"post_request_guard_sha256\": ${post_request_guard_json},\n"
    "  \"dirty_paths_file\": \"worktree-status.txt\",\n  \"dirty_paths_sha256\": \"${dirty_paths_sha}\",\n"
    "  \"remote_branch_sha\": ${remote_branch_json},\n  \"workflow_sha256\": ${workflow_json},\n"
    "  \"protected_baseline_script_sha256\": ${baseline_json},\n"
    "  \"pr_metadata_file\": \"pr-metadata.json\",\n  \"pr_metadata_sha256\": ${pr_metadata_json},\n"
    "  \"pr_body_file\": \"${pr_body_relative}\",\n"
    "  \"selected_job_instances\": ${job_selection_json},\n"
    "  \"proposed_commands\": ${proposed_commands_json},\n"
    "  \"build_platforms\": [\"linux-x64\",\"linux-arm64\",\"macos-x64\",\"macos-arm64\",\"windows-x64\",\"windows-arm64\"],\n"
    "  \"required_jobs\": [\"hygiene\",\"title\",\"build\",\"vector-inventory\",\"session-controls\",\"asan\",\"nofp\",\"hash-equality\",\"retroarch-e2e\",\"perf\",\"CI required\"],\n"
    "  \"campaign\": \"Linux x64 with NESTURBATOR_ACCURACYCOIN_FULL=ON\",\n"
    "  \"limits_minutes\": {\"build\":39,\"session_controls\":39,\"asan\":26,\"nofp\":2,\"hash_equality\":5,\"retroarch_e2e\":40,\"perf\":15,\"ci_required\":5,\"network_jobs\":5},\n"
    "  \"campaign_limits\": {\"pages\":22,\"navigation_frames_per_page\":5,\"execution_frames_per_row\":1800,\"total_frames\":41503,\"page_timeout_seconds\":15,\"collection_timeout_seconds\":10,\"campaign_ctest_timeout_seconds\":370,\"protected_page_timeout_seconds\":30,\"protected_max_pages\":22,\"protected_ctest_timeout_seconds\":760,\"ctest_workers\":4},\n"
    "  \"artifact_names\": [\"hashes-OS-ARCH\",\"accuracycoin-evidence-OS-ARCH-RUN-ATTEMPT\",\"phase20-perf-RUN-ATTEMPT\",\"vector-inventory\",\"retroarch-marker-progression\",\"retroarch-e2e-frames\"],\n"
    "  \"proposed_commands_sha256\": \"${proposed_commands_sha}\",\n"
    "  \"automatic_retry\": false,\n  \"remote_action_authorized\": false\n}\n")
  file(WRITE "${root}/pr-body.md"
    "## Summary\n\n"
    "- Make retained DMA trace overflow explicit and keep the Test9 final landing window complete.\n"
    "- Add offline controls for protected-key, platform-hash and performance evidence validation.\n"
    "- Correct the README's local-versus-hosted acceptance status.\n\n"
    "## Local verification\n\n"
    "- `cmake --build build/phase24-owned --target bus.unit`\n"
    "- `ctest --test-dir build/phase24-owned -R '^bus\\.unit$' --no-tests=error --timeout 30 --output-on-failure`\n"
    "- `build/phase24-owned/tests/bus.unit --dma-trace landing`\n"
    "- `cmake -DMODE=controls -DSOURCE_REF=. -DROOT=build/phase24-gap-controls -DBUS=build/phase24-owned/tests/bus.unit -P tests/cmake/phase24_dma_gap_evidence.cmake`\n\n"
    "## Evidence still required\n\n"
    "This PR run is requested to measure independent cold protected-main preservation, fresh current-candidate hosted results, all six frame/PCM/transition/native hash inventories, and the unchanged Cachegrind +1% gate. Local and synthetic control data do not satisfy those items.\n")
  file(SHA256 "${root}/pr-body.md" pr_body_sha)
  file(WRITE "${root}/request.md"
    "# Phase 24 prepared remote evidence request\n\n"
    "Candidate: `${candidate_sha}` (`${branch}`)\n"
    "Candidate tree: `${candidate_tree}`; current main base: `${main_sha}`\n"
    "Index tree: `${index_tree}`; initial worktree fingerprint: `${worktree_sha}`. The post-request guard excludes only this acceptance/state/evidence bookkeeping and still detects changes to every other tracked or untracked path.\n"
    "Dirty paths are recorded separately in `worktree-status.txt`; they are not staged or included in the candidate.\n\n"
    "Event: `${event}` on the existing CI pull-request path. The current workflow also runs on PR `edited` events; a title edit can cancel an in-progress PR run. No manual dispatch, force push, merge, release, or automatic retry is included.\n\n"
    "Jobs: `${job_selection}`. Build platforms: `${build_platforms}`. Linux x64 enables `NESTURBATOR_ACCURACYCOIN_FULL=ON`.\n\n"
    "Workflow presets: Linux and macOS builds use preset `ci` with workflow `ci-build`; Windows builds use `ci-msvc`. Linux x64 adds `NESTURBATOR_ACCURACYCOIN_FULL=ON`.\n\n"
    "Limits from the current workflow: build and session-controls 39 minutes, ASan 26, nofp 2, hash-equality 5, RetroArch E2E 40, perf 15, CI required 5, and hygiene/title/vector-inventory 5 each. The campaign remains 22 pages, 5 navigation frames per page, 1,800 execution frames per row, 41,503 total frames, 15-second page and 10-second collection caps, and a 370-second campaign CTest limit. Protected observation remains capped at 22 pages, 30 seconds per page and a 760-second setup CTest limit. The full CTest worker count remains four. No retries are authorized.\n\n"
    "Artifacts: `${artifact_selection}`. Cold protected-main preparation remains before candidate CTest configuration.\n\n"
    "Missing evidence: independent protected-main authority and every protected PASS observation; current-candidate hosted jobs and full campaign; six-platform full hash equality; real base_a/base_b/head Cachegrind receipts at 60/360 frames.\n\n"
    "Proposed action:\n\n```text\n${proposed_commands}\n```\n\n"
    "PR body: `${pr_body_relative}` (SHA-256 `${pr_body_sha}`).\n\n"
    "The request is prepared for review. It grants no remote authority by itself.\n")
  file(SHA256 "${root}/request.json" request_sha)
  file(WRITE "${root}/request.sha256" "${request_sha}  request.json\n")
  message(STATUS "prepared read-only remote request for ${candidate_sha}; no push, PR creation, or workflow trigger was performed")
endfunction()

function(phase24_validate_request)
  foreach(required_file IN ITEMS request.json request.md request.sha256 pr-body.md pr-metadata.json
      worktree-status.txt worktree-fingerprint.txt)
    if(NOT EXISTS "${root}/${required_file}" OR IS_DIRECTORY "${root}/${required_file}" OR
       IS_SYMLINK "${root}/${required_file}")
      message(FATAL_ERROR "prepared request is missing ${required_file}")
    endif()
  endforeach()
  file(SHA256 "${root}/request.json" actual_request_sha)
  file(STRINGS "${root}/request.sha256" stored_request_lines)
  if(NOT stored_request_lines MATCHES "^${actual_request_sha}  request\\.json$")
    message(FATAL_ERROR "prepared request digest does not match request.json")
  endif()
  file(READ "${root}/request.json" request_json)
  phase20_json_get(schema "${request_json}" schema)
  phase20_json_get(repository "${request_json}" repository)
  phase20_json_get(expected_head "${request_json}" candidate_head_sha)
  phase20_json_get(expected_tree "${request_json}" candidate_tree_sha)
  phase20_json_get(expected_branch "${request_json}" branch)
  phase20_json_get(expected_base "${request_json}" base_sha)
  phase20_json_get(request_event "${request_json}" event)
  phase20_json_get(expected_index "${request_json}" index_tree_sha)
  phase20_json_get(expected_worktree "${request_json}" worktree_sha256)
  phase20_json_get(expected_post_request_guard "${request_json}" post_request_guard_sha256)
  phase20_json_get(expected_dirty_sha "${request_json}" dirty_paths_sha256)
  phase20_json_get(expected_workflow_sha "${request_json}" workflow_sha256)
  phase20_json_get(expected_baseline_sha "${request_json}" protected_baseline_script_sha256)
  phase20_json_get(expected_pr_metadata_sha "${request_json}" pr_metadata_sha256)
  phase20_json_get(expected_remote_branch "${request_json}" remote_branch_sha)
  phase20_json_get(proposed_commands "${request_json}" proposed_commands)
  phase20_json_get(expected_commands_sha "${request_json}" proposed_commands_sha256)
  phase20_json_get(expected_body_path "${request_json}" pr_body_file)
  phase20_json_get(existing_pr_url "${request_json}" existing_pr_url)
  phase20_json_get(remote_authorized "${request_json}" remote_action_authorized)
  phase20_json_get(automatic_retry "${request_json}" automatic_retry)
  phase24_lower_sha("${expected_head}" 40 valid_expected_head)
  phase24_lower_sha("${expected_tree}" 40 valid_expected_tree)
  phase24_lower_sha("${expected_index}" 40 valid_expected_index)
  phase24_lower_sha("${expected_base}" 40 valid_expected_base)
  phase24_lower_sha("${expected_worktree}" 64 valid_expected_worktree)
  if(NOT schema STREQUAL "1" OR NOT repository STREQUAL NESTURBATOR_PHASE20_REPOSITORY OR
     NOT valid_expected_head OR NOT valid_expected_tree OR NOT valid_expected_index OR
     NOT valid_expected_base OR NOT valid_expected_worktree)
    message(FATAL_ERROR "prepared request has malformed schema or candidate identity")
  endif()
  phase24_git(actual_head rev-parse --verify HEAD^{commit})
  phase24_git(actual_tree rev-parse --verify HEAD^{tree})
  phase24_git(actual_branch branch --show-current)
  if(NOT actual_head STREQUAL expected_head OR NOT actual_tree STREQUAL expected_tree OR
     NOT actual_branch STREQUAL expected_branch)
    message(FATAL_ERROR "prepared request candidate binding is stale")
  endif()
  phase24_snapshot(snapshot_head snapshot_tree snapshot_index snapshot_worktree snapshot_dirty_file FALSE)
  file(SHA256 "${snapshot_dirty_file}" actual_dirty_sha)
  phase24_post_request_guard(actual_post_request_guard)
  phase24_lower_sha("${expected_dirty_sha}" 64 valid_expected_dirty)
  phase24_lower_sha("${expected_post_request_guard}" 64 valid_expected_post_request_guard)
  phase20_read_properties("${root}/worktree-fingerprint.txt" initial valid valid_fingerprint_error)
  if(NOT snapshot_head STREQUAL expected_head OR NOT snapshot_tree STREQUAL expected_tree OR
     NOT snapshot_index STREQUAL expected_index OR
     NOT valid_expected_dirty OR NOT actual_dirty_sha STREQUAL expected_dirty_sha OR
     NOT valid_expected_post_request_guard OR
     NOT actual_post_request_guard STREQUAL expected_post_request_guard OR
     NOT valid OR NOT valid_fingerprint_error STREQUAL "" OR
     NOT initial_worktree_sha256 STREQUAL expected_worktree)
    message(FATAL_ERROR "prepared request index/worktree snapshot is stale")
  endif()
  file(SHA256 "${source_dir}/.github/workflows/ci.yml" actual_workflow_sha)
  if(NOT actual_workflow_sha STREQUAL expected_workflow_sha)
    message(FATAL_ERROR "prepared request no longer matches the CI workflow selection")
  endif()
  file(SHA256 "${source_dir}/tests/cmake/prepare_scoreboard_baseline.cmake" actual_baseline_sha)
  file(SHA256 "${root}/pr-metadata.json" actual_pr_metadata_sha)
  string(SHA256 actual_commands_sha "${proposed_commands}")
  if(NOT actual_baseline_sha STREQUAL expected_baseline_sha OR
     NOT actual_pr_metadata_sha STREQUAL expected_pr_metadata_sha OR
     NOT actual_commands_sha STREQUAL expected_commands_sha OR
     NOT remote_authorized STREQUAL "false" OR NOT automatic_retry STREQUAL "false" OR
     proposed_commands MATCHES "force|workflow_dispatch|gh pr merge|gh release")
    message(FATAL_ERROR "prepared request metadata, command selection, or cold-baseline source is stale")
  endif()
  file(RELATIVE_PATH actual_body_path "${source_dir}" "${root}/pr-body.md")
  if(NOT expected_body_path STREQUAL actual_body_path)
    message(FATAL_ERROR "prepared request PR body path does not resolve inside its evidence ROOT")
  endif()
  string(JSON existing_pr_number_type ERROR_VARIABLE pr_number_error TYPE "${request_json}" existing_pr_number)
  if(NOT pr_number_error STREQUAL "NOTFOUND")
    message(FATAL_ERROR "prepared request existing PR identity is malformed")
  endif()
  set(push_command "git push origin HEAD:refs/heads/${expected_branch}")
  if(existing_pr_number_type STREQUAL "NULL")
    set(expected_event "opened")
    set(expected_commands
      "${push_command}\ngh pr create --repo szTheory/nesturbator --base main --head ${expected_branch} --title \"fix(dma): correct remaining NTSC DMA behavior\" --body-file ${expected_body_path}")
  elseif(existing_pr_number_type STREQUAL "NUMBER")
    set(expected_event "synchronize")
    set(expected_commands "${push_command}\nExisting PR: ${existing_pr_url}")
  else()
    message(FATAL_ERROR "prepared request existing PR identity has an unsupported JSON type")
  endif()
  if(NOT expected_event STREQUAL request_event OR NOT proposed_commands STREQUAL expected_commands)
    message(FATAL_ERROR "prepared request event or proposed action differs from the exact push/PR path")
  endif()

  phase24_git(remote_url remote get-url origin)
  string(TOLOWER "${remote_url}" remote_url_lower)
  if(NOT remote_url_lower MATCHES "github\\.com[:/]s[z]theory/nesturbator(\\.git)?$")
    message(FATAL_ERROR "origin changed away from the prepared repository")
  endif()
  phase24_expect_command(current_main_command gh api "repos/${NESTURBATOR_PHASE20_REPOSITORY}/branches/main" --jq .commit.sha)
  if(NOT current_main_command_OUTPUT STREQUAL expected_base)
    message(FATAL_ERROR "prepared request base is stale against current main")
  endif()
  phase24_expect_command(current_branch_command git ls-remote --heads origin "refs/heads/${expected_branch}")
  if(NOT current_branch_command_OUTPUT STREQUAL "")
    if(NOT current_branch_command_OUTPUT MATCHES "^([0-9a-f]+)[ \t]+refs/heads/${expected_branch}$")
      message(FATAL_ERROR "current candidate remote branch metadata is malformed")
    endif()
    set(current_remote_sha "${CMAKE_MATCH_1}")
  else()
    set(current_remote_sha "")
  endif()
  if(NOT current_remote_sha STREQUAL expected_remote_branch)
    message(FATAL_ERROR "prepared request remote branch changed since read-only preparation")
  endif()
  phase24_expect_command(current_pr_command gh pr list --repo "${NESTURBATOR_PHASE20_REPOSITORY}"
    --state all --head "szTheory:${expected_branch}" --limit 100
    --json number,state,url,title,baseRefName,headRefName,headRefOid,baseRefOid,headRepositoryOwner)
  file(READ "${root}/pr-metadata.json" prepared_pr_metadata)
  string(STRIP "${prepared_pr_metadata}" prepared_pr_metadata)
  if(NOT current_pr_command_OUTPUT STREQUAL prepared_pr_metadata)
    message(FATAL_ERROR "current pull request metadata differs from the prepared request")
  endif()
  file(SHA256 "${root}/pr-body.md" actual_body_sha)
  file(READ "${root}/request.md" request_markdown)
  string(FIND "${request_markdown}" "PR body: `${expected_body_path}` (SHA-256 `${actual_body_sha}`)" body_reference_position)
  if(body_reference_position EQUAL -1)
    message(FATAL_ERROR "prepared request does not bind the exact reviewable PR body")
  endif()
  string(FIND "${request_markdown}" "${proposed_commands}" command_position)
  if(command_position EQUAL -1)
    message(FATAL_ERROR "prepared request markdown omits the exact proposed commands")
  endif()
  message(STATUS "prepared request and committed candidate binding validated; no authority was granted")
endfunction()

function(phase24_find_tree_entry tree_json path out_sha out_mode out_error)
  phase20_json_get(tree_count "${tree_json}" tree LENGTH)
  if(NOT tree_count MATCHES "^[0-9]+$")
    set(${out_error} "GitHub tree response is malformed" PARENT_SCOPE)
    return()
  endif()
  set(found 0)
  if(tree_count GREATER 0)
    math(EXPR last "${tree_count} - 1")
    foreach(index RANGE 0 ${last})
      phase20_json_get(entry_path "${tree_json}" tree ${index} path)
      if(entry_path STREQUAL path)
        math(EXPR found "${found} + 1")
        phase20_json_get(entry_type "${tree_json}" tree ${index} type)
        phase20_json_get(entry_sha "${tree_json}" tree ${index} sha)
        phase20_json_get(entry_mode "${tree_json}" tree ${index} mode)
      endif()
    endforeach()
  endif()
  phase24_lower_sha("${entry_sha}" 40 valid_entry_sha)
  if(NOT found EQUAL 1 OR NOT entry_type STREQUAL "blob" OR
     NOT valid_entry_sha OR NOT entry_mode MATCHES "^100[0-7][0-7][0-7]$")
    set(${out_error} "GitHub tree has missing/duplicate/unsafe blob path ${path}" PARENT_SCOPE)
    return()
  endif()
  set(${out_sha} "${entry_sha}" PARENT_SCOPE)
  set(${out_mode} "${entry_mode}" PARENT_SCOPE)
  set(${out_error} "" PARENT_SCOPE)
endfunction()

function(phase24_validate_artifact_map root run_id attempt artifact_json out_error)
  set(map_path "${root}/provenance/artifact-map.tsv")
  set(file_map_path "${root}/provenance/artifact-files.tsv")
  set(extract_root "${root}/archive-check")
  if(EXISTS "${extract_root}" OR IS_SYMLINK "${extract_root}")
    set(${out_error} "archive verification scratch path already exists; preserve the receipt and use a fresh ROOT" PARENT_SCOPE)
    return()
  endif()
  foreach(path IN ITEMS "${map_path}" "${file_map_path}")
    if(NOT EXISTS "${path}" OR IS_DIRECTORY "${path}" OR IS_SYMLINK "${path}")
      set(${out_error} "immutable artifact ID/digest or downloaded-file map is missing" PARENT_SCOPE)
      return()
    endif()
  endforeach()
  file(STRINGS "${map_path}" artifact_rows)
  set(names)
  set(ids)
  foreach(row IN LISTS artifact_rows)
    if(NOT row MATCHES "^([^|]+)\\|([0-9]+)\\|sha256:([0-9a-f]+)\\|([^|]+)\\|([^|]+)$")
      set(${out_error} "artifact ID/digest row is malformed" PARENT_SCOPE)
      return()
    endif()
    set(name "${CMAKE_MATCH_1}")
    set(id "${CMAKE_MATCH_2}")
    set(digest "${CMAKE_MATCH_3}")
    set(created "${CMAKE_MATCH_4}")
    set(expires "${CMAKE_MATCH_5}")
    phase24_lower_sha("${digest}" 64 valid_digest)
    if(NOT valid_digest)
      set(${out_error} "artifact SHA-256 digest has the wrong length or case" PARENT_SCOPE)
      return()
    endif()
    if(name IN_LIST names OR id IN_LIST ids OR
       NOT created MATCHES "^[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]T[0-9][0-9]:[0-9][0-9]:[0-9][0-9]Z$" OR
       NOT expires MATCHES "^[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]T[0-9][0-9]:[0-9][0-9]:[0-9][0-9]Z$")
      set(${out_error} "artifact name/ID is duplicated or timestamps are malformed" PARENT_SCOPE)
      return()
    endif()
    if(expires STRLESS_EQUAL "${_PHASE24_NOW}")
      set(${out_error} "required artifact expired: ${name}" PARENT_SCOPE)
      return()
    endif()
    set(api_match 0)
    phase20_json_get(api_count "${artifact_json}" artifacts LENGTH)
    if(api_count GREATER 0)
      math(EXPR api_last "${api_count} - 1")
      foreach(index RANGE 0 ${api_last})
        phase20_json_get(api_id "${artifact_json}" artifacts ${index} id)
        if(api_id STREQUAL id)
          math(EXPR api_match "${api_match} + 1")
          phase20_json_get(api_name "${artifact_json}" artifacts ${index} name)
          phase20_json_get(api_digest "${artifact_json}" artifacts ${index} digest)
          phase20_json_get(api_expired "${artifact_json}" artifacts ${index} expired)
          phase20_json_get(api_created "${artifact_json}" artifacts ${index} created_at)
          phase20_json_get(api_expires "${artifact_json}" artifacts ${index} expires_at)
          phase20_json_get(api_run "${artifact_json}" artifacts ${index} workflow_run id)
          phase20_json_get(api_attempt "${artifact_json}" artifacts ${index} workflow_run run_attempt)
        endif()
      endforeach()
    endif()
    if(NOT api_match EQUAL 1 OR NOT api_name STREQUAL name OR NOT api_digest STREQUAL "sha256:${digest}" OR
       NOT api_expired STREQUAL "false" OR NOT api_created STREQUAL created OR
       NOT api_expires STREQUAL expires OR NOT api_run STREQUAL run_id OR
       NOT api_attempt STREQUAL attempt)
      set(${out_error} "artifact ${name} does not match one unexpired API artifact from this run/attempt" PARENT_SCOPE)
      return()
    endif()
    set(zip "${root}/provenance/artifact-zips/${id}.zip")
    if(NOT EXISTS "${zip}" OR IS_DIRECTORY "${zip}" OR IS_SYMLINK "${zip}")
      set(${out_error} "downloaded immutable artifact ${id} is missing or unsafe" PARENT_SCOPE)
      return()
    endif()
    file(SHA256 "${zip}" zip_sha)
    if(NOT zip_sha STREQUAL digest)
      set(${out_error} "downloaded artifact ${id} digest differs from its immutable API digest" PARENT_SCOPE)
      return()
    endif()
    list(APPEND names "${name}")
    list(APPEND ids "${id}")
  endforeach()
  set(required_names vector-inventory retroarch-e2e-frames retroarch-marker-progression
      "${NESTURBATOR_PHASE20_PERF_ARTIFACT_PREFIX}-${run_id}-${attempt}")
  foreach(platform IN LISTS NESTURBATOR_PHASE20_PLATFORMS)
    string(REPLACE "-" ";" parts "${platform}")
    list(GET parts 0 os)
    list(GET parts 1 arch)
    list(APPEND required_names "${NESTURBATOR_PHASE20_HASH_ARTIFACT_PREFIX}-${os}-${arch}"
      "${NESTURBATOR_PHASE20_ACCURACYCOIN_ARTIFACT_PREFIX}-${os}-${arch}-${run_id}-${attempt}")
  endforeach()
  foreach(required_name IN LISTS required_names)
    if(NOT required_name IN_LIST names)
      set(${out_error} "required artifact mapping is absent: ${required_name}" PARENT_SCOPE)
      return()
    endif()
  endforeach()
  file(MAKE_DIRECTORY "${extract_root}")
  foreach(id IN LISTS ids)
    set(zip "${root}/provenance/artifact-zips/${id}.zip")
    set(destination "${extract_root}/${id}")
    file(ARCHIVE_EXTRACT INPUT "${zip}" DESTINATION "${destination}")
  endforeach()
  file(STRINGS "${file_map_path}" file_rows)
  set(file_paths)
  set(archive_path_keys)
  foreach(row IN LISTS file_rows)
    if(NOT row MATCHES "^([0-9]+)\\|([^|]+)\\|([^|]+)\\|([0-9a-f]+)$")
      set(${out_error} "downloaded artifact file mapping is malformed" PARENT_SCOPE)
      return()
    endif()
    set(id "${CMAKE_MATCH_1}")
    set(archive_relative "${CMAKE_MATCH_2}")
    set(relative "${CMAKE_MATCH_3}")
    set(expected_sha "${CMAKE_MATCH_4}")
    phase24_lower_sha("${expected_sha}" 64 valid_file_sha)
    if(NOT valid_file_sha)
      set(${out_error} "downloaded artifact SHA-256 has the wrong length or case" PARENT_SCOPE)
      return()
    endif()
    if(NOT id IN_LIST ids OR archive_relative STREQUAL "" OR relative STREQUAL "" OR
       archive_relative MATCHES "^/|^[A-Za-z]:|(^|/)\\.\\.(/|$)|\\\\|;" OR
       relative MATCHES "^/|^[A-Za-z]:|(^|/)\\.\\.(/|$)|\\\\|;" OR
       relative IN_LIST file_paths)
      set(${out_error} "downloaded artifact path is unsafe, duplicate, or bound to an unknown artifact" PARENT_SCOPE)
      return()
    endif()
    set(archive_path_key "${id}|${archive_relative}")
    if(archive_path_key IN_LIST archive_path_keys)
      set(${out_error} "downloaded archive member is mapped more than once: ${archive_relative}" PARENT_SCOPE)
      return()
    endif()
    set(owner_name "")
    if(relative MATCHES "^hashes-([a-z]+-[a-z0-9]+)/")
      set(owner_name "${NESTURBATOR_PHASE20_HASH_ARTIFACT_PREFIX}-${CMAKE_MATCH_1}")
    elseif(relative MATCHES "^build-([a-z]+-[a-z0-9]+)/")
      string(REPLACE "-" ";" parts "${CMAKE_MATCH_1}")
      list(GET parts 0 os)
      list(GET parts 1 arch)
      set(owner_name "${NESTURBATOR_PHASE20_ACCURACYCOIN_ARTIFACT_PREFIX}-${os}-${arch}-${run_id}-${attempt}")
    elseif(relative MATCHES "^perf/")
      set(owner_name "${NESTURBATOR_PHASE20_PERF_ARTIFACT_PREFIX}-${run_id}-${attempt}")
    elseif(relative MATCHES "^retroarch/")
      set(owner_name "${NESTURBATOR_PHASE20_MARKER_ARTIFACT}")
    elseif(relative MATCHES "^retroarch-e2e-frames/")
      set(owner_name "retroarch-e2e-frames")
    elseif(relative MATCHES "^vector-inventory/")
      set(owner_name "vector-inventory")
    endif()
    if(owner_name STREQUAL "")
      set(${out_error} "downloaded artifact file is outside the selected artifact layout: ${relative}" PARENT_SCOPE)
      return()
    endif()
    list(FIND names "${owner_name}" owner_index)
    if(owner_index LESS 0)
      set(${out_error} "downloaded file has no selected owning artifact: ${relative}" PARENT_SCOPE)
      return()
    endif()
    list(GET ids ${owner_index} owner_id)
    if(NOT owner_id STREQUAL id)
      set(${out_error} "downloaded file is attributed to the wrong immutable artifact: ${relative}" PARENT_SCOPE)
      return()
    endif()
    set(archive_file "${extract_root}/${id}/${archive_relative}")
    if(NOT EXISTS "${archive_file}" OR IS_DIRECTORY "${archive_file}" OR IS_SYMLINK "${archive_file}")
      set(${out_error} "immutable artifact archive is missing mapped member or contains an unsafe member: ${archive_relative}" PARENT_SCOPE)
      return()
    endif()
    set(path "${root}/artifacts/${relative}")
    if(NOT EXISTS "${path}" OR IS_DIRECTORY "${path}" OR IS_SYMLINK "${path}")
      set(${out_error} "downloaded artifact file is missing or unsafe: ${relative}" PARENT_SCOPE)
      return()
    endif()
    file(SHA256 "${archive_file}" archive_sha)
    file(SHA256 "${path}" actual_sha)
    if(NOT archive_sha STREQUAL expected_sha OR NOT actual_sha STREQUAL archive_sha)
      set(${out_error} "downloaded artifact member or normalized file digest mismatch: ${relative}" PARENT_SCOPE)
      return()
    endif()
    list(APPEND file_paths "${relative}")
    list(APPEND archive_path_keys "${archive_path_key}")
  endforeach()
  set(extracted_archive_keys)
  foreach(id IN LISTS ids)
    file(GLOB_RECURSE archive_files LIST_DIRECTORIES FALSE "${extract_root}/${id}/*")
    foreach(path IN LISTS archive_files)
      if(IS_SYMLINK "${path}" OR IS_DIRECTORY "${path}")
        set(${out_error} "immutable artifact extraction contains an unsafe entry: ${path}" PARENT_SCOPE)
        return()
      endif()
      file(RELATIVE_PATH archive_relative "${extract_root}/${id}" "${path}")
      list(APPEND extracted_archive_keys "${id}|${archive_relative}")
    endforeach()
  endforeach()
  list(SORT extracted_archive_keys)
  list(SORT archive_path_keys)
  if(NOT extracted_archive_keys STREQUAL archive_path_keys)
    set(${out_error} "downloaded-file map does not cover the exact contents of the immutable artifact archives" PARENT_SCOPE)
    return()
  endif()
  file(GLOB_RECURSE extracted_files LIST_DIRECTORIES FALSE "${root}/artifacts/*")
  set(extracted_paths)
  foreach(path IN LISTS extracted_files)
    if(IS_SYMLINK "${path}")
      set(${out_error} "downloaded artifact tree contains a symlink: ${path}" PARENT_SCOPE)
      return()
    endif()
    file(RELATIVE_PATH relative "${root}/artifacts" "${path}")
    list(APPEND extracted_paths "${relative}")
  endforeach()
  list(SORT extracted_paths)
  list(SORT file_paths)
  if(NOT extracted_paths STREQUAL file_paths)
    set(${out_error} "downloaded artifact file map is incomplete or contains files outside the extracted set" PARENT_SCOPE)
    return()
  endif()
  file(REMOVE_RECURSE "${extract_root}")
  set(${out_error} "" PARENT_SCOPE)
endfunction()

function(phase24_validate_prepare_step root platform run_id attempt out_error)
  set(mapping_path "${root}/provenance/build-${platform}-job.txt")
  set(job_path "${root}/provenance/build-${platform}-job.json")
  set(raw_log "${root}/downloads/job-${platform}.txt")
  phase20_read_properties("${mapping_path}" mapping valid mapping_error)
  phase20_expected_build_job_name("${platform}" expected_job expected_run_step)
  phase20_validate_build_job_metadata("${job_path}" "${platform}" "${run_id}" "${attempt}"
    run_step_number run_error run_started run_completed)
  if(NOT mapping_error STREQUAL "" OR NOT run_error STREQUAL "" OR
     NOT EXISTS "${raw_log}" OR IS_DIRECTORY "${raw_log}" OR IS_SYMLINK "${raw_log}")
    set(${out_error} "${platform} cold-baseline preparation receipt or build log is missing: ${mapping_error}${run_error}" PARENT_SCOPE)
    return()
  endif()
  file(SHA256 "${raw_log}" actual_raw_sha)
  if(NOT actual_raw_sha STREQUAL mapping_raw_job_log_sha256 OR
     NOT mapping_job_name STREQUAL expected_job OR
     NOT mapping_run_id STREQUAL run_id OR NOT mapping_run_attempt STREQUAL attempt)
    set(${out_error} "${platform} preparation raw log is not bound to its selected build job/attempt" PARENT_SCOPE)
    return()
  endif()

  file(READ "${job_path}" job_json)
  phase20_json_get(step_count "${job_json}" steps LENGTH)
  set(prepare_count 0)
  if(step_count GREATER 0)
    math(EXPR last_step "${step_count} - 1")
    foreach(index RANGE 0 ${last_step})
      phase20_json_get(step_name "${job_json}" steps ${index} name)
      if(step_name STREQUAL "Prepare protected-main scoreboard baseline")
        math(EXPR prepare_count "${prepare_count} + 1")
        phase20_json_get(prepare_number "${job_json}" steps ${index} number)
        phase20_json_get(prepare_status "${job_json}" steps ${index} status)
        phase20_json_get(prepare_conclusion "${job_json}" steps ${index} conclusion)
        phase20_json_get(prepare_started "${job_json}" steps ${index} started_at)
        phase20_json_get(prepare_completed "${job_json}" steps ${index} completed_at)
      endif()
    endforeach()
  endif()
  phase20_iso_utc_second("${prepare_started}" prepare_start_second timestamp_error)
  if(NOT timestamp_error STREQUAL "")
    set(${out_error} "${platform} cold-baseline step ${timestamp_error}" PARENT_SCOPE)
    return()
  endif()
  phase20_iso_utc_second("${prepare_completed}" prepare_end_second timestamp_error)
  phase20_iso_utc_second("${run_started}" run_start_second run_time_error)
  if(prepare_count NOT EQUAL 1 OR NOT timestamp_error STREQUAL "" OR
     NOT run_time_error STREQUAL "" OR NOT prepare_number MATCHES "^[0-9]+$" OR
     NOT prepare_status STREQUAL "completed" OR NOT prepare_conclusion STREQUAL "success" OR
     NOT prepare_number LESS run_step_number OR prepare_start_second STRLESS prepare_end_second OR
     prepare_end_second STRGREATER run_start_second)
    set(${out_error} "${platform} cold-baseline step was absent, duplicated, failed, or ran after candidate CTest" PARENT_SCOPE)
    return()
  endif()

  file(READ "${raw_log}" raw_text)
  string(FIND "${raw_text}" "##[group]Prepare protected-main scoreboard baseline" prepare_group)
  string(FIND "${raw_text}" "Run cmake -P tests/cmake/prepare_scoreboard_baseline.cmake" prepare_command)
  if(prepare_group EQUAL -1 OR prepare_command EQUAL -1 OR prepare_command LESS prepare_group)
    set(${out_error} "${platform} raw job log does not retain the successful cold-baseline command group" PARENT_SCOPE)
    return()
  endif()
  set(${out_error} "" PARENT_SCOPE)
endfunction()

function(phase24_validate_prepare_contract request_json out_error)
  phase20_json_get(expected_workflow_sha "${request_json}" workflow_sha256)
  phase20_json_get(expected_prepare_sha "${request_json}" protected_baseline_script_sha256)
  file(SHA256 "${source_dir}/.github/workflows/ci.yml" workflow_sha)
  file(SHA256 "${source_dir}/tests/cmake/prepare_scoreboard_baseline.cmake" prepare_sha)
  if(NOT workflow_sha STREQUAL expected_workflow_sha OR NOT prepare_sha STREQUAL expected_prepare_sha)
    set(${out_error} "cold-baseline workflow or prepare script differs from the prepared candidate" PARENT_SCOPE)
    return()
  endif()
  file(READ "${source_dir}/.github/workflows/ci.yml" workflow_text)
  file(READ "${source_dir}/tests/cmake/prepare_scoreboard_baseline.cmake" prepare_text)
  string(FIND "${workflow_text}" "- name: Prepare protected-main scoreboard baseline" prepare_name)
  string(FIND "${workflow_text}" "run: cmake -P tests/cmake/prepare_scoreboard_baseline.cmake" prepare_run)
  string(FIND "${workflow_text}" "run: cmake --workflow --preset" ctest_run)
  string(FIND "${prepare_text}" "git fetch --no-tags" fetch_source)
  foreach(variable IN ITEMS NESTURBATOR_SCOREBOARD_BASELINE
      NESTURBATOR_SCOREBOARD_BASELINE_METADATA NESTURBATOR_SCOREBOARD_BASELINE_REVISION)
    string(FIND "${prepare_text}" "${variable}=" handoff_position)
    if(handoff_position EQUAL -1)
      set(${out_error} "protected-main preparation does not write ${variable}" PARENT_SCOPE)
      return()
    endif()
  endforeach()
  if(prepare_name EQUAL -1 OR prepare_run EQUAL -1 OR ctest_run EQUAL -1 OR
     fetch_source EQUAL -1 OR NOT prepare_name LESS prepare_run OR NOT prepare_run LESS ctest_run)
    set(${out_error} "workflow does not fetch protected main and prepare its baseline before candidate CTest" PARENT_SCOPE)
    return()
  endif()
  set(${out_error} "" PARENT_SCOPE)
endfunction()

function(phase24_validate_platform_baseline root platform base_sha scoreboard_sha expected_rows out_error)
  foreach(file IN ITEMS
      "${root}/artifacts/build-${platform}/protected-main/scoreboard.txt"
      "${root}/artifacts/build-${platform}/protected-main/metadata.txt"
      "${root}/artifacts/build-${platform}/LastTest.log")
    if(NOT EXISTS "${file}" OR IS_DIRECTORY "${file}" OR IS_SYMLINK "${file}")
      set(${out_error} "${platform} cold protected-main artifact or executed test log is missing: ${file}" PARENT_SCOPE)
      return()
    endif()
  endforeach()
  phase20_read_properties("${root}/artifacts/build-${platform}/protected-main/metadata.txt"
    lane valid metadata_error)
  file(SHA256 "${root}/artifacts/build-${platform}/protected-main/scoreboard.txt" lane_scoreboard_sha)
  file(STRINGS "${root}/artifacts/build-${platform}/protected-main/scoreboard.txt" lane_rows)
  list(LENGTH lane_rows lane_row_count)
  if(NOT metadata_error STREQUAL "" OR NOT lane_authority STREQUAL "protected-main" OR
     NOT lane_protected_revision STREQUAL base_sha OR
     NOT lane_scoreboard_sha256 STREQUAL scoreboard_sha OR
     NOT lane_scoreboard_sha STREQUAL scoreboard_sha OR
     NOT lane_row_count STREQUAL expected_rows OR
     NOT lane_scoreboard_rows STREQUAL expected_rows OR
     NOT lane_scoreboard_denominator STREQUAL expected_rows)
    set(${out_error} "${platform} protected scoreboard is not the complete independently fetched main baseline" PARENT_SCOPE)
    return()
  endif()
  set(${out_error} "" PARENT_SCOPE)
endfunction()

function(phase24_validate)
  if(NOT IS_DIRECTORY "${root}")
    message(FATAL_ERROR "MODE=validate requires a populated evidence ROOT")
  endif()
  foreach(output IN ITEMS "${root}/phase24-verdict.txt" "${root}/phase24-verdict.sha256")
    if(EXISTS "${output}")
      message(FATAL_ERROR "validation output already exists; use a new ROOT to preserve prior receipts")
    endif()
  endforeach()
  set(required
    "${root}/request.json"
    "${root}/request.sha256"
    "${root}/request.md"
    "${root}/pr-body.md"
    "${root}/pr-metadata.json"
    "${root}/delivery.json"
    "${root}/provenance/ci-run.json"
    "${root}/provenance/ci-jobs.json"
    "${root}/provenance/ci-artifacts.json"
    "${root}/provenance/pr.json"
    "${root}/provenance/candidate-commit.json"
    "${root}/provenance/tested-commit.json"
    "${root}/provenance/base-commit.json"
    "${root}/provenance/candidate-tree.json"
    "${root}/provenance/tested-tree.json"
    "${root}/provenance/base-tree.json"
    "${root}/provenance/protected-main-commit.json"
    "${root}/provenance/protected-main-tree.json"
    "${root}/provenance/protected-main-scoreboard.txt"
    "${root}/provenance/candidate-scoreboard.txt"
    "${root}/provenance/retrieval.md"
    "${root}/artifacts/build-linux-x64/protected-main/scoreboard.txt"
    "${root}/artifacts/build-linux-x64/protected-main/metadata.txt"
    "${root}/artifacts/build-linux-x64/accuracycoin/full/metadata.txt"
    "${root}/artifacts/build-linux-x64/accuracycoin/full/report.tsv"
    "${root}/artifacts/build-linux-x64/accuracycoin/full/inventory.tsv")
  foreach(path IN LISTS required)
    if(NOT EXISTS "${path}" OR IS_DIRECTORY "${path}" OR IS_SYMLINK "${path}")
      message(FATAL_ERROR "incomplete hosted evidence: missing or unsafe ${path}")
    endif()
  endforeach()
  file(SHA256 "${root}/request.json" request_digest)
  file(STRINGS "${root}/request.sha256" request_digest_rows)
  if(NOT request_digest_rows MATCHES "^${request_digest}  request\\.json$")
    message(FATAL_ERROR "prepared request receipt digest does not match request.json")
  endif()
  file(READ "${root}/request.json" request_json)
  file(READ "${root}/delivery.json" delivery)
  phase20_json_get(schema "${delivery}" schema)
  phase20_json_get(repository "${delivery}" repository)
  phase20_json_get(pr_number "${delivery}" pr_number)
  phase20_json_get(candidate_sha "${delivery}" candidate_head_sha)
  phase20_json_get(tested_sha "${delivery}" tested_sha)
  phase20_json_get(base_sha "${delivery}" base_sha)
  phase20_json_get(run_id "${delivery}" ci_run_id)
  phase20_json_get(attempt "${delivery}" ci_run_attempt)
  phase20_json_get(request_candidate "${request_json}" candidate_head_sha)
  phase20_json_get(request_base "${request_json}" base_sha)
  phase20_json_get(request_event "${request_json}" event)
  phase20_json_get(request_repository "${request_json}" repository)
  phase20_json_get(request_base_branch "${request_json}" base_branch)
  phase20_json_get(request_candidate_tree "${request_json}" candidate_tree_sha)
  phase20_json_get(request_workflow "${request_json}" expected_workflow)
  phase20_json_get(request_body_path "${request_json}" pr_body_file)
  phase20_json_get(request_pr_metadata_sha "${request_json}" pr_metadata_sha256)
  file(RELATIVE_PATH actual_body_path "${source_dir}" "${root}/pr-body.md")
  phase20_lower_hex("${candidate_sha}" 40 valid_candidate)
  phase20_lower_hex("${tested_sha}" 40 valid_tested)
  phase20_lower_hex("${base_sha}" 40 valid_base)
  if(NOT schema STREQUAL "1" OR NOT repository STREQUAL NESTURBATOR_PHASE20_REPOSITORY OR
     NOT pr_number MATCHES "^[1-9][0-9]*$" OR NOT run_id MATCHES "^[0-9]+$" OR
     NOT attempt MATCHES "^[1-9][0-9]*$" OR NOT valid_candidate OR NOT valid_tested OR NOT valid_base OR
     NOT request_repository STREQUAL NESTURBATOR_PHASE20_REPOSITORY OR
     NOT request_base_branch STREQUAL "main" OR NOT request_event MATCHES "^(opened|synchronize)$" OR
     NOT request_workflow STREQUAL "CI" OR
     NOT request_body_path STREQUAL actual_body_path OR
     NOT candidate_sha STREQUAL request_candidate OR NOT base_sha STREQUAL request_base OR
     candidate_sha STREQUAL tested_sha OR base_sha STREQUAL candidate_sha OR base_sha STREQUAL tested_sha)
    message(FATAL_ERROR "delivery identity is missing, stale, duplicated, or not bound to the prepared PR request")
  endif()
  phase24_validate_prepare_contract("${request_json}" error)
  if(NOT error STREQUAL "")
    message(FATAL_ERROR "cold protected-main preparation contract rejected: ${error}")
  endif()
  file(READ "${root}/provenance/ci-run.json" run_json)
  file(READ "${root}/provenance/ci-jobs.json" jobs_json)
  file(READ "${root}/provenance/ci-artifacts.json" artifacts_json)
  file(READ "${root}/provenance/pr.json" pr_json)
  file(READ "${root}/provenance/candidate-commit.json" candidate_commit)
  file(READ "${root}/provenance/tested-commit.json" tested_commit)
  file(READ "${root}/provenance/base-commit.json" base_commit)
  file(READ "${root}/provenance/candidate-tree.json" candidate_tree)
  file(READ "${root}/provenance/tested-tree.json" tested_tree)
  file(READ "${root}/provenance/base-tree.json" base_tree)
  file(READ "${root}/provenance/protected-main-commit.json" protected_commit)
  file(READ "${root}/provenance/protected-main-tree.json" protected_tree)
  file(SHA256 "${root}/pr-metadata.json" actual_pr_metadata_sha)
  file(SHA256 "${root}/pr-body.md" actual_pr_body_sha)
  file(READ "${root}/request.md" request_markdown)
  string(FIND "${request_markdown}" "PR body: `${request_body_path}`" body_reference_position)
  if(NOT actual_pr_metadata_sha STREQUAL request_pr_metadata_sha OR
     body_reference_position EQUAL -1 OR
     NOT request_markdown MATCHES "${actual_pr_body_sha}")
    message(FATAL_ERROR "prepared PR metadata or exact body digest is missing or mismatched")
  endif()

  phase20_validate_run_json("${root}/provenance/ci-run.json" "${root}/provenance/ci-jobs.json"
    "${root}/provenance/ci-artifacts.json" ci "${run_id}" "${attempt}"
    "${candidate_sha}" "${tested_sha}" "${base_sha}" "${pr_number}" error)
  if(NOT error STREQUAL "")
    message(FATAL_ERROR "CI run/job/artifact receipt rejected: ${error}")
  endif()
  phase24_validate_artifact_map("${root}" "${run_id}" "${attempt}" "${artifacts_json}" error)
  if(NOT error STREQUAL "")
    message(FATAL_ERROR "immutable artifact/file receipt failed: ${error}")
  endif()
  phase20_resolve_tested_sha("${run_json}" "${pr_json}" "${candidate_sha}" "${pr_number}"
    resolved_tested error)
  if(NOT error STREQUAL "" OR NOT resolved_tested STREQUAL tested_sha)
    message(FATAL_ERROR "current PR/tested merge mapping rejected: ${error}")
  endif()
  foreach(pair IN ITEMS
      "candidate|${candidate_sha}|${candidate_commit}|${candidate_tree}"
      "tested|${tested_sha}|${tested_commit}|${tested_tree}"
      "base|${base_sha}|${base_commit}|${base_tree}")
    string(REPLACE "|" ";" fields "${pair}")
    list(GET fields 0 label)
    list(GET fields 1 expected_sha)
    list(GET fields 2 commit_json)
    list(GET fields 3 tree_json)
    phase20_json_get(actual_sha "${commit_json}" sha)
    phase20_json_get(commit_tree_sha "${commit_json}" commit tree sha)
    phase20_json_get(tree_sha "${tree_json}" sha)
    phase20_json_get(truncated "${tree_json}" truncated)
    phase24_lower_sha("${tree_sha}" 40 valid_tree_sha)
    if(NOT actual_sha STREQUAL expected_sha OR NOT commit_tree_sha STREQUAL tree_sha OR
       NOT truncated STREQUAL "false" OR NOT valid_tree_sha)
      message(FATAL_ERROR "${label} commit/tree identity is incomplete or mismatched")
    endif()
  endforeach()
  phase20_json_get(protected_sha "${protected_commit}" sha)
  phase20_json_get(protected_tree_sha "${protected_commit}" commit tree sha)
  phase20_json_get(protected_tree_id "${protected_tree}" sha)
  phase20_json_get(protected_truncated "${protected_tree}" truncated)
  if(NOT protected_sha STREQUAL base_sha OR NOT protected_tree_sha STREQUAL protected_tree_id OR
     NOT protected_truncated STREQUAL "false")
    message(FATAL_ERROR "independent protected-main Git commit/tree does not match the PR base")
  endif()
  phase20_json_get(candidate_tree_id "${candidate_tree}" sha)
  if(NOT candidate_tree_id STREQUAL request_candidate_tree)
    message(FATAL_ERROR "candidate tree does not match the prepared request tree")
  endif()
  phase24_find_tree_entry("${protected_tree}" "tests/accuracy/scoreboard.txt"
    protected_blob_sha protected_blob_mode error)
  if(NOT error STREQUAL "" OR NOT protected_blob_mode STREQUAL "100644")
    message(FATAL_ERROR "independent protected-main tree does not contain the expected scoreboard blob: ${error}")
  endif()
  execute_process(
    COMMAND git hash-object "${root}/provenance/protected-main-scoreboard.txt"
    WORKING_DIRECTORY "${source_dir}"
    RESULT_VARIABLE blob_result
    OUTPUT_VARIABLE actual_blob_sha
    ERROR_VARIABLE blob_error
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  file(SHA256 "${root}/provenance/protected-main-scoreboard.txt" api_scoreboard_sha)
  file(SHA256 "${root}/artifacts/build-linux-x64/protected-main/scoreboard.txt" hosted_scoreboard_sha)
  if(NOT "${blob_result}" STREQUAL "0" OR NOT actual_blob_sha STREQUAL protected_blob_sha OR
     NOT api_scoreboard_sha STREQUAL hosted_scoreboard_sha)
    message(FATAL_ERROR "cold protected-main scoreboard bytes do not match the independently fetched Git blob")
  endif()
  phase20_read_properties("${root}/artifacts/build-linux-x64/protected-main/metadata.txt"
    protected valid metadata_error)
  file(STRINGS "${root}/artifacts/build-linux-x64/protected-main/scoreboard.txt" protected_rows)
  list(LENGTH protected_rows protected_count)
  if(NOT metadata_error STREQUAL "" OR NOT protected_authority STREQUAL "protected-main" OR
     NOT protected_protected_revision STREQUAL base_sha OR
     NOT protected_count MATCHES "^[1-9][0-9]*$")
    message(FATAL_ERROR "protected-main pass count or authority does not match the independently fetched base")
  endif()
  phase24_find_tree_entry("${candidate_tree}" "tests/accuracy/scoreboard.txt"
    candidate_scoreboard_blob candidate_scoreboard_mode error)
  execute_process(
    COMMAND git hash-object "${source_dir}/tests/accuracy/scoreboard.txt"
    WORKING_DIRECTORY "${source_dir}"
    RESULT_VARIABLE source_blob_result
    OUTPUT_VARIABLE source_scoreboard_blob
    ERROR_VARIABLE source_blob_error
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  file(SHA256 "${source_dir}/tests/accuracy/scoreboard.txt" source_scoreboard_sha)
  file(SHA256 "${root}/provenance/candidate-scoreboard.txt" received_scoreboard_sha)
  if(NOT error STREQUAL "" OR NOT "${source_blob_result}" STREQUAL "0" OR
     NOT candidate_scoreboard_mode STREQUAL "100644" OR
     NOT source_scoreboard_blob STREQUAL candidate_scoreboard_blob OR
     NOT source_scoreboard_sha STREQUAL received_scoreboard_sha)
    message(FATAL_ERROR "candidate scoreboard bytes do not match the committed candidate Git blob")
  endif()
  foreach(platform IN LISTS NESTURBATOR_PHASE20_PLATFORMS)
    phase24_validate_platform_baseline("${root}" "${platform}" "${base_sha}"
      "${api_scoreboard_sha}" "${protected_count}" error)
    if(NOT error STREQUAL "")
      message(FATAL_ERROR "protected baseline from ${platform} rejected: ${error}")
    endif()
    phase24_validate_prepare_step("${root}" "${platform}" "${run_id}" "${attempt}" error)
    if(NOT error STREQUAL "")
      message(FATAL_ERROR "cold protected-main step from ${platform} rejected: ${error}")
    endif()
  endforeach()
  set(NESTURBATOR_PHASE20_PROTECTED_MINIMUM "${protected_count}")
  phase20_validate_scoreboard("${root}/provenance/candidate-scoreboard.txt"
    "${root}/artifacts/build-linux-x64/protected-main/scoreboard.txt"
    "${root}/artifacts/build-linux-x64/protected-main/metadata.txt" "${base_sha}" error)
  if(NOT error STREQUAL "")
    message(FATAL_ERROR "protected-key scoreboard check failed: ${error}")
  endif()
  phase20_validate_build_logs("${root}" "${run_id}" "${attempt}" error)
  if(NOT error STREQUAL "")
    message(FATAL_ERROR "fresh per-platform protected observation checks failed: ${error}")
  endif()

  set(exact_platform_directories)
  foreach(platform IN LISTS NESTURBATOR_PHASE20_PLATFORMS)
    list(APPEND exact_platform_directories "${root}/artifacts/hashes-${platform}")
    if(NOT EXISTS "${root}/artifacts/hashes-${platform}/hashes.txt" OR
       IS_SYMLINK "${root}/artifacts/hashes-${platform}/hashes.txt")
      message(FATAL_ERROR "missing real hash artifact for platform ${platform}")
    endif()
  endforeach()
  file(GLOB platform_directories LIST_DIRECTORIES TRUE "${root}/artifacts/hashes-*")
  list(LENGTH platform_directories platform_directory_count)
  if(NOT platform_directory_count EQUAL 6)
    message(FATAL_ERROR "expected exactly the six named hash platform artifacts")
  endif()
  execute_process(
    COMMAND "${CMAKE_COMMAND}" "-DROOT=${root}/artifacts"
      -P "${CMAKE_CURRENT_LIST_DIR}/hash_inventory.cmake"
    RESULT_VARIABLE hash_result OUTPUT_VARIABLE hash_stdout ERROR_VARIABLE hash_stderr)
  if(NOT "${hash_result}" STREQUAL "0")
    message(FATAL_ERROR "complete six-platform hash inventory failed: ${hash_stdout}${hash_stderr}")
  endif()

  phase20_validate_campaign("${root}" "${source_dir}" "${tested_sha}" error)
  if(NOT error STREQUAL "")
    message(FATAL_ERROR "fresh full AccuracyCoin campaign did not validate: ${error}")
  endif()
  phase20_validate_perf("${root}" "${source_dir}" "${run_id}" "${attempt}"
    "${candidate_sha}" "${tested_sha}" "${base_sha}" error)
  if(NOT error STREQUAL "")
    message(FATAL_ERROR "raw Cachegrind evidence failed the unchanged +1% gate: ${error}")
  endif()

  set(scope_paths
    .github/workflows/ci.yml
    tests/CMakeLists.txt
    tests/cmake/prepare_scoreboard_baseline.cmake
    tests/cmake/phase20_hosted_evidence.cmake
    tests/cmake/hash_inventory.cmake
    tests/cmake/hash_keys.cmake
    tests/cmake/perf_compare.cmake
    tests/accuracy/scoreboard.txt
    tests/accuracy/baseline-metadata.txt
    tests/accuracy/migration.tsv
    tests/roms/manifest.txt
    tests/perf/allow.txt
    tests/cpu/test_bus.c
    tests/cpu/dma_abort_sweep.h
    tests/cmake/phase24_dma_gap_evidence.cmake
    README.md
    .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-ACCEPTANCE.md
    .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-REVIEW-DISPOSITION.md
    .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-HOSTED-EVIDENCE.md)
  foreach(path IN LISTS scope_paths)
    phase24_find_tree_entry("${candidate_tree}" "${path}" candidate_blob candidate_mode error)
    if(NOT error STREQUAL "")
      message(FATAL_ERROR "candidate source tree does not bind ${path}: ${error}")
    endif()
    phase24_find_tree_entry("${tested_tree}" "${path}" tested_blob tested_mode error)
    if(NOT error STREQUAL "" OR NOT candidate_blob STREQUAL tested_blob OR
       NOT candidate_mode STREQUAL tested_mode)
      message(FATAL_ERROR "tested merge tree has an unaccounted scoped change at ${path}")
    endif()
  endforeach()

  file(GLOB_RECURSE receipt_files LIST_DIRECTORIES FALSE
    "${root}/provenance/*" "${root}/artifacts/*")
  list(SORT receipt_files)
  set(manifest "")
  foreach(path IN LISTS receipt_files)
    if(IS_SYMLINK "${path}" OR IS_DIRECTORY "${path}")
      message(FATAL_ERROR "receipt contains an unsafe non-regular file: ${path}")
    endif()
    file(RELATIVE_PATH relative "${root}" "${path}")
    file(SHA256 "${path}" digest)
    string(APPEND manifest "${digest}  ${relative}\n")
  endforeach()
  file(WRITE "${root}/phase24-manifest.tsv" "${manifest}")
  file(SHA256 "${root}/phase24-manifest.tsv" manifest_sha)
  file(WRITE "${root}/phase24-verdict.txt"
    "schema=1\nstatus=passed\nrepository=${NESTURBATOR_PHASE20_REPOSITORY}\n"
    "pr_number=${pr_number}\ncandidate_head_sha=${candidate_sha}\ntested_sha=${tested_sha}\nbase_sha=${base_sha}\n"
    "ci_run_id=${run_id}\nci_run_attempt=${attempt}\nartifact_manifest_sha256=${manifest_sha}\n")
  file(SHA256 "${root}/phase24-verdict.txt" verdict_sha)
  file(WRITE "${root}/phase24-verdict.sha256" "${verdict_sha}  phase24-verdict.txt\n")
  message(STATUS "Phase 24 hosted evidence validated for candidate ${candidate_sha}; verdict and receipt checksums written")
endfunction()

if(MODE STREQUAL "controls")
  phase24_controls()
elseif(MODE STREQUAL "prepare")
  phase24_prepare()
elseif(MODE STREQUAL "request")
  phase24_validate_request()
elseif(MODE STREQUAL "validate")
  phase24_validate()
else()
  message(FATAL_ERROR "MODE must be controls, prepare, request, or validate; got '${MODE}'")
endif()
