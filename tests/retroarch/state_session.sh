#!/bin/sh
# Bounded owned RetroArch session. Arguments: RETROARCH CFG CORE CONTENT RA_DIR LOG FRAME_INDEX CALIBRATION.
# Each run gets its own HOME, config, screenshots, states, journal and verifier record. Only an
# exact read-only readiness query retries; action and publication requests are one-shot.
set -eu
[ "$#" -eq 8 ] || { echo "usage: state_session.sh RETROARCH CFG CORE CONTENT RA_DIR LOG FRAME_INDEX CALIBRATION" >&2; exit 2; }
RA=$1; CFG=$2; CORE=$3; CONTENT=$4; RA_DIR=$5; LOG=$6; FRAME_INDEX=$7; CALIBRATION=$8
STEM=${CONTENT##*/}; STEM=${STEM%.*}; NC=${STATE_SESSION_NC:-nc}
PROJECT_ROOT=$(CDPATH= cd "$(dirname "$0")/../.." && pwd -P)
. "$PROJECT_ROOT/tests/retroarch/interruption_trace.sh"
TIMEOUT_MS=${STATE_SESSION_ATTEMPT_MS:-120000}; OPERATION_MS=${STATE_SESSION_OPERATION_MS:-30000}
CLEANUP_MS=${STATE_SESSION_CLEANUP_MS:-5000}; READY_MS=${STATE_SESSION_READY_MS:-30000}
PUBLICATION_MS=${STATE_SESSION_PUBLICATION_MS:-5000}
SETTLE_MS=${STATE_SESSION_SETTLE_MS:-3000}; PAUSE_GAP_MS=${STATE_SESSION_PAUSE_GAP_MS:-1100}
PROGRESS_WAIT_MS=${STATE_SESSION_PROGRESS_WAIT_MS:-1000}; COMMAND_GAP_MS=${STATE_SESSION_COMMAND_GAP_MS:-100}
REWIND_GAP_MS=50; READINESS_CHILD_MS=5000; READINESS_NC_TIMEOUT_SECS=1
for path in "$RA" "$CFG" "$CORE" "$CONTENT" "$FRAME_INDEX" "$CALIBRATION"; do
    [ -e "$path" ] || { echo "state_session: INPUT_MISSING" >&2; exit 2; }
done
case "$TIMEOUT_MS:$OPERATION_MS:$CLEANUP_MS:$READY_MS:$PUBLICATION_MS" in
    *[!0-9:]*|::*|:*|*:) echo "state_session: INVALID_BUDGET" >&2; exit 2 ;;
esac
    [ "$TIMEOUT_MS" -gt "$CLEANUP_MS" ] && [ "$OPERATION_MS" -gt 0 ] || { echo "state_session: INVALID_BUDGET" >&2; exit 2; }
