#include "common/util/log_level.hpp"

namespace hypercom::util {

bool parse_log_level(std::string_view name, log_level &out)
{
    if (name == "debug") {
        out = log_level::debug;
    } else if (name == "info") {
        out = log_level::info;
    } else if (name == "warning") {
        out = log_level::warning;
    } else if (name == "error") {
        out = log_level::error;
    } else if (name == "silent") {
        out = log_level::silent;
    } else {
        return false;
    }
    return true;
}

std::string_view describe_log_level(log_level level)
{
    switch (level) {
        case log_level::debug:   return "debug";
        case log_level::info:    return "info";
        case log_level::warning: return "warning";
        case log_level::error:   return "error";
        case log_level::silent:  return "silent";
    }
    return "unknown";
}

} // namespace hypercom::util
