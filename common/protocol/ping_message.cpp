#include "common/protocol/ping_message.hpp"

namespace hypercom::proto {

void ping_request::write_to(byte_writer &writer) const
{
    writer.write_integer(token);
}

bool ping_request::read_from(byte_reader &reader)
{
    return reader.read_integer(token);
}

void ping_response::write_to(byte_writer &writer) const
{
    writer.write_integer(token);
    writer.write_integer(server_time);
}

bool ping_response::read_from(byte_reader &reader)
{
    return reader.read_integer(token) && reader.read_integer(server_time);
}

} // namespace hypercom::proto
