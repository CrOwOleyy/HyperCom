#pragma once

#include <string>
#include <string_view>

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/forum_record.hpp"

namespace hypercom::proto {

// N'importe qui cree un forum et en devient fondateur. Le serveur ne verifie
// que l'unicite et la forme du nom -- il n'y a pas de liste de noms interdits,
// et il n'en est pas prevu.
struct forum_create_request {
    std::string name;
    std::string description;
    std::string theme_json;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

struct forum_info_response {
    forum_record forum;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

// Meme logique que validate_handle : ASCII restreint, pour que deux forums ne
// puissent pas porter des noms visuellement identiques.
[[nodiscard]] bool validate_forum_name(std::string_view name);

} // namespace hypercom::proto
