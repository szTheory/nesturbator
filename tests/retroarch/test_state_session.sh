#!/bin/sh
# Deterministic controls for the production RetroArch session driver. The fake host and
# transport are this same script reached through private test-only symlinks.
set -eu

report_metric() {
    case "$1" in
        ''|*[!0-9]*) printf 'unavailable' ;;
        *) printf '%s' "$1" ;;
    esac
}

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

report_readiness_timing() {
    report_case=$1; report_fixture=$2; report_setup=$3; report_operation=$4
    report_later_prefix=$5; report_attempt=$6; report_driver=$7
    report_status=$8; report_reason=$9; report_lower=${10}; report_upper=${11}
    case "$report_case" in readiness-delayed|readiness-response-stall) ;; *) report_case=unknown ;; esac
    report_fixture=$(report_metric "$report_fixture")
    report_setup=$(report_metric "$report_setup")
    report_operation=$(report_metric "$report_operation")
    report_later_prefix=$(report_metric "$report_later_prefix")
    report_attempt=$(report_metric "$report_attempt")
    report_driver=$(report_metric "$report_driver")
    report_status=$(report_metric "$report_status")
    report_lower=$(report_metric "$report_lower")
    report_upper=$(report_metric "$report_upper")
    case "$report_reason" in
        unavailable) ;;
        ''|*[!A-Z0-9_]*) report_reason=unavailable ;;
    esac
    printf 'state session readiness case=%s: fixture=%sms setup=%sms operation=%sms later-prefix=%sms attempt-through-cleanup=%sms driver+cleanup=%sms driver-status=%s driver-reason=%s; bounds operation=[%s,%s) driver<25000ms\n' \
        "$report_case" "$report_fixture" "$report_setup" "$report_operation" \
        "$report_later_prefix" "$report_attempt" "$report_driver" "$report_status" \
        "$report_reason" "$report_lower" "$report_upper"
}

fake_nc() {
    request=$(cat)
    printf '%s\n' "$request" >>"$STATE_SESSION_TEST_SENDS"
    case "$request" in
        'GET_CONFIG_PARAM savestate_directory')
            [ -z "${STATE_SESSION_TEST_READINESS_SENDS:-}" ] ||
                printf '%s\n' "$request" >>"$STATE_SESSION_TEST_READINESS_SENDS"
            ;;
    esac
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
                exec perl -MTime::HiRes=sleep -e 'sleep 20'
            fi
            if [ "$fault" = readiness-response-stall ]; then
                printf '%s\n' "$$" >"$STATE_SESSION_TEST_STALL_PID"
                printf 'GET_CONFIG_PARAM savestate_directory %s' "$STATE_SESSION_TEST_EXPECTED_DIR"
                exec perl -MTime::HiRes=sleep -e 'sleep 8'
            fi
            if [ "$fault" = readiness-response-delay ]; then
                printf '%s\n' "$$" >"$STATE_SESSION_TEST_STALL_PID"
                perl -MTime::HiRes=sleep -e 'sleep 1.5' || exit $?
                printf 'GET_CONFIG_PARAM savestate_directory %s\n' "$STATE_SESSION_TEST_EXPECTED_DIR"
                exit 0
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

recording_action_nc() {
    request=$(cat)
    printf '%s\n' "$request" >>"$STATE_SESSION_TEST_ACTION_SENDS"
    case "$request" in
        'GET_CONFIG_PARAM savestate_directory')
            echo "action wrapper received read-only readiness request" >&2
            exit 97
            ;;
    esac
    printf '%s' "$request" | "$STATE_SESSION_TEST_FAKE_NC" "$@"
}

hosted_wrapper_nc() {
    request=$(cat)
    if [ -n "${HOSTED_WRAPPER_TEST_TIMES:-}" ]; then
        printf '%s\n' "$(perl -MTime::HiRes=clock_gettime,CLOCK_MONOTONIC -e 'printf "%.0f\n",clock_gettime(CLOCK_MONOTONIC)*1000')" \
            >>"$HOSTED_WRAPPER_TEST_TIMES"
    fi
    if [ "${HOSTED_WRAPPER_TEST_DELAY:-0}" = 1 ] && [ "$request" = FRAMEADVANCE ]; then
        printf '%s\n' "$$" >"$HOSTED_WRAPPER_TEST_CHILD_PID"
        exec perl -MTime::HiRes=sleep -e '
            $SIG{TERM} = "IGNORE";
            sleep(2);
            open my $fh, ">>", $ENV{HOSTED_WRAPPER_TEST_LATE_SEND} or die $!;
            print {$fh} "FRAMEADVANCE\n";
            close $fh;
        '
    fi
    if [ "${HOSTED_WRAPPER_TEST_FAIL_FIRST:-0}" = 1 ]; then
        count=0
        [ ! -f "$HOSTED_WRAPPER_TEST_SENDS" ] || count=$(wc -l <"$HOSTED_WRAPPER_TEST_SENDS" | tr -d ' ')
        count=$((count+1))
        printf '%s\n' "$request" >>"$HOSTED_WRAPPER_TEST_SENDS"
        [ "$count" -ne 1 ] || exit 93
    else
        printf '%s\n' "$request" >>"$HOSTED_WRAPPER_TEST_SENDS"
    fi
}

case "${0##*/}" in
    fake-nc) fake_nc; exit $? ;;
    recording-action-nc) recording_action_nc; exit $? ;;
    fake-retroarch) fake_host; exit $? ;;
    hosted-wrapper-nc) hosted_wrapper_nc; exit $? ;;
esac

if [ "$#" -ne 5 ]; then
    echo "usage: test_state_session.sh DRIVER FRAME_INDEX WORK_ROOT CMAKE_COMMAND RUN_RETROARCH_CMAKE" >&2
    exit 2
fi
DRIVER=$1
FRAME_INDEX=$2
WORK_ROOT=$3
CMAKE_COMMAND=$4
RUN_RETROARCH_CMAKE=$5
TEST_SCRIPT_DIR=$(CDPATH= cd "$(dirname "$DRIVER")" && pwd -P)
. "$TEST_SCRIPT_DIR/interruption_trace.sh"
CONTROL=${STATE_SESSION_CONTROL:-all}
case "$CONTROL" in
    all|readiness|interruption|interruption-capture-selftest) ;;
    *) echo "state session controls: invalid STATE_SESSION_CONTROL=$CONTROL" >&2; exit 2 ;;
esac

selftest_counted_grep() {
    printf 'call\n' >>"$INTERRUPTION_TEST_CALLS"
    grep -F "$1" "$2"
}

interruption_grep() {
    grep -F "$1" "$2"
}

selftest_controlled_error() {
    printf 'call\n' >>"$INTERRUPTION_TEST_CALLS"
    cat "$INTERRUPTION_TEST_STDOUT"
    cat "$INTERRUPTION_TEST_STDERR" >&2
    return 2
}

selftest_count_calls() {
    [ ! -f "$1" ] || wc -l <"$1" | tr -d ' '
    [ -f "$1" ] || printf '0'
}

selftest_independent_identity() {
    perl -e 'use strict; use warnings; my @st = stat($ARGV[0]) or die "stat: $!"; print join("\t", @st[0,1,7]), "\n"' "$1"
}

selftest_assert_events() {
    selftest_event_file=$1
    selftest_event_id=$2
    awk -F '\t' -v wanted="$selftest_event_id" '
        $1 != wanted || $2 != "harness" || $3 != NR || $4 !~ /^[0-9]+$/ || $7 !~ /^[0-9]+$/ { bad = 1 }
        NR > 1 && $4 < previous { bad = 1 }
        { previous = $4 }
        END { if (NR < 4 || bad) exit 1 }
    ' "$selftest_event_file"
}

