cmake_minimum_required(VERSION 3.25)

foreach(required_var IN ITEMS SOURCE_DIR WORK RUNNER ROM)
  if(NOT DEFINED ${required_var} OR "${${required_var}}" STREQUAL "")
    message(FATAL_ERROR "phase20 hosted evidence controls require ${required_var}")
  endif()
endforeach()

set(NESTURBATOR_PHASE20_EVIDENCE_LIBRARY_ONLY ON)
include("${SOURCE_DIR}/tests/cmake/phase20_hosted_evidence.cmake")

function(nesturbator_phase20_write_evidence_fixture root source_dir runner rom)
  file(REMOVE_RECURSE "${root}")
  file(MAKE_DIRECTORY "${root}/provenance" "${root}/downloads" "${root}/artifacts/perf"
    "${root}/artifacts/retroarch" "${root}/artifacts/nightly/vectors-full"
    "${root}/artifacts/nightly/mmc3-oracle")

  execute_process(COMMAND "${runner}" --accuracycoin-inventory --rom "${rom}"
    RESULT_VARIABLE inventory_result OUTPUT_VARIABLE inventory ERROR_VARIABLE inventory_error)
  if(NOT "${inventory_result}" STREQUAL "0")
    message(FATAL_ERROR "cannot create offline AccuracyCoin fixture inventory: ${inventory_error}")
  endif()

  file(READ "${source_dir}/tests/accuracy/scoreboard.txt" candidate_scoreboard)
  file(STRINGS "${source_dir}/tests/accuracy/scoreboard.txt" scoreboard_rows)
  file(WRITE "${root}/artifacts/build-linux-x64/accuracycoin/full/report.tsv"
    "key\tpage\trow\tkind\tstatus\tobserved\traw\tframes\tcause\tattempt_id\n")
  file(WRITE "${root}/artifacts/build-linux-x64/accuracycoin/full/inventory.tsv" "${inventory}")
  string(ASCII 9 tab)
  string(REPLACE "\n" ";" inventory_lines "${inventory}")
  foreach(line IN LISTS inventory_lines)
    if(line STREQUAL "" OR line MATCHES "^page${tab}")
      continue()
    endif()
    string(REPLACE "${tab}" ";" fields "${line}")
    list(LENGTH fields field_count)
    if(NOT field_count EQUAL 6)
      message(FATAL_ERROR "unexpected AccuracyCoin inventory row in fixture: ${line}")
    endif()
    list(GET fields 0 page)
    list(GET fields 1 row)
    list(GET fields 2 kind)
    list(GET fields 3 key)
    if(kind STREQUAL "DRAW")
      set(status draw)
      set(observed 1)
      set(raw -)
      set(frames -)
      set(cause draw-sentinel)
      set(report_kind draw)
    else()
      set(found_row FALSE)
      foreach(scoreboard_line IN LISTS scoreboard_rows)
        string(REPLACE "${tab}" ";" scoreboard_fields "${scoreboard_line}")
        list(LENGTH scoreboard_fields scoreboard_field_count)
        if(scoreboard_field_count EQUAL 5)
          list(GET scoreboard_fields 0 scoreboard_key)
          if(scoreboard_key STREQUAL key)
            list(GET scoreboard_fields 1 status)
            list(GET scoreboard_fields 2 raw)
            list(GET scoreboard_fields 3 frames)
            set(found_row TRUE)
            break()
          endif()
        endif()
      endforeach()
      if(NOT found_row)
        message(FATAL_ERROR "fixture scoreboard omitted AccuracyCoin inventory key ${key}")
      endif()
      set(observed 1)
      set(report_kind test)
      if(status STREQUAL "pass")
        set(cause terminal-pass)
      else()
        set(cause terminal-fail)
      endif()
    endif()
    if(page LESS 10)
      set(attempt "page-0${page}-cold-01")
    else()
      set(attempt "page-${page}-cold-01")
    endif()
    file(APPEND "${root}/artifacts/build-linux-x64/accuracycoin/full/report.tsv"
      "${key}\t${page}\t${row}\t${report_kind}\t${status}\t${observed}\t${raw}\t${frames}\t${cause}\t${attempt}\n")
  endforeach()

  include("${source_dir}/tests/cmake/accuracycoin_input_fingerprint.cmake")
  accuracycoin_compute_input_fingerprint("${source_dir}" fixture_fingerprint fixture_input_count)
  phase20_compute_effective_digest("${source_dir}" fixture_source_digest fixture_source_count
    fixture_source_manifest fixture_source_error)
  if(NOT fixture_source_error STREQUAL "")
    message(FATAL_ERROR "cannot create source identity fixture: ${fixture_source_error}")
  endif()
  configure_file("${source_dir}/tests/accuracy/scoreboard.txt"
    "${root}/artifacts/candidate-scoreboard.txt" COPYONLY)
  file(SHA256 "${root}/artifacts/build-linux-x64/accuracycoin/full/report.tsv" report_sha)
  file(SHA256 "${root}/artifacts/build-linux-x64/accuracycoin/full/inventory.tsv" inventory_sha)
  set(candidate_sha "cccccccccccccccccccccccccccccccccccccccc")
  set(base_sha "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb")
  set(tested_sha "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa")
  set(tree_sha "dddddddddddddddddddddddddddddddddddddddd")

  set(campaign_metadata "report_schema=1\ninventory_schema=1\nrom_pin=6bee919529289c555803b612b8c9e4f7eaaeeabb\nrom_sha256=8ac8904a3cf26fbdfa6be501e414d45886ff4478d30dee8e58fa436cf5fd5adc\ninventory_sha256=${inventory_sha}\nsource_revision=aaaaaaaaaaaa\nsource_dirty=0\nlibrary_version=0.1.11\nbehaviour_revision=8\nmode=campaign\ninput_start_policy=cold-boot;page-header-A;one-frame-A-press;one-frame-release\nboot_frame_budget=32\nnavigation_frame_budget_per_page=5\nexecution_frame_budget=1800\ntotal_frame_budget=41503\npages_launched=22\npages_start_attributed=22\nattempted_scored_rows=146\npages_completed=22\nscored_total=146\npass=114\nfail=32\nskip=0\nnot_run=0\ntimeout=0\ndraw=5\nobserved_scored_rows=146\nterminal_scored_rows=146\nhost_elapsed_guest_ns=8570616000\ntrustworthy_observation=1\ninfrastructure_verdict=complete\nbuild_inputs_sha256=${fixture_fingerprint}\nbuild_inputs_count=${fixture_input_count}\ncampaign_report_sha256=${report_sha}\n")
  file(WRITE "${root}/artifacts/build-linux-x64/accuracycoin/full/metadata.txt"
    "${campaign_metadata}")

  file(STRINGS "${source_dir}/tests/accuracy/scoreboard.txt" candidate_lines)
  set(protected_scoreboard "")
  set(protected_count 0)
  foreach(line IN LISTS candidate_lines)
    string(REPLACE "${tab}" ";" fields "${line}")
    list(GET fields 1 status)
    if(status STREQUAL "pass" AND protected_count LESS 108)
      list(GET fields 0 key)
      list(GET fields 2 raw)
      string(APPEND protected_scoreboard "${key}\tpass\t${raw}\t-\t-\n")
      math(EXPR protected_count "${protected_count} + 1")
    endif()
  endforeach()
  if(NOT protected_count EQUAL 108)
    message(FATAL_ERROR "offline fixture could not construct 108 protected PASS keys")
  endif()
  set(protected_path "${root}/artifacts/build-linux-x64/protected-main/scoreboard.txt")
  file(WRITE "${protected_path}" "${protected_scoreboard}")
  file(SHA256 "${protected_path}" protected_sha)
  file(WRITE "${root}/artifacts/build-linux-x64/protected-main/metadata.txt"
    "schema=1\nauthority=protected-main\nsource=canonical-baseline\nprotected_revision=${base_sha}\nrom_pin=6bee919529289c555803b612b8c9e4f7eaaeeabb\nrom_sha256=8ac8904a3cf26fbdfa6be501e414d45886ff4478d30dee8e58fa436cf5fd5adc\nscoreboard_rows=108\nscoreboard_denominator=108\nscoreboard_sha256=${protected_sha}\n")

  foreach(platform IN ITEMS linux-x64 linux-arm64 macos-x64 macos-arm64 windows-x64 windows-arm64)
    string(REPLACE "-" ";" platform_parts "${platform}")
    list(GET platform_parts 0 os)
    list(GET platform_parts 1 arch)
    file(MAKE_DIRECTORY "${root}/artifacts/hashes-${platform}"
      "${root}/artifacts/build-${platform}/accuracycoin"
      "${root}/artifacts/build-${platform}/protected-main")
    configure_file("${source_dir}/tests/runner/hashes.txt"
      "${root}/artifacts/hashes-${platform}/hashes.txt" COPYONLY)
    if(NOT platform STREQUAL "linux-x64")
      configure_file("${root}/artifacts/build-linux-x64/protected-main/scoreboard.txt"
        "${root}/artifacts/build-${platform}/protected-main/scoreboard.txt" COPYONLY)
      configure_file("${root}/artifacts/build-linux-x64/protected-main/metadata.txt"
        "${root}/artifacts/build-${platform}/protected-main/metadata.txt" COPYONLY)
    endif()
    file(WRITE "${root}/artifacts/build-${platform}/LastTest.log"
      "Test #1: accuracycoin.protected_observation ........ Passed\nTest #2: accuracycoin.protected ........ Passed\nTest #3: accuracy.scoreboard.no_lost_pass ........ Passed\nTest #4: vectors.n65v ........ Passed\nTest #5: holymapperel.M2_P128K_CR8K_V.decode ........ Passed\nTest #6: runner.boot_to_play ........ Passed\n")
  endforeach()

  foreach(rom IN ITEMS nesteroids runner M4_P256K_C256K)
    if(rom STREQUAL "nesteroids")
      set(mapper 0)
    elseif(rom STREQUAL "runner")
      set(mapper 1)
    else()
      set(mapper 4)
    endif()
    foreach(who IN ITEMS base_a base_b head)
      foreach(frames IN ITEMS 60 360)
        if(frames EQUAL 60)
          set(instructions 1000000)
        else()
          set(instructions 4000000)
        endif()
        file(WRITE "${root}/artifacts/perf/${who}_${rom}_${frames}.out"
          "version: 1\nsummary: ${instructions}\n")
      endforeach()
    endforeach()
  endforeach()
  file(WRITE "${root}/artifacts/perf/summary.md"
    "| ROM | mapper | base Ir/frame | head Ir/frame | delta Ir/frame | delta % | status |\n|---|---|---|---|---|---|---|\n| nesteroids | 0 | 10000 | 10000 | +0 | +0.00% | ok |\n| runner | 1 | 10000 | 10000 | +0 | +0.00% | ok |\n| M4_P256K_C256K | 4 | 10000 | 10000 | +0 | +0.00% | ok |\nbase ${base_sha} head ${tested_sha}\nvalgrind: valgrind-3.25.1\ngcc: gcc-14.2.0\n")
  file(WRITE "${root}/artifacts/perf/identity.txt"
    "run_id=1001\nrun_attempt=1\nbase_sha=${base_sha}\nhead_sha=${tested_sha}\ncandidate_head_sha=${candidate_sha}\nvalgrind_version=valgrind-3.25.1\ngcc_version=gcc-14.2.0\n")
  file(WRITE "${root}/artifacts/retroarch/marker-progression.txt"
    "progression boundaries=16 mode=complete verdict=VERDICT_OK\n")
  file(WRITE "${root}/artifacts/nightly/vectors-full/registered-tests.json"
    "{\"tests\":[{\"name\":\"cpu.vectors-full.00\"},{\"name\":\"cpu.vectors-full.ff\"}]}")
  file(WRITE "${root}/artifacts/nightly/vectors-full/vectors-full.junit.xml"
    "<testsuite name=\"vectors-full\" tests=\"2\" failures=\"0\" errors=\"0\" skipped=\"0\"><testcase name=\"cpu.vectors-full.00\"/><testcase name=\"cpu.vectors-full.ff\"/></testsuite>")
  file(WRITE "${root}/artifacts/nightly/mmc3-oracle/registered-tests.json"
    "{\"tests\":[{\"name\":\"mmc3.oracle.mmc3_irq_tests\"}]}")
  file(WRITE "${root}/artifacts/nightly/mmc3-oracle/mmc3-oracle.junit.xml"
    "<testsuite name=\"mmc3-oracle\" tests=\"1\" failures=\"0\" errors=\"0\" skipped=\"0\"><testcase name=\"mmc3.oracle.mmc3_irq_tests\"/></testsuite>")

  file(WRITE "${root}/provenance/pr.json"
    "{\"number\":42,\"head\":{\"sha\":\"${candidate_sha}\"},\"base\":{\"sha\":\"${base_sha}\"},\"merge_commit_sha\":\"${tested_sha}\"}")
  file(WRITE "${root}/provenance/ci-run.json"
    "{\"id\":1001,\"run_attempt\":1,\"event\":\"pull_request\",\"status\":\"completed\",\"conclusion\":\"success\",\"name\":\"CI\",\"path\":\".github/workflows/ci.yml\",\"run_started_at\":\"2026-10-26T00:00:00Z\",\"head_sha\":\"${candidate_sha}\",\"repository\":{\"full_name\":\"szTheory/nesturbator\"},\"pull_requests\":[{\"number\":42,\"base\":{\"sha\":\"${base_sha}\"},\"head\":{\"sha\":\"${candidate_sha}\"}}]}")
  file(WRITE "${root}/provenance/ci-commit.json"
    "{\"sha\":\"${tested_sha}\",\"tree\":{\"sha\":\"${tree_sha}\"},\"parents\":[{\"sha\":\"${base_sha}\"},{\"sha\":\"${candidate_sha}\"}]}")
  file(WRITE "${root}/provenance/candidate-commit.json"
    "{\"sha\":\"${candidate_sha}\",\"tree\":{\"sha\":\"${tree_sha}\"}}")
  file(WRITE "${root}/provenance/nightly-run.json"
    "{\"id\":2002,\"run_attempt\":1,\"event\":\"pull_request\",\"status\":\"completed\",\"conclusion\":\"success\",\"name\":\"Nightly\",\"path\":\".github/workflows/nightly.yml\",\"run_started_at\":\"2026-10-26T00:00:00Z\",\"head_sha\":\"${candidate_sha}\",\"repository\":{\"full_name\":\"szTheory/nesturbator\"},\"pull_requests\":[{\"number\":42,\"base\":{\"sha\":\"${base_sha}\"},\"head\":{\"sha\":\"${candidate_sha}\"}}]}")
  file(WRITE "${root}/provenance/ci-logs.txt" "workflow CI run 1001 attempt 1\n")
  file(WRITE "${root}/provenance/nightly-logs.txt" "workflow Nightly run 2002 attempt 1\n")

  set(job_entries "")
  set(job_names hygiene title vector-inventory asan hash-equality retroarch-e2e perf "CI required"
    "session-controls (ubuntu-24.04)" "session-controls (macos-15)"
    "nofp (ubuntu-24.04)" "nofp (ubuntu-24.04-arm)")
  set(job_names ${job_names}
    "build (ubuntu-24.04, ci, ci-build, linux, x64)"
    "build (ubuntu-24.04-arm, ci, ci-build, linux, arm64)"
    "build (macos-15, ci, ci-build, macos, arm64)"
    "build (macos-15-intel, ci, ci-build, macos, x64)"
    "build (windows-2025, ci-msvc, windows, x64)"
    "build (windows-11-arm, ci-msvc, windows, arm64)")
  set(ci_job_id 9000)
  foreach(name IN LISTS job_names)
    math(EXPR ci_job_id "${ci_job_id} + 1")
    if(NOT job_entries STREQUAL "")
      string(APPEND job_entries ",")
    endif()
    string(MAKE_C_IDENTIFIER "${name}" job_key)
    set("fixture_ci_job_id_${job_key}" "${ci_job_id}")
    phase20_expected_build_job_name("linux-x64" expected_job expected_step)
    set(job_steps "[]")
    if(name STREQUAL expected_job)
          set(job_steps "[{\"name\":\"${expected_step}\",\"number\":11,\"status\":\"completed\",\"conclusion\":\"success\",\"started_at\":\"2026-10-26T10:00:10Z\",\"completed_at\":\"2026-10-26T10:01:00Z\"}]")
    else()
      foreach(platform IN LISTS NESTURBATOR_PHASE20_PLATFORMS)
        phase20_expected_build_job_name("${platform}" platform_job platform_step)
        if(name STREQUAL platform_job)
          set(job_steps "[{\"name\":\"${platform_step}\",\"number\":11,\"status\":\"completed\",\"conclusion\":\"success\"}]")
        endif()
      endforeach()
    endif()
    string(APPEND job_entries "{\"id\":${ci_job_id},\"run_id\":1001,\"run_attempt\":1,\"name\":\"${name}\",\"status\":\"completed\",\"conclusion\":\"success\",\"steps\":${job_steps}}")
  endforeach()
  list(LENGTH job_names ci_job_count)
  file(WRITE "${root}/provenance/ci-jobs.json" "{\"total_count\":${ci_job_count},\"jobs\":[${job_entries}]}")
  file(WRITE "${root}/provenance/nightly-jobs.json"
    "{\"total_count\":2,\"jobs\":[{\"id\":9101,\"run_id\":2002,\"run_attempt\":1,\"name\":\"vectors-full\",\"status\":\"completed\",\"conclusion\":\"success\"},{\"id\":9102,\"run_id\":2002,\"run_attempt\":1,\"name\":\"mmc3-oracle\",\"status\":\"completed\",\"conclusion\":\"success\"}]}")

  set(artifact_entries "")
  set(artifact_id 3000)
  foreach(platform IN ITEMS linux-x64 linux-arm64 macos-x64 macos-arm64 windows-x64 windows-arm64)
    string(REPLACE "-" ";" platform_parts "${platform}")
    list(GET platform_parts 0 os)
    list(GET platform_parts 1 arch)
    foreach(name IN ITEMS "hashes-${platform}" "accuracycoin-evidence-${os}-${arch}-1001-1")
      math(EXPR artifact_id "${artifact_id} + 1")
      if(NOT artifact_entries STREQUAL "")
        string(APPEND artifact_entries ",")
      endif()
      string(APPEND artifact_entries "{\"id\":${artifact_id},\"name\":\"${name}\",\"expired\":false,\"created_at\":\"2026-10-26T01:00:00Z\",\"workflow_run\":{\"id\":1001,\"run_attempt\":1}}")
    endforeach()
  endforeach()
  foreach(name IN ITEMS phase20-perf-1001-1 retroarch-marker-progression)
    math(EXPR artifact_id "${artifact_id} + 1")
    string(APPEND artifact_entries ",{\"id\":${artifact_id},\"name\":\"${name}\",\"expired\":false,\"created_at\":\"2026-10-26T01:00:00Z\",\"workflow_run\":{\"id\":1001,\"run_attempt\":1}}")
  endforeach()
  set(ci_artifact_count ${artifact_id})
  math(EXPR ci_artifact_count "${ci_artifact_count} - 3000")
  file(WRITE "${root}/provenance/ci-artifacts.json" "{\"total_count\":${ci_artifact_count},\"artifacts\":[${artifact_entries}]}")
  file(WRITE "${root}/provenance/nightly-artifacts.json"
    "{\"total_count\":2,\"artifacts\":[{\"id\":4001,\"name\":\"vectors-full-evidence-2002\",\"expired\":false,\"created_at\":\"2026-10-26T01:00:00Z\",\"workflow_run\":{\"id\":2002,\"run_attempt\":1}},{\"id\":4002,\"name\":\"mmc3-oracle-evidence-2002\",\"expired\":false,\"created_at\":\"2026-10-26T01:00:00Z\",\"workflow_run\":{\"id\":2002,\"run_attempt\":1}}]}")

  foreach(platform IN LISTS NESTURBATOR_PHASE20_PLATFORMS)
    phase20_expected_build_job_name("${platform}" expected_job expected_step)
    string(MAKE_C_IDENTIFIER "${expected_job}" job_key)
    set(job_id "${fixture_ci_job_id_${job_key}}")
    set(job_metadata "${root}/provenance/build-${platform}-job.json")
    file(WRITE "${job_metadata}"
      "{\"id\":${job_id},\"run_id\":1001,\"run_attempt\":1,\"name\":\"${expected_job}\",\"status\":\"completed\",\"conclusion\":\"success\",\"steps\":[{\"name\":\"${expected_step}\",\"number\":11,\"status\":\"completed\",\"conclusion\":\"success\",\"started_at\":\"2026-10-26T10:00:10Z\",\"completed_at\":\"2026-10-26T10:01:00Z\"}]}")
    set(log "${root}/downloads/job-${platform}.txt")
    string(ASCII 239 187 191 log_bom)
    string(ASCII 27 log_esc)
    file(WRITE "${log}"
      "${log_bom}2026-10-26T09:59:50.0000000Z Current runner version: '2.338.0'\r\n"
      "2026-10-26T09:59:55.0000000Z ##[group]Run preceding-step-checks\r\n"
      "2026-10-26T09:59:56.0000000Z 1/6 Test #1: accuracycoin.protected_observation .... Passed 0.01 sec\r\n"
      "2026-10-26T09:59:56.0000000Z 2/6 Test #2: accuracycoin.protected ................ Passed 0.01 sec\r\n"
      "2026-10-26T09:59:56.0000000Z 3/6 Test #3: accuracy.scoreboard.no_lost_pass ....... Passed 0.01 sec\r\n"
      "2026-10-26T09:59:56.0000000Z 4/6 Test #4: vectors.n65v ............................ Passed 0.01 sec\r\n"
      "2026-10-26T09:59:56.0000000Z 5/6 Test #5: holymapperel.m2.decode .................. Passed 0.01 sec\r\n"
      "2026-10-26T09:59:56.0000000Z 6/6 Test #6: runner.boot_to_play ..................... Passed 0.01 sec\r\n"
      "2026-10-26T09:59:57.0000000Z ##[endgroup]\r\n"
      "2026-10-26T10:00:10.1000000Z ##[group]${expected_step}\r\n"
      "2026-10-26T10:00:11.1234567Z 1/6 Test #201: accuracycoin.protected_observation .... ${log_esc}[32mPassed${log_esc}[0m 16.82 sec\r\n"
      "2026-10-26T10:00:12.1234567Z 2/6 Test #202: accuracycoin.protected ................ Passed 0.01 sec\r\n"
      "2026-10-26T10:00:13.1234567Z 3/6 Test #254: accuracy.scoreboard.no_lost_pass ....... Passed 0.01 sec\r\n"
      "2026-10-26T10:00:14.1234567Z 4/6 Test #336: vectors.n65v ............................ Passed 0.01 sec\r\n"
      "2026-10-26T10:00:15.1234567Z 5/6 Test #116: holymapperel.m2.decode .................. Passed 0.00 sec\r\n"
      "2026-10-26T10:00:16.1234567Z 6/6 Test #289: runner.boot_to_play ..................... Passed 10.08 sec\r\n"
      "2026-10-26T10:00:59.0000000Z ##[endgroup]\r\n"
      "2026-10-26T10:01:01.0000000Z ##[group]Run following-step-checks\r\n"
      "2026-10-26T10:01:02.0000000Z 1/6 Test #1: accuracycoin.protected_observation .... Passed 0.01 sec\r\n"
      "2026-10-26T10:01:02.0000000Z 2/6 Test #2: accuracycoin.protected ................ Passed 0.01 sec\r\n"
      "2026-10-26T10:01:02.0000000Z 3/6 Test #3: accuracy.scoreboard.no_lost_pass ....... Passed 0.01 sec\r\n"
      "2026-10-26T10:01:02.0000000Z 4/6 Test #4: vectors.n65v ............................ Passed 0.01 sec\r\n"
      "2026-10-26T10:01:02.0000000Z 5/6 Test #5: holymapperel.m2.decode .................. Passed 0.01 sec\r\n"
      "2026-10-26T10:01:02.0000000Z 6/6 Test #6: runner.boot_to_play ..................... Passed 0.01 sec\r\n"
      "2026-10-26T10:01:03.0000000Z ##[endgroup]\r\n")
    file(SHA256 "${log}" raw_log_sha)
    phase20_extract_job_log("${log}" "${platform}" 2026-10-26T10:00:10Z 2026-10-26T10:01:00Z
      "${root}/provenance/build-${platform}-workflow.log" extract_error)
    if(NOT extract_error STREQUAL "")
      message(FATAL_ERROR "cannot derive offline selected workflow-step fixture: ${extract_error}")
    endif()
    file(SHA256 "${root}/provenance/build-${platform}-workflow.log" step_sha)
    file(WRITE "${root}/provenance/build-${platform}-job.txt"
      "job_id=${job_id}\njob_name=${expected_job}\nrun_id=1001\nrun_attempt=1\nstep_name=${expected_step}\nstep_number=11\nraw_job_log=job-${platform}.txt\nraw_job_log_sha256=${raw_log_sha}\nstep_log_sha256=${step_sha}\nstep_started_at=2026-10-26T10:00:10Z\nstep_completed_at=2026-10-26T10:01:00Z\n")
  endforeach()

  file(WRITE "${root}/provenance/source-identity.txt"
    "repository=szTheory/nesturbator\nci_run_id=1001\nci_run_attempt=1\nnightly_run_id=2002\nnightly_run_attempt=1\npr_number=42\ncandidate_head_sha=${candidate_sha}\ntested_sha=${tested_sha}\nbase_sha=${base_sha}\nsource_digest=${fixture_source_digest}\nsource_count=${fixture_source_count}\ncandidate_tree_sha=${tree_sha}\ntested_tree_sha=${tree_sha}\nprovenance_kind=fixture\n")
