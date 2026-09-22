#pragma once

#include <cstddef>
#include <string>

#include "client/ui/i18n.hpp"

namespace hypercom::client {

// Ce qui appartient a l'application entiere, et non a un serveur en particulier.
//
// Tout le reste vit dans le ui_state de chaque slot : forums, fils, amis,
// messages, tampons de saisie. Le partage se fait ici, et seulement ici.
struct app_state {
    language current_lang = language::french;
    std::size_t active_slot = 0;
    // Leve une seule fois apres une creation de compte : l'accueil est une
    // sequence de fenetre, pas une sequence de serveur.
    bool intro_requested = false;
    // Statut des actions applicatives (ajout de serveur, bascule). Les erreurs
    // propres a un serveur restent dans le ui_state de son slot.
    std::string status_message;
    bool status_is_error = false;
    // Saisie de la ligne d'invitation, dans la barre laterale.
    char invite_input[160] = {};
    // Passphrase de la session, gardee en memoire pour pouvoir reecrire le
    // registre quand on ajoute un serveur ou qu'on acquitte un avertissement.
    // Elle ne quitte jamais ce processus et n'est jamais ecrite sur disque.
    std::string passphrase;
    std::string registry_path;
    std::string master_seed_path;
};

} // namespace hypercom::client
