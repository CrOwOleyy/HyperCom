#pragma once

#include "common/crypto/key_types.hpp"
#include "common/crypto/noise_cipher_state.hpp"

#include <cstdint>
#include <span>
#include <string_view>

namespace hypercom::crypto {

// SymmetricState from the Noise specification.
//
// This is a struct with public fields manipulated by free functions,
// rather than a class. The O3 rule caps public methods at five, and the
// symmetric state needs six; splitting data on one side and operations on
// the other satisfies the standard without artificially cutting apart a
// concept that has only one meaning.
struct noise_symmetric_state {
    symmetric_key chaining_key{};
    symmetric_key handshake_hash{};
    noise_cipher_state cipher;
};

void initialize_symmetric_state(std::string_view protocol_name,
                                noise_symmetric_state &out);

void mix_hash(noise_symmetric_state &state, std::span<std::uint8_t const> data);

[[nodiscard]] bool mix_key(noise_symmetric_state &state,
                           std::span<std::uint8_t const> input_key_material);

// Split(): two independent transport keys, one per direction. The
// initiator sends with the first and receives with the second, the
// responder does the opposite.
[[nodiscard]] bool split_transport_keys(noise_symmetric_state const &state,
                                        symmetric_key &first,
                                        symmetric_key &second);

} // namespace hypercom::crypto
