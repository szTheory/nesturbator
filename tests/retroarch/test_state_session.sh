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

owner_sidecar_refusal_assess() {
    owner_assess_root=$1
    owner_assess_status=$2
    owner_assess_pid=$(cat "$owner_assess_root/driver.pid" 2>/dev/null || true)
    owner_assess_clock=$(cat "$owner_assess_root/clock.ms" 2>/dev/null || true)
    owner_assess_path=$(cat "$owner_assess_root/owner.path" 2>/dev/null || true)
    owner_assess_expected="$owner_assess_root/locks/nesturbator-retroarch-55355.lock.owner.attempt-${owner_assess_clock}-${owner_assess_pid}"
    OWNER_COLLISION_FAILURE=
    OWNER_COLLISION_OBSERVATION=
    if [ -z "$owner_assess_pid" ] || [ -z "$owner_assess_clock" ] ||
        [ "$owner_assess_path" != "$owner_assess_expected" ] ||
        [ ! -s "$owner_assess_root/owner.identity.before" ] ||
        [ ! -f "$owner_assess_root/driver.out" ]; then
        OWNER_COLLISION_FAILURE=owner-collision-exact-path-not-established
        return 1
    fi

    if [ "$owner_assess_status" -eq 1 ] && awk \
        -v driver="$DRIVER" -v path="$owner_assess_expected" '
        $0 == "state_session: LOCK_OWNER_UNAVAILABLE" { refusal++; next }
        NF {
            if (index($0, driver ": line ") == 1 &&
                index($0, ": " path ": cannot overwrite existing file") > 0) shell_error++
            else other++
        }
        END { exit refusal == 1 && shell_error <= 1 && other == 0 ? 0 : 1 }
    ' "$owner_assess_root/driver.out"; then
        OWNER_COLLISION_OBSERVATION=LOCK_OWNER_UNAVAILABLE
    elif [ "$owner_assess_status" -eq 2 ] && awk \
        -v driver="$DRIVER" -v path="$owner_assess_expected" '
        NF {
            if (index($0, driver ": ") != 1 ||
                index($0, ": cannot create " path ": File exists") == 0) other++
            refusal++
        }
        END { exit refusal == 1 && other == 0 ? 0 : 1 }
    ' "$owner_assess_root/driver.out"; then
        OWNER_COLLISION_OBSERVATION=PORTABLE_SHELL_OCCUPIED_PATH_REFUSAL
    else
        OWNER_COLLISION_FAILURE=actual-driver-refusal-not-proven
        return 1
    fi

    if [ -e "$owner_assess_root/host-launched" ] ||
        [ -e "$owner_assess_root/transport-launched" ]; then
        OWNER_COLLISION_FAILURE=host-or-transport-launched-before-refusal
        return 1
    fi
    if [ ! -f "$owner_assess_path" ] || [ -L "$owner_assess_path" ] ||
        ! cmp -s "$owner_assess_root/sentinel.expected" "$owner_assess_path" ||
        ! "$selftest_owner_real_perl" -e 'my @s = lstat($ARGV[0]) or exit 1; print join("\t", @s[0,1,2,3,7]), "\n"' \
            "$owner_assess_path" >"$owner_assess_root/owner.identity.after" ||
        ! cmp -s "$owner_assess_root/owner.identity.before" "$owner_assess_root/owner.identity.after"; then
        OWNER_COLLISION_FAILURE=foreign-owner-object-not-preserved
        return 1
    fi
    return 0
}

fake_nc() {
    request=$(cat)
    printf '%s\n' "$request" >>"$STATE_SESSION_TEST_SENDS"
    if [ -n "${STATE_SESSION_TEST_LOCK_SNAPSHOT_FILE:-}" ] && [ ! -e "$STATE_SESSION_TEST_LOCK_SNAPSHOT_FILE" ]; then
        CLEANUP_LOCK_ROOT="${STATE_SESSION_LOCK_ROOT:?}" python3 - "$STATE_SESSION_TEST_LOCK_SNAPSHOT_FILE" <<'PY'
import json, os, pathlib, sys
path=pathlib.Path(os.environ["CLEANUP_LOCK_ROOT"])/"nesturbator-retroarch-55355.lock"
st=path.lstat()
record={"device":st.st_dev,"inode":st.st_ino,"token":path.read_text().strip()}
pathlib.Path(sys.argv[1]).write_text(json.dumps(record,sort_keys=True)+"\n")
PY
    fi
    case "$request" in
        'GET_CONFIG_PARAM savestate_directory')
            [ -z "${STATE_SESSION_TEST_READINESS_SENDS:-}" ] ||
                printf '%s\n' "$request" >>"$STATE_SESSION_TEST_READINESS_SENDS"
            ;;
    esac
    fault=${STATE_SESSION_TEST_FAULT:-}
    if [ "$fault" = dropped-quit ] && [ -n "${STATE_SESSION_TEST_CASE_ROOT:-}" ] && [ "$request" = QUIT ]; then
        cleanup_root=$STATE_SESSION_TEST_CASE_ROOT
        cleanup_case=$STATE_SESSION_TEST_CASE_ID
        cleanup_invocation=$STATE_SESSION_TEST_INVOCATION_ID
        cleanup_request=${STATE_SESSION_TEST_EXPECTED_DIR%/states}/request.txt
        printf '%s' "$request" >"$cleanup_root/consumed.request"
        : >"$cleanup_root/reply.raw"
        cleanup_driver_pid=$(cat "${STATE_SESSION_TEST_DRIVER_PID_FILE:?}")
        CLEANUP_CASE_ROOT="$cleanup_root" CLEANUP_CASE_ID="$cleanup_case" \
        CLEANUP_INVOCATION_ID="$cleanup_invocation" CLEANUP_DRIVER_PID="$cleanup_driver_pid" \
        CLEANUP_TRANSPORT_PID="$$" CLEANUP_REQUEST_PATH="$cleanup_request" \
        CLEANUP_HOST_PID_FILE="$STATE_SESSION_TEST_HOST_PID" CLEANUP_LOCK_ROOT="${STATE_SESSION_LOCK_ROOT:?}" \
        python3 - "$cleanup_root/control.json" <<'PY'
import json, os, pathlib, sys
root=pathlib.Path(os.environ["CLEANUP_CASE_ROOT"]).resolve()
request=pathlib.Path(os.environ["CLEANUP_REQUEST_PATH"]).resolve()
hostfile=pathlib.Path(os.environ["CLEANUP_HOST_PID_FILE"]); host=int(hostfile.read_text().strip()) if hostfile.is_file() else 0
try: os.kill(host,0); host_live=host>1
except (ProcessLookupError,PermissionError,OSError): host_live=False
transport=int(os.environ["CLEANUP_TRANSPORT_PID"])
try: os.kill(transport,0); receive_live=True
except (ProcessLookupError,PermissionError,OSError): receive_live=False
lock=pathlib.Path(os.environ["CLEANUP_LOCK_ROOT"])/"nesturbator-retroarch-55355.lock"
try:
 st=lock.lstat(); lock_before={"device":st.st_dev,"inode":st.st_ino,"token":lock.read_text().strip()}
except OSError: lock_before=None
record={"case_id":os.environ["CLEANUP_CASE_ID"],"invocation_id":os.environ["CLEANUP_INVOCATION_ID"],
 "case_root":"cases/"+os.environ["CLEANUP_CASE_ID"],"producer":"fake-nc","mode":"dropped-quit",
 "driver_pid":int(os.environ["CLEANUP_DRIVER_PID"]),"transport_pid":int(os.environ["CLEANUP_TRANSPORT_PID"]),
 "host_pid":host if host>1 else None,"host_live_before_injection":host_live,
 "owned_live_before_injection":host_live and receive_live,"receive_live_before_injection":receive_live,
 "owned_lock_before_injection":lock_before,
 "request_path":request.relative_to(root).as_posix(),"request_before":"regular","request_after":"regular",
 "request_writable":True,"marker":os.environ["CLEANUP_INVOCATION_ID"]+":dropped-quit",
 "consumed_file":"cases/"+os.environ["CLEANUP_CASE_ID"]+"/consumed.request",
 "reply_file":"cases/"+os.environ["CLEANUP_CASE_ID"]+"/reply.raw","signal":None}
pathlib.Path(sys.argv[1]).write_text(json.dumps(record,sort_keys=True)+"\n")
PY
    fi
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
            if [ "$fault" = cleanup-request-failure ]; then
                cleanup_root=${STATE_SESSION_TEST_CASE_ROOT:?}
                cleanup_case=${STATE_SESSION_TEST_CASE_ID:?}
                cleanup_invocation=${STATE_SESSION_TEST_INVOCATION_ID:?}
                cleanup_request=${STATE_SESSION_TEST_EXPECTED_DIR%/states}/request.txt
                [ -f "$cleanup_request" ] && [ ! -L "$cleanup_request" ] || exit 95
                cleanup_before=$(CLEANUP_REQUEST_PATH="$cleanup_request" python3 - <<'PY'
import os, pathlib, stat
p=pathlib.Path(os.environ["CLEANUP_REQUEST_PATH"]); mode=p.lstat().st_mode
if not stat.S_ISREG(mode) or p.is_symlink(): raise SystemExit("request was not a regular file before injection")
print("regular")
PY
                ) || exit 96
                printf '%s' "$request" >"$cleanup_root/consumed.request"
                cleanup_mode=request-directory
                cleanup_marker="$cleanup_invocation:$cleanup_mode"
                case "$cleanup_case" in
                    exit-failure|signal-INT|signal-TERM|signal-HUP) ;;
                    writable-cleanup|foreign-owner-replacement)
                        cleanup_mode=malformed-readiness
                        cleanup_marker="$cleanup_invocation:$cleanup_mode"
                        ;;
                    *) exit 96 ;;
                esac
                cleanup_reply="GET_CONFIG_PARAM savestate_directory /wrong/$cleanup_invocation"
                case "$cleanup_case" in
                    signal-INT|signal-TERM|signal-HUP) : >"$cleanup_root/reply.raw" ;;
                    *) printf '%s\n' "$cleanup_reply" >"$cleanup_root/reply.raw" ;;
                esac
                if [ "$cleanup_case" = foreign-owner-replacement ]; then
                    set -- "${STATE_SESSION_LOCK_ROOT:?}"/nesturbator-retroarch-55355.lock.owner.*
                    cleanup_owner=$1
                    [ -f "$cleanup_owner" ] && [ ! -L "$cleanup_owner" ] || exit 99
                    CLEANUP_OWNER_PATH="$cleanup_owner" CLEANUP_CASE_ROOT="$cleanup_root" \
                    python3 - <<'PY'
import json, os, pathlib
owner=pathlib.Path(os.environ["CLEANUP_OWNER_PATH"])
st=owner.lstat()
record={"device":st.st_dev,"inode":st.st_ino,"token":owner.read_text().strip()}
root=pathlib.Path(os.environ["CLEANUP_CASE_ROOT"])
(root/"owned-sidecar-original.json").write_text(json.dumps(record,sort_keys=True)+"\n")
PY
                    cp "$cleanup_root/foreign.sentinel" "$cleanup_root/foreign-owner.tmp" || exit 99
                    mv "$cleanup_root/foreign-owner.tmp" "$cleanup_owner" || exit 99
                    CLEANUP_OWNER_PATH="$cleanup_owner" CLEANUP_CASE_ROOT="$cleanup_root" \
                    CLEANUP_FOREIGN_PID_FILE="$cleanup_root/foreign.pid" \
                    python3 - <<'PY'
import json, os, pathlib, shutil
owner=pathlib.Path(os.environ["CLEANUP_OWNER_PATH"])
st=owner.lstat(); pid=int(pathlib.Path(os.environ["CLEANUP_FOREIGN_PID_FILE"]).read_text().strip())
try: os.kill(pid,0); live=True
except (ProcessLookupError,PermissionError,OSError): live=False
root=pathlib.Path(os.environ["CLEANUP_CASE_ROOT"])
record={"pid":pid,"live":live,"lock":{"device":st.st_dev,"inode":st.st_ino,"token":owner.read_text().strip()}}
(root/"foreign-before.json").write_text(json.dumps(record,sort_keys=True)+"\n")
shutil.copyfile(owner,root/"foreign-bytes-before")
PY
                fi
                if [ "$cleanup_mode" = request-directory ]; then
                    rm "$cleanup_request" && mkdir "$cleanup_request" || exit 97
                fi
                cleanup_transport_pid=$$
                cleanup_driver_pid=
                cleanup_wait=0
                while [ ! -s "${STATE_SESSION_TEST_DRIVER_PID_FILE:?}" ] && [ "$cleanup_wait" -lt 100 ]; do
                    sleep 0.01
                    cleanup_wait=$((cleanup_wait+1))
                done
                [ -s "$STATE_SESSION_TEST_DRIVER_PID_FILE" ] || exit 98
                cleanup_driver_pid=$(cat "$STATE_SESSION_TEST_DRIVER_PID_FILE")
                CLEANUP_REQUEST_PATH="$cleanup_request" python3 - <<'PY' >"$cleanup_root/request-after.json"
import json, os, pathlib, stat
p=pathlib.Path(os.environ["CLEANUP_REQUEST_PATH"]); mode=p.lstat().st_mode
kind="regular" if stat.S_ISREG(mode) and not p.is_symlink() else "directory" if stat.S_ISDIR(mode) else "other"
writable=kind=="regular" and os.access(p,os.W_OK)
print(json.dumps({"kind":kind,"writable":writable},sort_keys=True))
PY
                cleanup_after=$(CLEANUP_REQUEST_AFTER_FILE="$cleanup_root/request-after.json" python3 - <<'PY'
import json, os, pathlib
print(json.loads(pathlib.Path(os.environ["CLEANUP_REQUEST_AFTER_FILE"]).read_text())["kind"])
PY
                ) || exit 96
                cleanup_writable=$(CLEANUP_REQUEST_AFTER_FILE="$cleanup_root/request-after.json" python3 - <<'PY'
import json, os, pathlib
print("true" if json.loads(pathlib.Path(os.environ["CLEANUP_REQUEST_AFTER_FILE"]).read_text())["writable"] else "false")
PY
                ) || exit 96
                CLEANUP_CASE_ROOT="$cleanup_root" CLEANUP_CASE_ID="$cleanup_case" \
                CLEANUP_INVOCATION_ID="$cleanup_invocation" CLEANUP_DRIVER_PID="$cleanup_driver_pid" \
                CLEANUP_TRANSPORT_PID="$cleanup_transport_pid" CLEANUP_MODE="$cleanup_mode" \
                CLEANUP_MARKER="$cleanup_marker" CLEANUP_REQUEST_PATH="$cleanup_request" \
                CLEANUP_REQUEST_BEFORE="$cleanup_before" CLEANUP_REQUEST_AFTER="$cleanup_after" \
                CLEANUP_REQUEST_WRITABLE="$cleanup_writable" \
                CLEANUP_HOST_PID_FILE="$STATE_SESSION_TEST_HOST_PID" \
                CLEANUP_LOCK_ROOT="${STATE_SESSION_LOCK_ROOT:?}" \
                python3 - "$cleanup_root/control.json" <<'PY'
import json, os, pathlib, sys
root = pathlib.Path(os.environ["CLEANUP_CASE_ROOT"]).resolve()
request = pathlib.Path(os.environ["CLEANUP_REQUEST_PATH"]).resolve()
host_file = pathlib.Path(os.environ["CLEANUP_HOST_PID_FILE"])
host_pid = int(host_file.read_text().strip()) if host_file.is_file() else 0
try: os.kill(host_pid, 0); host_live = host_pid > 1
except (ProcessLookupError, PermissionError, OSError): host_live = False
transport_pid=int(os.environ["CLEANUP_TRANSPORT_PID"])
try: os.kill(transport_pid,0); transport_live=True
except (ProcessLookupError, PermissionError, OSError): transport_live=False
lock = pathlib.Path(os.environ["CLEANUP_LOCK_ROOT"]) / "nesturbator-retroarch-55355.lock"
try:
    st = lock.lstat()
    lock_before = {"device": st.st_dev, "inode": st.st_ino, "token": lock.read_text().strip()}
except OSError: lock_before = None
record = {
    "case_id": os.environ["CLEANUP_CASE_ID"],
    "invocation_id": os.environ["CLEANUP_INVOCATION_ID"],
    "case_root": "cases/" + os.environ["CLEANUP_CASE_ID"],
    "producer": "fake-nc",
    "mode": os.environ["CLEANUP_MODE"],
    "driver_pid": int(os.environ["CLEANUP_DRIVER_PID"]),
    "transport_pid": int(os.environ["CLEANUP_TRANSPORT_PID"]),
    "host_pid": host_pid if host_pid > 1 else None,
    "host_live_before_injection": host_live,
    "owned_live_before_injection": host_live and transport_live,
    "receive_live_before_injection": transport_live,
    "owned_lock_before_injection": lock_before,
    "request_path": request.relative_to(root).as_posix(),
    "request_before": os.environ["CLEANUP_REQUEST_BEFORE"],
    "request_after": os.environ["CLEANUP_REQUEST_AFTER"],
    "request_writable": os.environ["CLEANUP_REQUEST_WRITABLE"] == "true",
    "marker": os.environ["CLEANUP_MARKER"],
    "consumed_file": "cases/" + os.environ["CLEANUP_CASE_ID"] + "/consumed.request",
    "reply_file": "cases/" + os.environ["CLEANUP_CASE_ID"] + "/reply.raw",
    "signal": None,
}
pathlib.Path(sys.argv[1]).write_text(json.dumps(record, sort_keys=True) + "\n")
PY
                printf '%s\n' "$cleanup_transport_pid" >"$cleanup_root/receive.pid"
                case "$cleanup_case" in
                    signal-INT|signal-TERM|signal-HUP)
                        printf '%s\n' "$cleanup_marker" >"$cleanup_root/armed"
                        exec perl -MTime::HiRes=sleep -e 'sleep 60'
                        ;;
                    *) printf '%s\n' "$cleanup_reply"; exit 0 ;;
                esac
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
    all|readiness|interruption|cleanup-request-failure|interruption-capture-selftest|interruption-trace-lock-selftest) ;;
    *) echo "state session controls: invalid STATE_SESSION_CONTROL=$CONTROL" >&2; exit 2 ;;
esac
if [ "$CONTROL" = cleanup-request-failure ]; then
    mkdir -p "$(dirname "$WORK_ROOT")" || exit 2
    if ! mkdir "$WORK_ROOT" 2>/dev/null; then
        echo 'state session cleanup controls: evidence root already exists; refusing re-entry' >&2
        exit 2
    fi
    WORK_ROOT=$(cd "$WORK_ROOT" && pwd -P)
    CLEANUP_MATRIX_KIND=$(basename "$WORK_ROOT")
    exec >"$WORK_ROOT/stdout.log" 2>"$WORK_ROOT/stderr.log"
fi

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

selftest_identity_event_skeleton() {
    selftest_identity_file=$1
    awk -F '\t' '
        BEGIN {
            split("before-assertion capture-start snapshot-pre-start snapshot-pre-end assertion-start assertion-end snapshot-post-start snapshot-post-end capture-end assertion-decision", expected, " ")
        }
        NR > 10 { bad = 1 }
        {
            if ($1 == "" || $2 != "harness" || $3 != NR || $4 !~ /^[0-9]+$/ ||
                $5 != expected[NR] || $6 !~ /^[0-9]+$/ || $7 !~ /^[0-9]+$/) bad = 1
            if (NR > 1 && $4 < previous) bad = 1
            previous = $4
        }
        END { if (NR != 10 || bad) exit 1 }
    ' "$selftest_identity_file"
}

selftest_identity_rows_owned() {
    selftest_identity_file=$1
    selftest_identity_expected=$2
    [ "$(awk -F '\t' -v wanted="$selftest_identity_expected" '$1 == wanted { n++ } END { print n + 0 }' \
        "$selftest_identity_file")" -eq 10 ]
}

