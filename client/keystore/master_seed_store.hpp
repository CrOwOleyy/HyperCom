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
    //
    // The new seed is derived from 128 bits of fresh entropy, and
    // phrase_out receives the 12 words encoding that entropy. This is the
    // only moment the phrase exists: the seed cannot be turned back into
    // words later, so the caller MUST show it to the user now.
    [[nodiscard]] bool create_seed(std::string_view passphrase,
                                   crypto::ed25519_seed &out,
                                   std::string &phrase_out,
                                   std::string &error_out);

    // Rebuilds the seed from a recovery phrase and seals it under a (new)
    // passphrase. Same refusal to overwrite as create_seed. A phrase with a
    // typo is rejected by its checksum before anything is written.
    [[nodiscard]] bool restore_seed(std::string_view passphrase,
                                    std::string_view phrase,
                                    crypto::ed25519_seed &out,
                                    std::string &error_out);

    [[nodiscard]] bool unlock_seed(std::string_view passphrase,
                                   crypto::ed25519_seed &out,
                                   std::string &error_out);

private:
    [[nodiscard]] bool seal_to_disk(std::string_view passphrase,
                                    crypto::ed25519_seed const &seed,
                                    std::string &error_out);

    std::string path_;
};

} // namespace hypercom::client
