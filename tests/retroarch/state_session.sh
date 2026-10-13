#!/bin/sh
# Bounded owned RetroArch session. Arguments: RETROARCH CFG CORE CONTENT RA_DIR LOG FRAME_INDEX CALIBRATION.
# Each run gets its own HOME, config, screenshots, states, journal and verifier record. Only an
# exact read-only readiness query retries; action and publication requests are one-shot.
set -eu
[ "$#" -eq 8 ] || { echo "usage: state_session.sh RETROARCH CFG CORE CONTENT RA_DIR LOG FRAME_INDEX CALIBRATION" >&2; exit 2; }
RA=$1; CFG=$2; CORE=$3; CONTENT=$4; RA_DIR=$5; LOG=$6; FRAME_INDEX=$7; CALIBRATION=$8
STEM=${CONTENT##*/}; STEM=${STEM%.*}; NC=${STATE_SESSION_NC:-nc}
TIMEOUT_MS=${STATE_SESSION_ATTEMPT_MS:-120000}; OPERATION_MS=${STATE_SESSION_OPERATION_MS:-10000}
CLEANUP_MS=${STATE_SESSION_CLEANUP_MS:-5000}; READY_MS=${STATE_SESSION_READY_MS:-30000}
PUBLICATION_MS=${STATE_SESSION_PUBLICATION_MS:-5000}; LOCK_ROOT=${STATE_SESSION_LOCK_ROOT:-${TMPDIR:-/tmp}}
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
printf 'step\tclass\tsends\telapsed_ms\tremaining_ms\texpected\tobserved\tartifact\treason\n' >"$JOURNAL"
HOST_PID=; CHILD_PID=; FIRST_FAILURE=; LAST_REASON=OK; send_count=0
CURRENT_STEP=setup; CURRENT_CLASS=none; CURRENT_SENDS=0; CURRENT_EXPECTED=none; CURRENT_OBSERVED=none; CURRENT_ARTIFACT=none
ACTIVE_OPERATION=; OPERATION_START_MS=; OPERATION_DEADLINE_MS=; OPERATION_BUDGET_MS=
elapsed_ms() { _n=$(clock_ms) || return 1; [ "$_n" -ge "$START_MS" ] || return 1; echo $((_n-START_MS)); }
remaining_ms() { _e=$(elapsed_ms) || return 1; _r=$((TIMEOUT_MS-_e)); [ "$_r" -gt 0 ] || _r=0; echo "$_r"; }
begin_operation() {
    ACTIVE_OPERATION=$1; OPERATION_BUDGET_MS=$2
    OPERATION_START_MS=$(clock_ms) || fail MONOTONIC_CLOCK_FAILURE
    OPERATION_DEADLINE_MS=$((OPERATION_START_MS+OPERATION_BUDGET_MS))
}
finish_operation() {
    _now=$(clock_ms) || fail MONOTONIC_CLOCK_FAILURE
    [ "$_now" -ge "$OPERATION_START_MS" ] || fail CLOCK_MOVED_BACKWARD
    [ "$_now" -le "$OPERATION_DEADLINE_MS" ] || fail "${ACTIVE_OPERATION}_DEADLINE"
    OPERATION_ELAPSED_MS=$((_now-OPERATION_START_MS))
    OPERATION_DEADLINE_VALUE=$OPERATION_BUDGET_MS
    ACTIVE_OPERATION=; OPERATION_START_MS=; OPERATION_DEADLINE_MS=; OPERATION_BUDGET_MS=
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
    printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' "$CURRENT_STEP" "$CURRENT_CLASS" "$CURRENT_SENDS" "$_e" "$_r" "$CURRENT_EXPECTED" "$CURRENT_OBSERVED" "$CURRENT_ARTIFACT" "$LAST_REASON" >>"$JOURNAL"
}
reap_one() {
    _pid=$1; [ -n "$_pid" ] || return 0
    kill -TERM "$_pid" 2>/dev/null || true; _until=$(( $(clock_ms) + 350 ))
    while kill -0 "$_pid" 2>/dev/null; do _now=$(clock_ms) || break; [ "$_now" -lt "$_until" ] || break; sleep 0.03; done
    kill -KILL "$_pid" 2>/dev/null || true; wait "$_pid" 2>/dev/null || true
}
cleanup() {
    [ -z "$CHILD_PID" ] || { reap_one "$CHILD_PID"; CHILD_PID=; }
    [ -z "$HOST_PID" ] || { reap_one "$HOST_PID"; HOST_PID=; }
    [ ! -s "$ATTEMPT_DIR/retroarch.log" ] || cp "$ATTEMPT_DIR/retroarch.log" "$LOG" 2>/dev/null || true
    [ ! -d "$LOCK_DIR" ] || rmdir "$LOCK_DIR" 2>/dev/null || true
}
on_signal() { [ -n "$FIRST_FAILURE" ] || FIRST_FAILURE="INTERRUPTED_$1"; LAST_REASON=$FIRST_FAILURE; record_row; cleanup; trap - INT TERM HUP; exit 130; }
trap cleanup EXIT; trap 'on_signal INT' INT; trap 'on_signal TERM' TERM; trap 'on_signal HUP' HUP
fail() { [ -n "$FIRST_FAILURE" ] || FIRST_FAILURE=$1; LAST_REASON=$FIRST_FAILURE; record_row; echo "state_session: $FIRST_FAILURE" >&2; exit 1; }
run_child() {
    _label=$1; _budget=$2; _input=$3; _out=$4; _err=$5; shift 5
    _left=$(remaining_ms) || fail MONOTONIC_CLOCK_FAILURE; _left=$((_left-CLEANUP_MS)); [ "$_left" -gt 0 ] || fail ATTEMPT_DEADLINE
    if [ -n "$ACTIVE_OPERATION" ]; then
        _op_left=$(operation_left) || fail "${ACTIVE_OPERATION}_DEADLINE"
        [ "$_op_left" -le "$_left" ] || _op_left=$_left
        _left=$_op_left
    fi
    [ "$_budget" -le "$_left" ] || _budget=$_left; _start=$(clock_ms) || fail MONOTONIC_CLOCK_FAILURE; _deadline=$((_start+_budget))
    "$@" <"$_input" >"$_out" 2>"$_err" & CHILD_PID=$!
    while kill -0 "$CHILD_PID" 2>/dev/null; do
        [ -z "$HOST_PID" ] || host_alive || { reap_one "$CHILD_PID"; CHILD_PID=; fail HOST_DIED; }
        _now=$(clock_ms) || fail MONOTONIC_CLOCK_FAILURE; [ "$_now" -ge "$_start" ] || fail CLOCK_MOVED_BACKWARD
        [ "$_now" -lt "$_deadline" ] || { reap_one "$CHILD_PID"; CHILD_PID=; fail "${_label}_DEADLINE"; }
        sleep 0.05
    done
    _status=0; wait "$CHILD_PID" || _status=$?; CHILD_PID=
    _end=$(clock_ms) || fail MONOTONIC_CLOCK_FAILURE; [ "$_end" -ge "$_start" ] || fail CLOCK_MOVED_BACKWARD
    [ $((_end-_start)) -le "$_budget" ] || fail "${_label}_DEADLINE"
    [ "$_status" -eq 0 ] || return "$_status"; return 0
}
REQUEST="$ATTEMPT_DIR/request.txt"; REPLY="$ATTEMPT_DIR/reply.txt"; EMPTY="$ATTEMPT_DIR/empty.txt"; : >"$EMPTY"
send_once() {
    _request=$1; _class=$2; CURRENT_STEP=$_class; CURRENT_CLASS=$_class; CURRENT_EXPECTED=$_request; CURRENT_OBSERVED=not-sent
    printf '%s' "$_request" >"$REQUEST"; send_count=$((send_count+1)); CURRENT_SENDS=1
    run_child "send_$_class" "$OPERATION_MS" "$REQUEST" "$ATTEMPT_DIR/send.out" "$ATTEMPT_DIR/send.err" \
        env "STATE_SESSION_TEST_EXPECTED_DIR=$ATTEMPT_DIR/states" \
        "STATE_SESSION_TEST_SHOTS=$ATTEMPT_DIR/shots" "$NC" -u -w0 127.0.0.1 55355 || fail "${_class}_SEND_FAILED"
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
        if run_child readiness "$_left" "$REQUEST" "$REPLY" "$ATTEMPT_DIR/readiness.err" \
            env "STATE_SESSION_TEST_EXPECTED_DIR=$ATTEMPT_DIR/states" "$NC" -u -w "$_secs" 127.0.0.1 55355; then
            printf 'GET_CONFIG_PARAM savestate_directory %s\n' "$ATTEMPT_DIR/states" >"$ATTEMPT_DIR/expected.txt"
            printf 'GET_CONFIG_PARAM savestate_directory %s' "$ATTEMPT_DIR/states" >"$ATTEMPT_DIR/expected-no-newline.txt"
            if cmp -s "$REPLY" "$ATTEMPT_DIR/expected.txt" || cmp -s "$REPLY" "$ATTEMPT_DIR/expected-no-newline.txt"; then
                CURRENT_OBSERVED=exact-isolated-path; LAST_REASON=OK; finish_operation; record_row; return 0
            fi
            CURRENT_OBSERVED=malformed-or-wrong-path; LAST_REASON=READINESS_REPLY
        else CURRENT_OBSERVED=lost-read; LAST_REASON=READINESS_LOST
        fi
        record_row; sleep 0.1
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
    [ -z "$ACTIVE_OPERATION" ] || [ "$OPERATION_DEADLINE_MS" -le "$_end" ] || _end=$OPERATION_DEADLINE_MS
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
                        run_child sips-strip "$OPERATION_MS" "$EMPTY" "$ATTEMPT_DIR/sips.out" "$ATTEMPT_DIR/sips.err" sips --deleteColorManagementProperties "$ORIGINAL_PATH" --out "$_clean" || fail SIPS_STRIP
                        run_child sips-convert "$OPERATION_MS" "$EMPTY" "$ATTEMPT_DIR/convert.out" "$ATTEMPT_DIR/convert.err" sips -s format bmp "$_clean" --out "$OBSERVATION_PATH" || fail SIPS_CONVERT ;;
                    *) fail PUBLICATION_FORMAT ;;
                esac
                [ -s "$OBSERVATION_PATH" ] || fail PUBLICATION_PARTIAL; ORIGINAL_HASH=$_hash; return 0
            fi
        fi
        _now=$(clock_ms) || fail MONOTONIC_CLOCK_FAILURE; [ "$_now" -lt "$_end" ] || { [ "$_count" -eq 0 ] && fail SCREENSHOT_MISSING; fail PUBLICATION_UNSTABLE; }
        sleep 0.05
    done
}
observe_image() {
    run_child marker-observe "$OPERATION_MS" "$EMPTY" "$ATTEMPT_DIR/observe.out" "$ATTEMPT_DIR/observe.err" \
        "$FRAME_INDEX" --marker-observe "$CONTENT" "$CALIBRATION" "$OBSERVATION_PATH" || fail MARKER_OBSERVE
    _line=$(cat "$ATTEMPT_DIR/observe.out"); OBSERVED_FRAME=$(printf '%s\n' "$_line" | sed -n 's/^marker frame=\([0-9][0-9]*\) guest=[0-9][0-9]*$/\1/p')
    [ -n "$OBSERVED_FRAME" ] || fail MARKER_OBSERVE_PARSE; [ "$OBSERVED_FRAME" -ge 4 ] && [ "$OBSERVED_FRAME" -le 10000 ] || fail MARKER_FRAME_RANGE
}
capture() {
    _owns_operation=0
    if [ -z "$ACTIVE_OPERATION" ]; then begin_operation "capture-$1" "$OPERATION_MS"; _owns_operation=1; fi
    _tag=$1; CURRENT_STEP=$_tag; CURRENT_CLASS=SCREENSHOT; CURRENT_EXPECTED=one-complete-image; CURRENT_OBSERVED=awaiting-publication
    for _f in "$ATTEMPT_DIR/shots"/*; do [ ! -e "$_f" ] || fail STALE_PUBLICATION; done
    send_once SCREENSHOT SCREENSHOT; publish_image "$_tag"; observe_image
    BOUNDARY_FRAME=$OBSERVED_FRAME; BOUNDARY_PATH=$OBSERVATION_PATH; BOUNDARY_NAME=$ORIGINAL_NAME; BOUNDARY_HASH=$ORIGINAL_HASH
    if [ "$_owns_operation" -eq 1 ]; then
        finish_operation; BOUNDARY_ELAPSED_MS=$OPERATION_ELAPSED_MS
    else
        BOUNDARY_ELAPSED_MS=
    fi
    BOUNDARY_DEADLINE_MS=$OPERATION_MS
    CURRENT_OBSERVED="frame-$BOUNDARY_FRAME"; CURRENT_ARTIFACT="$BOUNDARY_NAME:$BOUNDARY_HASH"; LAST_REASON=OK; record_row
}
write_prefix() {
    _rec="$ATTEMPT_DIR/progression-prefix.rec"; _anchor_hash=$(fnv64 "$ANCHOR_PATH") || fail ANCHOR_HASH
    _content_id=$(awk -F= '$1=="content_identity"{print $2;n++}END{if(n!=1)exit 1}' "$ATTEMPT_DIR/oracle-fixture/progression-prefix.rec") || fail FIXTURE_IDENTITY
    _cal_id=$(awk -F= '$1=="calibration_identity"{print $2;n++}END{if(n!=1)exit 1}' "$ATTEMPT_DIR/oracle-fixture/progression-prefix.rec") || fail FIXTURE_IDENTITY
    {
        printf 'NESTURBATOR-PROGRESSION 1\nmode=prefix\ncontent_identity=%s\ncalibration_identity=%s\nterminal_exit=running\n' "$_content_id" "$_cal_id"
        printf 'original_anchor_path_length=%s\noriginal_anchor_path=%s\noriginal_anchor_identity=%s\noriginal_anchor_frame=%s\n' "${#ANCHOR_PATH}" "$ANCHOR_PATH" "$_anchor_hash" "$ANCHOR_FRAME"
        printf 'rewound_anchor_path_length=0\nrewound_anchor_path=\nrewound_anchor_identity=0000000000000000\nrewound_anchor_frame=0\nboundary_count=3\n'
        _i=1
        for _img in "$OBSERVATION_1" "$OBSERVATION_2" "$OBSERVATION_3"; do
            if [ "$_i" -le 2 ]; then _op=capture; _rel=pause-equal-original; _frame=$ANCHOR_FRAME; _delta=0; else _op=advance; _rel=original-plus; _frame=$FRAME_3; _delta=1; fi
            _hash=$(fnv64 "$_img") || fail IMAGE_HASH; _id=$((START_MS+_i))
            printf 'boundary\nid=%s\noperation=%s\nrelation=%s\norigin=original\ncommand_count=1\nsend_count=1\nartifact_identity=%s\n' "$_i" "$_op" "$_rel" "$_id"
            printf 'artifact_path_length=%s\nartifact_path=%s\nartifact_hash=%s\ndisplay_frame=%s\ncandidate_count=1\ncandidate_first=%s\ncandidate_last=%s\n' "${#_img}" "$_img" "$_hash" "$_frame" "$_frame" "$_frame"
            case "$_i" in 1) _elapsed=$ELAPSED_1 ;; 2) _elapsed=$ELAPSED_2 ;; 3) _elapsed=$ELAPSED_3 ;; esac
            printf 'anchor_identity=%s\nanchor_frame=%s\ndisplay_lead=0\nexpected_delta=%s\nelapsed_ms=%s\ndeadline_ms=%s\nprocess_alive=1\nboundary_end\n' "$_anchor_hash" "$ANCHOR_FRAME" "$_delta" "$_elapsed" "$OPERATION_MS"
            _i=$((_i+1))
        done
        printf 'END\n'
    } >"$_rec" || fail RECORD_WRITE
    run_child verdict "$OPERATION_MS" "$EMPTY" "$ATTEMPT_DIR/verdict.out" "$ATTEMPT_DIR/verdict.err" \
        "$FRAME_INDEX" --marker-verdict-prefix "$CONTENT" "$CALIBRATION" "$_rec" || fail PROGRESSION_VERDICT
    grep -Fx 'progression boundaries=3 mode=prefix verdict=PREFIX_OK' "$ATTEMPT_DIR/verdict.out" >/dev/null || fail PROGRESSION_PREFIX
    cat "$ATTEMPT_DIR/verdict.out"; LAST_REASON=PREFIX_OK; CURRENT_STEP=prefix-verdict; CURRENT_CLASS=oracle
    CURRENT_EXPECTED=PREFIX_OK; CURRENT_OBSERVED=PREFIX_OK; CURRENT_ARTIFACT=progression-prefix.rec; record_row
}

mkdir -p "$ATTEMPT_DIR/oracle-fixture"
run_child fixture "$OPERATION_MS" "$EMPTY" "$ATTEMPT_DIR/fixture.out" "$ATTEMPT_DIR/fixture.err" \
    "$FRAME_INDEX" --progression-selftest "$ATTEMPT_DIR/oracle-fixture" || fail ORACLE_FIXTURE
cmp -s "$CONTENT" "$ATTEMPT_DIR/oracle-fixture/marker.nes" || fail CONTENT_FIXTURE_MISMATCH
cmp -s "$CALIBRATION" "$ATTEMPT_DIR/oracle-fixture/calibration.txt" || fail CALIBRATION_FIXTURE_MISMATCH
awk -v old="$RA_DIR" -v new="$ATTEMPT_DIR" '{line=$0;while((at=index(line,old))>0)line=substr(line,1,at-1)new substr(line,at+length(old));print line}' "$CFG" >"$ATTEMPT_DIR/session.cfg" || fail CONFIG_COPY
env "HOME=$ATTEMPT_DIR/profile" "CFFIXED_USER_HOME=$ATTEMPT_DIR/profile" "XDG_CONFIG_HOME=$ATTEMPT_DIR/profile/.config" \
    "XDG_DATA_HOME=$ATTEMPT_DIR/profile/.local/share" "XDG_CACHE_HOME=$ATTEMPT_DIR/profile/.cache" \
    "$RA" --verbose -c "$ATTEMPT_DIR/session.cfg" -L "$CORE" "$CONTENT" >"$ATTEMPT_DIR/retroarch.log" 2>&1 &
HOST_PID=$!
probe_ready
sleep "${STATE_SESSION_SETTLE_SECONDS:-3}"; [ "$(remaining_ms)" -gt "$CLEANUP_MS" ] || fail ATTEMPT_DEADLINE; host_alive || fail HOST_DIED
begin_operation pause-toggle "$OPERATION_MS"; send_once PAUSE_TOGGLE pause-toggle; sleep 0.1; finish_operation
capture boundary-01; OBSERVATION_1=$BOUNDARY_PATH; FRAME_1=$BOUNDARY_FRAME; ELAPSED_1=$BOUNDARY_ELAPSED_MS
sleep 1.1
capture boundary-02; OBSERVATION_2=$BOUNDARY_PATH; FRAME_2=$BOUNDARY_FRAME; ELAPSED_2=$BOUNDARY_ELAPSED_MS
[ "$FRAME_2" -eq "$FRAME_1" ] || fail PAUSE_DID_NOT_HOLD

_state_file=; _state_count=0
for _f in "$ATTEMPT_DIR/states"/*/"$STEM".state; do [ -s "$_f" ] || continue; _state_file=$_f; _state_count=$((_state_count+1)); done
 [ "$_state_count" -eq 0 ] || fail STATE_STALE
begin_operation save-origin "$OPERATION_MS"; send_once SAVE_STATE save-original
_deadline=$(( $(clock_ms)+PUBLICATION_MS )); [ "$_deadline" -le "$OPERATION_DEADLINE_MS" ] || _deadline=$OPERATION_DEADLINE_MS
while [ "$_state_count" -eq 0 ]; do
    host_alive || fail HOST_DIED; _state_count=0
    for _f in "$ATTEMPT_DIR/states"/*/"$STEM".state; do [ -s "$_f" ] || continue; _state_file=$_f; _state_count=$((_state_count+1)); done
    [ "$_state_count" -le 1 ] || fail STATE_AMBIGUOUS; [ "$_state_count" -eq 1 ] && break
    [ "$(clock_ms)" -lt "$_deadline" ] || fail STATE_MISSING; [ "$(operation_left)" -gt 0 ] || fail save-origin_DEADLINE; sleep 0.05
done
[ "$_state_count" -eq 1 ] || fail STATE_AMBIGUOUS
ANCHOR_PATH="$ATTEMPT_DIR/original-anchor.state"; cp "$_state_file" "$ANCHOR_PATH" || fail ANCHOR_COPY; ANCHOR_FRAME=$FRAME_1
run_child anchor "$OPERATION_MS" "$EMPTY" "$ATTEMPT_DIR/anchor.out" "$ATTEMPT_DIR/anchor.err" \
    "$FRAME_INDEX" --marker-anchor "$CONTENT" "$CALIBRATION" "$ANCHOR_PATH" "$ANCHOR_FRAME" || fail ANCHOR_ORIGIN
finish_operation
begin_operation advance "$OPERATION_MS"; send_once FRAMEADVANCE frame-advance; sleep 0.1
capture boundary-03; OBSERVATION_3=$BOUNDARY_PATH; FRAME_3=$BOUNDARY_FRAME
[ "$FRAME_3" -eq $((ANCHOR_FRAME+1)) ] || fail ADVANCE_NOT_EXACTLY_ONE
finish_operation; ELAPSED_3=$OPERATION_ELAPSED_MS
write_prefix
begin_operation quit "$OPERATION_MS"; send_once QUIT quit; _deadline=$OPERATION_DEADLINE_MS
while kill -0 "$HOST_PID" 2>/dev/null; do [ "$(clock_ms)" -lt "$_deadline" ] || fail QUIT_DEADLINE; sleep 0.05; done
_status=0; wait "$HOST_PID" || _status=$?; HOST_PID=; [ "$_status" -eq 0 ] || fail HOST_EXIT
finish_operation
LAST_REASON=OK; CURRENT_STEP=complete; CURRENT_CLASS=cleanup; CURRENT_EXPECTED=host-exited-zero; CURRENT_OBSERVED=host-exited-zero; CURRENT_ARTIFACT=journal.tsv; record_row
printf 'state_session: attempt=%s verdict=PREFIX_OK sends=%s journal=%s\n' "$ATTEMPT_ID" "$send_count" "$JOURNAL"
