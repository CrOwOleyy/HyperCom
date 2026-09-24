#pragma once

#include <cstdint>
#include <string_view>

namespace hypercom::proto {

// Application-level error codes, carried by status_error.
//
// They stay coarse on purpose. "unknown user" and "invalid signature" both
// map to authentication_failed: distinguishing the two would amount to
// telling an attacker which public keys exist.
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
    account_banned = 15,
};

[[nodiscard]] std::string_view describe_error_code(error_code code);

} // namespace hypercom::proto
