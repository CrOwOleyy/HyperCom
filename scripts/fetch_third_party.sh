#!/usr/bin/env bash
# Recupere les dependances de BRIEF.md 15 dans third_party/.
#
# Le telechargement est volontairement une action explicite : CMake ne va
# jamais chercher quoi que ce soit sur le reseau tout seul.
#
# Modele de confiance, a lire avant d'utiliser ce script :
#   Aucune empreinte n'est codee en dur ici. Ce script REFUSE d'installer une
#   archive dont l'empreinte n'est pas deja enregistree dans
#   third_party/checksums.txt. Au premier passage il affiche l'empreinte
#   calculee et l'URL officielle ou la verifier, puis s'arrete. C'est a un
#   humain de comparer, une fois, et d'enregistrer la valeur. Toutes les
#   executions suivantes sont alors protegees.
#
#   usage : ./scripts/fetch_third_party.sh [libsodium|sqlite3|imgui|all]

set -euo pipefail

readonly ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly VENDOR_DIR="${ROOT_DIR}/third_party"
readonly WORK_DIR="${VENDOR_DIR}/.download"
readonly CHECKSUM_FILE="${VENDOR_DIR}/checksums.txt"

# Versions par defaut, surchargeables par variable d'environnement. Toute
# modification impose de re-verifier l'empreinte correspondante.
readonly SODIUM_VERSION="${HYPERCOM_SODIUM_VERSION:-1.0.20}"
readonly SQLITE_YEAR="${HYPERCOM_SQLITE_YEAR:-2024}"
readonly SQLITE_STEM="${HYPERCOM_SQLITE_STEM:-sqlite-amalgamation-3470000}"
readonly IMGUI_VERSION="${HYPERCOM_IMGUI_VERSION:-v1.91.5}"

log() {
    printf '\033[0;32m[third_party]\033[0m %s\n' "$*"
}

fail() {
    printf '\033[0;31m[third_party] echec :\033[0m %s\n' "$*" >&2
    exit 1
}

lookup_expected_sha256() {
    local key="$1"
    [ -f "${CHECKSUM_FILE}" ] || return 0
    awk -v k="${key}" '$1 == k { print $2 }' "${CHECKSUM_FILE}" | head -n1
}

# Refuse toute archive non verifiee. C'est le seul point de confiance du
# script : il est deliberement bloquant.
verify_against_pinned_sha256() {
    local key="$1" file="$2" verify_url="$3" expected actual
    actual="$(sha256sum "${file}" | cut -d' ' -f1)"
    expected="$(lookup_expected_sha256 "${key}")"
    if [ -z "${expected}" ]; then
        fail "aucune empreinte enregistree pour '${key}'.

  empreinte calculee : ${actual}

  1. comparer cette valeur a celle publiee sur :
       ${verify_url}
  2. si elle correspond, enregistrer la ligne suivante dans
     third_party/checksums.txt puis relancer :

       ${key}  ${actual}

  Tant que cette verification n'est pas faite, l'archive n'est pas installee."
    fi
    if [ "${actual}" != "${expected}" ]; then
        rm -f "${file}"
        fail "EMPREINTE INVALIDE pour '${key}' -- archive supprimee.
  attendue : ${expected}
  obtenue  : ${actual}
Ne pas reessayer sans avoir compris pourquoi. Soit la version amont a change,
soit l'archive a ete alteree en transit."
    fi
    log "empreinte verifiee : ${key}"
}

download_archive() {
    local url="$1" target="$2" key="$3" verify_url="$4"
    if [ -f "${target}" ]; then
        log "archive deja presente : $(basename "${target}")"
    else
        log "telechargement de ${url}"
        curl --fail --location --proto '=https' --tlsv1.2 \
            --silent --show-error --output "${target}" "${url}"
    fi
    verify_against_pinned_sha256 "${key}" "${target}" "${verify_url}"
}

fetch_libsodium() {
    local prefix="${VENDOR_DIR}/libsodium"
    if [ -f "${prefix}/include/sodium.h" ]; then
        log "libsodium deja installe"
        return
    fi
    local archive="${WORK_DIR}/libsodium-${SODIUM_VERSION}.tar.gz"
    download_archive \
        "https://download.libsodium.org/libsodium/releases/libsodium-${SODIUM_VERSION}.tar.gz" \
        "${archive}" "libsodium-${SODIUM_VERSION}.tar.gz" \
        "https://download.libsodium.org/libsodium/releases/ (fichier .sig / minisign)"
    log "compilation de libsodium ${SODIUM_VERSION}, comptez quelques minutes"
    rm -rf "${WORK_DIR}/libsodium-src"
    mkdir -p "${WORK_DIR}/libsodium-src"
    tar -xzf "${archive}" -C "${WORK_DIR}/libsodium-src" --strip-components=1
    (
        cd "${WORK_DIR}/libsodium-src"
        ./configure --prefix="${prefix}" --disable-shared --enable-static --quiet
        make -j"$(nproc)" >/dev/null
        make install >/dev/null
    )
    log "libsodium installe dans third_party/libsodium"
}

fetch_sqlite3() {
    local prefix="${VENDOR_DIR}/sqlite3"
    if [ -f "${prefix}/sqlite3.c" ]; then
        log "sqlite3 deja present"
        return
    fi
    local archive="${WORK_DIR}/${SQLITE_STEM}.zip"
    download_archive \
        "https://www.sqlite.org/${SQLITE_YEAR}/${SQLITE_STEM}.zip" \
        "${archive}" "${SQLITE_STEM}.zip" \
        "https://www.sqlite.org/download.html (colonne SHA3-256/SHA-256)"
    rm -rf "${WORK_DIR}/sqlite-src"
    unzip -q "${archive}" -d "${WORK_DIR}/sqlite-src"
    mkdir -p "${prefix}"
    local source_dir="${WORK_DIR}/sqlite-src/${SQLITE_STEM}"
    cp "${source_dir}/sqlite3.c" "${source_dir}/sqlite3.h" \
        "${source_dir}/sqlite3ext.h" "${prefix}/"
    log "amalgamation sqlite3 installee dans third_party/sqlite3"
}

fetch_imgui() {
    local prefix="${VENDOR_DIR}/imgui"
    if [ -f "${prefix}/imgui.cpp" ]; then
        log "imgui deja present"
        return
    fi
    local archive="${WORK_DIR}/imgui-${IMGUI_VERSION}.tar.gz"
    download_archive \
        "https://github.com/ocornut/imgui/archive/refs/tags/${IMGUI_VERSION}.tar.gz" \
        "${archive}" "imgui-${IMGUI_VERSION}.tar.gz" \
        "https://github.com/ocornut/imgui/releases/tag/${IMGUI_VERSION}"
    mkdir -p "${prefix}"
    tar -xzf "${archive}" -C "${prefix}" --strip-components=1
    log "dear imgui ${IMGUI_VERSION} installe dans third_party/imgui"
}

main() {
    command -v curl >/dev/null || fail "curl est requis"
    command -v sha256sum >/dev/null || fail "sha256sum est requis"
    mkdir -p "${WORK_DIR}"
    case "${1:-all}" in
        libsodium) fetch_libsodium ;;
        sqlite3)   fetch_sqlite3 ;;
        imgui)     fetch_imgui ;;
        all)       fetch_libsodium; fetch_sqlite3; fetch_imgui ;;
        *)         fail "cible inconnue : $1 (libsodium|sqlite3|imgui|all)" ;;
    esac
    log "termine"
}

main "$@"
