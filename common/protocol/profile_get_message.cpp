#include "common/protocol/profile_get_message.hpp"

namespace hypercom::proto {

void profile_get_request::write_to(byte_writer &writer) const
{
    writer.write_fixed_bytes(target_pubkey);
}

bool profile_get_request::read_from(byte_reader &reader)
{
    return reader.read_fixed_bytes(target_pubkey);
}

void profile_response::write_to(byte_writer &writer) const
{
    profile.write_to(writer);
}

bool profile_response::read_from(byte_reader &reader)
{
    return profile.read_from(reader);
}

} // namespace hypercom::proto
