#pragma once

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/wire_key.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace hypercom::proto {

// Domain separation: a signature produced here is only valid for Hypercom
// authentication. Without this prefix, a malicious server could hand the
// client a "challenge" that is actually the hash of a message meant to be
// signed in another context, then replay the signature elsewhere.
constexpr std::string_view AUTH_SIGNATURE_DOMAIN = "hypercom-auth-v1";

struct auth_response {
    wire_signature signature{};

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

struct auth_accepted {
    std::uint64_t user_id = 0;
    std::string handle;
    std::uint64_t server_time = 0;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

// Builds the exact bytes to sign: domain, then nonce, then the announced
// public key. Both client and server go through here -- it's the only way
// to be sure they're signing and verifying the same thing.
void build_auth_signing_input(wire_nonce const &nonce,
                              wire_public_key const &client_pubkey,
                              std::vector<std::uint8_t> &out);

} // namespace hypercom::proto
