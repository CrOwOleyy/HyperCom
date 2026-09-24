#include "server/admin/admin_command.hpp"

namespace hypercom::server {
namespace {

constexpr std::size_t MAX_COMMAND_TOKENS = 16;

[[nodiscard]] bool is_blank(char character)
{
    return character == ' ' || character == '\t' || character == '\r';
}

// Reads a word starting at offset, quoted or not. Advances offset to the
// next character. Returns false on a quote that opens but never closes.
[[nodiscard]] bool take_token(std::string_view line, std::size_t &offset,
                              std::string &out)
{
    bool const quoted = line[offset] == '"';
    if (quoted) {
        ++offset;
    }
    std::string token;
    while (offset < line.size()) {
        char const character = line[offset];
        if (quoted && character == '"') {
            ++offset;
            out = std::move(token);
            return true;
        }
        if (!quoted && is_blank(character)) {
            break;
        }
        token.push_back(character);
        ++offset;
    }
    if (quoted) {
        return false;
    }
    out = std::move(token);
    return true;
}

} // namespace

bool parse_admin_command(std::string_view line, admin_command &out)
{
    std::vector<std::string> tokens;
    std::size_t offset = 0;
    while (offset < line.size()) {
        if (is_blank(line[offset])) {
            ++offset;
            continue;
        }
        if (tokens.size() >= MAX_COMMAND_TOKENS) {
            return false;
        }
        std::string token;
        if (!take_token(line, offset, token)) {
            return false;
        }
        tokens.push_back(std::move(token));
    }
    if (tokens.empty()) {
        return false;
    }
    out.verb = tokens.front();
    out.arguments.assign(tokens.begin() + 1, tokens.end());
    return true;
}

} // namespace hypercom::server
