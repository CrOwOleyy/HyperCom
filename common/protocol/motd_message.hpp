#pragma once

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"

#include <cstdint>
#include <string>

namespace hypercom::proto {

// Message of the day, pushed on connection. Editable live, no restart
// needed.
//
// revision saves the client from comparing strings to know whether it has
// already displayed this announcement.
struct motd_push {
    std::uint64_t revision = 0;
    std::string body;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
