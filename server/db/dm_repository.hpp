#pragma once

#include "common/protocol/dm_fetch_message.hpp"
#include "server/db/database_handle.hpp"

#include <cstdint>
#include <span>
#include <vector>

namespace hypercom::server {

// A blind mailbox.
//
// No method here can read a message's content: there is no key anywhere on
// this machine that would allow it. An administrator therefore can't
// change their mind -- there's nothing to change.
class dm_repository {
public:
    explicit dm_repository(database_handle &database);

    [[nodiscard]] bool insert_envelope(
        std::int64_t recipient_id, proto::wire_public_key const &sender_pubkey,
        std::span<std::uint8_t const> ciphertext, std::int64_t &out_id);

    [[nodiscard]] bool list_for_recipient(std::int64_t recipient_id,
                                          std::uint64_t since_id,
                                          std::uint16_t limit,
                                          proto::dm_list_response &out);

    // Acknowledging means deleting. What no longer exists on disk can't be
    // seized -- that's the project's DM retention policy.
    [[nodiscard]] bool
    delete_acknowledged(std::int64_t recipient_id,
                        std::vector<std::uint64_t> const &ids);

    [[nodiscard]] bool count_pending(std::int64_t recipient_id,
                                     std::uint32_t &out);

private:
    database_handle &database_;
};

} // namespace hypercom::server