selftest_identity_rejects() {
    selftest_identity_file=$1
    selftest_identity_expected=$2
    [ -f "$selftest_identity_file" ] && selftest_identity_event_skeleton "$selftest_identity_file" || return 1
    selftest_identity_status=0
    verify_interruption_event_file "$selftest_identity_file" "$selftest_identity_expected" ||
        selftest_identity_status=$?
    [ "$selftest_identity_status" -eq 1 ]
}

selftest_identity_nonid_equal() {
    cut -f2- "$1" >"$selftest_root/identity/original-nonidentity.tsv"
    cut -f2- "$2" >"$selftest_root/identity/copy-nonidentity.tsv"
    cmp -s "$selftest_root/identity/original-nonidentity.tsv" \
        "$selftest_root/identity/copy-nonidentity.tsv"
}

run_interruption_identity_case() {
    selftest_identity_name=$1
    selftest_identity_expected=$2
    selftest_identity_dir="$selftest_root/identity/$selftest_identity_name/capture"
    mkdir -p "$selftest_identity_dir"
    printf 'step\tclass\treason\nINTERRUPTED_TERM\tsignal\tTERM\n' \
        >"$selftest_identity_dir/journal.tsv"
    printf 'INTERRUPTED_TERM\tsignal\tTERM\n' >"$selftest_identity_dir/expected.stdout"
    : >"$selftest_identity_dir/expected.stderr"
    INTERRUPTION_EVENT_SEQUENCE=0
    INTERRUPTION_RECORDER_STATUS=0
    INTERRUPTION_RECORDER_REASON=
    INTERRUPTION_ASSERTION_STATUS=2
    selftest_identity_boundary_status=0
    capture_interruption_boundary "$selftest_identity_dir/journal.tsv" \
        "$selftest_identity_dir" "$selftest_identity_expected" "$$" /dev/null ||
        selftest_identity_boundary_status=$?

    [ "$selftest_identity_boundary_status" -eq 0 ] &&
        [ "$INTERRUPTION_ASSERTION_STATUS" -eq 0 ] &&
        [ "$INTERRUPTION_RECORDER_STATUS" -eq 0 ] &&
        cmp -s "$selftest_identity_dir/expected.stdout" "$selftest_identity_dir/assertion.stdout" &&
        cmp -s "$selftest_identity_dir/expected.stderr" "$selftest_identity_dir/assertion.stderr" &&
        cmp -s "$selftest_identity_dir/journal.tsv" "$selftest_identity_dir/snapshot-pre.bin" &&
        cmp -s "$selftest_identity_dir/journal.tsv" "$selftest_identity_dir/snapshot-post.bin" &&
        verify_interruption_snapshots "$selftest_identity_dir/snapshot-metadata.tsv" &&
        selftest_identity_event_skeleton "$selftest_identity_dir/events.tsv" || {
        echo "FAIL nested capture identity prerequisites for $selftest_identity_expected" >&2
        return 1
    }

    selftest_identity_validation_status=0
    verify_interruption_event_file "$selftest_identity_dir/events.tsv" \
        "$selftest_identity_expected" || selftest_identity_validation_status=$?
    if [ "$selftest_identity_validation_status" -ne 0 ]; then
        selftest_identity_owned_rows=$(awk -F '\t' -v wanted="$selftest_identity_expected" \
            '$1 == wanted { n++ } END { print n + 0 }' "$selftest_identity_dir/events.tsv")
        selftest_identity_capture_rows=$(awk -F '\t' '$1 == "capture" { n++ } END { print n + 0 }' \
            "$selftest_identity_dir/events.tsv")
        [ "$selftest_identity_validation_status" -eq 1 ] &&
            [ "$selftest_identity_owned_rows" -eq 2 ] &&
            [ "$selftest_identity_capture_rows" -eq 8 ] || {
            echo "FAIL nested capture identity validator rejected a non-identity condition" >&2
            return 1
        }
        printf 'TAP version 13\n1..1\nnot ok 1 - nested-capture-session-identity\n'
        printf '# expected 10 events owned by %s; got 2 owned outer rows and 8 capture-basename rows\n' \
            "$selftest_identity_expected"
        return 1
    fi

    selftest_identity_rows_owned "$selftest_identity_dir/events.tsv" \
        "$selftest_identity_expected" || {
        echo "FAIL nested capture identity accepted without every row carrying $selftest_identity_expected" >&2
        return 1
    }
    printf 'identity case %s expected=%s path=%s rows=10 validator=accepted\n' \
        "$selftest_identity_name" "$selftest_identity_expected" "$selftest_identity_name/capture"
}

run_interruption_identity_selftests() {
    selftest_root=$1
    selftest_identity_root="$selftest_root/identity"
    mkdir -p "$selftest_identity_root"

    run_interruption_identity_case session-A session-A-owned
    run_interruption_identity_case session-B session-B-owned

    selftest_identity_invalid="$selftest_identity_root/invalid-owned-id/capture"
    mkdir -p "$selftest_identity_invalid"
    printf 'step\tclass\treason\nINTERRUPTED_TERM\tsignal\tTERM\n' \
        >"$selftest_identity_invalid/journal.tsv"
    printf 'INTERRUPTED_TERM\tsignal\tTERM\n' >"$selftest_identity_invalid/expected.stdout"
    : >"$selftest_identity_invalid/expected.stderr"
    INTERRUPTION_EVENT_SEQUENCE=0
    INTERRUPTION_RECORDER_STATUS=0
    INTERRUPTION_RECORDER_REASON=
    selftest_identity_invalid_boundary_status=0
    capture_interruption_boundary "$selftest_identity_invalid/journal.tsv" \
        "$selftest_identity_invalid" '' "$$" /dev/null ||
        selftest_identity_invalid_boundary_status=$?
    [ "$selftest_identity_invalid_boundary_status" -eq 0 ] &&
        [ "$INTERRUPTION_ASSERTION_STATUS" -eq 0 ] &&
        [ "$INTERRUPTION_RECORDER_STATUS" -eq 1 ] &&
        [ "$INTERRUPTION_RECORDER_REASON" = session-id-invalid ] &&
        cmp -s "$selftest_identity_invalid/expected.stdout" \
            "$selftest_identity_invalid/assertion.stdout" &&
        cmp -s "$selftest_identity_invalid/expected.stderr" \
            "$selftest_identity_invalid/assertion.stderr" &&
        grep -F 'session-id-invalid' "$selftest_identity_invalid/recorder-errors.tsv" >/dev/null || {
        echo 'FAIL invalid session identity replaced the genuine assertion result' >&2; return 1;
    }
    printf 'identity invalid-owned-id: recorder failed separately; assertion status=0\n'

    selftest_identity_a="$selftest_identity_root/session-A/capture/events.tsv"
    selftest_identity_b="$selftest_identity_root/session-B/capture/events.tsv"
    selftest_identity_negative="$selftest_identity_root/negative"
    mkdir -p "$selftest_identity_negative"

    selftest_identity_rejects "$selftest_identity_a" session-A-wrong-expected || {
        echo 'FAIL wrong expected session ID was accepted' >&2; return 1;
    }
    printf 'identity negative wrong-expected-session: rejected after valid positive ledger\n'

    awk -F '\t' -v OFS='\t' 'NR == 5 { $1 = "mixed-inner" } { print }' \
        "$selftest_identity_a" >"$selftest_identity_negative/mixed-inner.copy.tsv"
    mv "$selftest_identity_negative/mixed-inner.copy.tsv" "$selftest_identity_negative/mixed-inner.tsv"
    selftest_identity_nonid_equal "$selftest_identity_a" \
        "$selftest_identity_negative/mixed-inner.tsv" &&
        selftest_identity_rejects "$selftest_identity_negative/mixed-inner.tsv" session-A-owned || {
        echo 'FAIL mixed internal event identity was accepted or changed another field' >&2; return 1;
    }
    printf 'identity negative mixed-inner-row: rejected with other fields unchanged\n'

    awk -F '\t' -v OFS='\t' 'NR == 1 { $1 = "mixed-outer" } { print }' \
        "$selftest_identity_a" >"$selftest_identity_negative/mixed-outer.tsv"
    selftest_identity_nonid_equal "$selftest_identity_a" \
        "$selftest_identity_negative/mixed-outer.tsv" &&
        selftest_identity_rejects "$selftest_identity_negative/mixed-outer.tsv" session-A-owned || {
        echo 'FAIL mixed outer event identity was accepted or changed another field' >&2; return 1;
    }
    printf 'identity negative mixed-outer-row: rejected with other fields unchanged\n'

    selftest_identity_reuse="$selftest_identity_root/reused-as-session-B/capture"
    mkdir -p "$selftest_identity_reuse"
    cp "$selftest_identity_a" "$selftest_identity_reuse/events.tsv"
    selftest_identity_nonid_equal "$selftest_identity_a" \
        "$selftest_identity_reuse/events.tsv" &&
        selftest_identity_rejects "$selftest_identity_reuse/events.tsv" session-B-owned || {
        echo 'FAIL other-session ledger reuse was accepted or changed event fields' >&2; return 1;
    }
    printf 'identity negative cross-session-ledger-reuse: rejected with original rows intact\n'
    printf 'state session interruption identity ledger: 2 positive sessions, 4 ownership negatives passed\n'
}

selftest_write_trace_lifecycle() {
    trace_writer_base=$1
    trace_writer_id=$2
    RA_DIR="$trace_writer_base/ra"
    LOCK_DIR="$trace_writer_base/locks/driver.lock"
    mkdir -p "$RA_DIR" "${LOCK_DIR%/*}" || return 1
    mkdir "$LOCK_DIR" || return 1
    STATE_SESSION_TEST_MODE=prefix
    STATE_SESSION_TEST_INTERRUPT_INVOCATION=$trace_writer_id
    STATE_SESSION_TEST_INTERRUPT_TRACE="$RA_DIR/interruption-events.tsv"
    STATE_SESSION_TEST_INTERRUPT_TRACE_ERRORS="$RA_DIR/interruption-trace-errors.tsv"
    if [ "${3:-}" = absent ]; then
        HOST_PID=
        CHILD_PID=
    else
        HOST_PID=$$
        CHILD_PID=$$
    fi
    FIRST_FAILURE=
    QUIT_SENT=0
    JOURNAL_FAILED=0
    send_count=0
    INTERRUPTION_TRACE_SEQUENCE=0
    INTERRUPTION_TRACE_FAILED=0
    trace_interruption_event handler-entry 0 || return 1
    FIRST_FAILURE=INTERRUPTED_TERM
    trace_interruption_event journal-close-success 0 || return 1
    HOST_PID=
    CHILD_PID=
    QUIT_SENT=1
    rmdir "$LOCK_DIR" || return 1
    trace_interruption_event handler-exit 130 || return 1
    [ "$INTERRUPTION_TRACE_FAILED" -eq 0 ]
}

selftest_write_trace_regular_lock_lifecycle() {
    trace_regular_base=$1
    trace_regular_id=$2
    RA_DIR="$trace_regular_base/ra"
    trace_regular_lock_root="$trace_regular_base/locks"
    LOCK_DIR="$trace_regular_lock_root/driver.lock"
    trace_regular_owner="$trace_regular_lock_root/driver.lock.owner.$trace_regular_id"
    mkdir -p "$RA_DIR" "$trace_regular_lock_root" || return 1
    [ ! -e "$trace_regular_owner" ] && [ ! -L "$trace_regular_owner" ] || return 1
    printf '%s\n' "$trace_regular_id" >"$trace_regular_owner" || return 1
    ln "$trace_regular_owner" "$LOCK_DIR" || return 1
    [ -f "$LOCK_DIR" ] && [ ! -L "$LOCK_DIR" ] &&
        [ "$(cat "$LOCK_DIR")" = "$trace_regular_id" ] || return 1
    perl -e '
        use strict;
        use warnings;
        my ($owner, $lock) = @ARGV;
        my @owner = stat($owner);
        my @lock = stat($lock);
        exit 1 unless @owner && @lock && $owner[0] == $lock[0] && $owner[1] == $lock[1];
    ' "$trace_regular_owner" "$LOCK_DIR" || return 1

    STATE_SESSION_TEST_MODE=prefix
    STATE_SESSION_TEST_INTERRUPT_INVOCATION=$trace_regular_id
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
    trace_interruption_event handler-entry 0 || return 1
    [ -f "$LOCK_DIR" ] && [ ! -L "$LOCK_DIR" ] &&
        [ "$(cat "$LOCK_DIR")" = "$trace_regular_id" ] || return 1
    FIRST_FAILURE=INTERRUPTED_TERM
    trace_interruption_event journal-close-success 0 || return 1
    [ -f "$LOCK_DIR" ] && [ ! -L "$LOCK_DIR" ] &&
        [ "$(cat "$LOCK_DIR")" = "$trace_regular_id" ] || return 1
    HOST_PID=
    CHILD_PID=
    QUIT_SENT=1
    rm -f "$LOCK_DIR" "$trace_regular_owner" || return 1
    [ ! -e "$LOCK_DIR" ] && [ ! -L "$LOCK_DIR" ] || return 1
    trace_interruption_event handler-exit 130 || return 1
    [ "$INTERRUPTION_TRACE_FAILED" -eq 0 ]
}

selftest_trace_edit() {
    trace_edit_file=$1
    trace_edit_row=$2
    trace_edit_action=$3
    trace_edit_column=${4:-0}
    trace_edit_value=${5:-}
    perl - "$trace_edit_file" "$trace_edit_row" "$trace_edit_action" \
        "$trace_edit_column" "$trace_edit_value" <<'PERL'
use strict;
use warnings;
my ($path, $row_index, $action, $column, $value) = @ARGV;
open my $input, "<:raw", $path or die "open fixture: $!";
local $/;
my $bytes = <$input>;
close $input or die "close fixture: $!";
my @rows = split /\n/, $bytes, -1;
my $had_newline = @rows > 1 && $rows[-1] eq "";
pop @rows if $had_newline;
if ($action eq "empty") {
    @rows = ();
    $had_newline = 0;
} elsif ($action eq "remove-row") {
    splice @rows, $row_index - 1, 1;
} elsif ($action eq "append-row") {
    push @rows, $rows[-1];
} elsif ($action eq "no-final-newline") {
    $had_newline = 0;
} else {
    die "invalid row index" if $row_index < 1 || $row_index > @rows;
    my @fields = split /\t/, $rows[$row_index - 1], -1;
    if ($action eq "set") {
        die "invalid field index" if $column < 1 || $column > @fields;
        $fields[$column - 1] = $value;
    } elsif ($action eq "drop-field") {
        pop @fields;
    } elsif ($action eq "extra-field") {
        push @fields, "unexpected";
    } else {
        die "invalid action";
    }
    $rows[$row_index - 1] = join("\t", @fields);
}
my $output = join("\n", @rows);
$output .= "\n" if $had_newline;
open my $out, ">:raw", $path or die "open output: $!";
print {$out} $output or die "write output: $!";
close $out or die "close output: $!";
PERL
}

selftest_trace_only_field_changed() {
    perl - "$1" "$2" "$3" "$4" <<'PERL'
use strict;
use warnings;
my ($original_path, $mutant_path, $target_row, $target_column) = @ARGV;
sub read_rows {
    my ($path) = @_;
    open my $input, "<:raw", $path or die "open trace: $!";
    local $/;
    my $bytes = <$input>;
    close $input or die "close trace: $!";
    return undef if !defined($bytes) || $bytes !~ /\n\z/;
    my @rows = split /\n/, $bytes, -1;
    pop @rows;
    return \@rows;
}
my $original = read_rows($original_path);
my $mutant = read_rows($mutant_path);
exit 1 unless defined($original) && defined($mutant) && @$original == @$mutant;
my $changed = 0;
for my $row (0 .. $#$original) {
    my @left = split /\t/, $original->[$row], -1;
    my @right = split /\t/, $mutant->[$row], -1;
    exit 1 unless @left == 16 && @right == 16;
    if ($row == $target_row - 1) {
        exit 1 if $left[$target_column - 1] eq $right[$target_column - 1];
        $changed = 1;
    } else {
        exit 1 unless $original->[$row] eq $mutant->[$row];
    }
    splice @left, $target_column - 1, 1;
    splice @right, $target_column - 1, 1;
    exit 1 unless join("\t", @left) eq join("\t", @right);
}
exit($changed ? 0 : 1);
PERL
}

selftest_trace_expect() {
    trace_expect_file=$1
    trace_expect_id=$2
    trace_expect_driver=$3
    trace_expect_result=$4
    trace_expect_name=$5
    trace_expect_status=0
    trace_expect_actual=$(verify_interruption_driver_trace \
        "$trace_expect_file" "$trace_expect_id" "$trace_expect_driver") ||
        trace_expect_status=$?
    if [ "$trace_expect_result" = TRACE_OK ]; then
        [ "$trace_expect_status" -eq 0 ] && [ "$trace_expect_actual" = TRACE_OK ] || {
            echo "FAIL trace selftest $trace_expect_name: expected TRACE_OK, got $trace_expect_actual (status $trace_expect_status)" >&2
            return 1
        }
    else
        [ "$trace_expect_status" -ne 0 ] && [ "$trace_expect_actual" = "$trace_expect_result" ] || {
            echo "FAIL trace selftest $trace_expect_name: expected $trace_expect_result, got $trace_expect_actual (status $trace_expect_status)" >&2
            return 1
        }
    fi
    trace_selftest_count=$((trace_selftest_count + 1))
    printf 'trace selftest: PASS %s (%s)\n' "$trace_expect_name" "$trace_expect_result"
}

selftest_trace_expect_edit() {
    trace_mutation_name=$1
    trace_mutation_row=$2
    trace_mutation_action=$3
    trace_mutation_column=${4:-0}
    trace_mutation_value=${5:-}
    trace_mutation_expected=$6
    trace_mutation_source=${7:-$trace_test_primary}
    trace_mutation_id=${8:-$trace_test_id}
    trace_mutation_file="$trace_test_root/cases/$trace_mutation_name.tsv"
    cp "$trace_mutation_source" "$trace_mutation_file" || return 1
    selftest_trace_edit "$trace_mutation_file" "$trace_mutation_row" \
        "$trace_mutation_action" "$trace_mutation_column" "$trace_mutation_value" || return 1
    if [ "$trace_mutation_action" = set ]; then
        selftest_trace_only_field_changed "$trace_mutation_source" "$trace_mutation_file" \
            "$trace_mutation_row" "$trace_mutation_column" || {
            echo "FAIL trace mutant $trace_mutation_name changed fields outside its target" >&2
            return 1
        }
    fi
    selftest_trace_expect "$trace_mutation_file" "$trace_mutation_id" "$$" \
        "$trace_mutation_expected" "$trace_mutation_name"
}

