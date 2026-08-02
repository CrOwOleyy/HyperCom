#include "common/protocol/profile_set_message.hpp"

#include "common/protocol/protocol_limits.hpp"
#include "common/protocol/text_field_codec.hpp"

namespace hypercom::proto {

void profile_set_request::write_to(byte_writer &writer) const
{
    write_text_field(writer, display_name);
    write_text_field(writer, bio);
    write_text_field(writer, theme_json);
    write_text_field(writer, banner_reference);
}

bool profile_set_request::read_from(byte_reader &reader)
{
    return read_text_field(reader, display_name, MAX_DISPLAY_NAME_LENGTH)
        && read_text_field(reader, bio, MAX_BIO_LENGTH)
        && read_text_field(reader, theme_json, MAX_THEME_JSON_LENGTH)
        && read_text_field(reader, banner_reference, MAX_BLOB_REFERENCE_LENGTH);
}

} // namespace hypercom::proto
