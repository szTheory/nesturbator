#!/bin/sh
# Deterministic controls for the production RetroArch session driver. The fake host and
# transport are this same script reached through private test-only symlinks.
set -eu

fake_nc() {
    request=$(cat)
    printf '%s\n' "$request" >>"$STATE_SESSION_TEST_SENDS"
    fault=${STATE_SESSION_TEST_FAULT:-}
    if [ "$fault" = send-error ] && [ "$request" = "${STATE_SESSION_TEST_FAIL_REQUEST:-}" ]; then exit 93; fi
    if [ "$fault" = stalled-write ] && [ "$request" = FRAMEADVANCE ]; then
        printf '%s\n' "$$" >"$STATE_SESSION_TEST_STALL_PID"
        exec perl -MTime::HiRes=sleep -e 'sleep 60'
    fi
    case "$request" in
        'GET_CONFIG_PARAM savestate_directory')
            if [ "$fault" = stalled-read ]; then
                printf '%s\n' "$$" >"$STATE_SESSION_TEST_STALL_PID"
                exec perl -MTime::HiRes=sleep -e 'sleep 5'
            fi
            if [ "$fault" = readiness-response-stall ]; then
                printf '%s\n' "$$" >"$STATE_SESSION_TEST_STALL_PID"
                printf 'GET_CONFIG_PARAM savestate_directory %s' "$STATE_SESSION_TEST_EXPECTED_DIR"
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
                            1) name='alternative frame 101.ppm' ;;
                            2|3|4) name='boundary 01 pause a.ppm' ;;
                            5) name='boundary 03 advance 1.ppm' ;;
                            6) name='boundary 04 advance 2.ppm' ;;
                            7) name='boundary 05 advance 3.ppm' ;;
                            8) name='boundary 06 advance 4.ppm' ;;
                            9) name='boundary 07 advance 5.ppm' ;;
                            10) name='boundary 08 load probe.ppm' ;;
                            11) name='boundary 09 post load 2.ppm' ;;
                            12) name='boundary 10 post load 3.ppm' ;;
                            13) name='boundary 11 resume forward.ppm' ;;
                            14) name='boundary 12 rewind coarse.ppm' ;;
                            15) name='boundary 13 rewound advance 1.ppm' ;;
                            16) name='boundary 14 rewound advance 2.ppm' ;;
                            17) name='boundary 15 rewound advance 3.ppm' ;;
                            18) name='boundary 16 runahead probe.ppm' ;;
                            *) exit 91 ;;
                        esac
                        source_image="$STATE_SESSION_TEST_FIXTURE/$name"
                    else
                        case "$count" in
                            1)
                                source_image="$STATE_SESSION_TEST_FIXTURE/alternative frame 101.ppm"
                                ;;
                            2)
                                if [ "$fault" = stalled-startup ]; then
                                    source_image="$STATE_SESSION_TEST_FIXTURE/alternative frame 101.ppm"
                                else
                                    source_image="$STATE_SESSION_TEST_FIXTURE/boundary 01 pause a.ppm"
                                fi
                                ;;
                            3)
                                if [ "$fault" = dropped-pause ]; then
                                    source_image="$STATE_SESSION_TEST_FIXTURE/boundary 03 advance 1.ppm"
                                else
                                    source_image="$STATE_SESSION_TEST_FIXTURE/boundary 01 pause a.ppm"
                                fi
                                ;;
                            4)
                                if [ "$fault" = dropped-pause ]; then
                                    source_image="$STATE_SESSION_TEST_FIXTURE/boundary 04 advance 2.ppm"
                                else
                                    source_image="$STATE_SESSION_TEST_FIXTURE/boundary 02 pause b.ppm"
                                fi
                                ;;
                            5)
                                if [ "${STATE_SESSION_TEST_DROP_ADVANCE:-0}" = 1 ] || [ "$fault" = dropped-advance ]; then
                                    source_image="$STATE_SESSION_TEST_FIXTURE/boundary 02 pause b.ppm"
                                elif [ "$fault" = doubled-advance ]; then
                                    source_image="$STATE_SESSION_TEST_FIXTURE/boundary 04 advance 2.ppm"
                                else
                                    source_image="$STATE_SESSION_TEST_FIXTURE/boundary 03 advance 1.ppm"
                                fi
                                ;;
                            *) exit 91 ;;
                        esac
                    fi
                    if [ "$fault" = wrong-load-origin ] && [ "$count" -eq 10 ]; then
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
[ "$count" -eq 5 ] || {
    echo "FAIL screenshot publication request count: expected 5, got $count" >&2
    exit 1
}
count=$(count_request 'GET_CONFIG_PARAM savestate_directory')
[ "$count" -eq 2 ] || {
    echo "FAIL read-only readiness retry count: expected 2, got $count" >&2
    exit 1
}