selftest_trace_tdd_case() {
    trace_tdd_root=$1
    trace_tdd_name=$2
    trace_tdd_case_root="$trace_tdd_root/trace-tdd/$trace_tdd_name"
    trace_tdd_id="session-$trace_test_run_id-tdd-$trace_tdd_name"
    printf 'TAP version 13\n1..1\n'
    mkdir -p "$trace_tdd_case_root" || {
        printf 'Bail out! trace TDD setup failed\n'
        return 1
    }
    if [ "$trace_tdd_name" = absent-host-child-observations ]; then
        selftest_write_trace_lifecycle "$trace_tdd_case_root" "$trace_tdd_id" absent || {
            printf 'Bail out! actual trace writer setup failed\n'
            return 1
        }
        trace_tdd_source="$trace_tdd_case_root/ra/interruption-events.tsv"
        trace_tdd_expected=TRACE_OK
    else
        selftest_write_trace_regular_lock_lifecycle "$trace_tdd_case_root" "$trace_tdd_id" || {
            printf 'Bail out! actual regular-lock trace writer setup failed\n'
            return 1
        }
        trace_tdd_source="$trace_tdd_case_root/ra/interruption-events.tsv"
        trace_tdd_original="$trace_tdd_case_root/original.tsv"
        cp "$trace_tdd_source" "$trace_tdd_original" || {
            printf 'Bail out! actual trace baseline copy failed\n'
            return 1
        }
        trace_tdd_expected=TRACE_VALUE
        case "$trace_tdd_name" in
            host-live-without-pid) set -- 1 set 8 unavailable ;;
            child-live-without-pid) set -- 1 set 10 unavailable ;;
            host-pid-changed-before-cleanup) set -- 2 set 8 "$(( $$ + 1 ))" ;;
            child-pid-changed-before-cleanup) set -- 2 set 10 "$(( $$ + 1 ))" ;;
            host-pid-retained-after-cleanup) set -- 3 set 8 "$$" ;;
            child-pid-retained-after-cleanup) set -- 3 set 10 "$$" ;;
            host-live-after-cleanup) set -- 3 set 9 1 ;;
            child-live-after-cleanup) set -- 3 set 11 1 ;;
            row2-quit-before-journal-close) set -- 2 set 12 1 ;;
            numeric-pid-with-unavailable-liveness) set -- 1 set 9 unavailable; trace_tdd_expected=TRACE_OK ;;
            *) printf 'Bail out! unknown trace TDD case\n'; return 1 ;;
        esac
        selftest_trace_edit "$trace_tdd_source" "$1" "$2" "$3" "$4" || {
            printf 'Bail out! actual trace mutation failed\n'
            return 1
        }
        selftest_trace_only_field_changed "$trace_tdd_original" "$trace_tdd_source" \
            "$1" "$3" || {
            printf 'Bail out! trace mutant changed fields outside its target\n'
            return 1
        }
    fi
    trace_tdd_status=0
    trace_tdd_actual=$(verify_interruption_driver_trace \
        "$trace_tdd_source" "$trace_tdd_id" "$$") || trace_tdd_status=$?
    if [ "$trace_tdd_expected" = TRACE_OK ]; then
        if [ "$trace_tdd_status" -eq 0 ] && [ "$trace_tdd_actual" = TRACE_OK ]; then
            printf 'ok 1 - %s\n' "$trace_tdd_name"
            return 0
        fi
    elif [ "$trace_tdd_status" -ne 0 ] && [ "$trace_tdd_actual" = "$trace_tdd_expected" ]; then
        printf 'ok 1 - %s\n' "$trace_tdd_name"
        return 0
    fi
    printf '# expected=%s status=%s actual=%s status=%s\n' \
        "$trace_tdd_expected" "$([ "$trace_tdd_expected" = TRACE_OK ] && printf 0 || printf 1)" \
        "$trace_tdd_actual" "$trace_tdd_status"
    printf 'not ok 1 - %s\n' "$trace_tdd_name"
    return 1
}

selftest_trace_non_lock_fields_equal() {
    trace_non_lock_original=$1
    trace_non_lock_mutant=$2
    trace_non_lock_dir=$3
    cut -f1-13,15- "$trace_non_lock_original" >"$trace_non_lock_dir/original.tsv" || return 1
    cut -f1-13,15- "$trace_non_lock_mutant" >"$trace_non_lock_dir/mutant.tsv" || return 1
    cmp -s "$trace_non_lock_dir/original.tsv" "$trace_non_lock_dir/mutant.tsv"
}

selftest_prepare_trace_writer() {
    trace_writer_base=$1
    trace_writer_id=$2
    RA_DIR="$trace_writer_base/ra"
    LOCK_DIR="$trace_writer_base/locks/driver.lock"
    mkdir -p "$RA_DIR" "${LOCK_DIR%/*}" || return 1
    mkdir "$LOCK_DIR" || return 1
    STATE_SESSION_TEST_MODE=prefix
    STATE_SESSION_TEST_INTERRUPT_INVOCATION=$trace_writer_id
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
}

selftest_trace_writer_gate_tests() {
    trace_gate_root=$1/writer-gates
    trace_gate_status=0

    selftest_prepare_trace_writer "$trace_gate_root/alias" trace-alias || return 1
    STATE_SESSION_TEST_INTERRUPT_TRACE="$RA_DIR/aliased-events.tsv"
    trace_interruption_event handler-entry 0 || trace_gate_status=$?
    [ "$trace_gate_status" -ne 0 ] && [ "$INTERRUPTION_TRACE_FAILED" -eq 1 ] &&
        [ ! -e "$STATE_SESSION_TEST_INTERRUPT_TRACE" ] &&
        [ -s "$STATE_SESSION_TEST_INTERRUPT_TRACE_ERRORS" ] || {
        echo "FAIL trace selftest writer-alias-gate" >&2; return 1;
    }
    trace_selftest_count=$((trace_selftest_count + 1))
    printf 'trace selftest: PASS writer-alias-gate (TRACE_REJECTED)\n'

    selftest_prepare_trace_writer "$trace_gate_root/symlink" trace-symlink || return 1
    trace_gate_sentinel="$trace_gate_root/sentinel.tsv"
    printf 'sentinel\n' >"$trace_gate_sentinel"
    cp "$trace_gate_sentinel" "$trace_gate_root/sentinel.before"
    ln -s "$trace_gate_sentinel" "$STATE_SESSION_TEST_INTERRUPT_TRACE" || return 1
    trace_gate_status=0
    trace_interruption_event handler-entry 0 || trace_gate_status=$?
    cmp -s "$trace_gate_root/sentinel.before" "$trace_gate_sentinel" || return 1
    [ "$trace_gate_status" -ne 0 ] && [ "$INTERRUPTION_TRACE_FAILED" -eq 1 ] &&
        [ -s "$STATE_SESSION_TEST_INTERRUPT_TRACE_ERRORS" ] || {
        echo "FAIL trace selftest writer-symlink-gate" >&2; return 1;
    }
    trace_selftest_count=$((trace_selftest_count + 1))
    printf 'trace selftest: PASS writer-symlink-gate (TRACE_REJECTED)\n'

    selftest_prepare_trace_writer "$trace_gate_root/reuse" trace-reused || return 1
    printf 'pre-existing\n' >"$STATE_SESSION_TEST_INTERRUPT_TRACE"
    cp "$STATE_SESSION_TEST_INTERRUPT_TRACE" "$trace_gate_root/reused.before"
    trace_gate_status=0
    trace_interruption_event handler-entry 0 || trace_gate_status=$?
    cmp -s "$trace_gate_root/reused.before" "$STATE_SESSION_TEST_INTERRUPT_TRACE" || return 1
    [ "$trace_gate_status" -ne 0 ] && [ "$INTERRUPTION_TRACE_FAILED" -eq 1 ] &&
        [ -s "$STATE_SESSION_TEST_INTERRUPT_TRACE_ERRORS" ] || {
        echo "FAIL trace selftest writer-reuse-gate" >&2; return 1;
    }
    trace_selftest_count=$((trace_selftest_count + 1))
    printf 'trace selftest: PASS writer-reuse-gate (TRACE_REJECTED)\n'

    selftest_prepare_trace_writer "$trace_gate_root/write-error" trace-write-error || return 1
    trace_gate_fake_bin="$trace_gate_root/fake-bin"
    mkdir -p "$trace_gate_fake_bin"
    printf '#!/bin/sh\nexit 99\n' >"$trace_gate_fake_bin/perl"
    chmod +x "$trace_gate_fake_bin/perl"
    trace_gate_status=0
    PATH="$trace_gate_fake_bin" trace_interruption_event handler-entry 0 || trace_gate_status=$?
    [ "$trace_gate_status" -ne 0 ] && [ "$INTERRUPTION_TRACE_FAILED" -eq 1 ] &&
        [ ! -e "$STATE_SESSION_TEST_INTERRUPT_TRACE" ] &&
        [ -s "$STATE_SESSION_TEST_INTERRUPT_TRACE_ERRORS" ] || {
        echo "FAIL trace selftest writer-failure-gate" >&2; return 1;
    }
    trace_selftest_count=$((trace_selftest_count + 1))
    printf 'trace selftest: PASS writer-failure-gate (TRACE_REJECTED)\n'
}

run_interruption_trace_selftests() (
    trace_test_root=$1/trace-lifecycle
    trace_test_run_id=$(mono_ms)-$$
    trace_test_id="session-$trace_test_run_id-owned-a"
    trace_test_id_b="session-$trace_test_run_id-owned-b"
    trace_test_primary="$trace_test_root/owned-a/ra/interruption-events.tsv"
    trace_selftest_count=0
    mkdir -p "$trace_test_root/cases" || { echo "FAIL trace lifecycle setup" >&2; return 1; }
    if [ -n "${STATE_SESSION_TDD_CASE:-}" ]; then
        selftest_trace_tdd_case "$trace_test_root" "$STATE_SESSION_TDD_CASE"
        return $?
    fi
    selftest_write_trace_lifecycle "$trace_test_root/owned-a" "$trace_test_id" || {
        echo "FAIL trace lifecycle actual writer setup" >&2; return 1;
    }
    awk -F '\t' -v wanted="$trace_test_id" '
        NR == 1 && ($1 != wanted || $2 != "driver" || $3 != "1" || $5 != "handler-entry" || $8 !~ /^[1-9][0-9]*$/) { bad = 1 }
        NR == 2 && ($1 != wanted || $2 != "driver" || $3 != "2" || $5 != "journal-close-success" || $8 !~ /^[1-9][0-9]*$/) { bad = 1 }
        NR == 3 && ($1 != wanted || $2 != "driver" || $3 != "3" || $5 != "handler-exit" || $8 != "unavailable" || $10 != "unavailable" || $14 != "absent") { bad = 1 }
        NF != 16 { bad = 1 }
        END { if (NR != 3 || bad) exit 1 }
    ' "$trace_test_primary" || {
        echo "FAIL trace lifecycle actual writer did not emit expected cleanup fields" >&2; return 1;
    }
    selftest_trace_expect "$trace_test_primary" "$trace_test_id" "$$" \
        TRACE_OK cleanup-host-pid-admission || return 1

    trace_test_regular="$trace_test_root/regular-owned/ra/interruption-events.tsv"
    trace_test_regular_id="session-$trace_test_run_id-regular-lock"
    selftest_write_trace_regular_lock_lifecycle "$trace_test_root/regular-owned" \
        "$trace_test_regular_id" || {
        echo "FAIL regular-lock lifecycle actual writer setup" >&2; return 1;
    }
    selftest_trace_expect "$trace_test_regular" "$trace_test_regular_id" "$$" \
        TRACE_OK regular-hard-link-lifecycle || return 1
    for trace_lifecycle_case in \
        host-live-without-pid child-live-without-pid \
        host-pid-changed-before-cleanup child-pid-changed-before-cleanup \
        host-pid-retained-after-cleanup child-pid-retained-after-cleanup \
        host-live-after-cleanup child-live-after-cleanup row2-quit-before-journal-close; do
        case "$trace_lifecycle_case" in
            host-live-without-pid) set -- 1 set 8 unavailable ;;
            child-live-without-pid) set -- 1 set 10 unavailable ;;
            host-pid-changed-before-cleanup) set -- 2 set 8 "$(( $$ + 1 ))" ;;
            child-pid-changed-before-cleanup) set -- 2 set 10 "$(( $$ + 1 ))" ;;
            host-pid-retained-after-cleanup) set -- 3 set 8 "$$" ;;
            child-pid-retained-after-cleanup) set -- 3 set 10 "$$" ;;
            host-live-after-cleanup) set -- 3 set 9 1 ;;
            child-live-after-cleanup) set -- 3 set 11 1 ;;
            row2-quit-before-journal-close) set -- 2 set 12 1 ;;
        esac
        selftest_trace_expect_edit "$trace_lifecycle_case" "$1" "$2" "$3" "$4" TRACE_VALUE \
            "$trace_test_regular" "$trace_test_regular_id" || return 1
    done
    selftest_trace_only_field_changed "$trace_test_regular" \
        "$trace_test_root/cases/row2-quit-before-journal-close.tsv" 2 12 || {
        echo 'FAIL row2 QUIT mutant changed fields outside its target' >&2; return 1;
    }
    trace_test_absent="$trace_test_root/absent-observations/ra/interruption-events.tsv"
    trace_test_absent_id="session-$trace_test_run_id-absent-observations"
    selftest_write_trace_lifecycle "$trace_test_root/absent-observations" \
        "$trace_test_absent_id" absent || return 1
    selftest_trace_expect "$trace_test_absent" "$trace_test_absent_id" "$$" \
        TRACE_OK absent-host-child-observations || return 1
    trace_test_unavailable_liveness="$trace_test_root/cases/numeric-pid-with-unavailable-liveness.tsv"
    cp "$trace_test_regular" "$trace_test_unavailable_liveness" || return 1
    selftest_trace_edit "$trace_test_unavailable_liveness" 1 set 9 unavailable || return 1
    selftest_trace_expect "$trace_test_unavailable_liveness" "$trace_test_regular_id" "$$" \
        TRACE_OK numeric-pid-with-unavailable-liveness || return 1
    selftest_trace_expect_edit absent-held-lock 1 set 14 absent TRACE_VALUE \
        "$trace_test_regular" "$trace_test_regular_id" || return 1
    selftest_trace_non_lock_fields_equal "$trace_test_regular" \
        "$trace_test_root/cases/absent-held-lock.tsv" "$trace_test_root/cases" || {
        echo "FAIL absent-held-lock mutant changed a non-lock field" >&2; return 1;
    }
    selftest_trace_expect_edit present-released-lock 3 set 14 present TRACE_VALUE \
        "$trace_test_regular" "$trace_test_regular_id" || return 1
    selftest_trace_non_lock_fields_equal "$trace_test_regular" \
        "$trace_test_root/cases/present-released-lock.tsv" "$trace_test_root/cases" || {
        echo "FAIL present-released-lock mutant changed a non-lock field" >&2; return 1;
    }

    trace_test_dangling="$trace_test_root/dangling-lock/ra/interruption-events.tsv"
    selftest_prepare_trace_writer "$trace_test_root/dangling-lock" \
        "session-$trace_test_run_id-dangling-lock" || return 1
    rmdir "$LOCK_DIR" || return 1
    trace_test_dangling_target="$trace_test_root/nonexistent-lock-target"
    ln -s "$trace_test_dangling_target" "$LOCK_DIR" || return 1
    trace_interruption_event handler-entry 0 || return 1
    [ -L "$LOCK_DIR" ] && [ ! -e "$trace_test_dangling_target" ] &&
        [ "$(awk -F '\t' 'NR == 1 { print $14 }' "$trace_test_dangling")" = present ] || {
        echo "FAIL dangling lock symlink was not recorded as occupied without target creation" >&2; return 1;
    }
    trace_selftest_count=$((trace_selftest_count + 1))
    printf 'trace selftest: PASS dangling-lock-symlink-occupancy (present without target follow)\n'

    selftest_trace_expect_edit short-field-count 1 drop-field 0 '' TRACE_FIELDS || return 1
    selftest_trace_expect_edit extra-field-count 1 extra-field 0 '' TRACE_FIELDS || return 1
    trace_empty="$trace_test_root/cases/empty.tsv"
    cp "$trace_test_primary" "$trace_empty" || return 1
    selftest_trace_edit "$trace_empty" 1 empty || return 1
    selftest_trace_expect "$trace_empty" "$trace_test_id" "$$" TRACE_FIELDS empty-trace || return 1
    selftest_trace_expect_edit truncated-final-line 0 no-final-newline 0 '' TRACE_FIELDS || return 1
    selftest_trace_expect_edit wrong-role 2 set 2 harness TRACE_ROLE || return 1
    selftest_trace_expect_edit wrong-invocation 2 set 1 other-invocation TRACE_INVOCATION || return 1
    selftest_trace_expect_edit noncontiguous-sequence 2 set 3 3 TRACE_SEQUENCE || return 1
    selftest_trace_expect_edit malformed-sequence 2 set 3 1.0 TRACE_SEQUENCE || return 1
    selftest_trace_expect_edit unavailable-sequence 2 set 3 unavailable TRACE_SEQUENCE || return 1
    selftest_trace_expect_edit nonnumeric-time 1 set 4 unavailable TRACE_TIME || return 1
    selftest_trace_expect_edit decreasing-time 2 set 4 0 TRACE_TIME || return 1
    selftest_trace_expect_edit wrong-event-order 2 set 5 handler-exit TRACE_EVENT || return 1
    selftest_trace_expect_edit invalid-status 2 set 6 unavailable TRACE_VALUE || return 1
    selftest_trace_expect_edit wrong-driver-pid 2 set 7 0 TRACE_DRIVER_PID || return 1
    selftest_trace_expect_edit mismatched-driver-pid 2 set 7 "$(( $$ + 1 ))" TRACE_DRIVER_PID || return 1
    selftest_trace_expect_edit zero-host-pid 3 set 8 0 TRACE_VALUE || return 1
    selftest_trace_expect_edit signed-child-pid 3 set 10 +1 TRACE_VALUE || return 1
    selftest_trace_expect_edit invalid-host-liveness 3 set 9 0 TRACE_VALUE || return 1
    selftest_trace_expect_edit invalid-child-liveness 3 set 11 2 TRACE_VALUE || return 1
    selftest_trace_expect_edit invalid-quit-flag 2 set 12 2 TRACE_VALUE || return 1
    selftest_trace_expect_edit invalid-journal-flag 2 set 13 2 TRACE_VALUE || return 1
    selftest_trace_expect_edit invalid-lock-value 2 set 14 unknown TRACE_VALUE || return 1
    selftest_trace_expect_edit fractional-send-count 2 set 15 1.0 TRACE_VALUE || return 1
    selftest_trace_expect_edit unavailable-send-count 2 set 15 unavailable TRACE_VALUE || return 1
    selftest_trace_expect_edit invalid-first-failure 2 set 16 JOURNAL_RECORD_FAILURE TRACE_VALUE || return 1
    selftest_trace_expect_edit inconsistent-first-failure 3 set 16 INTERRUPTED_INT TRACE_VALUE || return 1
    selftest_trace_expect_edit missing-event-row 2 remove-row 0 '' TRACE_EVENT || return 1
    selftest_trace_expect_edit extra-event-row 0 append-row 0 '' TRACE_EVENT || return 1

    trace_equal="$trace_test_root/cases/equal-monotonic-samples.tsv"
    cp "$trace_test_primary" "$trace_equal" || return 1
    trace_first_time=$(awk -F '\t' 'NR == 1 { print $4 }' "$trace_test_primary")
    selftest_trace_edit "$trace_equal" 2 set 4 "$trace_first_time" || return 1
    selftest_trace_edit "$trace_equal" 3 set 4 "$trace_first_time" || return 1
    selftest_trace_expect "$trace_equal" "$trace_test_id" "$$" TRACE_OK equal-monotonic-samples || return 1

    trace_test_second="$trace_test_root/owned-b/ra/interruption-events.tsv"
    selftest_write_trace_lifecycle "$trace_test_root/owned-b" "$trace_test_id_b" || {
        echo "FAIL trace lifecycle second owned writer setup" >&2; return 1;
    }
    selftest_trace_expect "$trace_test_second" "$trace_test_id_b" "$$" TRACE_OK second-owned-invocation || return 1
    trace_mixed="$trace_test_root/cases/mixed-invocations.tsv"
    cp "$trace_test_primary" "$trace_mixed" || return 1
    selftest_trace_edit "$trace_mixed" 2 set 1 "$trace_test_id_b" || return 1
    selftest_trace_expect "$trace_mixed" "$trace_test_id" "$$" \
        TRACE_INVOCATION cross-invocation-row || return 1

    selftest_trace_writer_gate_tests "$trace_test_root" || return 1
    printf 'state session interruption trace selftest: %s cases passed (40 retained + 11 lifecycle/QUIT cases)\n' "$trace_selftest_count"
)

