#pragma once

#include <cstdint>
#include <vector>

#include "common/protocol/top8_message.hpp"
#include "server/db/database_handle.hpp"

namespace hypercom::server {

struct top8_entry {
    std::uint8_t slot = 0;
    std::int64_t friend_id = 0;
};

class top8_repository {
public:
    explicit top8_repository(database_handle &database);

    // Remplacement integral plutot que mise a jour par emplacement : le client
    // envoie l'etat complet des huit cases, ce qui evite toute divergence entre
    // ce qu'il affiche et ce qui est stocke.
    [[nodiscard]] bool replace_slots(std::int64_t user_id,
                                     std::vector<top8_entry> const &entries);

    [[nodiscard]] bool list_slots(std::int64_t user_id,
                                  proto::top8_response &out);

private:
    database_handle &database_;
};

} // namespace hypercom::server
