#include "common/protocol/content_records.hpp"

#include "common/protocol/text_field_codec.hpp"

namespace hypercom::proto {

void post_record::write_to(byte_writer &writer) const
{
    writer.write_integer(id);
    writer.write_integer(forum_id);
    writer.write_fixed_bytes(author_pubkey);
    write_text_field(writer, author_handle);
    write_text_field(writer, title);
    write_text_field(writer, body);
    writer.write_integer(created_at);
    writer.write_integer(comment_count);
}

bool post_record::read_from(byte_reader &reader)
{
    return reader.read_integer(id)
        && reader.read_integer(forum_id)
        && reader.read_fixed_bytes(author_pubkey)
        && read_text_field(reader, author_handle, MAX_HANDLE_LENGTH)
        && read_text_field(reader, title, MAX_POST_TITLE_LENGTH)
        && read_text_field(reader, body, MAX_POST_BODY_LENGTH)
        && reader.read_integer(created_at)
        && reader.read_integer(comment_count);
}

void comment_record::write_to(byte_writer &writer) const
{
    writer.write_integer(id);
    writer.write_integer(post_id);
    writer.write_integer(parent_comment_id);
    writer.write_fixed_bytes(author_pubkey);
    write_text_field(writer, author_handle);
    write_text_field(writer, body);
    writer.write_integer(created_at);
    writer.write_integer(depth);
}

bool comment_record::read_from(byte_reader &reader)
{
    return reader.read_integer(id)
        && reader.read_integer(post_id)
        && reader.read_integer(parent_comment_id)
        && reader.read_fixed_bytes(author_pubkey)
        && read_text_field(reader, author_handle, MAX_HANDLE_LENGTH)
        && read_text_field(reader, body, MAX_COMMENT_BODY_LENGTH)
        && reader.read_integer(created_at)
        && reader.read_integer(depth);
}

} // namespace hypercom::proto
