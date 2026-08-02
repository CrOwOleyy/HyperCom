#pragma once

#include <string>
#include <string_view>

#include "common/crypto/identity_keypair.hpp"

namespace hypercom::client {

// La cle privee sur le disque de l'utilisateur, et nulle part ailleurs.
//
//   passphrase --Argon2id--> cle --XChaCha20-Poly1305--> cle privee scellee
//
// Elle ne quitte JAMAIS la machine. C'est la raison pour laquelle le client est
// un executable natif et non une page web : un client web recoit son code du
// serveur a chaque chargement, donc un serveur compromis pourrait exfiltrer la
// cle sans que personne ne s'en apercoive.
class identity_store {
public:
    explicit identity_store(std::string path);

    [[nodiscard]] bool has_stored_identity() const;

    // Genere une identite et la scelle. Refuse d'ecraser un fichier existant :
    // sur ce projet, ecraser une cle equivaut a supprimer un compte
    // definitivement, sans aucun recours possible.
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
