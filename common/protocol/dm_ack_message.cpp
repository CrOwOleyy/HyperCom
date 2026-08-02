#include "common/protocol/dm_ack_message.hpp"

#include "common/protocol/protocol_limits.hpp"

namespace hypercom::proto {

void dm_ack_request::write_to(byte_writer &writer) const
{
    writer.write_integer(static_cast<std::uint16_t>(envelope_ids.size()));
    for (std::uint64_t const identifier : envelope_ids) {
        writer.write_integer(identifier);
    }
}

bool dm_ack_request::read_from(byte_reader &reader)
{
    std::uint16_t announced_count = 0;
    if (!reader.read_integer(announced_count)) {
        return false;
    }
    if (announced_count > MAX_DM_BATCH_ITEMS) {
        return false;
    }
    std::vector<std::uint64_t> decoded;
    for (std::uint16_t index = 0; index < announced_count; ++index) {
        std::uint64_t identifier = 0;
        if (!reader.read_integer(identifier)) {
            return false;
        }
        decoded.push_back(identifier);
    }
    envelope_ids = std::move(decoded);
    return true;
}

} // namespace hypercom::proto
