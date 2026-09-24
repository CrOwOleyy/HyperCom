#pragma once

#include "client/net/server_connection.hpp"
#include "common/crypto/key_types.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace hypercom::client {

// Where the identity used on a given server comes from.
//
// derived: deduced from the master seed, nothing more to store.
// imported: a key pair predating multi-server support, drawn at random
//   and therefore not reproducible from a seed. Its file keeps being used
//   as is, which avoids losing accounts created before this change.
enum class identity_source : std::uint8_t {
    derived = 0,
    imported = 1,
};

struct server_entry {
    std::string label;
    server_endpoint endpoint;
    crypto::x25519_public_key server_key{};
    identity_source source = identity_source::derived;
    std::string imported_identity_path;
    // False until the user has seen the warning explaining what this
    // server's operator will be able to observe. Persisted so it's shown
    // only once per server.
    bool trust_acknowledged = false;
};

// The list of servers you've joined, sealed on disk.
//
// It's encrypted because it reveals affiliations: knowing which servers
// someone connects to often says more than the content they post there.
class server_registry {
public:
    explicit server_registry(std::string path);

    [[nodiscard]] bool has_stored_registry() const;

    [[nodiscard]] bool load(std::string_view passphrase,
                            std::vector<server_entry> &out,
                            std::string &error_out);

    [[nodiscard]] bool save(std::string_view passphrase,
                            std::vector<server_entry> const &entries,
                            std::string &error_out);

private:
    std::string path_;
};

} // namespace hypercom::client
