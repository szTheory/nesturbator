#!/bin/sh
# Deterministic controls for the production RetroArch session driver. The fake host and
# transport are this same script reached through private test-only symlinks.
set -eu

fake_nc() {
    request=$(cat)
    printf '%s\n' "$request" >>"$STATE_SESSION_TEST_SENDS"
    case "$request" in
        'GET_CONFIG_PARAM savestate_directory')
            read_count=0
            [ ! -f "$STATE_SESSION_TEST_READ_COUNT" ] || read_count=$(cat "$STATE_SESSION_TEST_READ_COUNT")
            read_count=$((read_count + 1))
            printf '%s\n' "$read_count" >"$STATE_SESSION_TEST_READ_COUNT"
            if [ "$read_count" -eq 1 ] && [ "${STATE_SESSION_TEST_DROP_FIRST_READ:-0}" = 1 ]; then exit 0; fi
            printf 'GET_CONFIG_PARAM savestate_directory %s\n' "$STATE_SESSION_TEST_EXPECTED_DIR"
            ;;
        PAUSE_TOGGLE|FRAMEADVANCE|SAVE_STATE|SCREENSHOT|QUIT)
            case "$request" in
                SAVE_STATE)
                    mkdir -p "$STATE_SESSION_TEST_EXPECTED_DIR/nesturbator"
                    cp "$STATE_SESSION_TEST_FIXTURE/retained original anchor.ra" \
                        "$STATE_SESSION_TEST_EXPECTED_DIR/nesturbator/marker.state"
                    ;;
                SCREENSHOT)
                    count=0
                    [ ! -f "$STATE_SESSION_TEST_SHOT_COUNT" ] || count=$(cat "$STATE_SESSION_TEST_SHOT_COUNT")
                    count=$((count + 1))
                    printf '%s\n' "$count" >"$STATE_SESSION_TEST_SHOT_COUNT"
                    case "$count" in
                        1|2) source_image="$STATE_SESSION_TEST_FIXTURE/pause capture one.ppm" ;;
                        3)
                            if [ "${STATE_SESSION_TEST_DROP_ADVANCE:-0}" = 1 ]; then
                                source_image="$STATE_SESSION_TEST_FIXTURE/pause capture one.ppm"
                            else
                                source_image="$STATE_SESSION_TEST_FIXTURE/advance one.ppm"
                            fi
                            ;;
                        *) exit 91 ;;
                    esac
                    cp "$source_image" "$STATE_SESSION_TEST_SHOTS/capture-$count.ppm"
                    ;;
                QUIT)
                    : >"$STATE_SESSION_TEST_QUIT_FILE"
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
if ! "$FRAME_INDEX" --progression-selftest "$fixture_dir"; then
    echo "fixture precondition failed: frame_index --progression-selftest" >&2
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
STATE_SESSION_TEST_DROP_FIRST_READ=1 \
STATE_SESSION_TEST_SHOTS="$ra_dir/shots" \
STATE_SESSION_LOCK_ROOT="$test_dir/locks" \
STATE_SESSION_SETTLE_SECONDS=0 \
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
negative_fixture="$negative_dir/fixture"
negative_ra="$negative_dir/ra"
mkdir -p "$negative_fixture" "$negative_ra/shots" "$negative_ra/states"
"$FRAME_INDEX" --progression-selftest "$negative_fixture" >/dev/null
ln -s "$script_path" "$negative_dir/fake-nc"
ln -s "$script_path" "$negative_dir/fake-retroarch"
: >"$negative_dir/config.cfg"
: >"$negative_dir/core.so"
negative_sends="$negative_dir/sends.log"
negative_host="$negative_dir/host.pid"
negative_quit="$negative_dir/quit"
negative_reads="$negative_dir/read.count"
negative_shots="$negative_dir/shot.count"
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
STATE_SESSION_TEST_DROP_ADVANCE=1 \
STATE_SESSION_TEST_SHOTS="$negative_ra/shots" \
STATE_SESSION_LOCK_ROOT="$negative_dir/locks" \
STATE_SESSION_SETTLE_SECONDS=0 \
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
