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
git add -A

status=0
out=$(sh scripts/hygiene.sh --tree 2>&1) || status=$?
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
echo "PASS"
