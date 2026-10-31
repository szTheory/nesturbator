# The hygiene lane (FRAME-06, ENGINEERING sections 5 and 7). Registered only
# when NESTURBATOR_HYGIENE is ON; every test carries the label hygiene.

# The forbidden-content scan over every file git would track: personal data,
# game images, unlisted binary files and the GSD stop policy.
add_test(NAME hygiene.tree
  COMMAND ${NESTURBATOR_SH} ${PROJECT_SOURCE_DIR}/scripts/hygiene.sh --tree
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

# The scan finds personal data in a scratch repository whose files are easy to
# miss, and fails closed on a file it cannot read.
add_test(NAME hygiene.scan_selftest
  COMMAND ${NESTURBATOR_SH} ${CMAKE_CURRENT_SOURCE_DIR}/scan_selftest.sh
    ${PROJECT_SOURCE_DIR}/scripts/hygiene.sh ${CMAKE_CURRENT_BINARY_DIR}/scan_selftest)

# Every uses: key under .github names an action by a full commit SHA. The two
# fixtures show the check fails on a tag and passes on a SHA.
set(pins ${PROJECT_SOURCE_DIR}/tests/cmake/action_pins.cmake)
add_test(NAME hygiene.action_pins
  COMMAND ${CMAKE_COMMAND} -DROOT=${PROJECT_SOURCE_DIR}/.github -P ${pins})
add_test(NAME hygiene.action_pins.bad
  COMMAND ${CMAKE_COMMAND} -DROOT=${CMAKE_CURRENT_SOURCE_DIR}/fixtures
    -DFILE=pins_bad.yml -P ${pins})
set_tests_properties(hygiene.action_pins.bad PROPERTIES WILL_FAIL TRUE)
add_test(NAME hygiene.action_pins.good
  COMMAND ${CMAKE_COMMAND} -DROOT=${CMAKE_CURRENT_SOURCE_DIR}/fixtures
    -DFILE=pins_good.yml -P ${pins})

# Every timeout-minutes line names the run it was measured from. The two
# fixtures show the check fails on a bare value and passes on a run ID.
set(timeouts ${PROJECT_SOURCE_DIR}/tests/cmake/timeout_comments.cmake)
add_test(NAME hygiene.timeout_comments
  COMMAND ${CMAKE_COMMAND} -DROOT=${PROJECT_SOURCE_DIR}/.github/workflows -P ${timeouts})
add_test(NAME hygiene.timeout_comments.bad
  COMMAND ${CMAKE_COMMAND} -DROOT=${CMAKE_CURRENT_SOURCE_DIR}/fixtures
    -DFILE=timeouts_bad.yml -P ${timeouts})
set_tests_properties(hygiene.timeout_comments.bad PROPERTIES WILL_FAIL TRUE)
add_test(NAME hygiene.timeout_comments.good
  COMMAND ${CMAKE_COMMAND} -DROOT=${CMAKE_CURRENT_SOURCE_DIR}/fixtures
    -DFILE=timeouts_good.yml -P ${timeouts})

# The nightly full-vector lane must remain cold, non-skippable and least
# privileged. Synthetic mutations exercise the same classifier as the file.
add_test(NAME hygiene.nightly_workflow_policy
  COMMAND ${CMAKE_COMMAND}
    -DROOT=${PROJECT_SOURCE_DIR}
    -DREPLAY_BINARY=$<TARGET_FILE:fuzz.execution>
    -DWORKFLOW=${PROJECT_SOURCE_DIR}/.github/workflows/nightly.yml
    -P ${PROJECT_SOURCE_DIR}/tests/cmake/nightly_workflow_policy.cmake)
add_test(NAME hygiene.nightly_workflow_policy.selftest
  COMMAND ${CMAKE_COMMAND} -DSELFTEST=ON
    -P ${PROJECT_SOURCE_DIR}/tests/cmake/nightly_workflow_policy.cmake)

# ci-required lists every other job, and its jq roll-up accepts a skipped perf
# only off pull requests. The script runs the expression taken from ci.yml.
add_test(NAME hygiene.ci_required_rollup
  COMMAND ${CMAKE_COMMAND}
    -DCI_YML=${PROJECT_SOURCE_DIR}/.github/workflows/ci.yml -DJQ=${NESTURBATOR_JQ}
    -DWORK=${CMAKE_CURRENT_BINARY_DIR}/ci_required_rollup
    -P ${PROJECT_SOURCE_DIR}/tests/cmake/ci_required_rollup.cmake)

add_test(NAME hygiene.ci_release_push_policy
  COMMAND ${CMAKE_COMMAND} -DROOT=${PROJECT_SOURCE_DIR}
    -P ${PROJECT_SOURCE_DIR}/tests/cmake/ci_release_push_policy.cmake)

# Every tracked C source but the vendored libretro.h is formatted as
# .clang-format says, checked with clang-format 18.
add_test(NAME hygiene.format
  COMMAND ${CMAKE_COMMAND} -DCLANG_FORMAT=${NESTURBATOR_CLANG_FORMAT} -DGIT=${NESTURBATOR_GIT}
    -DROOT=${PROJECT_SOURCE_DIR} -P ${PROJECT_SOURCE_DIR}/tests/cmake/format_check.cmake)

add_test(NAME hygiene.release_version_format
  COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
    -DCLANG_FORMAT=${NESTURBATOR_CLANG_FORMAT}
    -DWORK=${CMAKE_CURRENT_BINARY_DIR}/release_version_format
    -P ${PROJECT_SOURCE_DIR}/tests/cmake/release_version_format.cmake)

set_tests_properties(
  hygiene.tree hygiene.scan_selftest hygiene.action_pins hygiene.action_pins.bad hygiene.action_pins.good
  hygiene.timeout_comments hygiene.timeout_comments.bad hygiene.timeout_comments.good
  hygiene.nightly_workflow_policy hygiene.nightly_workflow_policy.selftest hygiene.format
  hygiene.ci_required_rollup hygiene.ci_release_push_policy hygiene.release_version_format
  PROPERTIES LABELS hygiene)

set_tests_properties(hygiene.ci_release_push_policy PROPERTIES TIMEOUT 5)
