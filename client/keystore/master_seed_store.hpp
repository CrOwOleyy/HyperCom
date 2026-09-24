#pragma once

#include "common/crypto/key_types.hpp"

#include <string>
#include <string_view>

namespace hypercom::client {

// The master seed, sealed on disk by the passphrase.
//
// It's the one secret worth backing up: every identity on every server
// derives from it (see server_identity.hpp). Losing it means losing all
// your accounts at once -- a deliberate tradeoff for having only one
// thing to keep safe instead of one per server.
class master_seed_store {
public:
    explicit master_seed_store(std::string path);

    [[nodiscard]] bool has_stored_seed() const;

    // Refuses to overwrite an existing seed: overwriting it would mean
    // losing every identity derived from it, with no recourse.
    [[nodiscard]] bool create_seed(std::string_view passphrase,
                                   crypto::ed25519_seed &out,
                                   std::string &error_out);

    [[nodiscard]] bool unlock_seed(std::string_view passphrase,
                                   crypto::ed25519_seed &out,
                                   std::string &error_out);

private:
    std::string path_;
};

} // namespace hypercom::client
