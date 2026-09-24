#pragma once

#include "common/crypto/identity_keypair.hpp"

#include <string>
#include <string_view>

namespace hypercom::client {

// The private key on the user's disk, and nowhere else.
//
//   passphrase --Argon2id--> key --XChaCha20-Poly1305--> sealed private key
//
// It NEVER leaves the machine. That's why the client is a native
// executable rather than a web page: a web client gets its code from the
// server on every load, so a compromised server could exfiltrate the key
// without anyone noticing.
class identity_store {
public:
    explicit identity_store(std::string path);

    [[nodiscard]] bool has_stored_identity() const;

    // Generates an identity and seals it. Refuses to overwrite an
    // existing file: on this project, overwriting a key is equivalent to
    // deleting an account permanently, with no possible recovery.
    [[nodiscard]] bool create_identity(std::string_view passphrase,
                                       crypto::identity_keypair &out,
                                       std::string &error_out);

    [[nodiscard]] bool unlock_identity(std::string_view passphrase,
                                       crypto::identity_keypair &out,
                                       std::string &error_out);

    [[nodiscard]] std::string const &get_path() const;

private:
    std::string path_;
};

} // namespace hypercom::client
