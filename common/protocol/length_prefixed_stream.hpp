#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace hypercom::proto {

// Message extraction over a TCP stream, shared by the client and the server.
//
// TCP is a byte stream: a single read can yield half a message, or three
// messages stuck together. This function is the only place in the project
// that handles this splitting, and it applies the brief's rule: the
// announced length is compared against the cap BEFORE any memory
// reservation.
//
// Returns true when a complete message has been detached from buffer.
// Returns false with malformed = true if the announced length violates the
// cap -- the connection must then be closed, never resumed: a
// desynchronized stream can't be recovered.
[[nodiscard]] bool extract_length_prefixed_message(
    std::vector<std::uint8_t> &buffer, std::size_t maximum_size,
    std::vector<std::uint8_t> &out, bool &malformed);

void append_length_prefixed_message(std::span<std::uint8_t const> payload,
                                    std::vector<std::uint8_t> &out);

} // namespace hypercom::proto
