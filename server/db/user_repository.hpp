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

    [[nodiscard]] bool update_last_seen(std::int64_t id,
                                        std::uint64_t timestamp);

private:
    database_handle &database_;
};

} // namespace hypercom::server
