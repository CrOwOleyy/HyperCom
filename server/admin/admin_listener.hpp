#pragma once

#include "server/net/unique_descriptor.hpp"

#include <string>

namespace hypercom::server {

// Admin listening socket, over AF_UNIX.
//
// AF_UNIX and nothing else: there is no code path that could expose it to
// the network. That's the property that matters here -- admin has no
// authentication of its own, it relies entirely on the socket file's
// permissions. Opening this on a TCP port would hand server control to
// whoever showed up first.
//
// The file is created with mode 0600: only the account running the server
// can connect to it.
class admin_listener {
public:
    admin_listener();

    // Removes the socket file on shutdown. Without this, a restart would
    // fail on a leftover file, and the socket would stay visible in the
    // filesystem even though nothing is listening behind it anymore.
    ~admin_listener();

    [[nodiscard]] bool open_listener(std::string const &path,
                                     std::string &error_out);

    // Returns -1 when there's nothing left to accept.
    [[nodiscard]] int accept_connection() const;

    [[nodiscard]] int get_descriptor() const;

private:
    unique_descriptor descriptor_;
    std::string path_;
};

} // namespace hypercom::server
