#!/bin/sh
# Bounded owned RetroArch session. Arguments: RETROARCH CFG CORE CONTENT RA_DIR LOG FRAME_INDEX CALIBRATION.
# Each run gets its own HOME, config, screenshots, states, journal and verifier record. Only an
# exact read-only readiness query retries; action and publication requests are one-shot.
set -eu
[ "$#" -eq 8 ] || { echo "usage: state_session.sh RETROARCH CFG CORE CONTENT RA_DIR LOG FRAME_INDEX CALIBRATION" >&2; exit 2; }
RA=$1; CFG=$2; CORE=$3; CONTENT=$4; RA_DIR=$5; LOG=$6; FRAME_INDEX=$7; CALIBRATION=$8
STEM=${CONTENT##*/}; STEM=${STEM%.*}; NC=${STATE_SESSION_NC:-nc}
PROJECT_ROOT=$(CDPATH= cd "$(dirname "$0")/../.." && pwd -P)
TIMEOUT_MS=${STATE_SESSION_ATTEMPT_MS:-120000}; OPERATION_MS=${STATE_SESSION_OPERATION_MS:-30000}
CLEANUP_MS=${STATE_SESSION_CLEANUP_MS:-5000}; READY_MS=${STATE_SESSION_READY_MS:-30000}
PUBLICATION_MS=${STATE_SESSION_PUBLICATION_MS:-5000}; LOCK_ROOT=${STATE_SESSION_LOCK_ROOT:-${TMPDIR:-/tmp}}
SETTLE_MS=${STATE_SESSION_SETTLE_MS:-3000}; PAUSE_GAP_MS=${STATE_SESSION_PAUSE_GAP_MS:-1100}
PROGRESS_WAIT_MS=${STATE_SESSION_PROGRESS_WAIT_MS:-1000}; COMMAND_GAP_MS=${STATE_SESSION_COMMAND_GAP_MS:-100}
REWIND_GAP_MS=50
for path in "$RA" "$CFG" "$CORE" "$CONTENT" "$FRAME_INDEX" "$CALIBRATION"; do
    [ -e "$path" ] || { echo "state_session: INPUT_MISSING" >&2; exit 2; }
done
case "$TIMEOUT_MS:$OPERATION_MS:$CLEANUP_MS:$READY_MS:$PUBLICATION_MS" in
    *[!0-9:]*|::*|:*|*:) echo "state_session: INVALID_BUDGET" >&2; exit 2 ;;
esac
    [ "$TIMEOUT_MS" -gt "$CLEANUP_MS" ] && [ "$OPERATION_MS" -gt 0 ] || { echo "state_session: INVALID_BUDGET" >&2; exit 2; }
