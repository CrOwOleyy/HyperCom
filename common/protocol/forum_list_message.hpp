#pragma once

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/forum_record.hpp"

#include <cstdint>
#include <vector>

namespace hypercom::proto {

// limit is a request, not an order: the server clamps it to MAX_LIST_ITEMS.
// It's the server's call how many it sends, never the client's.
struct forum_list_request {
    std::uint32_t offset = 0;
    std::uint16_t limit = DEFAULT_LIST_ITEMS;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

struct forum_list_response {
    std::vector<forum_record> forums;
    std::uint32_t total_count = 0;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
