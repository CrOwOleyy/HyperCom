#pragma once

#include "client/net/tcp_client_socket.hpp"

#include <cstdint>
#include <string>

namespace hypercom::client {

// SOCKS5 negotiation (RFC 1928) over a socket ALREADY connected to the
// proxy.
//
// This is what makes reaching a hidden .onion service possible: Tor isn't
// in DNS, a getaddrinfo() call would fail. The client connects to Tor's
// local proxy, asks it to open the connection to the .onion address, and
// the socket then becomes a transparent tunnel -- the Noise handshake
// runs on top of it with no idea any of this is happening.
//
// Only "no authentication" mode is handled: the proxy lives on the
// user's own machine over the loopback interface, there's no one to
// authenticate.
[[nodiscard]] bool perform_socks5_connect(tcp_client_socket &socket,
                                          std::string const &target_host,
                                          std::uint16_t target_port,
                                          std::string &error_out);

} // namespace hypercom::client
