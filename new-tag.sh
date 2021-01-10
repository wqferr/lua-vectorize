#!/usr/bin/env sh
set -e

[ -z "$1" ] && { echo 'No tag given'; exit 1; }
scm_rockspec_file='vectorize-scm-4.rockspec'
[ -f "$scm_rockspec_file" ] || { echo 'Update rockspec file name!'; exit 2; }

git tag -m "$2" "$1"
git push origin "$1"
rockspec_file="$(luarocks new_version "$scm_rockspec_file" "$1" --tag "$1" | cut -f2 -d' ')"
luarocks upload --api-key "$(cat ~/.config/luarocks/key)" "$rockspec_file"
