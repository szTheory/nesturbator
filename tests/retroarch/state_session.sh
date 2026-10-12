#!/bin/sh
# One paused RetroArch session driven over its network commands (13-RESEARCH flag 3, D-24, D-25
# as amended in 13-CONTEXT.md). Started by run_retroarch.cmake's state-net lane.
#
#   state_session.sh RETROARCH CFG CORE CONTENT RA_DIR LOG
#
# Commands are per-loop flags, not a queue, so every step sends one command, waits 50-100 ms and
# is confirmed by a new screenshot under a deadline. Screenshots land in RA_DIR/shot-net-<tag>.png.
# Every wait has a deadline; a missed deadline kills RetroArch and fails the session.
#
# After the run-ahead toggle the session never resumes: RetroArch drops commands while run-ahead
# is active and content is running (13-RESEARCH 3c), so QUIT is sent while still paused.
set -eu

if [ "$#" -ne 6 ]; then
    echo "usage: state_session.sh RETROARCH CFG CORE CONTENT RA_DIR LOG" >&2
    exit 2
fi
RA=$1
CFG=$2
CORE=$3
CONTENT=$4
RA_DIR=$5
LOG=$6
HOME_DIR="$RA_DIR/home"
# RetroArch names the state after the content: states/<core>/<content stem>.state.
STEM=${CONTENT##*/}
STEM=${STEM%.*}
mkdir -p "$HOME_DIR" "$RA_DIR/shots"

pid=""
trap '[ -z "$pid" ] || kill -9 "$pid" 2>/dev/null || true' EXIT

fail() {
    echo "state_session: $1" >&2
    exit 1
}

# Fire and forget: -w0 returns at once and still delivers the datagram.
cmd() {
    printf %s "$1" | nc -u -w0 127.0.0.1 55355
}

# Succeeds when a reply arrives within one second.
probe() {
    reply=$(printf %s 'GET_CONFIG_PARAM savestate_directory' | nc -u -w1 127.0.0.1 55355 || true)
    [ -n "$reply" ]
}

png_count() {
    n=0
    for f in "$RA_DIR"/shots/*.png; do
        [ -e "$f" ] && n=$((n + 1))
    done
    echo "$n"
}

# shot TAG: SCREENSHOT, then wait up to 5 s for a new png and move it to shot-net-TAG.png.
shot() {
    before=$(png_count)
    cmd SCREENSHOT
    i=0
    while [ "$(png_count)" -le "$before" ]; do
        i=$((i + 1))
        [ "$i" -le 50 ] || fail "no screenshot for $1 within 5 s"
        sleep 0.1
    done
    sleep 0.1
    for f in "$RA_DIR"/shots/*.png; do
        mv "$f" "$RA_DIR/shot-net-$1.png"
        break
    done
    rm -f "$RA_DIR"/shots/*.png
    [ -s "$RA_DIR/shot-net-$1.png" ] || fail "empty screenshot for $1"
}

state_file() {
    for f in "$RA_DIR"/states/*/"$STEM".state; do
        [ -e "$f" ] && echo "$f" && return 0
    done
    return 1
}

# Reads the file's checksum, or nothing when it does not exist.
state_sum() {
    f=$(state_file) || return 0
    cksum <"$f"
}

env "HOME=$HOME_DIR" "CFFIXED_USER_HOME=$HOME_DIR" "XDG_CONFIG_HOME=$HOME_DIR/.config" \
    "XDG_DATA_HOME=$HOME_DIR/.local/share" "XDG_CACHE_HOME=$HOME_DIR/.cache" \
    "$RA" --verbose -c "$CFG" -L "$CORE" "$CONTENT" >"$LOG" 2>&1 &
pid=$!

# 1. Ready when the command interface answers; then pause and prove it with two shots 1.1 s apart.
i=0
until probe; do
    i=$((i + 1))
    [ "$i" -le 120 ] || fail "RetroArch's command interface did not answer within 30 s"
    kill -0 "$pid" 2>/dev/null || fail "RetroArch exited before it answered"
    sleep 0.25
done
sleep 3
cmd PAUSE_TOGGLE
sleep 0.1
shot pause-a
sleep 1.1
shot pause-b

# 2. Save the state and wait for the file.
cmd SAVE_STATE
i=0
until state_file >/dev/null; do
    i=$((i + 1))
    [ "$i" -le 50 ] || fail "no state file within 5 s of SAVE_STATE"
    sleep 0.1
done
sleep 0.2

# 3. Five single-frame advances.
n=1
while [ "$n" -le 5 ]; do
    cmd FRAMEADVANCE
    sleep 0.1
    shot "adv-$n"
    n=$((n + 1))
done

# 4. Load slot 0 (a bare load is ignored while paused), then three advances. D-25 step 5 asks
# for one advance; load-1 alone is that check, load-2 and load-3 extend the chain.
cmd "LOAD_STATE_SLOT 0"
sleep 0.1
n=1
while [ "$n" -le 3 ]; do
    cmd FRAMEADVANCE
    sleep 0.1
    shot "load-$n"
    n=$((n + 1))
done

# 5. Resume with run-ahead off (commands are honoured while running), pause again, and prove it.
cmd PAUSE_TOGGLE
sleep 3
cmd PAUSE_TOGGLE
sleep 0.1
shot m1-a
sleep 1.1
shot m1-b

# 6. Forty paced rewinds. Then save at the rewound frame: RetroArch 1.22.2 ignores SAVE_STATE
# after a FRAMEADVANCE (measured), so the second save comes before the next advances.
n=0
while [ "$n" -lt 40 ]; do
    cmd REWIND
    sleep 0.05
    n=$((n + 1))
done
sleep 0.2
shot rew-0
old_sum=$(state_sum)
i=0
while [ "$(state_sum)" = "$old_sum" ]; do
    # A command is a per-loop flag that can be missed, so the save is sent again each second
    # until the file changes.
    [ $((i % 10)) -ne 0 ] || cmd SAVE_STATE
    i=$((i + 1))
    [ "$i" -le 50 ] || fail "the state file did not change within 5 s of the second SAVE_STATE"
    sleep 0.1
done
sleep 0.2
n=1
while [ "$n" -le 3 ]; do
    cmd FRAMEADVANCE
    sleep 0.1
    shot "rew-$n"
    n=$((n + 1))
done

# 7. Toggle run-ahead on while paused, load the second save, advance once. No resume follows.
cmd RUNAHEAD_TOGGLE
sleep 0.1
cmd "LOAD_STATE_SLOT 0"
sleep 0.1
cmd FRAMEADVANCE
sleep 0.1
shot ahead-1

# 8. Quit while still paused; wait for the process up to 10 s.
cmd QUIT
i=0
while kill -0 "$pid" 2>/dev/null; do
    i=$((i + 1))
    if [ "$i" -gt 100 ]; then
        kill -9 "$pid" 2>/dev/null || true
        echo "state_session: RetroArch did not exit within 10 s of QUIT" >&2
        exit 3
    fi
    sleep 0.1
done
status=0
wait "$pid" || status=$?
pid=""
exit "$status"
