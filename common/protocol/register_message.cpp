#include "common/protocol/register_message.hpp"

#include "common/protocol/protocol_limits.hpp"
#include "common/protocol/text_field_codec.hpp"

namespace hypercom::proto {
namespace {

constexpr std::size_t MIN_HANDLE_LENGTH = 3;

[[nodiscard]] bool is_handle_character(char character)
{
    if ((character >= 'a' && character <= 'z') ||
        (character >= 'A' && character <= 'Z')) {
        return true;
    }
    if (character >= '0' && character <= '9') {
        return true;
    }
    return character == '-' || character == '_';
}

} // namespace

void register_request::write_to(byte_writer &writer) const
{
    write_text_field(writer, handle);
}

bool register_request::read_from(byte_reader &reader)
{
    return read_text_field(reader, handle, MAX_HANDLE_LENGTH);
}

bool validate_handle(std::string_view handle)
{
    if (handle.size() < MIN_HANDLE_LENGTH ||
        handle.size() > MAX_HANDLE_LENGTH) {
        return false;
    }
    // Letters (lower and upper case), digits, hyphen, underscore required.
    if (handle.front() >= '0' && handle.front() <= '9') {
        return false;
    }
    for (char const character : handle) {
        if (!is_handle_character(character)) {
            return false;
        }
    }
    return true;
}

} // namespace hypercom::proto
