#pragma once

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"

#include <cstdint>
#include <vector>

namespace hypercom::proto {

// Acknowledgment: the server can then delete the referenced envelopes.
//
// This is the DM retention mechanism. An acknowledged envelope is erased,
// not archived -- what no longer exists on disk cannot be seized.
struct dm_ack_request {
    std::vector<std::uint64_t> envelope_ids;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