endfunction()

function(phase20_control_set_selected_job_log root mode)
  set(raw_log "${root}/downloads/job-linux-x64.txt")
  set(step_log "${root}/provenance/build-linux-x64-workflow.log")
  set(mapping_path "${root}/provenance/build-linux-x64-job.txt")
  set(results "")
  if(mode STREQUAL "positive")
    set(results
      "2026-10-26T10:00:11.1234567Z 1/6 Test #201: accuracycoin.protected_observation .... Passed 16.82 sec\r\n"
      "2026-10-26T10:00:12.1234567Z 2/6 Test #202: accuracycoin.protected ................ Passed 0.01 sec\r\n"
      "2026-10-26T10:00:13.1234567Z 3/6 Test #254: accuracy.scoreboard.no_lost_pass ....... Passed 0.01 sec\r\n"
      "2026-10-26T10:00:14.1234567Z 4/6 Test #336: vectors.n65v ............................ Passed 0.01 sec\r\n"
      "2026-10-26T10:00:15.1234567Z 5/6 Test #116: holymapperel.m2.decode .................. Passed 0.00 sec\r\n"
      "2026-10-26T10:00:16.1234567Z 6/6 Test #289: runner.boot_to_play ..................... Passed 10.08 sec\r\n")
  elseif(mode STREQUAL "empty")
    set(results "2026-10-26T10:00:11.0000000Z Test project /build/ci\r\n2026-10-26T10:00:12.0000000Z No tests were found!!!\r\n")
  elseif(mode STREQUAL "near-miss")
    set(results
      "2026-10-26T10:00:11.1234567Z 1/6 Test #201: accuracycoin.protected_observation .... Passed 16.82 sec\r\n"
      "2026-10-26T10:00:12.1234567Z 2/6 Test #202: accuracycoin.protected ................ Passed 0.01 sec\r\n"
      "2026-10-26T10:00:13.1234567Z 3/6 Test #254: accuracy.scoreboard.no_lost_pass ....... Passed 0.01 sec\r\n"
      "2026-10-26T10:00:14.1234567Z 4/6 Test #336: vectors.n65v ............................ Passed 0.01 sec\r\n"
      "2026-10-26T10:00:15.1234567Z 5/6 Test #116: holymapperel.M2_P128K_CR8K_V.decode .... Passed 0.00 sec\r\n"
      "2026-10-26T10:00:16.1234567Z 6/6 Test #289: runner.boot_to_play ..................... Passed 10.08 sec\r\n")
  elseif(mode STREQUAL "skipped")
    set(results
      "2026-10-26T10:00:11.1234567Z 1/6 Test #201: accuracycoin.protected_observation .... Passed 0.01 sec\r\n"
      "2026-10-26T10:00:12.1234567Z 2/6 Test #202: accuracycoin.protected ................ Passed 0.01 sec\r\n"
      "2026-10-26T10:00:13.1234567Z 3/6 Test #254: accuracy.scoreboard.no_lost_pass ....... Passed 0.01 sec\r\n"
      "2026-10-26T10:00:14.1234567Z 4/6 Test #336: vectors.n65v ............................ Passed 0.01 sec\r\n"
      "2026-10-26T10:00:15.1234567Z 5/6 Test #116: holymapperel.m2.decode .................. Skipped 0.00 sec\r\n"
      "2026-10-26T10:00:16.1234567Z 6/6 Test #289: runner.boot_to_play ..................... Passed 0.01 sec\r\n")
  elseif(mode STREQUAL "failed")
    set(results
      "2026-10-26T10:00:11.1234567Z 1/6 Test #201: accuracycoin.protected_observation .... Passed 0.01 sec\r\n"
      "2026-10-26T10:00:12.1234567Z 2/6 Test #202: accuracycoin.protected ................ Passed 0.01 sec\r\n"
      "2026-10-26T10:00:13.1234567Z 3/6 Test #254: accuracy.scoreboard.no_lost_pass ....... Passed 0.01 sec\r\n"
      "2026-10-26T10:00:14.1234567Z 4/6 Test #336: vectors.n65v ............................ Passed 0.01 sec\r\n"
      "2026-10-26T10:00:15.1234567Z 5/6 Test #116: holymapperel.m2.decode .................. Failed 0.00 sec\r\n"
      "2026-10-26T10:00:16.1234567Z 6/6 Test #289: runner.boot_to_play ..................... Passed 0.01 sec\r\n")
  elseif(mode STREQUAL "duplicate")
    set(results
      "2026-10-26T10:00:11.1234567Z 1/6 Test #201: accuracycoin.protected_observation .... Passed 0.01 sec\r\n"
      "2026-10-26T10:00:12.1234567Z 2/6 Test #202: accuracycoin.protected ................ Passed 0.01 sec\r\n"
      "2026-10-26T10:00:13.1234567Z 3/6 Test #254: accuracy.scoreboard.no_lost_pass ....... Passed 0.01 sec\r\n"
      "2026-10-26T10:00:14.1234567Z 4/6 Test #336: vectors.n65v ............................ Passed 0.01 sec\r\n"
      "2026-10-26T10:00:15.1234567Z 5/6 Test #116: holymapperel.m2.decode .................. Passed 0.00 sec\r\n"
      "2026-10-26T10:00:16.1234567Z 6/6 Test #117: holymapperel.m2.decode .................. Passed 0.00 sec\r\n"
      "2026-10-26T10:00:17.1234567Z 7/7 Test #289: runner.boot_to_play ..................... Passed 0.01 sec\r\n")
  elseif(mode STREQUAL "contradictory")
    set(results
      "2026-10-26T10:00:11.1234567Z 1/6 Test #201: accuracycoin.protected_observation .... Passed 0.01 sec\r\n"
      "2026-10-26T10:00:12.1234567Z 2/6 Test #202: accuracycoin.protected ................ Passed 0.01 sec\r\n"
      "2026-10-26T10:00:13.1234567Z 3/6 Test #254: accuracy.scoreboard.no_lost_pass ....... Passed 0.01 sec\r\n"
      "2026-10-26T10:00:14.1234567Z 4/6 Test #336: vectors.n65v ............................ Passed 0.01 sec\r\n"
      "2026-10-26T10:00:15.1234567Z 5/6 Test #116: holymapperel.m2.decode .................. Passed 0.00 sec\r\n"
      "2026-10-26T10:00:16.1234567Z 6/6 Test #117: holymapperel.m2.decode .................. Failed 0.00 sec\r\n"
      "2026-10-26T10:00:17.1234567Z 7/7 Test #289: runner.boot_to_play ..................... Passed 0.01 sec\r\n")
  elseif(NOT mode STREQUAL "other-step-only")
    message(FATAL_ERROR "unknown selected-job-log fixture mode: ${mode}")
  endif()

  string(ASCII 239 187 191 log_bom)
  string(ASCII 27 log_esc)
  file(WRITE "${raw_log}"
    "${log_bom}2026-10-26T09:59:50.0000000Z Current runner version: '2.338.0'\r\n"
    "2026-10-26T09:59:55.0000000Z ##[group]Run preceding-step-checks\r\n"
    "2026-10-26T09:59:56.0000000Z 1/6 Test #1: accuracycoin.protected_observation .... Passed 0.01 sec\r\n"
    "2026-10-26T09:59:56.0000000Z 2/6 Test #2: accuracycoin.protected ................ Passed 0.01 sec\r\n"
    "2026-10-26T09:59:56.0000000Z 3/6 Test #3: accuracy.scoreboard.no_lost_pass ....... Passed 0.01 sec\r\n"
    "2026-10-26T09:59:56.0000000Z 4/6 Test #4: vectors.n65v ............................ Passed 0.01 sec\r\n"
    "2026-10-26T09:59:56.0000000Z 5/6 Test #5: holymapperel.m2.decode .................. Passed 0.01 sec\r\n"
    "2026-10-26T09:59:56.0000000Z 6/6 Test #6: runner.boot_to_play ..................... Passed 0.01 sec\r\n"
    "2026-10-26T09:59:57.0000000Z ##[endgroup]\r\n"
    "2026-10-26T10:00:10.1000000Z ##[group]Run cmake --workflow --preset ci-build\r\n"
    "${results}"
    "2026-10-26T10:00:59.0000000Z ##[endgroup]\r\n"
    "2026-10-26T10:01:01.0000000Z ##[group]Run following-step-checks\r\n"
    "2026-10-26T10:01:02.0000000Z 1/6 Test #1: accuracycoin.protected_observation .... Passed 0.01 sec\r\n"
    "2026-10-26T10:01:02.0000000Z 2/6 Test #2: accuracycoin.protected ................ Passed 0.01 sec\r\n"
    "2026-10-26T10:01:02.0000000Z 3/6 Test #3: accuracy.scoreboard.no_lost_pass ....... Passed 0.01 sec\r\n"
    "2026-10-26T10:01:02.0000000Z 4/6 Test #4: vectors.n65v ............................ Passed 0.01 sec\r\n"
    "2026-10-26T10:01:02.0000000Z 5/6 Test #5: holymapperel.m2.decode .................. Passed 0.01 sec\r\n"
    "2026-10-26T10:01:02.0000000Z 6/6 Test #6: runner.boot_to_play ..................... Passed 0.01 sec\r\n"
    "2026-10-26T10:01:03.0000000Z ##[endgroup]\r\n")
  if(mode STREQUAL "positive")
    file(READ "${raw_log}" raw_text)
    string(REPLACE "Passed 16.82 sec" "${log_esc}[32mPassed${log_esc}[0m 16.82 sec" raw_text "${raw_text}")
    file(WRITE "${raw_log}" "${raw_text}")
  endif()
  file(READ "${mapping_path}" mapping_text)
  file(SHA256 "${raw_log}" raw_sha)
  file(WRITE "${root}/provenance/build-linux-x64-workflow.log" "")
  phase20_extract_job_log("${raw_log}" linux-x64 2026-10-26T10:00:10Z 2026-10-26T10:01:00Z
    "${step_log}" extract_error)
  if(NOT extract_error STREQUAL "")
    message(FATAL_ERROR "could not construct selected workflow-step fixture: ${extract_error}")
  endif()
  file(SHA256 "${step_log}" step_sha)
  string(REGEX REPLACE "raw_job_log_sha256=[0-9a-f]+" "raw_job_log_sha256=${raw_sha}" mapping_text "${mapping_text}")
  string(REGEX REPLACE "step_log_sha256=[0-9a-f]+" "step_log_sha256=${step_sha}" mapping_text "${mapping_text}")
  file(WRITE "${mapping_path}" "${mapping_text}")
