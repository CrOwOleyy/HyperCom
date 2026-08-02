#include "common/protocol/forum_list_message.hpp"

#include "common/protocol/record_list_codec.hpp"

namespace hypercom::proto {

void forum_list_request::write_to(byte_writer &writer) const
{
    writer.write_integer(offset);
    writer.write_integer(limit);
}

bool forum_list_request::read_from(byte_reader &reader)
{
    return reader.read_integer(offset) && reader.read_integer(limit);
}

void forum_list_response::write_to(byte_writer &writer) const
{
    write_record_list(writer, forums);
    writer.write_integer(total_count);
}

bool forum_list_response::read_from(byte_reader &reader)
{
    return read_record_list(reader, forums, MAX_LIST_ITEMS)
        && reader.read_integer(total_count);
}

} // namespace hypercom::proto
