#pragma once

#include "common/protocol/message_type.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace hypercom::proto {

// Framing: [u32 body_size][u8 type][payload]
//
// body_size counts the type byte and the payload, but not the length field
// itself. A valid frame therefore satisfies 1 <= body_size <= MAX_BODY_SIZE.
// The classic trap is assuming body_size only covers the payload -- reread
// encode_frame if in doubt.
struct frame_header {
    std::uint32_t body_size;
    message_type type;
};

// Reads only the length field. The stream reader uses it to know how many
// bytes to wait for before reserving anything.
[[nodiscard]] bool peek_body_size(std::span<std::uint8_t const> input,
                                  std::uint32_t &out);

// Full header: bounded length and recognized type. An unknown type fails
// here, which keeps it from ever reaching a handler.
[[nodiscard]] bool decode_frame_header(std::span<std::uint8_t const> input,
                                       frame_header &out);

// Fails if the payload exceeds the cap, which points to a caller bug
// rather than hostile data.
[[nodiscard]] bool encode_frame(message_type type,
                                std::span<std::uint8_t const> payload,
                                std::vector<std::uint8_t> &out);

} // namespace hypercom::proto
