"""Exercise the pre-push blob boundary in a scratch Git repository."""

import os
from pathlib import Path
import shutil
import subprocess
import sys


helper = Path(sys.argv[1]).resolve()
work = Path(sys.argv[2]).resolve()
if work.exists():
    shutil.rmtree(work)
work.mkdir(parents=True)


def git(*args):
    return subprocess.check_output(["git", "-C", str(work), *args], stderr=subprocess.STDOUT)


def commit(message):
    git("add", "--all")
    git("-c", "core.hooksPath=/dev/null", "-c", "commit.gpgsign=false", "commit", "-qm", message)


git("init", "-q")
git("config", "user.name", "Fixture")
git("config", "user.email", "fixture@users.noreply.github.com")
(work / "README").write_text("base\n")
commit("base")
git("update-ref", "refs/remotes/origin/main", "HEAD")

# A controlled formatter isolates the hook's job (choosing exact committed
# bytes and enforcing version/options) from clang-format's own formatting rules.
tool = work / "formatter fixture"
tool.write_text(
    "#!" + sys.executable + "\n"
    "import sys\n"
    "if '--version' in sys.argv:\n"
    " print('clang-format version 18.1.8'); sys.exit(0)\n"
    "assert '--dry-run' in sys.argv and '--Werror' in sys.argv\n"
    "assert any(a.startswith('--assume-filename=') for a in sys.argv)\n"
    "sys.exit(1 if b'int  bad' in sys.stdin.buffer.read() else 0)\n"
)
tool.chmod(0o755)
env = dict(os.environ, CLANG_FORMAT=str(tool))


def check(success, expected):
    result = subprocess.run(
        [sys.executable, str(helper)], cwd=work, env=env,
        capture_output=True, text=True, check=False
    )
    assert (result.returncode == 0) == success, result.stdout + result.stderr
    assert expected in result.stdout + result.stderr, result.stdout + result.stderr


check(True, "no outgoing C/C++ changes")
paths = ["space name.c", "line\nbreak.h", "dollar$(literal).cpp"]
for name in paths:
    (work / name).write_text("int good;\n")
(work / "libretro").mkdir()
(work / "libretro/libretro.h").write_text("int  bad;\n")
commit("sources")
(work / paths[0]).write_text("int  bad;\n")
check(True, "3 committed source blobs formatted")
(work / paths[0]).write_text("int good;\n")
git("update-ref", "refs/remotes/origin/main", "HEAD")
(work / paths[0]).write_text("int  bad;\n")
commit("bad source")
(work / paths[0]).write_text("int good;\n")
check(False, "unformatted committed source")
# Fixing the tip does not hide an unformatted outgoing intermediate commit.
commit("correct source")
check(False, "unformatted committed source")
git("update-ref", "refs/remotes/origin/main", "HEAD")
(work / paths[0]).unlink()
commit("delete source")
check(True, "no outgoing C/C++ changes")
git("update-ref", "refs/remotes/origin/main", "HEAD")
(work / "linked.c").symlink_to("README")
commit("link source")
check(False, "source is not a regular committed file")
git("update-ref", "refs/remotes/origin/main", "HEAD")
(work / "other.c").write_text("int good;\n")
commit("source for version check")
tool.write_text(tool.read_text().replace("version 18.1.8", "version 17.0.0"))
check(False, "clang-format 18 is required")
print("pre-push format: clean/dirty blobs, history, unusual paths, vendor, deletion, link and version controls passed")
