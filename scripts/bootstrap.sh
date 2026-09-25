#!/usr/bin/env bash
# Mise en route en une commande : verifie les dependances, compile, teste.
#
#   ./scripts/bootstrap.sh
#
# Le script n'installe RIEN tout seul. S'il manque quelque chose, il affiche la
# commande exacte et demande confirmation. Repondre non reste utile : la liste
# des paquets manquants est affichee, a installer soi-meme.

set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

BOLD=$'\033[1m'; GREEN=$'\033[0;32m'; RED=$'\033[0;31m'
YELLOW=$'\033[0;33m'; RESET=$'\033[0m'

say()  { printf '%s==>%s %s\n' "$GREEN" "$RESET" "$*"; }
warn() { printf '%s /!\\%s %s\n' "$YELLOW" "$RESET" "$*"; }
die()  { printf '%s echec :%s %s\n' "$RED" "$RESET" "$*" >&2; exit 1; }

# --- 1. quel gestionnaire de paquets ? ---------------------------------------

detect_package_manager() {
    if command -v apt-get >/dev/null; then echo apt
    elif command -v dnf     >/dev/null; then echo dnf
    elif command -v pacman  >/dev/null; then echo pacman
    elif command -v brew    >/dev/null; then echo brew
    else echo none
    fi
}

packages_for() {
    case "$1" in
        apt)    echo "build-essential cmake pkg-config libsodium-dev libsqlite3-dev libglfw3-dev" ;;
        dnf)    echo "gcc-c++ cmake pkgconf-pkg-config libsodium-devel sqlite-devel glfw-devel" ;;
        pacman) echo "base-devel cmake pkgconf libsodium sqlite glfw" ;;
        brew)   echo "cmake pkg-config libsodium sqlite glfw" ;;
        *)      echo "" ;;
    esac
}

install_command_for() {
    case "$1" in
        apt)    echo "sudo apt-get install -y $(packages_for apt)" ;;
        dnf)    echo "sudo dnf install -y $(packages_for dnf)" ;;
        pacman) echo "sudo pacman -S --needed $(packages_for pacman)" ;;
        brew)   echo "brew install $(packages_for brew)" ;;
        *)      echo "" ;;
    esac
}

# --- 2. que manque-t-il ? ----------------------------------------------------

MISSING=()

check_tool() {
    if command -v "$1" >/dev/null; then
        printf '  %-24s %sok%s\n' "$1" "$GREEN" "$RESET"
    else
        printf '  %-24s %smanquant%s\n' "$1" "$RED" "$RESET"
        MISSING+=("$1")
    fi
}

check_library() {
    local name="$1" label="$2" required="$3"
    if pkg-config --exists "$name" 2>/dev/null; then
        printf '  %-24s %sok%s (%s)\n' "$label" "$GREEN" "$RESET" \
            "$(pkg-config --modversion "$name")"
    elif [ "$required" = "optionnel" ]; then
        printf '  %-24s %sabsent, optionnel%s\n' "$label" "$YELLOW" "$RESET"
    else
        printf '  %-24s %smanquant%s\n' "$label" "$RED" "$RESET"
        MISSING+=("$label")
    fi
}

say "${BOLD}verification de l'environnement${RESET}"
check_tool cmake
check_tool g++
check_tool pkg-config
if command -v pkg-config >/dev/null; then
    check_library libsodium libsodium requis
    check_library sqlite3   sqlite3   requis
    # Sans glfw3, seuls le client graphique manque. Le reste se construit.
    check_library glfw3     glfw3     optionnel
else
    warn "pkg-config absent, impossible de verifier les bibliotheques"
fi

if [ ${#MISSING[@]} -gt 0 ]; then
    echo
    warn "il manque : ${MISSING[*]}"
    PM="$(detect_package_manager)"
    if [ "$PM" = none ]; then
        die "aucun gestionnaire de paquets reconnu.
Installer a la main : libsodium (dev), sqlite3 (dev), cmake, un compilateur C++20.
Sinon, la version vendorisee : ./scripts/fetch_third_party.sh"
    fi
    CMD="$(install_command_for "$PM")"
    echo
    echo "  commande proposee :"
    echo "    ${BOLD}${CMD}${RESET}"
    echo
    read -r -p "  la lancer maintenant ? [o/N] " answer
    case "$answer" in
        [oOyY]*)
            eval "$CMD" || die "l'installation a echoue"
            ;;
        *)
            echo "  rien n'a ete installe. Relancer ce script apres l'avoir fait."
            exit 1
            ;;
    esac
fi

# --- 3. compiler -------------------------------------------------------------

echo
say "${BOLD}compilation${RESET}"
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo >/dev/null \
    || die "la configuration cmake a echoue"

JOBS="$(command -v nproc >/dev/null && nproc || echo 4)"
cmake --build build -j "$JOBS" || die "la compilation a echoue"

# --- 4. tester ---------------------------------------------------------------

echo
say "${BOLD}tests unitaires${RESET}"
ctest --test-dir build --output-on-failure || die "des tests echouent"

# --- 5. resume ---------------------------------------------------------------

echo
say "${BOLD}binaires construits${RESET}"
for binary in build/bin/*; do
    [ -x "$binary" ] && printf '  %s\n' "$binary"
done

if [ ! -x build/bin/hypercom_server ]; then
    echo
    warn "hypercom_server absent : le serveur exige Linux (epoll).
      Sous Windows, le developper dans WSL2."
fi
if [ ! -x build/bin/hypercom_client ]; then
    echo
    warn "hypercom_client absent : il faut glfw3 et Dear ImGui.
      Pour ImGui :  ./scripts/fetch_third_party.sh imgui
      Le client CLI, lui, est pret."
fi

echo
say "${BOLD}pret.${RESET}"
echo "  demo complete   : ./scripts/local_demo.sh"
echo "  reprendre le code : voir CONTRIBUTING.md"
