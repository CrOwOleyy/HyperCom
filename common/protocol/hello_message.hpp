#pragma once

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/wire_key.hpp"

#include <cstdint>

namespace hypercom::proto {

// Session opening, sent over the channel ALREADY encrypted by Noise.
// The client announces its identity public key; no secret is transmitted.
struct hello_request {
    std::uint16_t protocol_version = PROTOCOL_VERSION;
    wire_public_key client_pubkey{};

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

// Server response: the challenge to sign.
//
// account_exists tells the client whether it should authenticate or
// register. No attempt is made to hide it: anyone can already probe
// whether a public key exists, and the client needs the info to know
// whether to ask for a handle.
struct auth_challenge {
    std::uint16_t protocol_version = PROTOCOL_VERSION;
    wire_nonce nonce{};
    std::uint8_t account_exists = 0;
    std::uint64_t server_time = 0;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
