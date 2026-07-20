#!/bin/bash

set -euo pipefail

project_root="$(cd "$(dirname "$0")/.." && pwd)"
build_dir="${OPENFVD_BUILD_DIR:-${TMPDIR:-/tmp}/openfvd-build-arm64}"
dist_dir="${OPENFVD_DIST_DIR:-$project_root/dist}"

if [[ "$(uname -m)" != "arm64" ]]; then
    echo "This script must run on an Apple Silicon Mac (arm64)." >&2
    exit 1
fi

if ! command -v brew >/dev/null 2>&1; then
    echo "Homebrew is required. Install it from https://brew.sh first." >&2
    exit 1
fi

qt_prefix="$(brew --prefix qt@5)"
brew --prefix glm >/dev/null

git -C "$project_root" submodule update --init --recursive
mkdir -p "$build_dir" "$dist_dir"

cd "$build_dir"
"$qt_prefix/bin/qmake" "$project_root/fvd.pro" CONFIG+=release CONFIG+=sdk_no_version_check
build_jobs="$(sysctl -n hw.logicalcpu 2>/dev/null || true)"
if [[ -z "$build_jobs" ]]; then build_jobs=4; fi
make -j"$build_jobs"

"$qt_prefix/bin/macdeployqt" FVD.app -always-overwrite
xattr -cr FVD.app
codesign --force --deep --sign - FVD.app
codesign --verify --deep --strict FVD.app

executable="$build_dir/FVD.app/Contents/MacOS/FVD"
architectures="$(lipo -archs "$executable")"
if [[ "$architectures" != "arm64" ]]; then
    echo "Expected an arm64 executable, got: $architectures" >&2
    exit 1
fi

while IFS= read -r -d '' candidate; do
    file_description="$(file "$candidate")"
    if [[ "$file_description" != *"Mach-O"* ]]; then continue; fi
    if [[ "$file_description" != *"arm64"* ]]; then
        echo "Non-arm64 binary in app bundle: $candidate ($file_description)" >&2
        exit 1
    fi

    while IFS= read -r dependency; do
        if [[ "$(basename "$dependency")" == "$(basename "$candidate")" ]]; then
            continue
        fi
        case "$dependency" in
            /opt/homebrew/*|/usr/local/*)
                echo "Undeployed Homebrew dependency in $candidate: $dependency" >&2
                exit 1
                ;;
        esac
    done < <(otool -L "$candidate" | tail -n +2 | awk '{print $1}')
done < <(find FVD.app -type f -print0)

artifact="$dist_dir/FVD-Apple-Silicon.zip"
ditto -c -k --norsrc --keepParent FVD.app "$artifact"

echo "Built $artifact ($architectures)"
