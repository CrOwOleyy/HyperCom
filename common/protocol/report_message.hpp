#pragma once

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/wire_key.hpp"

#include <cstdint>
#include <string>

namespace hypercom::proto {

// Reporting is a legally required mechanism, not a moderation tool: the
// server records it, it judges nothing, and neither
// message can ever carry the content of a DM, which stays unreadable to
// it.
//
// The response is status_ok or status_error, just like for a deletion:
// there's nothing more to return than an acknowledgment.

struct report_post_request {
    std::uint64_t post_id = 0;
    std::string reason;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

// target_pubkey, not a handle: a handle can change meaning if the account
// gets recreated, a public key never does.
struct report_account_request {
    wire_public_key target_pubkey{};
    std::string reason;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
