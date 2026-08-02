#pragma once

#include <cstdint>

#include "server/db/database_handle.hpp"

namespace hypercom::server {

// Compteurs affiches par la commande d'administration `stats`.
//
// Que des agregats : aucun de ces chiffres ne permet de remonter a une
// personne. C'est cohérent avec le reste -- l'administration sert a exploiter
// le serveur, pas a observer ses utilisateurs.
struct server_counts {
    std::int64_t users = 0;
    std::int64_t forums = 0;
    std::int64_t posts = 0;
    std::int64_t comments = 0;
    std::int64_t pending_envelopes = 0;
};

class stats_repository {
public:
    explicit stats_repository(database_handle &database);

    [[nodiscard]] bool collect_counts(server_counts &out);

private:
    database_handle &database_;
};

} // namespace hypercom::server
