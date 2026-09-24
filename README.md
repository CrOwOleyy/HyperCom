# HyperCom

**English** · [Français](docs/readme/README.fr.md) · [中文](docs/readme/README.zh.md) · [हिन्दी](docs/readme/README.hi.md) · [Español](docs/readme/README.es.md) · [العربية](docs/readme/README.ar.md) · [বাংলা](docs/readme/README.bn.md) · [Português](docs/readme/README.pt.md) · [Русский](docs/readme/README.ru.md) · [日本語](docs/readme/README.ja.md)

Decentralized, end-to-end encrypted social network: Reddit-style community
forums, MySpace-style profiles, private messages no server can read. Built
from scratch in C++20, no TLS, no web dependency — a Noise protocol
handshake written by hand on top of libsodium.

Each community runs its own server, Discord-style, instead of one central
service. A single master identity lets you join as many servers as you
like, each with a separate, uncorrelatable identity.

Full documentation: [ARCHITECTURE.md](ARCHITECTURE.md) (how the pieces fit
together), [docs/ADMIN.md](docs/ADMIN.md) (running a server),
[docs/THREAT_MODEL.md](docs/THREAT_MODEL.md) (what's protected and what
isn't), [SECURITY.md](SECURITY.md) (reporting a vulnerability),
[COMMANDS.md](COMMANDS.md) (full command reference).

## Requirements

- CMake ≥ 3.20, a C++20 compiler (MSVC on Windows, GCC/Clang on Linux)
- libsodium, SQLite and Dear ImGui: fetched once through a script, never
  pulled automatically by CMake

**The server only builds on Linux/WSL** (it relies on epoll and signalfd).
The client — CLI and GUI — builds on both Windows and Linux.

## Server side (Linux / WSL)

```bash
./scripts/fetch_third_party.sh
cmake -S . -B build/linux
cmake --build build/linux -j
```

First-time setup:

```bash
./build/linux/bin/hypercom_keygen server keys/server_static.key
chmod 600 keys/server_static.key
```

Create `hypercom.conf` at the repo root (every option is documented in
`docs/ADMIN.md §1-2`):

```ini
[server]
registration_open = true

[clearnet]
enabled      = true
bind_address = 0.0.0.0
port         = 7717

[onion]
enabled = false

[limits]
max_connections           = 512
handshake_timeout_seconds = 10
idle_timeout_seconds      = 0

[paths]
database     = hypercom.db
server_key   = keys/server_static.key
admin_socket = run/hypercom-admin.sock
```

Then:

```bash
./build/linux/bin/hypercom_server hypercom.conf
```

On startup, the server prints its public key and writes
`run/hypercom-connect.txt`. That file — or just the key inside it — is what
you hand to someone joining the server, over a channel you trust, never by
having them fetch it from the server itself.

Managing a running server (sessions, MOTD, reports, bans):
`docs/ADMIN.md §7`.

## Client side (Windows or Linux)

```powershell
.\scripts\fetch_third_party.ps1
cmake -S . -B build/windows
cmake --build build/windows --config RelWithDebInfo -j
```

```bash
./scripts/fetch_third_party.sh
cmake -S . -B build/linux
cmake --build build/linux -j
```

Load the environment once it's built — this gives you the `hserver`,
`hgui` and `hcli` shortcuts:

```powershell
cp env.example.ps1 env.ps1   # once, then edit with your own values
. .\env.ps1
```

Join a server using an invite link
(`hypercom://host:port#key`):

```
hcli server-add hypercom://203.0.113.7:7717#447a6def... my-server
hcli --server my-server whoami
```

Or directly with the connect file an admin handed you:

```
hcli --connect-file hypercom-connect.txt whoami
```

Launch the graphical client:

```
hgui
```

A second account, for testing two identities side by side locally:

```
hgui --identity account2.key
```

## Testing

```bash
cmake --build build/windows --config RelWithDebInfo -j
ctest --test-dir build/windows -C RelWithDebInfo --output-on-failure
```

Test suite details, sanitizer builds and the client's welcome sequence:
see [COMMANDS.md](COMMANDS.md).
