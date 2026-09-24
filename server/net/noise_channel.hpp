#pragma once

#include "common/crypto/noise_handshake_responder.hpp"
#include "common/crypto/noise_transport.hpp"

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace hypercom::server {

// Server side of the encrypted channel. Everything that crosses the
// connection passes through here: the application protocol is never
// visible on the wire, not even the frame type.
class noise_channel {
public:
    noise_channel(crypto::x25519_public_key const &static_public,
                  crypto::x25519_secret_key const &static_secret);

    // Consumes the client's first message and produces the reply. A failure
    // means an invalid handshake: the connection closes without any other
    // response, and especially without an error message that would
    // distinguish the causes.
    [[nodiscard]] bool
    accept_handshake_message(std::span<std::uint8_t const> input,
                             std::vector<std::uint8_t> &reply);

    [[nodiscard]] bool is_established() const;

    [[nodiscard]] bool open_message(std::span<std::uint8_t const> ciphertext,
                                    std::vector<std::uint8_t> &plaintext);

    [[nodiscard]] bool seal_message(std::span<std::uint8_t const> plaintext,
                                    std::vector<std::uint8_t> &ciphertext);

private:
    crypto::noise_handshake_responder handshake_;
    std::optional<crypto::noise_transport> transport_;
};

} // namespace hypercom::server
