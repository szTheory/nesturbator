#!/bin/sh
# Deterministic controls for the production RetroArch session driver. The fake host and
# transport are this same script reached through private test-only symlinks.
set -eu

fake_nc() {
    request=$(cat)
    printf '%s\n' "$request" >>"$STATE_SESSION_TEST_SENDS"
    fault=${STATE_SESSION_TEST_FAULT:-}
    if [ "$fault" = send-error ] && [ "$request" = "${STATE_SESSION_TEST_FAIL_REQUEST:-}" ]; then exit 93; fi
    case "$request" in
        'GET_CONFIG_PARAM savestate_directory')
            if [ "$fault" = stalled-read ]; then
                printf '%s\n' "$$" >"$STATE_SESSION_TEST_STALL_PID"
                exec perl -MTime::HiRes=sleep -e 'sleep 5'
            fi
            if [ "$fault" = stale-publication ]; then
                cp "$STATE_SESSION_TEST_FIXTURE/pause capture one.ppm" "$STATE_SESSION_TEST_SHOTS/stale.ppm"
            fi
            read_count=0
            [ ! -f "$STATE_SESSION_TEST_READ_COUNT" ] || read_count=$(cat "$STATE_SESSION_TEST_READ_COUNT")
            read_count=$((read_count + 1))
            printf '%s\n' "$read_count" >"$STATE_SESSION_TEST_READ_COUNT"
            if [ "$read_count" -eq 1 ] && [ "${STATE_SESSION_TEST_DROP_FIRST_READ:-0}" = 1 ]; then exit 0; fi
            if [ "$fault" = wrong-readiness ]; then
                printf 'GET_CONFIG_PARAM savestate_directory /wrong/path\n'
                exit 0
            fi
            printf 'GET_CONFIG_PARAM savestate_directory %s\n' "$STATE_SESSION_TEST_EXPECTED_DIR"
            ;;
        PAUSE_TOGGLE|FRAMEADVANCE|LOAD_STATE_SLOT\ 0|RUNAHEAD_TOGGLE|REWIND|SAVE_STATE|SCREENSHOT|QUIT)
            case "$request" in
                SAVE_STATE)
                    state_count=0
                    [ ! -f "$STATE_SESSION_TEST_STATE_COUNT" ] || state_count=$(cat "$STATE_SESSION_TEST_STATE_COUNT")
                    state_count=$((state_count + 1))
                    printf '%s\n' "$state_count" >"$STATE_SESSION_TEST_STATE_COUNT"
                    mkdir -p "$STATE_SESSION_TEST_EXPECTED_DIR/nesturbator"
                    if [ "$fault" = missing-state ]; then exit 0; fi
                    case "$state_count" in
                        1) state_source="$STATE_SESSION_TEST_FIXTURE/retained original anchor.ra" ;;
                        2) state_source="$STATE_SESSION_TEST_FIXTURE/retained rewound anchor.ra" ;;
                        *) exit 93 ;;
                    esac
                    if [ "$fault" = corrupt-state ]; then printf 'not a state container\n' >"$STATE_SESSION_TEST_EXPECTED_DIR/nesturbator/marker.state"; exit 0; fi
                    if [ "$fault" = wrong-save-origin ] && [ "$state_count" -eq 1 ]; then
                        state_source="$STATE_SESSION_TEST_FIXTURE/valid wrong origin.ra"
                    fi
                    cp "$state_source" "$STATE_SESSION_TEST_EXPECTED_DIR/nesturbator/marker.state"
                    if [ "$fault" = duplicate-state ]; then mkdir -p "$STATE_SESSION_TEST_EXPECTED_DIR/other"; cp "$state_source" "$STATE_SESSION_TEST_EXPECTED_DIR/other/marker.state"; fi
                    ;;
                SCREENSHOT)
                    if [ "$fault" = missing-image ]; then exit 0; fi
                    count=0
                    [ ! -f "$STATE_SESSION_TEST_SHOT_COUNT" ] || count=$(cat "$STATE_SESSION_TEST_SHOT_COUNT")
                    count=$((count + 1))
                    printf '%s\n' "$count" >"$STATE_SESSION_TEST_SHOT_COUNT"
                    if [ "${STATE_SESSION_TEST_MODE:-prefix}" = complete ]; then
                        case "$count" in
                            1) name='boundary 01 pause a.ppm' ;;
                            2) name='boundary 02 pause b.ppm' ;;
                            3) name='boundary 03 advance 1.ppm' ;;
                            4) name='boundary 04 advance 2.ppm' ;;
                            5) name='boundary 05 advance 3.ppm' ;;
                            6) name='boundary 06 advance 4.ppm' ;;
                            7) name='boundary 07 advance 5.ppm' ;;
                            8) name='boundary 08 load probe.ppm' ;;
                            9) name='boundary 09 post load 2.ppm' ;;
                            10) name='boundary 10 post load 3.ppm' ;;
                            11) name='boundary 11 resume forward.ppm' ;;
                            12) name='boundary 12 rewind coarse.ppm' ;;
                            13) name='boundary 13 rewound advance 1.ppm' ;;
                            14) name='boundary 14 rewound advance 2.ppm' ;;
                            15) name='boundary 15 rewound advance 3.ppm' ;;
                            16) name='boundary 16 runahead probe.ppm' ;;
                            *) exit 91 ;;
                        esac
                        source_image="$STATE_SESSION_TEST_FIXTURE/$name"
                    else
                        case "$count" in
                            1)
                                if [ "$fault" = dropped-pause ]; then source_image="$STATE_SESSION_TEST_FIXTURE/advance one.ppm"; else source_image="$STATE_SESSION_TEST_FIXTURE/pause capture one.ppm"; fi
                                ;;
                            2) source_image="$STATE_SESSION_TEST_FIXTURE/pause capture one.ppm" ;;
                            3)
                                if [ "${STATE_SESSION_TEST_DROP_ADVANCE:-0}" = 1 ] || [ "$fault" = dropped-advance ]; then
                                    source_image="$STATE_SESSION_TEST_FIXTURE/pause capture one.ppm"
                                elif [ "$fault" = doubled-advance ]; then
                                    source_image="$STATE_SESSION_TEST_FIXTURE/boundary 04 advance 2.ppm"
                                else
                                    source_image="$STATE_SESSION_TEST_FIXTURE/advance one.ppm"
                                fi
                                ;;
                            *) exit 91 ;;
                        esac
                    fi
                    if [ "$fault" = wrong-load-origin ] && [ "$count" -eq 8 ]; then
                        source_image="$STATE_SESSION_TEST_FIXTURE/boundary 09 post load 2.ppm"
                    fi
                    if [ "$fault" = truncated-image ]; then head -c 8 "$source_image" >"$STATE_SESSION_TEST_SHOTS/capture-$count.ppm"; exit 0; fi
                    cp "$source_image" "$STATE_SESSION_TEST_SHOTS/capture-$count.ppm"
                    if [ "$fault" = extra-image ]; then cp "$source_image" "$STATE_SESSION_TEST_SHOTS/extra-$count.ppm"; fi
                    ;;
                QUIT)
                    [ "$fault" = dropped-quit ] || : >"$STATE_SESSION_TEST_QUIT_FILE"
                    ;;
            esac
            ;;
        *)
            echo "unexpected fake transport request: $request" >&2
            exit 92
            ;;
    esac
}

