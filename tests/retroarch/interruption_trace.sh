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
        # The local driver publishes a token-bearing hard link; this records object occupancy only.
        if [ -L "$LOCK_DIR" ] || [ -e "$LOCK_DIR" ]; then _trace_lock=present; fi
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
    interruption_expected_driver_pid=${3:-}
    perl - "$interruption_trace_file" "$interruption_expected_id" \
        "$interruption_expected_driver_pid" <<'PERL'
use strict;
use warnings;

sub reject {
    my ($reason) = @_;
    print "$reason\n";
    exit 1;
}

sub decimal {
    return defined($_[0]) && $_[0] =~ /\A[0-9]+\z/;
}

sub positive_decimal {
    return decimal($_[0]) && $_[0] =~ /[1-9]/;
}

sub normalized_decimal {
    my ($value) = @_;
    $value =~ s/\A0+//;
    return length($value) ? $value : "0";
}

sub decimal_less_than {
    my ($left, $right) = map { normalized_decimal($_) } @_;
    return length($left) < length($right) ||
        (length($left) == length($right) && $left lt $right);
}

my ($path, $wanted_id, $wanted_driver_pid) = @ARGV;
reject("TRACE_INVOCATION") if !defined($wanted_id) || $wanted_id eq "";
reject("TRACE_DRIVER_PID") if !positive_decimal($wanted_driver_pid);
open my $trace, "<:raw", $path or reject("TRACE_FIELDS");
local $/;
my $bytes = <$trace>;
close $trace or reject("TRACE_FIELDS");
reject("TRACE_FIELDS") if !defined($bytes) || $bytes eq "" || $bytes !~ /\n\z/;
my @lines = split /\n/, $bytes, -1;
pop @lines;
reject("TRACE_FIELDS") if !@lines;
reject("TRACE_EVENT") if @lines != 3;

my @events = qw(handler-entry journal-close-success handler-exit);
my @statuses = qw(0 0 130);
my ($previous_time, $previous_send_count, $first_failure);
my @first_observation;
for my $index (0 .. $#lines) {
    my $row_number = $index + 1;
    my @field = split /\t/, $lines[$index], -1;
    reject("TRACE_FIELDS") if @field != 16;
    reject("TRACE_INVOCATION") if $field[0] ne $wanted_id;
    reject("TRACE_ROLE") if $field[1] ne "driver";
    reject("TRACE_SEQUENCE") if !decimal($field[2]) || $field[2] ne "$row_number";
    reject("TRACE_TIME") if !decimal($field[3]);
    if (defined $previous_time && decimal_less_than($field[3], $previous_time)) {
        reject("TRACE_TIME");
    }
    $previous_time = $field[3];
    reject("TRACE_EVENT") if $field[4] ne $events[$index];
    reject("TRACE_VALUE") if $field[5] ne $statuses[$index];
    reject("TRACE_DRIVER_PID") if !positive_decimal($field[6]) ||
        normalized_decimal($field[6]) ne normalized_decimal($wanted_driver_pid);
    for my $pid_index (7, 9) {
        reject("TRACE_VALUE") if $field[$pid_index] ne "unavailable" &&
            !positive_decimal($field[$pid_index]);
    }
    for my $alive_index (8, 10) {
        reject("TRACE_VALUE") if $field[$alive_index] ne "unavailable" &&
            $field[$alive_index] ne "1";
    }
    for my $pair ([7, 8], [9, 10]) {
        my ($pid_index, $alive_index) = @$pair;
        if ($row_number <= 2) {
            reject("TRACE_VALUE") if $field[$pid_index] eq "unavailable" &&
                $field[$alive_index] ne "unavailable";
            reject("TRACE_VALUE") if $field[$alive_index] eq "1" &&
                $field[$pid_index] eq "unavailable";
            if ($row_number == 1) {
                $first_observation[$pid_index] = $field[$pid_index];
            } else {
                reject("TRACE_VALUE") if $field[$pid_index] ne $first_observation[$pid_index];
            }
        } else {
            reject("TRACE_VALUE") if $field[$pid_index] ne "unavailable" ||
                $field[$alive_index] ne "unavailable";
        }
    }
    reject("TRACE_VALUE") if $field[11] !~ /\A[01]\z/;
    reject("TRACE_VALUE") if $field[12] !~ /\A[01]\z/;
    reject("TRACE_VALUE") if $field[13] ne ($row_number == 3 ? "absent" : "present");
    reject("TRACE_VALUE") if !decimal($field[14]);
    if (defined $previous_send_count && decimal_less_than($field[14], $previous_send_count)) {
        reject("TRACE_VALUE");
    }
    $previous_send_count = $field[14];
    if ($row_number == 1) {
        reject("TRACE_VALUE") if $field[11] ne "0" || $field[12] ne "0" ||
            $field[15] ne "none";
    } else {
        reject("TRACE_VALUE") if $field[12] ne "0" ||
            $field[15] !~ /\AINTERRUPTED_(?:INT|TERM|HUP)\z/;
        reject("TRACE_VALUE") if $row_number == 2 && $field[11] ne "0";
        if (defined $first_failure && $field[15] ne $first_failure) {
            reject("TRACE_VALUE");
        }
        $first_failure = $field[15];
    }
    reject("TRACE_VALUE") if $row_number == 3 && $field[11] ne "1";
}
print "TRACE_OK\n";
PERL
}
