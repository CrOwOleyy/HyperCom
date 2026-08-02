#pragma once

#include <cstdint>
#include <string>

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/content_records.hpp"

namespace hypercom::proto {

// parent_comment_id valant 0 designe une reponse directe au post.
// La profondeur reelle est calculee et plafonnee par le serveur : le client ne
// choisit pas ou il se greffe dans l'arbre au-dela de son parent immediat.
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
