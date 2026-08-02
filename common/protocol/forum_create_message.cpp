#include "common/protocol/forum_create_message.hpp"

#include "common/protocol/text_field_codec.hpp"

namespace hypercom::proto {
namespace {

constexpr std::size_t MIN_FORUM_NAME_LENGTH = 2;

[[nodiscard]] bool is_forum_name_character(char character)
{
    if (character >= 'a' && character <= 'z') {
        return true;
    }
    if (character >= '0' && character <= '9') {
        return true;
    }
    return character == '-' || character == '_';
}

} // namespace

void forum_create_request::write_to(byte_writer &writer) const
{
    write_text_field(writer, name);
    write_text_field(writer, description);
    write_text_field(writer, theme_json);
}

bool forum_create_request::read_from(byte_reader &reader)
{
    return read_text_field(reader, name, MAX_FORUM_NAME_LENGTH)
        && read_text_field(reader, description, MAX_FORUM_DESCRIPTION_LENGTH)
        && read_text_field(reader, theme_json, MAX_THEME_JSON_LENGTH);
}

void forum_info_response::write_to(byte_writer &writer) const
{
    forum.write_to(writer);
}

bool forum_info_response::read_from(byte_reader &reader)
{
    return forum.read_from(reader);
}

bool validate_forum_name(std::string_view name)
{
    if (name.size() < MIN_FORUM_NAME_LENGTH
        || name.size() > MAX_FORUM_NAME_LENGTH) {
        return false;
    }
    for (char const character : name) {
        if (!is_forum_name_character(character)) {
            return false;
        }
    }
    return true;
}

} // namespace hypercom::proto
