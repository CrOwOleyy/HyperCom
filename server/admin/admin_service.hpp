#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "server/admin/admin_context.hpp"
#include "server/admin/admin_listener.hpp"
#include "server/net/event_loop.hpp"
#include "server/net/unique_descriptor.hpp"

namespace hypercom::server {

// Une connexion d'administration : une commande, une reponse, puis fermeture.
//
// output est bufferise plutot qu'ecrit d'un bloc. Une liste de sessions peut
// depasser ce qu'une seule ecriture accepte, et bloquer la boucle d'evenements
// pour attendre la place reviendrait a suspendre tout le serveur le temps
// qu'un administrateur lise sa sortie.
struct admin_connection {
    unique_descriptor socket;
    std::string input;
    std::string output;
    bool response_ready = false;
};

struct admin_service {
    admin_listener listener;
    std::unordered_map<int, admin_connection> connections;
};

// Chemin vide = administration desactivee. Ce n'est pas une erreur : un
// serveur peut tourner sans, il perd juste la CLI.
[[nodiscard]] bool open_admin_service(admin_service &service,
                                      std::string const &path,
                                      event_loop &loop,
                                      std::string &error_out);

void accept_admin_connections(admin_service &service, event_loop &loop);

[[nodiscard]] bool owns_admin_descriptor(admin_service const &service,
                                         int descriptor);

// close_requests recueille les descripteurs de sessions CLIENT que la commande
// demande de fermer. L'appelant les applique ensuite : fermer pendant le
// parcours du registre invaliderait l'iterateur.
void service_admin_connection(admin_service &service, event_loop &loop,
                              int descriptor, admin_context &context,
                              std::vector<int> &close_requests);

void close_admin_connection(admin_service &service, event_loop &loop,
                            int descriptor);

} // namespace hypercom::server
