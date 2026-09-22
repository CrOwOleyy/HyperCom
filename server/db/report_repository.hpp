#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "common/protocol/wire_key.hpp"
#include "server/db/database_handle.hpp"

namespace hypercom::server {

struct report_row {
    std::int64_t id = 0;
    std::string kind;
    std::int64_t post_id = 0;
    proto::wire_public_key target_pubkey{};
    std::int64_t reporter_id = 0;
    std::string reason;
    std::int64_t created_at = 0;
};

// Un depot par entite (regle O3). Enregistre des signalements, ne les
// interprete jamais : c'est un canal de reception, pas un arbitre (BRIEF.md 13).
class report_repository {
public:
    explicit report_repository(database_handle &database);

    [[nodiscard]] bool record_post_report(std::int64_t post_id,
                                          std::int64_t reporter_id,
                                          std::string_view reason);

    [[nodiscard]] bool record_account_report(
        proto::wire_public_key const &target_pubkey, std::int64_t reporter_id,
        std::string_view reason);

    [[nodiscard]] bool list_reports(std::uint16_t limit,
                                    std::vector<report_row> &out);

    // Traite : l'admin l'a lu et agi (ou a juge qu'il n'y avait rien a faire).
    [[nodiscard]] bool clear_report(std::int64_t id);

private:
    database_handle &database_;
};

} // namespace hypercom::server
