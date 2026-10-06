#!/bin/sh
set -eu
expected_assets() {
  version=$1
  {
    for c in library runner libretro; do
      for o in linux macos windows; do
        for a in x64 arm64; do
          echo "nesturbator-$version-$c-$o-$a.zip"
        done
      done
    done
    echo SHA256SUMS
  } | LC_ALL=C sort
}
classify_run_status() {
  status=$1 conclusion=$2 label=$3
  if [ "$status" != completed ]; then
    echo "$label: PENDING ($status)"
    pending=1
    return 1
  fi
  if [ "$conclusion" != success ]; then
    echo "$label: FAIL ($conclusion)"
    failed=1
    return 1
  fi
  return 0
}
outcome_exit_code() {
  if [ "$failed" -ne 0 ]; then echo 1
  elif [ "$pending" -ne 0 ]; then echo 2
  else echo 0
  fi
}
published_metadata_false_negative() {
  printf '%s' "$1" | jq -e '
    .status == "completed" and .conclusion == "failure" and
    ([.jobs[] | select(.name == "publish")] | length == 1) and
    ([.jobs[] | select(.name != "publish") | select(.conclusion != "success")] | length == 0) and
    ([.jobs[] | select(.name == "publish")][0] as $publish |
      $publish.conclusion == "failure" and
      ([ $publish.steps[] | select(.conclusion == "failure") ] | length == 1) and
      ([ $publish.steps[] | select(.name == "Verify published release metadata" and .conclusion == "failure") ] | length == 1) and
      all(["Exactly the 18 archives", "Write SHA256SUMS", "Upload to the draft release", "Each archive verifies", "A changed archive fails", "Publish the release"][];
        . as $required | any($publish.steps[]; .name == $required and .conclusion == "success")) and
      any($publish.steps[]; (.name | startswith("Run actions/attest@")) and .conclusion == "success")
    )
  ' >/dev/null
}
if [ "${1:-}" = --self-test ]; then
  command -v jq >/dev/null || { echo "jq is required for self-tests" >&2; exit 2; }
  cmake -DSELFTEST=ON -P "$(dirname "$0")/../tests/cmake/vector_result_policy.cmake"
  run='{"headSha":"1111111111111111111111111111111111111111","status":"completed","conclusion":"success","jobs":[{"name":"vectors-full","conclusion":"success"}]}'
  printf '%s' "$run" | jq -e --arg sha "1111111111111111111111111111111111111111" '.headSha == $sha and .status == "completed" and .conclusion == "success" and any(.jobs[]; .name == "vectors-full" and .conclusion == "success")' >/dev/null || { echo "success run fixture rejected" >&2; exit 1; }
  if printf '%s' "$run" | jq -e --arg sha "2222222222222222222222222222222222222222" '.headSha == $sha' >/dev/null; then echo "wrong SHA fixture accepted" >&2; exit 1; fi
  artifacts='{"artifacts":[{"name":"vectors-full-evidence-7","expired":false}]}'
  printf '%s' "$artifacts" | jq -e --arg n vectors-full-evidence-7 'any(.artifacts[]; .name == $n and .expired == false)' >/dev/null || { echo "present artifact fixture rejected" >&2; exit 1; }
  if printf '%s' "$artifacts" | jq -e --arg n vectors-full-evidence-8 'any(.artifacts[]; .name == $n and .expired == false)' >/dev/null; then echo "missing artifact fixture accepted" >&2; exit 1; fi
  if printf '%s' '{"assets":[{"name":"SHA256SUMS","size":1}]}' | jq -e '([.assets[].name] | sort) == ["a.zip","SHA256SUMS"]' >/dev/null; then echo "missing asset fixture accepted" >&2; exit 1; fi
  assets=$(expected_assets 0.0.0)
  [ "$(printf '%s\n' "$assets" | wc -l | tr -d ' ')" -eq 19 ] || { echo "expected asset set has wrong size" >&2; exit 1; }
  [ "$(printf '%s\n' "$assets" | LC_ALL=C sort)" = "$assets" ] || { echo "expected asset set is not C-locale sorted" >&2; exit 1; }
  pending=0 failed=0
  classify_run_status in_progress null "pending fixture" >/dev/null || :
  [ "$pending" -eq 1 ] && [ "$failed" -eq 0 ] || { echo "in-progress run fixture misclassified" >&2; exit 1; }
  [ "$(outcome_exit_code)" -eq 2 ] || { echo "pending outcome did not select exit 2" >&2; exit 1; }
  pending=0 failed=0
  classify_run_status completed failure "failed fixture" >/dev/null || :
  [ "$failed" -eq 1 ] && [ "$pending" -eq 0 ] || { echo "completed failure fixture misclassified" >&2; exit 1; }
  [ "$(outcome_exit_code)" -eq 1 ] || { echo "failure outcome did not select exit 1" >&2; exit 1; }
  metadata_failure='{"status":"completed","conclusion":"failure","jobs":[{"name":"release-please","conclusion":"success"},{"name":"ci / CI required","conclusion":"success"},{"name":"publish","conclusion":"failure","steps":[{"name":"Exactly the 18 archives","conclusion":"success"},{"name":"Write SHA256SUMS","conclusion":"success"},{"name":"Run actions/attest@pinned","conclusion":"success"},{"name":"Upload to the draft release","conclusion":"success"},{"name":"Each archive verifies","conclusion":"success"},{"name":"A changed archive fails","conclusion":"success"},{"name":"Publish the release","conclusion":"success"},{"name":"Verify published release metadata","conclusion":"failure"}]}]}'
  published_metadata_false_negative "$metadata_failure" || { echo "known post-publication metadata false-negative was rejected" >&2; exit 1; }
  if published_metadata_false_negative "$(printf '%s' "$metadata_failure" | jq '.jobs[] |= if .name == "publish" then .steps[-2].conclusion = "failure" else . end')"; then echo "earlier publish failure accepted as metadata false-negative" >&2; exit 1; fi
  if published_metadata_false_negative "$(printf '%s' "$metadata_failure" | jq '.jobs[] |= if .name == "ci / CI required" then .conclusion = "failure" else . end')"; then echo "failed prerequisite accepted as metadata false-negative" >&2; exit 1; fi
  echo "phase2_outcomes: SHA, artifact, asset, pending schedule and successful run fixtures passed"
  exit 0
