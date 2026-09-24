# ADMIN — running the server

This document covers the collaborator's domain: configuration, the
database, hardening, announcements. **None of it requires writing
C++.**

## 1. Starting up

```
cp hypercom.conf.example hypercom.conf
./hypercom_keygen server keys/server_static.key
chmod 600 keys/server_static.key
./hypercom_server hypercom.conf
```

On first launch, the server applies migrations, generates its static
key if it's missing, then **prints its public key**. That's the one
clients pin.

> It needs to be communicated over a trusted channel. Having clients
> fetch it from the server itself would defeat the entire point of
> pinning.

The server **refuses to start on an invalid configuration** rather than
falling back to silent defaults. Errors are collected and shown
together, with their line number.

### Connect file

On startup, the server also writes this information in a form the
client can read directly, at `run/hypercom-connect.txt`:

```ini
host=203.0.113.7
port=7717
server_key=447a6def06c64a36...
```

A new user hands it to the client without retyping anything:

```
./hypercom_cli --connect-file hypercom-connect.txt whoami
```

A `--host`, `--port` or `--server-key` placed **after**
`--connect-file` on the command line overrides the corresponding value
from the file.

> **The trust model doesn't change.** This file contains nothing
> secret, but it carries the key to pin: it travels over the same
> trusted channel as the key itself. Having it downloaded from the
> server it describes would defeat pinning in exactly the same way.

The path is set with `connect_file` under `[paths]`; an empty value
disables writing it. Only the clearnet listener appears in it — the
onion port lives on a local loop and has no business being in a shared
file.

`bind_address` is a **listening** address: `0.0.0.0` means "every
interface" and is reachable by nobody. The server can't guess its
public address, so declare it:

```ini
[clearnet]
bind_address    = 0.0.0.0
advertised_host = 203.0.113.7
```

`advertised_host` is what goes into the connect file. Without it, the
server writes `bind_address` and warns at startup. **Editing the file
by hand is pointless**: it's rewritten on every startup.

## 2. Configuration

Everything lives in `hypercom.conf`, `key = value` per section. See
`hypercom.conf.example`, which is commented.

### The logging policy

```ini
[logging]
log_peer_addresses = false     # default
```

**By default, the server logs no IP address at all.** Setting it to
`true` triggers an explicit warning at startup. This is deliberate: on
a network that claims to be unsurveilled, logging IPs has to be a
decision made on purpose, never a side effect.

Concrete consequence: a server seizure doesn't reveal who connected or
from where.

The only path to a log goes through `logger::redact_peer_address`,
which returns `[redacted]` as long as the policy isn't opened up.
Callers don't have to remember this themselves.

**Never for `.onion`, regardless of this setting.** Tor relays over a
local loop: the server structurally never sees a real IP there, so
`true` only ever logs clearnet connections.

**On clearnet, `log_peer_addresses = true` now genuinely logs every
accepted connection** — before, the setting did nothing as long as
nothing actually wrote to the log. If `retention_days` is under a
year, a warning flags it at startup: that's the legal floor in France
for connection data (art. L.34-1 CPCE, art. 6-II LCEN), and logging
without respecting it brings compliance only in appearance.

### Tor hidden service

The same code serves both clearnet and `.onion`: Tor simply relays to
a listener on a local loop.

```ini
[onion]
enabled      = true
bind_address = 127.0.0.1
port         = 7718
```

On the `torrc` side:

```
HiddenServiceDir /var/lib/tor/hypercom/
HiddenServicePort 7717 127.0.0.1:7718
```

The onion port must **never** be exposed to the network.

## 3. Migrations

Drop a file into `db/migrations/NNNN_description.sql`. It's applied on
the next startup, inside a transaction: it goes through entirely or
not at all. The fixed-width numeric prefix gives the ordering.

Never edit a migration that's already been applied — add a new one.

### If a migration fails

The server refuses to start and the database is **not** modified (the
migration runs inside a transaction). The message gives the offending
file and the SQLite error.

The most likely case is `0002_handle_case_insensitive` on a database
created before it existed: if two accounts have handles that only
differ by case (`leyy` and `Leyy`), the unique index can't be created.
This is deliberate — silently renaming someone's account would be
worse than stopping.

To fix it, someone has to choose which account keeps the handle. No
command exists for this yet: it has to go through `sqlite3`, with the
server stopped.

