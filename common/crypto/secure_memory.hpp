#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace hypercom::crypto {

// Effacement qu'un optimiseur n'a pas le droit de supprimer. Un simple
// std::fill sur un tampon en fin de vie est regulierement elimine comme code
// mort : la cle resterait alors en memoire, puis dans le fichier d'echange.
void wipe_bytes(std::span<std::uint8_t> destination);

// Comparaison a temps constant. Toute comparaison portant sur un secret --
// cle, empreinte, jeton -- passe par ici. Un memcmp classique sort au premier
// octet different et transforme la duree en canal auxiliaire.
[[nodiscard]] bool compare_in_constant_time(std::span<std::uint8_t const> left,
                                            std::span<std::uint8_t const> right);

} // namespace hypercom::crypto
