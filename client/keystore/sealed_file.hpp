#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace hypercom::client {

// Reads and writes the client's secret files, at 0600.
//
// Shared by the master seed and the server registry: both write a sealed
// blob and nothing else, so there's no reason to have two implementations
// that would drift apart over time.

// On POSIX, a failing chmod is a reported error, not a swallowed warning:
// a world-readable secrets file must not go unnoticed.
[[nodiscard]] bool write_sealed_file(std::string const &path,
                                     std::span<std::uint8_t const> sealed,
                                     std::string &error_out);

[[nodiscard]] bool read_sealed_file(std::string const &path,
                                    std::vector<std::uint8_t> &out);

} // namespace hypercom::client
