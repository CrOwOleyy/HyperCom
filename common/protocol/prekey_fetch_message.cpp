#include "common/protocol/prekey_fetch_message.hpp"

namespace hypercom::proto {

void prekey_fetch_request::write_to(byte_writer &writer) const
{
    writer.write_fixed_bytes(target_pubkey);
}

bool prekey_fetch_request::read_from(byte_reader &reader)
{
    return reader.read_fixed_bytes(target_pubkey);
}

void prekey_bundle_response::write_to(byte_writer &writer) const
{
    writer.write_fixed_bytes(owner_pubkey);
    writer.write_fixed_bytes(prekey);
    writer.write_fixed_bytes(signature);
    writer.write_integer(created_at);
}

bool prekey_bundle_response::read_from(byte_reader &reader)
{
    return reader.read_fixed_bytes(owner_pubkey)
        && reader.read_fixed_bytes(prekey)
        && reader.read_fixed_bytes(signature)
        && reader.read_integer(created_at);
}

} // namespace hypercom::proto
