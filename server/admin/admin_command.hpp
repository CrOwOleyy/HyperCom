#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace hypercom::server {

// A parsed admin command line.
//
// The format is line-by-line text, not binary: the CLI needs to stay
// readable and scriptable for someone who doesn't write C++. That's the
// whole point of the admin socket -- the operator types commands, they
// don't program them.
struct admin_command {
    std::string verb;
    std::vector<std::string> arguments;
};

// Splits on whitespace, honoring double quotes: a MOTD contains spaces, and
// we're not going to ask the administrator to escape them. Returns false on
// an empty line or an unclosed quote.
[[nodiscard]] bool parse_admin_command(std::string_view line,
                                       admin_command &out);

} // namespace hypercom::server
