#include "common/protocol/status_message.hpp"

#include "common/protocol/protocol_limits.hpp"
#include "common/protocol/text_field_codec.hpp"

namespace hypercom::proto {
namespace {

[[nodiscard]] bool read_error_code(byte_reader &reader, error_code &out)
{
    std::uint16_t raw = 0;
    if (!reader.read_integer(raw)) {
        return false;
    }
    if (raw > static_cast<std::uint16_t>(error_code::not_implemented)) {
        return false;
    }
    out = static_cast<error_code>(raw);
    return true;
}

} // namespace

void status_ok_response::write_to(byte_writer &writer) const
{
    writer.write_integer(reference_id);
}

bool status_ok_response::read_from(byte_reader &reader)
{
    return reader.read_integer(reference_id);
}

void status_error_response::write_to(byte_writer &writer) const
{
    writer.write_integer(static_cast<std::uint16_t>(code));
    write_text_field(writer, detail);
}

bool status_error_response::read_from(byte_reader &reader)
{
    return read_error_code(reader, code)
        && read_text_field(reader, detail, MAX_ERROR_MESSAGE_LENGTH);
}

} // namespace hypercom::proto
