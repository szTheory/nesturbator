#!/bin/sh
# Shows that scripts/hygiene.sh finds personal data in files that are easy to
# miss. It builds a scratch repository, copies the script into it, adds one
# file per case and requires each expected finding.
#
#   scan_selftest.sh <hygiene.sh> <scratch dir>
#
# The home path and the address are assembled at run time so this file holds
# neither and passes the scan itself.
set -eu

script=$1
dir=$2
rm -rf "$dir"
mkdir -p "$dir/scripts"
cp "$script" "$dir/scripts/hygiene.sh"
cd "$dir"
git init -q .

u=Users
home="/$u/someone"
at=@
mail="someone${at}mail.test"

# A non-ASCII file name, which git quotes unless told not to.
cafe=$(printf 'caf\303\251.txt')
printf '%s %s\n' "$home" "$mail" >"$cafe"
# A file name holding a newline cannot be scanned; it must fail closed.
nl=$(printf 'new\nline.txt')
printf 'nothing personal\n' >"$nl"
# Latin-1 text, which is not valid UTF-8.
printf 'caf\351 %s %s\n' "$home" "$mail" >latin1.txt
git add -A

# Run under a UTF-8 locale, where grep -I would take latin1.txt as binary.
status=0
out=$(LC_ALL=C.UTF-8 sh scripts/hygiene.sh --tree 2>&1) || status=$?
printf '%s\n' "$out"
[ "$status" = 1 ] || { echo "FAIL: --tree exit status $status, expected 1"; exit 1; }

expect() {
  printf '%s\n' "$out" | grep -q -F -x -e "hygiene: $1" || {
    echo "FAIL: missing finding: $1"
    exit 1
  }
}
expect "home directory path: $cafe"
expect "email address: $cafe"
expect "cannot read file: new"
expect "cannot read file: line.txt"
expect "home directory path: latin1.txt"
expect "email address: latin1.txt"

# --history scans commit messages. The co-author trailer's noreply address
# is allowed there; a home path and another address are not.
git rm -q --cached -r .
printf 'clean\n' >clean.txt
git add clean.txt
git -c user.name=test -c user.email=test@users.noreply.github.com \
  -c commit.gpgsign=false -c core.hooksPath=/dev/null commit -q -F - <<EOF
test: a message with personal data

Seen at $home with $mail.

Co-Authored-By: Claude <noreply${at}anthropic.com>
EOF
c=$(git rev-parse HEAD)
status=0
out=$(sh scripts/hygiene.sh --history 2>&1) || status=$?
printf '%s\n' "$out"
[ "$status" = 1 ] || { echo "FAIL: --history exit status $status, expected 1"; exit 1; }
expect "home directory path: message of $c"
expect "email address: message of $c"
git -c user.name=test -c user.email=test@users.noreply.github.com \
  -c commit.gpgsign=false -c core.hooksPath=/dev/null commit -q --amend -F - <<EOF
test: a clean message

Co-Authored-By: Claude <noreply${at}anthropic.com>
EOF
out=$(sh scripts/hygiene.sh --history 2>&1) || {
  printf '%s\n' "$out"
  echo "FAIL: --history found something in a clean commit"
  exit 1
}
echo "PASS"
