#!/bin/sh
# Deterministic controls for the production RetroArch session driver. The fake host and
# transport are this same script reached through private test-only symlinks.
set -eu

fake_nc() {
    request=$(cat)
    printf '%s\n' "$request" >>"$STATE_SESSION_TEST_SENDS"
    case "$request" in
        'GET_CONFIG_PARAM savestate_directory')
            printf 'GET_CONFIG_PARAM savestate_directory %s\n' "$STATE_SESSION_EXPECTED_DIR"
            ;;
        PAUSE_TOGGLE|FRAMEADVANCE|SAVE_STATE|SCREENSHOT|QUIT)
            case "$request" in
                SAVE_STATE)
                    mkdir -p "$STATE_SESSION_EXPECTED_DIR/nesturbator"
                    cp "$STATE_SESSION_TEST_FIXTURE/retained original anchor.ra" \
                        "$STATE_SESSION_EXPECTED_DIR/nesturbator/marker.state"
                    ;;
                SCREENSHOT)
                    count=0
                    [ ! -f "$STATE_SESSION_TEST_SHOT_COUNT" ] || count=$(cat "$STATE_SESSION_TEST_SHOT_COUNT")
                    count=$((count + 1))
                    printf '%s\n' "$count" >"$STATE_SESSION_TEST_SHOT_COUNT"
                    case "$count" in
                        1|2) source_image="$STATE_SESSION_TEST_FIXTURE/pause capture one.ppm" ;;
                        3) source_image="$STATE_SESSION_TEST_FIXTURE/advance one.ppm" ;;
                        *) exit 91 ;;
                    esac
                    cp "$source_image" "$STATE_SESSION_TEST_SHOTS/capture-$count.ppm"
                    ;;
                QUIT)
                    host_pid=$(cat "$STATE_SESSION_TEST_HOST_PID")
                    kill -TERM "$host_pid" 2>/dev/null || true
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
    exec sleep 60
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
shot_count_file="$test_dir/shot.count"
driver_output="$test_dir/driver.out"

status=0
STATE_SESSION_NC="$test_dir/fake-nc" \
STATE_SESSION_TEST_MODE=1 \
STATE_SESSION_TEST_FIXTURE="$fixture_dir" \
STATE_SESSION_TEST_EXPECTED_DIR="$expected_state_dir" \
STATE_SESSION_TEST_SENDS="$send_log" \
STATE_SESSION_TEST_HOST_PID="$host_pid_file" \
STATE_SESSION_TEST_SHOT_COUNT="$shot_count_file" \
STATE_SESSION_TEST_SHOTS="$ra_dir/shots" \
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

printf 'state session tracer: PREFIX_OK; one FRAMEADVANCE, 3 attributed captures, one QUIT\n'
