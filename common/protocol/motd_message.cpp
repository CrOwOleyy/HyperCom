#include "common/protocol/motd_message.hpp"

#include "common/protocol/protocol_limits.hpp"
#include "common/protocol/text_field_codec.hpp"

namespace hypercom::proto {

void motd_push::write_to(byte_writer &writer) const
{
    writer.write_integer(revision);
    write_text_field(writer, body);
}

bool motd_push::read_from(byte_reader &reader)
{
    return reader.read_integer(revision)
        && read_text_field(reader, body, MAX_MOTD_LENGTH);
}

} // namespace hypercom::proto
