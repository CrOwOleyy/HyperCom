#include "common/protocol/social_records.hpp"

#include "common/protocol/text_field_codec.hpp"

namespace hypercom::proto {
namespace {

[[nodiscard]] bool read_friendship_status(byte_reader &reader,
                                          friendship_status &out)
{
    std::uint8_t raw = 0;
    if (!reader.read_integer(raw)) {
        return false;
    }
    if (raw > static_cast<std::uint8_t>(friendship_status::blocked)) {
        return false;
    }
    out = static_cast<friendship_status>(raw);
    return true;
}

} // namespace

void profile_record::write_to(byte_writer &writer) const
{
    writer.write_fixed_bytes(pubkey);
    write_text_field(writer, handle);
    write_text_field(writer, display_name);
    write_text_field(writer, bio);
    write_text_field(writer, theme_json);
    write_text_field(writer, banner_reference);
}

bool profile_record::read_from(byte_reader &reader)
{
    return reader.read_fixed_bytes(pubkey)
        && read_text_field(reader, handle, MAX_HANDLE_LENGTH)
        && read_text_field(reader, display_name, MAX_DISPLAY_NAME_LENGTH)
        && read_text_field(reader, bio, MAX_BIO_LENGTH)
        && read_text_field(reader, theme_json, MAX_THEME_JSON_LENGTH)
        && read_text_field(reader, banner_reference, MAX_BLOB_REFERENCE_LENGTH);
}

void friend_record::write_to(byte_writer &writer) const
{
    writer.write_fixed_bytes(pubkey);
    write_text_field(writer, handle);
    write_text_field(writer, display_name);
    writer.write_integer(static_cast<std::uint8_t>(status));
}

bool friend_record::read_from(byte_reader &reader)
{
    return reader.read_fixed_bytes(pubkey)
        && read_text_field(reader, handle, MAX_HANDLE_LENGTH)
        && read_text_field(reader, display_name, MAX_DISPLAY_NAME_LENGTH)
        && read_friendship_status(reader, status);
}

} // namespace hypercom::proto
