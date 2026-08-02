#pragma once

#include <cstdint>
#include <string>

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/wire_key.hpp"

namespace hypercom::proto {

// Un forum tel qu'il circule sur le fil.
//
// Le fondateur est designe par sa cle publique et pas par un identifiant de
// base, pour que le client puisse verifier a qui il a affaire sans dependre
// d'une table de correspondance que le serveur controle.
struct forum_record {
    std::uint64_t id = 0;
    std::string name;
    std::string description;
    wire_public_key founder_pubkey{};
    std::string founder_handle;
    std::string theme_json;
    std::uint64_t created_at = 0;
    std::uint32_t post_count = 0;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
