#pragma once

#include <string_view>

#include "common/protocol/motd_message.hpp"
#include "server/db/database_handle.hpp"

namespace hypercom::server {

// Message du jour, modifiable a chaud par la CLI d'administration sans
// redemarrage ni recompilation. C'est le canal d'annonce du
// collaborateur.
class motd_repository {
public:
    explicit motd_repository(database_handle &database);

    // Renvoie false s'il n'y a aucun MOTD actif, ce qui est un etat normal.
    [[nodiscard]] bool find_active_motd(proto::motd_push &out);

    [[nodiscard]] bool publish_motd(std::string_view body);

    // Desactive l'annonce en cours sans en publier de nouvelle. Les anciennes
    // lignes restent en base : elles servent d'historique des annonces, et
    // rien n'y est nominatif.
    [[nodiscard]] bool clear_active_motd();

private:
    database_handle &database_;
};

} // namespace hypercom::server
