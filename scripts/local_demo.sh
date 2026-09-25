#!/usr/bin/env bash
# End-to-end local demonstration.
#
# Spins up a throwaway server in a temporary directory, creates two
# accounts, and exercises the three blocks of the v1 scope: forums,
# profiles/friends, and encrypted private messages. Everything gets
# destroyed on exit -- nothing touches your real database or your real
# keys.
#
#   ./scripts/local_demo.sh                 uses build/
#   HYPERCOM_BUILD_DIR=build-asan ./scripts/local_demo.sh
#
# Serves as both a regression test and a way to explore the project.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="$ROOT/${HYPERCOM_BUILD_DIR:-build}/bin"
PORT="${HYPERCOM_DEMO_PORT:-7817}"

if [ ! -x "$BIN/hypercom_server" ]; then
    echo "binaries not found in $BIN" >&2
    echo "build first:  cmake -S . -B build && cmake --build build -j" >&2
    exit 1
fi

WORK="$(mktemp -d)"
cleanup() {
    if [ -n "${SERVER_PID:-}" ]; then
        kill "$SERVER_PID" 2>/dev/null || true
        wait "$SERVER_PID" 2>/dev/null || true
    fi
    rm -rf "$WORK"
}
trap cleanup EXIT

cd "$WORK"
mkdir -p keys db/migrations
cp "$ROOT"/db/migrations/*.sql db/migrations/

cat > hypercom.conf <<EOF
[server]
registration_open = true
[clearnet]
enabled      = true
bind_address = 127.0.0.1
port         = $PORT
[onion]
enabled      = false
bind_address = 127.0.0.1
port         = 7818
[limits]
max_connections                  = 64
max_connections_per_address      = 16
max_frame_size                   = 1048576
handshake_timeout_seconds        = 10
idle_timeout_seconds             = 300
requests_per_minute_per_address  = 600
requests_per_minute_per_identity = 600
[logging]
level              = info
log_peer_addresses = false
retention_days     = 7
file_path          =
[paths]
database     = hypercom.db
migrations   = db/migrations
server_key   = keys/server_static.key
admin_socket = run/hypercom-admin.sock
EOF

echo "=== 0. starting the server on 127.0.0.1:$PORT ==="
"$BIN/hypercom_server" hypercom.conf > server.log 2>&1 &
SERVER_PID=$!
sleep 2

SERVER_KEY="$(grep -oE '[0-9a-f]{64}' server.log | head -1)"
if [ -z "$SERVER_KEY" ]; then
    echo "the server didn't start:" >&2
    cat server.log >&2
    exit 1
fi
echo "server public key: $SERVER_KEY"

alice() {
    HYPERCOM_PASSPHRASE='passphrase-alice' "$BIN/hypercom_cli" \
        --server-key "$SERVER_KEY" --host 127.0.0.1 --port "$PORT" \
        --identity "$WORK/alice.key" "$@"
}
bob() {
    HYPERCOM_PASSPHRASE='passphrase-bob' "$BIN/hypercom_cli" \
        --server-key "$SERVER_KEY" --host 127.0.0.1 --port "$PORT" \
        --identity "$WORK/bob.key" "$@"
}

echo
echo "=== 1. creating accounts ==="
alice register alice
bob register bob

echo
echo "=== 2. forums, posts, nested comments ==="
alice forum-create cryptography "Everything that encrypts"
alice forum-list
alice post 1 "Noise instead of TLS" "No X.509, no certificate authority."
alice posts 1
bob comment 1 0 "Agreed, the attack surface is a lot smaller."
alice comment 1 1 "And one less dependency to patch."
alice thread 1

echo
echo "=== 3. profiles and friends ==="
alice profile-set "Alice" "I break protocols."
alice profile-get "$(alice whoami | grep -oE '[0-9a-f]{64}' | head -1)"
bob friend-add "$(alice whoami | grep -oE '[0-9a-f]{64}' | head -1)"
bob friends

echo
echo "=== 4. end-to-end encrypted private messages ==="
bob prekey-publish
ALICE_KEY="$(alice whoami | grep -oE '[0-9a-f]{64}' | head -1)"
BOB_KEY="$(bob whoami | grep -oE '[0-9a-f]{64}' | head -1)"
alice prekey-publish
alice dm-send "$BOB_KEY" "The server can't read this message."
bob dm-fetch

echo
echo "=== 5. what the server actually holds in its database ==="
if command -v sqlite3 >/dev/null; then
    echo "-- raw content of a private envelope:"
    sqlite3 hypercom.db \
        "SELECT id, hex(substr(ciphertext,1,24)) || '...' FROM dm_envelopes;"
    echo "-- no IP address anywhere in the log:"
    grep -ciE '([0-9]{1,3}\.){3}[0-9]{1,3}' server.log || echo "0"
else
    echo "sqlite3 missing, direct inspection skipped"
fi

echo
echo "=== server log ==="
cat server.log

echo
echo "=== SUCCESS: all three v1 blocks work ==="
