#pragma once

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/content_records.hpp"

#include <cstdint>
#include <vector>

namespace hypercom::proto {

struct post_list_request {
    std::uint64_t forum_id = 0;
    std::uint32_t offset = 0;
    std::uint16_t limit = DEFAULT_LIST_ITEMS;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

// In a list, the server truncates the body to MAX_POST_PREVIEW_LENGTH.
// Otherwise a single request could pull 200 x 16 KiB and overflow the
// frame.
struct post_list_response {
    std::vector<post_record> posts;
    std::uint32_t total_count = 0;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
