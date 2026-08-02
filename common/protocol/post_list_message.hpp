#pragma once

#include <cstdint>
#include <vector>

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/content_records.hpp"

namespace hypercom::proto {

struct post_list_request {
    std::uint64_t forum_id = 0;
    std::uint32_t offset = 0;
    std::uint16_t limit = DEFAULT_LIST_ITEMS;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

// Dans une liste, le serveur tronque le corps a MAX_POST_PREVIEW_LENGTH.
// Sinon une seule requete tirerait 200 x 16 KiB et ferait deborder la trame.
struct post_list_response {
    std::vector<post_record> posts;
    std::uint32_t total_count = 0;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