selftest_owner_sidecar_collision() {
    selftest_owner_root=$1
    selftest_owner_dir="$selftest_owner_root/owner-sidecar-collision"
    selftest_owner_lock_root="$selftest_owner_dir/locks"
    selftest_owner_bin="$selftest_owner_dir/bin"
    selftest_owner_real_perl=$(command -v perl)
    OWNER_COLLISION_FAILURE=
    mkdir -p "$selftest_owner_lock_root" "$selftest_owner_bin" "$selftest_owner_dir/ra"
    : >"$selftest_owner_dir/config.cfg"
    : >"$selftest_owner_dir/core.so"
    : >"$selftest_owner_dir/content.nes"
    : >"$selftest_owner_dir/calibration.txt"
    printf 'foreign owner-sidecar sentinel bytes\n' >"$selftest_owner_dir/sentinel.expected"

    cat >"$selftest_owner_bin/perl" <<'EOF'
#!/bin/sh
if [ "$1" = '-MTime::HiRes=clock_gettime,CLOCK_MONOTONIC' ]; then
    owner_clock=$("$STATE_SESSION_OWNER_REAL_PERL" "$@") || exit $?
    owner_driver=$(cat "$STATE_SESSION_OWNER_PID_FILE") || exit 91
    owner_path="$STATE_SESSION_OWNER_LOCK_ROOT/nesturbator-retroarch-55355.lock.owner.attempt-${owner_clock}-${owner_driver}"
    cat "$STATE_SESSION_OWNER_SENTINEL_FILE" >"$owner_path" || exit 92
    printf '%s\n' "$owner_clock" >"$STATE_SESSION_OWNER_CLOCK_FILE" || exit 93
    printf '%s\n' "$owner_path" >"$STATE_SESSION_OWNER_PATH_FILE" || exit 94
    "$STATE_SESSION_OWNER_REAL_PERL" -e 'my @s = lstat($ARGV[0]) or exit 1; print join("\t", @s[0,1,2,3,7]), "\n"' \
        "$owner_path" >"$STATE_SESSION_OWNER_IDENTITY_FILE" || exit 95
    printf '%s\n' "$owner_clock"
    exit 0
fi
exec "$STATE_SESSION_OWNER_REAL_PERL" "$@"
EOF
    cat >"$selftest_owner_dir/driver-runner" <<'EOF'
#!/bin/sh
printf '%s\n' "$$" >"$STATE_SESSION_OWNER_PID_FILE"
exec "${STATE_SESSION_OWNER_SHELL:-/bin/sh}" "$STATE_SESSION_OWNER_DRIVER" "$@"
EOF
    cat >"$selftest_owner_dir/fake-retroarch" <<'EOF'
#!/bin/sh
: >"$STATE_SESSION_OWNER_HOST_LAUNCH_FILE"
exit 0
EOF
    cat >"$selftest_owner_dir/fake-nc" <<'EOF'
#!/bin/sh
: >"$STATE_SESSION_OWNER_TRANSPORT_LAUNCH_FILE"
exit 0
EOF
    chmod +x "$selftest_owner_bin/perl" "$selftest_owner_dir/driver-runner" \
        "$selftest_owner_dir/fake-retroarch" "$selftest_owner_dir/fake-nc"

    selftest_owner_status=0
    PATH="$selftest_owner_bin:$PATH" \
    STATE_SESSION_OWNER_REAL_PERL="$selftest_owner_real_perl" \
    STATE_SESSION_OWNER_DRIVER="$DRIVER" \
    STATE_SESSION_OWNER_SHELL="${STATE_SESSION_TEST_SHELL:-/bin/sh}" \
    STATE_SESSION_OWNER_PID_FILE="$selftest_owner_dir/driver.pid" \
    STATE_SESSION_OWNER_LOCK_ROOT="$selftest_owner_lock_root" \
    STATE_SESSION_OWNER_SENTINEL_FILE="$selftest_owner_dir/sentinel.expected" \
    STATE_SESSION_OWNER_CLOCK_FILE="$selftest_owner_dir/clock.ms" \
    STATE_SESSION_OWNER_PATH_FILE="$selftest_owner_dir/owner.path" \
    STATE_SESSION_OWNER_IDENTITY_FILE="$selftest_owner_dir/owner.identity.before" \
    STATE_SESSION_OWNER_HOST_LAUNCH_FILE="$selftest_owner_dir/host-launched" \
    STATE_SESSION_OWNER_TRANSPORT_LAUNCH_FILE="$selftest_owner_dir/transport-launched" \
    STATE_SESSION_NC="$selftest_owner_dir/fake-nc" \
    STATE_SESSION_LOCK_ROOT="$selftest_owner_lock_root" \
        "$selftest_owner_dir/driver-runner" \
        "$selftest_owner_dir/fake-retroarch" "$selftest_owner_dir/config.cfg" \
        "$selftest_owner_dir/core.so" "$selftest_owner_dir/content.nes" \
        "$selftest_owner_dir/ra" "$selftest_owner_dir/retroarch.log" \
        "$FRAME_INDEX" "$selftest_owner_dir/calibration.txt" \
        >"$selftest_owner_dir/driver.out" 2>&1 &
    selftest_owner_driver_runner_pid=$!
    wait "$selftest_owner_driver_runner_pid" || selftest_owner_status=$?

    selftest_owner_driver_pid=$(cat "$selftest_owner_dir/driver.pid" 2>/dev/null || true)
    selftest_owner_clock=$(cat "$selftest_owner_dir/clock.ms" 2>/dev/null || true)
    selftest_owner_path=$(cat "$selftest_owner_dir/owner.path" 2>/dev/null || true)
    selftest_owner_expected_path="$selftest_owner_lock_root/nesturbator-retroarch-55355.lock.owner.attempt-${selftest_owner_clock}-${selftest_owner_driver_pid}"
    if [ -z "$selftest_owner_driver_pid" ] || [ "$selftest_owner_driver_pid" != "$selftest_owner_driver_runner_pid" ] ||
        [ -z "$selftest_owner_clock" ] || [ "$selftest_owner_path" != "$selftest_owner_expected_path" ] ||
        [ ! -s "$selftest_owner_dir/owner.identity.before" ]; then
        OWNER_COLLISION_FAILURE=owner-collision-exact-path-not-established
        return 1
    fi
    if ! owner_sidecar_refusal_assess "$selftest_owner_dir" "$selftest_owner_status"; then
        return 1
    fi
    return 0
}

selftest_owner_sidecar_mutant_fixture() {
    owner_mutant_root=$1
    owner_mutant_dir="$owner_mutant_root/owner-sidecar-mutants"
    mkdir -p "$owner_mutant_dir/locks"
    printf 'foreign owner-sidecar sentinel bytes\n' >"$owner_mutant_dir/sentinel.expected"
    printf '1001\n' >"$owner_mutant_dir/driver.pid"
    printf '2001\n' >"$owner_mutant_dir/clock.ms"
    owner_mutant_path="$owner_mutant_dir/locks/nesturbator-retroarch-55355.lock.owner.attempt-2001-1001"
    cp "$owner_mutant_dir/sentinel.expected" "$owner_mutant_path"
    printf '%s\n' "$owner_mutant_path" >"$owner_mutant_dir/owner.path"
    "$selftest_owner_real_perl" -e 'my @s = lstat($ARGV[0]) or exit 1; print join("\t", @s[0,1,2,3,7]), "\n"' \
        "$owner_mutant_path" >"$owner_mutant_dir/owner.identity.before" || return 1
    printf '%s: 90: cannot create %s: File exists\n' "$DRIVER" "$owner_mutant_path" \
        >"$owner_mutant_dir/driver.out"
}

selftest_owner_sidecar_mutant_rejected() {
    owner_mutant_root=$1
    owner_mutant_status=$2
    owner_mutant_expected=$3
    if owner_sidecar_refusal_assess "$owner_mutant_root/owner-sidecar-mutants" "$owner_mutant_status"; then
        printf 'FAIL owner-sidecar proof accepted unsafe mutant: %s\n' "$owner_mutant_expected" >&2
        return 1
    fi
    [ "$OWNER_COLLISION_FAILURE" = "$owner_mutant_expected" ] || {
        printf 'FAIL owner-sidecar proof mutant reason: expected=%s actual=%s\n' \
            "$owner_mutant_expected" "$OWNER_COLLISION_FAILURE" >&2
        return 1
    }
    printf 'owner-sidecar proof mutant rejected: %s\n' "$OWNER_COLLISION_FAILURE"
}

selftest_owner_sidecar_mutants() {
    owner_mutants_root=$1

    selftest_owner_sidecar_mutant_fixture "$owner_mutants_root/success" || return 1
    selftest_owner_sidecar_mutant_rejected "$owner_mutants_root/success" 0 actual-driver-refusal-not-proven || return 1

    selftest_owner_sidecar_mutant_fixture "$owner_mutants_root/clobber" || return 1
    owner_mutant_path="$owner_mutants_root/clobber/owner-sidecar-mutants/locks/nesturbator-retroarch-55355.lock.owner.attempt-2001-1001"
    printf 'clobbered foreign bytes\n' >"$owner_mutant_path"
    selftest_owner_sidecar_mutant_rejected "$owner_mutants_root/clobber" 2 foreign-owner-object-not-preserved || return 1

    selftest_owner_sidecar_mutant_fixture "$owner_mutants_root/removal" || return 1
    owner_mutant_path="$owner_mutants_root/removal/owner-sidecar-mutants/locks/nesturbator-retroarch-55355.lock.owner.attempt-2001-1001"
    rm "$owner_mutant_path"
    selftest_owner_sidecar_mutant_rejected "$owner_mutants_root/removal" 2 foreign-owner-object-not-preserved || return 1

    selftest_owner_sidecar_mutant_fixture "$owner_mutants_root/wrong-path" || return 1
    printf '%s\n' "$owner_mutants_root/wrong-path/owner-sidecar-mutants/locks/wrong.owner.path" \
        >"$owner_mutants_root/wrong-path/owner-sidecar-mutants/owner.path"
    selftest_owner_sidecar_mutant_rejected "$owner_mutants_root/wrong-path" 2 owner-collision-exact-path-not-established || return 1

    selftest_owner_sidecar_mutant_fixture "$owner_mutants_root/host-launch" || return 1
    : >"$owner_mutants_root/host-launch/owner-sidecar-mutants/host-launched"
    selftest_owner_sidecar_mutant_rejected "$owner_mutants_root/host-launch" 2 host-or-transport-launched-before-refusal || return 1

    selftest_owner_sidecar_mutant_fixture "$owner_mutants_root/transport-launch" || return 1
    : >"$owner_mutants_root/transport-launch/owner-sidecar-mutants/transport-launched"
    selftest_owner_sidecar_mutant_rejected "$owner_mutants_root/transport-launch" 2 host-or-transport-launched-before-refusal || return 1

    selftest_owner_sidecar_mutant_fixture "$owner_mutants_root/precondition" || return 1
    printf 'state_session: INPUT_MISSING\n' >"$owner_mutants_root/precondition/owner-sidecar-mutants/driver.out"
    selftest_owner_sidecar_mutant_rejected "$owner_mutants_root/precondition" 2 actual-driver-refusal-not-proven || return 1
    return 0
}

selftest_owner_sidecar_replacement() {
    selftest_replace_root=$1
    selftest_replace_dir="$selftest_replace_root/owner-sidecar-replacement"
    selftest_replace_lock_root="$selftest_replace_dir/locks"
    selftest_replace_bin="$selftest_replace_dir/bin"
    selftest_replace_real_perl=$(command -v perl)
    selftest_replace_real_ln=$(command -v ln)
    OWNER_REPLACEMENT_FAILURE=
    mkdir -p "$selftest_replace_lock_root" "$selftest_replace_bin" "$selftest_replace_dir/ra"
    : >"$selftest_replace_dir/config.cfg"
    : >"$selftest_replace_dir/core.so"
    : >"$selftest_replace_dir/content.nes"
    : >"$selftest_replace_dir/calibration.txt"
    printf 'foreign replacement owner-sidecar bytes\n' >"$selftest_replace_dir/replacement.expected"

    cat >"$selftest_replace_bin/perl" <<'EOF'
#!/bin/sh
if [ "$1" = '-MTime::HiRes=clock_gettime,CLOCK_MONOTONIC' ]; then
    owner_clock=$("$STATE_SESSION_OWNER_REAL_PERL" "$@") || exit $?
    owner_driver=$(cat "$STATE_SESSION_OWNER_PID_FILE") || exit 91
    printf '%s\n' "$owner_clock" >"$STATE_SESSION_OWNER_CLOCK_FILE" || exit 92
    printf '%s\n' "$owner_driver" >"$STATE_SESSION_OWNER_CLOCK_PID_FILE" || exit 93
    printf '%s\n' "$owner_clock"
    exit 0
fi
exec "$STATE_SESSION_OWNER_REAL_PERL" "$@"
EOF
    cat >"$selftest_replace_bin/ln" <<'EOF'
#!/bin/sh
if [ "$#" -eq 2 ] && [ "$2" = "$STATE_SESSION_OWNER_SIGNAL_LOCK" ]; then
    printf '%s\n' "$1" >"$STATE_SESSION_OWNER_PATH_FILE" || exit 91
    "$STATE_SESSION_OWNER_REAL_PERL" -e 'my @s = lstat($ARGV[0]) or exit 1; print join("\t", @s[0,1,2,3,7]), "\n"' \
        "$1" >"$STATE_SESSION_OWNER_IDENTITY_BEFORE" || exit 92
    "$STATE_SESSION_OWNER_REAL_LN" "$@" || exit $?
    "$STATE_SESSION_OWNER_REAL_PERL" -e 'my @s = lstat($ARGV[0]) or exit 1; print join("\t", @s[0,1,2,3,7]), "\n"' \
        "$2" >"$STATE_SESSION_OWNER_LOCK_IDENTITY" || exit 93
    cp "$STATE_SESSION_OWNER_REPLACEMENT_EXPECTED" "$1.replacement" || exit 94
    mv -f "$1.replacement" "$1" || exit 95
    "$STATE_SESSION_OWNER_REAL_PERL" -e 'my @s = lstat($ARGV[0]) or exit 1; print join("\t", @s[0,1,2,3,7]), "\n"' \
        "$1" >"$STATE_SESSION_OWNER_REPLACEMENT_IDENTITY" || exit 96
    kill -TERM "$PPID" 2>/dev/null || exit 97
    exit 0
fi
exec "$STATE_SESSION_OWNER_REAL_LN" "$@"
EOF
    cat >"$selftest_replace_dir/driver-runner" <<'EOF'
#!/bin/sh
printf '%s\n' "$$" >"$STATE_SESSION_OWNER_PID_FILE"
exec /bin/sh "$STATE_SESSION_OWNER_DRIVER" "$@"
EOF
    cat >"$selftest_replace_dir/fake-retroarch" <<'EOF'
#!/bin/sh
: >"$STATE_SESSION_OWNER_HOST_LAUNCH_FILE"
exit 0
EOF
    cat >"$selftest_replace_dir/fake-nc" <<'EOF'
#!/bin/sh
: >"$STATE_SESSION_OWNER_TRANSPORT_LAUNCH_FILE"
exit 0
EOF
    chmod +x "$selftest_replace_bin/perl" "$selftest_replace_bin/ln" \
        "$selftest_replace_dir/driver-runner" "$selftest_replace_dir/fake-retroarch" \
        "$selftest_replace_dir/fake-nc"

    selftest_replace_status=0
    PATH="$selftest_replace_bin:$PATH" \
    STATE_SESSION_OWNER_REAL_PERL="$selftest_replace_real_perl" \
    STATE_SESSION_OWNER_REAL_LN="$selftest_replace_real_ln" \
    STATE_SESSION_OWNER_DRIVER="$DRIVER" \
    STATE_SESSION_OWNER_PID_FILE="$selftest_replace_dir/driver.pid" \
    STATE_SESSION_OWNER_LOCK_ROOT="$selftest_replace_lock_root" \
    STATE_SESSION_OWNER_SIGNAL_LOCK="$selftest_replace_lock_root/nesturbator-retroarch-55355.lock" \
    STATE_SESSION_OWNER_CLOCK_FILE="$selftest_replace_dir/clock.ms" \
    STATE_SESSION_OWNER_CLOCK_PID_FILE="$selftest_replace_dir/clock.pid" \
    STATE_SESSION_OWNER_PATH_FILE="$selftest_replace_dir/owner.path" \
    STATE_SESSION_OWNER_IDENTITY_BEFORE="$selftest_replace_dir/owner.identity.before" \
    STATE_SESSION_OWNER_LOCK_IDENTITY="$selftest_replace_dir/lock.identity.after-link" \
    STATE_SESSION_OWNER_REPLACEMENT_IDENTITY="$selftest_replace_dir/replacement.identity" \
    STATE_SESSION_OWNER_REPLACEMENT_EXPECTED="$selftest_replace_dir/replacement.expected" \
    STATE_SESSION_OWNER_HOST_LAUNCH_FILE="$selftest_replace_dir/host-launched" \
    STATE_SESSION_OWNER_TRANSPORT_LAUNCH_FILE="$selftest_replace_dir/transport-launched" \
    STATE_SESSION_NC="$selftest_replace_dir/fake-nc" \
    STATE_SESSION_LOCK_ROOT="$selftest_replace_lock_root" \
        "$selftest_replace_dir/driver-runner" \
        "$selftest_replace_dir/fake-retroarch" "$selftest_replace_dir/config.cfg" \
        "$selftest_replace_dir/core.so" "$selftest_replace_dir/content.nes" \
        "$selftest_replace_dir/ra" "$selftest_replace_dir/retroarch.log" \
        "$FRAME_INDEX" "$selftest_replace_dir/calibration.txt" \
        >"$selftest_replace_dir/driver.out" 2>&1 &
    selftest_replace_runner_pid=$!
    wait "$selftest_replace_runner_pid" || selftest_replace_status=$?

    selftest_replace_driver_pid=$(cat "$selftest_replace_dir/driver.pid" 2>/dev/null || true)
    selftest_replace_clock=$(cat "$selftest_replace_dir/clock.ms" 2>/dev/null || true)
    selftest_replace_clock_pid=$(cat "$selftest_replace_dir/clock.pid" 2>/dev/null || true)
    selftest_replace_path=$(cat "$selftest_replace_dir/owner.path" 2>/dev/null || true)
    selftest_replace_expected_path="$selftest_replace_lock_root/nesturbator-retroarch-55355.lock.owner.attempt-${selftest_replace_clock}-${selftest_replace_driver_pid}"
    if [ -z "$selftest_replace_driver_pid" ] || [ "$selftest_replace_driver_pid" != "$selftest_replace_runner_pid" ] ||
        [ "$selftest_replace_clock_pid" != "$selftest_replace_driver_pid" ] ||
        [ -z "$selftest_replace_clock" ] || [ "$selftest_replace_path" != "$selftest_replace_expected_path" ] ||
        [ ! -s "$selftest_replace_dir/owner.identity.before" ] ||
        [ ! -s "$selftest_replace_dir/lock.identity.after-link" ] ||
        [ ! -s "$selftest_replace_dir/replacement.identity" ]; then
        OWNER_REPLACEMENT_FAILURE=owner-replacement-lifecycle-not-established
        return 1
    fi
    if [ "$selftest_replace_status" -ne 143 ] || [ -e "$selftest_replace_dir/host-launched" ] ||
        [ -e "$selftest_replace_dir/transport-launched" ] ||
        [ -e "$selftest_replace_lock_root/nesturbator-retroarch-55355.lock" ] ||
        ! cut -f1-2 "$selftest_replace_dir/owner.identity.before" >"$selftest_replace_dir/owner.inode.before" ||
        ! cut -f1-2 "$selftest_replace_dir/lock.identity.after-link" >"$selftest_replace_dir/lock.inode.after-link" ||
        ! cmp -s "$selftest_replace_dir/owner.inode.before" "$selftest_replace_dir/lock.inode.after-link"; then
        OWNER_REPLACEMENT_FAILURE=owned-hard-link-release-not-proven
        return 1
    fi
    if [ ! -f "$selftest_replace_path" ] || [ -L "$selftest_replace_path" ] ||
        ! cmp -s "$selftest_replace_dir/replacement.expected" "$selftest_replace_path" ||
        ! "$selftest_replace_real_perl" -e 'my @s = lstat($ARGV[0]) or exit 1; print join("\t", @s[0,1,2,3,7]), "\n"' \
            "$selftest_replace_path" >"$selftest_replace_dir/replacement.identity.after-cleanup" ||
        ! cmp -s "$selftest_replace_dir/replacement.identity" "$selftest_replace_dir/replacement.identity.after-cleanup"; then
        OWNER_REPLACEMENT_FAILURE=replaced-foreign-owner-object-not-preserved
        return 1
    fi
    OWNER_REPLACEMENT_FAILURE=
    return 0
}

