# THREAT_MODEL — what Hypercom protects, and what it doesn't

This document is written to be shown to users. It doesn't promise
anything the architecture doesn't actually hold.

## 1. The principle

**Non-surveillance is a technical property, not a promise.** The server
operator shouldn't *choose* not to read private conversations — they
should be *unable* to.

Concretely: there's no key anywhere on the server machine that can open
a private message. This isn't a policy an administrator could change,
it's an absence.

## 2. What's protected

| Threat | Protection |
|---|---|
| Network eavesdropping | Everything is encrypted with Noise_NK, including the frame type. |
| Impersonated server | Pinned static key: the handshake fails before any exchange. |
| Server operator reading DMs | It has no key. Encryption happens on the client. |
| Server forging a prekey to intercept | The prekey is signed by the identity key; the recipient verifies it. |
| Server replaying or modifying a DM | The whole header is authenticated associated data. |
| Database theft | It only holds opaque ciphertext and public posts. |
| Server seizure | No IP logged by default, no email, no password. |
| Account impersonation | Identity *is* a private key that never leaves the machine. |
| Future compromise of the chain key | Symmetric ratchet: past messages stay unreadable. |
| SQL injection | Prepared statements exclusively, no concatenation. |
| Hostile frame | Bounded parser, caps checked before allocation, fuzzed. |
| Admin tracking user presence | The admin CLI can't list who's online: `sessions` shows neither handle nor address, and it isn't configurable. |
| **Anyone** tracking someone's presence | No last-connection date is stored or served. The `last_seen` column was removed from the schema and the protocol. |
| Server dating private exchanges | The timestamp lives **inside** the ciphertext. The server no longer knows when a message was sent, only the order envelopes arrived in. |
| Correlation by account or relationship age | `users.created_at` and `friends.created_at` removed: no signup date, no timeline of social links. |
| Impersonation via handle casing | `COLLATE NOCASE` uniqueness: `alice`, `Alice` and `ALICE` are the same handle. |
| Bypassing the rate limit by reconnecting | Counters are shared across connections, not reset on each one. |
| Server stitching two connections from the same person | No resumption token exists. A reconnection is a fresh Noise handshake, with a fresh ephemeral key. See PROTOCOL.md §9. |
| Dead sessions piling up after a disconnect | TCP keepalive set on both sides (240s detection), with no application-level idle timeout that would require measuring anyone's activity. |

## 3. What's NOT protected — accepted limitations

### 3.1 Routing metadata

**The server sees who's writing to whom.** It stores `recipient_id` and
`sender_pubkey` — without them, it wouldn't know who to deliver the
envelope to.

What it no longer knows is **when**: the timestamp moved inside the
ciphertext, and arrival order (`id`) is all that's left. A seized server
therefore reveals the exchange graph, not its timing.

Hiding the graph itself would require a mix-net or blind relays, out of
scope for v1. This is the most important limitation of this model, and
it needs to be stated plainly to users.

### 3.1 bis What the server actually holds on a person

After full use (registration, forum, post, comment, friend, private
message), here's the entirety of what the database contains:

```
users        id, pubkey, handle, banned
friends      user_id, friend_id, status
top8         user_id, slot, friend_id
profiles     user_id, display_name, bio, theme_json, banner_ref
dm_envelopes id, recipient_id, sender_pubkey, ciphertext
reports      id, kind, post_id, target_pubkey, reporter_id, reason, created_at
```

No date on any of these rows, except `reports`: a report necessarily
carries a timestamp, otherwise the admin couldn't tell when they're
handling a queue. The other timestamps that remain are on **public
content** — `posts`, `comments`, `forums` — where they're visible to
anyone reading the thread anyway, and on `motd`, which is an admin
announcement.

`reports` is a deliberate exception to "no dates": reporting something
necessarily reveals who reported what and when. This is an accepted
choice, not an oversight — the reporting mechanism exists to meet a
legal obligation, not as surveillance in disguise, and its content is
only readable by the admin on the local socket, never over the network.

**Outside the database, in the log only, if `log_peer_addresses=true`:**
one line per accepted clearnet connection, address + timestamp. Never
for `.onion`, regardless of that setting — Tor relays over a local loop,
there's structurally no real IP to see on the server side. Nothing is
written by default.

