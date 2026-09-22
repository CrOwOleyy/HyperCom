#pragma once

#include <string>
#include <string_view>

#include "common/crypto/key_types.hpp"

namespace hypercom::client {

// La graine maitresse, scellee sur le disque par la passphrase.
//
// Elle est l'unique secret a sauvegarder : toutes les identites de tous les
// serveurs en derivent (voir server_identity.hpp). La perdre revient a perdre
// tous ses comptes d'un coup -- contrepartie assumee de n'avoir qu'une seule
// chose a mettre a l'abri plutot qu'une par serveur.
class master_seed_store {
public:
    explicit master_seed_store(std::string path);

    [[nodiscard]] bool has_stored_seed() const;

    // Refuse d'ecraser une graine existante : l'ecraser reviendrait a perdre
    // toutes les identites qui en derivent, sans aucun recours.
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
