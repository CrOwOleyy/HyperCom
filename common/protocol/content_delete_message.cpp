#include "common/protocol/content_delete_message.hpp"

namespace hypercom::proto {

void post_delete_request::write_to(byte_writer &writer) const
{
    writer.write_integer(post_id);
}

bool post_delete_request::read_from(byte_reader &reader)
{
    return reader.read_integer(post_id);
}

void comment_delete_request::write_to(byte_writer &writer) const
{
    writer.write_integer(comment_id);
}

bool comment_delete_request::read_from(byte_reader &reader)
{
    return reader.read_integer(comment_id);
}

} // namespace hypercom::proto
