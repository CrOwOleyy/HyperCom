#pragma once

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/content_records.hpp"

#include <cstdint>
#include <string>

namespace hypercom::proto {

// A parent_comment_id of 0 designates a direct reply to the post.
// The actual depth is computed and capped by the server: the client doesn't
// choose where it grafts onto the tree beyond its immediate parent.
struct comment_create_request {
    std::uint64_t post_id = 0;
    std::uint64_t parent_comment_id = 0;
    std::string body;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

struct comment_info_response {
    comment_record comment;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
