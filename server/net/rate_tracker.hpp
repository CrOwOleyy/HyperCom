#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include "server/net/rate_limiter.hpp"

namespace hypercom::server {

struct rate_window {
    std::uint32_t counter = 0;
    std::uint64_t window_start = 0;
};

// Compteurs de debit partages entre connexions.
//
// Ils vivaient auparavant dans session_state, donc une par connexion. Le
// resultat etait une limite sans effet : ouvrir une seconde connexion remettait
// le compteur a zero, et une adresse autorisee a huit connexions obtenait huit
// fois le quota annonce.
//
// COMPROMIS ASSUME, a connaitre avant de toucher a ce fichier : limiter par
// adresse impose de garder une table indexee par adresse IP en memoire. Le
// projet evite les IP partout ailleurs, et c'est le seul endroit ou il n'y a
// pas d'alternative -- sans cet index, la limite par adresse ne peut pas
// exister. Trois proprietes la distinguent d'un journal :
//   - elle ne contient qu'un compteur et un debut de fenetre, aucune trace de
//     ce qui a ete fait ;
//   - elle est purgee des que la fenetre expire, donc elle ne remonte jamais
//     au-dela de la minute ecoulee ;
//   - elle n'est jamais ecrite sur disque, et disparait avec le processus.
// Le registre de connexions maintient deja une table adresse -> nombre pour le
// plafond par adresse : on n'ajoute donc pas une categorie de donnee nouvelle.
struct rate_tracker {
    std::unordered_map<std::string, rate_window> by_address;
    std::unordered_map<std::int64_t, rate_window> by_identity;
};

// Les trois arguments d'un controle de debit, regroupes pour ne pas les
// trainer un par un a travers les handlers.
struct rate_policy {
    rate_limiter const &per_address;
    rate_limiter const &per_identity;
    rate_tracker &tracker;
};

[[nodiscard]] bool allow_address_request(rate_policy &policy,
                                         std::string const &address,
                                         std::uint64_t now);

// user_id valant 0 designe une session non encore enregistree : seule la
// limite par adresse s'applique alors.
[[nodiscard]] bool allow_identity_request(rate_policy &policy,
                                          std::int64_t user_id,
                                          std::uint64_t now);

// Sans cette purge, les deux tables grossiraient indefiniment -- ce qui serait
// a la fois une fuite memoire et, pour la table d'adresses, un historique.
void forget_expired_windows(rate_tracker &tracker, std::uint64_t now);

} // namespace hypercom::server
