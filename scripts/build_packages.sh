#!/usr/bin/env bash
set -euo pipefail

# Automatic Linux packaging script (DEB + TAR.GZ)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

BUILD_DIR="${ROOT_DIR}/build/linux"
PKG_DIR="${BUILD_DIR}/packages"

echo "=== [Hypercom] Linux Packaging ==="
echo "Build directory: ${BUILD_DIR}"

mkdir -p "${BUILD_DIR}"
mkdir -p "${PKG_DIR}"

cd "${ROOT_DIR}"
cmake -S . -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build "${BUILD_DIR}" -j

cd "${BUILD_DIR}"
cpack -G "DEB;TGZ"

mv -f hypercom-*.deb hypercom-*.tar.gz "${PKG_DIR}/" 2>/dev/null || true

echo "=== [Hypercom] Packages generated in: ${PKG_DIR} ==="
ls -lh "${PKG_DIR}"
