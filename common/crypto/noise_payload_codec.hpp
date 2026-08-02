#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include "common/crypto/noise_symmetric_state.hpp"

namespace hypercom::crypto {

// EncryptAndHash / DecryptAndHash de la specification Noise.
//
// Le hachage de handshake en cours sert de donnee associee. C'est ce qui lie
// chaque message a tout ce qui a ete echange avant lui : modifier un octet
// d'un message anterieur invalide l'authentification de tous les suivants.
// Un attaquant ne peut donc pas retirer, reordonner ni substituer une etape du
// handshake sans que la suite echoue.

[[nodiscard]] bool encrypt_and_hash(noise_symmetric_state &state,
                                    std::span<std::uint8_t const> plaintext,
                                    std::vector<std::uint8_t> &out);

[[nodiscard]] bool decrypt_and_hash(noise_symmetric_state &state,
                                    std::span<std::uint8_t const> ciphertext,
                                    std::vector<std::uint8_t> &out);

} // namespace hypercom::crypto
