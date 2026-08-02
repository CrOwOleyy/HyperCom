#include "common/protocol/post_list_message.hpp"

#include "common/protocol/record_list_codec.hpp"

namespace hypercom::proto {

void post_list_request::write_to(byte_writer &writer) const
{
    writer.write_integer(forum_id);
    writer.write_integer(offset);
    writer.write_integer(limit);
}

bool post_list_request::read_from(byte_reader &reader)
{
    return reader.read_integer(forum_id) && reader.read_integer(offset)
        && reader.read_integer(limit);
}

void post_list_response::write_to(byte_writer &writer) const
{
    write_record_list(writer, posts);
    writer.write_integer(total_count);
}

bool post_list_response::read_from(byte_reader &reader)
{
    return read_record_list(reader, posts, MAX_LIST_ITEMS)
        && reader.read_integer(total_count);
}

} // namespace hypercom::proto
