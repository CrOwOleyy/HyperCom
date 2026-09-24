#pragma once

#include "common/crypto/key_types.hpp"
#include "common/crypto/noise_cipher_state.hpp"

#include <cstdint>
#include <span>
#include <vector>

namespace hypercom::crypto {

// Established channel, after Split(). Each direction has its own key and
// counter, which avoids any nonce collision between client and server.
//
// Everything that passes through afterward -- including protocol frames --
// goes through this object. The application protocol is never visible on
// the wire.
class noise_transport {
public:
    noise_transport(symmetric_key const &send_key,
                    symmetric_key const &receive_key);

    [[nodiscard]] bool encrypt_message(std::span<std::uint8_t const> plaintext,
                                       std::vector<std::uint8_t> &out);

    [[nodiscard]] bool decrypt_message(std::span<std::uint8_t const> ciphertext,
                                       std::vector<std::uint8_t> &out);

private:
    noise_cipher_state send_state_;
    noise_cipher_state receive_state_;
};

} // namespace hypercom::crypto
