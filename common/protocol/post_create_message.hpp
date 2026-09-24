#pragma once

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/content_records.hpp"

#include <cstdint>
#include <string>

namespace hypercom::proto {

// Text and links only in v1, no media is stored. A link stays plain text;
// it's up to the client to decide whether to render it clickable.
struct post_create_request {
    std::uint64_t forum_id = 0;
    std::string title;
    std::string body;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

struct post_info_response {
    post_record post;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