clock_ms() {
    # Keep the initial bootstrap sample in Perl for the descriptor handoff; repeated
    # observations reuse the runner's native clock without starting an interpreter.
    if [ "${STATE_SESSION_LOCK_BOOTSTRAPPED:-}" = 1 ]; then
        "$FRAME_INDEX" --session-clock 2>/dev/null
    else
        perl -MTime::HiRes=clock_gettime,CLOCK_MONOTONIC -e 'printf "%.0f\n",clock_gettime(CLOCK_MONOTONIC)*1000' 2>/dev/null
    fi
}
lock_reservation_is_held() {
    trace_lock_reservation_is_held
}
record_test_lock_reservation() {
    [ -n "${STATE_SESSION_TEST_MODE:-}" ] || return 0
    STATE_LOCK_RESERVATION_PATH="$RA_DIR/lock-reservation.tsv" \
    STATE_LOCK_RESERVATION_PID="$$" STATE_LOCK_RESERVATION_PORT="$STATE_SESSION_LOCK_PORT" \
    STATE_LOCK_RESERVATION_DEVICE="$STATE_SESSION_LOCK_DEVICE" \
    STATE_LOCK_RESERVATION_INODE="$STATE_SESSION_LOCK_INODE" \
    perl -MSocket -MFcntl=:DEFAULT -e '
        use strict;
        use warnings;
        sub reject {
            my ($reason) = @_;
            warn "reservation-writer: $reason\n";
            exit 1;
        }
        my ($path, $pid, $boot_pid, $port, $device, $inode) =
            @ENV{qw(STATE_LOCK_RESERVATION_PATH STATE_LOCK_RESERVATION_PID STATE_SESSION_LOCK_BOOT_PID STATE_LOCK_RESERVATION_PORT STATE_LOCK_RESERVATION_DEVICE STATE_LOCK_RESERVATION_INODE)};
        reject("invalid-input") unless defined($path) && length($path) &&
            defined($pid) && $pid =~ /\A[1-9][0-9]*\z/ &&
            defined($boot_pid) && $boot_pid =~ /\A[1-9][0-9]*\z/ && $pid == $boot_pid &&
            defined($port) && $port =~ /\A[1-9][0-9]*\z/ && $port <= 65535 &&
            defined($device) && $device =~ /\A[0-9]+\z/ &&
            defined($inode) && $inode =~ /\A[0-9]+\z/;
        open my $guard, ">&=9" or reject("fd9-open: $!");
        my @identity = stat($guard);
        reject("fd9-identity") unless @identity && $identity[0] == $device && $identity[1] == $inode;
        my $type = getsockopt($guard, SOL_SOCKET, SO_TYPE);
        reject("socket-type") unless defined($type) && unpack("i", $type) == SOCK_STREAM;
        $! = 0;
        my $accepting = getsockopt($guard, SOL_SOCKET, SO_ACCEPTCONN);
        if (defined($accepting)) {
            reject("socket-listening") unless unpack("i", $accepting) == 0;
        } else {
            my $error = 0 + $!;
            reject("socket-accept-state-unavailable") unless $error == 42 || $error == 45 || $error == 92;
        }
        my $reuse = getsockopt($guard, SOL_SOCKET, SO_REUSEADDR);
        reject("reuseaddr") unless defined($reuse) && unpack("i", $reuse) == 0;
        my $local = getsockname($guard);
        reject("local-address") unless defined($local);
        my ($actual_port, $address) = sockaddr_in($local);
        reject("bound-endpoint") unless $actual_port == $port && inet_ntoa($address) eq "127.0.0.1";
        my $reuse_port = "unsupported";
        if (defined(&SO_REUSEPORT)) {
            my $actual = getsockopt($guard, SOL_SOCKET, SO_REUSEPORT);
            if (defined($actual)) {
                $reuse_port = unpack("i", $actual);
                reject("reuseport") unless $reuse_port == 0;
            } else {
                my $error = 0 + $!;
                reject("reuseport-unavailable") unless $error == 42 || $error == 45 || $error == 92;
            }
        }
        my $record = join("\t", qw(address port driver_pid fd device inode socket_type reuseaddr reuseport)) . "\n" .
            join("\t", "127.0.0.1", $actual_port, $pid, 9, $identity[0], $identity[1], "SOCK_STREAM", 0, $reuse_port) . "\n";
        sysopen my $output, $path, O_WRONLY | O_CREAT | O_EXCL, 0600 or reject("record-open: $!");
        print {$output} $record or reject("record-write: $!");
        close $output or reject("record-close: $!");
    '
}
lock_release_test_gate() {
    [ -n "${STATE_SESSION_TEST_RELEASE_GATE:-}" ] || return 0
    [ -n "${STATE_SESSION_TEST_MODE:-}" ] || return 1
    STATE_LOCK_RELEASE_GATE="$STATE_SESSION_TEST_RELEASE_GATE" \
    STATE_LOCK_RELEASE_CASE_ROOT="${STATE_SESSION_TEST_CASE_ROOT:-}" \
    perl -MCwd=abs_path -MFcntl=:mode -e '
        use strict;
        use warnings;
        my ($gate_path, $case_path) = @ENV{qw(STATE_LOCK_RELEASE_GATE STATE_LOCK_RELEASE_CASE_ROOT)};
        exit 1 unless defined($gate_path) && defined($case_path) && length($case_path);
        my $gate = abs_path($gate_path);
        my $case = abs_path($case_path);
        exit 1 unless defined($gate) && defined($case) && index($gate, "$case/") == 0;
        my @gate = lstat($gate_path);
        exit 1 unless @gate && S_ISDIR($gate[2]);
        for my $name (qw(.ready .continue)) {
            my @entry = lstat("$gate_path/$name");
            exit 1 unless @entry && S_ISFIFO($entry[2]);
        }
    ' 2>/dev/null || return 1
    STATE_LOCK_RELEASE_GATE="$STATE_SESSION_TEST_RELEASE_GATE" \
    STATE_LOCK_RELEASE_CASE_ROOT="${STATE_SESSION_TEST_CASE_ROOT:-}" \
    STATE_LOCK_RELEASE_START="$START_MS" STATE_LOCK_RELEASE_ATTEMPT="$TIMEOUT_MS" \
    STATE_LOCK_RELEASE_CLEANUP="$CLEANUP_MS" \
    STATE_LOCK_RELEASE_PID="$$" STATE_LOCK_RELEASE_PORT="$STATE_SESSION_LOCK_PORT" \
    perl -MFcntl=:DEFAULT -MIO::Select -MIO::Handle -e '
        use strict;
        use warnings;
        use Time::HiRes qw(clock_gettime CLOCK_MONOTONIC);
        my $root = $ENV{STATE_LOCK_RELEASE_GATE};
        my $pid = $ENV{STATE_LOCK_RELEASE_PID};
        my $port = $ENV{STATE_LOCK_RELEASE_PORT};
        my $events = "$root/.events";
        my $record = sub {
            my ($name) = @_;
            my $now = sprintf("%.0f", clock_gettime(CLOCK_MONOTONIC) * 1000);
            open my $output, ">>:raw", $events or die "release-event-open: $!\n";
            print {$output} join("\t", $name, $pid, $port, $now), "\n" or die "release-event-write: $!\n";
            close $output or die "release-event-close: $!\n";
        };
        my $now = sub { clock_gettime(CLOCK_MONOTONIC) * 1000 };
        my $deadline = $ENV{STATE_LOCK_RELEASE_START} + $ENV{STATE_LOCK_RELEASE_ATTEMPT};
        my $cleanup_end = $now->() + $ENV{STATE_LOCK_RELEASE_CLEANUP};
        $deadline = $cleanup_end if $cleanup_end < $deadline;
        if (defined($ENV{STATE_SESSION_TEST_GATE_DEADLINE_MS})) {
            my $outer = $ENV{STATE_SESSION_TEST_GATE_DEADLINE_MS};
            die "release-invalid-deadline\n" unless $outer =~ /\A[0-9]+\z/;
            $deadline = $outer if $outer < $deadline;
        }
        my $remaining = sub {
            my $left = ($deadline - $now->()) / 1000;
            die "release-gate-timeout\n" unless $left > 0;
            return $left;
        };
        sysopen my $ready, "$root/.ready", O_RDWR | O_NONBLOCK or die "release-ready-open: $!\n";
        sysopen my $continue, "$root/.continue", O_RDWR | O_NONBLOCK or die "release-continue-open: $!\n";
        my $write = sub {
            my ($bytes) = @_;
            while (length($bytes)) {
                IO::Select->new($ready)->can_write($remaining->()) or die "release-write-timeout\n";
                my $n = syswrite($ready, $bytes);
                if (!defined($n)) { next if $!{EAGAIN} || $!{EINTR}; die "release-write: $!\n"; }
                die "release-zero-write\n" unless $n;
                substr($bytes, 0, $n, "");
            }
        };
        $record->("release-boundary");
        $write->("ready\t$pid\t$port\n");
        my $line = "";
        while (index($line, "\n") < 0) {
            IO::Select->new($continue)->can_read($remaining->()) or die "release-continue-timeout\n";
            my $n = sysread($continue, my $byte, 1);
            if (!defined($n)) { next if $!{EAGAIN} || $!{EINTR}; die "release-read: $!\n"; }
            die "release-invalid-line\n" unless $n && length($line) < 32;
            $line .= $byte;
        }
        $line eq "continue\n" or die "release-continue-invalid\n";
        $write->("continued\t$pid\t$port\n");
        $record->("release-completed");
    ' 9>&- 2>"$RA_DIR/lock-release-gate.stderr"
}
release_owned_lock() {
    [ "${LOCK_RESERVATION_CLOSED:-0}" -eq 0 ] || return 0
    LOCK_RESERVATION_CLOSED=1
    lock_release_test_gate || LOCK_RELEASE_GATE_FAILED=1
    exec 9>&-
    STATE_SESSION_LOCK_PORT=
    return 0
}
case "${STATE_SESSION_TEST_MODE:-}" in
    ''|1|prefix|complete|lock-release-race) ;;
    *) echo "state_session: LOCK_OWNER_UNAVAILABLE" >&2; exit 2 ;;