selftest_regular_lock_trace_tap() (
    trace_tap_id="session-regular-lock-$(mono_ms)-$$"
    trace_tap_file="$WORK_ROOT/regular-lock-trace-selftest/ra/interruption-events.tsv"
    trace_tap_target='regular hard-link lifecycle records held occupancy until release'
    printf 'TAP version 13\n1..3\n'
    selftest_write_trace_regular_lock_lifecycle \
        "$WORK_ROOT/regular-lock-trace-selftest" "$trace_tap_id" || {
        printf 'Bail out! regular-lock writer fixture or event recording failed\n'
        return 1
    }
    trace_tap_rows_ok=0
    awk -F '\t' -v wanted="$trace_tap_id" -v driver="$$" '
        function positive(value) { return value ~ /^[0-9]+$/ && value ~ /[1-9]/ }
        NR == 1 && ($1 != wanted || $2 != "driver" || $3 != "1" || $4 !~ /^[0-9]+$/ ||
            $5 != "handler-entry" || $6 != "0" || $7 != driver || !positive($8) ||
            $9 != "1" || $10 != $8 || $11 != "1" || $12 != "0" || $13 != "0" ||
            $15 != "0" || $16 != "none") { bad = 1 }
        NR == 2 && ($1 != wanted || $2 != "driver" || $3 != "2" || $4 !~ /^[0-9]+$/ ||
            $5 != "journal-close-success" || $6 != "0" || $7 != driver || !positive($8) ||
            $9 != "1" || $10 != $8 || $11 != "1" || $12 != "0" || $13 != "0" ||
            $15 != "0" || $16 != "INTERRUPTED_TERM") { bad = 1 }
        NR == 3 && ($1 != wanted || $2 != "driver" || $3 != "3" || $4 !~ /^[0-9]+$/ ||
            $5 != "handler-exit" || $6 != "130" || $7 != driver || $8 != "unavailable" ||
            $9 != "unavailable" || $10 != "unavailable" || $11 != "unavailable" ||
            $12 != "1" || $13 != "0" || $15 != "0" || $16 != "INTERRUPTED_TERM") { bad = 1 }
        NF != 16 { bad = 1 }
        END { if (NR != 3 || bad) exit 1 }
    ' "$trace_tap_file" && trace_tap_rows_ok=1
    [ "$trace_tap_rows_ok" -eq 1 ] || {
        printf 'Bail out! regular-lock trace had invalid event, identity, timing, status, PID, journal, QUIT, send, or first-failure fields\n'
        return 1
    }
    trace_tap_status=0
    trace_tap_result=$(verify_interruption_driver_trace "$trace_tap_file" "$trace_tap_id" "$$") ||
        trace_tap_status=$?
    trace_tap_lock_values=$(awk -F '\t' 'NR == 1 { a=$14 } NR == 2 { b=$14 } NR == 3 { c=$14 } END { print a "," b "," c }' "$trace_tap_file")
    if [ "$trace_tap_status" -eq 1 ] && [ "$trace_tap_result" = TRACE_VALUE ] &&
        [ "$trace_tap_lock_values" = absent,absent,absent ]; then
        printf '# expected lock field 14: present,present,absent; validator: TRACE_OK\n'
        printf '# actual lock field 14: %s; validator: %s\n' "$trace_tap_lock_values" "$trace_tap_result"
        printf 'not ok 1 - %s\n' "$trace_tap_target"
        return 1
    fi
    if [ "$trace_tap_status" -eq 0 ] && [ "$trace_tap_result" = TRACE_OK ] &&
        [ "$trace_tap_lock_values" = present,present,absent ]; then
        printf 'ok 1 - %s\n' "$trace_tap_target"
    else
        printf 'Bail out! unexpected regular-lock result: fields=%s validator=%s status=%s\n' \
            "$trace_tap_lock_values" "$trace_tap_result" "$trace_tap_status"
        return 1
    fi
    tap_failure=0
    if selftest_owner_sidecar_collision "$WORK_ROOT"; then
        printf 'ok 2 - actual failed owner-sidecar create preserves foreign object bytes\n'
    elif [ "$OWNER_COLLISION_FAILURE" = foreign-owner-object-not-preserved ]; then
        printf '# actual driver refused the exact pre-created path with LOCK_OWNER_UNAVAILABLE\n'
        printf '# foreign object path and sentinel identity/bytes were changed by EXIT cleanup\n'
        printf 'not ok 2 - actual failed owner-sidecar create preserves foreign object bytes\n'
        tap_failure=1
    else
        printf 'Bail out! owner-sidecar collision precondition failed: %s\n' "$OWNER_COLLISION_FAILURE"
        return 1
    fi

    if selftest_owner_sidecar_replacement "$WORK_ROOT"; then
        printf 'ok 3 - actual startup cleanup preserves a replaced foreign owner object\n'
    elif [ "$OWNER_REPLACEMENT_FAILURE" = replaced-foreign-owner-object-not-preserved ]; then
        printf '# actual driver registered the owned hard link, then cleanup removed the replacement object\n'
        printf 'not ok 3 - actual startup cleanup preserves a replaced foreign owner object\n'
        tap_failure=1
    else
        printf 'Bail out! owner-sidecar replacement precondition failed: %s\n' "$OWNER_REPLACEMENT_FAILURE"
        return 1
    fi
    return "$tap_failure"
)

run_interruption_capture_selftests() {
    selftest_root=$1
    INTERRUPTION_CAPTURE_SELFTEST_ACTIVE=1
    selftest_run_id=$(mono_ms)-$$
    selftest_root="$selftest_root/selftest-$selftest_run_id"
    mkdir -p "$selftest_root"

    if [ -n "${STATE_SESSION_TDD_CASE:-}" ]; then
        case "${STATE_SESSION_TDD_KIND:-trace}" in
            trace)
                run_interruption_trace_selftests "$selftest_root"
                return $? ;;
            real-events)
                selftest_real_control_event_case "$selftest_root" \
                    "$STATE_SESSION_TDD_CASE" 1
                return $? ;;
            *)
                printf 'TAP version 13\n1..1\nBail out! invalid internal selftest kind\n'
                return 1 ;;
        esac
    fi

    if selftest_owner_sidecar_collision "$selftest_root"; then
        printf 'state session owner-sidecar selftest: %s; foreign object bytes and identity preserved\n' \
            "$OWNER_COLLISION_OBSERVATION"
    else
        echo "FAIL owner-sidecar collision: $OWNER_COLLISION_FAILURE" >&2
        return 1
    fi
    selftest_owner_sidecar_mutants "$selftest_root" || return 1
    if selftest_owner_sidecar_replacement "$selftest_root"; then
        printf 'state session owner-sidecar selftest: exact owned hard link released; replacement object and bytes preserved\n'
    else
        echo "FAIL owner-sidecar replacement: $OWNER_REPLACEMENT_FAILURE" >&2
        return 1
    fi

    selftest_match="$selftest_root/match"
    mkdir -p "$selftest_match"
    printf 'step\tclass\treason\nINTERRUPTED_TERM\tsignal\tTERM\nCOMPLETE\tfinish\tOK\n' >"$selftest_match/journal.tsv"
    printf 'INTERRUPTED_TERM\tsignal\tTERM\n' >"$selftest_match/expected.stdout"
    : >"$selftest_match/expected.stderr"
    INTERRUPTION_TEST_CALLS="$selftest_match/calls"
    export INTERRUPTION_TEST_CALLS
    selftest_status=0
    capture_interruption_assertion selftest_counted_grep INTERRUPTED_TERM \
        "$selftest_match/journal.tsv" "$selftest_match" /dev/null selftest-match || selftest_status=$?
    [ "$selftest_status" -eq 0 ] && [ "$INTERRUPTION_ASSERTION_STATUS" -eq 0 ] &&
        [ "$INTERRUPTION_RECORDER_STATUS" -eq 0 ] &&
        [ "$(selftest_count_calls "$selftest_match/calls")" -eq 1 ] &&
        cmp -s "$selftest_match/expected.stdout" "$selftest_match/assertion.stdout" &&
        cmp -s "$selftest_match/expected.stderr" "$selftest_match/assertion.stderr" || {
        echo "FAIL interruption capture status-0 bytes or one-call contract" >&2; return 1;
    }
    selftest_assert_events "$selftest_match/events.tsv" selftest-match || {
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
        "$selftest_miss/journal.tsv" "$selftest_miss" /dev/null selftest-no-match || selftest_status=$?
    [ "$selftest_status" -eq 1 ] && [ "$INTERRUPTION_ASSERTION_STATUS" -eq 1 ] &&
        [ "$INTERRUPTION_RECORDER_STATUS" -eq 0 ] &&
        [ "$(selftest_count_calls "$selftest_miss/calls")" -eq 1 ] &&
        cmp -s "$selftest_miss/expected.stdout" "$selftest_miss/assertion.stdout" &&
        cmp -s "$selftest_miss/expected.stderr" "$selftest_miss/assertion.stderr" || {
        echo "FAIL interruption capture status-1 bytes or one-call contract" >&2; return 1;
    }
    selftest_assert_events "$selftest_miss/events.tsv" selftest-no-match || {
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
        selftest-controlled-error \
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
            "$selftest_journal" "$selftest_unwritable" /dev/null \
            "selftest-$selftest_case" || selftest_status=$?
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
        selftest-replacement "$selftest_replace/replacement.tsv" || selftest_status=$?
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
    selftest_assert_events "$selftest_replace/events.tsv" selftest-replacement || {
        echo "FAIL replacement events are not invocation-owned and ordered" >&2; return 1;
    }

    selftest_second="$selftest_root/second-invocation"
    mkdir -p "$selftest_second"
    printf 'INTERRUPTED_TERM\tsecond\n' >"$selftest_second/journal.tsv"
    INTERRUPTION_TEST_CALLS="$selftest_second/calls"
    export INTERRUPTION_TEST_CALLS
    selftest_status=0
    capture_interruption_assertion selftest_counted_grep INTERRUPTED_TERM \
        "$selftest_second/journal.tsv" "$selftest_second" /dev/null \
        selftest-second-invocation || selftest_status=$?
    [ "$selftest_status" -eq 0 ] &&
        selftest_assert_events "$selftest_second/events.tsv" selftest-second-invocation || {
        echo "FAIL second recorder invocation mixed event ownership" >&2; return 1;
    }

    run_interruption_identity_selftests "$selftest_root"
    INTERRUPTION_CAPTURE_SELFTEST_ACTIVE=0
    printf 'state session interruption capture selftest: 7 cases passed (status 0/1/2, recorder errors, same-handle replacement, event ownership)\n'
    run_interruption_real_control_event_selftests "$selftest_root"
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
    interruption_assertion_session_id=${6-}
    interruption_assertion_replace_from=${7:-}
    INTERRUPTION_RECORDER_ID=$interruption_assertion_session_id
    INTERRUPTION_ASSERTION_STATUS=2
    if [ ! -e "$INTERRUPTION_RECORDER_DIR/events.tsv" ]; then
        INTERRUPTION_RECORDER_STATUS=0
        INTERRUPTION_RECORDER_REASON=
    fi
    INTERRUPTION_CAPTURE_OUT="$INTERRUPTION_RECORDER_DIR/assertion.stdout"
    INTERRUPTION_CAPTURE_ERR="$INTERRUPTION_RECORDER_DIR/assertion.stderr"
    INTERRUPTION_CAPTURE_META="$INTERRUPTION_RECORDER_DIR/snapshot-metadata.tsv"

    case "$interruption_assertion_session_id" in
        ''|*[!A-Za-z0-9._-]*) mark_interruption_recorder_error session-id-invalid ;;
    esac

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

verify_interruption_real_control_events() {
    perl - "$@" <<'PERL'
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

my ($path, $wanted_id, $driver_pid, $host_pid, $harness_pid) = @ARGV;
reject("EVENT_IDENTITY") if !defined($wanted_id) || $wanted_id eq "" ||
    !positive_decimal($driver_pid) || !positive_decimal($host_pid) ||
    !positive_decimal($harness_pid);
open my $input, "<:raw", $path or reject("EVENT_FIELDS");
local $/;
my $bytes = <$input>;
close $input or reject("EVENT_FIELDS");
reject("EVENT_FIELDS") if !defined($bytes) || $bytes eq "" || $bytes !~ /\n\z/;
my @lines = split /\n/, $bytes, -1;
pop @lines;
reject("EVENT_FIELDS") if @lines != 21;

my @events = qw(driver-launch-start driver-launch-complete host-observed pre-signal-liveness
    signal-term wait-driver glob-candidate selected-path before-assertion capture-start
    snapshot-pre-start snapshot-pre-end assertion-start assertion-end snapshot-post-start
    snapshot-post-end capture-end assertion-decision quit-send-count host-after-wait lock-after-wait);
my @statuses = (0, 0, 0, 0, 0, 130, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, undef, 0);
my @pids = ($harness_pid, $driver_pid, $host_pid, $host_pid, ($driver_pid) x 5,
    ($harness_pid) x 8, $driver_pid, $driver_pid, $host_pid, $driver_pid);
my @details = ("none", "none", "none", "driver_pid=$driver_pid driver_liveness=0",
    "none", "none", "candidate=1", "selected-first-expanded-candidate", "none",
    ("none") x 10, "kill-0-failure-is-not-distinguished-from-permission-denial", "none");
reject("EVENT_FIELDS") if @events != 21 || @statuses != 21 || @pids != 21 || @details != 21;

my $previous_time;
for my $index (0 .. $#lines) {
    my $row = $index + 1;
    my @field = split /\t/, $lines[$index], -1;
    reject("EVENT_FIELDS") if @field != 8;
    reject("EVENT_IDENTITY") if $field[0] ne $wanted_id || $field[1] ne "harness" ||
        !positive_decimal($field[6]) || $field[6] ne "$pids[$index]";
    reject("EVENT_SEQUENCE") if !decimal($field[2]) || $field[2] ne "$row" ||
        $field[4] ne $events[$index];
    reject("EVENT_TIME") if !decimal($field[3]);
    reject("EVENT_TIME") if defined($previous_time) &&
        decimal_less_than($field[3], $previous_time);
    $previous_time = $field[3];
    if ($row == 20) {
        reject("EVENT_VALUE") if !positive_decimal($field[5]);
    } else {
        reject("EVENT_VALUE") if $field[5] ne "$statuses[$index]";
    }
    reject("EVENT_VALUE") if $field[7] ne $details[$index];
}
print "EVENTS_OK\n";
PERL
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

selftest_write_real_control_event_ledger() {
    real_event_dir=$1
    real_event_id=$2
    real_event_driver_pid=$3
    real_event_host_pid=$4
    real_event_harness_pid=$5
    mkdir -p "$real_event_dir" || return 1
    printf 'step\tclass\treason\nINTERRUPTED_TERM\tsignal\tTERM\n' \
        >"$real_event_dir/journal.tsv" || return 1
    INTERRUPTION_RECORDER_DIR=$real_event_dir
    INTERRUPTION_RECORDER_ID=$real_event_id
    INTERRUPTION_EVENT_SEQUENCE=0
    INTERRUPTION_RECORDER_STATUS=0
    INTERRUPTION_RECORDER_REASON=
    record_interruption_event "$real_event_dir" "$real_event_id" harness \
        driver-launch-start 0 "$real_event_harness_pid" || return 1
    record_interruption_event "$real_event_dir" "$real_event_id" harness \
        driver-launch-complete 0 "$real_event_driver_pid" || return 1
    record_interruption_event "$real_event_dir" "$real_event_id" harness \
        host-observed 0 "$real_event_host_pid" || return 1
    record_interruption_event "$real_event_dir" "$real_event_id" harness \
        pre-signal-liveness 0 "$real_event_host_pid" \
        "driver_pid=$real_event_driver_pid driver_liveness=0" || return 1
    record_interruption_event "$real_event_dir" "$real_event_id" harness \
        signal-term 0 "$real_event_driver_pid" || return 1
    record_interruption_event "$real_event_dir" "$real_event_id" harness \
        wait-driver 130 "$real_event_driver_pid" || return 1
    kill "$real_event_driver_pid" 2>/dev/null || return 1
    wait "$real_event_driver_pid" 2>/dev/null || true
    record_interruption_event "$real_event_dir" "$real_event_id" harness \
        glob-candidate 0 "$real_event_driver_pid" candidate=1 || return 1
    record_interruption_event "$real_event_dir" "$real_event_id" harness \
        selected-path 0 "$real_event_driver_pid" selected-first-expanded-candidate || return 1
    capture_interruption_boundary "$real_event_dir/journal.tsv" "$real_event_dir" \
        "$real_event_id" "$real_event_driver_pid" "$real_event_dir/assertion-reemitted.txt" || return 1
    [ "$INTERRUPTION_ASSERTION_STATUS" -eq 0 ] &&
        [ "$INTERRUPTION_RECORDER_STATUS" -eq 0 ] &&
        [ -s "$real_event_dir/snapshot-metadata.tsv" ] &&
        cmp -s "$real_event_dir/journal.tsv" "$real_event_dir/snapshot-pre.bin" &&
        cmp -s "$real_event_dir/journal.tsv" "$real_event_dir/snapshot-post.bin" || return 1
    record_interruption_event "$real_event_dir" "$real_event_id" harness \
        quit-send-count 1 "$real_event_driver_pid" || return 1
    kill "$real_event_host_pid" 2>/dev/null || return 1
    wait "$real_event_host_pid" 2>/dev/null || true
    record_interruption_event "$real_event_dir" "$real_event_id" harness \
        host-after-wait 1 "$real_event_host_pid" \
        kill-0-failure-is-not-distinguished-from-permission-denial || return 1
    record_interruption_event "$real_event_dir" "$real_event_id" harness \
        lock-after-wait 0 "$real_event_driver_pid" || return 1
    return 0
}

selftest_real_control_event_expected_ledger() {
    perl - "$@" <<'PERL'
use strict;
use warnings;
my ($path, $wanted_id, $driver_pid, $host_pid, $harness_pid) = @ARGV;
open my $input, "<:raw", $path or exit 1;
local $/;
my $bytes = <$input>;
close $input or exit 1;
exit 1 if !defined($bytes) || $bytes !~ /\n\z/;
my @rows = split /\n/, $bytes, -1;
pop @rows;
my @events = qw(driver-launch-start driver-launch-complete host-observed pre-signal-liveness
    signal-term wait-driver glob-candidate selected-path before-assertion capture-start
    snapshot-pre-start snapshot-pre-end assertion-start assertion-end snapshot-post-start
    snapshot-post-end capture-end assertion-decision quit-send-count host-after-wait lock-after-wait);
my @statuses = (0, 0, 0, 0, 0, 130, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0);
my @pids = ($harness_pid, $driver_pid, $host_pid, $host_pid, $driver_pid, $driver_pid,
    $driver_pid, $driver_pid, $driver_pid, ($harness_pid) x 8, $driver_pid, $driver_pid,
    $host_pid, $driver_pid);
my @details = ("none", "none", "none", "driver_pid=$driver_pid driver_liveness=0",
    "none", "none", "candidate=1", "selected-first-expanded-candidate", "none",
    ("none") x 10, "kill-0-failure-is-not-distinguished-from-permission-denial", "none");
exit 1 unless @rows == 21 && @events == 21 && @statuses == 21 && @pids == 21 && @details == 21;
my $previous_time;
for my $index (0 .. $#rows) {
    my @field = split /\t/, $rows[$index], -1;
    exit 1 unless @field == 8 && $field[0] eq $wanted_id && $field[1] eq "harness" &&
        $field[2] eq "" . ($index + 1) && $field[3] =~ /\A[0-9]+\z/ &&
        $field[4] eq $events[$index] && $field[5] eq "$statuses[$index]" &&
        $field[6] eq "$pids[$index]" && $field[7] eq $details[$index];
    exit 1 if defined($previous_time) && $field[3] < $previous_time;
    $previous_time = $field[3];
}
PERL
}

selftest_event_ledger_mutate() {
    perl - "$@" <<'PERL'
use strict;
use warnings;
my ($path, $mutation, $wrong_pid) = @ARGV;
open my $input, "<:raw", $path or die "open event ledger: $!";
local $/;
my $bytes = <$input>;
close $input or die "close event ledger: $!";
my $newline = defined($bytes) && $bytes =~ s/\n\z//;
my @rows = split /\n/, defined($bytes) ? $bytes : "", -1;
my @field = map { [ split /\t/, $_, -1 ] } @rows;
if ($mutation eq "event-omission-resequenced") {
    splice @field, 3, 1;
    $field[$_][2] = "" . ($_ + 1) for 0 .. $#field;
} elsif ($mutation eq "event-reorder-valid-sequence") {
    ($field[9][4], $field[10][4]) = ($field[10][4], $field[9][4]);
} elsif ($mutation eq "event-short-row") {
    pop @field;
} elsif ($mutation eq "event-extra-row") {
    my @extra = @{$field[-1]};
    $extra[2] = "" . (@field + 1);
    push @field, \@extra;
} elsif ($mutation eq "event-missing-field") {
    pop @{$field[2]};
} elsif ($mutation eq "event-extra-field") {
    push @{$field[2]}, "unexpected";
} elsif ($mutation eq "event-wrong-role") {
    $field[2][1] = "driver";
} elsif ($mutation eq "event-wrong-id") {
    $field[2][0] = "different-session";
} elsif ($mutation eq "event-wrong-pid") {
    $field[2][6] = $wrong_pid;
} elsif ($mutation eq "event-bad-status") {
    $field[5][5] = "129";
} elsif ($mutation eq "event-bad-detail") {
    $field[6][7] = "candidate=2";
} elsif ($mutation eq "event-decreasing-time") {
    die "invalid first event time" unless $field[0][3] =~ /\A[0-9]+\z/ && $field[0][3] gt "0";
    $field[1][3] = "" . ($field[0][3] - 1);
} else {
    die "unknown event-ledger mutation";
}
my $output = join("\n", map { join("\t", @$_) } @field);
$output .= "\n" if $newline;
open my $out, ">:raw", $path or die "open mutant ledger: $!";
print {$out} $output or die "write mutant ledger: $!";
close $out or die "close mutant ledger: $!";
PERL
}

selftest_event_only_field_changed() {
    perl - "$@" <<'PERL'
use strict;
use warnings;
my ($original_path, $mutant_path, $target_row, $target_column) = @ARGV;
sub read_rows {
    my ($path) = @_;
    open my $input, "<:raw", $path or return undef;
    local $/;
    my $bytes = <$input>;
    close $input or return undef;
    return undef if !defined($bytes) || $bytes !~ /\n\z/;
    my @rows = split /\n/, $bytes, -1;
    pop @rows;
    return \@rows;
}
my $original = read_rows($original_path);
my $mutant = read_rows($mutant_path);
exit 1 unless defined($original) && defined($mutant) && @$original == @$mutant;
my $changed = 0;
for my $row (0 .. $#$original) {
    my @left = split /\t/, $original->[$row], -1;
    my @right = split /\t/, $mutant->[$row], -1;
    exit 1 unless @left == 8 && @right == 8;
    if ($row == $target_row - 1) {
        exit 1 if $left[$target_column - 1] eq $right[$target_column - 1];
        $changed = 1;
    } else {
        exit 1 unless $original->[$row] eq $mutant->[$row];
    }
    splice @left, $target_column - 1, 1;
    splice @right, $target_column - 1, 1;
    exit 1 unless join("\t", @left) eq join("\t", @right);
}
exit($changed ? 0 : 1);
PERL
}

selftest_real_control_event_case() {
    real_event_root=$1
    real_event_name=$2
    real_event_report=${3:-0}
    real_event_case_dir="$real_event_root/real-control-events/$real_event_name"
    real_event_id="session-$(mono_ms)-$$-$real_event_name"
    if [ "$real_event_report" -eq 1 ]; then printf 'TAP version 13\n1..1\n'; fi
    mkdir -p "$real_event_case_dir" || {
        [ "$real_event_report" -eq 1 ] && printf 'Bail out! real event setup failed\n'
        return 1
    }
    sleep 60 &
    real_event_driver_pid=$!
    sleep 60 &
    real_event_host_pid=$!
    real_event_harness_pid=$$
    real_event_cleanup() {
        kill "$real_event_driver_pid" "$real_event_host_pid" 2>/dev/null || true
        wait "$real_event_driver_pid" 2>/dev/null || true
        wait "$real_event_host_pid" 2>/dev/null || true
    }
    if ! selftest_write_real_control_event_ledger "$real_event_case_dir" "$real_event_id" \
        "$real_event_driver_pid" "$real_event_host_pid" "$real_event_harness_pid" ||
        ! selftest_real_control_event_expected_ledger "$real_event_case_dir/events.tsv" \
            "$real_event_id" "$real_event_driver_pid" "$real_event_host_pid" "$real_event_harness_pid"; then
        real_event_cleanup
        [ "$real_event_report" -eq 1 ] && printf 'Bail out! actual 21-row recorder or independent positive oracle failed\n'
        return 1
    fi
    real_event_expected=EVENTS_OK
    case "$real_event_name" in
        event-valid-21-row-ledger) ;;
        event-omission-resequenced|event-short-row|event-extra-row|event-missing-field|event-extra-field)
            real_event_expected=EVENT_FIELDS ;;
        event-reorder-valid-sequence) real_event_expected=EVENT_SEQUENCE ;;
        event-wrong-role|event-wrong-id|event-wrong-pid) real_event_expected=EVENT_IDENTITY ;;
        event-bad-status|event-bad-detail) real_event_expected=EVENT_VALUE ;;
        event-decreasing-time) real_event_expected=EVENT_TIME ;;
        *) real_event_cleanup; [ "$real_event_report" -eq 1 ] && printf 'Bail out! unknown real event test\n'; return 1 ;;
    esac
    if [ "$real_event_name" != event-valid-21-row-ledger ]; then
        cp "$real_event_case_dir/events.tsv" "$real_event_case_dir/original.tsv" || {
            real_event_cleanup
            [ "$real_event_report" -eq 1 ] && printf 'Bail out! event ledger baseline copy failed\n'
            return 1
        }
        real_event_wrong_pid=$((real_event_driver_pid + 1))
        while [ "$real_event_wrong_pid" = "$real_event_driver_pid" ] ||
            [ "$real_event_wrong_pid" = "$real_event_host_pid" ] ||
            [ "$real_event_wrong_pid" = "$real_event_harness_pid" ]; do
            real_event_wrong_pid=$((real_event_wrong_pid + 1))
        done
        selftest_event_ledger_mutate "$real_event_case_dir/events.tsv" \
            "$real_event_name" "$real_event_wrong_pid" || {
            real_event_cleanup
            [ "$real_event_report" -eq 1 ] && printf 'Bail out! event ledger mutation failed\n'
            return 1
        }
        case "$real_event_name" in
            event-wrong-role) selftest_event_only_field_changed "$real_event_case_dir/original.tsv" "$real_event_case_dir/events.tsv" 3 2 || { real_event_cleanup; return 1; } ;;
            event-wrong-id) selftest_event_only_field_changed "$real_event_case_dir/original.tsv" "$real_event_case_dir/events.tsv" 3 1 || { real_event_cleanup; return 1; } ;;
            event-wrong-pid) selftest_event_only_field_changed "$real_event_case_dir/original.tsv" "$real_event_case_dir/events.tsv" 3 7 || { real_event_cleanup; return 1; } ;;
            event-bad-status) selftest_event_only_field_changed "$real_event_case_dir/original.tsv" "$real_event_case_dir/events.tsv" 6 6 || { real_event_cleanup; return 1; } ;;
            event-bad-detail) selftest_event_only_field_changed "$real_event_case_dir/original.tsv" "$real_event_case_dir/events.tsv" 7 8 || { real_event_cleanup; return 1; } ;;
            event-decreasing-time) selftest_event_only_field_changed "$real_event_case_dir/original.tsv" "$real_event_case_dir/events.tsv" 2 4 || { real_event_cleanup; return 1; } ;;
        esac
    fi
    real_event_status=0
    real_event_result=$(verify_interruption_real_control_events "$real_event_case_dir/events.tsv" \
        "$real_event_id" "$real_event_driver_pid" "$real_event_host_pid" \
        "$real_event_harness_pid") || real_event_status=$?
    real_event_cleanup
    if [ "$real_event_expected" = EVENTS_OK ]; then
        if [ "$real_event_status" -eq 0 ]; then
            [ "$real_event_report" -eq 1 ] && printf 'ok 1 - %s\n' "$real_event_name" ||
                printf 'real-control ledger selftest: PASS %s (EVENTS_OK)\n' "$real_event_name"
            return 0
        fi
    elif [ "$real_event_status" -ne 0 ] && [ "$real_event_result" = "$real_event_expected" ]; then
        [ "$real_event_report" -eq 1 ] && printf 'ok 1 - %s\n' "$real_event_name" ||
            printf 'real-control ledger selftest: PASS %s (%s)\n' "$real_event_name" "$real_event_expected"
        return 0
    fi
    [ "$real_event_report" -eq 1 ] && {
        printf '# expected=%s status=%s actual=%s status=%s\n' \
            "$real_event_expected" "$([ "$real_event_expected" = EVENTS_OK ] && printf 0 || printf 1)" \
            "$real_event_result" "$real_event_status"
        printf 'not ok 1 - %s\n' "$real_event_name"
    } || echo "FAIL real-control ledger $real_event_name: expected $real_event_expected, got $real_event_result (status $real_event_status)" >&2
    return 1
}

