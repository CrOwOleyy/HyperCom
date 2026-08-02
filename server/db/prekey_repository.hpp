#pragma once

#include <cstdint>

#include "common/protocol/prekey_fetch_message.hpp"
#include "server/db/database_handle.hpp"

namespace hypercom::server {

// Le serveur est un annuaire de prekeys, jamais une autorite : il stocke la
// signature avec la cle et la ressert telle quelle. Il ne verifie meme pas
// qu'elle est valide au depot -- c'est le destinataire qui verifiera, et lui
// seul a de bonnes raisons d'y croire.
class prekey_repository {
public:
    explicit prekey_repository(database_handle &database);

    [[nodiscard]] bool replace_prekey(std::int64_t user_id,
                                      proto::wire_public_key const &prekey,
                                      proto::wire_signature const &signature);

    [[nodiscard]] bool find_bundle_by_pubkey(
        proto::wire_public_key const &owner_pubkey,
        proto::prekey_bundle_response &out);

private:
    database_handle &database_;
};

} // namespace hypercom::server