esac
if [ "${STATE_SESSION_LOCK_BOOTSTRAPPED:-}" = 1 ]; then
    START_MS=${STATE_SESSION_LOCK_START_MS:-}
    LOCK_PORT=${STATE_SESSION_LOCK_PORT:-}
    case "$START_MS:$LOCK_PORT" in *[!0-9:]*|::*|:*|*:) echo "state_session: LOCK_OWNER_UNAVAILABLE" >&2; exit 2 ;; esac
    if [ -z "${STATE_SESSION_TEST_MODE:-}" ] && [ "$LOCK_PORT" != 55355 ]; then
        echo "state_session: LOCK_OWNER_UNAVAILABLE" >&2
        exit 2
    fi
    LOCK_NOW=$(clock_ms) || { echo "state_session: LOCK_OWNER_UNAVAILABLE" >&2; exit 2; }
    if [ "$START_MS" -gt "$LOCK_NOW" ] || [ "$((LOCK_NOW-START_MS))" -ge "$TIMEOUT_MS" ] ||
        ! lock_reservation_is_held; then
        echo "state_session: LOCK_OWNER_UNAVAILABLE" >&2
        exit 2
    fi
else
    [ -z "${STATE_SESSION_LOCK_BOOTSTRAPPED:-}" ] &&
        [ -z "${STATE_SESSION_LOCK_DEVICE:-}" ] &&
        [ -z "${STATE_SESSION_LOCK_INODE:-}" ] &&
        [ -z "${STATE_SESSION_LOCK_BOOT_PID:-}" ] &&
        [ -z "${STATE_SESSION_LOCK_START_MS:-}" ] &&
        [ -z "${STATE_SESSION_LOCK_PORT:-}" ] || {
            echo "state_session: LOCK_OWNER_UNAVAILABLE" >&2; exit 2;
        }
    START_MS=$(clock_ms) || { echo "state_session: MONOTONIC_CLOCK_UNAVAILABLE" >&2; exit 2; }
    case "$START_MS" in *[!0-9]*|'') echo "state_session: MONOTONIC_CLOCK_UNAVAILABLE" >&2; exit 2 ;; esac
    trap 'echo "state_session: LOCK_OWNER_UNAVAILABLE" >&2' EXIT
    exec perl -e '
        use strict;
        use warnings;

        sub fail { warn "state_session: LOCK_OWNER_UNAVAILABLE\n"; exit 2; }
        BEGIN {
            eval { require Socket; Socket->import(); require Fcntl; Fcntl->import();
                require Time::HiRes; require Errno; Errno->import("EADDRINUSE"); 1 }
                or do { warn "state_session: LOCK_OWNER_UNAVAILABLE\n"; exit 2; };
        }
        my ($driver, @args) = @ARGV;
        my $start = shift @args;
        fail("invalid original start") unless defined($start) && $start =~ /\A[0-9]+\z/ && $start > 0;
        fail("invalid driver entry") unless defined($driver) && length($driver);
        fail("argument count") unless @args == 8;
        my $mode = $ENV{STATE_SESSION_TEST_MODE} // "";
        fail("unrecognized test mode") unless $mode =~ /\A(?:|1|prefix|complete|lock-release-race)\z/;
        my $test_mode = length($mode) != 0;
        my $now = Time::HiRes::clock_gettime(Time::HiRes::CLOCK_MONOTONIC()) * 1000;
        fail("invalid original start") unless $start <= $now;
        my $requested;
        if ($test_mode) {
            $requested = defined($ENV{STATE_SESSION_TEST_GUARD_PORT})
                ? $ENV{STATE_SESSION_TEST_GUARD_PORT} : "0";
            fail("invalid test guard port") unless $requested =~ /\A(?:0|[1-9][0-9]{0,4})\z/ && $requested <= 65535;
        } else {
            fail("test port override outside test mode") if defined($ENV{STATE_SESSION_TEST_GUARD_PORT});
            $requested = "55355";
        }
        my $protocol = getprotobyname("tcp");
        defined($protocol) or fail("TCP protocol unavailable");
        socket(my $socket, PF_INET, SOCK_STREAM, $protocol) or fail("socket: $!");
        my @reuse_options = (SO_REUSEADDR);
        push @reuse_options, SO_REUSEPORT() if defined(&SO_REUSEPORT);
        for my $option (@reuse_options) {
            $! = 0;
            my $set = eval { setsockopt($socket, SOL_SOCKET, $option, pack("i", 0)) };
            if (!$set) {
                my $error = 0 + $!;
                my $unsupported = $error == 42 || $error == 45 || $error == 92;
                next if $option != SO_REUSEADDR && $unsupported;
                fail("reuse option unavailable");
            }
            $! = 0;
            my $actual = getsockopt($socket, SOL_SOCKET, $option);
            if (!defined($actual)) {
                my $error = 0 + $!;
                next if $option != SO_REUSEADDR && ($error == 42 || $error == 45 || $error == 92);
            }
            fail("reuse option verification") unless defined($actual) && unpack("i", $actual) == 0;
        }
        if (!bind($socket, sockaddr_in(0 + $requested, inet_aton("127.0.0.1")))) {
            my $error = 0 + $!;
            if ($error == EADDRINUSE) { warn "state_session: ENDPOINT_BUSY\n"; exit 1; }
            fail("bind 127.0.0.1:$requested: $!");
        }
        my $local = getsockname($socket);
        defined($local) or fail("getsockname: $!");
        my ($port, $address) = sockaddr_in($local);
        fail("bound address mismatch") unless inet_ntoa($address) eq "127.0.0.1";
        fail("bound port mismatch") if $requested != 0 && $port != $requested;
        my $source_fd = fileno($socket);
        defined($source_fd) or fail("socket descriptor unavailable");
        my $guard_fd = $source_fd == 9 ? 9 : fcntl($socket, F_DUPFD, 9);
        defined($guard_fd) && $guard_fd == 9 or fail("descriptor 9 unavailable");
        close($socket) or fail("socket duplicate close: $!") if $source_fd != 9;
        open my $guard, ">&=9" or fail("descriptor 9 reopen: $!");
        my $fd_flags = fcntl($guard, F_GETFD, 0);
        defined($fd_flags) or fail("descriptor flags read: $!");
        fcntl($guard, F_SETFD, $fd_flags & ~FD_CLOEXEC) or fail("descriptor inheritance: $!");
        my @identity = stat($guard);
        @identity or fail("descriptor identity: $!");
        $ENV{STATE_SESSION_LOCK_BOOTSTRAPPED} = "1";
        $ENV{STATE_SESSION_LOCK_BOOT_PID} = "$$";
        $ENV{STATE_SESSION_LOCK_START_MS} = $start;
        $ENV{STATE_SESSION_LOCK_PORT} = "$port";
        $ENV{STATE_SESSION_LOCK_DEVICE} = "$identity[0]";
        $ENV{STATE_SESSION_LOCK_INODE} = "$identity[1]";
        exec "/bin/sh", $driver, @args or fail("exec /bin/sh: $!");
    ' "$0" "$START_MS" "$@"
