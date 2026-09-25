#pragma once

#include "common/crypto/key_types.hpp"

#include <cstddef>
#include <string_view>

namespace hypercom::crypto {

// Noise suite chosen as a replacement for TLS.
//
//   Noise_NK_25519_ChaChaPoly_SHA256
//
// NK: the server is authenticated by its static key, known to the client
// in advance (pinned or entered on first connection). The client stays
// anonymous at the transport level and authenticates afterward at the
// application level. No X.509, no certificate authority, no certificate
// parser.
//
// The name is exactly 32 bytes, i.e. HASHLEN: it is therefore used as-is
// as the initial hash state, without going through SHA-256, per the
// specification.
//
// TRADE-OFF WORTH KNOWING: this code is a homegrown implementation of a
// specified protocol, not homegrown crypto -- all the primitives come from
// libsodium. The residual risk is a mistake in how the steps are chained
// together, not in the primitives themselves. This is why
// noise_handshake_test verifies the full run and why the direction of the
// transport keys is tested both ways. Validation against the official
// Noise test vectors is still pending and is noted in
// docs/THREAT_MODEL.md.
constexpr std::string_view NOISE_PROTOCOL_NAME =
    "Noise_NK_25519_ChaChaPoly_SHA256";

// The prologue is mixed into the hash by both peers. It ties the session
// to this application and this version: a Hypercom handshake cannot be
// replayed against another service that shares the same Noise suite.
constexpr std::string_view NOISE_PROLOGUE = "hypercom-v1";

constexpr std::size_t NOISE_HANDSHAKE_MESSAGE_ONE_SIZE =
    X25519_PUBLIC_KEY_SIZE + AEAD_TAG_SIZE;
constexpr std::size_t NOISE_HANDSHAKE_MESSAGE_TWO_SIZE =
    X25519_PUBLIC_KEY_SIZE + AEAD_TAG_SIZE;

} // namespace hypercom::crypto
