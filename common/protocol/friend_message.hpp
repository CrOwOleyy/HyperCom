#pragma once

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/social_records.hpp"
#include "common/protocol/wire_key.hpp"

#include <vector>

namespace hypercom::proto {

// friend_list_request has no field: we always return the list for the
// authenticated session. Empty payload, so no structure to declare.
//
// status also serves to block, but the server only records the intent.
// The actual filtering happens client-side.
struct friend_add_request {
    wire_public_key target_pubkey{};
    friendship_status status = friendship_status::requested;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

struct friend_list_response {
    std::vector<friend_record> friends;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
