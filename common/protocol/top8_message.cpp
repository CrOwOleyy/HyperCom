#include "common/protocol/top8_message.hpp"

#include "common/protocol/record_list_codec.hpp"

namespace hypercom::proto {
namespace {

void write_slot_array(byte_writer &writer,
                      std::array<wire_public_key, TOP8_SLOT_COUNT> const &slots)
{
    for (wire_public_key const &slot : slots) {
        writer.write_fixed_bytes(slot);
    }
}

[[nodiscard]] bool
read_slot_array(byte_reader &reader,
                std::array<wire_public_key, TOP8_SLOT_COUNT> &slots)
{
    for (wire_public_key &slot : slots) {
        if (!reader.read_fixed_bytes(slot)) {
            return false;
        }
    }
    return true;
}

} // namespace

void top8_set_request::write_to(byte_writer &writer) const
{
    write_slot_array(writer, slots);
}

bool top8_set_request::read_from(byte_reader &reader)
{
    return read_slot_array(reader, slots);
}

void top8_response::write_to(byte_writer &writer) const
{
    write_slot_array(writer, slots);
    write_record_list(writer, details);
}

bool top8_response::read_from(byte_reader &reader)
{
    return read_slot_array(reader, slots)
        && read_record_list(reader, details, TOP8_SLOT_COUNT);
}

} // namespace hypercom::proto
