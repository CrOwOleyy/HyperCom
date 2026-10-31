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

## What makes it different

Free and open source, community-run servers already exist (Matrix,
Revolt, Mattermost…). HyperCom's difference is what the server learns
about you:

- **A different identity on every server.** Each server gets its own key
  pair, with no visible link between them, so two administrators cannot
  tell they host the same person. On Matrix or Revolt your account is the
  same everywhere.
- **Private messages the server can't read, and can't date.** DMs are
  end-to-end encrypted, and their timestamp lives inside the ciphertext:
  the server only knows who wrote to whom, never when. Forum posts and
  profiles are public content and are readable by the server.
- **No TLS, no certificate authority.** A Noise handshake on libsodium,
  with the server's key pinned by the client.
- **A server small enough to audit.** One `epoll` loop, no mutex, one
  SQLite file.
- **Anyone can own a server.** One binary, one config file, one SQLite
  database.
- **Works over Tor.** With the `[onion]` listener, the administrator never
  sees members' IP addresses.
- **No IP address kept by default.** `log_peer_addresses` is off.

HyperCom does not federate: one community, one server. See
[docs/THREAT_MODEL.md](docs/THREAT_MODEL.md) for what is *not* protected.

**Recovery:** the master identity has a 12-word recovery phrase, shown once
at creation; `hypercom_cli seed-restore` rebuilds it on a new machine.

**Planned, in this order:** forward secrecy for DMs; attachments (images,
files); an independent audit of the handshake. None of these exist yet.


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

## Status

This is a prototype: expect bugs, and some parts are still rough around
the edges. Found one? Open an issue, or send a pull request if you've
already got a fix. For anything else, reach out at
[leyy@pepepak.fr](mailto:leyy@pepepak.fr).