fake_host() {
    printf '%s\n' "$$" >"$STATE_SESSION_TEST_HOST_PID"
    if [ "${STATE_SESSION_TEST_HOST_DIE:-0}" = 1 ]; then sleep 0.5; exit 0; fi
    while [ ! -f "$STATE_SESSION_TEST_QUIT_FILE" ]; do sleep 0.05; done
}

case "${0##*/}" in
    fake-nc) fake_nc; exit $? ;;
    fake-retroarch) fake_host; exit $? ;;
esac

if [ "$#" -ne 3 ]; then
    echo "usage: test_state_session.sh DRIVER FRAME_INDEX WORK_ROOT" >&2
    exit 2
fi
DRIVER=$1
FRAME_INDEX=$2
WORK_ROOT=$3

test_dir="$WORK_ROOT/session-$$"
fixture_dir="$test_dir/fixture"
ra_dir="$test_dir/ra"
mkdir -p "$fixture_dir" "$ra_dir/shots" "$ra_dir/states"
if ! "$FRAME_INDEX" --progression-selftest "$fixture_dir" --controls; then
    echo "fixture precondition failed: frame_index --progression-selftest --controls" >&2
    exit 2
fi

script_path=$(cd "$(dirname "$0")" && pwd -P)/$(basename "$0")
ln -s "$script_path" "$test_dir/fake-nc"
ln -s "$script_path" "$test_dir/fake-retroarch"
: >"$test_dir/config.cfg"
: >"$test_dir/core.so"
expected_state_dir="$ra_dir/states"
send_log="$test_dir/sends.log"
host_pid_file="$test_dir/host.pid"
quit_file="$test_dir/quit"
shot_count_file="$test_dir/shot.count"
read_count_file="$test_dir/read.count"
state_count_file="$test_dir/state.count"
driver_output="$test_dir/driver.out"

status=0
STATE_SESSION_NC="$test_dir/fake-nc" \
STATE_SESSION_TEST_MODE=1 \
STATE_SESSION_TEST_FIXTURE="$fixture_dir" \
STATE_SESSION_TEST_EXPECTED_DIR="$expected_state_dir" \
STATE_SESSION_TEST_SENDS="$send_log" \
STATE_SESSION_TEST_HOST_PID="$host_pid_file" \
STATE_SESSION_TEST_QUIT_FILE="$quit_file" \
STATE_SESSION_TEST_SHOT_COUNT="$shot_count_file" \
STATE_SESSION_TEST_READ_COUNT="$read_count_file" \
STATE_SESSION_TEST_STATE_COUNT="$state_count_file" \
STATE_SESSION_TEST_DROP_FIRST_READ=1 \
STATE_SESSION_TEST_MODE=prefix \
STATE_SESSION_TEST_STOP_AFTER_PREFIX=1 \
STATE_SESSION_TEST_SHOTS="$ra_dir/shots" \
STATE_SESSION_LOCK_ROOT="$test_dir/locks" \
STATE_SESSION_SETTLE_MS=0 \
sh "$DRIVER" "$test_dir/fake-retroarch" "$test_dir/config.cfg" "$test_dir/core.so" \
    "$fixture_dir/marker.nes" "$ra_dir" "$test_dir/retroarch.log" "$FRAME_INDEX" \
    "$fixture_dir/calibration.txt" >"$driver_output" 2>&1 || status=$?

