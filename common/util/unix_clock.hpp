#pragma once

#include <cstdint>

namespace hypercom::util {

// The project's single timestamp format: seconds since the UNIX epoch,
// UTC. Every date stored or transmitted uses this unit, never a local
// time or a textual format.
[[nodiscard]] std::uint64_t get_unix_timestamp();

} // namespace hypercom::util
