#include "client/cli/invite_link.hpp"

#include <algorithm>
#include <charconv>

namespace hypercom::client {
namespace {

constexpr std::string_view LINK_PREFIX = "hypercom://";
constexpr std::size_t SERVER_KEY_HEX_LENGTH = 64;

[[nodiscard]] bool parse_port(std::string_view text, std::uint16_t &out)
{
    std::uint32_t value = 0;
    auto const result =
        std::from_chars(text.data(), text.data() + text.size(), value);
    if (result.ec != std::errc{} || result.ptr != text.data() + text.size() ||
        value == 0 || value > 65535) {
        return false;
    }
    out = static_cast<std::uint16_t>(value);
    return true;
}

[[nodiscard]] bool is_hex_key(std::string_view text)
{
    return text.size() == SERVER_KEY_HEX_LENGTH &&
           std::all_of(text.begin(), text.end(), [](char character) {
               return (character >= '0' && character <= '9') ||
                      (character >= 'a' && character <= 'f') ||
                      (character >= 'A' && character <= 'F');
           });
}

} // namespace

bool parse_invite_link(std::string_view text, invite_link &out,
                       std::string &error_out)
{
    if (!text.starts_with(LINK_PREFIX)) {
        error_out = "lien invalide : attendu hypercom://hote:port#cle";
        return false;
    }
    std::string_view body = text.substr(LINK_PREFIX.size());
    std::size_t const fragment = body.find('#');
    if (fragment == std::string_view::npos) {
        error_out = "lien invalide : la cle du serveur manque apres le #";
        return false;
    }
    std::string_view const key = body.substr(fragment + 1);
    if (!is_hex_key(key)) {
        error_out = "lien invalide : la cle doit faire 64 caracteres "
                    "hexadecimaux";
        return false;
    }
    body = body.substr(0, fragment);
    // rfind: a literal IPv6 host contains colons, only the last one
    // separates the port.
    std::size_t const separator = body.rfind(':');
    if (separator == std::string_view::npos ||
        !parse_port(body.substr(separator + 1), out.port)) {
        error_out = "lien invalide : port absent ou hors bornes";
        return false;
    }
    out.host = std::string{body.substr(0, separator)};
    if (out.host.empty()) {
        error_out = "lien invalide : hote manquant";
        return false;
    }
    out.server_key_hex = std::string{key};
    return true;
}

std::string format_invite_link(std::string_view host, std::uint16_t port,
                               std::string_view server_key_hex)
{
    return std::string{LINK_PREFIX} + std::string{host} + ":" +
           std::to_string(port) + "#" + std::string{server_key_hex};
}

} // namespace hypercom::client
