#include "common/crypto/secure_memory.hpp"

#include <sodium.h>

namespace hypercom::crypto {

void wipe_bytes(std::span<std::uint8_t> destination)
{
    if (destination.empty()) {
        return;
    }
    sodium_memzero(destination.data(), destination.size());
}

bool compare_in_constant_time(std::span<std::uint8_t const> left,
                              std::span<std::uint8_t const> right)
{
    // A length difference isn't a secret: two buffers of different sizes
    // can't be equal, and the size is already public.
    if (left.size() != right.size()) {
        return false;
    }
    if (left.empty()) {
        return true;
    }
    return sodium_memcmp(left.data(), right.data(), left.size()) == 0;
}

} // namespace hypercom::crypto
