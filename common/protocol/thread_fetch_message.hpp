#pragma once

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/content_records.hpp"

#include <cstdint>
#include <vector>

namespace hypercom::proto {

struct thread_fetch_request {
    std::uint64_t post_id = 0;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

// The post, then its comments flattened, already sorted in preorder
// traversal by the server. The client reconstructs the indentation using
// depth, without ever recursing on a structure that came from the
// network.
struct thread_response {
    post_record post;
    std::vector<comment_record> comments;
    std::uint8_t truncated = 0;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