if [ "$status" -ne 0 ] || ! grep -F 'verdict=PREFIX_OK' "$driver_output" >/dev/null 2>&1; then
    echo "FAIL actual shell tracer reaches the production prefix verdict: expected PREFIX_OK, driver exit=$status" >&2
    cat "$driver_output" >&2
    exit 1
fi

count_request() {
    awk -v wanted="$1" '$0 == wanted { n++ } END { print n + 0 }' "$send_log"
}
for request in PAUSE_TOGGLE SAVE_STATE FRAMEADVANCE QUIT; do
    count=$(count_request "$request")
    [ "$count" -eq 1 ] || {
        echo "FAIL one-shot $request send count: expected 1, got $count" >&2
        exit 1
    }
done
count=$(count_request SCREENSHOT)
[ "$count" -eq 3 ] || {
    echo "FAIL screenshot publication request count: expected 3, got $count" >&2
    exit 1
}
count=$(count_request 'GET_CONFIG_PARAM savestate_directory')
[ "$count" -eq 2 ] || {
    echo "FAIL read-only readiness retry count: expected 2, got $count" >&2
    exit 1
}

printf 'state session tracer: PREFIX_OK; one FRAMEADVANCE, 3 attributed captures, one QUIT\n'

# A dropped frame-advance is terminal even though the screenshot request succeeds. It is
# attempted once, fails on the wrong decoded frame, and the owned host is reaped within budget.
negative_dir="$WORK_ROOT/session-$$-missing-action"
negative_fixture="$fixture_dir"
negative_ra="$negative_dir/ra"
mkdir -p "$negative_ra/shots" "$negative_ra/states"
ln -s "$script_path" "$negative_dir/fake-nc"
ln -s "$script_path" "$negative_dir/fake-retroarch"
: >"$negative_dir/config.cfg"
: >"$negative_dir/core.so"
negative_sends="$negative_dir/sends.log"
negative_host="$negative_dir/host.pid"
negative_quit="$negative_dir/quit"
negative_reads="$negative_dir/read.count"
negative_shots="$negative_dir/shot.count"
negative_states="$negative_dir/state.count"
started=$(perl -MTime::HiRes=clock_gettime,CLOCK_MONOTONIC -e 'printf "%.0f\n",clock_gettime(CLOCK_MONOTONIC)*1000')
negative_status=0
STATE_SESSION_NC="$negative_dir/fake-nc" \
STATE_SESSION_TEST_FIXTURE="$negative_fixture" \
STATE_SESSION_TEST_EXPECTED_DIR="$negative_ra/states" \
STATE_SESSION_TEST_SENDS="$negative_sends" \
STATE_SESSION_TEST_HOST_PID="$negative_host" \
STATE_SESSION_TEST_QUIT_FILE="$negative_quit" \
STATE_SESSION_TEST_SHOT_COUNT="$negative_shots" \
STATE_SESSION_TEST_READ_COUNT="$negative_reads" \
STATE_SESSION_TEST_STATE_COUNT="$negative_states" \
STATE_SESSION_TEST_DROP_ADVANCE=1 \
STATE_SESSION_TEST_MODE=prefix \
STATE_SESSION_TEST_STOP_AFTER_PREFIX=1 \
STATE_SESSION_TEST_SHOTS="$negative_ra/shots" \
STATE_SESSION_LOCK_ROOT="$negative_dir/locks" \
STATE_SESSION_SETTLE_MS=0 \
STATE_SESSION_OPERATION_MS=10000 \
sh "$DRIVER" "$negative_dir/fake-retroarch" "$negative_dir/config.cfg" "$negative_dir/core.so" \
    "$negative_fixture/marker.nes" "$negative_ra" "$negative_dir/retroarch.log" "$FRAME_INDEX" \
    "$negative_fixture/calibration.txt" >"$negative_dir/driver.out" 2>&1 || negative_status=$?
finished=$(perl -MTime::HiRes=clock_gettime,CLOCK_MONOTONIC -e 'printf "%.0f\n",clock_gettime(CLOCK_MONOTONIC)*1000')
duration=$((finished - started))
[ "$negative_status" -ne 0 ] && grep -Fx 'state_session: ADVANCE_NOT_EXACTLY_ONE' "$negative_dir/driver.out" >/dev/null || {
    echo "FAIL dropped advance reaches its exact terminal reason (status=$negative_status)" >&2
    cat "$negative_dir/driver.out" >&2
    exit 1
}
[ "$duration" -lt 10000 ] || { echo "FAIL dropped advance exceeded elapsed bound: ${duration}ms" >&2; exit 1; }
advance_count=$(awk '$0 == "FRAMEADVANCE" { n++ } END { print n + 0 }' "$negative_sends")
[ "$advance_count" -eq 1 ] || { echo "FAIL dropped advance send count: expected 1, got $advance_count" >&2; exit 1; }
host_pid=$(cat "$negative_host")
if kill -0 "$host_pid" 2>/dev/null; then echo "FAIL dropped advance left the owned host alive" >&2; exit 1; fi
printf 'state session missing-action control: ADVANCE_NOT_EXACTLY_ONE; one send; %sms; host reaped\n' "$duration"

