#pragma once

#include <array>
#include <vector>

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/social_records.hpp"
#include "common/protocol/wire_key.hpp"

namespace hypercom::proto {

// Le « top 8 » de MySpace : huit emplacements ordonnes.
//
// Tableau de taille fixe plutot que liste, un emplacement vide se notant par
// une cle nulle. Accessoirement, il n'y a alors aucun plafond a verifier a la
// lecture.
struct top8_set_request {
    std::array<wire_public_key, TOP8_SLOT_COUNT> slots{};

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

// Cible dont on consulte le top 8. Sans ce champ, le message ne peut
// renvoyer que le sien propre -- ce qui viderait le concept de son sens
// social : voir le top 8 des autres, pas seulement geree le sien.
struct top8_get_request {
    wire_public_key target_pubkey{};

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

struct top8_response {
    std::array<wire_public_key, TOP8_SLOT_COUNT> slots{};
    std::vector<friend_record> details;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