```
sqlite3 hypercom.db "SELECT id, handle FROM users ORDER BY lower(handle);"
sqlite3 hypercom.db "UPDATE users SET handle = 'leyy_old' WHERE id = 4;"
```

The account isn't lost: its identity is its public key, not its
handle. Only the displayed name changes.

## 4. Backup

```
sqlite3 hypercom.db ".backup /backups/hypercom-$(date +%F).db"
```

Hot, without stopping the server. A raw copy (`cp`) taken mid-write can
produce an inconsistent file because of WAL.

Also back up `keys/server_static.key`: losing it breaks pinning for
every client, who will then refuse to connect.

## 5. MOTD

The `motd` table holds announcements, with a partial unique index
guaranteeing only one is active. It's pushed to clients on connection.

Pending the admin CLI (see §7), it's edited directly:

```sql
UPDATE motd SET active = 0 WHERE active = 1;
INSERT INTO motd (body, active, created_at)
VALUES ('maintenance Saturday 2pm', 1, strftime('%s','now'));
```

No restart needed: it's re-read on every connection.

Simpler now that the CLI exists (see §7):

```bash
hypercom_adminctl motd set "maintenance Saturday 2pm"
```

## 6. Deployment hardening

These measures are **documented but not implemented** — they're an
operational concern, not a code one:

- privilege drop after bind; the server should never run as root;
- seccomp-bpf sandbox and Linux namespaces;
- hardened systemd unit: `NoNewPrivileges`, `ProtectSystem=strict`,
  `PrivateTmp`, `MemoryDenyWriteExecute`;
- database and keys at `0600`, dedicated owner.

## 7. The admin CLI

It listens on an **AF_UNIX** socket whose path comes from `[paths]
admin_socket`. Never TCP: there's no code path capable of exposing it
to the network. The file is created at `0600`, so only the account
running the server can connect to it — that's the entire
authentication, and it rests entirely on filesystem permissions.

```bash
hypercom_adminctl help
```

| Command | Effect |
|---|---|
| `stats` | uptime, connections, database size |
| `sessions` | current connections |
| `sessions close <descriptor>` | closes a connection |
| `motd` | current announcement |
| `motd set "text"` | publishes an announcement, immediate effect |
| `motd clear` | disables the announcement |
| `backup <path>` | hot backup, consistent snapshot |
| `reports` | pending reports |
| `reports clear <id>` | closes a report, touches nothing else |
| `reports delete-post <id>` | deletes the reported post — same real deletion as the author themselves would trigger |
| `ban <hex_key>` | revokes an account's authentication |
| `unban <hex_key>` | restores it |
| `help` | the list |

The socket path is set with `--socket` if it isn't at the default:

```bash
hypercom_adminctl --socket /var/run/hypercom-admin.sock stats
```

**`sessions` shows neither handle nor address, and that isn't
configurable.** The list gives a descriptor, a state (`handshake`,
`hello`, `auth`, `authenticated`) and two durations — enough to spot a
lingering connection or a stuck handshake, never enough to know who's
online. Unlike `log_peer_addresses`, there's no option to turn identity
back on here: a tool that lists who's connecting and since when is a
surveillance tool, not an operations tool, and the project doesn't
ship one.

`motd set` validates the text as if it came from the network: correct
UTF-8, no control characters. The announcement gets served back to
every client, and a distracted administrator has no more right than a
stranger to slip anything through it.

`backup` goes through SQLite's backup API, not a file copy — under WAL
mode, a `cp` taken mid-write produces an inconsistent file.

**`reports` and `ban` are the sole exception to the absence of
moderation**: a capability reserved for legal compliance (LCEN art.
6-I-7), never for general editorial control. `reports delete-post`
deliberately exists only on this local socket — no network protocol
message lets anyone, including a client claiming to be an
administrator, erase someone else's content. `clear` and `delete-post`
stay two separate actions: closing a report never touches the post,
deleting a post doesn't automatically close its report. `ban` revokes
authentication, without touching content the account already
published.

## 8. What's still missing

| Item | Status |
|---|---|
| Hot-reloading the configuration | **not implemented** — changing a listener would require reopening it under connections already in flight |
| Log rotation and purging | **not implemented** — `retention_days` is read and validated, nothing enforces it |
| Privilege drop after bind | **not implemented** |
| seccomp-bpf sandbox | **not implemented** |

Until hot-reloading exists, any configuration change requires
restarting the server.