fi
LOCK_RESERVATION_CLOSED=0; LOCK_RELEASE_GATE_FAILED=0
trap release_owned_lock EXIT
trap 'exit 130' INT; trap 'exit 143' TERM; trap 'exit 129' HUP
mkdir -p "$RA_DIR/attempts"
if ! record_test_lock_reservation; then
    echo "state_session: LOCK_OWNER_UNAVAILABLE" >&2
    exit 2
fi
ATTEMPT_ID="attempt-${START_MS}-$$"; ATTEMPT_DIR="$RA_DIR/attempts/$ATTEMPT_ID"
LOCK_ROOT=${STATE_SESSION_LOCK_ROOT:-${TMPDIR:-/tmp}}
mkdir -p "$LOCK_ROOT" || { echo "state_session: LOCK_OWNER_UNAVAILABLE" >&2; exit 1; }
LOCK_DIR="$LOCK_ROOT/nesturbator-retroarch-55355.lock"
LOCK_OWNER_FILE="$LOCK_ROOT/nesturbator-retroarch-55355.lock.owner.$ATTEMPT_ID"
LOCK_OWNER_ALIAS="$LOCK_DIR/${LOCK_OWNER_FILE##*/}"
legacy_lock_path_state() {
    perl -MFcntl=:mode -e '
        use strict;
        use warnings;
        my $root = shift @ARGV;
        my @root = lstat($root);
        if (!@root || !S_ISDIR($root[2]) || !-r $root || !-x $root) {
            print "unavailable\n"; exit 0;
        }
        for my $path (@ARGV) {
            my @entry = lstat($path);
            if (@entry) {
                print S_ISDIR($entry[2]) ? "occupied-directory\n" : "occupied-object\n";
                exit 0;
            }
            if ($! != 2) { print "unavailable\n"; exit 0; }
        }
        print "clear\n";
    ' "$LOCK_ROOT" "$LOCK_DIR" "$LOCK_OWNER_FILE" 2>/dev/null
}
LEGACY_LOCK_STATE=$(legacy_lock_path_state) || LEGACY_LOCK_STATE=unavailable
case "$LEGACY_LOCK_STATE" in
    occupied-directory) echo "state_session: ENDPOINT_BUSY" >&2; exit 1 ;;
    occupied-object)
        echo "state_session: LOCK_OWNER_UNAVAILABLE" >&2
        exit 1 ;;
    clear) ;;
    *) echo "state_session: LOCK_OWNER_UNAVAILABLE" >&2; exit 1 ;;