run_interruption_trace_selftests() (
    trace_test_root=$1/trace-lifecycle
    trace_test_id="trace-$(mono_ms)-$$"
    RA_DIR="$trace_test_root/owned-a/ra"
    LOCK_DIR="$trace_test_root/owned-a/locks/driver.lock"
    trace_test_lock_root=${LOCK_DIR%/*}
    mkdir -p "$RA_DIR" "$trace_test_lock_root" || {
        echo "FAIL trace lifecycle setup" >&2; return 1;
    }
    mkdir "$LOCK_DIR" || { echo "FAIL trace lifecycle lock setup" >&2; return 1; }

    STATE_SESSION_TEST_MODE=prefix
    STATE_SESSION_TEST_INTERRUPT_INVOCATION=$trace_test_id
    STATE_SESSION_TEST_INTERRUPT_TRACE="$RA_DIR/interruption-events.tsv"
    STATE_SESSION_TEST_INTERRUPT_TRACE_ERRORS="$RA_DIR/interruption-trace-errors.tsv"
    HOST_PID=$$
    CHILD_PID=$$
    FIRST_FAILURE=
    QUIT_SENT=0
    JOURNAL_FAILED=0
    send_count=0
    INTERRUPTION_TRACE_SEQUENCE=0
    INTERRUPTION_TRACE_FAILED=0
    [ ! -e "$STATE_SESSION_TEST_INTERRUPT_TRACE" ] || {
        echo "FAIL trace cleanup-host-pid admission: source trace unexpectedly existed" >&2; return 1;
    }

    trace_interruption_event handler-entry 0 || {
        echo "FAIL trace cleanup-host-pid admission: actual writer rejected handler-entry" >&2; return 1;
    }
    FIRST_FAILURE=INTERRUPTED_TERM
    trace_interruption_event journal-close-success 0 || {
        echo "FAIL trace cleanup-host-pid admission: actual writer rejected journal-close-success" >&2; return 1;
    }
    HOST_PID=
    CHILD_PID=
    QUIT_SENT=1
    rmdir "$LOCK_DIR" || { echo "FAIL trace cleanup-host-pid admission: cleanup lock removal" >&2; return 1; }
    trace_interruption_event handler-exit 130 || {
        echo "FAIL trace cleanup-host-pid admission: actual writer rejected handler-exit" >&2; return 1;
    }

    awk -F '\t' -v wanted="$trace_test_id" '
        NR == 1 && ($1 != wanted || $2 != "driver" || $3 != "1" || $5 != "handler-entry" || $8 !~ /^[1-9][0-9]*$/) { bad = 1 }
        NR == 2 && ($1 != wanted || $2 != "driver" || $3 != "2" || $5 != "journal-close-success" || $8 !~ /^[1-9][0-9]*$/) { bad = 1 }
        NR == 3 && ($1 != wanted || $2 != "driver" || $3 != "3" || $5 != "handler-exit" || $8 != "unavailable" || $10 != "unavailable" || $14 != "absent") { bad = 1 }
        NF != 16 { bad = 1 }
        END { if (NR != 3 || bad) exit 1 }
    ' "$STATE_SESSION_TEST_INTERRUPT_TRACE" || {
        echo "FAIL trace cleanup-host-pid admission: actual writer did not emit retained cleanup shape" >&2; return 1;
    }
    trace_test_status=0
    trace_test_result=$(verify_interruption_driver_trace \
        "$STATE_SESSION_TEST_INTERRUPT_TRACE" "$trace_test_id" "$$") || trace_test_status=$?
    if [ "$trace_test_status" -ne 0 ]; then
        echo "FAIL trace cleanup-host-pid admission: actual writer emitted field 8 unavailable after cleanup, but the validator rejected the valid lifecycle" >&2
        return 1
    fi
    [ "$trace_test_result" = TRACE_OK ] || {
        echo "FAIL trace cleanup-host-pid admission: expected TRACE_OK, got $trace_test_result" >&2; return 1;
    }
    printf 'trace selftest: PASS cleanup-host-pid admission\n'
    printf 'state session interruption trace selftest: 1 case passed\n'
)

run_interruption_capture_selftests() {
    selftest_root=$1
    INTERRUPTION_CAPTURE_SELFTEST_ACTIVE=1
    selftest_run_id=$(mono_ms)-$$
    selftest_root="$selftest_root/selftest-$selftest_run_id"
    mkdir -p "$selftest_root"

    selftest_match="$selftest_root/match"
    mkdir -p "$selftest_match"
    printf 'step\tclass\treason\nINTERRUPTED_TERM\tsignal\tTERM\nCOMPLETE\tfinish\tOK\n' >"$selftest_match/journal.tsv"
    printf 'INTERRUPTED_TERM\tsignal\tTERM\n' >"$selftest_match/expected.stdout"
    : >"$selftest_match/expected.stderr"
    INTERRUPTION_TEST_CALLS="$selftest_match/calls"
    export INTERRUPTION_TEST_CALLS
    selftest_status=0
    capture_interruption_assertion selftest_counted_grep INTERRUPTED_TERM \
        "$selftest_match/journal.tsv" "$selftest_match" /dev/null || selftest_status=$?
    [ "$selftest_status" -eq 0 ] && [ "$INTERRUPTION_ASSERTION_STATUS" -eq 0 ] &&
        [ "$INTERRUPTION_RECORDER_STATUS" -eq 0 ] &&
        [ "$(selftest_count_calls "$selftest_match/calls")" -eq 1 ] &&
        cmp -s "$selftest_match/expected.stdout" "$selftest_match/assertion.stdout" &&
        cmp -s "$selftest_match/expected.stderr" "$selftest_match/assertion.stderr" || {
        echo "FAIL interruption capture status-0 bytes or one-call contract" >&2; return 1;
    }
    selftest_assert_events "$selftest_match/events.tsv" match || {
        echo "FAIL interruption capture match events are not invocation-owned and ordered" >&2; return 1;
    }
    selftest_decision_status=0
    interruption_assertion_decision 2>"$selftest_match/decision.stderr" || selftest_decision_status=$?
    [ "$selftest_decision_status" -eq 0 ] && [ ! -s "$selftest_match/decision.stderr" ] || {
        echo "FAIL interruption status-0 original decision changed" >&2; return 1;
    }

    selftest_miss="$selftest_root/no-match"
    mkdir -p "$selftest_miss"
    printf 'step\tclass\treason\nCOMPLETE\tfinish\tOK\n' >"$selftest_miss/journal.tsv"
    : >"$selftest_miss/expected.stdout"
    : >"$selftest_miss/expected.stderr"
    INTERRUPTION_TEST_CALLS="$selftest_miss/calls"
    export INTERRUPTION_TEST_CALLS
    selftest_status=0
    capture_interruption_assertion selftest_counted_grep INTERRUPTED_TERM \
        "$selftest_miss/journal.tsv" "$selftest_miss" /dev/null || selftest_status=$?
    [ "$selftest_status" -eq 1 ] && [ "$INTERRUPTION_ASSERTION_STATUS" -eq 1 ] &&
        [ "$INTERRUPTION_RECORDER_STATUS" -eq 0 ] &&
        [ "$(selftest_count_calls "$selftest_miss/calls")" -eq 1 ] &&
        cmp -s "$selftest_miss/expected.stdout" "$selftest_miss/assertion.stdout" &&
        cmp -s "$selftest_miss/expected.stderr" "$selftest_miss/assertion.stderr" || {
        echo "FAIL interruption capture status-1 bytes or one-call contract" >&2; return 1;
    }
    selftest_assert_events "$selftest_miss/events.tsv" no-match || {
        echo "FAIL interruption capture no-match events are not invocation-owned and ordered" >&2; return 1;
    }
    selftest_decision_status=0
    interruption_assertion_decision 2>"$selftest_miss/decision.stderr" || selftest_decision_status=$?
    printf 'FAIL interruption first failure was not retained\n' >"$selftest_miss/expected-decision.stderr"
    [ "$selftest_decision_status" -eq 1 ] &&
        cmp -s "$selftest_miss/expected-decision.stderr" "$selftest_miss/decision.stderr" || {
        echo "FAIL interruption status-1 failure decision or message changed" >&2; return 1;
    }

    selftest_error="$selftest_root/execution-error"
    mkdir -p "$selftest_error"
    printf 'controlled stdout\000tail\r\n' >"$selftest_error/expected.stdout"
    printf 'controlled stderr\tstatus-2\000tail\r\n' >"$selftest_error/expected.stderr"
    printf 'INTERRUPTED_TERM\tsignal\tTERM\n' >"$selftest_error/journal.tsv"
    INTERRUPTION_TEST_CALLS="$selftest_error/calls"
    INTERRUPTION_TEST_STDOUT="$selftest_error/expected.stdout"
    INTERRUPTION_TEST_STDERR="$selftest_error/expected.stderr"
    export INTERRUPTION_TEST_CALLS INTERRUPTION_TEST_STDOUT INTERRUPTION_TEST_STDERR
    selftest_status=0
    capture_interruption_assertion selftest_controlled_error INTERRUPTED_TERM \
        "$selftest_error/journal.tsv" "$selftest_error" /dev/null \
        2>"$selftest_error/original.stderr" || selftest_status=$?
    [ "$selftest_status" -eq 2 ] && [ "$INTERRUPTION_ASSERTION_STATUS" -eq 2 ] &&
        [ "$INTERRUPTION_RECORDER_STATUS" -eq 0 ] &&
        [ "$(selftest_count_calls "$selftest_error/calls")" -eq 1 ] &&
        cmp -s "$selftest_error/expected.stdout" "$selftest_error/assertion.stdout" &&
        cmp -s "$selftest_error/expected.stderr" "$selftest_error/assertion.stderr" &&
        cmp -s "$selftest_error/expected.stderr" "$selftest_error/original.stderr" || {
        echo "FAIL interruption capture status-2 exact byte or one-call contract" >&2; return 1;
    }
    selftest_decision_status=0
    interruption_assertion_decision 2>"$selftest_error/decision.stderr" || selftest_decision_status=$?
    [ "$selftest_decision_status" -eq 1 ] &&
        cmp -s "$selftest_miss/expected-decision.stderr" "$selftest_error/decision.stderr" || {
        echo "FAIL interruption status-2 failure decision or message changed" >&2; return 1;
    }

    for selftest_case in recorder-match recorder-no-match; do
        selftest_failure="$selftest_root/$selftest_case"
        mkdir -p "$selftest_failure"
        printf 'INTERRUPTED_TERM\tsignal\tTERM\n' >"$selftest_failure/match.tsv"
        printf 'COMPLETE\tfinish\tOK\n' >"$selftest_failure/no-match.tsv"
        INTERRUPTION_TEST_CALLS="$selftest_failure/calls"
        export INTERRUPTION_TEST_CALLS
        if [ "$selftest_case" = recorder-match ]; then
            selftest_journal="$selftest_failure/match.tsv"
            selftest_expected_status=0
        else
            selftest_journal="$selftest_failure/no-match.tsv"
            selftest_expected_status=1
        fi
        selftest_unwritable="$selftest_failure/missing-parent/invocation"
        selftest_status=0
        capture_interruption_assertion selftest_counted_grep INTERRUPTED_TERM \
            "$selftest_journal" "$selftest_unwritable" /dev/null || selftest_status=$?
        [ "$selftest_status" -eq "$selftest_expected_status" ] &&
            [ "$INTERRUPTION_ASSERTION_STATUS" -eq "$selftest_expected_status" ] &&
            [ "$INTERRUPTION_RECORDER_STATUS" -ne 0 ] &&
            [ -n "$INTERRUPTION_RECORDER_REASON" ] &&
            [ "$(selftest_count_calls "$selftest_failure/calls")" -eq 1 ] || {
            echo "FAIL recorder failure changed original assertion status for $selftest_case" >&2; return 1;
        }
        selftest_decision_status=0
        interruption_assertion_decision 2>"$selftest_failure/decision.stderr" ||
            selftest_decision_status=$?
        selftest_gate_status=0
        interruption_recorder_gate 2>"$selftest_failure/recorder-gate.stderr" ||
            selftest_gate_status=$?
        [ "$selftest_decision_status" -eq "$selftest_expected_status" ] &&
            [ "$selftest_gate_status" -eq 1 ] &&
            grep -F 'FAIL interruption recorder unusable:' "$selftest_failure/recorder-gate.stderr" >/dev/null || {
            echo "FAIL recorder gate replaced the original assertion decision for $selftest_case" >&2; return 1;
        }
        if [ "$selftest_expected_status" -eq 0 ]; then
            [ ! -s "$selftest_failure/decision.stderr" ] || {
                echo "FAIL recorder failure turned a matching assertion into failure" >&2; return 1;
            }
        else
            printf 'FAIL interruption first failure was not retained\n' >"$selftest_failure/expected-decision.stderr"
            cmp -s "$selftest_failure/expected-decision.stderr" "$selftest_failure/decision.stderr" || {
                echo "FAIL recorder failure changed the no-match failure message" >&2; return 1;
            }
        fi
    done

    selftest_replace="$selftest_root/replacement"
    mkdir -p "$selftest_replace"
    printf 'old-handle\tINTERRUPTED_TERM\n' >"$selftest_replace/journal.tsv"
    printf 'new-path\tINTERRUPTED_TERM\n' >"$selftest_replace/replacement.tsv"
    cp "$selftest_replace/journal.tsv" "$selftest_replace/expected-pre.snapshot"
    cp "$selftest_replace/replacement.tsv" "$selftest_replace/expected-post.snapshot"
    selftest_pre_identity=$(selftest_independent_identity "$selftest_replace/journal.tsv") || return 1
    INTERRUPTION_TEST_CALLS="$selftest_replace/calls"
    export INTERRUPTION_TEST_CALLS
    selftest_status=0
    capture_interruption_assertion selftest_counted_grep INTERRUPTED_TERM \
        "$selftest_replace/journal.tsv" "$selftest_replace" /dev/null \
        "$selftest_replace/replacement.tsv" || selftest_status=$?
    selftest_post_identity=$(selftest_independent_identity "$selftest_replace/journal.tsv") || return 1
    selftest_pre_record=$(awk -F '\t' '$1 == "pre" { print $2 "\t" $3 "\t" $4 "\t" $5 "\t" $6 "\t" $7 "\t" $8 }' \
        "$selftest_replace/snapshot-metadata.tsv")
    selftest_post_record=$(awk -F '\t' '$1 == "post" { print $2 "\t" $3 "\t" $4 "\t" $5 "\t" $6 "\t" $7 "\t" $8 }' \
        "$selftest_replace/snapshot-metadata.tsv")
    selftest_pre_handle=$(printf '%s\n' "$selftest_pre_record" | cut -f1-3)
    selftest_pre_after=$(printf '%s\n' "$selftest_pre_record" | cut -f4-6)
    selftest_post_handle=$(printf '%s\n' "$selftest_post_record" | cut -f1-3)
    selftest_post_after=$(printf '%s\n' "$selftest_post_record" | cut -f4-6)
    selftest_pre_bytes=$(printf '%s\n' "$selftest_pre_record" | cut -f7)
    selftest_post_bytes=$(printf '%s\n' "$selftest_post_record" | cut -f7)
    selftest_expected_pre_size=$(wc -c <"$selftest_replace/expected-pre.snapshot" | tr -d ' ')
    selftest_expected_post_size=$(wc -c <"$selftest_replace/expected-post.snapshot" | tr -d ' ')
    [ "$selftest_status" -eq 0 ] && [ "$INTERRUPTION_ASSERTION_STATUS" -eq 0 ] &&
        [ "$INTERRUPTION_RECORDER_STATUS" -eq 0 ] &&
        [ "$(selftest_count_calls "$selftest_replace/calls")" -eq 1 ] &&
        cmp -s "$selftest_replace/expected-pre.snapshot" "$selftest_replace/snapshot-pre.bin" &&
        cmp -s "$selftest_replace/expected-post.snapshot" "$selftest_replace/snapshot-post.bin" &&
        [ "$selftest_pre_handle" = "$selftest_pre_after" ] &&
        [ "$selftest_post_handle" = "$selftest_post_after" ] &&
        [ "$selftest_pre_handle" = "$selftest_pre_identity" ] &&
        [ "$selftest_post_handle" = "$selftest_post_identity" ] &&
        [ "$(printf '%s\n' "$selftest_pre_handle" | cut -f3)" -eq "$selftest_pre_bytes" ] &&
        [ "$(printf '%s\n' "$selftest_post_handle" | cut -f3)" -eq "$selftest_post_bytes" ] &&
        [ "$selftest_pre_bytes" -eq "$selftest_expected_pre_size" ] &&
        [ "$selftest_post_bytes" -eq "$selftest_expected_post_size" ] &&
        [ "$selftest_pre_handle" != "$selftest_post_handle" ] || {
        echo "FAIL same-handle replacement snapshots lost independent bytes or identity" >&2; return 1;
    }
    selftest_assert_events "$selftest_replace/events.tsv" replacement || {
        echo "FAIL replacement events are not invocation-owned and ordered" >&2; return 1;
    }

    selftest_second="$selftest_root/second-invocation"
    mkdir -p "$selftest_second"
    printf 'INTERRUPTED_TERM\tsecond\n' >"$selftest_second/journal.tsv"
    INTERRUPTION_TEST_CALLS="$selftest_second/calls"
    export INTERRUPTION_TEST_CALLS
    selftest_status=0
    capture_interruption_assertion selftest_counted_grep INTERRUPTED_TERM \
        "$selftest_second/journal.tsv" "$selftest_second" /dev/null || selftest_status=$?
    [ "$selftest_status" -eq 0 ] &&
        selftest_assert_events "$selftest_second/events.tsv" second-invocation || {
        echo "FAIL second recorder invocation mixed event ownership" >&2; return 1;
    }

    INTERRUPTION_CAPTURE_SELFTEST_ACTIVE=0
    printf 'state session interruption capture selftest: 7 cases passed (status 0/1/2, recorder errors, same-handle replacement, event ownership)\n'
    run_interruption_trace_selftests "$selftest_root"
}

INTERRUPTION_ASSERTION_STATUS=0
INTERRUPTION_RECORDER_STATUS=0
INTERRUPTION_RECORDER_REASON=
INTERRUPTION_EVENT_SEQUENCE=0

mark_interruption_recorder_error() {
    INTERRUPTION_RECORDER_STATUS=1
    [ -n "$INTERRUPTION_RECORDER_REASON" ] || INTERRUPTION_RECORDER_REASON=$1
    if [ -n "${INTERRUPTION_RECORDER_DIR:-}" ] && [ -d "$INTERRUPTION_RECORDER_DIR" ]; then
        printf '%s\t%s\n' "${INTERRUPTION_RECORDER_ID:-unknown}" "$1" \
            >>"$INTERRUPTION_RECORDER_DIR/recorder-errors.tsv" 2>/dev/null || true
    fi
}

record_interruption_event() {
    INTERRUPTION_RECORDER_DIR=$1
    INTERRUPTION_RECORDER_ID=$2
    interruption_event_role=$3
    interruption_event_name=$4
    interruption_event_status=$5
    interruption_event_pid=$6
    interruption_event_detail=${7:-none}
    [ -d "$INTERRUPTION_RECORDER_DIR" ] || {
        mark_interruption_recorder_error "event-directory-missing:$interruption_event_name"
        return 1
    }
    interruption_event_time=$(mono_ms 2>/dev/null) || {
        mark_interruption_recorder_error "event-clock-unavailable:$interruption_event_name"
        return 1
    }
    case "$interruption_event_time" in *[!0-9]*|'')
        mark_interruption_recorder_error "event-clock-invalid:$interruption_event_name"
        return 1 ;;
    esac
    INTERRUPTION_EVENT_SEQUENCE=$((INTERRUPTION_EVENT_SEQUENCE + 1))
    printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
        "$INTERRUPTION_RECORDER_ID" "$interruption_event_role" \
        "$INTERRUPTION_EVENT_SEQUENCE" "$interruption_event_time" \
        "$interruption_event_name" "$interruption_event_status" \
        "$interruption_event_pid" "$interruption_event_detail" \
        >>"$INTERRUPTION_RECORDER_DIR/events.tsv" 2>/dev/null || {
        mark_interruption_recorder_error "event-write-failed:$interruption_event_name"
        return 1
    }
}

snapshot_interruption_file() {
    interruption_snapshot_label=$1
    interruption_snapshot_source=$2
    interruption_snapshot_bytes=$3
    interruption_snapshot_metadata=$4
    interruption_snapshot_replacement=${5:-}
    INTERRUPTION_SNAPSHOT_STATUS=0
    if [ ! -d "$INTERRUPTION_RECORDER_DIR" ]; then
        INTERRUPTION_SNAPSHOT_STATUS=1
        mark_interruption_recorder_error "snapshot-$interruption_snapshot_label-directory-missing"
        return 1
    fi
    if ! INTERRUPTION_SNAPSHOT_LABEL="$interruption_snapshot_label" \
        INTERRUPTION_SNAPSHOT_SOURCE="$interruption_snapshot_source" \
        INTERRUPTION_SNAPSHOT_BYTES="$interruption_snapshot_bytes" \
        INTERRUPTION_SNAPSHOT_METADATA="$interruption_snapshot_metadata" \
        INTERRUPTION_SNAPSHOT_REPLACEMENT="$interruption_snapshot_replacement" \
        perl -e '
            use strict;
            use warnings;
            my ($label, $source, $copy, $metadata, $replacement) =
                @ENV{qw(INTERRUPTION_SNAPSHOT_LABEL INTERRUPTION_SNAPSHOT_SOURCE INTERRUPTION_SNAPSHOT_BYTES INTERRUPTION_SNAPSHOT_METADATA INTERRUPTION_SNAPSHOT_REPLACEMENT)};
            open my $input, "<:raw", $source or die "open source failed: $!\n";
            my @before = stat($input);
            @before or die "stat open handle failed: $!\n";
            if (length($replacement)) {
                rename($replacement, $source) or die "replace pathname failed: $!\n";
            }
            open my $output, ">:raw", $copy or die "open snapshot output failed: $!\n";
            my $read_bytes = 0;
            while (1) {
                my $count = read($input, my $buffer, 65536);
                defined $count or die "read open handle failed: $!\n";
                last if $count == 0;
                print {$output} $buffer or die "write snapshot failed: $!\n";
                $read_bytes += $count;
            }
            my @after = stat($input);
            @after or die "restat open handle failed: $!\n";
            close $output or die "close snapshot output failed: $!\n";
            close $input or die "close source handle failed: $!\n";
            open my $record, ">>:raw", $metadata or die "open metadata failed: $!\n";
            print {$record} join("\t", $label, @before[0,1,7], @after[0,1,7], $read_bytes, "ok"), "\n"
                or die "write metadata failed: $!\n";
            close $record or die "close metadata failed: $!\n";
        ' 2>"$INTERRUPTION_RECORDER_DIR/snapshot-$interruption_snapshot_label.stderr"
    then
        INTERRUPTION_SNAPSHOT_STATUS=1
        mark_interruption_recorder_error "snapshot-$interruption_snapshot_label-failed"
    fi
    return "$INTERRUPTION_SNAPSHOT_STATUS"
}

capture_interruption_assertion() {
    interruption_assertion_command=$1
    interruption_assertion_pattern=$2
    interruption_assertion_journal=$3
    INTERRUPTION_RECORDER_DIR=$4
    interruption_assertion_stdout_destination=$5
    interruption_assertion_replace_from=${6:-}
    INTERRUPTION_RECORDER_ID=${INTERRUPTION_RECORDER_DIR##*/}
    INTERRUPTION_ASSERTION_STATUS=2
    if [ ! -e "$INTERRUPTION_RECORDER_DIR/events.tsv" ]; then
        INTERRUPTION_RECORDER_STATUS=0
        INTERRUPTION_RECORDER_REASON=
    fi
    INTERRUPTION_CAPTURE_OUT="$INTERRUPTION_RECORDER_DIR/assertion.stdout"
    INTERRUPTION_CAPTURE_ERR="$INTERRUPTION_RECORDER_DIR/assertion.stderr"
    INTERRUPTION_CAPTURE_META="$INTERRUPTION_RECORDER_DIR/snapshot-metadata.tsv"

    if [ "$interruption_assertion_command" != interruption_grep ] &&
        [ "${INTERRUPTION_CAPTURE_SELFTEST_ACTIVE:-0}" -ne 1 ]; then
        mark_interruption_recorder_error assertion-injection-outside-selftest
        interruption_assertion_command=interruption_grep
    fi
    if [ -n "$interruption_assertion_replace_from" ] &&
        [ "${INTERRUPTION_CAPTURE_SELFTEST_ACTIVE:-0}" -ne 1 ]; then
        mark_interruption_recorder_error pathname-replacement-outside-selftest
        interruption_assertion_replace_from=
    fi

    if [ -d "$INTERRUPTION_RECORDER_DIR" ]; then
        if [ ! -e "$INTERRUPTION_RECORDER_DIR/events.tsv" ]; then
            : >"$INTERRUPTION_RECORDER_DIR/events.tsv" ||
                mark_interruption_recorder_error events-open-failed
            INTERRUPTION_EVENT_SEQUENCE=0
        else
            INTERRUPTION_EVENT_SEQUENCE=$(awk 'END { print NR + 0 }' \
                "$INTERRUPTION_RECORDER_DIR/events.tsv") || {
                INTERRUPTION_EVENT_SEQUENCE=0
                mark_interruption_recorder_error events-read-failed
            }
        fi
        if [ ! -e "$INTERRUPTION_RECORDER_DIR/snapshot-metadata.tsv" ]; then
            printf 'label\tdevice\tinode\tsize\tafter_device\tafter_inode\tafter_size\tbytes_read\tstatus\n' \
                >"$INTERRUPTION_RECORDER_DIR/snapshot-metadata.tsv" ||
                mark_interruption_recorder_error snapshot-metadata-open-failed
        fi
    else
        INTERRUPTION_EVENT_SEQUENCE=0
        mark_interruption_recorder_error event-directory-missing
    fi
    if [ -e "$INTERRUPTION_CAPTURE_OUT" ] || [ -e "$INTERRUPTION_CAPTURE_ERR" ]; then
        mark_interruption_recorder_error assertion-output-already-exists
    fi
    record_interruption_event "$INTERRUPTION_RECORDER_DIR" "$INTERRUPTION_RECORDER_ID" \
        harness capture-start 0 "$$" || true

    record_interruption_event "$INTERRUPTION_RECORDER_DIR" "$INTERRUPTION_RECORDER_ID" \
        harness snapshot-pre-start 0 "$$" || true
    snapshot_interruption_file pre "$interruption_assertion_journal" \
        "$INTERRUPTION_RECORDER_DIR/snapshot-pre.bin" "$INTERRUPTION_CAPTURE_META" \
        "$interruption_assertion_replace_from" || true
    record_interruption_event "$INTERRUPTION_RECORDER_DIR" "$INTERRUPTION_RECORDER_ID" \
        harness snapshot-pre-end "$INTERRUPTION_SNAPSHOT_STATUS" "$$" || true
    record_interruption_event "$INTERRUPTION_RECORDER_DIR" "$INTERRUPTION_RECORDER_ID" \
        harness assertion-start 0 "$$" || true

    interruption_capture_ready=0
    if [ -d "$INTERRUPTION_RECORDER_DIR" ] &&
        [ ! -e "$INTERRUPTION_CAPTURE_OUT" ] && [ ! -e "$INTERRUPTION_CAPTURE_ERR" ] &&
        ( : >"$INTERRUPTION_CAPTURE_OUT" && : >"$INTERRUPTION_CAPTURE_ERR" ) 2>/dev/null; then
        interruption_capture_ready=1
    else
        mark_interruption_recorder_error assertion-output-open-failed
    fi

    if [ "$interruption_capture_ready" -eq 1 ]; then
        if "$interruption_assertion_command" "$interruption_assertion_pattern" \
            "$interruption_assertion_journal" >"$INTERRUPTION_CAPTURE_OUT" 2>"$INTERRUPTION_CAPTURE_ERR"; then
            INTERRUPTION_ASSERTION_STATUS=0
        else
            INTERRUPTION_ASSERTION_STATUS=$?
        fi
        cat "$INTERRUPTION_CAPTURE_OUT" >"$interruption_assertion_stdout_destination" ||
            mark_interruption_recorder_error assertion-stdout-reemit-failed
        cat "$INTERRUPTION_CAPTURE_ERR" >&2 ||
            mark_interruption_recorder_error assertion-stderr-reemit-failed
    else
        if "$interruption_assertion_command" "$interruption_assertion_pattern" \
            "$interruption_assertion_journal" >"$interruption_assertion_stdout_destination"; then
            INTERRUPTION_ASSERTION_STATUS=0
        else
            INTERRUPTION_ASSERTION_STATUS=$?
        fi
    fi

    record_interruption_event "$INTERRUPTION_RECORDER_DIR" "$INTERRUPTION_RECORDER_ID" \
        harness assertion-end "$INTERRUPTION_ASSERTION_STATUS" "$$" || true
    record_interruption_event "$INTERRUPTION_RECORDER_DIR" "$INTERRUPTION_RECORDER_ID" \
        harness snapshot-post-start 0 "$$" || true
    snapshot_interruption_file post "$interruption_assertion_journal" \
        "$INTERRUPTION_RECORDER_DIR/snapshot-post.bin" "$INTERRUPTION_CAPTURE_META" || true
    record_interruption_event "$INTERRUPTION_RECORDER_DIR" "$INTERRUPTION_RECORDER_ID" \
        harness snapshot-post-end "$INTERRUPTION_SNAPSHOT_STATUS" "$$" || true
    record_interruption_event "$INTERRUPTION_RECORDER_DIR" "$INTERRUPTION_RECORDER_ID" \
        harness capture-end "$INTERRUPTION_RECORDER_STATUS" "$$" || true
    return "$INTERRUPTION_ASSERTION_STATUS"
}

