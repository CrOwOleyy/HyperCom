#include "common/util/hex_codec.hpp"

namespace hypercom::util {
namespace {

constexpr char HEX_DIGITS[] = "0123456789abcdef";

[[nodiscard]] bool convert_hex_digit(char character, std::uint8_t &out)
{
    if (character >= '0' && character <= '9') {
        out = static_cast<std::uint8_t>(character - '0');
        return true;
    }
    if (character >= 'a' && character <= 'f') {
        out = static_cast<std::uint8_t>(character - 'a' + 10);
        return true;
    }
    if (character >= 'A' && character <= 'F') {
        out = static_cast<std::uint8_t>(character - 'A' + 10);
        return true;
    }
    return false;
}

} // namespace

void encode_hex(std::span<std::uint8_t const> input, std::string &out)
{
    out.clear();
    out.reserve(input.size() * 2);
    for (std::uint8_t const byte : input) {
        out.push_back(HEX_DIGITS[byte >> 4]);
        out.push_back(HEX_DIGITS[byte & 0x0F]);
    }
}

bool decode_hex(std::string_view input, std::vector<std::uint8_t> &out)
{
    if (input.size() % 2 != 0) {
        return false;
    }
    std::vector<std::uint8_t> decoded;
    decoded.reserve(input.size() / 2);
    for (std::size_t index = 0; index < input.size(); index += 2) {
        std::uint8_t high = 0;
        std::uint8_t low = 0;
        if (!convert_hex_digit(input[index], high)) {
            return false;
        }
        if (!convert_hex_digit(input[index + 1], low)) {
            return false;
        }
        decoded.push_back(static_cast<std::uint8_t>((high << 4) | low));
    }
    out = std::move(decoded);
    return true;
}

} // namespace hypercom::util
