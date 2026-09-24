#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace hypercom::crypto {

// Wipe that an optimizer is not allowed to eliminate. A plain std::fill on
// a buffer at the end of its life is routinely stripped out as dead code:
// the key would then linger in memory, and then in the swap file.
void wipe_bytes(std::span<std::uint8_t> destination);

// Constant-time comparison. Any comparison involving a secret -- key,
// fingerprint, token -- goes through here. A plain memcmp bails out at the
// first differing byte and turns timing into a side channel.
[[nodiscard]] bool
compare_in_constant_time(std::span<std::uint8_t const> left,
                         std::span<std::uint8_t const> right);

} // namespace hypercom::crypto
