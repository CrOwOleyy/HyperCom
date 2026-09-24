#pragma once

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/wire_key.hpp"

#include <cstdint>
#include <string>

namespace hypercom::proto {

struct post_record {
    std::uint64_t id = 0;
    std::uint64_t forum_id = 0;
    wire_public_key author_pubkey{};
    std::string author_handle;
    std::string title;
    std::string body;
    std::uint64_t created_at = 0;
    std::uint32_t comment_count = 0;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

// Comments travel flat, not as a tree: each one carries its parent and its
// depth, computed server-side by the recursive query. The client only has
// to regroup them.
//
// Serializing a nested tree would give the sender control over the parser's
// recursion depth. No thanks.
struct comment_record {
    std::uint64_t id = 0;
    std::uint64_t post_id = 0;
    std::uint64_t parent_comment_id = 0;
    wire_public_key author_pubkey{};
    std::string author_handle;
    std::string body;
    std::uint64_t created_at = 0;
    std::uint16_t depth = 0;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
