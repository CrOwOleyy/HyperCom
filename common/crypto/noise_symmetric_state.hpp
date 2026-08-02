#pragma once

#include <cstdint>
#include <span>
#include <string_view>

#include "common/crypto/key_types.hpp"
#include "common/crypto/noise_cipher_state.hpp"

namespace hypercom::crypto {

// SymmetricState de la specification Noise.
//
// C'est une structure a champs publics manipulee par des fonctions libres,
// plutot qu'une classe. La regle O3 plafonne a cinq methodes publiques, et
// l'etat symetrique en demande six ; le decoupage en donnees d'un cote,
// operations de l'autre respecte la norme sans decouper artificiellement un
// concept qui n'a qu'un seul sens.
struct noise_symmetric_state {
    symmetric_key chaining_key{};
    symmetric_key handshake_hash{};
    noise_cipher_state cipher;
};

void initialize_symmetric_state(std::string_view protocol_name,
                                noise_symmetric_state &out);

void mix_hash(noise_symmetric_state &state,
              std::span<std::uint8_t const> data);

[[nodiscard]] bool mix_key(noise_symmetric_state &state,
                           std::span<std::uint8_t const> input_key_material);

// Split() : deux cles de transport independantes, une par sens. L'initiateur
// emet avec la premiere et recoit avec la seconde, le repondeur fait l'inverse.
[[nodiscard]] bool split_transport_keys(noise_symmetric_state const &state,
                                        symmetric_key &first,
                                        symmetric_key &second);

} // namespace hypercom::crypto