printf 'state session tracer: PREFIX_OK; one FRAMEADVANCE, 2 startup captures, 3 boundary captures, one QUIT\n'

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
STATE_SESSION_ATTEMPT_MS=30000 \
STATE_SESSION_CLEANUP_MS=1000 \
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
negative_attempt_dir=
for _negative_attempt in "$negative_ra"/attempts/attempt-*; do
    [ -d "$_negative_attempt" ] || continue
    negative_attempt_dir=$_negative_attempt
    break
done
[ -n "$negative_attempt_dir" ] || { echo "FAIL dropped advance did not retain its attempt tree" >&2; exit 1; }
negative_row=$(awk -F '\t' '$11 == "ADVANCE_NOT_EXACTLY_ONE" { row=$0 } END { print row }' "$negative_attempt_dir/journal.tsv")
negative_attempt_elapsed=$(printf '%s\n' "$negative_row" | cut -f4)
negative_operation_elapsed=$(printf '%s\n' "$negative_row" | cut -f6)
[ "$negative_operation_elapsed" -gt 0 ] && [ "$negative_operation_elapsed" -le 10000 ] || {
    echo "FAIL dropped advance exceeded measured operation deadline: ${negative_operation_elapsed}ms" >&2; exit 1;
}
[ "$negative_attempt_elapsed" -ge "$negative_operation_elapsed" ] && [ "$negative_attempt_elapsed" -lt 29000 ] && [ "$duration" -lt 32000 ] || {
    echo "FAIL dropped advance exceeded bounded attempt/cleanup time: operation=${negative_operation_elapsed}ms attempt=${negative_attempt_elapsed}ms process=${duration}ms" >&2
    exit 1
}
advance_count=$(awk '$0 == "FRAMEADVANCE" { n++ } END { print n + 0 }' "$negative_sends")
[ "$advance_count" -eq 1 ] || { echo "FAIL dropped advance send count: expected 1, got $advance_count" >&2; exit 1; }
host_pid=$(cat "$negative_host")
if kill -0 "$host_pid" 2>/dev/null; then echo "FAIL dropped advance left the owned host alive" >&2; exit 1; fi
printf 'state session missing-action control: ADVANCE_NOT_EXACTLY_ONE; one send; operation %sms, attempt %sms, process %sms; host reaped\n' \
    "$negative_operation_elapsed" "$negative_attempt_elapsed" "$duration"

