#!/usr/bin/env bash
set -euo pipefail

# Script de packaging AppImage sous Linux
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

BUILD_DIR="${ROOT_DIR}/build/linux"
APPDIR="${BUILD_DIR}/AppDir"
PKG_DIR="${BUILD_DIR}/packages"

echo "=== [Hypercom] Packaging AppImage ==="

mkdir -p "${APPDIR}/usr/bin"
mkdir -p "${APPDIR}/usr/share/applications"
mkdir -p "${APPDIR}/usr/share/icons/hicolor/256x256/apps"
mkdir -p "${PKG_DIR}"

# 1. Compilation des binaires
cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build "${BUILD_DIR}" -j --target hypercom_client hypercom_cli

# 2. Copie des binaires dans AppDir
cp "${BUILD_DIR}/bin/hypercom_client" "${APPDIR}/usr/bin/"
cp "${BUILD_DIR}/bin/hypercom_cli" "${APPDIR}/usr/bin/"

# 3. Fichier .desktop pour AppImage
cat <<'EOF' > "${APPDIR}/hypercom.desktop"
[Desktop Entry]
Name=Hypercom
Exec=hypercom_client
Icon=hypercom
Type=Application
Categories=Network;InstantMessaging;
Comment=Reseau social chiffre de bout en bout
EOF

# 4. Icone fictive/SVG minimale si absente
if [ ! -f "${APPDIR}/hypercom.png" ]; then
    touch "${APPDIR}/hypercom.png"
fi

# 5. AppRun script autonome
cat <<'EOF' > "${APPDIR}/AppRun"
#!/bin/sh
HERE="$(dirname "$(readlink -f "$0")")"
export PATH="${HERE}/usr/bin:${PATH}"
export LD_LIBRARY_PATH="${HERE}/usr/lib:${LD_LIBRARY_PATH}"
exec "${HERE}/usr/bin/hypercom_client" "$@"
EOF
chmod +x "${APPDIR}/AppRun"

# 6. Generation AppImage via appimagetool si installe
if command -v appimagetool >/dev/null 2>&1; then
    appimagetool "${APPDIR}" "${PKG_DIR}/Hypercom-x86_64.AppImage"
    echo "=== AppImage generee dans : ${PKG_DIR}/Hypercom-x86_64.AppImage ==="
else
    echo "=== Structure AppDir preparee sous : ${APPDIR} ==="
    echo "(Pour produire le fichier .AppImage binaire final, installez 'appimagetool')"
fi
