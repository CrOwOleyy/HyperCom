#include "common/protocol/post_create_message.hpp"

#include "common/protocol/text_field_codec.hpp"

namespace hypercom::proto {

void post_create_request::write_to(byte_writer &writer) const
{
    writer.write_integer(forum_id);
    write_text_field(writer, title);
    write_text_field(writer, body);
}

bool post_create_request::read_from(byte_reader &reader)
{
    return reader.read_integer(forum_id)
        && read_text_field(reader, title, MAX_POST_TITLE_LENGTH)
        && read_text_field(reader, body, MAX_POST_BODY_LENGTH);
}

void post_info_response::write_to(byte_writer &writer) const
{
    post.write_to(writer);
}

bool post_info_response::read_from(byte_reader &reader)
{
    return post.read_from(reader);
}

} // namespace hypercom::proto
