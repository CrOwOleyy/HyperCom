#pragma once

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"

#include <cstdint>

namespace hypercom::proto {

// The token is echoed back unchanged: it lets the client match response to
// request, and measure latency without a shared clock.
struct ping_request {
    std::uint64_t token = 0;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

struct ping_response {
    std::uint64_t token = 0;
    std::uint64_t server_time = 0;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
