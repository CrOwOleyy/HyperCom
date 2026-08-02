#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace hypercom::util {

// Conversion hexadecimale pour l'affichage et la configuration.
//
// AVERTISSEMENT : ces deux fonctions ne sont PAS a temps constant. Elles sont
// reservees aux donnees publiques -- cles publiques, empreintes, identifiants.
// Une cle privee ne passe jamais par ici : elle reste binaire, et son stockage
// sur disque est traite par common/crypto/keystore_envelope.
void encode_hex(std::span<std::uint8_t const> input, std::string &out);

// Accepte les deux casses, refuse tout caractere non hexadecimal et toute
// longueur impaire. Le vecteur de sortie n'est ecrit qu'en cas de succes.
[[nodiscard]] bool decode_hex(std::string_view input,
                              std::vector<std::uint8_t> &out);

} // namespace hypercom::util