endfunction()

function(phase20_adapter_zip source_dir prefix archive work_dir)
  file(REMOVE_RECURSE "${work_dir}")
  file(MAKE_DIRECTORY "${work_dir}/${prefix}")
  file(COPY "${source_dir}/" DESTINATION "${work_dir}/${prefix}")
  file(GLOB_RECURSE members LIST_DIRECTORIES FALSE RELATIVE "${work_dir}" "${work_dir}/*")
  if(NOT members)
    message(FATAL_ERROR "adapter ZIP fixture has no members: ${source_dir}")
  endif()
  execute_process(COMMAND "${CMAKE_COMMAND}" -E tar cf "${archive}" --format=zip ${members}
    WORKING_DIRECTORY "${work_dir}" RESULT_VARIABLE zip_result
    OUTPUT_VARIABLE zip_stdout ERROR_VARIABLE zip_stderr)
  if(NOT "${zip_result}" STREQUAL "0")
    message(FATAL_ERROR "cannot package adapter artifact: ${zip_stdout}${zip_stderr}")
  endif()
endfunction()

function(phase20_adapter_find_id json name out_id)
  phase20_json_get(count "${json}" artifacts LENGTH)
  set(found_id "")
  if(count GREATER 0)
    math(EXPR last "${count} - 1")
    foreach(index RANGE 0 ${last})
      phase20_json_get(actual_name "${json}" artifacts ${index} name)
      if(actual_name STREQUAL name)
        if(NOT found_id STREQUAL "")
          set(${out_id} "" PARENT_SCOPE)
          return()
        endif()
        phase20_json_get(found_id "${json}" artifacts ${index} id)
      endif()
    endforeach()
  endif()
  set(${out_id} "${found_id}" PARENT_SCOPE)
