#include "common/protocol/friend_message.hpp"

#include "common/protocol/record_list_codec.hpp"

namespace hypercom::proto {

void friend_add_request::write_to(byte_writer &writer) const
{
    writer.write_fixed_bytes(target_pubkey);
    writer.write_integer(static_cast<std::uint8_t>(status));
}

bool friend_add_request::read_from(byte_reader &reader)
{
    std::uint8_t raw_status = 0;
    if (!reader.read_fixed_bytes(target_pubkey)
        || !reader.read_integer(raw_status)) {
        return false;
    }
    if (raw_status > static_cast<std::uint8_t>(friendship_status::blocked)) {
        return false;
    }
    status = static_cast<friendship_status>(raw_status);
    return true;
}

void friend_list_response::write_to(byte_writer &writer) const
{
    write_record_list(writer, friends);
}

bool friend_list_response::read_from(byte_reader &reader)
{
    return read_record_list(reader, friends, MAX_FRIEND_ITEMS);
}

} // namespace hypercom::proto