mkdir -p "$RA_DIR/attempts" "$LOCK_ROOT"
clock_ms() { perl -MTime::HiRes=clock_gettime,CLOCK_MONOTONIC -e 'printf "%.0f\n",clock_gettime(CLOCK_MONOTONIC)*1000' 2>/dev/null; }
START_MS=$(clock_ms) || { echo "state_session: MONOTONIC_CLOCK_UNAVAILABLE" >&2; exit 2; }
case "$START_MS" in *[!0-9]*|'') echo "state_session: MONOTONIC_CLOCK_UNAVAILABLE" >&2; exit 2 ;; esac
ATTEMPT_ID="attempt-${START_MS}-$$"; ATTEMPT_DIR="$RA_DIR/attempts/$ATTEMPT_ID"
LOCK_DIR="$LOCK_ROOT/nesturbator-retroarch-55355.lock"
mkdir "$LOCK_DIR" 2>/dev/null || { echo "state_session: ENDPOINT_BUSY" >&2; exit 1; }
mkdir -p "$ATTEMPT_DIR/profile" "$ATTEMPT_DIR/states" "$ATTEMPT_DIR/shots" "$ATTEMPT_DIR/originals" "$ATTEMPT_DIR/observations"
JOURNAL="$ATTEMPT_DIR/journal.tsv"
printf 'step\tclass\tsends\tattempt_elapsed_ms\tattempt_remaining_ms\toperation_elapsed_ms\toperation_remaining_ms\texpected\tobserved\tartifact\treason\n' >"$JOURNAL"
HOST_PID=; CHILD_PID=; FIRST_FAILURE=; LAST_REASON=OK; send_count=0; QUIT_SENT=0
CURRENT_STEP=setup; CURRENT_CLASS=none; CURRENT_SENDS=0; CURRENT_EXPECTED=none; CURRENT_OBSERVED=none; CURRENT_ARTIFACT=none
ACTIVE_OPERATION=; OPERATION_START_MS=; OPERATION_DEADLINE_MS=; OPERATION_BUDGET_MS=; OPERATION_STOP_MS=; OPERATION_DEADLINE_VALUE=0
LAST_OP_ELAPSED_MS=0; LAST_OP_REMAINING_MS=0
elapsed_ms() { _n=$(clock_ms) || return 1; [ "$_n" -ge "$START_MS" ] || return 1; echo $((_n-START_MS)); }
remaining_ms() { _e=$(elapsed_ms) || return 1; _r=$((TIMEOUT_MS-_e)); [ "$_r" -gt 0 ] || _r=0; echo "$_r"; }
begin_operation() {
    ACTIVE_OPERATION=$1; OPERATION_BUDGET_MS=$2
    OPERATION_STOP_MS=
    OPERATION_START_MS=$(clock_ms) || fail MONOTONIC_CLOCK_FAILURE
    OPERATION_DEADLINE_MS=$((OPERATION_START_MS+OPERATION_BUDGET_MS))
    LAST_OP_ELAPSED_MS=0; LAST_OP_REMAINING_MS=$OPERATION_BUDGET_MS
}
finish_operation() {
    _now=$(clock_ms) || fail MONOTONIC_CLOCK_FAILURE
    [ "$_now" -ge "$OPERATION_START_MS" ] || fail CLOCK_MOVED_BACKWARD
    [ "$_now" -le "$OPERATION_DEADLINE_MS" ] || fail_operation_deadline
    OPERATION_ELAPSED_MS=$((_now-OPERATION_START_MS))
    LAST_OP_ELAPSED_MS=$OPERATION_ELAPSED_MS; LAST_OP_REMAINING_MS=$((OPERATION_DEADLINE_MS-_now))
    OPERATION_DEADLINE_VALUE=$OPERATION_BUDGET_MS
    ACTIVE_OPERATION=; OPERATION_START_MS=; OPERATION_DEADLINE_MS=; OPERATION_BUDGET_MS=; OPERATION_STOP_MS=
}
operation_left() {
    [ -n "$ACTIVE_OPERATION" ] || { echo 0; return 0; }
    _now=$(clock_ms) || return 1
    [ "$_now" -ge "$OPERATION_START_MS" ] || return 1
    _left=$((OPERATION_DEADLINE_MS-_now)); [ "$_left" -gt 0 ] || return 1
    echo "$_left"
}
host_alive() { [ -n "$HOST_PID" ] && kill -0 "$HOST_PID" 2>/dev/null; }
record_row() {
    _e=$(elapsed_ms 2>/dev/null || echo 0); _r=$(remaining_ms 2>/dev/null || echo 0)
    if [ -n "$ACTIVE_OPERATION" ]; then
        if [ -n "$OPERATION_STOP_MS" ]; then ROW_NOW=$OPERATION_STOP_MS
        else ROW_NOW=$(clock_ms 2>/dev/null || echo "$OPERATION_START_MS")
        fi
        ROW_OP_ELAPSED=$((ROW_NOW-OPERATION_START_MS)); ROW_OP_REMAINING=$((OPERATION_DEADLINE_MS-ROW_NOW))
        [ "$ROW_OP_ELAPSED" -ge 0 ] || ROW_OP_ELAPSED=0; [ "$ROW_OP_REMAINING" -ge 0 ] || ROW_OP_REMAINING=0
    else ROW_OP_ELAPSED=$LAST_OP_ELAPSED_MS; ROW_OP_REMAINING=$LAST_OP_REMAINING_MS
    fi
    _expected=$(printf '%s' "$CURRENT_EXPECTED" | sanitize_text)
    _observed=$(printf '%s' "$CURRENT_OBSERVED" | sanitize_text)
    _artifact=$(printf '%s' "$CURRENT_ARTIFACT" | sanitize_text)
    printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' "$CURRENT_STEP" "$CURRENT_CLASS" "$CURRENT_SENDS" "$_e" "$_r" "$ROW_OP_ELAPSED" "$ROW_OP_REMAINING" "$_expected" "$_observed" "$_artifact" "$LAST_REASON" >>"$JOURNAL"
}
sanitize_text() {
    SESSION_ATTEMPT_DIR=$ATTEMPT_DIR SESSION_PROJECT_ROOT=$PROJECT_ROOT SESSION_RA_DIR=$RA_DIR perl -pe \
        's/\Q$ENV{SESSION_ATTEMPT_DIR}\E/<attempt>/g; s/\Q$ENV{SESSION_PROJECT_ROOT}\E/<workspace>/g; s/\Q$ENV{SESSION_RA_DIR}\E/<retroarch>/g'
}
sanitize_file() {
    SESSION_ATTEMPT_DIR=$ATTEMPT_DIR SESSION_PROJECT_ROOT=$PROJECT_ROOT SESSION_RA_DIR=$RA_DIR perl -pe \
        's/\Q$ENV{SESSION_ATTEMPT_DIR}\E/<attempt>/g; s/\Q$ENV{SESSION_PROJECT_ROOT}\E/<workspace>/g; s/\Q$ENV{SESSION_RA_DIR}\E/<retroarch>/g' "$1" >"$2"
}
reap_one() {
    _pid=$1; [ -n "$_pid" ] || return 0
    kill -TERM "$_pid" 2>/dev/null || true; _until=$(( $(clock_ms) + 350 ))
    while kill -0 "$_pid" 2>/dev/null; do _now=$(clock_ms) || break; [ "$_now" -lt "$_until" ] || break; sleep 0.03; done
    kill -KILL "$_pid" 2>/dev/null || true; wait "$_pid" 2>/dev/null || true
}
nc_exec() {
    if [ -n "${STATE_SESSION_TEST_MODE:-}" ]; then
        exec env "STATE_SESSION_TEST_EXPECTED_DIR=$ATTEMPT_DIR/states" \
            "STATE_SESSION_TEST_SHOTS=$ATTEMPT_DIR/shots" "$NC" "$@"
    fi
    exec "$NC" "$@"
}
cleanup_quit() {
    [ -n "$HOST_PID" ] && host_alive || return 0
    [ "$QUIT_SENT" -eq 0 ] || return 0
    QUIT_SENT=1; printf 'QUIT' >"$REQUEST"
    CURRENT_STEP=cleanup-quit; CURRENT_CLASS=QUIT; CURRENT_SENDS=1
    CURRENT_EXPECTED=QUIT-at-most-once; CURRENT_OBSERVED=cleanup-send-attempt
    nc_exec -u -w0 127.0.0.1 55355 <"$REQUEST" >"$ATTEMPT_DIR/cleanup-quit.out" \
        2>"$ATTEMPT_DIR/cleanup-quit.err" &
    CHILD_PID=$!; _deadline=$(( $(clock_ms) + 300 ))
    while kill -0 "$CHILD_PID" 2>/dev/null; do
        _now=$(clock_ms) || break; [ "$_now" -lt "$_deadline" ] || break; sleep 0.02
    done
    if kill -0 "$CHILD_PID" 2>/dev/null; then reap_one "$CHILD_PID"; else wait "$CHILD_PID" 2>/dev/null || true; fi
    CHILD_PID=; LAST_REASON=${FIRST_FAILURE:-$LAST_REASON}; CURRENT_ARTIFACT=cleanup-quit; record_row
}
cleanup() {
    [ -z "$CHILD_PID" ] || { reap_one "$CHILD_PID"; CHILD_PID=; }
    cleanup_quit
    [ -z "$HOST_PID" ] || { reap_one "$HOST_PID"; HOST_PID=; }
    if [ -f "$ATTEMPT_DIR/session.cfg" ]; then
        sanitize_file "$ATTEMPT_DIR/session.cfg" "$ATTEMPT_DIR/session.sanitized.cfg" 2>/dev/null || true
        rm -f "$ATTEMPT_DIR/session.cfg" 2>/dev/null || true
    fi
    if [ -f "$ATTEMPT_DIR/retroarch.log" ]; then
        sanitize_file "$ATTEMPT_DIR/retroarch.log" "$ATTEMPT_DIR/retroarch.sanitized.log" 2>/dev/null || true
        [ ! -f "$ATTEMPT_DIR/retroarch.sanitized.log" ] || cp "$ATTEMPT_DIR/retroarch.sanitized.log" "$LOG" 2>/dev/null || true
        rm -f "$ATTEMPT_DIR/retroarch.log" 2>/dev/null || true
    fi
    [ ! -d "$LOCK_DIR" ] || rmdir "$LOCK_DIR" 2>/dev/null || true
}
stop_operation_clock() {
    [ -n "$ACTIVE_OPERATION" ] && [ -n "$OPERATION_START_MS" ] || return 0
    [ -z "$OPERATION_STOP_MS" ] || return 0
    OPERATION_STOP_NOW=$(clock_ms) || return 1
    [ "$OPERATION_STOP_NOW" -ge "$OPERATION_START_MS" ] || return 1
    OPERATION_STOP_MS=$OPERATION_STOP_NOW
}
on_signal() { [ -n "$FIRST_FAILURE" ] || FIRST_FAILURE="INTERRUPTED_$1"; stop_operation_clock || true; LAST_REASON=$FIRST_FAILURE; record_row; cleanup; trap - INT TERM HUP; exit 130; }
trap cleanup EXIT; trap 'on_signal INT' INT; trap 'on_signal TERM' TERM; trap 'on_signal HUP' HUP
fail() { stop_operation_clock || true; [ -n "$FIRST_FAILURE" ] || FIRST_FAILURE=$1; LAST_REASON=$FIRST_FAILURE; record_row; echo "state_session: $FIRST_FAILURE" >&2; exit 1; }
fail_operation_deadline() {
    OPERATION_FAILURE_LABEL=$(printf '%s' "$ACTIVE_OPERATION" | tr '[:lower:]-' '[:upper:]_')
    fail "${OPERATION_FAILURE_LABEL}_DEADLINE"
}
run_child() {
    CHILD_LABEL=$1; CHILD_BUDGET=$2; CHILD_INPUT=$3; CHILD_OUT=$4; CHILD_ERR=$5; shift 5
    CHILD_LIMIT_REASON=child
    CHILD_LEFT=$(remaining_ms) || fail MONOTONIC_CLOCK_FAILURE; CHILD_LEFT=$((CHILD_LEFT-CLEANUP_MS)); [ "$CHILD_LEFT" -gt 0 ] || fail ATTEMPT_DEADLINE
    if [ "$CHILD_BUDGET" -gt "$CHILD_LEFT" ]; then CHILD_BUDGET=$CHILD_LEFT; CHILD_LIMIT_REASON=attempt; fi
    if [ -n "$ACTIVE_OPERATION" ]; then
        CHILD_OP_LEFT=$(operation_left) || fail_operation_deadline
        if [ "$CHILD_OP_LEFT" -lt "$CHILD_BUDGET" ]; then CHILD_BUDGET=$CHILD_OP_LEFT; CHILD_LIMIT_REASON=operation; fi
    fi
    CHILD_START=$(clock_ms) || fail MONOTONIC_CLOCK_FAILURE; CHILD_DEADLINE=$((CHILD_START+CHILD_BUDGET))
    "$@" <"$CHILD_INPUT" >"$CHILD_OUT" 2>"$CHILD_ERR" & CHILD_PID=$!
    while kill -0 "$CHILD_PID" 2>/dev/null; do
        [ -z "$HOST_PID" ] || host_alive || { stop_operation_clock || fail MONOTONIC_CLOCK_FAILURE; reap_one "$CHILD_PID"; CHILD_PID=; fail HOST_DIED; }
        CHILD_NOW=$(clock_ms) || fail MONOTONIC_CLOCK_FAILURE; [ "$CHILD_NOW" -ge "$CHILD_START" ] || fail CLOCK_MOVED_BACKWARD
        [ "$CHILD_NOW" -lt "$CHILD_DEADLINE" ] || {
            stop_operation_clock || fail MONOTONIC_CLOCK_FAILURE
            reap_one "$CHILD_PID"; CHILD_PID=
            case "$CHILD_LIMIT_REASON" in
                attempt) fail ATTEMPT_DEADLINE ;;
                operation) fail_operation_deadline ;;
                *)
                    if [ "$CHILD_LABEL" = readiness ]; then return 124; fi
                    CHILD_DEADLINE_LABEL=$(printf '%s' "$CHILD_LABEL" | tr '[:lower:]-' '[:upper:]_'); fail "${CHILD_DEADLINE_LABEL}_DEADLINE"
                    ;;
            esac
        }
        sleep 0.05
    done
    CHILD_STATUS=0; wait "$CHILD_PID" || CHILD_STATUS=$?; CHILD_PID=
    CHILD_END=$(clock_ms) || fail MONOTONIC_CLOCK_FAILURE; [ "$CHILD_END" -ge "$CHILD_START" ] || fail CLOCK_MOVED_BACKWARD
    [ $((CHILD_END-CHILD_START)) -le "$CHILD_BUDGET" ] || {
        case "$CHILD_LIMIT_REASON" in
            attempt) fail ATTEMPT_DEADLINE ;;
            operation) fail_operation_deadline ;;
            *) CHILD_DEADLINE_LABEL=$(printf '%s' "$CHILD_LABEL" | tr '[:lower:]-' '[:upper:]_'); fail "${CHILD_DEADLINE_LABEL}_DEADLINE" ;;
        esac
    }
    [ "$CHILD_STATUS" -eq 0 ] || return "$CHILD_STATUS"; return 0
}
sleep_ms() {
    _delay=$1; _label=$2
    [ "$_delay" -gt 0 ] || return 0
    _budget=$((_delay+1000))
    run_child "delay_$_label" "$_budget" "$EMPTY" "$ATTEMPT_DIR/delay.out" \
        "$ATTEMPT_DIR/delay.err" perl -MTime::HiRes=sleep -e 'sleep($ARGV[0]/1000)' "$_delay" || fail "DELAY_$_label"
}
REQUEST="$ATTEMPT_DIR/request.txt"; REPLY="$ATTEMPT_DIR/reply.txt"; EMPTY="$ATTEMPT_DIR/empty.txt"; : >"$EMPTY"
send_once() {
    _request=$1; _class=$2; CURRENT_STEP=$_class; CURRENT_CLASS=$_class; CURRENT_EXPECTED=$_request; CURRENT_OBSERVED=not-sent
    if [ "$_request" = QUIT ]; then [ "$QUIT_SENT" -eq 0 ] || fail QUIT_DUPLICATE; QUIT_SENT=1; fi
    printf '%s' "$_request" >"$REQUEST"; send_count=$((send_count+1)); CURRENT_SENDS=1
    run_child "send_$_class" "$OPERATION_MS" "$REQUEST" "$ATTEMPT_DIR/send-$send_count.out" "$ATTEMPT_DIR/send-$send_count.err" \
        nc_exec -u -w0 127.0.0.1 55355 || fail "${_class}_SEND_FAILED"
    CURRENT_OBSERVED=send-accepted-locally; LAST_REASON=OK; record_row
}
probe_ready() {
    begin_operation readiness "$READY_MS"; _start=$OPERATION_START_MS; _deadline=$OPERATION_DEADLINE_MS; _tries=0
    while :; do
        host_alive || fail HOST_DIED; _now=$(clock_ms) || fail MONOTONIC_CLOCK_FAILURE
        [ "$_now" -ge "$_start" ] || fail CLOCK_MOVED_BACKWARD; [ "$_now" -lt "$_deadline" ] || fail READINESS_DEADLINE
        _tries=$((_tries+1)); printf 'GET_CONFIG_PARAM savestate_directory' >"$REQUEST"
        CURRENT_STEP=readiness; CURRENT_CLASS=read-only; CURRENT_SENDS=$_tries
        CURRENT_EXPECTED="GET_CONFIG_PARAM savestate_directory $ATTEMPT_DIR/states"; CURRENT_OBSERVED=no-valid-reply
        _left=$((_deadline-_now)); [ "$_left" -le 1000 ] || _left=1000; _secs=$(((_left+999)/1000)); [ "$_secs" -gt 0 ] || _secs=1
        _reply="$ATTEMPT_DIR/reply-$_tries.txt"; _reply_err="$ATTEMPT_DIR/readiness-$_tries.err"
        _read_status=0
        run_child readiness "$_left" "$REQUEST" "$_reply" "$_reply_err" \
            nc_exec -u -w "$_secs" 127.0.0.1 55355 || _read_status=$?
        printf 'GET_CONFIG_PARAM savestate_directory %s\n' "$ATTEMPT_DIR/states" >"$ATTEMPT_DIR/expected.txt"
        printf 'GET_CONFIG_PARAM savestate_directory %s' "$ATTEMPT_DIR/states" >"$ATTEMPT_DIR/expected-no-newline.txt"
        if cmp -s "$_reply" "$ATTEMPT_DIR/expected.txt" || cmp -s "$_reply" "$ATTEMPT_DIR/expected-no-newline.txt"; then
            CURRENT_OBSERVED=exact-isolated-path; LAST_REASON=OK; finish_operation; record_row; return 0
        fi
        if [ "$_read_status" -ne 0 ] && [ "$_read_status" -ne 124 ]; then fail READINESS_TRANSPORT; fi
        if [ -s "$_reply" ]; then CURRENT_OBSERVED=malformed-or-wrong-path; LAST_REASON=READINESS_REPLY; record_row; fail READINESS_REPLY; fi
        CURRENT_OBSERVED=lost-read; LAST_REASON=READINESS_LOST
        record_row; sleep_ms 100 readiness-retry
    done
}

