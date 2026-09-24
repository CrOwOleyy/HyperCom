#pragma once

#include "common/protocol/motd_message.hpp"
#include "server/db/database_handle.hpp"

#include <string_view>

namespace hypercom::server {

// Message of the day, editable live through the admin CLI without a
// restart or a recompile. This is the operator's announcement channel.
class motd_repository {
public:
    explicit motd_repository(database_handle &database);

    // Returns false when there's no active MOTD, which is a normal state.
    [[nodiscard]] bool find_active_motd(proto::motd_push &out);

    [[nodiscard]] bool publish_motd(std::string_view body);

    // Deactivates the current announcement without publishing a new one.
    // Old rows remain in the database: they serve as an announcement
    // history, and nothing in them identifies anyone.
    [[nodiscard]] bool clear_active_motd();

private:
    database_handle &database_;
};

} // namespace hypercom::server
