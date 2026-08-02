#include "common/protocol/comment_create_message.hpp"

#include "common/protocol/text_field_codec.hpp"

namespace hypercom::proto {

void comment_create_request::write_to(byte_writer &writer) const
{
    writer.write_integer(post_id);
    writer.write_integer(parent_comment_id);
    write_text_field(writer, body);
}

bool comment_create_request::read_from(byte_reader &reader)
{
    return reader.read_integer(post_id)
        && reader.read_integer(parent_comment_id)
        && read_text_field(reader, body, MAX_COMMENT_BODY_LENGTH);
}

void comment_info_response::write_to(byte_writer &writer) const
{
    comment.write_to(writer);
}

bool comment_info_response::read_from(byte_reader &reader)
{
    return comment.read_from(reader);
}

} // namespace hypercom::proto