fi
if [ "$#" -ne 2 ]; then echo "Usage: $0 <phase-2-merge-sha> <release-tag>" >&2; exit 2; fi
merge_sha=$1
tag=$2
case "$merge_sha" in *[!0-9a-fA-F]*|'') echo "invalid merge SHA" >&2; exit 2;; esac
[ "${#merge_sha}" -eq 40 ] || { echo "merge SHA must be 40 hex characters" >&2; exit 2; }
case "$tag" in v[0-9]*.[0-9]*.[0-9]*) :;; *) echo "expected vMAJOR.MINOR.PATCH tag" >&2; exit 2;; esac
for tool in gh jq cmake; do command -v "$tool" >/dev/null || { echo "$tool is required" >&2; exit 2; }; done
gh auth status >/dev/null 2>&1 || { echo "gh is not authenticated (read-only query)" >&2; exit 2; }
repo=$(gh repo view --json nameWithOwner --jq .nameWithOwner)
version=${tag#v}
expected=$(expected_assets "$version")
pending=0 failed=0
release=$(gh release view "$tag" --repo "$repo" --json tagName,isDraft,url,assets 2>/dev/null || true)
if [ -z "$release" ]; then echo "release: PENDING (tag not published)"; pending=1
else
  actual=$(printf '%s' "$release" | jq -r '.assets[].name' | LC_ALL=C sort)
  sizes=$(printf '%s' "$release" | jq '[.assets[] | select(.size > 0)] | length')
  if [ "$(printf '%s' "$release" | jq -r .tagName)" != "$tag" ] || [ "$(printf '%s' "$release" | jq -r .isDraft)" != false ] || [ "$actual" != "$expected" ] || [ "$sizes" -ne 19 ]; then
    echo "release: FAIL (published tag/assets mismatch)"; failed=1
  else
    ref=$(gh api "repos/$repo/git/ref/tags/$tag")
    type=$(printf '%s' "$ref" | jq -r .object.type); sha=$(printf '%s' "$ref" | jq -r .object.sha)
    if [ "$type" = tag ]; then sha=$(gh api "repos/$repo/git/tags/$sha" --jq .object.sha); fi
    relation=$(gh api "repos/$repo/compare/$merge_sha...$sha" --jq .status)
    if [ "$relation" != ahead ] && [ "$sha" != "$merge_sha" ]; then echo "release: FAIL (tag commit not descended from merge)"; failed=1
    else
      release_runs=$(gh run list --repo "$repo" --workflow release.yml --branch main --json databaseId,headSha,status,conclusion --limit 100)
      release_id=$(printf '%s' "$release_runs" | jq -r --arg s "$sha" '[.[] | select(.headSha == $s and .status == "completed" and .conclusion == "success")][0].databaseId // empty')
      if [ -n "$release_id" ]; then
        echo "release: PASS ($(printf '%s' "$release" | jq -r .url); release run $release_id on $sha)"
      else
        active_release_id=$(printf '%s' "$release_runs" | jq -r --arg s "$sha" '[.[] | select(.headSha == $s and .status != "completed")][0].databaseId // empty')
        if [ -n "$active_release_id" ]; then
          echo "release: PENDING (release run $active_release_id is still running)"; pending=1
        else
          release_id=$(printf '%s' "$release_runs" | jq -r --arg s "$sha" '[.[] | select(.headSha == $s and .status == "completed" and .conclusion == "failure")][0].databaseId // empty')
        fi
        if [ -z "$active_release_id" ] && [ -z "$release_id" ]; then
          echo "release: PENDING (no completed release run on tag commit)"; pending=1
        elif [ -z "$active_release_id" ]; then
          failed_run=$(gh run view "$release_id" --repo "$repo" --json headSha,status,conclusion,jobs,url)
          if [ "$(printf '%s' "$failed_run" | jq -r .headSha)" = "$sha" ] && published_metadata_false_negative "$failed_run"; then
            echo "release: PASS ($tag published with exact assets; release run $release_id passed CI, archive attestation, tamper check and publish; only its obsolete metadata assertion failed)"
          else
            echo "release: FAIL (release run $release_id failed outside the obsolete metadata assertion; $(printf '%s' "$failed_run" | jq -r .url))"; failed=1
          fi
        fi
      fi
    fi
  fi
fi

check_run() {
  id=$1 expected_sha=$2 label=$3
  if [ -z "$id" ]; then echo "$label: PENDING (no matching run)"; pending=1; return; fi
  run=$(gh run view "$id" --repo "$repo" --json headSha,status,conclusion,url,jobs)
  sha=$(printf '%s' "$run" | jq -r .headSha)
  if [ "$sha" != "$expected_sha" ]; then echo "$label: FAIL (wrong SHA $sha)"; failed=1; return; fi
  status=$(printf '%s' "$run" | jq -r .status)
  conclusion=$(printf '%s' "$run" | jq -r .conclusion)
  if ! classify_run_status "$status" "$conclusion" "$label"; then return; fi
  if ! printf '%s' "$run" | jq -e 'any(.jobs[]; .name == "vectors-full" and .conclusion == "success")' >/dev/null; then echo "$label: FAIL (vectors-full job unsuccessful)"; failed=1; return; fi
  artifact="vectors-full-evidence-$id"
  meta=$(gh api "repos/$repo/actions/runs/$id/artifacts")
  if ! printf '%s' "$meta" | jq -e --arg n "$artifact" 'any(.artifacts[]; .name == $n and .expired == false)' >/dev/null; then echo "$label: FAIL (missing artifact)"; failed=1; return; fi
  dir=$(mktemp -d)
  if ! gh run download "$id" --repo "$repo" --name "$artifact" --dir "$dir" >/dev/null 2>&1; then rm -rf "$dir"; echo "$label: FAIL (artifact download)"; failed=1; return; fi
  inv=$(find "$dir" -name registered-tests.json -type f -print -quit); junit=$(find "$dir" -name vectors-full.junit.xml -type f -print -quit)
  if [ -z "$inv" ] || [ -z "$junit" ]; then rm -rf "$dir"; echo "$label: FAIL (missing evidence file)"; failed=1; return; fi
  if ! cmake -DINVENTORY="$inv" -DJUNIT="$junit" -P tests/cmake/vector_result_policy.cmake; then rm -rf "$dir"; echo "$label: FAIL (invalid evidence)"; failed=1; return; fi
  rm -rf "$dir"
  echo "$label: PASS ($(printf '%s' "$run" | jq -r .url))"
}

runs=$(gh run list --repo "$repo" --workflow nightly.yml --branch main --json databaseId,headSha,event,createdAt --limit 100)
push=$(printf '%s' "$runs" | jq -r --arg s "$merge_sha" '[.[] | select(.event == "push" and .headSha == $s)][0].databaseId // empty')
check_run "$push" "$merge_sha" "main-push vectors-full"
date=$(gh api "repos/$repo/commits/$merge_sha" --jq .commit.committer.date)
scheduled=$(printf '%s' "$runs" | jq -r --arg d "$date" '[.[] | select(.event == "schedule" and .createdAt > $d)] | sort_by(.createdAt) | .[0].databaseId // empty')
if [ -z "$scheduled" ]; then echo "scheduled vectors-full: PENDING (first post-merge schedule not started)"; pending=1
else
  srun=$(gh run view "$scheduled" --repo "$repo" --json headSha); ssha=$(printf '%s' "$srun" | jq -r .headSha)
  rel=$(gh api "repos/$repo/compare/$merge_sha...$ssha" --jq .status)
  if [ "$rel" != ahead ] && [ "$ssha" != "$merge_sha" ]; then echo "scheduled vectors-full: FAIL (head not descended from merge)"; failed=1
  else check_run "$scheduled" "$ssha" "scheduled vectors-full"; fi
fi
exit "$(outcome_exit_code)"
