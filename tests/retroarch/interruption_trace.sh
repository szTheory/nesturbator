# Shared, source-only interruption trace helpers. Callers own all process state.
trace_interruption_event() {
    [ "${STATE_SESSION_TEST_MODE:-}" = prefix ] &&
        [ -n "${STATE_SESSION_TEST_INTERRUPT_INVOCATION:-}" ] &&
        [ -n "${STATE_SESSION_TEST_INTERRUPT_TRACE:-}" ] || return 0
    _trace_path=$STATE_SESSION_TEST_INTERRUPT_TRACE
    _trace_errors=${STATE_SESSION_TEST_INTERRUPT_TRACE_ERRORS:-}
    _trace_error=0
    if [ "$_trace_path" != "$RA_DIR/interruption-events.tsv" ] ||
        [ "$_trace_errors" != "$RA_DIR/interruption-trace-errors.tsv" ] ||
        [ -L "$_trace_path" ] || [ -L "$_trace_errors" ] ||
        { [ "$INTERRUPTION_TRACE_SEQUENCE" -eq 0 ] && [ -e "$_trace_path" ]; }; then
        _trace_error=1
    fi
    if [ "$_trace_error" -eq 0 ]; then
        INTERRUPTION_TRACE_SEQUENCE=$((INTERRUPTION_TRACE_SEQUENCE + 1))
        _trace_host_pid=${HOST_PID:-unavailable}
        _trace_host_alive=unavailable
        if [ -n "$HOST_PID" ]; then
            if kill -0 "$HOST_PID" 2>/dev/null; then _trace_host_alive=1; else _trace_host_alive=unavailable; fi
        fi
        _trace_child_pid=${CHILD_PID:-unavailable}
        _trace_child_alive=unavailable
        if [ -n "$CHILD_PID" ]; then
            if kill -0 "$CHILD_PID" 2>/dev/null; then _trace_child_alive=1; else _trace_child_alive=unavailable; fi
        fi
        _trace_lock=absent
        [ ! -d "$LOCK_DIR" ] || _trace_lock=present
        _trace_first_failure=${FIRST_FAILURE:-none}
        STATE_INTERRUPT_TRACE="$_trace_path" STATE_INTERRUPT_ID="$STATE_SESSION_TEST_INTERRUPT_INVOCATION" \
        STATE_INTERRUPT_SEQUENCE="$INTERRUPTION_TRACE_SEQUENCE" STATE_INTERRUPT_EVENT="$1" \
        STATE_INTERRUPT_STATUS="$2" STATE_INTERRUPT_DRIVER_PID="$$" \
        STATE_INTERRUPT_HOST_PID="$_trace_host_pid" STATE_INTERRUPT_HOST_ALIVE="$_trace_host_alive" \
        STATE_INTERRUPT_CHILD_PID="$_trace_child_pid" STATE_INTERRUPT_CHILD_ALIVE="$_trace_child_alive" \
        STATE_INTERRUPT_QUIT_SENT="$QUIT_SENT" STATE_INTERRUPT_JOURNAL_FAILED="$JOURNAL_FAILED" \
        STATE_INTERRUPT_LOCK="$_trace_lock" STATE_INTERRUPT_SEND_COUNT="$send_count" \
        STATE_INTERRUPT_FIRST_FAILURE="$_trace_first_failure" \
        perl -MTime::HiRes=clock_gettime,CLOCK_MONOTONIC -e '
            use strict;
            use warnings;
            my $now = sprintf("%.0f", clock_gettime(CLOCK_MONOTONIC) * 1000);
            die "invalid monotonic trace value" unless $now =~ /\A[0-9]+\z/;
            open my $trace, ">>:raw", $ENV{STATE_INTERRUPT_TRACE} or die "trace open failed: $!";
            printf {$trace} "%s\tdriver\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n",
                @ENV{qw(STATE_INTERRUPT_ID STATE_INTERRUPT_SEQUENCE)}, $now,
                @ENV{qw(STATE_INTERRUPT_EVENT STATE_INTERRUPT_STATUS STATE_INTERRUPT_DRIVER_PID
                    STATE_INTERRUPT_HOST_PID STATE_INTERRUPT_HOST_ALIVE STATE_INTERRUPT_CHILD_PID
                    STATE_INTERRUPT_CHILD_ALIVE STATE_INTERRUPT_QUIT_SENT STATE_INTERRUPT_JOURNAL_FAILED
                    STATE_INTERRUPT_LOCK STATE_INTERRUPT_SEND_COUNT STATE_INTERRUPT_FIRST_FAILURE)};
            close $trace or die "trace close failed: $!";
        ' 2>/dev/null || _trace_error=1
    fi
    if [ "$_trace_error" -ne 0 ]; then
        INTERRUPTION_TRACE_FAILED=1
        if [ -n "$_trace_errors" ]; then
            printf '%s\t%s\t%s\n' \
                "${STATE_SESSION_TEST_INTERRUPT_INVOCATION:-unavailable}" "$1" "$$" \
                >>"$_trace_errors" 2>/dev/null || true
        fi
        return 1
    fi
    return 0
}

verify_interruption_driver_trace() {
    interruption_trace_file=$1
    interruption_expected_id=$2
    [ -s "$interruption_trace_file" ] || return 1
    awk -F '\t' -v wanted="$interruption_expected_id" '
        BEGIN { expected[1] = "handler-entry"; expected[2] = "journal-close-success"; expected[3] = "handler-exit" }
        $1 != wanted || $2 != "driver" || $3 != NR || $3 > 3 || $4 !~ /^[0-9]+$/ ||
            $5 != expected[NR] || $6 !~ /^[0-9]+$/ || $7 !~ /^[0-9]+$/ ||
            $8 !~ /^[0-9]+$/ || ($9 != "unavailable" && $9 !~ /^[0-9]+$/) ||
            ($10 != "unavailable" && $10 !~ /^[0-9]+$/) ||
            ($11 != "unavailable" && $11 !~ /^[0-9]+$/) ||
            $12 !~ /^[0-9]+$/ || $13 !~ /^[0-9]+$/ ||
            $14 !~ /^(present|absent)$/ || $15 !~ /^[0-9]+$/ { bad = 1 }
        NR > 1 && $4 < previous { bad = 1 }
        { previous = $4 }
        END { if (NR != 3 || bad) exit 1 }
    ' "$interruption_trace_file"
}
