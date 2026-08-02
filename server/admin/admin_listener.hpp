#pragma once

#include <string>

#include "server/net/unique_descriptor.hpp"

namespace hypercom::server {

// Socket d'ecoute de l'administration, en AF_UNIX.
//
// AF_UNIX et rien d'autre : il n'existe aucun chemin de code capable de
// l'exposer au reseau. C'est la propriete qui compte ici -- l'administration
// n'a pas d'authentification propre, elle s'appuie entierement sur les
// permissions du fichier de socket. Ouvrir ca sur un port TCP donnerait le
// controle du serveur au premier venu.
//
// Le fichier est cree en 0600 : seul le compte qui fait tourner le serveur peut
// s'y connecter.
class admin_listener {
public:
    admin_listener();

    // Supprime le fichier de socket a l'arret. Sans ca, un redemarrage
    // echouerait sur un fichier residuel, et le socket resterait visible dans
    // le systeme de fichiers alors que plus personne n'ecoute derriere.
    ~admin_listener();

    [[nodiscard]] bool open_listener(std::string const &path,
                                     std::string &error_out);

    // Renvoie -1 quand il n'y a plus rien a accepter.
    [[nodiscard]] int accept_connection() const;

    [[nodiscard]] int get_descriptor() const;

private:
    unique_descriptor descriptor_;
    std::string path_;
};

} // namespace hypercom::server
