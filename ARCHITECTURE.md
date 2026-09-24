# Architecture

**English** · [Français](docs/architecture/ARCHITECTURE.fr.md) · [中文](docs/architecture/ARCHITECTURE.zh.md) · [हिन्दी](docs/architecture/ARCHITECTURE.hi.md) · [Español](docs/architecture/ARCHITECTURE.es.md) · [العربية](docs/architecture/ARCHITECTURE.ar.md) · [বাংলা](docs/architecture/ARCHITECTURE.bn.md) · [Português](docs/architecture/ARCHITECTURE.pt.md) · [Русский](docs/architecture/ARCHITECTURE.ru.md) · [日本語](docs/architecture/ARCHITECTURE.ja.md)

This document explains how the pieces fit together: what happens between
the moment a client connects and the moment a message lands in a forum
thread or a DM inbox. For the detail on a specific topic, the other docs
go further: [PROTOCOL.md](docs/PROTOCOL.md) for the wire format,
[SCHEMA.md](docs/SCHEMA.md) for the database, [THREAT_MODEL.md](docs/THREAT_MODEL.md)
for what's protected or not, [ADMIN.md](docs/ADMIN.md) for running a
server.

## The server never runs anything in parallel

`server_runtime::run_until_stopped` is a single `epoll` loop. No
thread-per-connection, no pool. Every connection, every database query,
every crypto operation passes through that same loop one after another,
and there isn't a single mutex anywhere in `server/` — there's nothing to
protect since nothing ever runs concurrently. Two requests touching the
same row at the same time, a rate-limit counter corrupted by a concurrent
write: that whole family of bugs simply has nowhere to happen. The
trade-off shows up as a ceiling on throughput, but for a server serving
one community rather than a service with millions of users, the single
loop never becomes the bottleneck.

## From socket to forum thread

An incoming message travels through these layers, in this order:

```
TCP socket
  → raw byte buffer (connection_socket)
  → length-prefix stripping (extract_length_prefixed_message)
  → Noise handshake in progress, or decryption once established
  → frame header decoding (frame_codec)
  → rate limiting (per address, then per identity)
  → routing by message family (request_router)
  → handler (account_handler, forum_handler, dm_handler, ...)
  → repository (post_repository, dm_repository, ...)
  → SQLite
```

The first fork — handshake or application frame — is decided in
`connection_processor.cpp`. As long as `channel.is_established()` returns
false, every incoming message advances the Noise handshake instead of
being treated as a request; once the channel is established, everything
that arrives gets decrypted and read as a frame.

Routing is split into families (session, content, social, DM, report)
rather than one giant `switch` over every message type: the project's
coding rules cap a function at sixty lines, and a switch covering the
twenty-odd message types that exist today would blow well past that.
`route_message` tries each family in turn and stops as soon as one of
them recognizes the type.

Each handler only knows its own job — `handle_dm_send_request` has no
idea the `posts` table exists. What they share is the `handler_context`
(config, logger, database connection, client connection) and the
repositories, which are the only place in the codebase that touches
SQLite directly. A handler building its own SQL query would be a red flag
at this point.

## The encrypted channel

The transport isn't TLS — there's no web client to satisfy, and TLS
drags along X.509 and certificate authorities, neither of which this
project has any use for. Instead: Noise NK on top of libsodium. The
client already knows the server's static public key (pinned on first
connection, or read from a shared connect file); the handshake simply
fails if an impostor server tries to answer in its place.

Once the handshake completes (the step the Noise protocol calls `Split`),
each direction of the conversation gets its own key and its own nonce
counter inside `noise_transport`. A message from the client and a message
from the server can therefore never share a nonce — if they did,
ChaCha20-Poly1305 would stop being safe. Everything that follows,
application protocol included, exists in the clear only on the two
machines at either end.

## One client, many servers

The client takes more after Discord than Slack: a single window, a
server bar down the side, and each server keeping its own identity. An
identity shared across two servers would be an identifier two
administrators could cross-reference to establish it's the same person on
both — which the project avoids by giving each server its own key pair,
with no visible link between them.

Two structures split the state:

- `app_state` holds whatever belongs to the application as a whole: the
  language, the active slot, the passphrase kept in memory for the
  session (never written to disk).
- `server_slot` holds everything that belongs to *one* server: its
  connection, its identity, its session, and the view state of whatever
  it's showing.

Slots live in a `std::vector<std::unique_ptr<server_slot>>`, never by
value. The reason comes down to an implementation detail that's easy to
break by accident: `client_session` keeps references into
`server_connection` and into the slot's identity. If the vector held
`server_slot` by value, a `push_back` that triggers a reallocation would
silently invalidate those references — the `unique_ptr` pins the slot's
address for good, so adding a server never moves the ones already there.

## Where to look for what

| Question | Directory |
|---|---|
| How a message is structured on the wire | `common/protocol/` |
| Encryption, key derivation, DMs | `common/crypto/` |
| Network loop, rate limiting, sessions | `server/net/` |
| SQLite access | `server/db/` |
| Business logic per message type | `server/handlers/` |
| Admin commands (local socket) | `server/admin/` |
| Connection, keystore, multi-server | `client/net/`, `client/keystore/` |
| ImGui interface | `client/ui/` |
