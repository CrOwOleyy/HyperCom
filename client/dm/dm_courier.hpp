#pragma once

#include "common/crypto/identity_keypair.hpp"
#include "common/protocol/prekey_fetch_message.hpp"

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace hypercom::client {

// End-to-end encryption of private messages, client side only.
//
// Nothing in this file exists server side, and that's the whole point:
// there's no code, no key, and no path over there that could open an
// envelope.

// The prekey is DERIVED from the identity rather than stored: HKDF of the
// Ed25519 seed. The practical consequence is that there's only one secret
// to back up, and the prekey turns out identical on whatever machine the
// key is restored on.
//
// A deliberate tradeoff in v1: this prekey doesn't rotate. Forward
// secrecy therefore relies entirely on the ephemeral key, regenerated for
// every message. Prekey rotation is a planned v2 addition, designed to
// need no change to the envelope format.
[[nodiscard]] bool derive_local_prekey(crypto::identity_keypair const &identity,
                                       crypto::x25519_public_key &public_out,
                                       crypto::x25519_secret_key &secret_out);

// Call BEFORE using a prekey served by the server. A server substituting
// its own prekey to interpose itself would fail here: it doesn't have the
// target person's identity key.
[[nodiscard]] bool
verify_prekey_bundle(proto::prekey_bundle_response const &bundle);

[[nodiscard]] bool
seal_direct_message(crypto::identity_keypair const &sender,
                    proto::prekey_bundle_response const &recipient_bundle,
                    std::string_view text, std::vector<std::uint8_t> &out,
                    std::string &error_out);

// sent_at_out comes from INSIDE the ciphertext: the server doesn't know
// the send date, only the recipient recovers it by decrypting.
[[nodiscard]] bool
open_direct_message(crypto::identity_keypair const &recipient,
                    std::span<std::uint8_t const> envelope,
                    std::string &text_out, std::uint64_t &sent_at_out,
                    std::string &error_out);

} // namespace hypercom::client
