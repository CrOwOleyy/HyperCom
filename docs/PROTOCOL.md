# PROTOCOL — Hypercom v1 wire protocol

Homegrown binary protocol. No HTTP, no JSON, no text.

## 1. Layers

```
  TCP  ──►  [u32 len][Noise message]        transport, encrypted
                          │
                          ▼ decryption
            [u32 body_size][u8 type][payload]   application frame
```

The outer length prefix is in the clear — it has to be, to split the
stream — but **everything it frames is encrypted**, including the type
byte. A network observer only ever sees message sizes.

## 2. Handshake — `Noise_NK_25519_ChaChaPoly_SHA256`

```
NK:
  <- s          server's static key, pinned by the client
  ...
  -> e, es      message 1
  <- e, ee      message 2  →  Split()
```

- **Prologue**: `hypercom-v1`, mixed by both peers. Binds the session to
  this application and this version.
- The suite name is exactly 32 bytes, i.e. HASHLEN: it's used directly
  as the initial hash state, without going through SHA-256.
- **Key direction**: the initiator sends with the first output of
  `Split()`, the responder with the second. Getting this backwards
  produces a channel that establishes fine but then fails on the first
  message — hence the bidirectional test.

A client that pins the wrong key **fails on the very first message**,
without having revealed anything. This is the full replacement for the
certificate chain: no authority to query, just a key to compare.

## 3. Application framing

```
[u32 body_size][u8 type][payload]
```

`body_size` counts the type byte and the payload, not the field itself:
`1 ≤ body_size ≤ MAX_FRAME_SIZE - 4`, i.e. 1 MiB total.

**Non-negotiable parser rules** (`common/protocol/`):

- Every read goes through `byte_reader`, which never steps outside its
  buffer.
- Every announced size is compared against its cap **before** any
  allocation. A frame announcing 4 GiB is rejected without reserving a
  single byte.
- A failure consumes nothing and doesn't write the output.
- An unknown type is rejected by `decode_frame_header`, never handed to
  a handler.

The parser is stateless, has no I/O, and has no link to libsodium —
precisely so it can be fuzzed on its own (`tools/fuzz/`).

## 4. Field encoding

| Type | Encoding |
|---|---|
| integers | little-endian, fixed size |
| text | `[u32 size][UTF-8 bytes]`, validated |
| blob | `[u32 size][bytes]` |
| public key / signature | 32 / 64 raw bytes |
| list | `[u16 count][elements]`, count capped |
| timestamp | `u64`, UNIX UTC seconds |

The same `u32` prefix serves both text and blobs: two extra bytes per
string, in exchange for a single decoding path to audit.

**Text validation** — accepted text gets served back as-is to other
clients, so validation isn't cosmetic. Rejected: invalid or overlong
UTF-8, surrogate halves, out-of-plane code points, C0 controls except
tab and newline, DEL, and carriage return.

## 5. Session opening sequence

```
C → S   hello_request      { version, public key }
S → C   auth_challenge     { 32-byte nonce, version, account_exists }
C → S   auth_response      { Ed25519 signature }
S → C   auth_accepted      { user_id, handle, time }         if the account exists
        or status_ok       { 0 }                              otherwise
C → S   register_request   { handle }                         if needed
S → C   auth_accepted
S → C   motd_push                                             if a MOTD is active
```

The client signs `"hypercom-auth-v1" || nonce || public_key`. Domain
separation prevents a signature produced here from being valid in a
different context.

No password is transmitted, stored, or even exists server-side.

## 6. Message families

| Range | Family |
|---|---|
| `0x0*` | session: hello, auth, ping, motd, status |
| `0x1*` | account: register, prekey publish/fetch |
| `0x2*` | forums: create, list |
| `0x3*` | content: post create/list, thread, comment |
| `0x4*` | social: profile, friends, top8 |
| `0x5*` | private: dm send/fetch/ack |
| `0x6*` | blobs — **reserved for v2**, values set aside so adding P2P doesn't renumber anything |

Values are frozen: they're part of the wire format.

## 7. Private message envelope

Opaque to the server, defined by `common/crypto/dm_envelope`:

```
[u8 version][32 sender identity][32 ephemeral][u32 counter][ciphertext+tag]
└──────────────── authenticated associated data ────────────────┘
```

The header travels in the clear — the recipient needs it to derive the
key — but it's fully authenticated. Modifying a single byte, including
the counter, makes decryption fail: the server can neither replay it
under a different counter, nor spoof the sender.

