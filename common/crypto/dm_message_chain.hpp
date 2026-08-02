#pragma once

#include <cstdint>
#include <vector>

#include "common/crypto/key_types.hpp"

namespace hypercom::crypto {

// Nombre maximal de messages qu'on accepte de sauter d'un coup. Une enveloppe
// annoncant un compteur tres eloigne forcerait sinon des millions de
// derivations : c'est un deni de service a un octet.
constexpr std::uint32_t MAX_SKIPPED_MESSAGE_KEYS = 1000;

// Cliquet symetrique. La cle de chaine avance a chaque message et l'etat
// precedent est efface :
//
//   mk_i     = HKDF(ck_i, "message")
//   ck_{i+1} = HKDF(ck_i, "chain")      puis ck_i est detruite
//
// Consequence concrete : quelqu'un qui saisit la machine aujourd'hui et
// obtient ck_n ne peut PAS relire les messages 0..n-1. C'est la confidentialite
// persistante symetrique promise par BRIEF.md 6.
//
// Limite connue de la v1 : le cliquet Diffie-Hellman complet, qui protegerait
// aussi les messages FUTURS apres compromission, arrive en v2. Le format
// d'enveloppe est deja pret a l'accueillir.
class dm_message_chain {
public:
    explicit dm_message_chain(symmetric_key const &initial_chain_key);

    // Avance d'un cran et rend la cle du message courant.
    [[nodiscard]] bool derive_next_message_key(symmetric_key &out);

    // Rattrape les messages arrives dans le desordre. Les cles sautees sont
    // rendues a l'appelant, a lui de les conserver le temps necessaire.
    [[nodiscard]] bool advance_to_counter(std::uint32_t target,
                                          std::vector<symmetric_key> &skipped);

    [[nodiscard]] std::uint32_t get_counter() const;

    // Pour la persistance cote client uniquement. Ce qui sort d'ici doit etre
    // stocke chiffre, jamais en clair.
    [[nodiscard]] symmetric_key const &get_chain_key() const;

private:
    symmetric_key chain_key_;
    std::uint32_t counter_;
};

} // namespace hypercom::crypto
