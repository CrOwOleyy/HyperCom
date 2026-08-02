#pragma once

#include <cstdint>
#include <span>

#include "common/crypto/key_types.hpp"

namespace hypercom::crypto {

// HKDF-SHA256 (RFC 5869) bati sur HMAC-SHA256 de libsodium.
//
// Sert deux clients : la fonction HKDF du framework Noise, et la derivation des
// cles de session des messages prives. Les deux ont exactement les memes
// besoins, il n'y a aucune raison d'en ecrire deux versions.

[[nodiscard]] bool extract_pseudo_random_key(
    std::span<std::uint8_t const> salt,
    std::span<std::uint8_t const> input_key_material, symmetric_key &out);

// Un seul bloc de sortie suffit : toutes nos cles font 32 octets, soit
// exactement la taille de sortie de SHA-256. block_index commence a 1.
[[nodiscard]] bool expand_key_block(symmetric_key const &pseudo_random_key,
                                    std::span<std::uint8_t const> info,
                                    std::uint8_t block_index,
                                    symmetric_key &out);

// HKDF(chaining_key, ikm) -> deux sorties, exactement la primitive decrite par
// la specification Noise sous le nom HKDF(..., num_outputs = 2).
[[nodiscard]] bool derive_key_pair(symmetric_key const &chaining_key,
                                   std::span<std::uint8_t const> input_key_material,
                                   symmetric_key &first, symmetric_key &second);

} // namespace hypercom::crypto
