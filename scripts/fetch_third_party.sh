#!/usr/bin/env bash
# Fetches the allowed dependencies into third_party/.
#
# Downloading is deliberately an explicit action: CMake never reaches
# out to the network on its own.
#
# Trust model, read before using this script:
#   No checksum is hardcoded here. This script REFUSES to install an
#   archive whose checksum isn't already recorded in
#   third_party/checksums.txt. On the first pass it prints the computed
#   checksum and the official URL to verify it against, then stops. A
#   human has to compare it, once, and record the value. Every run
#   after that is then protected.
#
#   usage: ./scripts/fetch_third_party.sh [libsodium|sqlite3|imgui|all]

set -euo pipefail

readonly ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly VENDOR_DIR="${ROOT_DIR}/third_party"
readonly WORK_DIR="${VENDOR_DIR}/.download"
readonly CHECKSUM_FILE="${VENDOR_DIR}/checksums.txt"

# Default versions, overridable through an environment variable. Any
# change requires re-verifying the corresponding checksum.
readonly SODIUM_VERSION="${HYPERCOM_SODIUM_VERSION:-1.0.20}"
readonly SQLITE_YEAR="${HYPERCOM_SQLITE_YEAR:-2024}"
readonly SQLITE_STEM="${HYPERCOM_SQLITE_STEM:-sqlite-amalgamation-3470000}"
readonly IMGUI_VERSION="${HYPERCOM_IMGUI_VERSION:-v1.91.5}"

log() {
    printf '\033[0;32m[third_party]\033[0m %s\n' "$*"
}

fail() {
    printf '\033[0;31m[third_party] error:\033[0m %s\n' "$*" >&2
    exit 1
}

lookup_expected_sha256() {
    local key="$1"
    [ -f "${CHECKSUM_FILE}" ] || return 0
    awk -v k="${key}" '$1 == k { print $2 }' "${CHECKSUM_FILE}" | head -n1
}

# Refuses any unverified archive. This is the script's only point of
# trust: it's deliberately blocking.
verify_against_pinned_sha256() {
    local key="$1" file="$2" verify_url="$3" expected actual
    actual="$(sha256sum "${file}" | cut -d' ' -f1)"
    expected="$(lookup_expected_sha256 "${key}")"
    if [ -z "${expected}" ]; then
        fail "no checksum recorded for '${key}'.

  computed checksum: ${actual}

  1. compare this value against the one published at:
       ${verify_url}
  2. if it matches, record the following line in
     third_party/checksums.txt then re-run:

       ${key}  ${actual}

  Until this check is done, the archive isn't installed."
    fi
    if [ "${actual}" != "${expected}" ]; then
        rm -f "${file}"
        fail "INVALID CHECKSUM for '${key}' -- archive deleted.
  expected: ${expected}
  got     : ${actual}
Do not retry without understanding why. Either the upstream version
changed, or the archive was tampered with in transit."
    fi
    log "checksum verified: ${key}"
}

download_archive() {
    local url="$1" target="$2" key="$3" verify_url="$4"
    if [ -f "${target}" ]; then
        log "archive already present: $(basename "${target}")"
    else
        log "downloading ${url}"
        curl --fail --location --proto '=https' --tlsv1.2 \
            --silent --show-error --output "${target}" "${url}"
    fi
    verify_against_pinned_sha256 "${key}" "${target}" "${verify_url}"
}

fetch_libsodium() {
    local prefix="${VENDOR_DIR}/libsodium"
    if [ -f "${prefix}/include/sodium.h" ]; then
        log "libsodium already installed"
        return
    fi
    local archive="${WORK_DIR}/libsodium-${SODIUM_VERSION}.tar.gz"
    download_archive \
        "https://download.libsodium.org/libsodium/releases/libsodium-${SODIUM_VERSION}.tar.gz" \
        "${archive}" "libsodium-${SODIUM_VERSION}.tar.gz" \
        "https://download.libsodium.org/libsodium/releases/ (.sig / minisign file)"
    log "compiling libsodium ${SODIUM_VERSION}, this takes a few minutes"
    rm -rf "${WORK_DIR}/libsodium-src"
    mkdir -p "${WORK_DIR}/libsodium-src"
    tar -xzf "${archive}" -C "${WORK_DIR}/libsodium-src" --strip-components=1
    (
        cd "${WORK_DIR}/libsodium-src"
        ./configure --prefix="${prefix}" --disable-shared --enable-static --quiet
        make -j"$(nproc)" >/dev/null
        make install >/dev/null
    )
    log "libsodium installed into third_party/libsodium"
}

fetch_sqlite3() {
    local prefix="${VENDOR_DIR}/sqlite3"
    if [ -f "${prefix}/sqlite3.c" ]; then
        log "sqlite3 already present"
        return
    fi
    local archive="${WORK_DIR}/${SQLITE_STEM}.zip"
    download_archive \
        "https://www.sqlite.org/${SQLITE_YEAR}/${SQLITE_STEM}.zip" \
        "${archive}" "${SQLITE_STEM}.zip" \
        "https://www.sqlite.org/download.html (SHA3-256/SHA-256 column)"
    rm -rf "${WORK_DIR}/sqlite-src"
    unzip -q "${archive}" -d "${WORK_DIR}/sqlite-src"
    mkdir -p "${prefix}"
    local source_dir="${WORK_DIR}/sqlite-src/${SQLITE_STEM}"
    cp "${source_dir}/sqlite3.c" "${source_dir}/sqlite3.h" \
        "${source_dir}/sqlite3ext.h" "${prefix}/"
    log "sqlite3 amalgamation installed into third_party/sqlite3"
}

fetch_imgui() {
    local prefix="${VENDOR_DIR}/imgui"
    if [ -f "${prefix}/imgui.cpp" ]; then
        log "imgui already present"
        return
    fi
    local archive="${WORK_DIR}/imgui-${IMGUI_VERSION}.tar.gz"
    download_archive \
        "https://github.com/ocornut/imgui/archive/refs/tags/${IMGUI_VERSION}.tar.gz" \
        "${archive}" "imgui-${IMGUI_VERSION}.tar.gz" \
        "https://github.com/ocornut/imgui/releases/tag/${IMGUI_VERSION}"
    mkdir -p "${prefix}"
    tar -xzf "${archive}" -C "${prefix}" --strip-components=1
    log "dear imgui ${IMGUI_VERSION} installed into third_party/imgui"
}

main() {
    command -v curl >/dev/null || fail "curl is required"
    command -v sha256sum >/dev/null || fail "sha256sum is required"
    mkdir -p "${WORK_DIR}"
    case "${1:-all}" in
        libsodium) fetch_libsodium ;;
        sqlite3)   fetch_sqlite3 ;;
        imgui)     fetch_imgui ;;
        all)       fetch_libsodium; fetch_sqlite3; fetch_imgui ;;
        *)         fail "unknown target: $1 (libsodium|sqlite3|imgui|all)" ;;
    esac
    log "done"
}

main "$@"
