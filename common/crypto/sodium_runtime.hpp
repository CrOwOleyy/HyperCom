#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace hypercom::crypto {

// Call once at the start of any process that touches crypto, BEFORE any
// other function in this namespace. sodium_init() picks implementations
// based on the CPU and seeds the random generator; skipping it leaves
// everything else undefined.
//
// Returns false if initialization fails. In that case the program must
// stop: there is no acceptable degraded mode.
[[nodiscard]] bool initialize_sodium();

// Cryptographic randomness. The project's only source -- neither
// std::random_device, rand(), nor mt19937 must appear in security code.
void fill_random_bytes(std::span<std::uint8_t> destination);

} // namespace hypercom::crypto
