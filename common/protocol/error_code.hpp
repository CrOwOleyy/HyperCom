#pragma once

#include <cstdint>
#include <string_view>

namespace hypercom::proto {

// Codes d'erreur applicatifs, transportes par status_error.
//
// Ils restent grossiers a dessein. « utilisateur inconnu » et « signature
// invalide » renvoient tous deux authentication_failed : distinguer les deux
// reviendrait a dire a un attaquant quelles cles publiques existent.
enum class error_code : std::uint16_t {
    none = 0,
    malformed_frame = 1,
    unsupported_version = 2,
    not_authenticated = 3,
    already_authenticated = 4,
    authentication_failed = 5,
    handle_unavailable = 6,
    invalid_field = 7,
    not_found = 8,
    permission_denied = 9,
    rate_limited = 10,
    payload_too_large = 11,
    duplicate_entry = 12,
    internal_error = 13,
    not_implemented = 14,
};

[[nodiscard]] std::string_view describe_error_code(error_code code);

} // namespace hypercom::proto
