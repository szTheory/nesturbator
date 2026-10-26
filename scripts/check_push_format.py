#!/usr/bin/env python3
"""Check outgoing committed source blobs with the repository's clang-format 18."""

import os
from pathlib import Path
import re
import shutil
import subprocess
import sys


def git(*args):
    return subprocess.check_output(["git", *args])


def formatter():
    configured = os.environ.get("CLANG_FORMAT")
    candidates = [configured] if configured else [
        shutil.which("clang-format-18"), shutil.which("clang-format")
    ]
    if not configured and shutil.which("brew"):
        prefix = subprocess.run(
            ["brew", "--prefix", "llvm@18"], capture_output=True, text=True, check=False
        )
        if prefix.returncode == 0:
            candidates.append(str(Path(prefix.stdout.strip()) / "bin" / "clang-format"))
    for candidate in candidates:
        if not candidate:
            continue
        version = subprocess.run(
            [candidate, "--version"], capture_output=True, text=True, check=False
        )
        if version.returncode == 0 and re.search(r"\bversion 18\.", version.stdout):
            return candidate
    raise RuntimeError("clang-format 18 is required; set CLANG_FORMAT to its executable")


def main():
    os.chdir(os.fsdecode(git("rev-parse", "--show-toplevel").strip()))
    commits = git("rev-list", "--branches", "--not", "--remotes").splitlines()
    blobs = {}
    for commit in commits:
        revision = commit.decode("ascii")
        paths = git(
            "diff-tree", "--root", "--no-commit-id", "--name-only", "-r", "-m", "-z",
            "--diff-filter=ACMRT", revision, "--", "*.c", "*.h", "*.cpp"
        ).split(b"\0")
        for path in paths:
            if not path or path == b"libretro/libretro.h":
                continue
            spec = commit + b":" + path
            # Reject links and submodules rather than reading outside a commit.
            entry = git("ls-tree", "-z", revision, "--", os.fsdecode(path)).split(b"\t", 1)[0]
            mode, kind, blob = entry.split()
            if mode not in (b"100644", b"100755") or kind != b"blob":
                raise RuntimeError("source is not a regular committed file: " + repr(path))
            blobs[(blob, path)] = spec
    if not blobs:
        print("pre-push format: no outgoing C/C++ changes")
        return 0
    tool = formatter()
    failed = False
    for (_, path), spec in blobs.items():
        result = subprocess.run(
            [tool, "--dry-run", "--Werror", "--assume-filename=" + os.fsdecode(path)],
            input=git("show", os.fsdecode(spec)), check=False
        )
        if result.returncode:
            print("pre-push format: unformatted committed source " + repr(spec), file=sys.stderr)
            failed = True
    if not failed:
        print(f"pre-push format: {len(blobs)} committed source blobs formatted")
    return int(failed)


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, RuntimeError, subprocess.CalledProcessError) as error:
        print(f"pre-push format: {error}", file=sys.stderr)
        sys.exit(1)