# Shell/awk FNV-1a 64-bit identities for the shared C evidence reader.
fnv64() {
    _size=$(wc -c <"$1" | tr -d ' '); [ "$_size" -gt 0 ] && [ "$_size" -le 1048576 ] || return 1
    perl -e 'use integer; my $h=14695981039346656037; while(read STDIN,my $b,65536){ for my $x (unpack("C*",$b)){ $h ^= $x; $h *= 1099511628211 } } printf "%016x\n",$h' <"$1"
}
publish_image() {
    _tag=$1; _now=$(clock_ms) || fail MONOTONIC_CLOCK_FAILURE
    _attempt_left=$(remaining_ms) || fail MONOTONIC_CLOCK_FAILURE
    _attempt_end=$((_now+_attempt_left-CLEANUP_MS)); _end=$((_now+PUBLICATION_MS))
    [ "$_end" -le "$_attempt_end" ] || _end=$_attempt_end
    [ -z "$ACTIVE_OPERATION" ] || [ "$OPERATION_DEADLINE_MS" -ge "$_end" ] || _end=$OPERATION_DEADLINE_MS
    _last=; _stable=0
    while :; do
        [ -z "$HOST_PID" ] || host_alive || fail HOST_DIED; _count=0; _file=
        for _f in "$ATTEMPT_DIR/shots"/*; do [ -f "$_f" ] || continue; _count=$((_count+1)); _file=$_f; done
        [ "$_count" -le 1 ] || fail EXTRA_PUBLICATION
        if [ "$_count" -eq 1 ] && [ -s "$_file" ]; then
            _hash=$(fnv64 "$_file") || fail PUBLICATION_SIZE
            if [ "$_hash" = "$_last" ]; then _stable=$((_stable+1)); else _last=$_hash; _stable=0; fi
            if [ "$_stable" -ge 1 ]; then
                ORIGINAL_NAME=${_file##*/}; ORIGINAL_PATH="$ATTEMPT_DIR/originals/${_tag}-${ORIGINAL_NAME}"
                [ ! -e "$ORIGINAL_PATH" ] || fail PUBLICATION_ATTRIBUTION
                cp "$_file" "$ORIGINAL_PATH" || fail PUBLICATION_COPY; rm "$_file" || fail PUBLICATION_CLEAR
                case "$ORIGINAL_NAME" in
                    *.ppm) OBSERVATION_PATH="$ATTEMPT_DIR/observations/${_tag}.ppm"; cp "$ORIGINAL_PATH" "$OBSERVATION_PATH" || fail OBSERVATION_COPY ;;
                    *.png)
                        _clean="$ATTEMPT_DIR/observations/${_tag}-clean.png"; OBSERVATION_PATH="$ATTEMPT_DIR/observations/${_tag}.bmp"
                        run_child "sips-strip-$_tag" "$OPERATION_MS" "$EMPTY" "$ATTEMPT_DIR/sips-$_tag.out" "$ATTEMPT_DIR/sips-$_tag.err" sips --deleteColorManagementProperties "$ORIGINAL_PATH" --out "$_clean" || fail SIPS_STRIP
                        run_child "sips-convert-$_tag" "$OPERATION_MS" "$EMPTY" "$ATTEMPT_DIR/convert-$_tag.out" "$ATTEMPT_DIR/convert-$_tag.err" sips -s format bmp "$_clean" --out "$OBSERVATION_PATH" || fail SIPS_CONVERT ;;
                    *) fail PUBLICATION_FORMAT ;;
                esac
                [ -s "$OBSERVATION_PATH" ] || fail PUBLICATION_PARTIAL; ORIGINAL_HASH=$_hash; return 0
            fi
        fi
        _now=$(clock_ms) || fail MONOTONIC_CLOCK_FAILURE; [ "$_now" -lt "$_end" ] || { [ "$_count" -eq 0 ] && fail SCREENSHOT_MISSING; fail PUBLICATION_UNSTABLE; }
        sleep_ms 50 publication-poll
    done
}
observe_image() {
    _tag=$1
    run_child marker-observe "$OPERATION_MS" "$EMPTY" "$ATTEMPT_DIR/observe-$_tag.out" "$ATTEMPT_DIR/observe-$_tag.err" \
        "$FRAME_INDEX" --marker-observe "$CONTENT" "$CALIBRATION" "$OBSERVATION_PATH" || fail MARKER_OBSERVE
    _line=$(cat "$ATTEMPT_DIR/observe-$_tag.out"); OBSERVED_FRAME=$(printf '%s\n' "$_line" | sed -n 's/^marker frame=\([0-9][0-9]*\) guest=[0-9][0-9]*$/\1/p')
    [ -n "$OBSERVED_FRAME" ] || fail MARKER_OBSERVE_PARSE; [ "$OBSERVED_FRAME" -ge 4 ] && [ "$OBSERVED_FRAME" -le 10000 ] || fail MARKER_FRAME_RANGE
}
capture() {
    _owns_operation=0
    if [ -z "$ACTIVE_OPERATION" ]; then begin_operation "capture-$1" "$OPERATION_MS"; _owns_operation=1; fi
    _tag=$1; CURRENT_STEP=$_tag; CURRENT_CLASS=SCREENSHOT; CURRENT_EXPECTED=one-complete-image; CURRENT_OBSERVED=awaiting-publication
    for _f in "$ATTEMPT_DIR/shots"/*; do [ ! -e "$_f" ] || fail STALE_PUBLICATION; done
    send_once SCREENSHOT SCREENSHOT; publish_image "$_tag"; observe_image "$_tag"
    BOUNDARY_FRAME=$OBSERVED_FRAME; BOUNDARY_PATH=$OBSERVATION_PATH; BOUNDARY_NAME=$ORIGINAL_NAME; BOUNDARY_HASH=$ORIGINAL_HASH
    if [ "$_owns_operation" -eq 1 ]; then finish_operation; BOUNDARY_ELAPSED_MS=$OPERATION_ELAPSED_MS; else BOUNDARY_ELAPSED_MS=; fi
    BOUNDARY_DEADLINE_MS=$OPERATION_MS
    CURRENT_OBSERVED="frame-$BOUNDARY_FRAME"; CURRENT_ARTIFACT="$BOUNDARY_NAME:$BOUNDARY_HASH"; LAST_REASON=OK
}
BOUNDARIES="$ATTEMPT_DIR/boundaries.tsv"; : >"$BOUNDARIES"; BOUNDARY_COUNT=0
ORIGINAL_ANCHOR_PATH=; ORIGINAL_ANCHOR_ID=; ORIGINAL_ANCHOR_FRAME=0
REWOUND_ANCHOR_PATH=; REWOUND_ANCHOR_ID=; REWOUND_ANCHOR_FRAME=0
record_boundary() {
    _op=$1; _relation=$2; _origin=$3; _commands=$4; _sends=$5; _anchor_id=$6
    _anchor_frame=$7; _lead=$8; _delta=$9; _elapsed=${10}; _deadline=${11}
    host_alive || fail HOST_DIED
    _hash=$(fnv64 "$BOUNDARY_PATH") || fail IMAGE_HASH
    BOUNDARY_COUNT=$((BOUNDARY_COUNT+1)); _artifact_id=$((START_MS+BOUNDARY_COUNT))
    printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t1\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t1\n' \
        "$BOUNDARY_COUNT" "$_op" "$_relation" "$_origin" "$_commands" "$_sends" \
        "$_artifact_id" "$BOUNDARY_PATH" "$_hash" "$BOUNDARY_FRAME" "$BOUNDARY_FRAME" "$BOUNDARY_FRAME" \
        "$_anchor_id" "$_anchor_frame" "$_lead" "$_delta" "$_elapsed" "$_deadline" >>"$BOUNDARIES"
    CURRENT_STEP="boundary-$BOUNDARY_COUNT"; CURRENT_CLASS="$_op"; CURRENT_SENDS="$_sends"
    CURRENT_EXPECTED="$_relation from $_origin"; CURRENT_OBSERVED="frame-$BOUNDARY_FRAME"
    CURRENT_ARTIFACT="$BOUNDARY_NAME:$ORIGINAL_HASH"; LAST_REASON=OK; record_row
}
record_path() {
    _record_path=$1
    [ -n "$_record_path" ] || { printf '\n'; return 0; }
    if [ -n "${STATE_SESSION_RECORD_ROOT:-}" ]; then
        case "$_record_path" in
            "$STATE_SESSION_RECORD_ROOT"/*) _record_path=${_record_path#"$STATE_SESSION_RECORD_ROOT"/} ;;
            *) return 1 ;;
        esac
    fi
    printf '%s\n' "$_record_path"
}
write_record() {
    _mode=$1; _terminal=$2; _record="$ATTEMPT_DIR/progression-session.rec"
    _content_id=$(awk -F= '$1=="content_identity"{print $2;n++}END{if(n!=1)exit 1}' "$ATTEMPT_DIR/oracle-fixture/progression-prefix.rec") || fail FIXTURE_IDENTITY
    _cal_id=$(awk -F= '$1=="calibration_identity"{print $2;n++}END{if(n!=1)exit 1}' "$ATTEMPT_DIR/oracle-fixture/progression-prefix.rec") || fail FIXTURE_IDENTITY
    _original_record_path=$(record_path "$ORIGINAL_ANCHOR_PATH") || fail RECORD_PATH
    _rewound_record_path=$(record_path "$REWOUND_ANCHOR_PATH") || fail RECORD_PATH
    {
        printf 'NESTURBATOR-PROGRESSION 1\nmode=%s\ncontent_identity=%s\ncalibration_identity=%s\nterminal_exit=%s\n' "$_mode" "$_content_id" "$_cal_id" "$_terminal"
        printf 'original_anchor_path_length=%s\noriginal_anchor_path=%s\noriginal_anchor_identity=%s\noriginal_anchor_frame=%s\n' \
            "${#_original_record_path}" "$_original_record_path" "$ORIGINAL_ANCHOR_ID" "$ORIGINAL_ANCHOR_FRAME"
        printf 'rewound_anchor_path_length=%s\nrewound_anchor_path=%s\nrewound_anchor_identity=%s\nrewound_anchor_frame=%s\n' \
            "${#_rewound_record_path}" "$_rewound_record_path" "${REWOUND_ANCHOR_ID:-0000000000000000}" "$REWOUND_ANCHOR_FRAME"
        printf 'boundary_count=%s\n' "$BOUNDARY_COUNT"
        while IFS="$(printf '\t')" read -r _id _op _rel _origin _commands _sends _artifact_id _path _hash _candidate_count _frame _candidate_first _candidate_last _anchor_id _anchor_frame _lead _delta _elapsed _deadline _alive; do
            [ "$_candidate_count" = 1 ] || fail INTERNAL_CANDIDATE_COUNT
            _record_artifact_path=$(record_path "$_path") || fail RECORD_PATH
            printf 'boundary\nid=%s\noperation=%s\nrelation=%s\norigin=%s\ncommand_count=%s\nsend_count=%s\nartifact_identity=%s\n' \
                "$_id" "$_op" "$_rel" "$_origin" "$_commands" "$_sends" "$_artifact_id"
            printf 'artifact_path_length=%s\nartifact_path=%s\nartifact_hash=%s\ndisplay_frame=%s\ncandidate_count=1\ncandidate_first=%s\ncandidate_last=%s\n' \
                "${#_record_artifact_path}" "$_record_artifact_path" "$_hash" "$_frame" "$_frame" "$_frame"
            printf 'anchor_identity=%s\nanchor_frame=%s\ndisplay_lead=%s\nexpected_delta=%s\nelapsed_ms=%s\ndeadline_ms=%s\nprocess_alive=%s\nboundary_end\n' \
                "$_anchor_id" "$_anchor_frame" "$_lead" "$_delta" "$_elapsed" "$_deadline" "$_alive"
        done <"$BOUNDARIES"
        printf 'END\n'
    } >"$_record" || fail RECORD_WRITE
}
production_verdict() {
    _kind=$1; _mode=$2; _terminal=$3
    write_record "$_mode" "$_terminal"
    if [ "$_mode" = prefix ]; then
        _command=--marker-verdict-prefix
        _expected="progression boundaries=$BOUNDARY_COUNT mode=prefix verdict=PREFIX_OK"
    else
        _command=--marker-verdict
        _expected='progression boundaries=16 mode=complete verdict=VERDICT_OK'
    fi
    _status=0
    _oracle_out="$ATTEMPT_DIR/oracle-${_kind}-${BOUNDARY_COUNT}.out"; _oracle_err="$ATTEMPT_DIR/oracle-${_kind}-${BOUNDARY_COUNT}.err"
    run_child "oracle_$_kind" "$OPERATION_MS" "$EMPTY" "$_oracle_out" \
        "$_oracle_err" "$FRAME_INDEX" "$_command" "$CONTENT" "$CALIBRATION" \
        "$ATTEMPT_DIR/progression-session.rec" || _status=$?
    if [ "$_status" -eq 0 ] && grep -Fx "$_expected" "$_oracle_out" >/dev/null; then
        LAST_REASON=OK; CURRENT_STEP="oracle-$_kind"; CURRENT_CLASS=oracle; CURRENT_SENDS=0
        CURRENT_EXPECTED="$_expected"; CURRENT_OBSERVED="$_expected"; CURRENT_ARTIFACT=progression-session.rec; record_row
        cat "$_oracle_out"; return 0
    fi
    _reason=$(sed -n 's/.* verdict=\([A-Z_]*\).*/\1/p' "$_oracle_out" | tail -1)
    [ -n "$_reason" ] || _reason=$(sed -n 's/^marker: \([A-Z_]*\) .*/\1/p' "$_oracle_err" | tail -1)
    [ -n "$_reason" ] || _reason=ORACLE_EXECUTION
    if [ "$_reason" = PROGRESSION ]; then
        case "$CURRENT_EXPECTED" in *-plus\ from\ *) _reason=ADVANCE_NOT_EXACTLY_ONE ;; esac
    fi
    fail "$_reason"
}
verify_prefix() { production_verdict prefix prefix running; }
state_path() {
    _count=0; _found=
    for _f in "$ATTEMPT_DIR/states"/*/"$STEM".state; do
        [ -f "$_f" ] || continue; _count=$((_count+1)); _found=$_f
    done
    [ "$_count" -gt 0 ] || return 1
    [ "$_count" -eq 1 ] || return 2
    printf '%s\n' "$_found"
}
wait_state_stable() {
    _old_hash=$1; _deadline=$(( $(clock_ms)+PUBLICATION_MS))
    [ -z "$ACTIVE_OPERATION" ] || [ "$OPERATION_DEADLINE_MS" -ge "$_deadline" ] || _deadline=$OPERATION_DEADLINE_MS
    _attempt_end=$((START_MS+TIMEOUT_MS-CLEANUP_MS)); [ "$_attempt_end" -ge "$_deadline" ] || _deadline=$_attempt_end
    _last_hash=; _stable=0
    while :; do
        host_alive || fail HOST_DIED
        _state_status=0; _path=$(state_path) || _state_status=$?
        [ "$_state_status" -ne 2 ] || fail STATE_AMBIGUOUS
        if [ -n "$_path" ] && [ -s "$_path" ]; then
            _hash=$(fnv64 "$_path") || fail STATE_SIZE
            if [ "$_hash" = "$_last_hash" ]; then _stable=$((_stable+1)); else _last_hash=$_hash; _stable=0; fi
            if [ "$_stable" -ge 1 ]; then
                [ -z "$_old_hash" ] || [ "$_hash" != "$_old_hash" ] || fail STATE_UNCHANGED
                STATE_PATH=$_path; STATE_HASH=$_hash; return 0
            fi
        fi
        _now=$(clock_ms) || fail MONOTONIC_CLOCK_FAILURE
        [ "$_now" -lt "$_deadline" ] || fail STATE_MISSING
        [ -z "$ACTIVE_OPERATION" ] || [ "$(operation_left)" -gt 0 ] || fail_operation_deadline
        sleep_ms 50 state-poll
    done
}
save_anchor() {
    _which=$1; _frame=$2; _old_hash=$3; _owns_operation=0
    if [ -z "$ACTIVE_OPERATION" ]; then begin_operation "save-$_which" "$OPERATION_MS"; _owns_operation=1; fi
    send_once SAVE_STATE "save-$_which"
    wait_state_stable "$_old_hash"
    case "$_which" in
        original) ANCHOR_PATH="$ATTEMPT_DIR/original-anchor.state" ;;
        rewound) ANCHOR_PATH="$ATTEMPT_DIR/rewound-anchor.state" ;;
        *) fail INTERNAL_ANCHOR_KIND ;;
    esac
    cp "$STATE_PATH" "$ANCHOR_PATH" || fail ANCHOR_COPY
    _anchor_out="$ATTEMPT_DIR/anchor-$_which.out"; _anchor_err="$ATTEMPT_DIR/anchor-$_which.err"
    if ! run_child "anchor-$_which" "$OPERATION_MS" "$EMPTY" "$_anchor_out" \
        "$_anchor_err" "$FRAME_INDEX" --marker-anchor "$CONTENT" "$CALIBRATION" \
        "$ANCHOR_PATH" "$_frame"; then
        _anchor_reason=$(sed -n 's/^marker: \([a-z-]*\)$/\1/p' "$_anchor_err" | tr '[:lower:]-' '[:upper:]_')
        [ -n "$_anchor_reason" ] || _anchor_reason=EXECUTION
        fail "ANCHOR_$_anchor_reason"
    fi
    _anchor_id=$(fnv64 "$ANCHOR_PATH") || fail ANCHOR_HASH
    case "$_which" in
        original) ORIGINAL_ANCHOR_PATH=$ANCHOR_PATH; ORIGINAL_ANCHOR_ID=$_anchor_id; ORIGINAL_ANCHOR_FRAME=$_frame ;;
        rewound) REWOUND_ANCHOR_PATH=$ANCHOR_PATH; REWOUND_ANCHOR_ID=$_anchor_id; REWOUND_ANCHOR_FRAME=$_frame ;;
    esac
    if [ "$_owns_operation" -eq 1 ]; then finish_operation; fi
}

mkdir -p "$ATTEMPT_DIR/oracle-fixture"
run_child fixture "$OPERATION_MS" "$EMPTY" "$ATTEMPT_DIR/fixture.out" "$ATTEMPT_DIR/fixture.err" \
    "$FRAME_INDEX" --progression-selftest "$ATTEMPT_DIR/oracle-fixture" || fail ORACLE_FIXTURE
cmp -s "$CONTENT" "$ATTEMPT_DIR/oracle-fixture/marker.nes" || fail CONTENT_FIXTURE_MISMATCH
cmp -s "$CALIBRATION" "$ATTEMPT_DIR/oracle-fixture/calibration.txt" || fail CALIBRATION_FIXTURE_MISMATCH
awk -v old="$RA_DIR" -v new="$ATTEMPT_DIR" '{line=$0;at=index(line,old);if(at>0)line=substr(line,1,at-1)new substr(line,at+length(old));print line}' "$CFG" >"$ATTEMPT_DIR/session.cfg" || fail CONFIG_COPY
env "HOME=$ATTEMPT_DIR/profile" "CFFIXED_USER_HOME=$ATTEMPT_DIR/profile" "XDG_CONFIG_HOME=$ATTEMPT_DIR/profile/.config" \
    "XDG_DATA_HOME=$ATTEMPT_DIR/profile/.local/share" "XDG_CACHE_HOME=$ATTEMPT_DIR/profile/.cache" \
    "$RA" --verbose -c "$ATTEMPT_DIR/session.cfg" -L "$CORE" "$CONTENT" >"$ATTEMPT_DIR/retroarch.log" 2>&1 &
HOST_PID=$!
probe_ready
sleep_ms "$SETTLE_MS" startup-settle; host_alive || fail HOST_DIED
begin_operation pause-original "$OPERATION_MS"; send_once PAUSE_TOGGLE pause-original; sleep_ms "$COMMAND_GAP_MS" pause-settle; finish_operation

# First capture supplies the exact expected origin; save and validate it before any frame change.
capture boundary-01; FRAME_1=$BOUNDARY_FRAME
save_anchor original "$FRAME_1" ""
record_boundary capture pause-equal-original original 1 1 "$ORIGINAL_ANCHOR_ID" "$ORIGINAL_ANCHOR_FRAME" 0 0 "$BOUNDARY_ELAPSED_MS" "$BOUNDARY_DEADLINE_MS"
verify_prefix
sleep_ms "$PAUSE_GAP_MS" first-pause-gap
capture boundary-02
[ "$BOUNDARY_FRAME" -eq "$FRAME_1" ] || fail PAUSE_DID_NOT_HOLD
record_boundary capture pause-equal-original original 1 1 "$ORIGINAL_ANCHOR_ID" "$ORIGINAL_ANCHOR_FRAME" 0 0 "$BOUNDARY_ELAPSED_MS" "$BOUNDARY_DEADLINE_MS"
verify_prefix

_n=1
while [ "$_n" -le 5 ]; do
    begin_operation "advance-$_n" "$OPERATION_MS"; send_once FRAMEADVANCE "advance-$_n"
    sleep_ms "$COMMAND_GAP_MS" "advance-$_n"; capture "boundary-0$((_n+2))"
    finish_operation
    record_boundary advance original-plus original 1 1 "$ORIGINAL_ANCHOR_ID" "$ORIGINAL_ANCHOR_FRAME" 0 "$_n" "$OPERATION_ELAPSED_MS" "$OPERATION_DEADLINE_VALUE"
    verify_prefix
    if [ "${STATE_SESSION_TEST_STOP_AFTER_PREFIX:-0}" = 1 ] && [ "${STATE_SESSION_TEST_MODE:-}" = prefix ] && [ "$_n" -eq 1 ]; then exit 0; fi
    _n=$((_n+1))
done

begin_operation load-probe "$OPERATION_MS"; send_once 'LOAD_STATE_SLOT 0' load-probe
send_once FRAMEADVANCE load-probe; sleep_ms "$COMMAND_GAP_MS" load-probe; capture boundary-08; finish_operation
record_boundary load-probe original-load-probe original 2 2 "$ORIGINAL_ANCHOR_ID" "$ORIGINAL_ANCHOR_FRAME" 0 1 "$OPERATION_ELAPSED_MS" "$OPERATION_DEADLINE_VALUE"; verify_prefix

_n=1
while [ "$_n" -le 2 ]; do
    begin_operation "post-load-$_n" "$OPERATION_MS"; send_once FRAMEADVANCE "post-load-$_n"
    sleep_ms "$COMMAND_GAP_MS" "post-load-$_n"; capture "boundary-0$((_n+8))"; finish_operation
    record_boundary advance original-post-load original 1 1 "$ORIGINAL_ANCHOR_ID" "$ORIGINAL_ANCHOR_FRAME" 0 "$((_n+1))" "$OPERATION_ELAPSED_MS" "$OPERATION_DEADLINE_VALUE"; verify_prefix
    _n=$((_n+1))
done

begin_operation resume-pause "$OPERATION_MS"; send_once PAUSE_TOGGLE resume
sleep_ms "$PROGRESS_WAIT_MS" observed-forward-progress
send_once PAUSE_TOGGLE pause-after-progress; sleep_ms "$COMMAND_GAP_MS" pause-after-progress
capture boundary-11
[ "$BOUNDARY_FRAME" -gt $((ORIGINAL_ANCHOR_FRAME+3)) ] || fail RESUME_NOT_FORWARD
finish_operation
record_boundary resume-and-pause resume-forward original 2 2 "$ORIGINAL_ANCHOR_ID" "$ORIGINAL_ANCHOR_FRAME" 0 0 "$OPERATION_ELAPSED_MS" "$OPERATION_DEADLINE_VALUE"; verify_prefix

begin_operation rewind-burst "$OPERATION_MS"; _n=1
while [ "$_n" -le 40 ]; do send_once REWIND "rewind-$_n"; sleep_ms "$REWIND_GAP_MS" "rewind-$_n"; _n=$((_n+1)); done
capture boundary-12; FRAME_12=$BOUNDARY_FRAME
save_anchor rewound "$FRAME_12" "$STATE_HASH"
finish_operation
record_boundary rewind-burst rewind-coarse rewound 40 40 "$REWOUND_ANCHOR_ID" "$REWOUND_ANCHOR_FRAME" 0 0 "$OPERATION_ELAPSED_MS" "$OPERATION_DEADLINE_VALUE"; verify_prefix

_n=1
while [ "$_n" -le 3 ]; do
    begin_operation "rewound-advance-$_n" "$OPERATION_MS"; send_once FRAMEADVANCE "rewound-advance-$_n"
    sleep_ms "$COMMAND_GAP_MS" "rewound-advance-$_n"; capture "boundary-1$((_n+2))"; finish_operation
    record_boundary advance rewound-plus rewound 1 1 "$REWOUND_ANCHOR_ID" "$REWOUND_ANCHOR_FRAME" 0 "$_n" "$OPERATION_ELAPSED_MS" "$OPERATION_DEADLINE_VALUE"; verify_prefix
    _n=$((_n+1))
done

begin_operation runahead-toggle "$OPERATION_MS"; send_once RUNAHEAD_TOGGLE runahead-toggle; finish_operation
begin_operation runahead-load-probe "$OPERATION_MS"; send_once 'LOAD_STATE_SLOT 0' runahead-load-probe
send_once FRAMEADVANCE runahead-load-probe; sleep_ms "$COMMAND_GAP_MS" runahead-load-probe
capture boundary-16; finish_operation
record_boundary runahead-load-probe rewound-load-probe rewound 2 2 "$REWOUND_ANCHOR_ID" "$REWOUND_ANCHOR_FRAME" 1 1 "$OPERATION_ELAPSED_MS" "$OPERATION_DEADLINE_VALUE"; verify_prefix

begin_operation quit "$OPERATION_MS"; send_once QUIT quit; _deadline=$OPERATION_DEADLINE_MS
while kill -0 "$HOST_PID" 2>/dev/null; do
    _now=$(clock_ms) || fail MONOTONIC_CLOCK_FAILURE
    [ "$_now" -lt "$_deadline" ] || fail QUIT_DEADLINE
    [ "$_now" -lt "$((START_MS+TIMEOUT_MS-CLEANUP_MS))" ] || fail ATTEMPT_DEADLINE
    if [ -n "${STATE_SESSION_TEST_QUIT_GRACE_MS:-}" ]; then
        _test_quit_deadline=$((OPERATION_START_MS+STATE_SESSION_TEST_QUIT_GRACE_MS))
        [ "$_now" -lt "$_test_quit_deadline" ] || fail QUIT_DEADLINE
    fi
    sleep 0.05
done
_status=0; wait "$HOST_PID" || _status=$?; HOST_PID=
[ "$_status" -eq 0 ] || fail HOST_EXIT
finish_operation
production_verdict complete complete 0
LAST_REASON=VERDICT_OK; CURRENT_STEP=complete; CURRENT_CLASS=cleanup; CURRENT_EXPECTED=host-exited-zero
CURRENT_OBSERVED=host-exited-zero; CURRENT_ARTIFACT=journal.tsv; record_row
printf 'state_session: attempt=%s verdict=VERDICT_OK boundaries=%s sends=%s journal=<attempt>/journal.tsv\n' \
    "$ATTEMPT_ID" "$BOUNDARY_COUNT" "$send_count"
