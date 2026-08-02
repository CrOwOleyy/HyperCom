#pragma once

#include <vector>

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/social_records.hpp"
#include "common/protocol/wire_key.hpp"

namespace hypercom::proto {

// friend_list_request n'a pas de champ : on renvoie toujours la liste de la
// session authentifiee. Payload vide, donc pas de structure a declarer.
//
// status sert aussi a bloquer, mais le serveur ne fait que memoriser
// l'intention. Le filtrage reel se passe cote client.
struct friend_add_request {
    wire_public_key target_pubkey{};
    friendship_status status = friendship_status::requested;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

struct friend_list_response {
    std::vector<friend_record> friends;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
