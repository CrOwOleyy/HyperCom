#pragma once

#include "server/net/unique_descriptor.hpp"

#include <cstdint>
#include <span>
#include <vector>

namespace hypercom::server {

// Byte-level plumbing for a connection: non-blocking reads, write queue. It
// knows nothing about Noise or the protocol -- that's intentional, it keeps
// it testable on its own and keeps the encryption in a single place.
class connection_socket {
public:
    explicit connection_socket(int descriptor);

    // Appends to destination everything that's available. Returns false on
    // end of stream or a fatal error: the connection must then be closed.
    [[nodiscard]] bool read_available(std::vector<std::uint8_t> &destination);

    // Writes what it can without blocking. has_remaining indicates whether
    // bytes are still pending, in which case the loop must watch for
    // EPOLLOUT.
    [[nodiscard]] bool flush_pending_writes(bool &has_remaining);

    void queue_bytes(std::span<std::uint8_t const> data);

    [[nodiscard]] int get_descriptor() const;

private:
    unique_descriptor descriptor_;
    std::vector<std::uint8_t> pending_output_;
};

} // namespace hypercom::server
