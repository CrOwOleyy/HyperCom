#include "common/protocol/frame_codec.hpp"

#include "common/protocol/byte_writer.hpp"
#include "common/protocol/endian_codec.hpp"
#include "common/protocol/protocol_limits.hpp"

namespace hypercom::proto {

bool peek_body_size(std::span<std::uint8_t const> input, std::uint32_t &out)
{
    std::uint32_t announced = 0;
    if (!load_little_endian(input, announced)) {
        return false;
    }
    // Une trame vide n'existe pas : il y a toujours au moins l'octet de type.
    if (announced < FRAME_TYPE_FIELD_SIZE || announced > MAX_BODY_SIZE) {
        return false;
    }
    out = announced;
    return true;
}

bool decode_frame_header(std::span<std::uint8_t const> input,
                         frame_header &out)
{
    if (input.size() < FRAME_HEADER_SIZE) {
        return false;
    }
    std::uint32_t body_size = 0;
    if (!peek_body_size(input, body_size)) {
        return false;
    }
    std::uint8_t const raw_type = input[FRAME_LENGTH_FIELD_SIZE];
    if (!is_known_message_type(raw_type)) {
        return false;
    }
    out.body_size = body_size;
    out.type = static_cast<message_type>(raw_type);
    return true;
}

bool encode_frame(message_type type, std::span<std::uint8_t const> payload,
                  std::vector<std::uint8_t> &out)
{
    if (payload.size() > MAX_PAYLOAD_SIZE) {
        return false;
    }
    out.clear();
    out.reserve(FRAME_HEADER_SIZE + payload.size());
    byte_writer writer{out};
    writer.write_integer(
        static_cast<std::uint32_t>(FRAME_TYPE_FIELD_SIZE + payload.size()));
    writer.write_integer(static_cast<std::uint8_t>(type));
    writer.write_fixed_bytes(payload);
    return true;
}

} // namespace hypercom::proto