mono_ms() { perl -MTime::HiRes=clock_gettime,CLOCK_MONOTONIC -e 'printf "%.0f\n",clock_gettime(CLOCK_MONOTONIC)*1000'; }
assert_sanitized_tree() {
    _attempt=$1
    for _file in journal.tsv session.sanitized.cfg retroarch.sanitized.log; do
        [ -f "$_attempt/$_file" ] || { echo "FAIL missing sanitized $_file" >&2; exit 1; }
        if grep -F -e "$WORK_ROOT" -e '/Users/' -e '/home/' "$_attempt/$_file" >/dev/null; then
            echo "FAIL unsanitized path in $_attempt/$_file" >&2; exit 1
        fi
    done
    [ ! -e "$_attempt/session.cfg" ] && [ ! -e "$_attempt/retroarch.log" ] || {
        echo "FAIL raw generated config/log retained in $_attempt" >&2; exit 1;
    }
}
run_fault() {
    name=$1; expected=$2; fault=$3; fail_request=${4:-}; mode=${5:-prefix}; max_ms=${6:-20000}; ready_ms=${7:-30000}; publication_ms=${8:-1000}; attempt_override=${9:-}; operation_override=${10:-30000}
    case_dir="$WORK_ROOT/session-$$-$name"; case_ra="$case_dir/ra"
    case_host_die=0; [ "$fault" != host-died ] || case_host_die=1
    mkdir -p "$case_ra/shots" "$case_ra/states"
    ln -s "$script_path" "$case_dir/fake-nc"; ln -s "$script_path" "$case_dir/fake-retroarch"
    : >"$case_dir/config.cfg"; : >"$case_dir/core.so"
    case_started=$(mono_ms); case_status=0
    if [ "$mode" = complete ]; then stop_after=0; attempt_ms=120000; else stop_after=1; attempt_ms=30000; fi
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
    STATE_SESSION_OPERATION_MS="$operation_override" \
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
    [ -f "$case_attempt/startup-captures.tsv" ] || { echo "FAIL $name did not retain startup capture evidence" >&2; exit 1; }
    case "$fault" in
        stalled-startup)
            startup_rows=$(wc -l <"$case_attempt/startup-captures.tsv" | tr -d ' ')
            pause_count=$(awk '$0 == "PAUSE_TOGGLE" { n++ } END { print n + 0 }' "$case_dir/sends.log")
            save_count=$(awk '$0 == "SAVE_STATE" { n++ } END { print n + 0 }' "$case_dir/sends.log")
            screenshot_count=$(awk '$0 == "SCREENSHOT" { n++ } END { print n + 0 }' "$case_dir/sends.log")
            [ "$startup_rows" -eq 3 ] && [ "$pause_count" -eq 0 ] && [ "$save_count" -eq 0 ] && [ "$screenshot_count" -eq 2 ] &&
                awk -F '\t' 'NR == 2 { first=$4; a=$5 } NR == 3 { second=$4; b=$5 } END { exit !(NR == 3 && first == second && a != "" && b != "" && a != b) }' \
                    "$case_attempt/startup-captures.tsv" || {
                echo "FAIL stalled startup was not retained as two valid equal captures before any pause or save" >&2; exit 1;
            }
            ;;
        dropped-pause)
            pause_count=$(awk '$0 == "PAUSE_TOGGLE" { n++ } END { print n + 0 }' "$case_dir/sends.log")
            save_count=$(awk '$0 == "SAVE_STATE" { n++ } END { print n + 0 }' "$case_dir/sends.log")
            screenshot_count=$(awk '$0 == "SCREENSHOT" { n++ } END { print n + 0 }' "$case_dir/sends.log")
            [ "$pause_count" -eq 1 ] && [ "$save_count" -eq 0 ] && [ "$screenshot_count" -eq 4 ] &&
                awk -F '\t' 'NR == 2 { first=$4 } NR == 3 { second=$4 } END { exit !(NR == 3 && first < second) }' \
                    "$case_attempt/startup-captures.tsv" || {
                echo "FAIL dropped pause did not retain an advancing startup pair and one toggle before rejection" >&2; exit 1;
            }
            ;;
        stalled-read|stalled-write)
            [ -s "$case_dir/stall.pid" ] || { echo "FAIL $name had no child pid evidence" >&2; exit 1; }
            stall_pid=$(cat "$case_dir/stall.pid")
            if kill -0 "$stall_pid" 2>/dev/null; then echo "FAIL $name did not reap the blocked child" >&2; exit 1; fi
            if [ "$expected" = READINESS_DEADLINE ]; then
                stall_row=$(grep -F 'READINESS_DEADLINE' "$1/journal.tsv" | tail -1)
                stall_elapsed=$(printf '%s\n' "$stall_row" | cut -f6)
                stall_attempt_elapsed=$(printf '%s\n' "$stall_row" | cut -f4)
                [ "$stall_elapsed" -gt 0 ] && [ "$stall_elapsed" -le 800 ] || { echo "FAIL readiness child did not obey measured monotonic budget: ${stall_elapsed}ms" >&2; exit 1; }
                [ "$stall_attempt_elapsed" -ge "$stall_elapsed" ] && [ "$stall_attempt_elapsed" -lt "$max_ms" ] || {
                    echo "FAIL readiness cleanup was not bounded separately from operation time: operation=${stall_elapsed}ms attempt=${stall_attempt_elapsed}ms" >&2
                    exit 1
                }
                printf 'state session child deadline: %sms operation elapsed; child reaped\n' "$stall_elapsed"
            else
                attempt_row=$(grep -F 'ATTEMPT_DEADLINE' "$1/journal.tsv" | tail -1)
                attempt_elapsed=$(printf '%s\n' "$attempt_row" | cut -f4)
                attempt_remaining=$(printf '%s\n' "$attempt_row" | cut -f5)
                attempt_min=$((attempt_ms - 1500))
                [ "$attempt_elapsed" -ge "$attempt_min" ] && [ "$attempt_elapsed" -lt "$attempt_ms" ] &&
                    [ "$attempt_remaining" -gt 0 ] || {
                    echo "FAIL child did not hit the measured attempt cap with cleanup reserve: elapsed=${attempt_elapsed}ms remaining=${attempt_remaining}ms budget=${attempt_ms}ms" >&2
                    exit 1
                }
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
    _run_dir=$1; _run_ra=$2; _run_fault=${3:-}; _run_lock_root=${4:-$_run_dir/locks}
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
    STATE_SESSION_TEST_FAULT="$_run_fault" \
    STATE_SESSION_TEST_MODE=prefix \
    STATE_SESSION_TEST_STOP_AFTER_PREFIX=1 \
    STATE_SESSION_TEST_SHOTS="$_run_ra/shots" \
    STATE_SESSION_TEST_HOST_DIE=0 \
    STATE_SESSION_LOCK_ROOT="$_run_lock_root" \
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
# A 300 ms fake-image publication window expired under hosted Intel scheduling before the
# duplicate-state prerequisite. The test-only 1 s bound remains inside each 30 s operation cap.
run_fault wrong-readiness READINESS_REPLY wrong-readiness
run_fault stalled-read READINESS_DEADLINE stalled-read '' prefix 5000 350
# Setup must have time to launch the blocked FRAMEADVANCE child. Its 60 s stall and operation
# ceiling exceed the 30 s attempt cap, so the test proves attempt-deadline cleanup deterministically.
run_fault stalled-child-attempt-cap ATTEMPT_DEADLINE stalled-write FRAMEADVANCE prefix 45000 30000 1000 30000 60000
run_fault pause-send-once pause-original_SEND_FAILED send-error PAUSE_TOGGLE
run_fault screenshot-send-once SCREENSHOT_SEND_FAILED send-error SCREENSHOT
run_fault save-send-once save-original_SEND_FAILED send-error SAVE_STATE
run_fault advance-send-once advance-1_SEND_FAILED send-error FRAMEADVANCE
run_fault missing-screenshot SCREENSHOT_MISSING missing-image
run_fault extra-screenshot EXTRA_PUBLICATION extra-image
run_fault stale-screenshot STALE_PUBLICATION stale-publication
run_fault truncated-screenshot MARKER_OBSERVE truncated-image
run_fault stalled-startup STARTUP_NOT_ADVANCING stalled-startup
run_fault dropped-pause PAUSE_DID_NOT_HOLD dropped-pause
run_fault missing-state STATE_MISSING missing-state
run_fault duplicate-state STATE_AMBIGUOUS duplicate-state
run_fault corrupt-anchor ANCHOR_CONTAINER_SIZE corrupt-state
run_fault valid-wrong-origin-anchor ANCHOR_NATIVE_ORIGIN wrong-save-origin
run_fault doubled-advance ADVANCE_NOT_EXACTLY_ONE doubled-advance
run_fault host-died HOST_DIED host-died '' prefix 20000 30000 1000
# Hosted complete controls measured 34.879 s under ASan and 30.458 s on macOS arm64. Keep
# the per-operation limit at 30 s while giving the whole deterministic control a finite 75 s cap.
run_fault load-send-once load-probe_SEND_FAILED send-error 'LOAD_STATE_SLOT 0' complete 75000 30000 1000
run_fault wrong-load-origin PROGRESSION wrong-load-origin '' complete 75000 30000 1000
run_fault quit-send-once QUIT_DEADLINE dropped-quit '' complete 190000 30000 1000 180000

