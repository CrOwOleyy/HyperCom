#pragma once

#include <cstdint>
#include <string>

#include "common/protocol/wire_key.hpp"

namespace hypercom::server {

// Les autres depots remplissent directement les enregistrements de
// common/protocol : ce sont deja des structures de donnees pures, et ajouter
// une couche de types miroir ne ferait que doubler le code de recopie sans
// rien empecher. users est le seul cas sans equivalent protocolaire -- une
// ligne utilisateur n'est jamais servie telle quelle sur le fil, seul son
// profil l'est.
struct user_row {
    std::int64_t id = 0;
    proto::wire_public_key pubkey{};
    std::string handle;
    // Reserve a l'action sur signalement (BRIEF.md 13) : un compte banni ne
    // peut plus s'authentifier, ce qui laisse son contenu passe intact --
    // bannir et supprimer restent deux actions distinctes et deliberees.
    bool banned = false;
};

} // namespace hypercom::server
