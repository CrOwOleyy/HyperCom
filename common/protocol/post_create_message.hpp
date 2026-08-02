#pragma once

#include <cstdint>
#include <string>

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/content_records.hpp"

namespace hypercom::proto {

// Texte et liens uniquement en v1, aucun media n'est stocke. Un lien reste du
// texte brut ; c'est au client de decider s'il le rend cliquable.
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
