#pragma once

#include "server/net/unique_descriptor.hpp"

#include <cstdint>
#include <string>

namespace hypercom::server {

// Listening socket. The same code serves both clearnet and the hidden
// service: Tor simply relays to a listener on the loopback interface, so
// there is no onion-specific logic at all.
class tcp_listener {
public:
    tcp_listener();

    [[nodiscard]] bool open_listener(std::string const &address,
                                     std::uint16_t port,
                                     std::string &error_out);

    // Returns -1 when there's nothing left to accept. peer_address is only
    // filled in to be passed to the logger, which will redact it if the
    // logging policy doesn't allow it.
    [[nodiscard]] int accept_connection(std::string &peer_address) const;

    [[nodiscard]] int get_descriptor() const;

private:
    unique_descriptor descriptor_;
};

} // namespace hypercom::server
