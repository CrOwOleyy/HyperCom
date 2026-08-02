#pragma once

#include <cstdint>

namespace hypercom::util {

// Horodatage unique du projet : secondes depuis l'epoque UNIX, UTC.
// Toutes les dates stockees ou transmises utilisent cette unite, jamais une
// heure locale ni un format textuel.
[[nodiscard]] std::uint64_t get_unix_timestamp();

} // namespace hypercom::util
