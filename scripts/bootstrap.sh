#!/usr/bin/env bash
# One-command setup: checks dependencies, builds, tests.
#
#   ./scripts/bootstrap.sh
#
# The script installs NOTHING on its own. If something is missing, it
# prints the exact command and asks for confirmation. Answering no is
# still useful: the list of missing packages is shown, to install
# yourself.

set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

BOLD=$'\033[1m'; GREEN=$'\033[0;32m'; RED=$'\033[0;31m'
YELLOW=$'\033[0;33m'; RESET=$'\033[0m'

say()  { printf '%s==>%s %s\n' "$GREEN" "$RESET" "$*"; }
warn() { printf '%s /!\\%s %s\n' "$YELLOW" "$RESET" "$*"; }
die()  { printf '%s error:%s %s\n' "$RED" "$RESET" "$*" >&2; exit 1; }

# --- 1. which package manager? -----------------------------------------------

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

# --- 2. what's missing? -------------------------------------------------------

MISSING=()

check_tool() {
    if command -v "$1" >/dev/null; then
        printf '  %-24s %sok%s\n' "$1" "$GREEN" "$RESET"
    else
        printf '  %-24s %smissing%s\n' "$1" "$RED" "$RESET"
        MISSING+=("$1")
    fi
}

check_library() {
    local name="$1" label="$2" required="$3"
    if pkg-config --exists "$name" 2>/dev/null; then
        printf '  %-24s %sok%s (%s)\n' "$label" "$GREEN" "$RESET" \
            "$(pkg-config --modversion "$name")"
    elif [ "$required" = "optional" ]; then
        printf '  %-24s %smissing, optional%s\n' "$label" "$YELLOW" "$RESET"
    else
        printf '  %-24s %smissing%s\n' "$label" "$RED" "$RESET"
        MISSING+=("$label")
    fi
}

say "${BOLD}checking the environment${RESET}"
check_tool cmake
check_tool g++
check_tool pkg-config
if command -v pkg-config >/dev/null; then
    check_library libsodium libsodium required
    check_library sqlite3   sqlite3   required
    # Without glfw3, only the graphical client is missing. Everything else builds.
    check_library glfw3     glfw3     optional
else
    warn "pkg-config missing, can't check libraries"
fi

if [ ${#MISSING[@]} -gt 0 ]; then
    echo
    warn "missing: ${MISSING[*]}"
    PM="$(detect_package_manager)"
    if [ "$PM" = none ]; then
        die "no recognized package manager.
Install by hand: libsodium (dev), sqlite3 (dev), cmake, a C++20 compiler.
Otherwise, the vendored version: ./scripts/fetch_third_party.sh"
    fi
    CMD="$(install_command_for "$PM")"
    echo
    echo "  suggested command:"
    echo "    ${BOLD}${CMD}${RESET}"
    echo
    read -r -p "  run it now? [y/N] " answer
    case "$answer" in
        [yY]*)
            eval "$CMD" || die "installation failed"
            ;;
        *)
            echo "  nothing was installed. Re-run this script once it's done."
            exit 1
            ;;
    esac
fi

# --- 3. build ------------------------------------------------------------------

echo
say "${BOLD}building${RESET}"
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo >/dev/null \
    || die "cmake configuration failed"

JOBS="$(command -v nproc >/dev/null && nproc || echo 4)"
cmake --build build -j "$JOBS" || die "build failed"

# --- 4. test ---------------------------------------------------------------

echo
say "${BOLD}unit tests${RESET}"
ctest --test-dir build --output-on-failure || die "some tests failed"

# --- 5. summary ---------------------------------------------------------------

echo
say "${BOLD}binaries built${RESET}"
for binary in build/bin/*; do
    [ -x "$binary" ] && printf '  %s\n' "$binary"
done

if [ ! -x build/bin/hypercom_server ]; then
    echo
    warn "hypercom_server missing: the server requires Linux (epoll).
      Under Windows, develop it in WSL2."
fi
if [ ! -x build/bin/hypercom_client ]; then
    echo
    warn "hypercom_client missing: glfw3 and Dear ImGui are required.
      For ImGui:  ./scripts/fetch_third_party.sh imgui
      The CLI client, meanwhile, is ready."
fi

echo
say "${BOLD}ready.${RESET}"
echo "  full demo       : ./scripts/local_demo.sh"
echo "  picking up the code: see CONTRIBUTING.md"
