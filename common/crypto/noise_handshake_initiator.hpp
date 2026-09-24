#pragma once

#include "common/crypto/key_types.hpp"
#include "common/crypto/noise_symmetric_state.hpp"

#include <cstdint>
#include <span>
#include <vector>

namespace hypercom::crypto {

// Client side of the NK pattern:
//
//   <- s                (known in advance, pinned)
//   ...
//   -> e, es            write_first_message
//   <- e, ee            read_second_message
//
// The client knows the server's static key before opening the connection.
// This is what replaces the certificate chain: there is no authority to
// query, only a key to compare.
class noise_handshake_initiator {
public:
    explicit noise_handshake_initiator(
        x25519_public_key const &server_static_public);

    [[nodiscard]] bool
    write_first_message(std::span<std::uint8_t const> payload,
                        std::vector<std::uint8_t> &out);

    // Wipes the ephemeral key as soon as it's used for the last time, without
    // waiting for the object's destruction: this is what gives the handshake
    // its forward secrecy.
    [[nodiscard]] bool
    read_second_message(std::span<std::uint8_t const> input,
                        std::vector<std::uint8_t> &payload_out);

    // The initiator sends with the first key and receives with the second.
    [[nodiscard]] bool export_transport_keys(symmetric_key &send_key,
                                             symmetric_key &receive_key) const;

    [[nodiscard]] bool is_complete() const;

private:
    noise_symmetric_state state_;
    x25519_public_key remote_static_;
    x25519_public_key ephemeral_public_;
    x25519_secret_key ephemeral_secret_;
    bool first_message_sent_;
    bool complete_;
};

} // namespace hypercom::crypto