endfunction()

function(phase20_adapter_find_job_id json name out_id)
  phase20_json_get(count "${json}" jobs LENGTH)
  set(found_id "")
  if(count GREATER 0)
    math(EXPR last "${count} - 1")
    foreach(index RANGE 0 ${last})
      phase20_json_get(actual_name "${json}" jobs ${index} name)
      if(actual_name STREQUAL name)
        phase20_json_get(found_id "${json}" jobs ${index} id)
        break()
      endif()
    endforeach()
  endif()
  set(${out_id} "${found_id}" PARENT_SCOPE)
endfunction()

function(phase20_adapter_route fake_root endpoint response)
  if(ARGC GREATER 3)
    set(route_ending "${ARGV3}")
  else()
    set(route_ending "\r\n")
  endif()
  if(NOT EXISTS "${fake_root}/${response}")
    message(FATAL_ERROR "adapter route response does not exist: ${response}")
  endif()
  file(APPEND "${fake_root}/routes.tsv" "${endpoint}\t${response}${route_ending}")
endfunction()

function(phase20_adapter_api_routes fake_root)
  # Exercise duplicate terminal CR handling across Windows text-mode generation.
  phase20_adapter_route("${fake_root}" "repos/szTheory/nesturbator/actions/runs/1001/attempts/1"
    "fixture/provenance/ci-run.json" "\r\r\n")
  phase20_adapter_route("${fake_root}" "repos/szTheory/nesturbator/actions/runs/1001/attempts/1/jobs?per_page=100" "fixture/provenance/ci-jobs.json")
  phase20_adapter_route("${fake_root}" "repos/szTheory/nesturbator/actions/runs/1001/artifacts?per_page=100" "fixture/provenance/ci-artifacts.json")
  phase20_adapter_route("${fake_root}" "repos/szTheory/nesturbator/actions/runs/2002/attempts/1" "fixture/provenance/nightly-run.json")
  phase20_adapter_route("${fake_root}" "repos/szTheory/nesturbator/actions/runs/2002/attempts/1/jobs?per_page=100" "fixture/provenance/nightly-jobs.json")
  phase20_adapter_route("${fake_root}" "repos/szTheory/nesturbator/actions/runs/2002/artifacts?per_page=100" "fixture/provenance/nightly-artifacts.json")
  phase20_adapter_route("${fake_root}" "repos/szTheory/nesturbator/pulls/42" "fixture/provenance/pr.json")
  phase20_adapter_route("${fake_root}" "repos/szTheory/nesturbator/git/commits/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" "fixture/provenance/ci-commit.json")
  phase20_adapter_route("${fake_root}" "repos/szTheory/nesturbator/git/commits/cccccccccccccccccccccccccccccccccccccccc" "fixture/provenance/candidate-commit.json")
  phase20_adapter_route("${fake_root}" "repos/szTheory/nesturbator/git/trees/dddddddddddddddddddddddddddddddddddddddd?recursive=1" "fixture/provenance/ci-tree.json")
  phase20_adapter_route("${fake_root}" "1001/1" "fixture/provenance/ci-logs.txt")
  phase20_adapter_route("${fake_root}" "2002/1" "fixture/provenance/nightly-logs.txt")
endfunction()

function(phase20_adapter_copy_route fake_root endpoint input output)
  get_filename_component(parent "${fake_root}/${output}" DIRECTORY)
  file(MAKE_DIRECTORY "${parent}")
  file(COPY_FILE "${input}" "${fake_root}/${output}")
  phase20_adapter_route("${fake_root}" "${endpoint}" "${output}")
endfunction()

if(NOT DEFINED CASE)
  set(CASE all)
endif()
file(MAKE_DIRECTORY "${WORK}")
set(FIXTURE "${WORK}/fixture")
nesturbator_phase20_write_evidence_fixture("${FIXTURE}" "${SOURCE_DIR}" "${RUNNER}" "${ROM}")