run_interruption_real_control_event_selftests() {
    real_event_test_root=$1
    for real_event_case in event-valid-21-row-ledger event-omission-resequenced \
        event-reorder-valid-sequence event-short-row event-extra-row event-missing-field \
        event-extra-field event-wrong-role event-wrong-id event-wrong-pid \
        event-bad-status event-bad-detail event-decreasing-time; do
        selftest_real_control_event_case "$real_event_test_root" "$real_event_case" 0 || return 1
    done
    printf 'state session real-control ledger selftest: 13 positive/negative cases passed\n'
}

capture_interruption_boundary() {
    interruption_boundary_journal=$1
    interruption_boundary_capture_dir=$2
    interruption_boundary_session_id=$3
    interruption_boundary_driver_pid=$4
    interruption_boundary_stdout=$5
    interruption_boundary_assertion_failure=0

    record_interruption_event "$interruption_boundary_capture_dir" \
        "$interruption_boundary_session_id" harness before-assertion 0 \
        "$interruption_boundary_driver_pid" || true
    capture_interruption_assertion interruption_grep INTERRUPTED_TERM \
        "$interruption_boundary_journal" "$interruption_boundary_capture_dir" \
        "$interruption_boundary_stdout" "$interruption_boundary_session_id" || true
    interruption_assertion_decision || interruption_boundary_assertion_failure=1
    record_interruption_event "$interruption_boundary_capture_dir" \
        "$interruption_boundary_session_id" harness assertion-decision \
        "$INTERRUPTION_ASSERTION_STATUS" "$interruption_boundary_driver_pid" || true

    return "$interruption_boundary_assertion_failure"
}

run_interruption_control() {
    interruption_fixture_dir=$1
    interrupt_dir="$WORK_ROOT/session-$$-interrupt"
    interrupt_ra="$interrupt_dir/ra"
    interrupt_capture_dir="$interrupt_dir/capture"
    interrupt_capture_id=${interrupt_dir##*/}
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
    interrupt_assertion_failure=0
    capture_interruption_boundary "$interrupt_journal" "$interrupt_capture_dir" \
        "$interrupt_capture_id" "$interrupt_driver" /dev/null ||
        interrupt_assertion_failure=1

    verify_interruption_snapshots "$interrupt_capture_dir/snapshot-metadata.tsv" ||
        mark_interruption_recorder_error snapshot-metadata-or-stability-invalid
    interrupt_trace_status=0
    interrupt_trace_result=$(verify_interruption_driver_trace \
        "$interrupt_trace" "$interrupt_capture_id" "$interrupt_driver") ||
        interrupt_trace_status=$?
    if [ "$interrupt_trace_status" -ne 0 ] || [ "$interrupt_trace_result" != TRACE_OK ]; then
        mark_interruption_recorder_error "driver-trace-${interrupt_trace_result:-TRACE_FIELDS}"
    fi
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
    if [ -e "$interrupt_dir/locks/nesturbator-retroarch-55355.lock" ] ||
        [ -L "$interrupt_dir/locks/nesturbator-retroarch-55355.lock" ]; then interrupt_lock_post=1; else interrupt_lock_post=0; fi
    record_interruption_event "$interrupt_capture_dir" "$interrupt_capture_id" \
        harness lock-after-wait "$interrupt_lock_post" "$interrupt_driver" || true
    [ "$interrupt_lock_post" -eq 0 ] || {
        echo "FAIL interruption left endpoint lock behind" >&2; return 1;
    }
    assert_sanitized_tree "$interrupt_attempt"

    interrupt_real_control_status=0
    interrupt_real_control_result=$(verify_interruption_real_control_events \
        "$interrupt_capture_dir/events.tsv" "$interrupt_capture_id" \
        "$interrupt_driver" "$interrupt_host" "$$") || interrupt_real_control_status=$?
    if [ "$interrupt_real_control_status" -ne 0 ]; then
        case "$interrupt_real_control_result" in
            EVENT_*) mark_interruption_recorder_error "harness-event-$interrupt_real_control_result" ;;
            *) mark_interruption_recorder_error harness-event-order-or-identity-invalid ;;
        esac
    fi
    interruption_recorder_gate || interrupt_recorder_failure=1
    [ "${interrupt_recorder_failure:-0}" -eq 0 ] && [ "$interrupt_assertion_failure" -eq 0 ] || return 1
    printf 'state session interruption control: TERM retained; one QUIT; host reaped; capture=usable; trace=3 owned events\n'
}

if [ "$CONTROL" = interruption-trace-lock-selftest ]; then
    selftest_regular_lock_trace_tap
    exit $?
fi

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


