#include "common/protocol/length_prefixed_stream.hpp"

#include "common/protocol/byte_writer.hpp"
#include "common/protocol/endian_codec.hpp"
#include "common/protocol/protocol_limits.hpp"

namespace hypercom::proto {

bool extract_length_prefixed_message(std::vector<std::uint8_t> &buffer,
                                     std::size_t maximum_size,
                                     std::vector<std::uint8_t> &out,
                                     bool &malformed)
{
    malformed = false;
    if (buffer.size() < FRAME_LENGTH_FIELD_SIZE) {
        return false;
    }
    std::uint32_t announced = 0;
    if (!load_little_endian(std::span<std::uint8_t const>{buffer}, announced)) {
        return false;
    }
    std::size_t const length = static_cast<std::size_t>(announced);
    // Checked before allocation, and even before waiting for the bytes: a
    // peer announcing 4 GiB gets rejected immediately, without the server
    // having reserved or waited for anything.
    if (length == 0 || length > maximum_size) {
        malformed = true;
        return false;
    }
    if (buffer.size() < FRAME_LENGTH_FIELD_SIZE + length) {
        return false;
    }
    auto const begin =
        buffer.begin() + static_cast<std::ptrdiff_t>(FRAME_LENGTH_FIELD_SIZE);
    out.assign(begin, begin + static_cast<std::ptrdiff_t>(length));
    buffer.erase(buffer.begin(), begin + static_cast<std::ptrdiff_t>(length));
    return true;
}

void append_length_prefixed_message(std::span<std::uint8_t const> payload,
                                    std::vector<std::uint8_t> &out)
{
    byte_writer writer{out};
    writer.write_integer(static_cast<std::uint32_t>(payload.size()));
    writer.write_fixed_bytes(payload);
}

} // namespace hypercom::proto
