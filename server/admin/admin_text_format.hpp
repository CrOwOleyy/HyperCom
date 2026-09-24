#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace hypercom::server {

// Formatting for text rendered by the admin CLI.
//
// Output aligned in columns rather than JSON: the CLI speaks to a human at
// a terminal first, and stays readable with awk or cut if needed.

[[nodiscard]] std::string format_duration(std::uint64_t seconds);

[[nodiscard]] std::string pad_right(std::string_view text, std::size_t width);

} // namespace hypercom::server
