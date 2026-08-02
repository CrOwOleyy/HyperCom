#pragma once

#include <string>
#include <vector>

#include "server/admin/admin_command.hpp"
#include "server/admin/admin_context.hpp"

namespace hypercom::server {

// `sessions` / `sessions close <descripteur>`
//
// close_requests recueille les descripteurs a fermer. La commande ne ferme
// rien elle-meme : detruire une connexion pendant qu'on parcourt le registre
// invaliderait l'iterateur. C'est la boucle d'evenements qui applique la
// fermeture, une fois la reponse construite.
//
// La liste ne montre ni pseudo ni adresse : juste des connexions anonymes,
// leur etat et leur duree. Voir le commentaire de list_sessions() pour la
// raison -- ce n'est pas un oubli, c'est la limite volontaire de la commande.
[[nodiscard]] std::string run_sessions_command(
    admin_context &context, admin_command const &command,
    std::vector<int> &close_requests);

} // namespace hypercom::server