mono_ms() { perl -MTime::HiRes=clock_gettime,CLOCK_MONOTONIC -e 'printf "%.0f\n",clock_gettime(CLOCK_MONOTONIC)*1000'; }
assert_sanitized_tree() {
    _attempt=$1
    for _file in journal.tsv session.sanitized.cfg retroarch.sanitized.log; do
        [ -f "$_attempt/$_file" ] || { echo "FAIL missing sanitized $_file" >&2; exit 1; }
        if grep -E -F -e "$WORK_ROOT" -e '/Users/' -e '/home/' "$_attempt/$_file" >/dev/null; then
            echo "FAIL unsanitized path in $_attempt/$_file" >&2; exit 1
        fi
    done
    [ ! -e "$_attempt/session.cfg" ] && [ ! -e "$_attempt/retroarch.log" ] || {
        echo "FAIL raw generated config/log retained in $_attempt" >&2; exit 1;
    }
}
run_fault() {
    name=$1; expected=$2; fault=$3; fail_request=${4:-}; mode=${5:-prefix}; max_ms=${6:-20000}; ready_ms=${7:-30000}; publication_ms=${8:-300}; attempt_override=${9:-}
    case_dir="$WORK_ROOT/session-$$-$name"; case_ra="$case_dir/ra"
    case_host_die=0; [ "$fault" != host-died ] || case_host_die=1
    mkdir -p "$case_ra/shots" "$case_ra/states"
    ln -s "$script_path" "$case_dir/fake-nc"; ln -s "$script_path" "$case_dir/fake-retroarch"
    : >"$case_dir/config.cfg"; : >"$case_dir/core.so"
    case_started=$(mono_ms); case_status=0
    if [ "$mode" = complete ]; then stop_after=0; attempt_ms=100000; else stop_after=1; attempt_ms=30000; fi
    [ -z "$attempt_override" ] || attempt_ms=$attempt_override
    STATE_SESSION_NC="$case_dir/fake-nc" \
    STATE_SESSION_TEST_FIXTURE="$fixture_dir" \
    STATE_SESSION_TEST_EXPECTED_DIR="$case_ra/states" \
    STATE_SESSION_TEST_SENDS="$case_dir/sends.log" \
    STATE_SESSION_TEST_HOST_PID="$case_dir/host.pid" \
    STATE_SESSION_TEST_QUIT_FILE="$case_dir/quit" \
    STATE_SESSION_TEST_SHOT_COUNT="$case_dir/shot.count" \
    STATE_SESSION_TEST_READ_COUNT="$case_dir/read.count" \
    STATE_SESSION_TEST_STATE_COUNT="$case_dir/state.count" \
    STATE_SESSION_TEST_STALL_PID="$case_dir/stall.pid" \
    STATE_SESSION_TEST_FAIL_REQUEST="$fail_request" \
    STATE_SESSION_TEST_FAULT="$fault" \
    STATE_SESSION_TEST_MODE="$mode" \
    STATE_SESSION_TEST_STOP_AFTER_PREFIX="$stop_after" \
    STATE_SESSION_TEST_SHOTS="$case_ra/shots" \
    STATE_SESSION_TEST_HOST_DIE="$case_host_die" \
    STATE_SESSION_LOCK_ROOT="$case_dir/locks" \
    STATE_SESSION_ATTEMPT_MS="$attempt_ms" \
    STATE_SESSION_OPERATION_MS=30000 \
    STATE_SESSION_CLEANUP_MS=1000 \
    STATE_SESSION_READY_MS="$ready_ms" \
    STATE_SESSION_PUBLICATION_MS="$publication_ms" \
    STATE_SESSION_SETTLE_MS=0 \
    STATE_SESSION_PROGRESS_WAIT_MS=20 \
    STATE_SESSION_QUIT_GRACE_MS=300 \
    sh "$DRIVER" "$case_dir/fake-retroarch" "$case_dir/config.cfg" "$case_dir/core.so" \
        "$fixture_dir/marker.nes" "$case_ra" "$case_dir/retroarch.log" "$FRAME_INDEX" \
        "$fixture_dir/calibration.txt" >"$case_dir/driver.out" 2>&1 || case_status=$?
    case_finished=$(mono_ms); case_duration=$((case_finished-case_started))
    if [ "$case_status" -eq 0 ] || ! grep -Fx "state_session: $expected" "$case_dir/driver.out" >/dev/null ||
        ! awk -F '\t' -v reason="$expected" '$11 == reason { found=1 } END { exit !found }' "$case_ra"/attempts/*/journal.tsv; then
        echo "FAIL $name expected exact terminal reason $expected (status=$case_status, ${case_duration}ms)" >&2
        cat "$case_dir/driver.out" >&2
        exit 1
    fi
    [ "$case_duration" -lt "$max_ms" ] || { echo "FAIL $name elapsed bound: ${case_duration}ms >= ${max_ms}ms" >&2; exit 1; }
    case_host=$(cat "$case_dir/host.pid")
    if kill -0 "$case_host" 2>/dev/null; then echo "FAIL $name left owned host alive" >&2; exit 1; fi
    set -- "$case_ra"/attempts/*
    [ -d "$1" ] || { echo "FAIL $name did not retain its attempt tree" >&2; exit 1; }
    case_attempt=$1
    assert_sanitized_tree "$case_attempt"
    case "$fault" in
        stalled-read)
            [ -s "$case_dir/stall.pid" ] || { echo "FAIL stalled-read had no child pid evidence" >&2; exit 1; }
            stall_pid=$(cat "$case_dir/stall.pid")
            if kill -0 "$stall_pid" 2>/dev/null; then echo "FAIL stalled-read did not reap the blocked read child" >&2; exit 1; fi
            if [ "$expected" = READINESS_DEADLINE ]; then
                stall_row=$(grep -F 'READINESS_DEADLINE' "$1/journal.tsv" | tail -1)
                stall_elapsed=$(printf '%s\n' "$stall_row" | cut -f6)
                [ "$stall_elapsed" -gt 0 ] && [ "$stall_elapsed" -le 800 ] || { echo "FAIL readiness child did not obey measured monotonic budget: ${stall_elapsed}ms" >&2; exit 1; }
                printf 'state session child deadline: %sms operation elapsed; child reaped\n' "$stall_elapsed"
            else
                attempt_elapsed=$(grep -F 'ATTEMPT_DEADLINE' "$1/journal.tsv" | tail -1 | cut -f4)
                [ "$attempt_elapsed" -gt 0 ] && [ "$attempt_elapsed" -lt 2000 ] || { echo "FAIL child exceeded attempt budget before cleanup reserve: ${attempt_elapsed}ms" >&2; exit 1; }
            fi
            ;;
        wrong-load-origin)
            [ -s "$case_attempt/original-anchor.state" ] && [ "$(wc -l <"$case_attempt/boundaries.tsv" | tr -d ' ')" -eq 8 ] || {
                echo "FAIL wrong-load-origin lacked validated anchor and eight published boundaries" >&2; exit 1;
            }
            ;;
    esac
    if [ -n "$fail_request" ]; then
        failed_count=$(awk -v wanted="$fail_request" '$0 == wanted { n++ } END { print n + 0 }' "$case_dir/sends.log")
        [ "$failed_count" -eq 1 ] || { echo "FAIL $name request count: expected exactly one $fail_request, got $failed_count" >&2; exit 1; }
    fi
    if [ "$fault" = dropped-quit ]; then
        quit_count=$(awk '$0 == "QUIT" { n++ } END { print n + 0 }' "$case_dir/sends.log")
        [ "$quit_count" -eq 1 ] || { echo "FAIL dropped-quit retried QUIT: expected 1, got $quit_count" >&2; exit 1; }
    fi
    printf 'state session fault control: %s -> %s; %sms; host reaped\n' "$name" "$expected" "$case_duration"
}
run_prefix_driver() {
    _run_dir=$1; _run_ra=$2
    STATE_SESSION_NC="$_run_dir/fake-nc" \
    STATE_SESSION_TEST_FIXTURE="$fixture_dir" \
    STATE_SESSION_TEST_EXPECTED_DIR="$_run_ra/states" \
    STATE_SESSION_TEST_SENDS="$_run_dir/sends.log" \
    STATE_SESSION_TEST_HOST_PID="$_run_dir/host.pid" \
    STATE_SESSION_TEST_QUIT_FILE="$_run_dir/quit" \
    STATE_SESSION_TEST_SHOT_COUNT="$_run_dir/shot.count" \
    STATE_SESSION_TEST_READ_COUNT="$_run_dir/read.count" \
    STATE_SESSION_TEST_STATE_COUNT="$_run_dir/state.count" \
    STATE_SESSION_TEST_STALL_PID="$_run_dir/stall.pid" \
    STATE_SESSION_TEST_FAIL_REQUEST='' \
    STATE_SESSION_TEST_FAULT='' \
    STATE_SESSION_TEST_MODE=prefix \
    STATE_SESSION_TEST_STOP_AFTER_PREFIX=1 \
    STATE_SESSION_TEST_SHOTS="$_run_ra/shots" \
    STATE_SESSION_TEST_HOST_DIE=0 \
    STATE_SESSION_LOCK_ROOT="$shared_lock_root" \
    STATE_SESSION_ATTEMPT_MS=30000 \
    STATE_SESSION_OPERATION_MS=30000 \
    STATE_SESSION_CLEANUP_MS=1000 \
    STATE_SESSION_READY_MS=30000 \
    STATE_SESSION_PUBLICATION_MS=5000 \
    STATE_SESSION_SETTLE_MS=0 \
    STATE_SESSION_PROGRESS_WAIT_MS=20 \
    sh "$DRIVER" "$_run_dir/fake-retroarch" "$_run_dir/config.cfg" "$_run_dir/core.so" \
        "$fixture_dir/marker.nes" "$_run_ra" "$_run_dir/retroarch.log" "$FRAME_INDEX" \
        "$fixture_dir/calibration.txt"
}

# The synthetic marker fixtures satisfy each semantic precondition; these controls execute the
# actual shell driver and its shared production oracle, then require its own exact failure row.
run_fault wrong-readiness READINESS_REPLY wrong-readiness
run_fault stalled-read READINESS_DEADLINE stalled-read '' prefix 1800 350
run_fault stalled-child-attempt-cap ATTEMPT_DEADLINE stalled-read '' prefix 5000 5000 300 2500
run_fault pause-send-once pause-original_SEND_FAILED send-error PAUSE_TOGGLE
run_fault screenshot-send-once SCREENSHOT_SEND_FAILED send-error SCREENSHOT
run_fault save-send-once save-original_SEND_FAILED send-error SAVE_STATE
run_fault advance-send-once advance-1_SEND_FAILED send-error FRAMEADVANCE
run_fault missing-screenshot SCREENSHOT_MISSING missing-image
run_fault extra-screenshot EXTRA_PUBLICATION extra-image
run_fault stale-screenshot STALE_PUBLICATION stale-publication
run_fault truncated-screenshot MARKER_OBSERVE truncated-image
run_fault dropped-pause ANCHOR_NATIVE_ORIGIN dropped-pause
run_fault missing-state STATE_MISSING missing-state
run_fault duplicate-state STATE_AMBIGUOUS duplicate-state
run_fault corrupt-anchor ANCHOR_CONTAINER_SIZE corrupt-state
run_fault valid-wrong-origin-anchor ANCHOR_NATIVE_ORIGIN wrong-save-origin
run_fault doubled-advance ADVANCE_NOT_EXACTLY_ONE doubled-advance
run_fault host-died HOST_DIED host-died '' prefix 10000 30000 300
run_fault load-send-once load-probe_SEND_FAILED send-error 'LOAD_STATE_SLOT 0' complete 30000 30000 300
run_fault wrong-load-origin PROGRESSION wrong-load-origin '' complete 30000 30000 300
run_fault quit-send-once QUIT_DEADLINE dropped-quit '' complete 120000 30000 300

# Repeated sessions keep their own state, image and journal trees. A concurrent session must
# fail before it reaches the fake host or sends to the occupied fixed command endpoint.
shared_dir="$WORK_ROOT/session-$$-isolation"; shared_lock_root="$shared_dir/locks"
first_dir="$shared_dir/first"; first_ra="$first_dir/ra"
second_dir="$shared_dir/second"; second_ra="$second_dir/ra"
for _dir in "$first_dir" "$second_dir"; do
    mkdir -p "$_dir/ra/shots" "$_dir/ra/states"
    ln -s "$script_path" "$_dir/fake-nc"; ln -s "$script_path" "$_dir/fake-retroarch"
    : >"$_dir/config.cfg"; : >"$_dir/core.so"
done
run_prefix_driver "$first_dir" "$first_ra" >"$first_dir/driver.out" 2>&1 & first_driver=$!
_tries=0
while [ ! -d "$shared_lock_root/nesturbator-retroarch-55355.lock" ] && [ "$_tries" -lt 200 ]; do
    kill -0 "$first_driver" 2>/dev/null || { echo "FAIL first concurrent session exited before lock acquisition" >&2; cat "$first_dir/driver.out" >&2; exit 1; }
    sleep 0.02; _tries=$((_tries+1))
done
[ -d "$shared_lock_root/nesturbator-retroarch-55355.lock" ] || { echo "FAIL first concurrent session never held endpoint lock" >&2; exit 1; }
second_status=0
run_prefix_driver "$second_dir" "$second_ra" >"$second_dir/driver.out" 2>&1 || second_status=$?
[ "$second_status" -ne 0 ] && grep -Fx 'state_session: ENDPOINT_BUSY' "$second_dir/driver.out" >/dev/null || {
    echo "FAIL simultaneous attempt did not fail with ENDPOINT_BUSY (status=$second_status)" >&2; cat "$second_dir/driver.out" >&2; exit 1;
}
[ ! -s "$second_dir/sends.log" ] && [ ! -s "$second_dir/host.pid" ] || { echo "FAIL concurrent attempt sent or launched a host before endpoint ownership" >&2; exit 1; }
first_status=0; wait "$first_driver" || first_status=$?
[ "$first_status" -eq 0 ] && grep -F 'verdict=PREFIX_OK' "$first_dir/driver.out" >/dev/null || { echo "FAIL first concurrent attempt did not complete its prefix" >&2; cat "$first_dir/driver.out" >&2; exit 1; }
first_host=$(cat "$first_dir/host.pid"); if kill -0 "$first_host" 2>/dev/null; then echo "FAIL first concurrent host was not reaped" >&2; exit 1; fi
set -- "$first_ra"/attempts/*; [ "$#" -eq 1 ] || { echo "FAIL first attempt tree isolation count: expected one, got $#" >&2; exit 1; }

# Reuse the same persistent RA parent after the lock is released; each new attempt still gets a
# fresh private tree and a new fake session. Prior screenshot/state/journal files cannot satisfy it.
rm -f "$first_dir/sends.log" "$first_dir/host.pid" "$first_dir/quit" "$first_dir/shot.count" \
    "$first_dir/read.count" "$first_dir/state.count"
second_status=0
run_prefix_driver "$first_dir" "$first_ra" >"$first_dir/repeat.out" 2>&1 || second_status=$?
[ "$second_status" -eq 0 ] && grep -F 'verdict=PREFIX_OK' "$first_dir/repeat.out" >/dev/null || {
    echo "FAIL repeated attempt did not independently complete its prefix" >&2; cat "$first_dir/repeat.out" >&2; exit 1;
}
set -- "$first_ra"/attempts/*
[ "$#" -eq 2 ] && [ "$1" != "$2" ] || { echo "FAIL repeated attempt reused an earlier owned tree" >&2; exit 1; }
assert_sanitized_tree "$1"; assert_sanitized_tree "$2"
printf 'state session isolation control: concurrent ENDPOINT_BUSY; two unique sequential attempts\n'

# TERM during the owned session must retain the first interruption reason, send one cleanup
# QUIT, remove the lock and reap the host and active transport child.
interrupt_dir="$WORK_ROOT/session-$$-interrupt"; interrupt_ra="$interrupt_dir/ra"
mkdir -p "$interrupt_ra/shots" "$interrupt_ra/states"
ln -s "$script_path" "$interrupt_dir/fake-nc"; ln -s "$script_path" "$interrupt_dir/fake-retroarch"
: >"$interrupt_dir/config.cfg"; : >"$interrupt_dir/core.so"
STATE_SESSION_NC="$interrupt_dir/fake-nc" \
STATE_SESSION_TEST_FIXTURE="$fixture_dir" \
STATE_SESSION_TEST_EXPECTED_DIR="$interrupt_ra/states" \
STATE_SESSION_TEST_SENDS="$interrupt_dir/sends.log" \
STATE_SESSION_TEST_HOST_PID="$interrupt_dir/host.pid" \
STATE_SESSION_TEST_QUIT_FILE="$interrupt_dir/quit" \
STATE_SESSION_TEST_SHOT_COUNT="$interrupt_dir/shot.count" \
STATE_SESSION_TEST_READ_COUNT="$interrupt_dir/read.count" \
STATE_SESSION_TEST_STATE_COUNT="$interrupt_dir/state.count" \
STATE_SESSION_TEST_STALL_PID="$interrupt_dir/stall.pid" \
STATE_SESSION_TEST_FAIL_REQUEST='' \
STATE_SESSION_TEST_FAULT='' \
STATE_SESSION_TEST_MODE=prefix \
STATE_SESSION_TEST_STOP_AFTER_PREFIX=1 \
STATE_SESSION_TEST_SHOTS="$interrupt_ra/shots" \
STATE_SESSION_TEST_HOST_DIE=0 \
STATE_SESSION_LOCK_ROOT="$interrupt_dir/locks" \
STATE_SESSION_ATTEMPT_MS=30000 \
STATE_SESSION_OPERATION_MS=30000 \
STATE_SESSION_CLEANUP_MS=1000 \
STATE_SESSION_READY_MS=30000 \
STATE_SESSION_PUBLICATION_MS=5000 \
STATE_SESSION_SETTLE_MS=0 \
STATE_SESSION_PROGRESS_WAIT_MS=20 \
sh "$DRIVER" "$interrupt_dir/fake-retroarch" "$interrupt_dir/config.cfg" "$interrupt_dir/core.so" \
    "$fixture_dir/marker.nes" "$interrupt_ra" "$interrupt_dir/retroarch.log" "$FRAME_INDEX" \
    "$fixture_dir/calibration.txt" >"$interrupt_dir/driver.out" 2>&1 & interrupt_driver=$!
_tries=0
while [ ! -s "$interrupt_dir/host.pid" ] && [ "$_tries" -lt 300 ]; do
    kill -0 "$interrupt_driver" 2>/dev/null || { echo "FAIL interrupt session ended before host setup" >&2; cat "$interrupt_dir/driver.out" >&2; exit 1; }
    sleep 0.02; _tries=$((_tries+1))
done
[ -s "$interrupt_dir/host.pid" ] || { echo "FAIL interrupt control never observed owned host" >&2; exit 1; }
kill -TERM "$interrupt_driver" 2>/dev/null || { echo "FAIL could not signal owned driver" >&2; exit 1; }
interrupt_status=0; wait "$interrupt_driver" || interrupt_status=$?
[ "$interrupt_status" -eq 130 ] || { echo "FAIL interrupted driver exit status: expected 130, got $interrupt_status" >&2; exit 1; }
set -- "$interrupt_ra"/attempts/*; interrupt_attempt=$1
grep -F 'INTERRUPTED_TERM' "$interrupt_attempt/journal.tsv" >/dev/null || { echo "FAIL interruption first failure was not retained" >&2; exit 1; }
[ "$(awk '$0 == "QUIT" { n++ } END { print n + 0 }' "$interrupt_dir/sends.log")" -eq 1 ] || {
    echo "FAIL interruption cleanup did not send exactly one QUIT" >&2; exit 1;
}
interrupt_host=$(cat "$interrupt_dir/host.pid"); if kill -0 "$interrupt_host" 2>/dev/null; then echo "FAIL interruption left host alive" >&2; exit 1; fi
[ ! -d "$interrupt_dir/locks/nesturbator-retroarch-55355.lock" ] || { echo "FAIL interruption left endpoint lock behind" >&2; exit 1; }
assert_sanitized_tree "$interrupt_attempt"
printf 'state session interruption control: TERM retained; one QUIT; host reaped\n'

# Expansion gate: the same actual driver must now run the full sixteen-boundary plan through
# complete VERDICT_OK, retaining both native origins and using the prescribed forty rewinds.
complete_dir="$WORK_ROOT/session-$$-complete"
complete_fixture="$fixture_dir"
complete_ra="$complete_dir/ra"
mkdir -p "$complete_ra/shots" "$complete_ra/states"
ln -s "$script_path" "$complete_dir/fake-nc"
ln -s "$script_path" "$complete_dir/fake-retroarch"
printf 'savestate_directory = "%s/states"\nscreenshot_directory = "%s/shots"\n' \
    "$complete_ra" "$complete_ra" >"$complete_dir/config.cfg"
: >"$complete_dir/core.so"
complete_sends="$complete_dir/sends.log"
complete_host="$complete_dir/host.pid"
complete_quit="$complete_dir/quit"
complete_reads="$complete_dir/read.count"
complete_shots="$complete_dir/shot.count"
complete_states="$complete_dir/state.count"
complete_status=0
STATE_SESSION_NC="$complete_dir/fake-nc" \
STATE_SESSION_TEST_FIXTURE="$complete_fixture" \
STATE_SESSION_TEST_EXPECTED_DIR="$complete_ra/states" \
STATE_SESSION_TEST_SENDS="$complete_sends" \
STATE_SESSION_TEST_HOST_PID="$complete_host" \
STATE_SESSION_TEST_QUIT_FILE="$complete_quit" \
STATE_SESSION_TEST_SHOT_COUNT="$complete_shots" \
STATE_SESSION_TEST_READ_COUNT="$complete_reads" \
STATE_SESSION_TEST_STATE_COUNT="$complete_states" \
STATE_SESSION_TEST_MODE=complete \
STATE_SESSION_RECORD_ROOT="$PWD" \
STATE_SESSION_TEST_SHOTS="$complete_ra/shots" \
STATE_SESSION_LOCK_ROOT="$complete_dir/locks" \
STATE_SESSION_SETTLE_MS=0 \
STATE_SESSION_PROGRESS_WAIT_MS=20 \
sh "$DRIVER" "$complete_dir/fake-retroarch" "$complete_dir/config.cfg" "$complete_dir/core.so" \
    "$complete_fixture/marker.nes" "$complete_ra" "$complete_dir/retroarch.log" "$FRAME_INDEX" \
    "$complete_fixture/calibration.txt" >"$complete_dir/driver.out" 2>&1 || complete_status=$?
if [ "$complete_status" -ne 0 ] || ! grep -Fx 'progression boundaries=16 mode=complete verdict=VERDICT_OK' \
    "$complete_dir/driver.out" >/dev/null; then
    echo "FAIL complete session reaches the production verdict (status=$complete_status): expected 16 boundaries and VERDICT_OK" >&2
    cat "$complete_dir/driver.out" >&2
    exit 1
fi
complete_count() { awk -v wanted="$1" '$0 == wanted { n++ } END { print n + 0 }' "$complete_sends"; }
for pair in 'PAUSE_TOGGLE:3' 'SAVE_STATE:2' 'FRAMEADVANCE:12' 'LOAD_STATE_SLOT 0:2' \
    'RUNAHEAD_TOGGLE:1' 'REWIND:40' 'SCREENSHOT:16' 'QUIT:1'; do
    request=${pair%:*}
    expected=${pair##*:}
    actual=$(complete_count "$request")
    [ "$actual" -eq "$expected" ] || {
        echo "FAIL complete one-shot send count for $request: expected $expected, got $actual" >&2
        exit 1
    }
done
set -- "$complete_ra"/attempts/attempt-*
[ -d "$1" ] || { echo "FAIL complete record has no retained attempt tree" >&2; exit 1; }
complete_attempt=$1
assert_sanitized_tree "$complete_attempt"
grep -Fx 'savestate_directory = "<attempt>/states"' "$complete_attempt/session.sanitized.cfg" >/dev/null || {
    echo "FAIL complete session config did not replace its nested owned directory exactly once" >&2
    exit 1
}
original_record_path=$(sed -n 's/^original_anchor_path=//p' "$complete_attempt/progression-session.rec")
case "$original_record_path" in
    /*) echo "FAIL complete progression record retained an absolute original-anchor path" >&2; exit 1 ;;
esac
[ -f "$original_record_path" ] || { echo "FAIL relative progression record path does not resolve from the session root" >&2; exit 1; }
printf 'state session complete control: VERDICT_OK; sixteen boundaries; forty rewinds; exact one-shot counts\n'
