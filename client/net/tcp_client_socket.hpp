#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace hypercom::client {

// Portable Windows / Linux TCP socket.
//
// It's blocking, with a read timeout. This is a deliberate choice: the
// client has only one connection, and a full event loop would serve no
// purpose. The short timeout lets the immediate-mode UI poll the network
// on every frame without ever freezing.
class tcp_client_socket {
public:
    tcp_client_socket();

    ~tcp_client_socket();

    // Reconnectable: calling this on an already-open socket closes the
    // previous one before starting over. This is what enables
    // reconnection via a full re-handshake without exposing a public
    // close method, which rule O3 wouldn't allow.
    [[nodiscard]] bool connect_to_host(std::string const &host,
                                       std::uint16_t port,
                                       std::string &error_out);

    // Writes the entire buffer, looping over partial writes.
    [[nodiscard]] bool send_all(std::span<std::uint8_t const> data);

    // Appends what's available. received_any distinguishes normal
    // silence (timeout elapsed) from a closed connection, which is
    // signaled by false.
    [[nodiscard]] bool receive_available(std::vector<std::uint8_t> &destination,
                                         bool &received_any);

private:
    void close_handle();

    // Deliberately wide type: SOCKET is 64 bits on Windows, int on
    // POSIX.
    std::intptr_t handle_;
};

} // namespace hypercom::client