run_fault() {
    name=$1; expected=$2; fault=$3; fail_request=${4:-}; mode=${5:-prefix}; max_ms=${6:-20000}; ready_ms=${7:-30000}; publication_ms=${8:-1000}; attempt_override=${9:-}; operation_override=${10:-30000}
    case_dir=${STATE_SESSION_TEST_CASE_ROOT_OVERRIDE:-"$WORK_ROOT/session-$$-$name"}; case_ra="$case_dir/ra"
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
    primary_elapsed_file=${STATE_SESSION_TEST_PRIMARY_ELAPSED_FILE:-}
    (
    export STATE_SESSION_NC="$case_dir/fake-nc" \
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
    STATE_SESSION_TEST_CASE_ID="${STATE_SESSION_TEST_CASE_ID:-}" \
    STATE_SESSION_TEST_INVOCATION_ID="${STATE_SESSION_TEST_INVOCATION_ID:-}" \
    STATE_SESSION_TEST_CASE_ROOT="${STATE_SESSION_TEST_CASE_ROOT:-}" \
    STATE_SESSION_TEST_DRIVER_PID_FILE="${STATE_SESSION_TEST_DRIVER_PID_FILE:-$case_dir/driver.pid}"
    case_status=0
    if [ -n "${STATE_SESSION_TEST_CASE_ROOT:-}" ]; then
        sh "$DRIVER" "$case_dir/fake-retroarch" "$case_dir/config.cfg" "$case_dir/core.so" \
            "$fixture_dir/marker.nes" "$case_ra" "$case_dir/retroarch.log" "$FRAME_INDEX" \
            "$fixture_dir/calibration.txt" >"$case_dir/driver.stdout" 2>"$case_dir/driver.stderr" &
        case_driver_pid=$!
        printf '%s\n' "$case_driver_pid" >"$STATE_SESSION_TEST_DRIVER_PID_FILE"
        if [ -n "${STATE_SESSION_TEST_CASE_ID:-}" ]; then
            CLEANUP_LAUNCH_CASE="$STATE_SESSION_TEST_CASE_ID" CLEANUP_LAUNCH_INVOCATION="$STATE_SESSION_TEST_INVOCATION_ID" \
            CLEANUP_LAUNCH_ROOT="$STATE_SESSION_TEST_CASE_ROOT" CLEANUP_LAUNCH_PID="$case_driver_pid" \
            CLEANUP_LAUNCH_DRIVER="$DRIVER" CLEANUP_LAUNCH_SCRIPT="$script_path" CLEANUP_LAUNCH_RA="$case_ra" \
            CLEANUP_LAUNCH_FRAME="$FRAME_INDEX" CLEANUP_LAUNCH_FIXTURE="$fixture_dir" \
            CLEANUP_LAUNCH_LOCKS="$case_dir/locks" python3 - "$STATE_SESSION_TEST_CASE_ROOT/invocation.json" <<'PY'
import json, os, subprocess, sys
case=os.environ["CLEANUP_LAUNCH_CASE"]
root=os.path.realpath(os.environ["CLEANUP_LAUNCH_ROOT"])
driver=os.environ["CLEANUP_LAUNCH_DRIVER"]
script=os.environ["CLEANUP_LAUNCH_SCRIPT"]
fixture=os.environ["CLEANUP_LAUNCH_FIXTURE"]
ra=os.environ["CLEANUP_LAUNCH_RA"]
argv=["sh",driver,root+"/fake-retroarch",root+"/config.cfg",root+"/core.so",
      fixture+"/marker.nes",ra,root+"/retroarch.log",os.environ["CLEANUP_LAUNCH_FRAME"],fixture+"/calibration.txt"]
environment={"STATE_SESSION_TEST_CASE_ID":case,"STATE_SESSION_TEST_INVOCATION_ID":os.environ["CLEANUP_LAUNCH_INVOCATION"],
 "STATE_SESSION_TEST_CASE_ROOT":root,"STATE_SESSION_LOCK_ROOT":os.environ["CLEANUP_LAUNCH_LOCKS"]}
record={"case_id":case,"invocation_id":os.environ["CLEANUP_LAUNCH_INVOCATION"],"case_root":"cases/"+case,
 "driver_pid":int(os.environ["CLEANUP_LAUNCH_PID"]),
 "driver_blob":subprocess.check_output(["git","hash-object",driver],text=True).strip(),
 "harness_blob":subprocess.check_output(["git","hash-object",script],text=True).strip(),
 "argv":argv,"environment":environment,"files":{}}
open(sys.argv[1],"w").write(json.dumps(record,sort_keys=True)+"\n")
PY
        fi
        wait "$case_driver_pid" || case_status=$?
        cat "$case_dir/driver.stdout" "$case_dir/driver.stderr" >"$case_dir/driver.out"
    else
        sh "$DRIVER" "$case_dir/fake-retroarch" "$case_dir/config.cfg" "$case_dir/core.so" \
            "$fixture_dir/marker.nes" "$case_ra" "$case_dir/retroarch.log" "$FRAME_INDEX" \
            "$fixture_dir/calibration.txt" >"$case_dir/driver.out" 2>&1 || case_status=$?
    fi
    printf '%s\n' "$case_status" >"$case_dir/driver.status"
    case_driver_finished=$(mono_ms); case_driver_elapsed=$((case_driver_finished-case_driver_started))
    printf '%s\n' "$case_driver_elapsed" >"$case_dir/driver.elapsed"
    [ -z "$primary_elapsed_file" ] || printf '%s\n' "$case_driver_elapsed" >"$primary_elapsed_file"
    )
    case_status=$(cat "$case_dir/driver.status")
    case_driver_elapsed=$(cat "$case_dir/driver.elapsed")
    printf '%s\n' "$case_status" >"$case_dir/driver.status"
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
    STATE_SESSION_TEST_CASE_ID="${STATE_SESSION_TEST_CASE_ID:-}" \
    STATE_SESSION_TEST_INVOCATION_ID="${STATE_SESSION_TEST_INVOCATION_ID:-}" \
    STATE_SESSION_TEST_CASE_ROOT="${STATE_SESSION_TEST_CASE_ROOT:-}" \
    STATE_SESSION_TEST_LOCK_SNAPSHOT_FILE="${STATE_SESSION_TEST_LOCK_SNAPSHOT_FILE:-}" \
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

run_cleanup_request_failure_controls() {
    cleanup_matrix_root=$WORK_ROOT
    [ "$CONTROL" != all ] || cleanup_matrix_root="$WORK_ROOT/cleanup-request-failure-$$"
    mkdir -p "$cleanup_matrix_root"
    cleanup_matrix_root=$(cd "$cleanup_matrix_root" && pwd -P)
    if [ "$CONTROL" = cleanup-request-failure ]; then cleanup_phase=$CLEANUP_MATRIX_KIND; else cleanup_phase=green; fi
    if [ "$CONTROL" = all ]; then
        cp "$DRIVER" "$cleanup_matrix_root/state_session.sh"
        cp "$script_path" "$cleanup_matrix_root/test_state_session.sh"
        printf 'planned\n' >"$cleanup_matrix_root/phase-ledger.txt"
    fi
    CLEANUP_ROOT="$cleanup_matrix_root" CLEANUP_SCRIPT="$script_path" CLEANUP_DRIVER="$DRIVER" \
    CLEANUP_FRAME_INDEX="$FRAME_INDEX" CLEANUP_FIXTURE="$fixture_dir" CLEANUP_PHASE="$cleanup_phase" \
    CLEANUP_LEDGER="$cleanup_matrix_root/phase-ledger.txt" \
    python3 - <<'PY' || return 2
import json, os, pathlib, shutil, signal, subprocess, time
root=pathlib.Path(os.environ["CLEANUP_ROOT"]).resolve()
script=os.environ["CLEANUP_SCRIPT"]
driver=os.environ["CLEANUP_DRIVER"]
frame=os.environ["CLEANUP_FRAME_INDEX"]
fixture=os.environ["CLEANUP_FIXTURE"]
phase=os.environ["CLEANUP_PHASE"]
ledger_path=pathlib.Path(os.environ["CLEANUP_LEDGER"])
if ledger_path.read_text().splitlines()!=["planned"]: raise RuntimeError("cleanup slot is not fresh")
with ledger_path.open("a") as ledger: ledger.write("consumed\n")
names=["exit-failure"] if phase=="red" else ["exit-failure","signal-INT","signal-TERM","signal-HUP","writable-cleanup","foreign-endpoint","foreign-owner-replacement"]
case_rows=[]
def lock_record(path):
    p=pathlib.Path(path)
    try:
        st=p.lstat()
        if not p.is_file() or p.is_symlink(): return None
        return {"device":st.st_dev,"inode":st.st_ino,"token":p.read_text().strip()}
    except OSError: return None
def alive(pid):
    if not pid: return False
    try: os.kill(int(pid),0); return True
    except (ProcessLookupError, PermissionError, OSError): return False
def reason_from(case):
    journals=list((case/"ra"/"attempts").glob("attempt-*/journal.tsv"))
    if journals:
        for line in journals[0].read_text().splitlines()[1:]:
            fields=line.split("\t")
            if len(fields)==11 and fields[10] not in ("","OK"): return fields[10]
    for line in (case/"driver.stderr").read_text(errors="replace").splitlines():
        if line.startswith("state_session: "): return line.split(": ",1)[1]
    return ""
def foreign_fixture(case,token):
    (case/"foreign.sentinel").write_bytes((token+"\n").encode())
    (case/"foreign.sends").write_bytes(b"")
    (case/"foreign.signals").write_bytes(b"")
    pidfile=case/"foreign.pid"; done=case/"foreign.done"
    proc=subprocess.Popen(["sh","-c",'trap \'printf "INT\\n" >>"$3"\' INT; trap \'printf "TERM\\n" >>"$3"\' TERM; trap \'printf "HUP\\n" >>"$3"\' HUP; echo $$ > "$1"; while [ ! -e "$2" ]; do sleep 0.03; done',"foreign",str(pidfile),str(done),str(case/"foreign.signals")])
    deadline=time.monotonic()+2
    while not pidfile.exists() and time.monotonic()<deadline: time.sleep(.005)
    if not pidfile.exists(): raise RuntimeError("foreign fixture failed to publish PID")
    return proc,int(pidfile.read_text().strip()),done
for name in names:
    case=root/"cases"/name; case.mkdir(parents=True,exist_ok=False)
    case=case.resolve(); ra=case/"ra"; locks=case/"locks"
    (ra/"shots").mkdir(parents=True); (ra/"states").mkdir(); locks.mkdir()
    (case/"fake-nc").symlink_to(pathlib.Path(script)); (case/"fake-retroarch").symlink_to(pathlib.Path(script))
    for f in ["config.cfg","core.so","sends.log","host.pid","shot.count","read.count","state.count"]: (case/f).write_bytes(b"")
    invocation=f"{phase}:{name}:primary"
    foreign=None
    lock=locks/"nesturbator-retroarch-55355.lock"
    if name in ["foreign-endpoint","foreign-owner-replacement"]:
        foreign=foreign_fixture(case,"foreign-"+invocation)
    if name=="foreign-endpoint":
        lock.write_text("foreign-"+invocation+"\n")
        before=lock_record(lock)
        (case/"foreign-before.json").write_text(json.dumps({"pid":foreign[1],"live":alive(foreign[1]),"lock":before},sort_keys=True)+"\n")
        shutil.copyfile(lock,case/"foreign-bytes-before")
        control={"case_id":name,"invocation_id":invocation,"case_root":"cases/"+name,"producer":"harness-before-launch","mode":"occupied-endpoint","driver_pid":None,"transport_pid":None,"request_path":None,"request_before":None,"request_after":None,"request_writable":None,"marker":invocation+":occupied-endpoint"}
        (case/"control.json").write_text(json.dumps(control,sort_keys=True)+"\n")
    env=os.environ.copy()
    env.update({"STATE_SESSION_NC":str(case/"fake-nc"),"STATE_SESSION_TEST_READINESS_NC":str(case/"fake-nc"),
      "STATE_SESSION_TEST_FIXTURE":fixture,"STATE_SESSION_TEST_EXPECTED_DIR":str(ra/"states"),
      "STATE_SESSION_TEST_SENDS":str(case/"sends.log"),"STATE_SESSION_TEST_HOST_PID":str(case/"host.pid"),
      "STATE_SESSION_TEST_QUIT_FILE":str(case/"quit"),"STATE_SESSION_TEST_SHOT_COUNT":str(case/"shot.count"),
      "STATE_SESSION_TEST_READ_COUNT":str(case/"read.count"),"STATE_SESSION_TEST_STATE_COUNT":str(case/"state.count"),
      "STATE_SESSION_TEST_FAULT":"" if name=="foreign-endpoint" else "cleanup-request-failure",
      "STATE_SESSION_TEST_MODE":"prefix","STATE_SESSION_TEST_STOP_AFTER_PREFIX":"1",
      "STATE_SESSION_TEST_SHOTS":str(ra/"shots"),"STATE_SESSION_TEST_HOST_DIE":"0","STATE_SESSION_LOCK_ROOT":str(locks),
      "STATE_SESSION_TEST_CASE_ID":name,"STATE_SESSION_TEST_INVOCATION_ID":invocation,"STATE_SESSION_TEST_CASE_ROOT":str(case),
      "STATE_SESSION_TEST_DRIVER_PID_FILE":str(case/"driver.pid"),"STATE_SESSION_ATTEMPT_MS":"30000",
      "STATE_SESSION_OPERATION_MS":"30000","STATE_SESSION_CLEANUP_MS":"1000","STATE_SESSION_READY_MS":"30000",
      "STATE_SESSION_PUBLICATION_MS":"1000","STATE_SESSION_SETTLE_MS":"0","STATE_SESSION_COMMAND_GAP_MS":"100",
      "STATE_SESSION_PROGRESS_WAIT_MS":"20"})
    argv=["sh",driver,str(case/"fake-retroarch"),str(case/"config.cfg"),str(case/"core.so"),str(pathlib.Path(fixture)/"marker.nes"),str(ra),str(case/"retroarch.log"),frame,str(pathlib.Path(fixture)/"calibration.txt")]
    start=time.monotonic_ns(); proc=subprocess.Popen(argv,env=env,stdout=(case/"driver.stdout").open("wb"),stderr=(case/"driver.stderr").open("wb"))
    (case/"driver.pid").write_text(str(proc.pid)+"\n")
    selected_env={k:env[k] for k in ["STATE_SESSION_TEST_CASE_ID","STATE_SESSION_TEST_INVOCATION_ID","STATE_SESSION_TEST_CASE_ROOT","STATE_SESSION_LOCK_ROOT"]}
    launch={"case_id":name,"invocation_id":invocation,"case_root":"cases/"+name,"driver_pid":proc.pid,
      "driver_blob":subprocess.check_output(["git","hash-object",driver],text=True).strip(),
      "harness_blob":subprocess.check_output(["git","hash-object",script],text=True).strip(),
      "argv":argv,"environment":selected_env,"files":{}}
    (case/"invocation.json").write_text(json.dumps(launch,sort_keys=True)+"\n")
    watchdog=False; sent_signal=None
    if name.startswith("signal-"):
        deadline=time.monotonic()+8
        while (not (case/"control.json").is_file() or not (case/"armed").is_file()) and proc.poll() is None and time.monotonic()<deadline: time.sleep(.01)
        if not (case/"control.json").is_file() or not (case/"armed").is_file(): watchdog=True
        else:
            control=json.loads((case/"control.json").read_text()); hp=control.get("host_pid"); rp=control.get("transport_pid")
            time.sleep(.03)
            if not alive(hp) or not alive(rp) or not lock_record(lock): watchdog=True
            else:
                sent_signal=name.split("-",1)[1]
                proc.send_signal({"INT":signal.SIGINT,"TERM":signal.SIGTERM,"HUP":signal.SIGHUP}[sent_signal])
                control["signal"]=sent_signal
                (case/"control.json").write_text(json.dumps(control,sort_keys=True)+"\n")
    try: status=proc.wait(timeout=20)
    except subprocess.TimeoutExpired:
        watchdog=True; proc.send_signal(signal.SIGTERM)
        try: status=proc.wait(timeout=1)
        except subprocess.TimeoutExpired: proc.kill(); status=proc.wait()
    elapsed=(time.monotonic_ns()-start)//1_000_000
    (case/"driver.status").write_text(str(status)+"\n")
    if name!="foreign-endpoint":
        deadline=time.monotonic()+1
        while not (case/"control.json").is_file() and time.monotonic()<deadline: time.sleep(.005)
    control=json.loads((case/"control.json").read_text())
    control["driver_pid"]=proc.pid
    (case/"control.json").write_text(json.dumps(control,sort_keys=True)+"\n")
    host_pid=int((case/"host.pid").read_text().strip()) if (case/"host.pid").read_text().strip() else None
    receive_pid=int((case/"receive.pid").read_text().strip()) if (case/"receive.pid").exists() and (case/"receive.pid").read_text().strip() else control.get("transport_pid")
    host_live=alive(host_pid); child_live=alive(receive_pid)
    lock_after=lock_record(lock)
    journal=next(iter((ra/"attempts").glob("attempt-*/journal.tsv")),None)
    first=reason_from(case)
    expected=130 if name.startswith("signal-") else 1
    expected_reason=("INTERRUPTED_"+name.split("-",1)[1]) if name.startswith("signal-") else "ENDPOINT_BUSY" if name=="foreign-endpoint" else "READINESS_REPLY"
    if phase=="red":
        if name!="exit-failure" or first!="READINESS_REPLY" or not control.get("owned_live_before_injection") or \
           control.get("mode")!="request-directory" or control.get("request_after")!="directory" or \
           control.get("request_writable") is not False:
            raise RuntimeError("RED did not exercise the original READINESS_REPLY cleanup-write failure")
    if name in ["exit-failure","signal-INT","signal-TERM","signal-HUP","writable-cleanup","foreign-owner-replacement"]:
        received=case/"receive.pid"
        if not received.exists(): received.write_text(str(control.get("transport_pid") or "")+"\n")
    foreign_after=None; foreign_path=None
    if name=="foreign-endpoint": foreign_path=lock
    elif name=="foreign-owner-replacement":
        owners=list(locks.glob("*.lock.owner.*")); foreign_path=owners[0] if owners else None
    if foreign is not None:
        if name=="foreign-owner-replacement":
            before_file=case/"foreign-before.json"
            if not before_file.is_file(): raise RuntimeError("foreign owner replacement did not retain before measurement")
        if foreign_path is None: raise RuntimeError("foreign lock object missing")
        foreign_after={"pid":foreign[1],"live":alive(foreign[1]),"lock":lock_record(foreign_path)}
        (case/"foreign-after.json").write_text(json.dumps(foreign_after,sort_keys=True)+"\n")
        shutil.copyfile(foreign_path,case/"foreign-bytes-after")
        foreign[2].write_text("")
        try: foreign[0].wait(timeout=2)
        except subprocess.TimeoutExpired: foreign[0].terminate(); foreign[0].wait(timeout=2)
    foreign_preserved=True
    if foreign is not None:
        before=json.loads((case/"foreign-before.json").read_text())
        foreign_preserved=(foreign_after is not None and before["live"] is True and foreign_after["live"] is True and
          before["lock"]==foreign_after["lock"] and (case/"foreign-bytes-before").read_bytes()==(case/"foreign-bytes-after").read_bytes())
    owned_lock_after=None if name=="foreign-endpoint" else lock_after
    passed=(status==expected and first==expected_reason and not watchdog and not host_live and not child_live and
      ((lock_after is None) if name!="foreign-endpoint" else foreign_preserved))
    result={"name":name,"case_id":name,"invocation_id":invocation,"case_root":"cases/"+name,"driver_pid":proc.pid,
      "host_pid":host_pid,"receive_pid":receive_pid,"actual_status":status,"expected_status":expected,"first_reason":first,
      "elapsed_ms":int(elapsed),"watchdog_intervened":watchdog,"host_alive_before_emergency":host_live,
      "child_alive_before_emergency":child_live,"lock_owned_before_emergency":lock_after is not None and name!="foreign-endpoint",
      "owned_live_before_injection":bool(control.get("owned_live_before_injection",False)),
      "owned_lock_before_injection":control.get("owned_lock_before_injection"),"owned_lock_after":owned_lock_after,
      "injection_confirmed":name!="foreign-endpoint","request_directory":name in ["exit-failure","signal-INT","signal-TERM","signal-HUP"],
      "setup_ok":True,"passed":passed,"signal":sent_signal,"foreign_pid":foreign[1] if foreign else None,
      "foreign_path":str(foreign_path) if foreign_path else None,"journal":str(journal) if journal else None}
    (case/"primary-result.json").write_text(json.dumps(result,sort_keys=True)+"\n")
    if phase=="red":
        if passed or not (host_live or lock_after is not None): raise RuntimeError("RED did not prove the specific CR-01 cleanup failure")
        for pid in [host_pid,receive_pid]:
            if alive(pid):
                try: os.kill(pid,signal.SIGTERM)
                except OSError: pass
        token=lock_after.get("token") if lock_after else ""
        owner=locks/("nesturbator-retroarch-55355.lock.owner."+token) if token else None
        try:
            if owner and lock_record(owner) and lock_after and (lock_record(owner)["device"],lock_record(owner)["inode"])==(lock_after["device"],lock_after["inode"]):
                lock.unlink(missing_ok=True); owner.unlink(missing_ok=True)
        except OSError: pass
    elif not passed:
        raise RuntimeError("cleanup case failed: "+name+" status="+str(status)+" reason="+first)
    case_rows.append(name)
(root/"primary-cases.json").write_text(json.dumps(case_rows)+"\n")
PY
    if [ "$cleanup_phase" != red ]; then
        cleanup_already_root="$cleanup_matrix_root/cases/already-quit"
        mkdir -p "$cleanup_matrix_root/cases"
        mkdir "$cleanup_already_root" || return 2
        STATE_SESSION_TEST_CASE_ROOT_OVERRIDE="$cleanup_already_root" STATE_SESSION_TEST_CASE_ID=already-quit \
        STATE_SESSION_TEST_INVOCATION_ID="green:already-quit:primary" \
        STATE_SESSION_TEST_CASE_ROOT="$cleanup_already_root" \
        STATE_SESSION_TEST_DRIVER_PID_FILE="$cleanup_already_root/driver.pid" \
        STATE_SESSION_TEST_PRIMARY_ELAPSED_FILE="$cleanup_already_root/primary.elapsed" \
        run_fault quit-send-once QUIT_DEADLINE dropped-quit '' complete 250000 30000 1000 240000 \
            >"$cleanup_already_root/harness.stdout" 2>"$cleanup_already_root/harness.stderr" || return 2
        for cleanup_name in exit-failure signal-INT signal-TERM signal-HUP writable-cleanup already-quit foreign-owner-replacement; do
            cleanup_case_root="$cleanup_matrix_root/cases/$cleanup_name"
            cleanup_reacq="$cleanup_case_root/reacquisition"
            mkdir -p "$cleanup_reacq/ra/shots" "$cleanup_reacq/ra/states" "$cleanup_reacq/locks"
            ln -s "$script_path" "$cleanup_reacq/fake-nc"
            ln -s "$script_path" "$cleanup_reacq/fake-retroarch"
            : >"$cleanup_reacq/config.cfg"; : >"$cleanup_reacq/core.so"; : >"$cleanup_reacq/host.pid"
            cleanup_reacq_started=$(mono_ms); cleanup_reacq_status=0
            STATE_SESSION_TEST_CASE_ID="$cleanup_name" \
            STATE_SESSION_TEST_INVOCATION_ID="green:$cleanup_name:reacquisition" \
            STATE_SESSION_TEST_CASE_ROOT="$cleanup_case_root" \
            STATE_SESSION_TEST_LOCK_SNAPSHOT_FILE="$cleanup_reacq/lock.json" \
            run_prefix_driver "$cleanup_reacq" "$cleanup_reacq/ra" '' "$cleanup_case_root/locks" \
                >"$cleanup_reacq/stdout.log" 2>"$cleanup_reacq/stderr.log" || cleanup_reacq_status=$?
            cleanup_reacq_elapsed=$(($(mono_ms)-cleanup_reacq_started))
            cleanup_reacq_host=$(cat "$cleanup_reacq/host.pid" 2>/dev/null || true)
            printf '%s\n' "$cleanup_reacq_status" >"$cleanup_reacq/status.txt"
            printf '%s\n' "$cleanup_reacq_elapsed" >"$cleanup_reacq/elapsed.txt"
            printf '%s\n' "$cleanup_reacq_host" >"$cleanup_reacq/host-pid.txt"
            [ "$cleanup_reacq_status" -eq 0 ] && [ "$cleanup_reacq_elapsed" -lt 20000 ] && \
                [ -s "$cleanup_reacq/lock.json" ] && [ -n "$cleanup_reacq_host" ] && \
                ! kill -0 "$cleanup_reacq_host" 2>/dev/null || {
                echo "FAIL $cleanup_name endpoint reacquisition status=$cleanup_reacq_status elapsed=${cleanup_reacq_elapsed}ms" >&2; return 1;
            }
        done
    fi
    if [ "$cleanup_phase" = red ]; then
        printf '1\n' >"$cleanup_matrix_root/wait-status.txt"
    else
        printf '0\n' >"$cleanup_matrix_root/wait-status.txt"
    fi
    if [ "$cleanup_phase" = red ]; then cleanup_source_commit=772dbb0fdaedb3d8af52beeeab854943c867441b
    else cleanup_source_commit=$(git rev-parse HEAD); fi
    CLEANUP_ROOT="$cleanup_matrix_root" CLEANUP_PHASE="$cleanup_phase" CLEANUP_MATRIX_STARTED="${CLEANUP_MATRIX_STARTED:-$(mono_ms)}" \
    CLEANUP_SOURCE_COMMIT="$cleanup_source_commit" CLEANUP_DRIVER="$DRIVER" CLEANUP_FIXTURE="$fixture_dir" \
    CLEANUP_FRAME_INDEX="$FRAME_INDEX" CLEANUP_DRIVER_SOURCE="tests/retroarch/state_session.sh" \
    CLEANUP_HARNESS_SOURCE="tests/retroarch/test_state_session.sh" python3 - <<'PY' || return 2
import hashlib, json, os, pathlib, subprocess, time
root=pathlib.Path(os.environ["CLEANUP_ROOT"]).resolve(); phase=os.environ["CLEANUP_PHASE"]
driver=os.environ["CLEANUP_DRIVER"]
names=["exit-failure","signal-INT","signal-TERM","signal-HUP","writable-cleanup","already-quit","foreign-endpoint","foreign-owner-replacement"]
if phase=="red": names=names[:1]
def sha(path): return hashlib.sha256(pathlib.Path(path).read_bytes()).hexdigest()
def git(*args): return subprocess.check_output(["git",*args],text=True).strip()
def rel(p): return pathlib.Path(p).resolve().relative_to(root).as_posix()
def lock(path):
    p=pathlib.Path(path)
    try:
        st=p.lstat()
        if not p.is_file() or p.is_symlink(): return None
        return {"device":st.st_dev,"inode":st.st_ino,"token":p.read_text().strip()}
    except OSError: return None
def live(pid):
    if not pid: return False
    try: os.kill(int(pid),0); return True
    except (ProcessLookupError,PermissionError,OSError): return False
rows=[]; primary={}
for name in names:
    case=root/"cases"/name
    if name=="already-quit":
        control=json.loads((case/"control.json").read_text()); host_text=(case/"host.pid").read_text().strip()
        host=int(host_text) if host_text else None; receive=control.get("transport_pid")
        journal=next(iter((case/"ra"/"attempts").glob("attempt-*/journal.tsv")),None)
        reason=""
        if journal:
            for line in journal.read_text().splitlines()[1:]:
                f=line.split("\t")
                if len(f)==11 and f[10] not in ("","OK"): reason=f[10]; break
        elapsed=int((case/"primary.elapsed").read_text().strip()); status=int((case/"driver.status").read_text().strip())
        raw={"name":name,"case_id":name,"invocation_id":"green:already-quit:primary","case_root":"cases/"+name,
          "driver_pid":int((case/"driver.pid").read_text().strip()),"host_pid":host,"receive_pid":receive,
          "actual_status":status,"expected_status":1,"first_reason":reason,"elapsed_ms":elapsed,"watchdog_intervened":False,
          "host_alive_before_emergency":live(host),"child_alive_before_emergency":live(receive),
          "lock_owned_before_emergency":lock(case/"locks"/"nesturbator-retroarch-55355.lock") is not None,
          "owned_live_before_injection":bool(control.get("owned_live_before_injection")),
          "owned_lock_before_injection":control.get("owned_lock_before_injection"),
          "owned_lock_after":lock(case/"locks"/"nesturbator-retroarch-55355.lock"),"injection_confirmed":True,
          "request_directory":False,"setup_ok":True,"passed":status==1 and reason=="QUIT_DEADLINE",
          "signal":None,"foreign_pid":None,"journal":str(journal) if journal else None}
        (case/"primary-result.json").write_text(json.dumps(raw,sort_keys=True)+"\n")
    else: raw=json.loads((case/"primary-result.json").read_text())
    primary[name]=raw
for name in names:
    case=root/"cases"/name; raw=primary[name]; invocation=raw["invocation_id"]
    control_path=case/"control.json"; control=json.loads(control_path.read_text())
    status=int((case/"driver.status").read_text().strip()) if (case/"driver.status").exists() else int(raw["actual_status"])
    (case/"driver.status").write_text(str(status)+"\n")
    sends=case/"sends.log"; sends.touch(exist_ok=True)
    hostfile=case/"host.pid"; hostfile.touch(exist_ok=True)
    std_out=case/"driver.stdout"; std_err=case/"driver.stderr"
    for f in [std_out,std_err]: f.touch(exist_ok=True)
    journal=pathlib.Path(raw["journal"]) if raw.get("journal") else None
    host=raw.get("host_pid"); receive=raw.get("receive_pid")
    control_live=json.loads(control_path.read_text())
    reacq=None
    if name not in ["foreign-endpoint"] and phase!="red":
        rd=case/"reacquisition"; lockfile=rd/"lock.json"
        reacq={"status":int((rd/"status.txt").read_text().strip()),"lock":json.loads(lockfile.read_text()),
          "host_pid":int((rd/"host-pid.txt").read_text().strip()),"host_live_after":False,
          "elapsed_ms":int((rd/"elapsed.txt").read_text().strip())}
        if reacq["status"]!=0 or reacq["elapsed_ms"]>=20000: raise RuntimeError("endpoint reacquisition failed: "+name)
    owned_before=raw.get("owned_lock_before_injection")
    if name=="already-quit" and owned_before is None: raise RuntimeError("already-quit lock was not measured")
    life={"owned_processes":[],"owned_lock_before":owned_before,
      "owned_lock_after":raw.get("owned_lock_after"),"reacquisition":reacq}
    if host:
        life["owned_processes"].append({"pid":host,"live_before":bool(control_live.get("host_live_before_injection")),
          "live_after":bool(raw["host_alive_before_emergency"])})
    if receive and name!="already-quit":
        life["owned_processes"].append({"pid":receive,"live_before":bool(control_live.get("receive_live_before_injection")),
          "live_after":bool(raw["child_alive_before_emergency"])})
    if name=="already-quit" and receive:
        life["owned_processes"].append({"pid":receive,"live_before":bool(control_live.get("receive_live_before_injection")),
          "live_after":bool(raw["child_alive_before_emergency"])})
    if name=="foreign-owner-replacement": life["owned_sidecar_original"]=json.loads((case/"owned-sidecar-original.json").read_text())
    if name in ["foreign-endpoint","foreign-owner-replacement"]:
        life["owned_lock_before"]=None if name=="foreign-endpoint" else owned_before
        life["owned_lock_after"]=None
        life["reacquisition"]=None if name=="foreign-endpoint" else reacq
        life["foreign_pid"]=raw["foreign_pid"]
    life_path=case/"lifecycle.json"; life_path.write_text(json.dumps(life,sort_keys=True)+"\n")
    o={"case_id":name,"invocation_id":invocation,"case_root":"cases/"+name,
      "setup_ok":True,"injection_confirmed":name!="foreign-endpoint",
      "owned_live_before_injection":bool(raw.get("owned_live_before_injection")),
      "driver_pid":int(raw["driver_pid"]),"host_pid":host,"receive_pid":receive,
      "request_directory":bool(raw.get("request_directory")),"expected_status":int(raw["expected_status"]),
      "actual_status":status,"first_reason":raw["first_reason"],
      "host_alive_before_emergency":bool(raw["host_alive_before_emergency"]),
      "child_alive_before_emergency":bool(raw["child_alive_before_emergency"]),
      "lock_owned_before_emergency":bool(raw["lock_owned_before_emergency"]),
      "watchdog_intervened":bool(raw["watchdog_intervened"]),"elapsed_ms":int(raw["elapsed_ms"]),
      "passed":bool(raw["passed"]),"quit_send_count":(case/"sends.log").read_text().splitlines().count("QUIT"),
      "control_file":rel(control_path),"driver_stdout_file":rel(std_out),"driver_stderr_file":rel(std_err),
      "driver_status_file":rel(case/"driver.status"),"host_pid_file":rel(hostfile),"sends_file":rel(sends),"lifecycle_file":rel(life_path)}
    if name!="foreign-endpoint":
        o["consumed_file"]=control["consumed_file"]; o["reply_file"]=control["reply_file"]
        if journal: o["cleanup_journal_file"]=rel(journal)
    if name in ["exit-failure","signal-INT","signal-TERM","signal-HUP"]:
        o["diagnostic_file"]=rel(std_err)
        if phase=="green":
            quit_rows=[line.split("\t") for line in journal.read_text().splitlines()[1:] if line.split("\t")[1]=="QUIT"]
            if len(quit_rows)!=1 or quit_rows[0][0]!="cleanup-quit" or quit_rows[0][2]!="0" or quit_rows[0][10]!="CLEANUP_REQUEST_WRITE_FAILED":
                raise RuntimeError("cleanup request failure lacks exact zero-send diagnostic row: "+name)
            if o["quit_send_count"]!=0 or "CLEANUP_REQUEST_WRITE_FAILED" not in std_err.read_text():
                raise RuntimeError("cleanup request failure lacks diagnostic or sent QUIT: "+name)
    if reacq:
        rd=case/"reacquisition"; o.update({"reacquisition_status_file":rel(rd/"status.txt"),
          "reacquisition_host_pid_file":rel(rd/"host-pid.txt"),"reacquisition_lock_file":rel(rd/"lock.json")})
    if name in ["foreign-endpoint","foreign-owner-replacement"]:
        o.update({"foreign_before_file":rel(case/"foreign-before.json"),"foreign_after_file":rel(case/"foreign-after.json"),
          "foreign_bytes_before_file":rel(case/"foreign-bytes-before"),"foreign_bytes_after_file":rel(case/"foreign-bytes-after"),
          "foreign_sends_file":rel(case/"foreign.sends"),"foreign_signals_file":rel(case/"foreign.signals")})
    observation=case/"observation.json"; observation.write_text(json.dumps(o,sort_keys=True)+"\n")
    roles={k:v for k,v in o.items() if k.endswith("_file")}; roles["observation_file"]=rel(observation)
    filemap={role:{"path":ref,"sha256":sha(root/ref)} for role,ref in roles.items()}
    case_root=root/"cases"/name
    argv=["sh",driver,str(case_root/"fake-retroarch"),str(case_root/"config.cfg"),str(case_root/"core.so"),
      str(pathlib.Path(os.environ["CLEANUP_FIXTURE"])/"marker.nes"),str(case_root/"ra"),str(case_root/"retroarch.log"),
      os.environ["CLEANUP_FRAME_INDEX"],str(pathlib.Path(os.environ["CLEANUP_FIXTURE"])/"calibration.txt")]
    env={"STATE_SESSION_TEST_CASE_ID":name,"STATE_SESSION_TEST_INVOCATION_ID":invocation,
      "STATE_SESSION_TEST_CASE_ROOT":str(case_root.resolve()),"STATE_SESSION_LOCK_ROOT":str(case_root/"locks")}
    launch={"case_id":name,"invocation_id":invocation,"case_root":"cases/"+name,"driver_pid":int(raw["driver_pid"]),
      "driver_blob":git("hash-object",str(root/"state_session.sh")),
      "harness_blob":git("hash-object",str(root/"test_state_session.sh")),"argv":argv,"environment":env,"files":filemap}
    invocation_file=case/"invocation.json"
    existing_launch=json.loads(invocation_file.read_text())
    if existing_launch["driver_pid"]!=launch["driver_pid"] or existing_launch["argv"]!=argv or existing_launch["environment"]!=env:
        raise RuntimeError("final invocation record did not match the actual launch: "+name)
    existing_launch.update({"driver_blob":launch["driver_blob"],"harness_blob":launch["harness_blob"],"files":filemap})
    invocation_file.write_text(json.dumps(existing_launch,sort_keys=True)+"\n")
    rows.append({"name":name,"case_id":name,"invocation_id":invocation,"case_root":"cases/"+name,
      "observation_file":rel(observation),"invocation_file":rel(invocation_file)})
    if phase=="green" and not o["passed"]: raise RuntimeError("saved cleanup case did not pass: "+name)
for name in names:
    if name=="foreign-endpoint": continue
    life=json.loads((root/"cases"/name/"lifecycle.json").read_text())
    if life["reacquisition"]:
        old=life["owned_lock_before"]; new=life["reacquisition"]["lock"]
        if old is None or old["token"]==new["token"] or old==new: raise RuntimeError("reacquired lock identity did not advance: "+name)
    (root/"cases"/name/"lifecycle.json").write_text(json.dumps(life,sort_keys=True)+"\n")
if phase!="red":
    rows.sort(key=lambda x:names.index(x["name"]))
for row in rows:
    row["invocation_file"]=row["invocation_file"]
elapsed=int(subprocess.check_output(["perl","-MTime::HiRes=clock_gettime,CLOCK_MONOTONIC","-e",'printf "%.0f\\n",clock_gettime(CLOCK_MONOTONIC)*1000'],text=True).strip())-int(os.environ["CLEANUP_MATRIX_STARTED"])
complete=0
if phase!="red": complete=int((root/"cases"/"already-quit"/"primary.elapsed").read_text().strip())
if not 0<elapsed<=420000 or not 0<=elapsed-complete<=90000: raise RuntimeError("cleanup matrix exceeded measured wall budget")
files={}
for path in sorted(root.rglob("*")):
    if path.is_file() and path.name!="evidence.json": files[path.relative_to(root).as_posix()]=sha(path)
ledger=(root/"phase-ledger.txt").read_text().splitlines()
if ledger!=["planned","consumed"]: raise RuntimeError("cleanup matrix slot ledger did not begin exactly once")
with (root/"phase-ledger.txt").open("a") as ledger_file: ledger_file.write("completed\n")
ledger.append("completed")
files["phase-ledger.txt"]=sha(root/"phase-ledger.txt")
e={"ledger":ledger,"source_commit":os.environ["CLEANUP_SOURCE_COMMIT"],
  "driver_blob":git("hash-object",str(root/"state_session.sh")),
  "harness_blob":git("hash-object",str(root/"test_state_session.sh")),
  "status":"EXPECTED_CR01_FAILURE" if phase=="red" else "PASS","exit":1 if phase=="red" else 0,
  "elapsed_ms":elapsed,"cases":rows,"file_sha256":files}
(root/"evidence.json").write_text(json.dumps(e,sort_keys=True,indent=2)+"\n")
PY
    if [ "$cleanup_phase" = red ]; then
        echo 'state session cleanup request failure controls: CR-01 reproduced; pre-emergency host/lock evidence retained'
        return 1
    fi
    echo 'state session cleanup request failure controls: 8 cases passed.'
}


script_path=$(cd "$(dirname "$0")" && pwd -P)/$(basename "$0")
test_started=$(mono_ms)
if [ "$CONTROL" = all ] || [ "$CONTROL" = cleanup-request-failure ]; then CLEANUP_MATRIX_STARTED=$(mono_ms); fi
if [ "$CONTROL" = cleanup-request-failure ]; then
    cp "$DRIVER" "$WORK_ROOT/state_session.sh"
    cp "$script_path" "$WORK_ROOT/test_state_session.sh"
    printf 'planned\n' >"$WORK_ROOT/phase-ledger.txt"
fi
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
if [ "$CONTROL" = cleanup-request-failure ]; then
    cleanup_control_status=0
    run_cleanup_request_failure_controls || cleanup_control_status=$?
    exit "$cleanup_control_status"
fi
if [ "$CONTROL" = all ]; then run_cleanup_request_failure_controls || exit $?; fi

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

run_startup_lock_signal_control() {
    startup_signal_dir="$WORK_ROOT/session-$$-startup-lock-signal"
    startup_signal_ra="$startup_signal_dir/ra"
    startup_signal_locks="$startup_signal_dir/locks"
    startup_signal_bin="$startup_signal_dir/bin"
    startup_signal_lock="$startup_signal_locks/nesturbator-retroarch-55355.lock"
    startup_signal_real_mkdir=$(command -v mkdir)
    startup_signal_real_ln=$(command -v ln)
    mkdir -p "$startup_signal_ra/shots" "$startup_signal_ra/states" "$startup_signal_bin"
    ln -sf "$script_path" "$startup_signal_dir/fake-nc"
    ln -sf "$script_path" "$startup_signal_dir/fake-retroarch"
    : >"$startup_signal_dir/config.cfg"
    : >"$startup_signal_dir/core.so"
    cat >"$startup_signal_bin/mkdir" <<'EOF'
#!/bin/sh
if [ "$#" -eq 1 ] && [ "$1" = "${STATE_SESSION_TEST_SIGNAL_LOCK:-}" ]; then
    "$STATE_SESSION_TEST_REAL_MKDIR" "$@" || exit $?
    kill -TERM "$PPID" 2>/dev/null || true
    exit 0
fi
exec "$STATE_SESSION_TEST_REAL_MKDIR" "$@"
EOF
    cat >"$startup_signal_bin/ln" <<'EOF'
#!/bin/sh
signal_lock=0
case "$#" in
    2) [ "$2" = "${STATE_SESSION_TEST_SIGNAL_LOCK:-}" ] && signal_lock=1 ;;
    3) [ "$1" = '-s' ] && [ "$3" = "${STATE_SESSION_TEST_SIGNAL_LOCK:-}" ] && signal_lock=1 ;;
