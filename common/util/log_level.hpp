#pragma once

#include <string_view>

namespace hypercom::util {

enum class log_level : unsigned char {
    debug = 0,
    info = 1,
    warning = 2,
    error = 3,
    silent = 4,
};

// Resolves the level named in hypercom.conf. Returns false on an unknown
// name: the server then refuses to start rather than silently falling
// back to a default value.
[[nodiscard]] bool parse_log_level(std::string_view name, log_level &out);

[[nodiscard]] std::string_view describe_log_level(log_level level);

} // namespace hypercom::util
