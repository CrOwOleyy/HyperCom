#include "common/protocol/text_field_codec.hpp"

#include <cstdint>
#include <vector>

namespace hypercom::proto {
namespace {

[[nodiscard]] bool is_allowed_ascii(std::uint8_t byte)
{
    if (byte == 0x09U || byte == 0x0AU) {
        return true;
    }
    return byte >= 0x20U && byte != 0x7FU;
}

// Rejette les encodages sur-longs, les substituts et le hors-plan.
[[nodiscard]] bool is_canonical_code_point(std::uint32_t code_point,
                                           std::size_t length)
{
    if (code_point >= 0xD800U && code_point <= 0xDFFFU) {
        return false;
    }
    if (length == 2) {
        return code_point >= 0x80U;
    }
    if (length == 3) {
        return code_point >= 0x800U;
    }
    return code_point >= 0x10000U && code_point <= 0x10FFFFU;
}

// Renvoie le nombre d'octets consommes, ou 0 si la sequence est invalide.
[[nodiscard]] std::size_t measure_utf8_sequence(std::string_view text,
                                                std::size_t offset)
{
    std::uint8_t const lead = static_cast<std::uint8_t>(text[offset]);
    if (lead < 0x80U) {
        return is_allowed_ascii(lead) ? 1U : 0U;
    }
    std::size_t length = 0;
    std::uint32_t code_point = 0;
    if ((lead & 0xE0U) == 0xC0U) {
        length = 2;
        code_point = lead & 0x1FU;
    } else if ((lead & 0xF0U) == 0xE0U) {
        length = 3;
        code_point = lead & 0x0FU;
    } else if ((lead & 0xF8U) == 0xF0U) {
        length = 4;
        code_point = lead & 0x07U;
    } else {
        return 0U;
    }
    if (offset + length > text.size()) {
        return 0U;
    }
    for (std::size_t index = 1; index < length; ++index) {
        auto const byte = static_cast<std::uint8_t>(text[offset + index]);
        if ((byte & 0xC0U) != 0x80U) {
            return 0U;
        }
        code_point = (code_point << 6U) | (byte & 0x3FU);
    }
    return is_canonical_code_point(code_point, length) ? length : 0U;
}

} // namespace

bool validate_text_field(std::string_view text)
{
    std::size_t offset = 0;
    while (offset < text.size()) {
        std::size_t const consumed = measure_utf8_sequence(text, offset);
        if (consumed == 0) {
            return false;
        }
        offset += consumed;
    }
    return true;
}

bool read_text_field(byte_reader &reader, std::string &out,
                     std::size_t maximum_length)
{
    std::vector<std::uint8_t> raw;
    if (!reader.read_length_prefixed(raw, maximum_length)) {
        return false;
    }
    std::string decoded(raw.begin(), raw.end());
    if (!validate_text_field(decoded)) {
        return false;
    }
    out = std::move(decoded);
    return true;
}

void write_text_field(byte_writer &writer, std::string_view text)
{
    writer.write_length_prefixed(
        {reinterpret_cast<std::uint8_t const *>(text.data()), text.size()});
}

} // namespace hypercom::proto
