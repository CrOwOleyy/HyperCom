#pragma once

#include "common/crypto/key_types.hpp"
#include "common/crypto/noise_symmetric_state.hpp"

#include <cstdint>
#include <span>
#include <vector>

namespace hypercom::crypto {

// Server side of the NK pattern. Mirror image of
// noise_handshake_initiator.
//
// The server knows nothing about the client at the transport level: NK
// only authenticates the responder. The client's identity is established
// afterward, at the application level, by signing the challenge
// (BRIEF.md 6). This separation is deliberate: it lets a client connect
// anonymously, for instance just to read a public forum.
class noise_handshake_responder {
public:
    noise_handshake_responder(x25519_public_key const &static_public,
                              x25519_secret_key const &static_secret);

    [[nodiscard]] bool
    read_first_message(std::span<std::uint8_t const> input,
                       std::vector<std::uint8_t> &payload_out);

    [[nodiscard]] bool
    write_second_message(std::span<std::uint8_t const> payload,
                         std::vector<std::uint8_t> &out);

    // The responder sends with the SECOND key and receives with the first:
    // the exact reverse of the initiator.
    [[nodiscard]] bool export_transport_keys(symmetric_key &send_key,
                                             symmetric_key &receive_key) const;

    [[nodiscard]] bool is_complete() const;

private:
    noise_symmetric_state state_;
    x25519_secret_key static_secret_;
    x25519_public_key remote_ephemeral_;
    bool first_message_read_;
    bool complete_;
};

} // namespace hypercom::crypto
