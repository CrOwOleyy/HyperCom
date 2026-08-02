#include "common/util/unix_clock.hpp"

#include <chrono>

namespace hypercom::util {

std::uint64_t get_unix_timestamp()
{
    auto const now = std::chrono::system_clock::now().time_since_epoch();
    auto const seconds =
        std::chrono::duration_cast<std::chrono::seconds>(now).count();
    return seconds < 0 ? 0U : static_cast<std::uint64_t>(seconds);
}

} // namespace hypercom::util
