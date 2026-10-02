#!/bin/sh
# Repository hygiene for a public repo: no personal data, no game images, no
# binary file the test-ROM manifest does not list, and no chain-mode GSD
# settings in anything that gets committed or pushed.
#
#   scripts/hygiene.sh --staged             the commit being made   (pre-commit)
#   scripts/hygiene.sh --tree               every file git would track  (CI)
#   scripts/hygiene.sh --history [revs...]  commits; default --all    (pre-push)
#
# Findings name the rule and the file, never the matched text, so the output is
# safe to paste anywhere. Exit status is 1 if anything was found.
set -eu

cd "$(git rev-parse --show-toplevel)"

SELF=scripts/hygiene.sh
MANIFEST=tests/roms/manifest.txt
fail=0
found() { printf 'hygiene: %s: %s\n' "$1" "$2" >&2; fail=1; }

# A home directory names a person. Hosted CI runner homes are the exception.
HOME_PATH='(/Users/|/home/)[A-Za-z0-9._-]+|[A-Za-z]:\\Users\\[A-Za-z0-9._-]+|-Users-[A-Za-z0-9._]+-'
HOME_OK='(/Users/|/home/)runner'
# Any address except GitHub noreply, documentation domains and SSH remotes.
EMAIL='[A-Za-z0-9._%+-]+@[A-Za-z0-9-]+(\.[A-Za-z0-9-]+)*\.[A-Za-z][A-Za-z]+'
EMAIL_OK='@users\.noreply\.github\.com|noreply@github\.com|@example\.(com|org|net)|git@github\.com'
ROM_NAME='\.(nes|fds|unf|unif|nsf|nsfe|sav|srm|state)$'

# stdin is text. True if some line matches $1 and does not match $2.
leaks() { grep -I -E -e "$1" | grep -v -E -e "$2" | grep -q .; }

# True if $1 is the first field of a line in the test-ROM manifest.
in_manifest() { [ -f "$MANIFEST" ] && awk -v p="$1" '$1 == p { f = 1 } END { exit !f }' "$MANIFEST"; }

# $1 is a path; $2 is a command that prints that path's content.
check_file() {
  [ "$1" = "$SELF" ] && return 0
  if printf '%s\n' "$1" | grep -q -i -E -e "$ROM_NAME" && ! in_manifest "$1"; then
    found 'game image by name' "$1"
  fi
  magic=$($2 "$1" 2>/dev/null | head -c 4 | od -An -tx1 | tr -d ' \n')
  case $magic in
    4e45531a | 4644531a) in_manifest "$1" || found 'game image by content' "$1" ;;
  esac
  # A file git treats as binary (a NUL byte, which grep -I also keys on in the
  # C locale) must be listed in the manifest. This also stops save states, raw
  # dumps and screenshots (ENGINEERING section 7).
  size=$($2 "$1" 2>/dev/null | head -c 1 | wc -c | tr -d ' ')
  if [ "$size" != 0 ] && ! $2 "$1" 2>/dev/null | LC_ALL=C grep -I -q '' && ! in_manifest "$1"; then
    found 'binary file not in manifest' "$1"
  fi
  if $2 "$1" 2>/dev/null | leaks "$HOME_PATH" "$HOME_OK"; then found 'home directory path' "$1"; fi
  if $2 "$1" 2>/dev/null | leaks "$EMAIL" "$EMAIL_OK"; then found 'email address' "$1"; fi
}

check_files() { # $1 is a newline-separated list; $2 as in check_file
  old_ifs=$IFS
  IFS='
'
  set -f
  for f in $1; do check_file "$f" "$2"; done
  set +f
  IFS=$old_ifs
}

# Chaining GSD steps is off for this project. The live config is what governs,
# so the working-tree file is checked. NESTURBATOR_ALLOW_CHAIN=1 bypasses this
# for a run the owner has chosen to chain.
check_stop_policy() {
  [ "${NESTURBATOR_ALLOW_CHAIN:-0}" = 1 ] && return 0
  [ -f .planning/config.json ] || return 0
  grep -q -E '"mode"[[:space:]]*:[[:space:]]*"interactive"' .planning/config.json ||
    found 'GSD mode is not interactive' .planning/config.json
  if grep -q -E '"(auto_advance|_auto_chain_active)"[[:space:]]*:[[:space:]]*true' .planning/config.json; then
    found 'GSD auto-advance or chain flag is on' .planning/config.json
  fi
}

check_identity() { # $1 is an email; $2 says where it came from
  case $1 in
    *@users.noreply.github.com | noreply@github.com) ;;
    *) found 'commit identity is not a noreply address' "$2" ;;
  esac
}

staged_show() { git show ":$1"; }
# A tracked file deleted from the working tree is still read from the index.
tree_show() { if [ -f "$1" ]; then cat "$1"; else git show ":$1"; fi; }

mode=${1:---tree}
[ $# -gt 0 ] && shift
case $mode in
  --staged)
    check_files "$(git diff --cached --name-only --diff-filter=ACMR)" staged_show
    check_stop_policy
    check_identity "$(git var GIT_AUTHOR_IDENT | sed -E 's/.*<([^>]*)>.*/\1/')" 'author'
    check_identity "$(git var GIT_COMMITTER_IDENT | sed -E 's/.*<([^>]*)>.*/\1/')" 'committer'
    ;;
  --tree)
    check_files "$(git ls-files --cached --others --exclude-standard)" tree_show
    check_stop_policy
    ;;
  --history)
    [ $# -gt 0 ] || set -- --all
    for c in $(git rev-list "$@"); do
      if git grep -I -h -E -e "$HOME_PATH" "$c" -- . ":!$SELF" 2>/dev/null | grep -v -E -e "$HOME_OK" | grep -q .; then
        found 'home directory path' "commit $c"
      fi
      if git grep -I -h -E -e "$EMAIL" "$c" -- . ":!$SELF" 2>/dev/null | grep -v -E -e "$EMAIL_OK" | grep -q .; then
        found 'email address' "commit $c"
      fi
      # --numstat shows a binary file's line counts as "-".
      for b in $(git diff-tree -r --root --no-commit-id --numstat "$c" |
        awk -F '\t' '$1 == "-" && $2 == "-" { print $3 }'); do
        [ "$b" = "$SELF" ] || in_manifest "$b" || found 'binary file not in manifest' "$b (in history)"
      done
    done
    for e in $(git log "$@" --format='%ae%n%ce' | sort -u); do check_identity "$e" 'history'; done
    for n in $(git log "$@" --name-only --format= | sort -u | grep -i -E -e "$ROM_NAME" || true); do
      in_manifest "$n" || found 'game image by name' "$n (in history)"
    done
    ;;
  *)
    printf 'usage: %s --staged | --tree | --history [rev-list args]\n' "$0" >&2
    exit 2
    ;;
esac

exit "$fail"
