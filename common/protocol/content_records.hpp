#pragma once

#include <cstdint>
#include <string>

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/wire_key.hpp"

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

// Les commentaires voyagent a plat, pas en arbre : chacun porte son parent et
// sa profondeur, calculee cote serveur par la requete recursive. Le client
// n'a plus qu'a regrouper.
//
// Serialiser un arbre imbrique donnerait a l'emetteur le controle de la
// profondeur de recursion du parseur. Non merci.
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
