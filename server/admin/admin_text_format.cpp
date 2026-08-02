#include "server/admin/admin_text_format.hpp"

namespace hypercom::server {

std::string format_duration(std::uint64_t seconds)
{
    std::uint64_t const days = seconds / 86400;
    std::uint64_t const hours = (seconds % 86400) / 3600;
    std::uint64_t const minutes = (seconds % 3600) / 60;
    std::uint64_t const remainder = seconds % 60;
    std::string text;
    if (days > 0) {
        text += std::to_string(days) + "j ";
    }
    if (days > 0 || hours > 0) {
        text += std::to_string(hours) + "h ";
    }
    if (days > 0 || hours > 0 || minutes > 0) {
        text += std::to_string(minutes) + "m ";
    }
    text += std::to_string(remainder) + "s";
    return text;
}

std::string pad_right(std::string_view text, std::size_t width)
{
    std::string padded{text};
    if (padded.size() < width) {
        padded.append(width - padded.size(), ' ');
    }
    padded.push_back(' ');
    return padded;
}

} // namespace hypercom::server
