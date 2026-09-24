#include "server/config/config_parser.hpp"

#include <charconv>
#include <fstream>
#include <sstream>

namespace hypercom::server {
namespace {

[[nodiscard]] std::string_view trim_spaces(std::string_view text)
{
    while (!text.empty() && (text.front() == ' ' || text.front() == '\t')) {
        text.remove_prefix(1);
    }
    while (!text.empty() &&
           (text.back() == ' ' || text.back() == '\t' || text.back() == '\r')) {
        text.remove_suffix(1);
    }
    return text;
}

[[nodiscard]] bool parse_unsigned(std::string_view text, std::uint32_t &out)
{
    auto const result =
        std::from_chars(text.data(), text.data() + text.size(), out);
    return result.ec == std::errc{} && result.ptr == text.data() + text.size();
}

[[nodiscard]] bool parse_boolean(std::string_view text, bool &out)
{
    if (text == "true" || text == "yes" || text == "1") {
        out = true;
        return true;
    }
    if (text == "false" || text == "no" || text == "0") {
        out = false;
        return true;
    }
    return false;
}

// Returns false if the key is unknown or the value malformed. The
// distinction between the two is made by the caller, which knows the line
// number.
[[nodiscard]] bool apply_setting(std::string_view section, std::string_view key,
                                 std::string_view value, server_config &out)
{
    listener_config *const listener = section == "clearnet" ? &out.clearnet
                                      : section == "onion"  ? &out.onion
                                                            : nullptr;
    if (listener != nullptr) {
        if (key == "enabled") {
            return parse_boolean(value, listener->enabled);
        }
        if (key == "bind_address") {
            listener->bind_address = std::string{value};
            return true;
        }
        if (key == "advertised_host") {
            listener->advertised_host = std::string{value};
            return true;
        }
        if (key == "port") {
            std::uint32_t port = 0;
            if (!parse_unsigned(value, port) || port == 0 || port > 65535) {
                return false;
            }
            listener->port = static_cast<std::uint16_t>(port);
            return true;
        }
        return false;
    }
    if (section == "limits") {
        if (key == "max_connections") {
            return parse_unsigned(value, out.limits.max_connections);
        }
        if (key == "max_connections_per_address") {
            return parse_unsigned(value,
                                  out.limits.max_connections_per_address);
        }
        if (key == "max_frame_size") {
            return parse_unsigned(value, out.limits.max_frame_size);
        }
        if (key == "handshake_timeout_seconds") {
            return parse_unsigned(value, out.limits.handshake_timeout_seconds);
        }
        if (key == "idle_timeout_seconds") {
            return parse_unsigned(value, out.limits.idle_timeout_seconds);
        }
        if (key == "requests_per_minute_per_address") {
            return parse_unsigned(value,
                                  out.limits.requests_per_minute_per_address);
        }
        if (key == "requests_per_minute_per_identity") {
            return parse_unsigned(value,
                                  out.limits.requests_per_minute_per_identity);
        }
        return false;
    }
    if (section == "logging") {
        if (key == "level") {
            return util::parse_log_level(value, out.logging.level);
        }
        if (key == "log_peer_addresses") {
            return parse_boolean(value, out.logging.log_peer_addresses);
        }
        if (key == "retention_days") {
            return parse_unsigned(value, out.logging.retention_days);
        }
        if (key == "file_path") {
            out.logging.file_path = std::string{value};
            return true;
        }
        return false;
    }
    if (section == "paths") {
        if (key == "database") {
            out.paths.database_path = std::string{value};
            return true;
        }
        if (key == "migrations") {
            out.paths.migrations_directory = std::string{value};
            return true;
        }
        if (key == "server_key") {
            out.paths.server_key_path = std::string{value};
            return true;
        }
        if (key == "admin_socket") {
            out.paths.admin_socket_path = std::string{value};
            return true;
        }
        if (key == "connect_file") {
            out.paths.connect_file_path = std::string{value};
            return true;
        }
        return false;
    }
    if (section == "server" && key == "registration_open") {
        return parse_boolean(value, out.registration_open);
    }
    return false;
}

void handle_config_line(std::string_view line, std::size_t line_number,
                        std::string &section, server_config &out,
                        std::vector<config_error> &errors)
{
    std::string_view const trimmed = trim_spaces(line);
    if (trimmed.empty() || trimmed.front() == '#') {
        return;
    }
    if (trimmed.front() == '[') {
        if (trimmed.back() != ']') {
            errors.push_back({line_number, "section non fermee"});
            return;
        }
        section = std::string{trimmed.substr(1, trimmed.size() - 2)};
        return;
    }
    std::size_t const separator = trimmed.find('=');
    if (separator == std::string_view::npos) {
        errors.push_back({line_number, "ligne sans '=' et hors section"});
        return;
    }
    std::string_view const key = trim_spaces(trimmed.substr(0, separator));
    std::string_view const value = trim_spaces(trimmed.substr(separator + 1));
    if (section.empty()) {
        errors.push_back({line_number, "reglage hors de toute section"});
        return;
    }
    if (!apply_setting(section, key, value, out)) {
        errors.push_back({line_number, "cle inconnue ou valeur invalide : [" +
                                           section + "] " + std::string{key}});
    }
}

} // namespace

bool parse_config_text(std::string_view text, server_config &out,
                       std::vector<config_error> &errors)
{
    std::string section;
    std::size_t line_number = 0;
    std::size_t offset = 0;
    while (offset <= text.size()) {
        std::size_t const end = text.find('\n', offset);
        std::string_view const line = text.substr(
            offset, end == std::string_view::npos ? text.size() - offset
                                                  : end - offset);
        ++line_number;
        handle_config_line(line, line_number, section, out, errors);
        if (end == std::string_view::npos) {
            break;
        }
        offset = end + 1;
    }
    return errors.empty();
}

bool parse_config_file(std::string const &path, server_config &out,
                       std::vector<config_error> &errors)
{
    std::ifstream input{path, std::ios::binary};
    if (!input) {
        errors.push_back({0, "fichier illisible : " + path});
        return false;
    }
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return parse_config_text(buffer.str(), out, errors);
}

std::string format_config_errors(std::string const &path,
                                 std::vector<config_error> const &errors)
{
    std::ostringstream report;
    report << "configuration invalide : " << path << '\n';
    for (config_error const &error : errors) {
        report << "  ligne " << error.line_number << " : " << error.message
               << '\n';
    }
    return report.str();
}

} // namespace hypercom::server
