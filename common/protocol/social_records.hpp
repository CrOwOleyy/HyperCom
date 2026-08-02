#pragma once

#include <cstdint>
#include <string>

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/wire_key.hpp"

namespace hypercom::proto {

// banner_reference designe un blob par son hash, jamais par une URL serveur.
// En v1 le champ circule et se stocke mais rien n'est heberge -- c'est ce qui
// permettra de brancher le P2P en v2 sans migration ni changement de format.
struct profile_record {
    wire_public_key pubkey{};
    std::string handle;
    std::string display_name;
    std::string bio;
    std::string theme_json;
    std::string banner_reference;
    std::uint64_t created_at = 0;
    std::uint64_t last_seen = 0;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

enum class friendship_status : std::uint8_t {
    requested = 0,
    accepted = 1,
    blocked = 2,
};

struct friend_record {
    wire_public_key pubkey{};
    std::string handle;
    std::string display_name;
    friendship_status status = friendship_status::requested;
    std::uint64_t created_at = 0;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
