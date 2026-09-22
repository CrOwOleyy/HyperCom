#pragma once

#include <string>

#include "client/keystore/server_registry.hpp"
#include "client/ui/app_state.hpp"
#include "client/ui/server_slot.hpp"

namespace hypercom::client {

// Construit un slot depuis une entree du registre : resout son identite
// (derivee de la graine, ou chargee depuis un fichier importe) et prepare la
// connexion, sans encore l'ouvrir.
[[nodiscard]] bool build_slot(server_entry const &entry, app_state const &app,
                              std::unique_ptr<server_slot> &out,
                              std::string &error_out);

// Ouvre la connexion et authentifie, si ce n'est pas deja fait. Un serveur dont
// l'avertissement n'a pas ete acquitte n'est jamais contacte : c'est la seule
// garantie que l'utilisateur a vu ce que l'operateur pourra observer avant que
// quoi que ce soit ne parte.
[[nodiscard]] bool connect_slot(server_slot &slot, std::string &error_out);

// Ajoute un serveur depuis une ligne hypercom:// et le persiste dans le
// registre chiffre.
void add_server_from_invite(app_state &app, server_slot_list &slots);

// Acquitte l'avertissement de premiere connexion et le retient dans le
// registre, pour ne le montrer qu'une fois par serveur.
void acknowledge_slot_trust(app_state &app, server_slot_list &slots,
                            std::size_t index);

} // namespace hypercom::client
