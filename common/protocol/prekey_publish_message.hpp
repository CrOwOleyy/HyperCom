#pragma once

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/wire_key.hpp"

#include <cstdint>
#include <string_view>
#include <vector>

namespace hypercom::proto {

constexpr std::string_view PREKEY_SIGNATURE_DOMAIN = "hypercom-prekey-v1";

// Publication of the X25519 prekey, signed by the Ed25519 identity key.
//
// This is what lets the server distribute prekeys without being able to
// forge them: it serves the signature along with it, and the recipient
// verifies it against the identity key they already know. The server
// remains a directory, never an authority.
struct prekey_publish_request {
    wire_public_key prekey{};
    wire_signature signature{};

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

// Same role as build_auth_signing_input: guarantee that the signer and the
// verifier operate on exactly the same bytes. The identity key is included
// so that a signed prekey can't be reattached to another identity.
void build_prekey_signing_input(wire_public_key const &identity_pubkey,
                                wire_public_key const &prekey,
                                std::vector<std::uint8_t> &out);

} // namespace hypercom::proto
