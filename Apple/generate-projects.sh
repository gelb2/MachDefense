#!/bin/sh
set -eu
root=$(CDPATH='' cd -- "$(dirname -- "$0")/.." && pwd)
command -v xcodegen >/dev/null 2>&1 || { printf '%s\n' 'Install XcodeGen before generating the projects.' >&2; exit 1; }
xcodegen generate --spec "$root/Modules/project.yml" --project "$root/Modules"
xcodegen generate --spec "$root/Apple/iOS/project.yml" --project "$root/Apple/iOS"
