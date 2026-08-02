#include "server/net/tcp_listener.hpp"

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <string>

namespace hypercom::server {
namespace {

constexpr int LISTEN_BACKLOG = 128;

#if defined(_WIN32)
[[nodiscard]] bool start_windows_sockets()
{
    WSADATA data{};
    return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}
#endif

[[nodiscard]] bool make_non_blocking(int descriptor)
{
#if defined(_WIN32)
    u_long mode = 1;
    return ::ioctlsocket(static_cast<SOCKET>(descriptor), FIONBIO, &mode) == 0;
#else
    int const flags = ::fcntl(descriptor, F_GETFL, 0);
    if (flags < 0) {
        return false;
    }
    return ::fcntl(descriptor, F_SETFL, flags | O_NONBLOCK) == 0;
#endif
}

[[nodiscard]] bool build_address(std::string const &address,
                                 std::uint16_t port, sockaddr_in &out)
{
    out = {};
    out.sin_family = AF_INET;
    out.sin_port = ::htons(port);
    return ::inet_pton(AF_INET, address.c_str(), &out.sin_addr) == 1;
}

} // namespace

tcp_listener::tcp_listener() : descriptor_{} {}

bool tcp_listener::open_listener(std::string const &address,
                                 std::uint16_t port, std::string &error_out)
{
#if defined(_WIN32)
    if (!start_windows_sockets()) {
        error_out = "WSAStartup a echoue";
        return false;
    }
#endif
    sockaddr_in bound_address{};
    if (!build_address(address, port, bound_address)) {
        error_out = "adresse IPv4 invalide : " + address;
        return false;
    }
    unique_descriptor socket_handle{static_cast<int>(::socket(AF_INET, SOCK_STREAM, 0))};
    if (socket_handle.get_value() < 0) {
        error_out = "socket() a echoue";
        return false;
    }
    int const enable = 1;
    static_cast<void>(::setsockopt(socket_handle.get_value(), SOL_SOCKET,
                                   SO_REUSEADDR,
                                   reinterpret_cast<char const *>(&enable),
                                   sizeof(enable)));
    if (::bind(socket_handle.get_value(),
               reinterpret_cast<sockaddr const *>(&bound_address),
               sizeof(bound_address))
        != 0) {
        error_out = "bind " + address + ":" + std::to_string(port) + " a echoue";
        return false;
    }
    if (!make_non_blocking(socket_handle.get_value())
        || ::listen(socket_handle.get_value(), LISTEN_BACKLOG) != 0) {
        error_out = "listen() a echoue";
        return false;
    }
    descriptor_ = std::move(socket_handle);
    return true;
}

int tcp_listener::accept_connection(std::string &peer_address) const
{
    sockaddr_in remote{};
    socklen_t length = sizeof(remote);
    int const accepted = static_cast<int>(::accept(descriptor_.get_value(),
                                                   reinterpret_cast<sockaddr *>(&remote),
                                                   &length));
    if (accepted < 0) {
        return -1;
    }
    if (!make_non_blocking(accepted)) {
#if defined(_WIN32)
        ::closesocket(static_cast<SOCKET>(accepted));
#else
        ::close(accepted);
#endif
        return -1;
    }
    int const enable = 1;
    static_cast<void>(::setsockopt(accepted, IPPROTO_TCP, TCP_NODELAY,
                                   reinterpret_cast<char const *>(&enable),
                                   sizeof(enable)));
    char text[INET_ADDRSTRLEN] = {};
    if (::inet_ntop(AF_INET, &remote.sin_addr, text, sizeof(text)) != nullptr) {
        peer_address = text;
    }
    return accepted;
}

int tcp_listener::get_descriptor() const
{
    return descriptor_.get_value();
}

} // namespace hypercom::server