if(CASE STREQUAL "adapter" OR CASE STREQUAL "availability")
  if(NOT DEFINED FAKE_GH OR NOT EXISTS "${FAKE_GH}")
    message(FATAL_ERROR "phase20 adapter control requires the owned fake-gh executable")
  endif()
  set(fake_root "${WORK}/fake-gh")
  set(source_clone "${WORK}/source")
  set(adapter_root "${source_clone}/build/ci/collection")
  set(call_log "${WORK}/fake-gh-calls.tsv")
  file(REMOVE_RECURSE "${fake_root}" "${adapter_root}" "${source_clone}")
  file(REMOVE "${call_log}")
  file(MAKE_DIRECTORY "${fake_root}" "${source_clone}" "${WORK}/empty-hooks")
  file(WRITE "${fake_root}/routes.tsv" "")
  file(COPY_FILE "${FAKE_GH}" "${fake_root}/gh")
  file(COPY "${SOURCE_DIR}/" DESTINATION "${source_clone}"
    PATTERN ".git" EXCLUDE PATTERN ".planning" EXCLUDE PATTERN "build" EXCLUDE)
  execute_process(COMMAND git init WORKING_DIRECTORY "${source_clone}"
    RESULT_VARIABLE git_result OUTPUT_VARIABLE git_output ERROR_VARIABLE git_error)
  if(NOT "${git_result}" STREQUAL "0")
    message(FATAL_ERROR "cannot initialize source-bound adapter fixture: ${git_error}")
  endif()
  execute_process(COMMAND git add -A WORKING_DIRECTORY "${source_clone}"
    RESULT_VARIABLE git_result OUTPUT_VARIABLE git_output ERROR_VARIABLE git_error)
  if(NOT "${git_result}" STREQUAL "0")
    message(FATAL_ERROR "cannot stage adapter source fixture: ${git_error}")
  endif()
  execute_process(COMMAND git -c "core.hooksPath=${WORK}/empty-hooks" -c user.name=phase20-fixture
      -c user.email=phase20-fixture commit -m adapter-fixture
    WORKING_DIRECTORY "${source_clone}" RESULT_VARIABLE git_result
    OUTPUT_VARIABLE git_output ERROR_VARIABLE git_error)
  if(NOT "${git_result}" STREQUAL "0")
    message(FATAL_ERROR "cannot commit adapter source fixture: ${git_error}")
  endif()
  message(STATUS "phase20 adapter: isolated source clone is ready")
  phase20_compute_effective_digest("${source_clone}" adapter_digest adapter_count adapter_manifest digest_error)
  if(NOT digest_error STREQUAL "")
    message(FATAL_ERROR "cannot enumerate source-bound adapter fixture: ${digest_error}")
  endif()
  set(tree_entries "")
  set(adapter_paths "")
  set(adapter_path_lines "")
  string(REPLACE "\n" ";" adapter_rows "${adapter_manifest}")
  foreach(row IN LISTS adapter_rows)
    if(row STREQUAL "")
      continue()
    endif()
    string(REPLACE "\t" ";" fields "${row}")
    list(GET fields 0 path)
    list(APPEND adapter_paths "${path}")
    string(APPEND adapter_path_lines "${path}\n")
  endforeach()
  set(adapter_path_file "${WORK}/adapter-source-paths.txt")
  file(WRITE "${adapter_path_file}" "${adapter_path_lines}")
  execute_process(
    COMMAND git hash-object --no-filters --stdin-paths
    WORKING_DIRECTORY "${source_clone}" INPUT_FILE "${adapter_path_file}"
    RESULT_VARIABLE hash_result OUTPUT_VARIABLE blob_sha_lines ERROR_VARIABLE hash_error
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(NOT "${hash_result}" STREQUAL "0")
    message(FATAL_ERROR "cannot hash source-bound adapter fixture paths: ${hash_error}")
  endif()
  string(REPLACE "\r\n" "\n" blob_sha_lines "${blob_sha_lines}")
  string(REPLACE "\r" "\n" blob_sha_lines "${blob_sha_lines}")
  string(REPLACE "\n" ";" blob_shas "${blob_sha_lines}")
  list(LENGTH adapter_paths adapter_path_count)
  list(LENGTH blob_shas hash_count)
  if(NOT hash_count EQUAL adapter_path_count)
    message(FATAL_ERROR
      "adapter source tree hash count ${hash_count} does not match path count ${adapter_path_count}")
  endif()
  set(hash_index 0)
  foreach(path IN LISTS adapter_paths)
    list(GET blob_shas ${hash_index} blob_sha)
    if(NOT tree_entries STREQUAL "")
      string(APPEND tree_entries ",")
    endif()
    string(APPEND tree_entries "{\"path\":\"${path}\",\"mode\":\"100644\",\"type\":\"blob\",\"sha\":\"${blob_sha}\"}")
    math(EXPR hash_index "${hash_index} + 1")
  endforeach()
  message(STATUS "phase20 adapter: batched ${adapter_path_count} source hashes")
  file(MAKE_DIRECTORY "${fake_root}/fixture")
  file(COPY "${FIXTURE}/provenance" "${FIXTURE}/artifacts" "${FIXTURE}/downloads"
    DESTINATION "${fake_root}/fixture")
  foreach(name IN ITEMS ci-run.json ci-jobs.json ci-artifacts.json nightly-run.json
      nightly-jobs.json nightly-artifacts.json pr.json ci-commit.json
      candidate-commit.json)
    if(NOT EXISTS "${fake_root}/fixture/provenance/${name}")
      message(FATAL_ERROR "adapter fixture copy omitted provenance/${name}")
    endif()
  endforeach()
  file(WRITE "${fake_root}/fixture/provenance/ci-tree.json"
    "{\"sha\":\"dddddddddddddddddddddddddddddddddddddddd\",\"truncated\":false,\"tree\":[${tree_entries}]}")
  set(delivery_file "${adapter_root}/delivery.json")
  file(MAKE_DIRECTORY "${adapter_root}")
  file(WRITE "${delivery_file}"
    "{\"repository\":\"szTheory/nesturbator\",\"ci_run_id\":\"1001\",\"ci_run_attempt\":\"1\",\"nightly_run_id\":\"2002\",\"nightly_run_attempt\":\"1\",\"candidate_head_sha\":\"cccccccccccccccccccccccccccccccccccccccc\",\"pr_number\":\"42\"}\n")
  phase20_adapter_api_routes("${fake_root}")
  file(READ "${fake_root}/fixture/provenance/ci-artifacts.json" ci_artifacts)
  file(READ "${fake_root}/fixture/provenance/nightly-artifacts.json" nightly_artifacts)
  set(zip_work "${WORK}/zip-work")
  set(artifact_index 0)
  foreach(platform IN LISTS NESTURBATOR_PHASE20_PLATFORMS)
    string(REPLACE "-" ";" parts "${platform}")
    list(GET parts 0 os)
    list(GET parts 1 arch)
    foreach(kind IN ITEMS hashes build)
      if(kind STREQUAL "hashes")
        set(name "hashes-${os}-${arch}")
        set(payload "${FIXTURE}/artifacts/hashes-${platform}")
      else()
        set(name "accuracycoin-evidence-${os}-${arch}-1001-1")
        set(payload "${FIXTURE}/artifacts/build-${platform}")
      endif()
      phase20_adapter_find_id("${ci_artifacts}" "${name}" artifact_id)
      if(NOT artifact_id MATCHES "^[0-9]+$")
        message(FATAL_ERROR "adapter fixture missing artifact ID for ${name}")
      endif()
      math(EXPR artifact_index "${artifact_index} + 1")
      if(artifact_index GREATER 6)
        set(prefix bundle)
      else()
        set(prefix "")
      endif()
      set(zip_path "${WORK}/${artifact_id}.zip")
      phase20_adapter_zip("${payload}" "${prefix}" "${zip_path}" "${zip_work}")
      phase20_adapter_copy_route("${fake_root}"
        "repos/szTheory/nesturbator/actions/artifacts/${artifact_id}/zip"
        "${zip_path}" "fixture/zips/${artifact_id}.zip")
    endforeach()
  endforeach()
  foreach(name IN ITEMS "phase20-perf-1001-1" retroarch-marker-progression)
    phase20_adapter_find_id("${ci_artifacts}" "${name}" artifact_id)
    if(name STREQUAL "phase20-perf-1001-1")
      set(payload "${FIXTURE}/artifacts/perf")
    else()
      set(payload "${FIXTURE}/artifacts/retroarch")
    endif()
    set(zip_path "${WORK}/${artifact_id}.zip")
    phase20_adapter_zip("${payload}" "bundle" "${zip_path}" "${zip_work}")
    phase20_adapter_copy_route("${fake_root}"
      "repos/szTheory/nesturbator/actions/artifacts/${artifact_id}/zip"
      "${zip_path}" "fixture/zips/${artifact_id}.zip")
  endforeach()
  foreach(lane IN ITEMS vectors-full mmc3-oracle)
    phase20_adapter_find_id("${nightly_artifacts}" "${lane}-evidence-2002" artifact_id)
    set(payload "${FIXTURE}/artifacts/nightly/${lane}")
    set(zip_path "${WORK}/${artifact_id}.zip")
    phase20_adapter_zip("${payload}" "bundle" "${zip_path}" "${zip_work}")
    phase20_adapter_copy_route("${fake_root}"
      "repos/szTheory/nesturbator/actions/artifacts/${artifact_id}/zip"
      "${zip_path}" "fixture/zips/${artifact_id}.zip")
  endforeach()
  foreach(platform IN LISTS NESTURBATOR_PHASE20_PLATFORMS)
    phase20_expected_build_job_name("${platform}" expected_job expected_step)
    file(READ "${fake_root}/fixture/provenance/ci-jobs.json" jobs_json)
    phase20_adapter_find_job_id("${jobs_json}" "${expected_job}" job_id)
    phase20_adapter_route("${fake_root}"
      "repos/szTheory/nesturbator/actions/jobs/${job_id}"
      "fixture/provenance/build-${platform}-job.json")
    phase20_adapter_copy_route("${fake_root}"
      "repos/szTheory/nesturbator/actions/jobs/${job_id}/logs"
      "${FIXTURE}/downloads/job-${platform}.txt" "fixture/job-logs/job-${platform}.txt")
  endforeach()
  message(STATUS "phase20 adapter: all artifact ZIP and job-log fixtures are ready")
  if(CMAKE_HOST_WIN32)
    set(path_separator ";")
  else()
    set(path_separator ":")
  endif()
  set(fake_path "${fake_root}${path_separator}$ENV{PATH}")
  if(CASE STREQUAL "adapter")
    message(STATUS "phase20 adapter: starting production collect")
    execute_process(
      COMMAND "${CMAKE_COMMAND}" -E env
        "PATH=${fake_path}"
        "PHASE20_FAKE_GH_FIXTURE=${fake_root}"
        "PHASE20_FAKE_GH_CALLS=${call_log}"
        "${CMAKE_COMMAND}" "-DMODE=collect" "-DROOT=${adapter_root}"
        "-DSOURCE_REF=${source_clone}"
        -P "${SOURCE_DIR}/tests/cmake/phase20_hosted_evidence.cmake"
      RESULT_VARIABLE collect_result OUTPUT_VARIABLE collect_stdout ERROR_VARIABLE collect_stderr)
    if(NOT "${collect_result}" STREQUAL "0")
      message(FATAL_ERROR
        "authentic-shaped production collect did not accept the fixture: ${collect_stdout}${collect_stderr}")
    endif()
    if(NOT EXISTS "${adapter_root}/manifest.txt" OR NOT EXISTS "${adapter_root}/verdict.txt" OR
       NOT EXISTS "${adapter_root}/provenance/normalization.tsv")
      message(FATAL_ERROR "production collect omitted its hosted evidence manifest, verdict or raw normalization ledger")
    endif()
    file(READ "${adapter_root}/downloads/job-linux-x64.txt" collected_job_log_hex HEX)
    file(SHA256 "${adapter_root}/downloads/job-linux-x64.txt" collected_job_log_sha)
    phase20_read_properties("${adapter_root}/provenance/build-linux-x64-job.txt" collected_job
      valid_collected_job collected_job_error)
    if(NOT collected_job_log_hex MATCHES "^efbbbf" OR
       NOT collected_job_log_sha STREQUAL collected_job_raw_job_log_sha256 OR
       NOT EXISTS "${adapter_root}/provenance/build-linux-x64-workflow.log" OR
       NOT collected_job_raw_job_log STREQUAL "job-linux-x64.txt" OR
       NOT collected_job_step_started_at STREQUAL "2026-10-26T10:00:10Z" OR
       NOT collected_job_step_completed_at STREQUAL "2026-10-26T10:01:00Z" OR
       NOT collected_job_error STREQUAL "")
      message(FATAL_ERROR "collector did not preserve original job-log bytes and bind the derived selected step")
    endif()
    file(READ "${call_log}" collect_calls)
    if(collect_calls MATCHES "rejected-api-flag")
      message(FATAL_ERROR "production collector invoked unsupported gh api --repo")
    endif()
    if(NOT collect_calls MATCHES "api-raw\t[^\n]*/logs" OR
       NOT collect_calls MATCHES "api-raw\t[^\n]*/zip")
      message(FATAL_ERROR "production collect did not use escape-enabled raw job-log and artifact-ZIP requests")
    endif()
    message(STATUS "phase20 production collect adapter accepted the authentic-shaped fixture")
    execute_process(
      COMMAND "${CMAKE_COMMAND}" -E env
        "PATH=${fake_path}" "PHASE20_FAKE_GH_FIXTURE=${fake_root}"
        "PHASE20_FAKE_GH_CALLS=${call_log}" "PHASE20_FAKE_GH_FAILURE=api"
        "${CMAKE_COMMAND}" "-DMODE=collect" "-DROOT=${adapter_root}"
        "-DSOURCE_REF=${source_clone}"
        -P "${SOURCE_DIR}/tests/cmake/phase20_hosted_evidence.cmake"
      RESULT_VARIABLE retry_result OUTPUT_VARIABLE retry_stdout ERROR_VARIABLE retry_stderr)
    file(GLOB history_entries LIST_DIRECTORIES TRUE "${adapter_root}/scratch/history/*/provenance")
    if("${retry_result}" STREQUAL "0" OR EXISTS "${adapter_root}/manifest.txt" OR
       NOT EXISTS "${adapter_root}/verdict.txt" OR NOT history_entries)
      message(FATAL_ERROR "failed collect retry preserved stale hosted-pass output or lost raw history: ${retry_stdout}${retry_stderr}")
    endif()
    file(READ "${adapter_root}/verdict.txt" retry_verdict)
    if(retry_verdict MATCHES "verdict=hosted-pass")
      message(FATAL_ERROR "failed collect retry left a hosted-pass verdict")
    endif()
    message(STATUS "phase20 failed recollection cleared success and preserved raw history")

    set(old_gh_root "${adapter_root}/old-gh")
    file(MAKE_DIRECTORY "${old_gh_root}")
    configure_file("${delivery_file}" "${old_gh_root}/delivery.json" COPYONLY)
    file(REMOVE "${call_log}")
    execute_process(
      COMMAND "${CMAKE_COMMAND}" -E env
        "PATH=${fake_path}" "PHASE20_FAKE_GH_FIXTURE=${fake_root}"
        "PHASE20_FAKE_GH_CALLS=${call_log}" "PHASE20_FAKE_GH_NO_ESCAPE_FLAG=1"
        "${CMAKE_COMMAND}" "-DMODE=collect" "-DROOT=${old_gh_root}"
        "-DSOURCE_REF=${source_clone}"
        -P "${SOURCE_DIR}/tests/cmake/phase20_hosted_evidence.cmake"
      RESULT_VARIABLE old_gh_result OUTPUT_VARIABLE old_gh_stdout ERROR_VARIABLE old_gh_stderr)
    if("${old_gh_result}" STREQUAL "0" OR EXISTS "${old_gh_root}/manifest.txt" OR
       NOT EXISTS "${old_gh_root}/verdict.txt")
      message(FATAL_ERROR "a gh executable without raw-response support was accepted: ${old_gh_stdout}${old_gh_stderr}")
    endif()
    file(READ "${old_gh_root}/verdict.txt" old_gh_verdict)
    file(READ "${call_log}" old_gh_calls)
    if(NOT old_gh_verdict MATCHES "cli: installed GitHub CLI does not advertise --allow-escape-sequences" OR
       NOT old_gh_calls MATCHES "api-help" OR old_gh_calls MATCHES "api-raw")
      message(FATAL_ERROR "unsupported gh escape option was not feature-detected before a raw request: ${old_gh_verdict} / ${old_gh_calls}")
    endif()
    message(STATUS "phase20 collector rejects old gh before downloading raw evidence")
  endif()

  set(availability_root "${source_clone}/build/ci/availability-check")
  set(candidate_file "${WORK}/expected-candidate.txt")
  file(REMOVE_RECURSE "${availability_root}")
  file(MAKE_DIRECTORY "${availability_root}")
  file(WRITE "${availability_root}/delivery.json"
    "{\"repository\":\"szTheory/nesturbator\",\"ci_run_id\":\"1001\",\"ci_run_attempt\":\"1\",\"nightly_run_id\":\"2002\",\"nightly_run_attempt\":\"1\",\"candidate_head_sha\":\"cccccccccccccccccccccccccccccccccccccccc\",\"pr_number\":\"42\"}\n")
  file(WRITE "${candidate_file}" "cccccccccccccccccccccccccccccccccccccccc\n")
  file(REMOVE "${call_log}")
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env
      "PATH=${fake_path}"
      "PHASE20_FAKE_GH_FIXTURE=${fake_root}"
      "PHASE20_FAKE_GH_CALLS=${call_log}"
      "PHASE20_FAKE_GH_CONTENT_FORBIDDEN=1"
      "${CMAKE_COMMAND}" "-DMODE=availability" "-DROOT=${availability_root}"
      "-DSOURCE_REF=${source_clone}" "-DEXPECTED_CANDIDATE_FILE=${candidate_file}"
      "-DEXPECTED_PR_NUMBER=42"
      -P "${SOURCE_DIR}/tests/cmake/phase20_hosted_evidence.cmake"
    RESULT_VARIABLE availability_result OUTPUT_VARIABLE availability_stdout ERROR_VARIABLE availability_stderr)
  if(NOT "${availability_result}" STREQUAL "0" OR
     NOT EXISTS "${availability_root}/availability/availability.txt")
    message(FATAL_ERROR "production metadata-only availability rejected a complete fixture: ${availability_stdout}${availability_stderr}")
  endif()
  file(READ "${availability_root}/availability/availability.txt" availability_record)
  file(READ "${call_log}" availability_calls)
  if(NOT availability_record MATCHES "verdict=available" OR
     availability_calls MATCHES "/zip|/logs|run-view|rejected-content" OR
     EXISTS "${availability_root}/manifest.txt" OR EXISTS "${availability_root}/verdict.txt" OR
     EXISTS "${availability_root}/provenance/source-identity.txt" OR
     EXISTS "${availability_root}/availability/.metadata")
    message(FATAL_ERROR "availability wrote hosted content evidence or requested logs/archives")
  endif()

  file(WRITE "${candidate_file}" "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\n")
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env
      "PATH=${fake_path}" "PHASE20_FAKE_GH_FIXTURE=${fake_root}"
      "PHASE20_FAKE_GH_CALLS=${call_log}" "PHASE20_FAKE_GH_CONTENT_FORBIDDEN=1"
      "${CMAKE_COMMAND}" "-DMODE=availability" "-DROOT=${availability_root}"
      "-DSOURCE_REF=${source_clone}" "-DEXPECTED_CANDIDATE_FILE=${candidate_file}"
      "-DEXPECTED_PR_NUMBER=42" -P "${SOURCE_DIR}/tests/cmake/phase20_hosted_evidence.cmake"
    RESULT_VARIABLE stale_result OUTPUT_VARIABLE stale_stdout ERROR_VARIABLE stale_stderr)
  if("${stale_result}" STREQUAL "0" OR EXISTS "${availability_root}/availability/availability.txt")
    message(FATAL_ERROR "failed availability retry preserved stale success output")
  endif()
  file(WRITE "${candidate_file}" "cccccccccccccccccccccccccccccccccccccccc\n")
  set(original_metadata "${WORK}/availability-original")
  file(REMOVE_RECURSE "${original_metadata}")
  file(MAKE_DIRECTORY "${original_metadata}")
  set(metadata_names ci-run.json ci-jobs.json ci-artifacts.json pr.json ci-commit.json
    candidate-commit.json ci-tree.json build-linux-x64-job.json)
  foreach(name IN LISTS metadata_names)
    file(COPY_FILE "${fake_root}/fixture/provenance/${name}" "${original_metadata}/${name}")
  endforeach()
  set(ENV{PATH} "${fake_path}")
  set(ENV{PHASE20_FAKE_GH_FIXTURE} "${fake_root}")
  set(ENV{PHASE20_FAKE_GH_CALLS} "${call_log}")
  set(ENV{PHASE20_FAKE_GH_CONTENT_FORBIDDEN} "1")
  set(ENV{PHASE20_FAKE_GH_FAILURE} "")
  set(ENV{PHASE20_FAKE_GH_FAIL_MATCH} "")
  macro(phase20_restore_availability_metadata)
    foreach(saved_name IN LISTS metadata_names)
      file(COPY_FILE "${original_metadata}/${saved_name}"
        "${fake_root}/fixture/provenance/${saved_name}")
    endforeach()
  endmacro()
  function(phase20_assert_availability_rejection label expected)
    nesturbator_phase20_check_availability("${availability_root}" "${source_clone}"
      "${candidate_file}" "42" availability_ok availability_reason)
    string(FIND "${availability_reason}" "${expected}" expected_position)
    if(availability_ok OR expected_position EQUAL -1 OR
       EXISTS "${availability_root}/availability/availability.txt")
      message(FATAL_ERROR "availability ${label} was accepted, left stale success, or failed at the wrong stage: ${availability_reason}")
    endif()
    message(STATUS "phase20 availability control passed: ${label} -> ${availability_reason}")
  endfunction()

  phase20_restore_availability_metadata()
  file(READ "${fake_root}/fixture/provenance/ci-run.json" text)
  string(REPLACE "\"path\":\".github/workflows/ci.yml\"" "\"path\":\".github/workflows/other.yml\"" text "${text}")
  file(WRITE "${fake_root}/fixture/provenance/ci-run.json" "${text}")
  phase20_assert_availability_rejection("wrong-workflow" "status/workflow")

  phase20_restore_availability_metadata()
  file(READ "${fake_root}/fixture/provenance/ci-jobs.json" text)
  string(REPLACE "\"total_count\":18" "\"total_count\":19" text "${text}")
  file(WRITE "${fake_root}/fixture/provenance/ci-jobs.json" "${text}")
  phase20_assert_availability_rejection("truncated-job-inventory" "inventory is incomplete or truncated")

  phase20_restore_availability_metadata()
  file(READ "${fake_root}/fixture/provenance/ci-jobs.json" text)
  string(REPLACE "build (ubuntu-24.04, ci, ci-build, linux, x64)" "build (wrong-lane)" text "${text}")
  file(WRITE "${fake_root}/fixture/provenance/ci-jobs.json" "${text}")
  phase20_assert_availability_rejection("missing-matrix-job" "CI platform job missing from hosted run: linux-x64")

  phase20_restore_availability_metadata()
  file(READ "${fake_root}/fixture/provenance/ci-artifacts.json" text)
  string(REPLACE "\"name\":\"hashes-linux-x64\"" "\"name\":\"missing-hashes-linux-x64\"" text "${text}")
  file(WRITE "${fake_root}/fixture/provenance/ci-artifacts.json" "${text}")
  phase20_assert_availability_rejection("missing-artifact" "required artifact missing: hashes-linux-x64")

  phase20_restore_availability_metadata()
  file(READ "${fake_root}/fixture/provenance/ci-artifacts.json" text)
  string(REPLACE "\"name\":\"hashes-linux-x64\",\"expired\":false" "\"name\":\"hashes-linux-x64\",\"expired\":true" text "${text}")
  file(WRITE "${fake_root}/fixture/provenance/ci-artifacts.json" "${text}")
  phase20_assert_availability_rejection("expired-artifact" "required artifact expired: hashes-linux-x64")

  phase20_restore_availability_metadata()
  file(READ "${fake_root}/fixture/provenance/ci-artifacts.json" text)
  string(REPLACE "\"id\":3002" "\"id\":3001" text "${text}")
  file(WRITE "${fake_root}/fixture/provenance/ci-artifacts.json" "${text}")
  phase20_assert_availability_rejection("duplicate-artifact-id" "duplicate or invalid artifact ID")

  phase20_restore_availability_metadata()
  file(READ "${fake_root}/fixture/provenance/ci-artifacts.json" text)
  string(REPLACE "\"workflow_run\":{\"id\":1001" "\"workflow_run\":{\"id\":9999" text "${text}")
  file(WRITE "${fake_root}/fixture/provenance/ci-artifacts.json" "${text}")
  phase20_assert_availability_rejection("wrong-run-artifact" "wrong workflow run")

  phase20_restore_availability_metadata()
  file(READ "${fake_root}/fixture/provenance/ci-artifacts.json" text)
  string(REPLACE "\"run_attempt\":1" "\"run_attempt\":2" text "${text}")
  file(WRITE "${fake_root}/fixture/provenance/ci-artifacts.json" "${text}")
  phase20_assert_availability_rejection("old-attempt-artifact" "another run attempt")

  phase20_restore_availability_metadata()
  file(READ "${fake_root}/fixture/provenance/ci-artifacts.json" text)
  string(REPLACE ",\"created_at\":\"2026-10-26T01:00:00Z\"" "" text "${text}")
  file(WRITE "${fake_root}/fixture/provenance/ci-artifacts.json" "${text}")
  phase20_assert_availability_rejection("missing-artifact-freshness" "missing or malformed freshness timestamp")

  phase20_restore_availability_metadata()
  file(READ "${fake_root}/fixture/provenance/pr.json" text)
  string(REPLACE "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb" "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee" text "${text}")
  file(WRITE "${fake_root}/fixture/provenance/pr.json" "${text}")
  phase20_assert_availability_rejection("wrong-candidate-base" "PR snapshot does not match the run candidate/base identity")

  phase20_restore_availability_metadata()
  file(READ "${fake_root}/fixture/provenance/ci-commit.json" text)
  string(REPLACE "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb" "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee" text "${text}")
  file(WRITE "${fake_root}/fixture/provenance/ci-commit.json" "${text}")
  phase20_assert_availability_rejection("wrong-merge-parent" "merge parents do not match candidate/base")

  phase20_restore_availability_metadata()
  file(READ "${fake_root}/fixture/provenance/ci-tree.json" text)
  string(REPLACE "\"truncated\":false" "\"truncated\":true" text "${text}")
  file(WRITE "${fake_root}/fixture/provenance/ci-tree.json" "${text}")
  phase20_assert_availability_rejection("truncated-source-tree" "GitHub source tree is truncated")

  phase20_restore_availability_metadata()
  file(READ "${fake_root}/fixture/provenance/ci-tree.json" text)
  string(JSON text SET "${text}" tree 0 sha "\"eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\"")
  file(WRITE "${fake_root}/fixture/provenance/ci-tree.json" "${text}")
  phase20_assert_availability_rejection("source-tree-content-mismatch" "effective source content differs from hosted tree")

  phase20_restore_availability_metadata()
  file(READ "${fake_root}/fixture/provenance/build-linux-x64-job.json" text)
  string(REPLACE "\"run_attempt\":1" "\"run_attempt\":2" text "${text}")
  file(WRITE "${fake_root}/fixture/provenance/build-linux-x64-job.json" "${text}")
  phase20_assert_availability_rejection("wrong-attempt-job" "does not bind the selected successful run/attempt")

  phase20_restore_availability_metadata()
  set(README_PATH "${source_clone}/README.md")
  file(APPEND "${README_PATH}" "\nlocal drift\n")
  phase20_assert_availability_rejection("local-source-drift" "effective source checkout is dirty")
  file(COPY_FILE "${SOURCE_DIR}/README.md" "${README_PATH}")

  phase20_restore_availability_metadata()
  set(ENV{PHASE20_FAKE_GH_FAILURE} "auth")
  phase20_assert_availability_rejection("authentication-failure" "auth:")
  set(ENV{PHASE20_FAKE_GH_FAILURE} "cli")
  phase20_assert_availability_rejection("cli-failure" "cli:")
  set(ENV{PHASE20_FAKE_GH_FAILURE} "")
  set(ENV{PHASE20_FAKE_GH_CONTENT_FORBIDDEN} "")
  message(STATUS "phase20 production metadata-only availability and stale-success controls passed")
  return()
