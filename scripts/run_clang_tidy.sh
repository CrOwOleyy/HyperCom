#!/usr/bin/env bash
# Runs clang-tidy over the project's own code, using the compile
# database from an existing build directory.
#
#   ./scripts/run_clang_tidy.sh [build-dir]
#
# Defaults to build/. The build directory must exist and have been
# configured with CMAKE_EXPORT_COMPILE_COMMANDS ON (the project's
# default), so clang-tidy knows each file's include paths and defines.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${1:-$ROOT/build}"

if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
    echo "no compile_commands.json in $BUILD_DIR" >&2
    echo "configure first:  cmake -S . -B $BUILD_DIR" >&2
    exit 1
fi

mapfile -t FILES < <(
    find "$ROOT"/common "$ROOT"/server "$ROOT"/client "$ROOT"/tools "$ROOT"/tests \
        -name '*.cpp' -not -path '*/third_party/*'
)

clang-tidy -p "$BUILD_DIR" "${FILES[@]}"
