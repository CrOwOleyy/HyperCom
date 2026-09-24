#pragma once

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/dm_envelope_record.hpp"

#include <cstdint>
#include <vector>

namespace hypercom::proto {

// Inbox listing. since_id is 0 on the first call, then the identifier of
// the last envelope already received: the client catches up without the
// server having to remember its position.
struct dm_fetch_request {
    std::uint64_t since_id = 0;
    std::uint16_t limit = MAX_DM_BATCH_ITEMS;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

struct dm_list_response {
    std::vector<dm_envelope_record> envelopes;
    std::uint8_t has_more = 0;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
