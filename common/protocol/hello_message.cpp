#include "common/protocol/hello_message.hpp"

namespace hypercom::proto {

void hello_request::write_to(byte_writer &writer) const
{
    writer.write_integer(protocol_version);
    writer.write_fixed_bytes(client_pubkey);
}

bool hello_request::read_from(byte_reader &reader)
{
    return reader.read_integer(protocol_version)
        && reader.read_fixed_bytes(client_pubkey);
}

void auth_challenge::write_to(byte_writer &writer) const
{
    writer.write_integer(protocol_version);
    writer.write_fixed_bytes(nonce);
    writer.write_integer(account_exists);
    writer.write_integer(server_time);
}

bool auth_challenge::read_from(byte_reader &reader)
{
    return reader.read_integer(protocol_version)
        && reader.read_fixed_bytes(nonce)
        && reader.read_integer(account_exists)
        && reader.read_integer(server_time);
}

} // namespace hypercom::proto
