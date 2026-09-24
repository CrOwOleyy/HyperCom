#pragma once

#include "common/protocol/byte_reader.hpp"
#include "common/protocol/byte_writer.hpp"
#include "common/protocol/social_records.hpp"
#include "common/protocol/wire_key.hpp"

#include <array>
#include <vector>

namespace hypercom::proto {

// MySpace's "top 8": eight ordered slots.
//
// A fixed-size array rather than a list, with an empty slot denoted by a
// null key. Incidentally, that also means there's no cap to check on
// read.
struct top8_set_request {
    std::array<wire_public_key, TOP8_SLOT_COUNT> slots{};

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

// Target whose top 8 is being queried. Without this field, the message
// could only return one's own -- which would empty the concept of its
// social meaning: seeing others' top 8, not just managing one's own.
struct top8_get_request {
    wire_public_key target_pubkey{};

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

struct top8_response {
    std::array<wire_public_key, TOP8_SLOT_COUNT> slots{};
    std::vector<friend_record> details;

    void write_to(byte_writer &writer) const;

    [[nodiscard]] bool read_from(byte_reader &reader);
};

} // namespace hypercom::proto
