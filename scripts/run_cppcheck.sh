#!/usr/bin/env bash
# Runs cppcheck over the project's own code.
#
#   ./scripts/run_cppcheck.sh [build-dir]
#
# Defaults to build/, used only to point cppcheck at the same compile
# database clang-tidy uses, so it resolves includes the same way.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${1:-$ROOT/build}"

if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
    echo "no compile_commands.json in $BUILD_DIR" >&2
    echo "configure first:  cmake -S . -B $BUILD_DIR" >&2
    exit 1
fi

cppcheck \
    --project="$BUILD_DIR/compile_commands.json" \
    --enable=warning,performance,portability \
    --error-exitcode=1 \
    --suppress=missingIncludeSystem \
    -i"$ROOT/third_party" \
    -i"$BUILD_DIR"
