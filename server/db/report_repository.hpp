#pragma once

#include "common/protocol/wire_key.hpp"
#include "server/db/database_handle.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

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

// One repository per entity (rule O3). Records reports, never interprets
// them: it's an intake channel, not an arbiter.
class report_repository {
public:
    explicit report_repository(database_handle &database);

    [[nodiscard]] bool record_post_report(std::int64_t post_id,
                                          std::int64_t reporter_id,
                                          std::string_view reason);

    [[nodiscard]] bool
    record_account_report(proto::wire_public_key const &target_pubkey,
                          std::int64_t reporter_id, std::string_view reason);

    [[nodiscard]] bool list_reports(std::uint16_t limit,
                                    std::vector<report_row> &out);

    // Resolved: the admin has read it and acted (or decided there was
    // nothing to do).
    [[nodiscard]] bool clear_report(std::int64_t id);

private:
    database_handle &database_;
};

} // namespace hypercom::server
