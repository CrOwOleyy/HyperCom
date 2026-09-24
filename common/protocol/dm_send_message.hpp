#pragma once

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/wire_key.hpp"

#include <cstdint>
#include <vector>

namespace hypercom::proto {

// Deposit of an envelope into the recipient's mailbox.
//
// The server cannot verify that ciphertext is well-formed -- if it could,
// that would mean it could read it. It just stores it.
struct dm_send_request {
    wire_public_key recipient_pubkey{};
    std::vector<std::uint8_t> ciphertext;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
