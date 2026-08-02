#pragma once

#include <cstdint>

#include "common/protocol/profile_set_message.hpp"
#include "common/protocol/social_records.hpp"
#include "server/db/database_handle.hpp"

namespace hypercom::server {

class profile_repository {
public:
    explicit profile_repository(database_handle &database);

    // Le serveur stocke theme_json sans jamais l'interpreter : il n'a pas a
    // comprendre l'apparence d'un profil, et un parseur JSON de plus serait
    // une surface d'attaque de plus pour rien.
    [[nodiscard]] bool replace_profile(std::int64_t user_id,
                                       proto::profile_set_request const &request);

    [[nodiscard]] bool find_by_pubkey(proto::wire_public_key const &pubkey,
                                      proto::profile_record &out);

private:
    database_handle &database_;
};

} // namespace hypercom::server