### 3.2 No future forward secrecy

The ratchet is symmetric. Someone who compromises a user's identity key
can read their **future** messages. The full Diffie-Hellman ratchet,
which would close that window, is planned for v2 — the envelope format
is already ready to accommodate it.

### 3.3 Non-rotating prekey

The prekey is deterministically derived from the identity and doesn't
rotate. Forward secrecy therefore rests entirely on the ephemeral key,
regenerated on every message. Acceptable, but less robust than
rotation.

### 3.4 Lost key = lost account

No recourse, permanently. This is the price of having no authority:
nobody can reset an account, so nobody can steal one that way either. A
12-word recovery phrase is planned for v2.

### 3.5 Homegrown Noise implementation

The code in `common/crypto/noise_*` is a homegrown implementation of a
**specified** protocol, not homegrown cryptography: every primitive
comes from libsodium. The residual risk is an error in how the steps
chain together, not in the primitives themselves.

**Fact**: `tests/noise_official_vectors_test.cpp` replays the handshake
with the fixed keys of an official vector (noise-c source,
`Noise_NK_25519_ChaChaPoly_SHA256`, no PSK) and compares every byte —
handshake messages, final hash, transport keys, four transport messages
— against the reference.

This test covers a blind spot round-trip testing doesn't: a bug present
identically on both the client and server side (a wrong `mix_hash`
order, for example) would stay invisible to two parties talking to each
other while making the same mistake. Comparing against an external
reference is the only way to catch that.

### 3.6 First contact with the server's key

Pinning only protects if the key arrives through a trusted channel. If a
user fetches it from the server itself, or from a site the attacker
controls, pinning protects nothing.

### 3.7 No moderation — except the legal exception

That's the point of the project, but it's also an exposure. Filtering
lives in each person's client.

One single exception exists: `reports`/`ban` on the local admin socket.
It changes nothing about what a client can do — no network protocol
message lets anyone erase someone else's content, only their own
content stays removable. What it does change: an administrator with
hands-on access to the machine can now ban an account or delete a
specifically reported post, in response to a precise legal obligation,
never by general editorial judgment.

### 3.8 Post content

Posts are **public by nature**. Nothing encrypts them, and nothing
should: a forum readable only by its own author isn't a forum.

## 4. Assumptions

- The client machine isn't compromised. If it is, the private key is
  too, regardless of how good the protocol is.
- libsodium is correct.
- The user picks a resistant passphrase. Argon2id (MODERATE parameters,
  256 MiB) slows down an offline attack, it doesn't make one impossible
  against a weak passphrase.

## 5. State of hardening

| Measure | Status |
|---|---|
| `-Wall -Wextra -Werror`, stack protector, RELRO, PIE | **done**, applied to every target |
| ASAN / UBSAN / TSAN targets | **done**, `-DHYPERCOM_SANITIZER=...` |
| Test suite passing under ASAN+UBSAN | **done** |
| Parser fuzzing harness | **done**, covers all 36 decoders. Continuous campaign still to set up: libFuzzer requires clang, absent from the current environment |
| Fuzzing corpus replay in the suite | **done**, `tests/fuzz_corpus_replay_test.cpp` replays known edge cases under gcc and under sanitizers, without libFuzzer |
| Round trip of every message | **done**, three `message_roundtrip_*` suites: every message encoded then decoded, checked to leave no byte behind — the only way to catch a silent field shift |
| Reconnection without a resumption token | **done**, `tests/reconnection_test.cpp`, also validated under TSAN |
| Prepared statements exclusively | **done** |
| No mutable globals (G4) | **done**, including shutdown via `signalfd` rather than a global flag |
| No IP logging by default | **done** |
| Official Noise test vectors | **done**, `tests/noise_official_vectors_test.cpp` |
| Automatic log purging | **configured, not enforced** — `retention_days` is read but no rotation is implemented |
| Privilege drop after bind | **not done** |
| seccomp-bpf + namespaces | **not done** |
| Hardened systemd unit | **not done** |

The last three lines fall under deployment, and the collaborator's
domain.
