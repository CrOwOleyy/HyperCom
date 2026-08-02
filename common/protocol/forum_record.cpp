#include "common/protocol/forum_record.hpp"

#include "common/protocol/text_field_codec.hpp"

namespace hypercom::proto {

void forum_record::write_to(byte_writer &writer) const
{
    writer.write_integer(id);
    write_text_field(writer, name);
    write_text_field(writer, description);
    writer.write_fixed_bytes(founder_pubkey);
    write_text_field(writer, founder_handle);
    write_text_field(writer, theme_json);
    writer.write_integer(created_at);
    writer.write_integer(post_count);
}

bool forum_record::read_from(byte_reader &reader)
{
    return reader.read_integer(id)
        && read_text_field(reader, name, MAX_FORUM_NAME_LENGTH)
        && read_text_field(reader, description, MAX_FORUM_DESCRIPTION_LENGTH)
        && reader.read_fixed_bytes(founder_pubkey)
        && read_text_field(reader, founder_handle, MAX_HANDLE_LENGTH)
        && read_text_field(reader, theme_json, MAX_THEME_JSON_LENGTH)
        && reader.read_integer(created_at)
        && reader.read_integer(post_count);
}

} // namespace hypercom::proto
