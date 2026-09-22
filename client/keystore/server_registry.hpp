#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "client/net/server_connection.hpp"
#include "common/crypto/key_types.hpp"

namespace hypercom::client {

// D'ou vient l'identite utilisee sur un serveur donne.
//
// derived : deduite de la graine maitresse, rien de plus a stocker.
// imported : une paire de cles anterieure au multi-serveurs, tiree au hasard et
//   donc non reproductible depuis une graine. Son fichier reste utilise tel
//   quel, ce qui evite de perdre les comptes crees avant ce changement.
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
    // Faux tant que l'utilisateur n'a pas vu l'avertissement expliquant ce que
    // l'operateur de ce serveur pourra observer. Persiste pour ne l'afficher
    // qu'une fois par serveur.
    bool trust_acknowledged = false;
};

// La liste des serveurs frequentes, scellee sur le disque.
//
// Elle est chiffree parce qu'elle revele des appartenances : savoir sur quels
// serveurs quelqu'un se connecte en dit souvent plus long que le contenu de ce
// qu'il y publie.
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