The nonce is zero, and that's safe **only** because the key is unique
per message: the `dm_message_chain` ratchet never produces the same one
twice.

## 8. Error codes

Deliberately coarse. `unknown_user` and `invalid_signature` both reply
`authentication_failed`: a too-precise code would tell an attacker
about the server's internal state.

An error code is never meant to be read by a human — that's the job of
the `detail` field, which can change without notice. Wiring client
logic to `detail` instead of `code` is a bug.

Three errors are **fatal to the connection**, because a desynchronized
stream can't be recovered: `malformed_frame`, a length announced past
the cap, and a decryption failure. In all three cases the connection is
closed, never resumed.

## 9. Connection lifecycle

### Lifetime

**No application-level idle timeout.** A session stays open as long as
the peer is there. This is consistent with the rest: a server-side
timeout would require measuring everyone's activity, and therefore
keeping a record of it.

Keeping the connection alive relies entirely on **TCP keepalive**,
enabled on both sides with the same settings
(`server/net/tcp_listener.cpp` and `client/net/tcp_client_socket.cpp`):

| Parameter | Value | Effect |
|---|---|---|
| `SO_KEEPALIVE` | enabled | automatic probes |
| `TCP_KEEPIDLE` | 120s | delay before the first probe |
| `TCP_KEEPINTVL` | 30s | interval between two probes |
| `TCP_KEEPCNT` | 4 | probes before giving up |

A peer that disappears without `FIN` — network drop, crash, dropping
out of Tor's range — is therefore detected in about 240s. Without this
setting, Linux would wait two hours, and dead sessions would pile up.

Both ends probe at the same cadence, deliberately: otherwise it's
always the same side declaring the connection dead.

### Closing

**TCP `FIN`, no goodbye message.** There's no `close` message in the
protocol. An explicit goodbye wouldn't add anything — it isn't reliable
anyway, since an abrupt drop never sends one — and the client has to be
able to handle a disappearance without warning regardless.

### Reconnection — full re-handshake

A reconnection is an **entirely new session**: a new Noise handshake
with a fresh ephemeral key, then a new challenge-response.

**No resumption token exists.** This is a choice, not an oversight: a
session token would be exactly what would let the server stitch two
connections from the same person together, and so reconstruct a
continuity of presence that the rest of the architecture works to avoid
producing (see THREAT_MODEL.md §2, the lines on presence and
`last_seen`).

The accepted trade-off is cost: a handshake and a signature on every
reconnect. At the scale targeted — a few hundred users — that's
negligible.

In code, `server_connection::open_session` **is** the reconnection:
calling it again on a dropped connection resets the handshake, the
channel, and the input buffer before starting over. Nothing from the
previous session survives. `tests/reconnection_test.cpp` verifies this
cycle on the same object.

> This path only matters for long-lived clients, i.e. the graphical
> client. The CLI spawns a new process per command and so does a full
> handshake every time regardless.

### What the client has to resync after a reconnect

The server doesn't remember where the client was. Catching up is the
client's job, and the protocol is designed to make that possible
without any server-side state:

| Data | How |
|---|---|
| private messages | `DM_FETCH` with `since_id` = last envelope received |
| posts and threads | `POST_LIST` / `THREAD_FETCH`, `offset` pagination |
| MOTD | pushed by the server again on every connection |

## 10. Versioning and evolution

`PROTOCOL_VERSION` is 1. It travels in `hello_request` and
`auth_challenge`.

**Rule: whatever isn't recognized gets rejected.** No tolerance, no
fields silently ignored. A decoder that accepts what it doesn't
understand is a decoder whose actual behavior nobody can be sure of
anymore.

Rejection applies at three levels, independently:

1. **Version** — a `hello_request` with a different version gets
   `unsupported_version` and doesn't open a session
   (`server/handlers/session_handler.cpp`).
2. **Unknown type byte** — rejected by `decode_frame_header`, before any
   handler sees it.
3. **Known but unexpected type** — a server response sent by a client,
   or a blob message reserved for v2, gets `not_implemented`
   (`server/handlers/request_router.cpp`).

### Evolving a message

`message_type` values are **frozen**: they're part of the wire format
and don't get renumbered. The `0x6*` range is already reserved for v2's
blobs for exactly this reason.

A field therefore does **not** get added to an existing message — a v1
client would read the following fields shifted. To extend: create a new
message type at a free value within the family. Both versions coexist,
and a v1 client cleanly rejects what it doesn't know about.

This is more verbose than a self-describing format, and that's the
point: the decoder stays a sequential read of fields at known
positions, with no conditional branch driven by the data received.
