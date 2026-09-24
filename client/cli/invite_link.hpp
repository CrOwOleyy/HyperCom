#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace hypercom::client {

// Pasteable invite line: hypercom://host:port#key_hex
//
// It's the only way to join a server: there's no directory, you get this
// link from someone you trust. Nothing in it is secret -- the server's
// key is public by nature -- but it must arrive over a trusted channel,
// otherwise pinning no longer protects anything.
//
// The fragment (#) carries the key by convention: it's the part of a URL
// that's never sent to a web server, a reminder that it has no business
// being anywhere but in the client.
struct invite_link {
    std::string host;
    std::uint16_t port = 0;
    std::string server_key_hex;
};

[[nodiscard]] bool parse_invite_link(std::string_view text, invite_link &out,
                                     std::string &error_out);

[[nodiscard]] std::string format_invite_link(std::string_view host,
                                             std::uint16_t port,
                                             std::string_view server_key_hex);

} // namespace hypercom::client
