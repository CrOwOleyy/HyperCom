#pragma once

#include "server/config/server_config.hpp"

#include <string>
#include <vector>

namespace hypercom::server {

// Overall consistency, after each value has been individually parsed.
//
// The server REFUSES to start on an invalid configuration rather than
// falling back to silent defaults. A server that quietly falls back to a
// default port or logging policy is a server that will, one day, log IPs
// without anyone having decided so.
//
// problems and warnings are genuinely two different things: the former
// prevent startup, the latter are printed and then the server starts.
// Conflating them would make an otherwise legitimate setting unusable --
// enabling IP logging should be loud, not forbidden.
[[nodiscard]] bool validate_config(server_config const &config,
                                   std::vector<std::string> &problems,
                                   std::vector<std::string> &warnings);

} // namespace hypercom::server