interruption_assertion_decision() {
    [ "$INTERRUPTION_ASSERTION_STATUS" -eq 0 ] && return 0
    echo "FAIL interruption first failure was not retained" >&2
    return 1
}

interruption_recorder_gate() {
    [ "$INTERRUPTION_RECORDER_STATUS" -eq 0 ] || {
        echo "FAIL interruption recorder unusable: ${INTERRUPTION_RECORDER_REASON:-unknown-recorder-error}" >&2
        return 1
    }
    return 0
}

verify_interruption_event_file() {
    interruption_events_file=$1
    interruption_expected_id=$2
    awk -F '\t' -v wanted="$interruption_expected_id" '
        $1 != wanted || $2 != "harness" || $3 != NR || $4 !~ /^[0-9]+$/ ||
            $6 !~ /^[0-9]+$/ || $7 !~ /^[0-9]+$/ { bad = 1 }
        NR > 1 && $4 < previous { bad = 1 }
        { previous = $4 }
        END { if (NR < 8 || bad) exit 1 }
    ' "$interruption_events_file"
}

verify_interruption_snapshots() {
    interruption_metadata_file=$1
    awk -F '\t' '
        NR == 1 { next }
        NR > 3 { bad = 1 }
        $1 != "pre" && $1 != "post" { bad = 1 }
        $2 !~ /^[0-9]+$/ || $3 !~ /^[0-9]+$/ || $4 !~ /^[0-9]+$/ ||
            $5 !~ /^[0-9]+$/ || $6 !~ /^[0-9]+$/ || $7 !~ /^[0-9]+$/ ||
            $8 !~ /^[0-9]+$/ || $9 != "ok" || $2 != $5 || $3 != $6 ||
            $4 != $7 || $4 != $8 { bad = 1 }
        { seen[$1]++ }
        END { if (NR != 3 || bad || seen["pre"] != 1 || seen["post"] != 1) exit 1 }
    ' "$interruption_metadata_file"
}

