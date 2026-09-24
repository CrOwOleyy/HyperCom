#pragma once

#include "common/protocol/prekey_fetch_message.hpp"
#include "server/db/database_handle.hpp"

#include <cstdint>

namespace hypercom::server {

// The server is a prekey directory, never an authority: it stores the
// signature alongside the key and serves it back as-is. It doesn't even
// check that it's valid on upload -- the recipient will verify it, and
// only they have good reason to trust it.
class prekey_repository {
public:
    explicit prekey_repository(database_handle &database);

    [[nodiscard]] bool replace_prekey(std::int64_t user_id,
                                      proto::wire_public_key const &prekey,
                                      proto::wire_signature const &signature);

    [[nodiscard]] bool
    find_bundle_by_pubkey(proto::wire_public_key const &owner_pubkey,
                          proto::prekey_bundle_response &out);

private:
    database_handle &database_;
};

} // namespace hypercom::server
