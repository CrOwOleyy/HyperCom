#pragma once

#include "common/crypto/key_types.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace hypercom::crypto {

constexpr std::uint8_t DM_ENVELOPE_VERSION = 1;

// [u8 version][32 sender identity][32 sender ephemeral][u32 counter]
constexpr std::size_t DM_ENVELOPE_HEADER_SIZE =
    1 + ED25519_PUBLIC_KEY_SIZE + X25519_PUBLIC_KEY_SIZE + 4;

// What the server stores without being able to open it.
//
// The header travels in the clear -- it has to, the recipient needs it to
// derive the key -- but it is fully authenticated as associated data.
// Changing a single one of its bytes, including the counter, makes
// decryption fail. A server can therefore neither replay a message under a
// different counter nor spoof the sender.
struct dm_envelope_header {
    std::uint8_t version = DM_ENVELOPE_VERSION;
    ed25519_public_key sender_identity{};
    x25519_public_key sender_ephemeral{};
    std::uint32_t counter = 0;
};

// The recipient reads the header first: it indicates which key to derive.
// The read is bounds-checked and assumes nothing about the rest.
[[nodiscard]] bool
parse_dm_envelope_header(std::span<std::uint8_t const> envelope,
                         dm_envelope_header &out);

[[nodiscard]] bool seal_dm_envelope(dm_envelope_header const &header,
                                    symmetric_key const &message_key,
                                    std::span<std::uint8_t const> plaintext,
                                    std::vector<std::uint8_t> &out);

[[nodiscard]] bool open_dm_envelope(std::span<std::uint8_t const> envelope,
                                    symmetric_key const &message_key,
                                    std::vector<std::uint8_t> &plaintext_out);

} // namespace hypercom::crypto
