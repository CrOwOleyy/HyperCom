#pragma once

#include <memory>
#include <vector>

#include "client/cli/cli_context.hpp"
#include "client/keystore/server_registry.hpp"
#include "client/net/client_session.hpp"
#include "client/net/server_connection.hpp"
#include "client/ui/ui_state.hpp"
#include "common/crypto/identity_keypair.hpp"

namespace hypercom::client {

// Tout ce qui appartient a UN serveur : sa connexion, son identite propre, sa
// session, et l'etat affichable de son contenu.
//
// L'identite est differente sur chaque serveur (voir server_identity.hpp), donc
// rien ici n'est partageable entre slots -- c'est justement le but.
struct server_slot {
    server_entry entry;
    crypto::identity_keypair identity;
    std::unique_ptr<server_connection> connection;
    std::unique_ptr<client_session> session;
    ui_state view;
};

// Les slots se stockent en unique_ptr, jamais en valeur dans un vecteur.
//
// client_session garde server_connection et identity_keypair par REFERENCE : si
// le conteneur reallouait, ces references pointeraient dans le vide. Le
// unique_ptr fige l'adresse du slot entier, identite comprise.
using server_slot_list = std::vector<std::unique_ptr<server_slot>>;

// Construit le contexte attendu par toutes les fonctions de dessin et d'action
// existantes. Il ne contient que des references : le fabriquer a chaque image
// ne coute rien, et evite d'avoir a toucher les quarante-sept signatures qui
// le prennent en parametre.
[[nodiscard]] inline cli_context make_context(server_slot &slot)
{
    return cli_context{*slot.connection, slot.identity, *slot.session};
}

} // namespace hypercom::client