esac
if [ -n "${STATE_SESSION_TEST_RELEASE_GATE:-}" ] &&
    [ -n "${STATE_SESSION_TEST_MODE:-}" ]; then
    kill -TERM "$$"
fi
mkdir -p "$ATTEMPT_DIR/profile" "$ATTEMPT_DIR/states" "$ATTEMPT_DIR/shots" "$ATTEMPT_DIR/originals" "$ATTEMPT_DIR/observations"
JOURNAL="$ATTEMPT_DIR/journal.tsv"
printf 'step\tclass\tsends\tattempt_elapsed_ms\tattempt_remaining_ms\toperation_elapsed_ms\toperation_remaining_ms\texpected\tobserved\tartifact\treason\n' >"$JOURNAL"
HOST_PID=; CHILD_PID=; FIRST_FAILURE=; LAST_REASON=OK; send_count=0; QUIT_SENT=0; JOURNAL_FAILED=0
CURRENT_STEP=setup; CURRENT_CLASS=none; CURRENT_SENDS=0; CURRENT_EXPECTED=none; CURRENT_OBSERVED=none; CURRENT_ARTIFACT=none
ACTIVE_OPERATION=; OPERATION_START_MS=; OPERATION_DEADLINE_MS=; OPERATION_BUDGET_MS=; OPERATION_STOP_MS=; OPERATION_DEADLINE_VALUE=0
LAST_OP_ELAPSED_MS=0; LAST_OP_REMAINING_MS=0
INTERRUPTION_TRACE_SEQUENCE=0; INTERRUPTION_TRACE_FAILED=0
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
    [ "$JOURNAL_FAILED" -eq 0 ] || return 0
    _journal_status=0
    SESSION_JOURNAL="$JOURNAL" \
    SESSION_START_MS="$START_MS" SESSION_TIMEOUT_MS="$TIMEOUT_MS" \
    SESSION_OPERATION_ACTIVE="$ACTIVE_OPERATION" \
    SESSION_OPERATION_START_MS="$OPERATION_START_MS" \
    SESSION_OPERATION_DEADLINE_MS="$OPERATION_DEADLINE_MS" \
    SESSION_OPERATION_STOP_MS="$OPERATION_STOP_MS" \
    SESSION_LAST_OPERATION_ELAPSED_MS="$LAST_OP_ELAPSED_MS" \
    SESSION_LAST_OPERATION_REMAINING_MS="$LAST_OP_REMAINING_MS" \
    SESSION_STEP="$CURRENT_STEP" SESSION_CLASS="$CURRENT_CLASS" \
    SESSION_SENDS="$CURRENT_SENDS" SESSION_REASON="$LAST_REASON" \
    SESSION_EXPECTED="$CURRENT_EXPECTED" SESSION_OBSERVED="$CURRENT_OBSERVED" \
    SESSION_ARTIFACT="$CURRENT_ARTIFACT" SESSION_ATTEMPT_DIR="$ATTEMPT_DIR" \
    SESSION_PROJECT_ROOT="$PROJECT_ROOT" SESSION_RA_DIR="$RA_DIR" \
    perl -MTime::HiRes=clock_gettime,CLOCK_MONOTONIC -e '
        use strict;
        use warnings;
        sub number {
            my ($key) = @_;
            my $value = $ENV{$key};
            if (!defined $value || $value !~ /\A[0-9]+\z/) {
                print STDERR "state_session: journal field invalid\n";
                exit 81;
            }
            return 0 + $value;
        }
        my $now = sprintf("%.0f", clock_gettime(CLOCK_MONOTONIC) * 1000);
        if ($now !~ /\A[0-9]+\z/) {
            print STDERR "state_session: journal clock observation invalid\n";
            exit 80;
        }
        $now = 0 + $now;
        my $start = number("SESSION_START_MS");
        my $budget = number("SESSION_TIMEOUT_MS");
        if ($now < $start) {
            print STDERR "state_session: journal clock moved backward\n";
            exit 80;
        }
        my $attempt_elapsed = $now - $start;
        my $attempt_remaining = $budget - $attempt_elapsed;
        $attempt_remaining = 0 if $attempt_remaining < 0;

        my ($operation_elapsed, $operation_remaining);
        if (length($ENV{SESSION_OPERATION_ACTIVE} // "") != 0) {
            my $operation_start = number("SESSION_OPERATION_START_MS");
            my $operation_deadline = number("SESSION_OPERATION_DEADLINE_MS");
            my $operation_now = length($ENV{SESSION_OPERATION_STOP_MS} // "") != 0
                ? number("SESSION_OPERATION_STOP_MS") : $now;
            if ($operation_now < $operation_start) {
                print STDERR "state_session: journal operation clock moved backward\n";
                exit 80;
            }
            $operation_elapsed = $operation_now - $operation_start;
            $operation_remaining = $operation_deadline - $operation_now;
            $operation_remaining = 0 if $operation_remaining < 0;
        } else {
            $operation_elapsed = number("SESSION_LAST_OPERATION_ELAPSED_MS");
            $operation_remaining = number("SESSION_LAST_OPERATION_REMAINING_MS");
        }

        my %text;
        for my $key (qw(EXPECTED OBSERVED ARTIFACT)) {
            my $value = $ENV{"SESSION_$key"} // "";
            $value =~ s/\Q$ENV{SESSION_ATTEMPT_DIR}\E/<attempt>/g;
            $value =~ s/\Q$ENV{SESSION_PROJECT_ROOT}\E/<workspace>/g;
            $value =~ s/\Q$ENV{SESSION_RA_DIR}\E/<retroarch>/g;
            $text{$key} = $value;
        }
        my @row = (
            $ENV{SESSION_STEP} // "", $ENV{SESSION_CLASS} // "",
            $ENV{SESSION_SENDS} // "", $attempt_elapsed, $attempt_remaining,
            $operation_elapsed, $operation_remaining, $text{EXPECTED},
            $text{OBSERVED}, $text{ARTIFACT}, $ENV{SESSION_REASON} // ""
        );
        open my $journal, ">>", $ENV{SESSION_JOURNAL} or do {
            print STDERR "state_session: journal open failed\n";
            exit 82;
        };
        binmode $journal;
        print {$journal} join("\t", @row), "\n" or do {
            print STDERR "state_session: journal write failed\n";
            exit 82;
        };
        close $journal or do {
            print STDERR "state_session: journal close failed\n";
            exit 82;
        };
    ' || _journal_status=$?
    if [ "$_journal_status" -ne 0 ]; then
        JOURNAL_FAILED=1
        [ -n "$FIRST_FAILURE" ] || FIRST_FAILURE=JOURNAL_RECORD_FAILURE
        LAST_REASON=$FIRST_FAILURE
        return 1
    fi
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
    exec 9>&-
    if [ -n "${STATE_SESSION_TEST_MODE:-}" ]; then
        exec env "STATE_SESSION_TEST_EXPECTED_DIR=$ATTEMPT_DIR/states" \
            "STATE_SESSION_TEST_SHOTS=$ATTEMPT_DIR/shots" "$NC" "$@"
    fi
    exec "$NC" "$@"
}
readiness_nc_exec() {
    exec 9>&-
    if [ -n "${STATE_SESSION_TEST_MODE:-}" ]; then
        _readiness_nc=${STATE_SESSION_TEST_READINESS_NC:-$NC}
        exec env "STATE_SESSION_TEST_EXPECTED_DIR=$ATTEMPT_DIR/states" \
            "STATE_SESSION_TEST_SHOTS=$ATTEMPT_DIR/shots" "$_readiness_nc" "$@"
    fi
    _readiness_nc=${STATE_SESSION_REAL_NC:-nc}
    exec "$_readiness_nc" "$@"
}
cleanup_quit() {
    [ -n "$HOST_PID" ] && host_alive || return 0
    [ "$QUIT_SENT" -eq 0 ] || return 0
    QUIT_SENT=1
    CURRENT_STEP=cleanup-quit; CURRENT_CLASS=QUIT; CURRENT_SENDS=0
    CURRENT_EXPECTED=QUIT-at-most-once; CURRENT_OBSERVED=cleanup-send-attempt
    if ! printf 'QUIT' >"$REQUEST"; then
        _cleanup_reason=$LAST_REASON
        CURRENT_OBSERVED=CLEANUP_REQUEST_WRITE_FAILED
        LAST_REASON=CLEANUP_REQUEST_WRITE_FAILED; CURRENT_ARTIFACT=cleanup-quit
        printf '%s\n' 'state_session: CLEANUP_REQUEST_WRITE_FAILED' >&2 || true
        record_row || true
        LAST_REASON=${FIRST_FAILURE:-$_cleanup_reason}
        return 0
    fi
    CURRENT_SENDS=1
    nc_exec -u -w0 127.0.0.1 55355 <"$REQUEST" >"$ATTEMPT_DIR/cleanup-quit.out" \
        2>"$ATTEMPT_DIR/cleanup-quit.err" &
    CHILD_PID=$!; _deadline=$(( $(clock_ms) + 300 ))
    while kill -0 "$CHILD_PID" 2>/dev/null; do
        _now=$(clock_ms) || break; [ "$_now" -lt "$_deadline" ] || break; sleep 0.02
    done
    if kill -0 "$CHILD_PID" 2>/dev/null; then reap_one "$CHILD_PID"; else wait "$CHILD_PID" 2>/dev/null || true; fi
    CHILD_PID=; LAST_REASON=${FIRST_FAILURE:-$LAST_REASON}; CURRENT_ARTIFACT=cleanup-quit; record_row || true
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
    release_owned_lock
}
stop_operation_clock() {
    [ -n "$ACTIVE_OPERATION" ] && [ -n "$OPERATION_START_MS" ] || return 0
    [ -z "$OPERATION_STOP_MS" ] || return 0
    OPERATION_STOP_NOW=$(clock_ms) || return 1
    [ "$OPERATION_STOP_NOW" -ge "$OPERATION_START_MS" ] || return 1
    OPERATION_STOP_MS=$OPERATION_STOP_NOW
}
on_signal() {
    trace_interruption_event handler-entry 0 || true
    [ -n "$FIRST_FAILURE" ] || FIRST_FAILURE="INTERRUPTED_$1"
    stop_operation_clock || true
    LAST_REASON=$FIRST_FAILURE
    _signal_row_status=0
    if [ "$JOURNAL_FAILED" -eq 0 ]; then record_row || _signal_row_status=$?; else _signal_row_status=1; fi
    if [ "$_signal_row_status" -eq 0 ] && [ "$JOURNAL_FAILED" -eq 0 ]; then
        trace_interruption_event journal-close-success 0 || true
    else
        trace_interruption_event journal-close-failed "$_signal_row_status" || true
    fi
    cleanup
    trace_interruption_event handler-exit 130 || true
    trap - INT TERM HUP
    exit 130
}
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
    ( exec 9>&-; "$@" ) <"$CHILD_INPUT" >"$CHILD_OUT" 2>"$CHILD_ERR" & CHILD_PID=$!
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
        # Keep nc's 1 s timeout inside a wider process-supervision slice; run_child also caps it
        # by live operation and attempt time, including the cleanup reserve.
        _left=$((_deadline-_now)); [ "$_left" -le "$READINESS_CHILD_MS" ] || _left=$READINESS_CHILD_MS
        _reply="$ATTEMPT_DIR/reply-$_tries.txt"; _reply_err="$ATTEMPT_DIR/readiness-$_tries.err"
        _read_status=0
        run_child readiness "$_left" "$REQUEST" "$_reply" "$_reply_err" \
            readiness_nc_exec -u -w "$READINESS_NC_TIMEOUT_SECS" 127.0.0.1 55355 || _read_status=$?
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

# Use the evidence reader's bounded native FNV-1a rather than interpreting every
# artifact byte inside the publication window. Empty and >1 MiB files still fail.
fnv64() {
    "$FRAME_INDEX" --session-hash "$1"
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
STARTUP_CAPTURES="$ATTEMPT_DIR/startup-captures.tsv"
printf 'tag\trelation\tscreenshot_send_ordinal\tdisplay_frame\tartifact_path\tartifact_identity\n' >"$STARTUP_CAPTURES"
ORIGINAL_ANCHOR_PATH=; ORIGINAL_ANCHOR_ID=; ORIGINAL_ANCHOR_FRAME=0
REWOUND_ANCHOR_PATH=; REWOUND_ANCHOR_ID=; REWOUND_ANCHOR_FRAME=0
record_startup_capture() {
    _tag=$1; _relation=$2
    _artifact_path="originals/${_tag}-${ORIGINAL_NAME}"
    printf '%s\t%s\t%s\t%s\t%s\t%s\n' \
        "$_tag" "$_relation" "$send_count" "$BOUNDARY_FRAME" "$_artifact_path" "$BOUNDARY_HASH" >>"$STARTUP_CAPTURES" || fail STARTUP_EVIDENCE_WRITE
}
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
( exec 9>&-; exec env "HOME=$ATTEMPT_DIR/profile" "CFFIXED_USER_HOME=$ATTEMPT_DIR/profile" \
    "XDG_CONFIG_HOME=$ATTEMPT_DIR/profile/.config" "XDG_DATA_HOME=$ATTEMPT_DIR/profile/.local/share" \
    "XDG_CACHE_HOME=$ATTEMPT_DIR/profile/.cache" "$RA" --verbose -c "$ATTEMPT_DIR/session.cfg" \
    -L "$CORE" "$CONTENT" ) >"$ATTEMPT_DIR/retroarch.log" 2>&1 &
HOST_PID=$!
probe_ready
sleep_ms "$SETTLE_MS" startup-settle; host_alive || fail HOST_DIED

# The cleanup-matrix reacquisition probe checks lock ownership and teardown only.
# Its primary session control covers progression, so this test-only path can stop
# after readiness without repeating captures and state-anchor validation.
if [ "${STATE_SESSION_TEST_STOP_AFTER_READY:-0}" = 1 ] && [ -n "${STATE_SESSION_TEST_MODE:-}" ]; then
    exit 0
fi

# Prove that the host was running before toggling pause. A stalled or already-paused
# startup fails closed; equal marker frames are never accepted as evidence of a pause.
capture startup-forward-01; STARTUP_FRAME_1=$BOUNDARY_FRAME
record_startup_capture startup-forward-01 baseline
capture startup-forward-02
record_startup_capture startup-forward-02 strictly-forward-from-startup-forward-01
[ "$BOUNDARY_FRAME" -gt "$STARTUP_FRAME_1" ] || fail STARTUP_NOT_ADVANCING

begin_operation pause-original "$OPERATION_MS"; send_once PAUSE_TOGGLE pause-original; sleep_ms "$COMMAND_GAP_MS" pause-settle; finish_operation

# The existing pair proves that the single toggle paused an advancing host. Save only
# after both independently published, valid captures agree on the displayed frame.
capture boundary-01; FRAME_1=$BOUNDARY_FRAME
FRAME_1_PATH=$BOUNDARY_PATH; FRAME_1_NAME=$BOUNDARY_NAME; FRAME_1_HASH=$BOUNDARY_HASH
FRAME_1_ELAPSED=$BOUNDARY_ELAPSED_MS; FRAME_1_DEADLINE=$BOUNDARY_DEADLINE_MS
sleep_ms "$PAUSE_GAP_MS" first-pause-gap
capture boundary-02
[ "$BOUNDARY_FRAME" -eq "$FRAME_1" ] || fail PAUSE_DID_NOT_HOLD
FRAME_2_PATH=$BOUNDARY_PATH; FRAME_2_NAME=$BOUNDARY_NAME; FRAME_2_HASH=$BOUNDARY_HASH
FRAME_2_ELAPSED=$BOUNDARY_ELAPSED_MS; FRAME_2_DEADLINE=$BOUNDARY_DEADLINE_MS
save_anchor original "$FRAME_1" ""
BOUNDARY_FRAME=$FRAME_1; BOUNDARY_PATH=$FRAME_1_PATH; BOUNDARY_NAME=$FRAME_1_NAME; BOUNDARY_HASH=$FRAME_1_HASH
BOUNDARY_ELAPSED_MS=$FRAME_1_ELAPSED; BOUNDARY_DEADLINE_MS=$FRAME_1_DEADLINE
record_boundary capture pause-equal-original original 1 1 "$ORIGINAL_ANCHOR_ID" "$ORIGINAL_ANCHOR_FRAME" 0 0 "$BOUNDARY_ELAPSED_MS" "$BOUNDARY_DEADLINE_MS"
verify_prefix
BOUNDARY_FRAME=$FRAME_1; BOUNDARY_PATH=$FRAME_2_PATH; BOUNDARY_NAME=$FRAME_2_NAME; BOUNDARY_HASH=$FRAME_2_HASH
BOUNDARY_ELAPSED_MS=$FRAME_2_ELAPSED; BOUNDARY_DEADLINE_MS=$FRAME_2_DEADLINE
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
