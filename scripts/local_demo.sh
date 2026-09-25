#!/usr/bin/env bash
# Demonstration locale de bout en bout.
#
# Monte un serveur jetable dans un repertoire temporaire, cree deux comptes,
# et exerce les trois blocs du perimetre v1 : forums, profils/amis, et messages
# prives chiffres. Tout est detruit en sortant -- rien ne touche votre vraie
# base ni vos vraies cles.
#
#   ./scripts/local_demo.sh                 utilise build/
#   HYPERCOM_BUILD_DIR=build-asan ./scripts/local_demo.sh
#
# Sert autant de test de non-regression que de decouverte du projet.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="$ROOT/${HYPERCOM_BUILD_DIR:-build}/bin"
PORT="${HYPERCOM_DEMO_PORT:-7817}"

if [ ! -x "$BIN/hypercom_server" ]; then
    echo "binaires introuvables dans $BIN" >&2
    echo "construire d'abord :  cmake -S . -B build && cmake --build build -j" >&2
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

echo "=== 0. demarrage du serveur sur 127.0.0.1:$PORT ==="
"$BIN/hypercom_server" hypercom.conf > server.log 2>&1 &
SERVER_PID=$!
sleep 2

SERVER_KEY="$(grep -oE '[0-9a-f]{64}' server.log | head -1)"
if [ -z "$SERVER_KEY" ]; then
    echo "le serveur n'a pas demarre :" >&2
    cat server.log >&2
    exit 1
fi
echo "cle publique du serveur : $SERVER_KEY"

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
echo "=== 1. creation des comptes ==="
alice register alice
bob register bob

echo
echo "=== 2. forums, posts, commentaires imbriques ==="
alice forum-create cryptographie "Tout ce qui chiffre"
alice forum-list
alice post 1 "Noise plutot que TLS" "Pas de X.509, pas d autorite de certification."
alice posts 1
bob comment 1 0 "D accord, la surface d attaque est bien plus petite."
alice comment 1 1 "Et une dependance de moins a patcher."
alice thread 1

echo
echo "=== 3. profils et amis ==="
alice profile-set "Alice" "Je casse des protocoles."
alice profile-get "$(alice whoami | grep -oE '[0-9a-f]{64}' | head -1)"
bob friend-add "$(alice whoami | grep -oE '[0-9a-f]{64}' | head -1)"
bob friends

echo
echo "=== 4. messages prives chiffres de bout en bout ==="
bob prekey-publish
ALICE_KEY="$(alice whoami | grep -oE '[0-9a-f]{64}' | head -1)"
BOB_KEY="$(bob whoami | grep -oE '[0-9a-f]{64}' | head -1)"
alice prekey-publish
alice dm-send "$BOB_KEY" "Ce message, le serveur ne peut pas le lire."
bob dm-fetch

echo
echo "=== 5. ce que le serveur a reellement en base ==="
if command -v sqlite3 >/dev/null; then
    echo "-- contenu brut d une enveloppe privee :"
    sqlite3 hypercom.db \
        "SELECT id, hex(substr(ciphertext,1,24)) || '...' FROM dm_envelopes;"
    echo "-- aucune adresse IP nulle part dans le journal :"
    grep -ciE '([0-9]{1,3}\.){3}[0-9]{1,3}' server.log || echo "0"
else
    echo "sqlite3 absent, inspection directe ignoree"
fi

echo
echo "=== journal du serveur ==="
cat server.log

echo
echo "=== SUCCES : les trois blocs de la v1 fonctionnent ==="
