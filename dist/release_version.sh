#!/bin/env bash

function colorecho {
  echo -e "\e[32m${1}\e[0m"
}

set -o errexit
while getopts ":c:nrd" opt; do
  case $opt in
    d)
      set -o xtrace
      ;;
    c)
      CHANGES="$OPTARG"
      ;;
    n)
      NO_NEWS_POST=1
      ;;
    r)
      RELEASE=1
      ;;
    :)
      echo "Option -$OPTARG requires an argument." >&2
      exit 1
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      exit 1
      ;;
  esac
done
shift $(($OPTIND - 1))

version="${1}"
if ! [[ "$version" =~ ^[0-9]+\.[0-9]+\.[0-9]+ ]]; then
  echo "Expected version number as parameter: major.minor.patch" && exit 1
fi
if [ `git rev-list --parents HEAD | tail -1` != '4d409110b9771e688acbb995422541f03ef0d8a7' ]; then
  echo "cwd is not the albert repository" && exit 1
fi

albert_root=`git rev-parse --show-toplevel`
plugins_root="$albert_root/plugins"
modules_root="$albert_root/plugins/python/share/modules"
doc_root="$albert_root/doc"
changelog="$albert_root/dist/albert.changes"

# [All] Sync

for path in "${albert_root}" "${plugins_root}" "${modules_root}"; do
  colorecho "Sync dev branch of repo at $path"
  git -C "${path}" checkout dev
  git -C "${path}" pull
  git -C "${path}" push
done
colorecho "Sync dev branch of repo at ${doc_root}"
git -C "${doc_root}" pull
git -C "${doc_root}" push

# [Plugins] Commit submodule changes and push

colorecho "Commit python extensions submodule sha"
git -C "${plugins_root}" commit "${modules_root}" -m "Update modules." && git -C "${plugins_root}" push

# [Albert] Commit submodule changes and push

colorecho "Commit plugins submodule sha"
git -C "${albert_root}" commit "${plugins_root}" -m "Update modules." && git -C "${albert_root}" push

# Travis note

[ -z "$RELEASE" ] && echo "Wait for Travis to succeed and run again with -r (Release)" && exit 1

# [Albert] Write/use existing changelog

if [ -n "$CHANGES" ]; then
  atomic_changelog="$CHANGES"
else
  atomic_changelog=$(mktemp)
  for path in "${albert_root}" "${plugins_root}" "${modules_root}"; do
    module_name=`basename "${path}"`
    changes=`git -C "${path}" log --pretty=format:"* [$module_name] %s" master..dev`
    echo "${changes}"  >> "${atomic_changelog}"
  done
  $EDITOR "${atomic_changelog}"
fi

# Update changelog with atomic changes
tmp_changelog=$(mktemp)
echo -e "v${version} ($(date --rfc-3339=date))\n" > "$tmp_changelog"
cat "$atomic_changelog" >> "$tmp_changelog"
echo -en "\n" >> "$tmp_changelog"
cat "$changelog" >> "$tmp_changelog"
$EDITOR "${tmp_changelog}"
mv "$tmp_changelog" "$changelog"

# [Docs] Add news post

if [ -z "$NO_NEWS_POST" ]; then
  news_path="${doc_root}/src/_posts/`date --rfc-3339=date`-albert-v${version}-released.md"
  [ -e "$news_path" ] && echo "There is already a news post: $news_path" && exit 1
  cat > "$news_path" <<- EOM
---
layout: page
title:  "Albert v${version} released"
date: $(date --rfc-3339=seconds)
---
EOM
  cat "${atomic_changelog}" >> "$news_path"
  cat >> "$news_path" <<- EOM

Further the release contains minor changes and fixes.

Check the [git log](https://github.com/albertlauncher/albert/commits/v${version}) for details.
EOM
  $EDITOR "$news_path"
  git -C "${doc_root}" add "${news_path}"
  git -C "${doc_root}" commit -m "$version"
  git -C "${doc_root}" push
fi

# [Albert] Change project version

sed -i.bak "s/^project.*$/project(albert VERSION ${version})/" "$albert_root/CMakeLists.txt"

# [Albert] Commit changelog, CMakeLists and submodule changes and finally push

git -C "${albert_root}" add "${changelog}"
git -C "${albert_root}" add "${albert_root}/CMakeLists.txt"
git -C "${albert_root}" add "${doc_root}"
git -C "${albert_root}" commit -m "v${version}"
git -C "${albert_root}" push

# Tag the release

git -C "${albert_root}" tag "v${version}"
git -C "${albert_root}" push --tags

## Rebase and push master

for path in "${albert_root}" "${plugins_root}" "${modules_root}"; do
  git -C "${path}" checkout master
  git -C "${path}" rebase dev
  git -C "${path}" push
  git -C "${path}" checkout dev
done

## Clean up

rm -f "${albert_root}/CMakeLists.txt.bak"

echo -e "\e[31mDone.\nAlbert v$version released.\e[0m"
