#pragma once

#include <cstdint>
#include <string_view>

#include "server/db/database_handle.hpp"
#include "server/db/user_row.hpp"

namespace hypercom::server {

// Un depot par entite plutot qu'une classe database fourre-tout. C'est ce que
// la regle O3 impose en pratique, et ca tombe bien : on sait toujours ou
// chercher une requete.
class user_repository {
public:
    explicit user_repository(database_handle &database);

    [[nodiscard]] bool find_by_pubkey(proto::wire_public_key const &pubkey,
                                      user_row &out);

    [[nodiscard]] bool find_by_id(std::int64_t id, user_row &out);

    [[nodiscard]] bool create_user(proto::wire_public_key const &pubkey,
                                   std::string_view handle,
                                   std::int64_t &out_id);

    // Reserve a l'action sur signalement (BRIEF.md 13). Revoque
    // l'authentification, ne touche a aucun contenu deja publie.
    [[nodiscard]] bool set_banned(std::int64_t user_id, bool banned);

    // Il n'existe deliberement PAS de update_last_seen : enregistrer la
    // derniere connexion revient a tenir un journal de presence, et il etait
    // meme lisible par tout le monde via profile_get. Voir la migration
    // 0003_remove_last_seen.sql.

private:
    database_handle &database_;
};

} // namespace hypercom::server
