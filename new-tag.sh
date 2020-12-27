#!/usr/bin/env sh
set -e

[ -z "$1" ] && { echo 'No tag given'; exit 1; }

git tag -m '' "$1"
rockspec_file=$(luarocks new_version vectorize-scm-0.rockspec "$1" --tag "$1" | cut -f2 -d' ')
luarocks upload --api-key "$(cat ~/.config/luarocks/key)" "$rockspec_file"
git push origin "$1"
