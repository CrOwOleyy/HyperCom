#pragma once

#include <cstdint>
#include <vector>

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/content_records.hpp"

namespace hypercom::proto {

struct thread_fetch_request {
    std::uint64_t post_id = 0;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

// Le post, puis ses commentaires a plat, deja tries en parcours prefixe par le
// serveur. Le client reconstitue l'indentation avec depth, sans jamais
// recurser sur une structure venue du reseau.
struct thread_response {
    post_record post;
    std::vector<comment_record> comments;
    std::uint8_t truncated = 0;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