run_interruption_control() {
    interruption_fixture_dir=$1
    interrupt_dir="$WORK_ROOT/session-$$-interrupt"
    interrupt_ra="$interrupt_dir/ra"
    interrupt_capture_dir="$interrupt_dir/capture"
    interrupt_capture_id=${interrupt_capture_dir##*/}
    interrupt_trace="$interrupt_ra/interruption-events.tsv"
    interrupt_trace_errors="$interrupt_ra/interruption-trace-errors.tsv"
    mkdir -p "$interrupt_ra/shots" "$interrupt_ra/states" "$interrupt_capture_dir"
    ln -s "$script_path" "$interrupt_dir/fake-nc"
    ln -s "$script_path" "$interrupt_dir/fake-retroarch"
    : >"$interrupt_dir/config.cfg"; : >"$interrupt_dir/core.so"
    INTERRUPTION_RECORDER_DIR=$interrupt_capture_dir
    INTERRUPTION_RECORDER_ID=$interrupt_capture_id
    INTERRUPTION_EVENT_SEQUENCE=0
    INTERRUPTION_RECORDER_STATUS=0
    INTERRUPTION_RECORDER_REASON=
    interrupt_recorder_failure=0
    record_interruption_event "$interrupt_capture_dir" "$interrupt_capture_id" \
        harness driver-launch-start 0 "$$" || true
    STATE_SESSION_NC="$interrupt_dir/fake-nc" \
    STATE_SESSION_TEST_FIXTURE="$interruption_fixture_dir" \
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
    STATE_SESSION_TEST_INTERRUPT_INVOCATION="$interrupt_capture_id" \
    STATE_SESSION_TEST_INTERRUPT_TRACE="$interrupt_trace" \
    STATE_SESSION_TEST_INTERRUPT_TRACE_ERRORS="$interrupt_trace_errors" \
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
        "$interruption_fixture_dir/marker.nes" "$interrupt_ra" "$interrupt_dir/retroarch.log" "$FRAME_INDEX" \
        "$interruption_fixture_dir/calibration.txt" >"$interrupt_dir/driver.out" 2>&1 &
    interrupt_driver=$!
    record_interruption_event "$interrupt_capture_dir" "$interrupt_capture_id" \
        harness driver-launch-complete 0 "$interrupt_driver" || true
    _tries=0
    while [ ! -s "$interrupt_dir/host.pid" ] && [ "$_tries" -lt 300 ]; do
        if ! kill -0 "$interrupt_driver" 2>/dev/null; then
            echo "FAIL interrupt session ended before host setup" >&2
            cat "$interrupt_dir/driver.out" >&2
            return 1
        fi
        sleep 0.02; _tries=$((_tries+1))
    done
    if [ ! -s "$interrupt_dir/host.pid" ]; then
        echo "FAIL interrupt control never observed owned host" >&2
        return 1
    fi
    interrupt_host=$(cat "$interrupt_dir/host.pid")
    record_interruption_event "$interrupt_capture_dir" "$interrupt_capture_id" \
        harness host-observed 0 "$interrupt_host" || true
    if kill -0 "$interrupt_driver" 2>/dev/null; then interrupt_driver_live=0; else interrupt_driver_live=$?; fi
    if kill -0 "$interrupt_host" 2>/dev/null; then interrupt_host_live=0; else interrupt_host_live=$?; fi
    record_interruption_event "$interrupt_capture_dir" "$interrupt_capture_id" \
        harness pre-signal-liveness "$interrupt_host_live" "$interrupt_host" \
        "driver_pid=$interrupt_driver driver_liveness=$interrupt_driver_live" || true
    interrupt_signal_status=0
    kill -TERM "$interrupt_driver" 2>/dev/null || interrupt_signal_status=$?
    record_interruption_event "$interrupt_capture_dir" "$interrupt_capture_id" \
        harness signal-term "$interrupt_signal_status" "$interrupt_driver" || true
    interrupt_status=0
    if wait "$interrupt_driver"; then interrupt_status=0; else interrupt_status=$?; fi
    record_interruption_event "$interrupt_capture_dir" "$interrupt_capture_id" \
        harness wait-driver "$interrupt_status" "$interrupt_driver" || true
    [ "$interrupt_status" -eq 130 ] || {
        echo "FAIL interrupted driver exit status: expected 130, got $interrupt_status" >&2; return 1;
    }

    set -- "$interrupt_ra"/attempts/*
    interrupt_attempt=$1
    : >"$interrupt_capture_dir/glob-candidates.tsv" 2>/dev/null ||
        mark_interruption_recorder_error candidate-file-open-failed
    interrupt_candidate_number=0
    for interrupt_candidate do
        interrupt_candidate_number=$((interrupt_candidate_number + 1))
        printf '%s\t%s\n' "$interrupt_candidate_number" "$interrupt_candidate" \
            >>"$interrupt_capture_dir/glob-candidates.tsv" 2>/dev/null ||
            mark_interruption_recorder_error candidate-file-write-failed
        record_interruption_event "$interrupt_capture_dir" "$interrupt_capture_id" \
            harness glob-candidate 0 "$interrupt_driver" "candidate=$interrupt_candidate_number" || true
    done
    printf '%s\n' "$interrupt_attempt" >"$interrupt_capture_dir/selected-path.txt" 2>/dev/null ||
        mark_interruption_recorder_error selected-path-write-failed
    record_interruption_event "$interrupt_capture_dir" "$interrupt_capture_id" \
        harness selected-path 0 "$interrupt_driver" selected-first-expanded-candidate || true

    interrupt_journal="$interrupt_attempt/journal.tsv"
    record_interruption_event "$interrupt_capture_dir" "$interrupt_capture_id" \
        harness before-assertion 0 "$interrupt_driver" || true
    capture_interruption_assertion interruption_grep INTERRUPTED_TERM \
        "$interrupt_journal" "$interrupt_capture_dir" /dev/null || true
    interrupt_assertion_failure=0
    interruption_assertion_decision || interrupt_assertion_failure=1
    record_interruption_event "$interrupt_capture_dir" "$interrupt_capture_id" \
        harness assertion-decision "$INTERRUPTION_ASSERTION_STATUS" "$interrupt_driver" || true

    verify_interruption_snapshots "$interrupt_capture_dir/snapshot-metadata.tsv" ||
        mark_interruption_recorder_error snapshot-metadata-or-stability-invalid
    verify_interruption_driver_trace "$interrupt_trace" "$interrupt_capture_id" ||
        mark_interruption_recorder_error driver-trace-missing-or-invalid
    [ ! -s "$interrupt_trace_errors" ] ||
        mark_interruption_recorder_error driver-trace-reported-write-error
    [ -f "$interrupt_capture_dir/assertion.stdout" ] &&
        [ -f "$interrupt_capture_dir/assertion.stderr" ] ||
        mark_interruption_recorder_error assertion-output-capture-missing
    [ -s "$interrupt_capture_dir/glob-candidates.tsv" ] &&
        [ -s "$interrupt_capture_dir/selected-path.txt" ] ||
        mark_interruption_recorder_error glob-selection-evidence-missing

    interrupt_quit_count=$(awk '$0 == "QUIT" { n++ } END { print n + 0 }' "$interrupt_dir/sends.log")
    record_interruption_event "$interrupt_capture_dir" "$interrupt_capture_id" \
        harness quit-send-count "$interrupt_quit_count" "$interrupt_driver" || true
    [ "$interrupt_quit_count" -eq 1 ] || {
        echo "FAIL interruption cleanup did not send exactly one QUIT" >&2; return 1;
    }
    if kill -0 "$interrupt_host" 2>/dev/null; then interrupt_host_post_live=0; else interrupt_host_post_live=$?; fi
    record_interruption_event "$interrupt_capture_dir" "$interrupt_capture_id" \
        harness host-after-wait "$interrupt_host_post_live" "$interrupt_host" \
        "kill-0-failure-is-not-distinguished-from-permission-denial" || true
    [ "$interrupt_host_post_live" -ne 0 ] || {
        echo "FAIL interruption left host alive" >&2; return 1;
    }
    if [ -d "$interrupt_dir/locks/nesturbator-retroarch-55355.lock" ]; then interrupt_lock_post=1; else interrupt_lock_post=0; fi
    record_interruption_event "$interrupt_capture_dir" "$interrupt_capture_id" \
        harness lock-after-wait "$interrupt_lock_post" "$interrupt_driver" || true
    [ "$interrupt_lock_post" -eq 0 ] || {
        echo "FAIL interruption left endpoint lock behind" >&2; return 1;
    }
    assert_sanitized_tree "$interrupt_attempt"

    verify_interruption_event_file "$interrupt_capture_dir/events.tsv" "$interrupt_capture_id" ||
        mark_interruption_recorder_error harness-event-order-or-identity-invalid
    interruption_recorder_gate || interrupt_recorder_failure=1
    [ "${interrupt_recorder_failure:-0}" -eq 0 ] && [ "$interrupt_assertion_failure" -eq 0 ] || return 1
    printf 'state session interruption control: TERM retained; one QUIT; host reaped; capture=usable; trace=3 owned events\n'
}

if [ "$CONTROL" = interruption-capture-selftest ]; then
    run_interruption_capture_selftests "$WORK_ROOT"
    exit 0
fi

if [ "$CONTROL" = all ] || [ "$CONTROL" = interruption ]; then
    run_interruption_capture_selftests "$WORK_ROOT/interruption-capture-selftest"
fi

if [ "${STATE_SESSION_REPORT_SELFTEST:-0}" = 1 ] || [ "$CONTROL" = readiness ]; then
    readiness_name=readiness-response-stall
    readiness_fixture_elapsed=5666; readiness_setup_elapsed=42
    readiness_operation_ms=5460; readiness_later_prefix=16102
    readiness_attempt_elapsed=23104; readiness_elapsed=25946
    readiness_status=0; readiness_reason=OK
    readiness_lower=4500; readiness_upper=7000
    selftest_status=0
    if selftest_output=$(
        if command -v report_readiness_timing >/dev/null 2>&1; then
            report_readiness_timing "$readiness_name" "$readiness_fixture_elapsed" \
                "$readiness_setup_elapsed" "$readiness_operation_ms" \
                "$readiness_later_prefix" "$readiness_attempt_elapsed" \
                "$readiness_elapsed" "$readiness_status" "$readiness_reason" \
                "$readiness_lower" "$readiness_upper"
        fi
        if [ "$readiness_elapsed" -lt 25000 ]; then
            exit 0
        fi
        echo "FAIL $readiness_name full driver+cleanup bound: ${readiness_elapsed}ms"
        exit 1
    2>&1); then
        selftest_status=0
    else
        selftest_status=$?
    fi
    case "$selftest_output" in
        *"case=readiness-response-stall"*"fixture=5666ms"*"setup=42ms"*"operation=5460ms"*"later-prefix=16102ms"*"attempt-through-cleanup=23104ms"*"driver+cleanup=25946ms"*"driver-status=0"*"driver-reason=OK"*"FAIL readiness-response-stall full driver+cleanup bound: 25946ms"*) ;;
        *) echo "FAIL over-bound timing report was incomplete: $selftest_output" >&2; exit 1 ;;
    esac
    [ "$selftest_status" -eq 1 ] || {
        echo "FAIL over-bound timing assertion returned $selftest_status, expected 1" >&2
        exit 1
    }
    early_output=$(report_readiness_timing readiness-delayed 5666 42 unavailable \
        unavailable unavailable 2000 1 READINESS_REPLY 1201 4500)
    case "$early_output" in
        *"case=readiness-delayed"*"fixture=5666ms"*"setup=42ms"*"operation=unavailable"*"later-prefix=unavailable"*"attempt-through-cleanup=unavailable"*"driver+cleanup=2000ms"*"driver-status=1"*"driver-reason=READINESS_REPLY"*) ;;
        *) echo "FAIL early-error timing report invented or lost fields: $early_output" >&2; exit 1 ;;
    esac
    echo "state session timing report selftest: over-bound failure retained measured fields and status"
    if [ "${STATE_SESSION_REPORT_SELFTEST:-0}" = 1 ]; then
        exit 0
    fi
fi

script_path=$(cd "$(dirname "$0")" && pwd -P)/$(basename "$0")
test_started=$(mono_ms)
fixture_started=$(mono_ms)

test_dir="$WORK_ROOT/session-$$"
fixture_dir="$test_dir/fixture"
ra_dir="$test_dir/ra"
mkdir -p "$fixture_dir" "$ra_dir/shots" "$ra_dir/states"
if [ "$CONTROL" = readiness ]; then
    NESTURBATOR_RUN=$(cd "$(dirname "$FRAME_INDEX")/../../runner" && pwd -P)/nesturbator-run
    if ! "$FRAME_INDEX" --progression-selftest "$fixture_dir"; then
        echo "fixture precondition failed: frame_index --progression-selftest" >&2
        exit 2
    fi
    if [ ! -x "$NESTURBATOR_RUN" ] ||
        ! "$NESTURBATOR_RUN" --frames 101 --rom "$fixture_dir/marker.nes" \
            --dump-frame "101:$fixture_dir/alternative frame 101.ppm"; then
        echo "fixture precondition failed: nesturbator-run marker frame 101" >&2
        exit 2
    fi
    cp "$fixture_dir/pause capture one.ppm" "$fixture_dir/boundary 01 pause a.ppm" &&
        cp "$fixture_dir/pause capture two.ppm" "$fixture_dir/boundary 02 pause b.ppm" &&
        cp "$fixture_dir/advance one.ppm" "$fixture_dir/boundary 03 advance 1.ppm" || {
        echo "fixture precondition failed: readiness prefix aliases" >&2; exit 2;
    }
elif ! "$FRAME_INDEX" --progression-selftest "$fixture_dir" --controls; then
    echo "fixture precondition failed: frame_index --progression-selftest --controls" >&2
    exit 2
fi
fixture_finished=$(mono_ms)
fixture_elapsed=$((fixture_finished-fixture_started))

if [ "$CONTROL" = interruption ]; then
    run_interruption_control "$fixture_dir"
    exit 0
fi

ln -s "$script_path" "$test_dir/fake-nc"
ln -s "$script_path" "$test_dir/recording-action-nc"
ln -s "$script_path" "$test_dir/fake-retroarch"
: >"$test_dir/config.cfg"
: >"$test_dir/core.so"
expected_state_dir="$ra_dir/states"
send_log="$test_dir/sends.log"
action_send_log="$test_dir/action-sends.log"
readiness_send_log="$test_dir/readiness-sends.log"
host_pid_file="$test_dir/host.pid"
quit_file="$test_dir/quit"
shot_count_file="$test_dir/shot.count"
read_count_file="$test_dir/read.count"
state_count_file="$test_dir/state.count"
driver_output="$test_dir/driver.out"

if [ "$CONTROL" = all ]; then
status=0
STATE_SESSION_NC="$test_dir/recording-action-nc" \
STATE_SESSION_TEST_READINESS_NC="$test_dir/fake-nc" \
STATE_SESSION_TEST_MODE=1 \
STATE_SESSION_TEST_FIXTURE="$fixture_dir" \
STATE_SESSION_TEST_EXPECTED_DIR="$expected_state_dir" \
STATE_SESSION_TEST_SENDS="$send_log" \
STATE_SESSION_TEST_ACTION_SENDS="$action_send_log" \
STATE_SESSION_TEST_READINESS_SENDS="$readiness_send_log" \
STATE_SESSION_TEST_FAKE_NC="$test_dir/fake-nc" \
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
action_readiness_count=$(awk '$0 == "GET_CONFIG_PARAM savestate_directory" { n++ } END { print n + 0 }' "$action_send_log")
readiness_route_count=$(awk '$0 == "GET_CONFIG_PARAM savestate_directory" { n++ } END { print n + 0 }' "$readiness_send_log")
action_total=$(wc -l <"$action_send_log" | tr -d ' ')
sent_total=$(wc -l <"$send_log" | tr -d ' ')
[ "$action_readiness_count" -eq 0 ] && [ "$readiness_route_count" -eq 2 ] &&
    [ "$action_total" -eq $((sent_total-2)) ] || {
    echo "FAIL independent readiness/action routing: action=$action_total action_reads=$action_readiness_count readiness_reads=$readiness_route_count sent=$sent_total" >&2
    exit 1
}

printf 'state session tracer: PREFIX_OK; readiness bypassed the action wrapper; one FRAMEADVANCE; 2 startup and 3 boundary captures; one QUIT\n'

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
fi

run_fault() {
    name=$1; expected=$2; fault=$3; fail_request=${4:-}; mode=${5:-prefix}; max_ms=${6:-20000}; ready_ms=${7:-30000}; publication_ms=${8:-1000}; attempt_override=${9:-}; operation_override=${10:-30000}
    case_dir="$WORK_ROOT/session-$$-$name"; case_ra="$case_dir/ra"
    command_gap_ms=100
    [ "$fault" != stalled-write ] || command_gap_ms=0
    case_host_die=0; [ "$fault" != host-died ] || case_host_die=1
    case_setup_started=$(mono_ms)
    mkdir -p "$case_ra/shots" "$case_ra/states"
    ln -s "$script_path" "$case_dir/fake-nc"; ln -s "$script_path" "$case_dir/fake-retroarch"
    : >"$case_dir/config.cfg"; : >"$case_dir/core.so"
    case_setup_finished=$(mono_ms); case_setup_elapsed=$((case_setup_finished-case_setup_started))
    case_status=0; case_driver_started=$(mono_ms)
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
    STATE_SESSION_COMMAND_GAP_MS="$command_gap_ms" \
    STATE_SESSION_PROGRESS_WAIT_MS=20 \
    STATE_SESSION_QUIT_GRACE_MS=300 \
    sh "$DRIVER" "$case_dir/fake-retroarch" "$case_dir/config.cfg" "$case_dir/core.so" \
        "$fixture_dir/marker.nes" "$case_ra" "$case_dir/retroarch.log" "$FRAME_INDEX" \
        "$fixture_dir/calibration.txt" >"$case_dir/driver.out" 2>&1 || case_status=$?
    case_driver_finished=$(mono_ms); case_driver_elapsed=$((case_driver_finished-case_driver_started))
    if [ "$case_status" -eq 0 ] || ! grep -Fx "state_session: $expected" "$case_dir/driver.out" >/dev/null ||
        ! awk -F '\t' -v reason="$expected" '$11 == reason { found=1 } END { exit !found }' "$case_ra"/attempts/*/journal.tsv; then
        echo "FAIL $name expected exact terminal reason $expected (status=$case_status, setup ${case_setup_elapsed}ms, driver+cleanup ${case_driver_elapsed}ms)" >&2
        cat "$case_dir/driver.out" >&2
        for failed_journal in "$case_ra"/attempts/*/journal.tsv; do
            [ -f "$failed_journal" ] || continue
            printf 'FAIL %s last journal row: ' "$name" >&2
            tail -n 1 "$failed_journal" >&2
        done
        exit 1
    fi
    [ "$case_driver_elapsed" -lt "$max_ms" ] || {
        echo "FAIL $name driver-plus-cleanup bound: ${case_driver_elapsed}ms >= ${max_ms}ms (fixture setup ${case_setup_elapsed}ms)" >&2
        exit 1
    }
    case_host=$(cat "$case_dir/host.pid")
    if kill -0 "$case_host" 2>/dev/null; then echo "FAIL $name left owned host alive" >&2; exit 1; fi
    set -- "$case_ra"/attempts/*
    [ -d "$1" ] || { echo "FAIL $name did not retain its attempt tree" >&2; exit 1; }
    case_attempt=$1
    assert_sanitized_tree "$case_attempt"
    journal_contract=$(awk -F '\t' -v budget="$attempt_ms" -v reason="$expected" '
        NR == 1 {
            if ($0 != "step\tclass\tsends\tattempt_elapsed_ms\tattempt_remaining_ms\toperation_elapsed_ms\toperation_remaining_ms\texpected\tobserved\tartifact\treason" || NF != 11) bad="header"
            next
        }
        NF != 11 { bad="field-count"; next }
        $4 !~ /^[0-9]+$/ || $5 !~ /^[0-9]+$/ || $6 !~ /^[0-9]+$/ || $7 !~ /^[0-9]+$/ { bad="non-numeric-time"; next }
        $5 > 0 && ($4 + $5) != budget { bad="incoherent-attempt-budget" }
        $11 == reason && $1 != "cleanup-quit" {
            if (first_reason == "") { first_reason=$11; first_elapsed=$4 }
            terminal_elapsed=$4; terminal_operation_elapsed=$6; terminal_operation_remaining=$7
        }
        { last_reason=$11; last_elapsed=$4; last_operation_elapsed=$6; last_operation_remaining=$7; if ($1 == "cleanup-quit") cleanup_seen=1 }
        END {
            if (bad != "") { print bad; exit 1 }
            if (first_reason != reason) { print "missing-first-reason"; exit 1 }
            if (last_reason != reason) { print "cleanup-replaced-first-reason"; exit 1 }
            if (last_elapsed < terminal_elapsed || terminal_elapsed < first_elapsed) { print "attempt-time-moved-backward"; exit 1 }
            if (last_operation_elapsed != terminal_operation_elapsed || last_operation_remaining != terminal_operation_remaining) { print "terminal-operation-time-changed-during-cleanup"; exit 1 }
            if (cleanup_seen && last_elapsed <= terminal_elapsed) { print "cleanup-attempt-time-not-advanced"; exit 1 }
            print "ok"
        }
    ' "$case_attempt/journal.tsv") || {
        echo "FAIL $name journal contract: $journal_contract" >&2; exit 1;
    }
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
                [ -s "$case_attempt/original-anchor.state" ] &&
                    grep -F 'marker anchor frame=' "$case_attempt/anchor-original.out" >/dev/null || {
                    echo "FAIL $name reached the blocked action without a native-validated original anchor" >&2; exit 1;
                }
                awk -F '\t' 'NR == 2 { first=$4 } NR == 3 { second=$4 } END { exit !(NR == 3 && first < second) }' \
                    "$case_attempt/startup-captures.tsv" || {
                    echo "FAIL $name lacked two valid forward startup captures before its pause" >&2; exit 1;
                }
                awk -F '\t' 'NR == 1 { first=$11 } NR == 2 { second=$11 } END { exit !(NR == 2 && first == second) }' \
                    "$case_attempt/boundaries.tsv" || {
                    echo "FAIL $name lacked its independently published equal pause pair before the blocked action" >&2; exit 1;
                }
                pause_count=$(awk '$0 == "PAUSE_TOGGLE" { n++ } END { print n + 0 }' "$case_dir/sends.log")
                save_count=$(awk '$0 == "SAVE_STATE" { n++ } END { print n + 0 }' "$case_dir/sends.log")
                screenshot_count=$(awk '$0 == "SCREENSHOT" { n++ } END { print n + 0 }' "$case_dir/sends.log")
                advance_count=$(awk '$0 == "FRAMEADVANCE" { n++ } END { print n + 0 }' "$case_dir/sends.log")
                [ "$pause_count" -eq 1 ] && [ "$save_count" -eq 1 ] && [ "$screenshot_count" -eq 4 ] && [ "$advance_count" -eq 1 ] || {
                    echo "FAIL $name lacked exact pause/anchor/capture prerequisites before its single blocked advance" >&2; exit 1;
                }
                awk -F '\t' '$1 == "advance-1" && $11 == "ATTEMPT_DEADLINE" { found=($2 == "advance-1" && $3 == 1 && $8 == "FRAMEADVANCE") } END { exit !found }' \
                    "$case_attempt/journal.tsv" || {
                    echo "FAIL $name attempt-deadline row did not attribute the single blocked FRAMEADVANCE" >&2; exit 1;
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
    printf 'state session fault control: %s -> %s; setup %sms; driver+cleanup %sms; host reaped\n' \
        "$name" "$expected" "$case_setup_elapsed" "$case_driver_elapsed"
}
run_prefix_driver() {
    _run_dir=$1; _run_ra=$2; _run_fault=${3:-}; _run_lock_root=${4:-$_run_dir/locks}
    ln -sf "$script_path" "$_run_dir/recording-action-nc"
    STATE_SESSION_NC="$_run_dir/recording-action-nc" \
    STATE_SESSION_TEST_READINESS_NC="$_run_dir/fake-nc" \
    STATE_SESSION_TEST_FIXTURE="$fixture_dir" \
    STATE_SESSION_TEST_EXPECTED_DIR="$_run_ra/states" \
    STATE_SESSION_TEST_SENDS="$_run_dir/sends.log" \
    STATE_SESSION_TEST_ACTION_SENDS="$_run_dir/action-sends.log" \
    STATE_SESSION_TEST_READINESS_SENDS="$_run_dir/readiness-sends.log" \
    STATE_SESSION_TEST_FAKE_NC="$_run_dir/fake-nc" \
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

readiness_cases_run=0
run_readiness_case() {
    readiness_cases_run=$((readiness_cases_run+1))
    readiness_name=$1; readiness_fault=$2; readiness_lower=$3; readiness_upper=$4
    readiness_dir="$WORK_ROOT/session-$$-$readiness_name"; readiness_ra="$readiness_dir/ra"
    readiness_setup_started=$(mono_ms)
    mkdir -p "$readiness_ra/shots" "$readiness_ra/states"
    ln -s "$script_path" "$readiness_dir/fake-nc"; ln -s "$script_path" "$readiness_dir/fake-retroarch"
    : >"$readiness_dir/config.cfg"; : >"$readiness_dir/core.so"
    readiness_setup_finished=$(mono_ms)
    readiness_setup_elapsed=$((readiness_setup_finished-readiness_setup_started))
    readiness_started=$(mono_ms); readiness_status=0
    run_prefix_driver "$readiness_dir" "$readiness_ra" "$readiness_fault" >"$readiness_dir/driver.out" 2>&1 || readiness_status=$?
    readiness_finished=$(mono_ms); readiness_elapsed=$((readiness_finished-readiness_started))
    readiness_operation_ms=unavailable; readiness_later_prefix=unavailable
    readiness_attempt_elapsed=unavailable; readiness_reason=unknown
    set -- "$readiness_ra"/attempts/attempt-*
    if [ "$#" -eq 1 ] && [ -d "$1" ] && [ -s "$1/journal.tsv" ]; then
        readiness_attempt=$1
        readiness_row=$(awk -F '\t' '$1 == "readiness" { print; exit }' "$readiness_attempt/journal.tsv")
        readiness_oracle_row=$(awk -F '\t' '$1 == "oracle-prefix" && $11 == "OK" { row=$0 } END { print row }' \
            "$readiness_attempt/journal.tsv")
        readiness_cleanup_row=$(awk -F '\t' '$1 == "cleanup-quit" { row=$0 } END { print row }' \
            "$readiness_attempt/journal.tsv")
        if [ -n "$readiness_row" ]; then
            readiness_operation_ms=$(printf '%s\n' "$readiness_row" | cut -f6)
        fi
        if [ -n "$readiness_row" ] && [ -n "$readiness_oracle_row" ]; then
            readiness_row_elapsed=$(printf '%s\n' "$readiness_row" | cut -f4)
            readiness_oracle_elapsed=$(printf '%s\n' "$readiness_oracle_row" | cut -f4)
            case "$readiness_row_elapsed:$readiness_oracle_elapsed" in
                *[!0-9:]*|:*|*:) ;;
                *)
                    if [ "$readiness_oracle_elapsed" -ge "$readiness_row_elapsed" ]; then
                        readiness_later_prefix=$((readiness_oracle_elapsed-readiness_row_elapsed))
                    fi
                    ;;
            esac
        fi
        if [ -n "$readiness_cleanup_row" ]; then
            readiness_attempt_elapsed=$(printf '%s\n' "$readiness_cleanup_row" | cut -f4)
        fi
        readiness_reason=$(awk -F '\t' 'NR > 1 && NF == 11 { reason=$11 } END { print reason }' \
            "$readiness_attempt/journal.tsv")
    fi
    if [ "$readiness_status" -ne 0 ]; then
        readiness_driver_reason=$(sed -n 's/^state_session: \([A-Z][A-Z0-9_]*\)$/\1/p' \
            "$readiness_dir/driver.out" | tail -1)
        [ -n "$readiness_driver_reason" ] && readiness_reason=$readiness_driver_reason
        [ "$readiness_reason" != unknown ] || readiness_reason=unavailable
    else
        [ "$readiness_reason" != unknown ] || readiness_reason=OK
    fi
    report_readiness_timing "$readiness_name" "$fixture_elapsed" "$readiness_setup_elapsed" \
        "$readiness_operation_ms" "$readiness_later_prefix" "$readiness_attempt_elapsed" \
        "$readiness_elapsed" "$readiness_status" "$readiness_reason" \
        "$readiness_lower" "$readiness_upper"
    [ "$readiness_status" -eq 0 ] && grep -F 'verdict=PREFIX_OK' "$readiness_dir/driver.out" >/dev/null || {
        echo "FAIL $readiness_name full prefix did not reach PREFIX_OK (status=$readiness_status)" >&2
        cat "$readiness_dir/driver.out" >&2; exit 1;
    }
    for boundary in 1 2 3; do
        grep -Fx "progression boundaries=$boundary mode=prefix verdict=PREFIX_OK" "$readiness_dir/driver.out" >/dev/null || {
            echo "FAIL $readiness_name missing PREFIX_OK boundary $boundary" >&2; exit 1;
        }
    done
    set -- "$readiness_ra"/attempts/attempt-*
    [ "$#" -eq 1 ] && [ -d "$1" ] || {
        echo "FAIL $readiness_name did not retain exactly one attempt" >&2; exit 1;
    }
    readiness_attempt=$1
    readiness_row=$(awk -F '\t' '$1 == "readiness" { count++; if ($3 != 1 || $8 != "GET_CONFIG_PARAM savestate_directory <attempt>/states" || $9 != "exact-isolated-path" || $11 != "OK") exit 1; row=$0 } END { if (count != 1) exit 1; print row }' \
        "$readiness_attempt/journal.tsv") || {
        echo "FAIL $readiness_name exact one-send readiness journal row" >&2; exit 1;
    }
    readiness_operation_ms=$(printf '%s\n' "$readiness_row" | cut -f6)
    [ "$readiness_operation_ms" -ge "$readiness_lower" ] && [ "$readiness_operation_ms" -lt "$readiness_upper" ] || {
        echo "FAIL $readiness_name readiness operation bound: ${readiness_operation_ms}ms not in [${readiness_lower},${readiness_upper})" >&2; exit 1;
    }
    awk -F '\t' '
        NR == 1 { if ($0 != "step\tclass\tsends\tattempt_elapsed_ms\tattempt_remaining_ms\toperation_elapsed_ms\toperation_remaining_ms\texpected\tobserved\tartifact\treason" || NF != 11) exit 1; next }
        NF != 11 { bad=1; next }
        $4 !~ /^[0-9]+$/ || $5 !~ /^[0-9]+$/ || $6 !~ /^[0-9]+$/ || $7 !~ /^[0-9]+$/ { bad=1; next }
        $5 > 0 && ($4 + $5) != 30000 {
            printf "FAIL journal attempt budget: step=%s elapsed=%s remaining=%s budget=30000\n", $1, $4, $5 > "/dev/stderr"
            bad=1
        }
        ($6 + $7) != 30000 {
            printf "FAIL journal operation budget: step=%s elapsed=%s remaining=%s budget=30000\n", $1, $6, $7 > "/dev/stderr"
            bad=1
        }
        $1 == "readiness" { count++ }
        END { if (count != 1 || bad) exit 1 }
    ' "$readiness_attempt/journal.tsv" || {
        echo "FAIL $readiness_name readiness journal row accounting/schema" >&2; exit 1;
    }
    readiness_verdict_elapsed=$(awk -F '\t' '$1 == "oracle-prefix" && $11 == "OK" { value=$4 } END { print value }' "$readiness_attempt/journal.tsv")
    readiness_operation_attempt=$(printf '%s\n' "$readiness_row" | cut -f4)
    readiness_later_prefix=$((readiness_verdict_elapsed-readiness_operation_attempt))
    readiness_cleanup_elapsed=$(awk -F '\t' '$1 == "cleanup-quit" { value=$4 } END { print value }' "$readiness_attempt/journal.tsv")
    readiness_verdict_operation=$(awk -F '\t' '$1 == "oracle-prefix" && $11 == "OK" { value=$6 ":" $7 } END { print value }' "$readiness_attempt/journal.tsv")
    readiness_cleanup_operation=$(awk -F '\t' '$1 == "cleanup-quit" { value=$6 ":" $7 } END { print value }' "$readiness_attempt/journal.tsv")
    [ -n "$readiness_verdict_elapsed" ] && [ -n "$readiness_cleanup_elapsed" ] &&
        [ "$readiness_later_prefix" -ge 0 ] && [ "$readiness_cleanup_elapsed" -gt "$readiness_verdict_elapsed" ] &&
        [ "$readiness_verdict_operation" = "$readiness_cleanup_operation" ] || {
        echo "FAIL $readiness_name readiness/prefix/cleanup journal intervals" >&2; exit 1;
    }
    [ -s "$readiness_dir/stall.pid" ] || { echo "FAIL $readiness_name recorded no readiness child PID" >&2; exit 1; }
    readiness_child=$(cat "$readiness_dir/stall.pid")
    if kill -0 "$readiness_child" 2>/dev/null; then echo "FAIL $readiness_name left its readiness child alive" >&2; exit 1; fi
    readiness_host=$(cat "$readiness_dir/host.pid")
    if kill -0 "$readiness_host" 2>/dev/null; then echo "FAIL $readiness_name left its host alive" >&2; exit 1; fi
    readiness_send_count=$(awk '$0 == "GET_CONFIG_PARAM savestate_directory" { n++ } END { print n + 0 }' "$readiness_dir/sends.log")
    readiness_route_count=$(awk '$0 == "GET_CONFIG_PARAM savestate_directory" { n++ } END { print n + 0 }' "$readiness_dir/readiness-sends.log")
    readiness_action_read_count=$(awk '$0 == "GET_CONFIG_PARAM savestate_directory" { n++ } END { print n + 0 }' "$readiness_dir/action-sends.log")
    readiness_action_total=$(wc -l <"$readiness_dir/action-sends.log" | tr -d ' ')
    readiness_total=$(wc -l <"$readiness_dir/sends.log" | tr -d ' ')
    [ "$readiness_send_count" -eq 1 ] && [ "$readiness_route_count" -eq 1 ] &&
        [ "$readiness_action_read_count" -eq 0 ] &&
        [ "$readiness_action_total" -eq $((readiness_total-1)) ] || {
        echo "FAIL $readiness_name readiness/action routing: sent=$readiness_send_count routed=$readiness_route_count action-reads=$readiness_action_read_count action=$readiness_action_total total=$readiness_total" >&2; exit 1;
    }
    for request in PAUSE_TOGGLE SAVE_STATE FRAMEADVANCE QUIT; do
        request_count=$(awk -v wanted="$request" '$0 == wanted { n++ } END { print n + 0 }' "$readiness_dir/sends.log")
        [ "$request_count" -eq 1 ] || { echo "FAIL $readiness_name one-shot $request count=$request_count" >&2; exit 1; }
    done
    readiness_screenshot_sends=$(awk '$0 == "SCREENSHOT" { n++ } END { print n + 0 }' "$readiness_dir/sends.log")
    readiness_screenshot_captures=$(cat "$readiness_dir/shot.count")
    [ "$readiness_screenshot_sends" -eq 5 ] && [ "$readiness_screenshot_captures" -eq 5 ] || {
        echo "FAIL $readiness_name capture/send count: sends=$readiness_screenshot_sends captures=$readiness_screenshot_captures" >&2; exit 1;
    }
    readiness_startup_rows=$(wc -l <"$readiness_attempt/startup-captures.tsv" | tr -d ' ')
    readiness_boundary_rows=$(wc -l <"$readiness_attempt/boundaries.tsv" | tr -d ' ')
    readiness_state_count=$(cat "$readiness_dir/state.count")
    [ "$readiness_startup_rows" -eq 3 ] && [ "$readiness_boundary_rows" -eq 3 ] && [ "$readiness_state_count" -eq 1 ] || {
        echo "FAIL $readiness_name prefix attribution: startup=$readiness_startup_rows boundaries=$readiness_boundary_rows states=$readiness_state_count" >&2; exit 1;
    }
    [ "$readiness_elapsed" -ge "$readiness_lower" ] && [ "$readiness_elapsed" -lt 25000 ] || {
        echo "FAIL $readiness_name full driver+cleanup bound: ${readiness_elapsed}ms" >&2; exit 1;
    }
    assert_sanitized_tree "$readiness_attempt"
    readiness_outer_finished=$(mono_ms)
    readiness_outer_elapsed=$((readiness_outer_finished-test_started))
    printf 'state session readiness enclosing case=%s: CTest-process-so-far=%sms; child+host reaped\n' \
        "$readiness_name" "$readiness_outer_elapsed"
}

run_readiness_controls() {
    run_readiness_case readiness-delayed readiness-response-delay 1201 4500
    run_readiness_case readiness-response-stall readiness-response-stall 4500 7000
}

if [ "$CONTROL" = readiness ]; then
    run_readiness_controls
    [ "$readiness_cases_run" -eq 2 ] || {
        echo "FAIL focused readiness controls: expected delayed and stalled full-prefix cases, got $readiness_cases_run" >&2
        exit 1
    }
    readiness_lane_finished=$(mono_ms)
    readiness_lane_elapsed=$((readiness_lane_finished-test_started))
    printf 'state session readiness lane: cases=%s fixture=%sms enclosing-process=%sms bound=CTest<60000ms\n' \
        "$readiness_cases_run" "$fixture_elapsed" "$readiness_lane_elapsed"
    exit 0
fi

# A failed attempt-tree setup after lock acquisition must retain mkdir's exact failure status,
# release only the lock this invocation acquired, and let a later session acquire the endpoint.
setup_failure_dir="$WORK_ROOT/session-$$-setup-failure"
setup_failure_ra="$setup_failure_dir/ra"
setup_failure_locks="$setup_failure_dir/locks"
setup_failure_bin="$setup_failure_dir/bin"
setup_failure_lock="$setup_failure_locks/nesturbator-retroarch-55355.lock"
mkdir -p "$setup_failure_ra/shots" "$setup_failure_ra/states" "$setup_failure_bin"
ln -s "$script_path" "$setup_failure_dir/fake-nc"
ln -s "$script_path" "$setup_failure_dir/fake-retroarch"
: >"$setup_failure_dir/config.cfg"; : >"$setup_failure_dir/core.so"
real_mkdir=$(command -v mkdir)
cat >"$setup_failure_bin/mkdir" <<'EOF'
#!/bin/sh
for argument do
    case "$argument" in
        "${STATE_SESSION_TEST_FAIL_MKDIR_PREFIX}"*)
            echo 'injected mkdir failure at attempt setup' >&2
            exit 73
            ;;
    esac
done
exec "$STATE_SESSION_TEST_REAL_MKDIR" "$@"
EOF
chmod +x "$setup_failure_bin/mkdir"
setup_failure_status=0
PATH="$setup_failure_bin:$PATH" \
STATE_SESSION_TEST_FAIL_MKDIR_PREFIX="$setup_failure_ra/attempts/attempt-" \
STATE_SESSION_TEST_REAL_MKDIR="$real_mkdir" \
STATE_SESSION_NC="$setup_failure_dir/fake-nc" \
STATE_SESSION_TEST_READINESS_NC="$setup_failure_dir/fake-nc" \
STATE_SESSION_TEST_FIXTURE="$fixture_dir" \
STATE_SESSION_TEST_EXPECTED_DIR="$setup_failure_ra/states" \
STATE_SESSION_TEST_SENDS="$setup_failure_dir/sends.log" \
STATE_SESSION_TEST_ACTION_SENDS="$setup_failure_dir/action-sends.log" \
STATE_SESSION_TEST_READINESS_SENDS="$setup_failure_dir/readiness-sends.log" \
STATE_SESSION_TEST_FAKE_NC="$setup_failure_dir/fake-nc" \
STATE_SESSION_TEST_HOST_PID="$setup_failure_dir/host.pid" \
STATE_SESSION_TEST_QUIT_FILE="$setup_failure_dir/quit" \
STATE_SESSION_TEST_SHOT_COUNT="$setup_failure_dir/shot.count" \
STATE_SESSION_TEST_READ_COUNT="$setup_failure_dir/read.count" \
STATE_SESSION_TEST_STATE_COUNT="$setup_failure_dir/state.count" \
STATE_SESSION_TEST_STALL_PID="$setup_failure_dir/stall.pid" \
STATE_SESSION_TEST_MODE=prefix \
STATE_SESSION_TEST_STOP_AFTER_PREFIX=1 \
STATE_SESSION_TEST_SHOTS="$setup_failure_ra/shots" \
STATE_SESSION_LOCK_ROOT="$setup_failure_locks" \
STATE_SESSION_SETTLE_MS=0 \
sh "$DRIVER" "$setup_failure_dir/fake-retroarch" "$setup_failure_dir/config.cfg" \
    "$setup_failure_dir/core.so" "$fixture_dir/marker.nes" "$setup_failure_ra" \
    "$setup_failure_dir/retroarch.log" "$FRAME_INDEX" "$fixture_dir/calibration.txt" \
    >"$setup_failure_dir/driver.out" 2>&1 || setup_failure_status=$?
[ "$setup_failure_status" -eq 73 ] &&
    grep -Fx 'injected mkdir failure at attempt setup' "$setup_failure_dir/driver.out" >/dev/null &&
    [ ! -e "$setup_failure_lock" ] &&
    [ ! -e "$setup_failure_dir/host.pid" ] && [ ! -e "$setup_failure_dir/sends.log" ] || {
    echo "FAIL attempt setup failure did not preserve mkdir status and release its owned lock (status=$setup_failure_status)" >&2
    cat "$setup_failure_dir/driver.out" >&2
    exit 1
}
run_prefix_driver "$setup_failure_dir" "$setup_failure_ra" '' "$setup_failure_locks"
[ -s "$setup_failure_dir/sends.log" ] && [ ! -e "$setup_failure_lock" ] || {
    echo 'FAIL later session could not acquire the released endpoint lock' >&2
    exit 1
}
printf 'state session setup-failure control: mkdir status 73 preserved; owned lock released; later acquisition succeeded\n'

# The synthetic marker fixtures satisfy each semantic precondition; these controls execute the
# actual shell driver and its shared production oracle, then require its own exact failure row.
# A 300 ms fake-image publication window expired under hosted Intel scheduling before the
# duplicate-state prerequisite. The test-only 1 s bound remains inside each 30 s operation cap.
run_fault wrong-readiness READINESS_REPLY wrong-readiness
# Intel run 38641853044 measured 6.004 s for this driver+cleanup control; other
# long tests ran concurrently, so attribute no unproven cause to that excess. Keep
# the readiness operation at 350 ms, journal bound at 800 ms and exact reason.
# The direct exec-backed fake child sleeps 20 s: it cannot disappear naturally
# before this 7.5 s ceiling and the owned-child-reap assertion.
run_fault stalled-read READINESS_DEADLINE stalled-read '' prefix 7500 350
# Setup must have time to launch the blocked FRAMEADVANCE child. Its 60 s stall and operation
# ceiling exceed the 30 s attempt cap, so the test proves attempt-deadline cleanup deterministically.
# Skip irrelevant synthetic command gaps in this one long control; their short helper deadlines
# could preempt the intended fault before the blocked action is launched.
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
# Run 38598006408's macOS Intel control reached boundary 16 at 179.251 s; the
# 180 s attempt cap left no 30 s operation window to prove QUIT_DEADLINE. Keep
# the exact negative and operation deadline, with a finite test-only 240/250 s
# attempt/outer envelope and the same 1 s cleanup reserve.
run_fault quit-send-once QUIT_DEADLINE dropped-quit '' complete 250000 30000 1000 240000

run_readiness_controls

# Exercise the exact CMake-generated hosted action wrapper with an independent recording
# transport. A normal mutation forwards once, drop forwards zero times, and double forwards
# twice. A failed first send must remain the first failure and suppress no later eligible send.
wrapper_dir="$WORK_ROOT/session-$$-hosted-wrapper"
mkdir -p "$wrapper_dir"
ln -s "$script_path" "$wrapper_dir/hosted-wrapper-nc"
wrapper="$wrapper_dir/nc-one-shot.sh"
"$CMAKE_COMMAND" \
    "-DHOSTED_FAULT_WRAPPER_SELFTEST_OUTPUT=$wrapper" \
    "-DHOSTED_FAULT_WRAPPER_REAL_NC=$wrapper_dir/hosted-wrapper-nc" \
    -P "$RUN_RETROARCH_CMAKE"
printf 'PAUSE_TOGGLE' >"$wrapper_dir/pause.request"
HOSTED_MARKER_FAULT=double-advance HOSTED_FAULT_STATE="$wrapper_dir/pause-fault" \
    HOSTED_FAULT_LOG="$wrapper_dir/pause-fault.log" HOSTED_WRAPPER_TEST_SENDS="$wrapper_dir/pause.sends" \
    "$wrapper" -u -w0 127.0.0.1 55355 <"$wrapper_dir/pause.request"
[ "$(wc -l <"$wrapper_dir/pause.sends" | tr -d ' ')" -eq 1 ] &&
    [ "$(cat "$wrapper_dir/pause.sends")" = PAUSE_TOGGLE ] || {
    echo "FAIL hosted action wrapper did not forward a non-advance mutation exactly once" >&2; exit 1;
}
printf 'FRAMEADVANCE' >"$wrapper_dir/drop.request"
HOSTED_MARKER_FAULT=drop-advance HOSTED_FAULT_STATE="$wrapper_dir/drop-fault" \
    HOSTED_FAULT_LOG="$wrapper_dir/drop-fault.log" HOSTED_WRAPPER_TEST_SENDS="$wrapper_dir/drop.sends" \
    "$wrapper" -u -w0 127.0.0.1 55355 <"$wrapper_dir/drop.request"
[ ! -e "$wrapper_dir/drop.sends" ] && grep -Fx 'fault=drop-advance' "$wrapper_dir/drop-fault.log" >/dev/null || {
    echo "FAIL hosted dropped-advance wrapper sent a packet or lost its injection record" >&2; exit 1;
}
printf 'FRAMEADVANCE' >"$wrapper_dir/double.request"
wrapper_double_started=$(mono_ms)
HOSTED_MARKER_FAULT=double-advance HOSTED_FAULT_STATE="$wrapper_dir/double-fault" \
    HOSTED_FAULT_LOG="$wrapper_dir/double-fault.log" HOSTED_WRAPPER_TEST_SENDS="$wrapper_dir/double.sends" \
    HOSTED_WRAPPER_TEST_TIMES="$wrapper_dir/double.times" \
    "$wrapper" -u -w0 127.0.0.1 55355 <"$wrapper_dir/double.request"
[ "$(wc -l <"$wrapper_dir/double.sends" | tr -d ' ')" -eq 2 ] &&
    [ "$(cat "$wrapper_dir/double.sends")" = "FRAMEADVANCE
FRAMEADVANCE" ] || {
    echo "FAIL hosted doubled-advance wrapper did not forward two distinct sends" >&2; exit 1;
}
set -- $(cat "$wrapper_dir/double.times")
[ "$#" -eq 2 ] || { echo "FAIL hosted doubled-advance wrapper did not timestamp both sends" >&2; exit 1; }
first_double_send_ms=$1; second_double_send_ms=$2
wrapper_double_gap=$((second_double_send_ms-first_double_send_ms))
wrapper_double_finished=$(mono_ms)
wrapper_double_elapsed=$((wrapper_double_finished-wrapper_double_started))
[ "$wrapper_double_gap" -ge 200 ] && [ "$wrapper_double_gap" -le 2000 ] &&
    [ "$wrapper_double_elapsed" -ge 200 ] &&
    grep -Fx 'forwarded_send_count=2' "$wrapper_dir/double-fault.log" >/dev/null &&
    grep -E '^inter_send_gap_ms=[0-9]+$' "$wrapper_dir/double-fault.log" >/dev/null || {
    echo "FAIL hosted doubled-advance requests were not separated within the finite host-processing window (${wrapper_double_gap}ms, ${wrapper_double_elapsed}ms)" >&2; exit 1;
}
printf 'FRAMEADVANCE' >"$wrapper_dir/fail.request"
wrapper_failure=0
HOSTED_MARKER_FAULT=double-advance HOSTED_FAULT_STATE="$wrapper_dir/fail-fault" \
    HOSTED_FAULT_LOG="$wrapper_dir/fail-fault.log" HOSTED_WRAPPER_TEST_SENDS="$wrapper_dir/fail.sends" \
    HOSTED_WRAPPER_TEST_FAIL_FIRST=1 "$wrapper" -u -w0 127.0.0.1 55355 \
    <"$wrapper_dir/fail.request" || wrapper_failure=$?
[ "$wrapper_failure" -eq 93 ] && [ "$(wc -l <"$wrapper_dir/fail.sends" | tr -d ' ')" -eq 1 ] || {
    echo "FAIL hosted double wrapper did not preserve the first child-send failure" >&2; exit 1;
}

# On wrapper interruption, its nested nc child ignores TERM to force the bounded KILL/reap
# branch. The child would publish a delayed FRAMEADVANCE after two seconds if left alive.
printf 'FRAMEADVANCE' >"$wrapper_dir/timeout.request"
wrapper_started=$(mono_ms); wrapper_status=0
HOSTED_MARKER_FAULT=double-advance HOSTED_FAULT_STATE="$wrapper_dir/timeout-fault" \
    HOSTED_FAULT_LOG="$wrapper_dir/timeout-fault.log" HOSTED_WRAPPER_TEST_SENDS="$wrapper_dir/timeout.sends" \
    HOSTED_WRAPPER_TEST_DELAY=1 HOSTED_WRAPPER_TEST_CHILD_PID="$wrapper_dir/child.pid" \
    HOSTED_WRAPPER_TEST_LATE_SEND="$wrapper_dir/late.sends" \
    "$wrapper" -u -w0 127.0.0.1 55355 <"$wrapper_dir/timeout.request" \
    >"$wrapper_dir/timeout.out" 2>&1 & wrapper_pid=$!
_tries=0
while [ ! -s "$wrapper_dir/child.pid" ] && [ "$_tries" -lt 200 ]; do
    kill -0 "$wrapper_pid" 2>/dev/null || { echo "FAIL hosted wrapper exited before nested child launch" >&2; exit 1; }
    sleep 0.01; _tries=$((_tries+1))
done
[ -s "$wrapper_dir/child.pid" ] || { echo "FAIL hosted wrapper recorded no nested child PID" >&2; exit 1; }
wrapper_child=$(cat "$wrapper_dir/child.pid")
kill -0 "$wrapper_child" 2>/dev/null || { echo "FAIL hosted nested child exited before TERM" >&2; exit 1; }
kill -TERM "$wrapper_pid" 2>/dev/null || { echo "FAIL could not interrupt owned hosted wrapper" >&2; exit 1; }
(perl -MTime::HiRes=sleep -e 'sleep 2; kill 9, $ARGV[0]' "$wrapper_pid") & wrapper_watchdog=$!
wait "$wrapper_pid" || wrapper_status=$?
kill "$wrapper_watchdog" 2>/dev/null || true
wait "$wrapper_watchdog" 2>/dev/null || true
wrapper_finished=$(mono_ms); wrapper_lifetime=$((wrapper_finished-wrapper_started))
[ "$wrapper_status" -ne 0 ] && [ "$wrapper_lifetime" -lt 350 ] || {
    echo "FAIL hosted wrapper TERM cleanup did not exit within the 350 ms driver grace (status=$wrapper_status, ${wrapper_lifetime}ms)" >&2; exit 1;
}
if kill -0 "$wrapper_child" 2>/dev/null; then
    kill -KILL "$wrapper_child" 2>/dev/null || true
    echo "FAIL hosted wrapper left its nested nc child live" >&2; exit 1
fi
sleep 2.1
[ ! -e "$wrapper_dir/late.sends" ] && [ ! -e "$wrapper_dir/timeout.sends" ] || {
    echo "FAIL interrupted hosted wrapper allowed a delayed mutation send" >&2; exit 1;
}
set -- "$wrapper_dir"/timeout-fault.request.*
[ ! -e "$1" ] || { echo "FAIL hosted wrapper leaked its owned request file" >&2; exit 1; }
printf 'state session hosted-wrapper control: double=2 sends, drop=0, mutation=1; interrupted child reaped in %sms; no late send\n' \
    "$wrapper_lifetime"

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

run_interruption_control "$fixture_dir"

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
# The measured macOS Intel path reached boundary 16 at 179.251 s before QUIT.
# Give this positive fixture its bounded QUIT and complete-verdict windows too.
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
STATE_SESSION_ATTEMPT_MS=240000 \
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
