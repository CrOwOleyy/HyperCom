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
    std::uint64_t created_at = 0;
    std::uint64_t last_seen = 0;
};

} // namespace hypercom::server
