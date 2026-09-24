#pragma once

#include "common/protocol/profile_set_message.hpp"
#include "common/protocol/social_records.hpp"
#include "server/db/database_handle.hpp"

#include <cstdint>

namespace hypercom::server {

class profile_repository {
public:
    explicit profile_repository(database_handle &database);

    // The server stores theme_json without ever interpreting it: it has no
    // need to understand a profile's appearance, and one more JSON parser
    // would be one more attack surface for nothing.
    [[nodiscard]] bool
    replace_profile(std::int64_t user_id,
                    proto::profile_set_request const &request);

    [[nodiscard]] bool find_by_pubkey(proto::wire_public_key const &pubkey,
                                      proto::profile_record &out);

private:
    database_handle &database_;
};

} // namespace hypercom::server
