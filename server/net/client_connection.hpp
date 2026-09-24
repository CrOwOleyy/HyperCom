#pragma once

#include "server/net/connection_socket.hpp"
#include "server/net/noise_channel.hpp"
#include "server/net/session_state.hpp"

#include <cstdint>
#include <vector>

namespace hypercom::server {

// A connection = its plumbing, its encrypted channel, its application
// state.
//
// A structure with no methods at all. Rule O3 caps classes at five public
// methods, and a connection touches too many things to fit that budget. By
// splitting the three responsibilities into three already-complete types,
// the aggregate has nothing left to do itself -- which is exactly what the
// rule is meant to achieve.
struct client_connection {
    connection_socket socket;
    noise_channel channel;
    session_state session;
    std::vector<std::uint8_t> input_buffer;
};

} // namespace hypercom::server
