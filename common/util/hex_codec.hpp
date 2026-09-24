#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace hypercom::util {

// Hex conversion for display and configuration.
//
// WARNING: these two functions are NOT constant-time. They're reserved
// for public data -- public keys, fingerprints, identifiers. A private
// key never goes through here: it stays binary, and its on-disk storage
// is handled by common/crypto/keystore_envelope.
void encode_hex(std::span<std::uint8_t const> input, std::string &out);

// Accepts both cases, rejects any non-hexadecimal character and any odd
// length. The output vector is written only on success.
[[nodiscard]] bool decode_hex(std::string_view input,
                              std::vector<std::uint8_t> &out);

} // namespace hypercom::util
