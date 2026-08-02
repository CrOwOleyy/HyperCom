#pragma once

#include <cstdint>
#include <vector>

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/forum_record.hpp"

namespace hypercom::proto {

// limit est une demande, pas un ordre : le serveur la rabat sur MAX_LIST_ITEMS.
// C'est au serveur de decider combien il envoie, jamais au client.
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