endif()

if(CASE STREQUAL "red")
  nesturbator_phase20_validate_evidence("${FIXTURE}" "${SOURCE_DIR}" ok reason TRUE)
  if(ok)
    message("1..1\nok 1 - accepts the complete synthetic six-platform hosted evidence bundle")
  else()
    message("1..1\nnot ok 1 - accepts the complete synthetic six-platform hosted evidence bundle\n# validator: ${reason}")
    message(FATAL_ERROR "TAP assertion failed: valid fixture must pass the content validator")
  endif()
  return()
endif()

if(CASE STREQUAL "all")
  set(manifest_newline_root "${WORK}/manifest-newline-control")
  file(REMOVE_RECURSE "${manifest_newline_root}")
  file(MAKE_DIRECTORY "${manifest_newline_root}/artifacts" "${manifest_newline_root}/provenance")
  file(WRITE "${manifest_newline_root}/artifacts/sample.txt" "evidence bytes\n")
  phase20_compute_evidence_manifest("${manifest_newline_root}" expected_manifest
    expected_manifest_sha manifest_error)
  if(NOT manifest_error STREQUAL "")
    message(FATAL_ERROR "cannot create evidence manifest newline control: ${manifest_error}")
  endif()
  if(CMAKE_HOST_WIN32)
    set(stored_manifest_text "${expected_manifest}")
  else()
    string(REPLACE "\n" "\r\n" stored_manifest_text "${expected_manifest}")
  endif()
  set(stored_manifest_path "${manifest_newline_root}/provenance/evidence-files.sha256")
  file(WRITE "${stored_manifest_path}" "${stored_manifest_text}")
  file(SHA256 "${stored_manifest_path}" stored_manifest_sha)
  phase20_verify_evidence_manifest("${manifest_newline_root}" "${stored_manifest_sha}"
    manifest_verify_error)
  if(NOT manifest_verify_error STREQUAL "")
    message(FATAL_ERROR
      "CRLF-formatted evidence checksum manifest was rejected: ${manifest_verify_error}")
  endif()
  file(WRITE "${manifest_newline_root}/artifacts/sample.txt" "changed evidence bytes\n")
  phase20_verify_evidence_manifest("${manifest_newline_root}" "${stored_manifest_sha}"
    manifest_verify_error)
  if(NOT manifest_verify_error MATCHES "checksum mismatch")
    message(FATAL_ERROR "changed evidence bytes were accepted: ${manifest_verify_error}")
  endif()

  nesturbator_phase20_validate_evidence("${FIXTURE}" "${SOURCE_DIR}" ok reason TRUE)
  if(NOT ok OR NOT reason STREQUAL "fixture-pass")
    message(FATAL_ERROR "positive hosted evidence fixture was rejected: ${reason}")
  endif()
  nesturbator_phase20_validate_evidence("${FIXTURE}" "${SOURCE_DIR}" hosted_ok hosted_reason FALSE)
  if(hosted_ok OR NOT hosted_reason MATCHES "fixture provenance cannot establish hosted evidence")
    message(FATAL_ERROR "offline fixture was incorrectly accepted as hosted evidence: ${hosted_reason}")
  endif()
  set(missing_delivery_root "${WORK}/missing-delivery")
  file(REMOVE_RECURSE "${missing_delivery_root}")
  nesturbator_phase20_collect_evidence("${missing_delivery_root}" "${SOURCE_DIR}" collect_ok collect_reason)
  if(collect_ok OR NOT collect_reason MATCHES "pending_delivery" OR
     EXISTS "${missing_delivery_root}/manifest.txt")
    message(FATAL_ERROR "missing live delivery IDs did not remain pending without a hosted manifest")
  endif()

  function(phase20_expect_rejection case_name expected_text)
    nesturbator_phase20_write_evidence_fixture("${FIXTURE}" "${SOURCE_DIR}" "${RUNNER}" "${ROM}")
    set(artifact_json "${FIXTURE}/provenance/ci-artifacts.json")
    set(campaign_meta "${FIXTURE}/artifacts/build-linux-x64/accuracycoin/full/metadata.txt")
    set(protected_path "${FIXTURE}/artifacts/build-linux-x64/protected-main/scoreboard.txt")
    set(perf_dir "${FIXTURE}/artifacts/perf")
    if(case_name STREQUAL "wrong-head-mapping")
      file(READ "${FIXTURE}/provenance/source-identity.txt" text)
      string(REPLACE "candidate_head_sha=cccccccccccccccccccccccccccccccccccccccc"
        "candidate_head_sha=eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee" text "${text}")
      file(WRITE "${FIXTURE}/provenance/source-identity.txt" "${text}")
    elseif(case_name STREQUAL "wrong-run-head")
      file(READ "${FIXTURE}/provenance/ci-run.json" text)
      string(JSON text SET "${text}" head_sha "\"eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\"")
      file(WRITE "${FIXTURE}/provenance/ci-run.json" "${text}")
    elseif(case_name STREQUAL "wrong-pr-merge")
      file(READ "${FIXTURE}/provenance/pr.json" text)
      string(JSON text SET "${text}" merge_commit_sha "\"eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\"")
      file(WRITE "${FIXTURE}/provenance/pr.json" "${text}")
    elseif(case_name STREQUAL "stale-pr-snapshot")
      file(READ "${FIXTURE}/provenance/pr.json" text)
      string(JSON text SET "${text}" head sha "\"eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\"")
      file(WRITE "${FIXTURE}/provenance/pr.json" "${text}")
    elseif(case_name STREQUAL "missing-matrix-job")
      file(READ "${FIXTURE}/provenance/ci-jobs.json" text)
      string(REPLACE "nofp (ubuntu-24.04)" "missing-nofp" text "${text}")
      file(WRITE "${FIXTURE}/provenance/ci-jobs.json" "${text}")
    elseif(case_name STREQUAL "wrong-build-matrix")
      file(READ "${FIXTURE}/provenance/ci-jobs.json" text)
      string(REPLACE "build (ubuntu-24.04, ci, ci-build, linux, x64)"
        "build (ubuntu-24.04, ci-build, linux, x64)" text "${text}")
      file(WRITE "${FIXTURE}/provenance/ci-jobs.json" "${text}")
    elseif(case_name STREQUAL "wrong-merge-parent")
      file(READ "${FIXTURE}/provenance/ci-commit.json" text)
      string(REPLACE "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee" text "${text}")
      file(WRITE "${FIXTURE}/provenance/ci-commit.json" "${text}")
    elseif(case_name STREQUAL "missing-platform")
      file(REMOVE_RECURSE "${FIXTURE}/artifacts/hashes-windows-arm64")
    elseif(case_name STREQUAL "duplicate-platform")
      file(MAKE_DIRECTORY "${FIXTURE}/artifacts/hashes-windows-arm64-copy")
      configure_file("${FIXTURE}/artifacts/hashes-windows-arm64/hashes.txt"
        "${FIXTURE}/artifacts/hashes-windows-arm64-copy/hashes.txt" COPYONLY)
    elseif(case_name STREQUAL "lost-protected-key")
      file(STRINGS "${protected_path}" rows)
      list(GET rows 0 first_row)
      string(REPLACE "\t" ";" fields "${first_row}")
      list(GET fields 0 key)
      file(READ "${FIXTURE}/artifacts/candidate-scoreboard.txt" text)
      string(REPLACE "${key}\tpass\t" "${key}\tfail\t" text "${text}")
      file(WRITE "${FIXTURE}/artifacts/candidate-scoreboard.txt" "${text}")
    elseif(case_name STREQUAL "dirty-campaign")
      file(READ "${campaign_meta}" text)
      string(REPLACE "source_dirty=0" "source_dirty=1" text "${text}")
      file(WRITE "${campaign_meta}" "${text}")
    elseif(case_name STREQUAL "stale-fingerprint")
      file(READ "${campaign_meta}" text)
      string(REGEX REPLACE "build_inputs_sha256=[0-9a-f]+"
        "build_inputs_sha256=0000000000000000000000000000000000000000000000000000000000000000" text "${text}")
      file(WRITE "${campaign_meta}" "${text}")
    elseif(case_name STREQUAL "absent-hash-key")
      set(hash_file "${FIXTURE}/artifacts/hashes-linux-arm64/hashes.txt")
      file(STRINGS "${hash_file}" rows)
      list(POP_BACK rows)
      list(JOIN rows "\n" text)
      file(WRITE "${hash_file}" "${text}\n")
    elseif(case_name STREQUAL "missing-artifact")
      file(READ "${artifact_json}" text)
      string(REPLACE "\"name\":\"hashes-linux-x64\"" "\"name\":\"missing-hashes-linux-x64\"" text "${text}")
      file(WRITE "${artifact_json}" "${text}")
    elseif(case_name STREQUAL "expired-artifact")
      file(READ "${artifact_json}" text)
      string(REPLACE "\"expired\":false" "\"expired\":true" text "${text}")
      file(WRITE "${artifact_json}" "${text}")
    elseif(case_name STREQUAL "skipped-job")
      file(READ "${FIXTURE}/provenance/ci-jobs.json" text)
      string(REPLACE "\"name\":\"hash-equality\",\"status\":\"completed\",\"conclusion\":\"success\""
        "\"name\":\"hash-equality\",\"status\":\"completed\",\"conclusion\":\"skipped\"" text "${text}")
      file(WRITE "${FIXTURE}/provenance/ci-jobs.json" "${text}")
    elseif(case_name STREQUAL "cancelled-job")
      file(READ "${FIXTURE}/provenance/ci-jobs.json" text)
      string(REPLACE "\"name\":\"hash-equality\",\"status\":\"completed\",\"conclusion\":\"success\""
        "\"name\":\"hash-equality\",\"status\":\"completed\",\"conclusion\":\"cancelled\"" text "${text}")
      file(WRITE "${FIXTURE}/provenance/ci-jobs.json" "${text}")
    elseif(case_name STREQUAL "na-performance")
      file(REMOVE "${perf_dir}/head_runner_360.out")
    elseif(case_name STREQUAL "base-spread")
      file(WRITE "${perf_dir}/base_b_runner_360.out" "version: 1\nsummary: 4000001\n")
    elseif(case_name STREQUAL "performance-regression")
      file(WRITE "${perf_dir}/head_runner_360.out" "version: 1\nsummary: 4040001\n")
    elseif(case_name STREQUAL "missing-host")
      file(REMOVE "${FIXTURE}/artifacts/retroarch/marker-progression.txt")
    elseif(case_name STREQUAL "missing-nightly")
      file(REMOVE "${FIXTURE}/artifacts/nightly/vectors-full/vectors-full.junit.xml")
    elseif(case_name STREQUAL "empty-job-log")
      set(job_log_case empty)
    elseif(case_name STREQUAL "near-miss-holy-mapperel")
      set(job_log_case near-miss)
    elseif(case_name STREQUAL "skipped-required-test")
      set(job_log_case skipped)
    elseif(case_name STREQUAL "failed-required-test")
      set(job_log_case failed)
    elseif(case_name STREQUAL "duplicate-required-test")
      set(job_log_case duplicate)
    elseif(case_name STREQUAL "contradictory-required-test")
      set(job_log_case contradictory)
    elseif(case_name STREQUAL "other-step-only-results")
      set(job_log_case other-step-only)
    elseif(case_name STREQUAL "wrong-attempt-job")
      file(READ "${FIXTURE}/provenance/build-linux-x64-job.json" text)
      string(REPLACE "\"run_attempt\":1" "\"run_attempt\":2" text "${text}")
      file(WRITE "${FIXTURE}/provenance/build-linux-x64-job.json" "${text}")
    else()
      message(FATAL_ERROR "unknown phase20 evidence control: ${case_name}")
    endif()
    if(DEFINED job_log_case)
      phase20_control_set_selected_job_log("${FIXTURE}" "${job_log_case}")
    endif()
    nesturbator_phase20_validate_evidence("${FIXTURE}" "${SOURCE_DIR}" accepted actual_reason TRUE)
    string(FIND "${actual_reason}" "${expected_text}" reason_position)
    if(accepted OR reason_position EQUAL -1)
      message(FATAL_ERROR "${case_name} was accepted or rejected at the wrong stage: ${actual_reason}")
    endif()
    message(STATUS "phase20 evidence control passed: ${case_name} -> ${actual_reason}")
  endfunction()

  phase20_expect_rejection("wrong-head-mapping" "PR candidate/base mapping")
  phase20_expect_rejection("wrong-run-head" "run head SHA does not match the PR candidate SHA")
  phase20_expect_rejection("wrong-pr-merge" "PR merge SHA differs from tested source identity")
  phase20_expect_rejection("stale-pr-snapshot" "PR snapshot does not match the run candidate/base identity")
  phase20_expect_rejection("missing-matrix-job" "CI job missing from hosted run: nofp (ubuntu-24.04)")
  phase20_expect_rejection("wrong-build-matrix" "CI platform job missing from hosted run: linux-x64")
  phase20_expect_rejection("wrong-merge-parent" "tested merge parents")
  phase20_expect_rejection("missing-platform" "hash artifact missing for platform windows-arm64")
  phase20_expect_rejection("duplicate-platform" "expected 6 platform artifacts, found 7")
  phase20_expect_rejection("lost-protected-key" "protected key lost in candidate scoreboard")
  phase20_expect_rejection("dirty-campaign" "metadata source_dirty mismatch")
  phase20_expect_rejection("stale-fingerprint" "campaign source fingerprint is stale")
  phase20_expect_rejection("absent-hash-key" "hash inventory content validation failed")
  phase20_expect_rejection("missing-artifact" "required artifact missing: hashes-linux-x64")
  phase20_expect_rejection("expired-artifact" "required artifact expired: hashes-linux-x64")
  phase20_expect_rejection("skipped-job" "skipped/cancelled/failed job: hash-equality")
  phase20_expect_rejection("cancelled-job" "skipped/cancelled/failed job: hash-equality")
  phase20_expect_rejection("na-performance" "performance raw measurement is missing")
  phase20_expect_rejection("base-spread" "performance base_a/base_b spread is nonzero")
  phase20_expect_rejection("performance-regression" "performance per-ROM slowdown exceeds +1.00%")
  phase20_expect_rejection("missing-host" "RetroArch marker-progression artifact missing")
  phase20_expect_rejection("missing-nightly" "Nightly vectors-full inventory/JUnit evidence missing or ambiguous")
  phase20_expect_rejection("empty-job-log" "job execution log is missing passing accuracycoin.protected_observation")
  phase20_expect_rejection("near-miss-holy-mapperel" "job execution log is missing passing holymapperel.m2.decode")
  phase20_expect_rejection("skipped-required-test" "recorded Skipped for holymapperel.m2.decode")
  phase20_expect_rejection("failed-required-test" "recorded Failed for holymapperel.m2.decode")
  phase20_expect_rejection("duplicate-required-test" "duplicate terminal CTest result for holymapperel.m2.decode")
  phase20_expect_rejection("contradictory-required-test" "duplicate terminal CTest result for holymapperel.m2.decode")
  phase20_expect_rejection("other-step-only-results" "job execution log is missing passing accuracycoin.protected_observation")
  phase20_expect_rejection("wrong-attempt-job" "does not bind the selected successful run/attempt")

  set(suffix_root "${WORK}/suffix-normalization")
  set(suffix_output "${WORK}/suffix-normalized.txt")
  file(REMOVE_RECURSE "${suffix_root}" "${WORK}/provenance")
  file(MAKE_DIRECTORY "${suffix_root}/bundle" "${WORK}/provenance")
  file(WRITE "${suffix_root}/data.txt" "raw bytes\n")
  phase20_copy_unique("${suffix_root}" "data.txt" "${suffix_output}"
    "${WORK}" "suffix-control" error)
  if(NOT error STREQUAL "" OR NOT EXISTS "${WORK}/provenance/normalization.tsv")
    message(FATAL_ERROR "root-level depth-zero suffix failed normalization: ${error}")
  endif()
  file(WRITE "${suffix_root}/bundle/data.txt" "different raw bytes\n")
  phase20_copy_unique("${suffix_root}" "data.txt" "${suffix_output}"
    "${WORK}" "suffix-control" error)
  if(NOT error MATCHES "found 2")
    message(FATAL_ERROR "root-plus-nested suffix ambiguity was not rejected: ${error}")
  endif()

  file(REMOVE_RECURSE "${suffix_root}")
  file(MAKE_DIRECTORY "${suffix_root}/bundle/perf" "${suffix_root}/bundle/nightly")
  file(WRITE "${suffix_root}/summary.md" "perf root\n")
  file(WRITE "${suffix_root}/bundle/perf/summary.md" "perf nested\n")
  phase20_copy_unique("${suffix_root}" "summary.md" "${suffix_output}"
    "${WORK}" "perf-collision" error)
  if(NOT error MATCHES "found 2")
    message(FATAL_ERROR "root/nested performance file collision was not rejected: ${error}")
  endif()
  file(WRITE "${suffix_root}/registered-tests.json" "nightly root\n")
  file(WRITE "${suffix_root}/bundle/nightly/registered-tests.json" "nightly nested\n")
  phase20_copy_unique("${suffix_root}" "registered-tests.json" "${suffix_output}"
    "${WORK}" "nightly-collision" error)
  if(NOT error MATCHES "found 2")
    message(FATAL_ERROR "root/nested Nightly inventory collision was not rejected: ${error}")
  endif()
  phase20_copy_unique("${suffix_root}" "missing-content.txt" "${suffix_output}"
    "${WORK}" "missing-content" error)
  if(NOT error MATCHES "found 0")
    message(FATAL_ERROR "missing artifact content was not rejected: ${error}")
  endif()

  set(unsafe_stage "${WORK}/unsafe-archive")
  file(REMOVE_RECURSE "${unsafe_stage}")
  file(MAKE_DIRECTORY "${unsafe_stage}")
  file(WRITE "${WORK}/outside-sentinel.txt" "unchanged\n")
  file(CREATE_LINK "${WORK}/outside-sentinel.txt" "${unsafe_stage}/unsafe-link" SYMBOLIC)
  execute_process(COMMAND "${CMAKE_COMMAND}" -E tar cf "${WORK}/unsafe.zip"
      --format=zip unsafe-link WORKING_DIRECTORY "${unsafe_stage}"
    RESULT_VARIABLE unsafe_zip_result OUTPUT_VARIABLE unsafe_zip_stdout ERROR_VARIABLE unsafe_zip_stderr)
  if(NOT "${unsafe_zip_result}" STREQUAL "0")
    message(FATAL_ERROR "cannot create symbolic-link archive control: ${unsafe_zip_stdout}${unsafe_zip_stderr}")
  endif()
  phase20_check_archive_members("${WORK}/unsafe.zip" unsafe_entries unsafe_error)
  file(READ "${WORK}/outside-sentinel.txt" outside_contents)
  if(NOT unsafe_error MATCHES "symbolic-link" OR NOT outside_contents STREQUAL "unchanged\n")
    message(FATAL_ERROR "archive symlink was not rejected before touching its target: ${unsafe_error}")
  endif()

  string(ASCII 239 187 191 ambiguous_bom)
  file(WRITE "${WORK}/ambiguous-job-log.txt"
    "${ambiguous_bom}2026-10-26T10:00:10.1000000Z ##[group]Run cmake --workflow --preset ci-build\n"
    "2026-10-26T10:00:11.1000000Z 1/6 Test #1: accuracycoin.protected_observation .... Passed 0.01 sec\n"
    "2026-10-26T10:00:12.1000000Z ##[group]Run cmake --workflow --preset ci-build\n"
    "2026-10-26T10:00:13.1000000Z 1/6 Test #1: accuracycoin.protected_observation .... Passed 0.01 sec\n")
  phase20_extract_job_log("${WORK}/ambiguous-job-log.txt" "linux-x64" 2026-10-26T10:00:10Z
    2026-10-26T10:01:00Z "${WORK}/ambiguous-job.txt" ambiguous_error)
  if(NOT ambiguous_error MATCHES "no unique selected workflow-step group")
    message(FATAL_ERROR "duplicate workflow-step groups were not rejected: ${ambiguous_error}")
  endif()

  nesturbator_phase20_write_evidence_fixture("${FIXTURE}" "${SOURCE_DIR}" "${RUNNER}" "${ROM}")
  file(READ "${SOURCE_DIR}/tests/cmake/phase20_hosted_evidence.cmake" source_before)
  file(APPEND "${SOURCE_DIR}/tests/cmake/phase20_hosted_evidence.cmake" "\n# phase20 post-test source-change control\n")
  nesturbator_phase20_validate_evidence("${FIXTURE}" "${SOURCE_DIR}" accepted source_reason TRUE)
  file(WRITE "${SOURCE_DIR}/tests/cmake/phase20_hosted_evidence.cmake" "${source_before}")
  if(accepted OR NOT source_reason MATCHES "source digest mismatch")
    message(FATAL_ERROR "post-test source change was accepted or rejected at the wrong stage: ${source_reason}")
  endif()
  message(STATUS "phase20 evidence control passed: post-test source changes invalidate the recorded digest")
  message(STATUS "phase20 hosted evidence fixture and named rejection controls passed")
endif()