esac
if [ "$signal_lock" -eq 1 ]; then
    "$STATE_SESSION_TEST_REAL_LN" "$@" || exit $?
    kill -TERM "$PPID" 2>/dev/null || true
    exit 0
fi
exec "$STATE_SESSION_TEST_REAL_LN" "$@"
EOF
    chmod +x "$startup_signal_bin/mkdir" "$startup_signal_bin/ln"
    startup_signal_status=0
    (
        PATH="$startup_signal_bin:$PATH"
        export PATH
        STATE_SESSION_TEST_SIGNAL_LOCK="$startup_signal_lock"
        STATE_SESSION_TEST_REAL_MKDIR="$startup_signal_real_mkdir"
        STATE_SESSION_TEST_REAL_LN="$startup_signal_real_ln"
        export STATE_SESSION_TEST_SIGNAL_LOCK STATE_SESSION_TEST_REAL_MKDIR STATE_SESSION_TEST_REAL_LN
        run_prefix_driver "$startup_signal_dir" "$startup_signal_ra" '' "$startup_signal_locks"
    ) >"$startup_signal_dir/driver.out" 2>&1 || startup_signal_status=$?
    [ "$startup_signal_status" -eq 143 ] || {
        echo "FAIL startup TERM status: expected=143 actual=$startup_signal_status" >&2
        cat "$startup_signal_dir/driver.out" >&2
        exit 1
    }
    [ ! -e "$startup_signal_lock" ] && [ ! -L "$startup_signal_lock" ] || {
        echo 'FAIL startup TERM left endpoint lock behind' >&2
        exit 1
    }
    run_prefix_driver "$startup_signal_dir" "$startup_signal_ra" '' "$startup_signal_locks"
    [ -s "$startup_signal_dir/sends.log" ] &&
        [ ! -e "$startup_signal_lock" ] && [ ! -L "$startup_signal_lock" ] || {
        echo 'FAIL later session could not acquire the startup-signal lock' >&2
        exit 1
    }
    printf 'state session startup lock signal control: TERM status 143; owned lock released; later acquisition succeeded\n'
}

if [ "$CONTROL" = readiness ]; then
    run_startup_lock_signal_control
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
while [ ! -e "$shared_lock_root/nesturbator-retroarch-55355.lock" ] &&
    [ ! -L "$shared_lock_root/nesturbator-retroarch-55355.lock" ] && [ "$_tries" -lt 200 ]; do
    kill -0 "$first_driver" 2>/dev/null || { echo "FAIL first concurrent session exited before lock acquisition" >&2; cat "$first_dir/driver.out" >&2; exit 1; }
    sleep 0.02; _tries=$((_tries+1))
done
[ -e "$shared_lock_root/nesturbator-retroarch-55355.lock" ] ||
    [ -L "$shared_lock_root/nesturbator-retroarch-55355.lock" ] || {
    echo "FAIL first concurrent session never held endpoint lock" >&2; exit 1;
}
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
