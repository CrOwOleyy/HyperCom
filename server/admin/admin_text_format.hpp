#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace hypercom::server {

// Mise en forme du texte rendu par l'administration.
//
// Sortie alignee en colonnes plutot que du JSON : la CLI s'adresse d'abord a
// un humain devant un terminal, et reste lisible par awk ou cut si besoin.

[[nodiscard]] std::string format_duration(std::uint64_t seconds);

[[nodiscard]] std::string pad_right(std::string_view text, std::size_t width);

} // namespace hypercom::server
