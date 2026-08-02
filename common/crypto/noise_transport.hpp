#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include "common/crypto/key_types.hpp"
#include "common/crypto/noise_cipher_state.hpp"

namespace hypercom::crypto {

// Canal etabli, apres Split(). Chaque sens a sa cle et son compteur propres,
// ce qui evite toute collision de nonce entre client et serveur.
//
// Tout ce qui passe ensuite -- trames de protocole comprises -- traverse cet
// objet. Le protocole applicatif n'est jamais visible sur le fil.
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