# Some native UDP clients can print a complete response and then remain alive until the per-read
# timeout. The production parser must validate those exact bytes before treating that child timeout
# as a lost read, then reap the child and continue once within the readiness operation budget.
readiness_dir="$WORK_ROOT/session-$$-readiness-response"; readiness_ra="$readiness_dir/ra"
mkdir -p "$readiness_ra/shots" "$readiness_ra/states"
ln -s "$script_path" "$readiness_dir/fake-nc"; ln -s "$script_path" "$readiness_dir/fake-retroarch"
: >"$readiness_dir/config.cfg"; : >"$readiness_dir/core.so"
readiness_started=$(mono_ms); readiness_status=0
run_prefix_driver "$readiness_dir" "$readiness_ra" readiness-response-stall >"$readiness_dir/driver.out" 2>&1 || readiness_status=$?
readiness_finished=$(mono_ms); readiness_elapsed=$((readiness_finished-readiness_started))
[ "$readiness_status" -eq 0 ] && grep -F 'verdict=PREFIX_OK' "$readiness_dir/driver.out" >/dev/null || {
    echo "FAIL exact response before per-read timeout did not proceed (status=$readiness_status)" >&2
    cat "$readiness_dir/driver.out" >&2; exit 1;
}
set -- "$readiness_ra"/attempts/*; readiness_attempt=$1
awk -F '\t' 'NR > 1 && $1 == "readiness" { count++; if ($3 != 1 || $10 != "exact-isolated-path" || $11 != "OK") exit 1 } END { exit !(count == 1) }' \
    "$readiness_attempt/journal.tsv" || { echo "FAIL exact-readiness response was not accepted in one attempt" >&2; exit 1; }
readiness_operation_ms=$(awk -F '\t' 'NR > 1 && $1 == "readiness" { print $6 }' "$readiness_attempt/journal.tsv")
[ "$readiness_operation_ms" -gt 0 ] && [ "$readiness_operation_ms" -lt 3000 ] || {
    echo "FAIL exact readiness response exceeded its measured operation bound: ${readiness_operation_ms}ms" >&2; exit 1;
}
[ -s "$readiness_dir/stall.pid" ] || { echo "FAIL readiness timeout control recorded no owned child" >&2; exit 1; }
readiness_child=$(cat "$readiness_dir/stall.pid")
if kill -0 "$readiness_child" 2>/dev/null; then echo "FAIL readiness timeout control left its child alive" >&2; exit 1; fi
readiness_host=$(cat "$readiness_dir/host.pid")
if kill -0 "$readiness_host" 2>/dev/null; then echo "FAIL readiness timeout control left its host alive" >&2; exit 1; fi
[ "$readiness_elapsed" -ge 700 ] && [ "$readiness_elapsed" -lt 15000 ] || {
    echo "FAIL exact readiness response elapsed bound: ${readiness_elapsed}ms" >&2; exit 1;
}
assert_sanitized_tree "$readiness_attempt"
printf 'state session readiness control: exact response retained; timed-out child reaped; %sms\n' "$readiness_elapsed"

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
run_prefix_driver "$first_dir" "$first_ra" '' "$shared_lock_root" >"$first_dir/driver.out" 2>&1 & first_driver=$!
_tries=0
while [ ! -d "$shared_lock_root/nesturbator-retroarch-55355.lock" ] && [ "$_tries" -lt 200 ]; do
    kill -0 "$first_driver" 2>/dev/null || { echo "FAIL first concurrent session exited before lock acquisition" >&2; cat "$first_dir/driver.out" >&2; exit 1; }
    sleep 0.02; _tries=$((_tries+1))
done
[ -d "$shared_lock_root/nesturbator-retroarch-55355.lock" ] || { echo "FAIL first concurrent session never held endpoint lock" >&2; exit 1; }
second_status=0
run_prefix_driver "$second_dir" "$second_ra" '' "$shared_lock_root" >"$second_dir/driver.out" 2>&1 || second_status=$?
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
run_prefix_driver "$first_dir" "$first_ra" '' "$shared_lock_root" >"$first_dir/repeat.out" 2>&1 || second_status=$?
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
STATE_SESSION_ATTEMPT_MS=180000 \
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
    'RUNAHEAD_TOGGLE:1' 'REWIND:40' 'SCREENSHOT:18' 'QUIT:1'; do
    request=${pair%:*}
    expected=${pair##*:}
    actual=$(complete_count "$request")
    [ "$actual" -eq "$expected" ] || {
        echo "FAIL complete one-shot send count for $request: expected $expected, got $actual" >&2
        exit 1
    }
done
complete_total=$(awk '$0 != "GET_CONFIG_PARAM savestate_directory" { n++ } END { print n + 0 }' "$complete_sends")
complete_readiness=$(awk '$0 == "GET_CONFIG_PARAM savestate_directory" { n++ } END { print n + 0 }' "$complete_sends")
[ "$complete_total" -eq 79 ] && [ "$complete_readiness" -eq 1 ] || {
    echo "FAIL complete send accounting: expected 79 one-shot plus one readiness query, got $complete_total one-shot and $complete_readiness readiness" >&2
    exit 1
}
set -- "$complete_ra"/attempts/attempt-*
complete_attempt=$1
awk -F '\t' 'NR == 2 { ok1=($1 == "startup-forward-01" && $2 == "baseline" && $3 == 1); previous=$4 }
    NR == 3 { ok2=($1 == "startup-forward-02" && $2 == "strictly-forward-from-startup-forward-01" && $3 == 2 && $4 > previous) }
    END { exit !(NR == 3 && ok1 && ok2) }' "$complete_attempt/startup-captures.tsv" || {
    echo "FAIL complete retained startup captures lack exact send ordinals or strict progress" >&2; exit 1;
}
awk '
    $0 == "PAUSE_TOGGLE" { pauses++; if (pauses == 3) paused_boundary_line = NR }
    $0 == "SCREENSHOT" { screenshots++; if (screenshots == 13) resume_boundary_line = NR }
    END { exit !(paused_boundary_line && resume_boundary_line && paused_boundary_line < resume_boundary_line) }
' "$complete_sends" || {
    echo "FAIL resume-forward boundary was captured before the pause transition" >&2
    exit 1
}
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
