#pragma once

#include "client/ui/app_state.hpp"
#include "client/ui/server_slot.hpp"

namespace hypercom::client {

// Colonne des serveurs, a gauche : un bouton par serveur, plus le champ
// d'ajout par ligne d'invitation. C'est le seul moyen de rejoindre un serveur,
// il n'existe aucun annuaire.
void draw_server_bar(app_state &app, server_slot_list &slots, float width);

// Avertissement bloquant avant le premier echange avec un serveur inconnu.
//
// Renvoie true tant que l'avertissement occupe l'ecran : l'appelant ne doit
// alors rien dessiner d'autre, et surtout ne rien envoyer au serveur.
[[nodiscard]] bool draw_trust_warning(app_state &app, server_slot_list &slots,
                                      float scale);

} // namespace hypercom::client
