#include "client/net/tcp_client_socket.hpp"

#include <array>
#include <cstring>

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <cerrno>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#endif

namespace hypercom::client {
namespace {

constexpr std::size_t READ_CHUNK_SIZE = 16 * 1024;
constexpr int RECEIVE_TIMEOUT_MILLISECONDS = 200;
constexpr std::intptr_t INVALID_HANDLE = -1;

#if defined(_WIN32)
[[nodiscard]] bool start_windows_sockets()
{
    WSADATA data{};
    // Winsock compte lui-meme ses initialisations : appeler WSAStartup une
    // fois par socket est correct, et evite une globale (regle G4).
    return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}
#endif

void apply_receive_timeout(std::intptr_t handle)
{
#if defined(_WIN32)
    DWORD const timeout = RECEIVE_TIMEOUT_MILLISECONDS;
    setsockopt(static_cast<SOCKET>(handle), SOL_SOCKET, SO_RCVTIMEO,
               reinterpret_cast<char const *>(&timeout), sizeof(timeout));
#else
    timeval timeout{};
    timeout.tv_usec = RECEIVE_TIMEOUT_MILLISECONDS * 1000;
    setsockopt(static_cast<int>(handle), SOL_SOCKET, SO_RCVTIMEO, &timeout,
               sizeof(timeout));
#endif
}

[[nodiscard]] bool is_timeout_error()
{
#if defined(_WIN32)
    int const code = WSAGetLastError();
    return code == WSAETIMEDOUT || code == WSAEWOULDBLOCK;
#else
    return errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR;
#endif
}

} // namespace

tcp_client_socket::tcp_client_socket() : handle_{INVALID_HANDLE}
{
#if defined(_WIN32)
    static_cast<void>(start_windows_sockets());
#endif
}

tcp_client_socket::~tcp_client_socket()
{
    if (handle_ != INVALID_HANDLE) {
#if defined(_WIN32)
        closesocket(static_cast<SOCKET>(handle_));
        WSACleanup();
#else
        ::close(static_cast<int>(handle_));
#endif
    }
}

bool tcp_client_socket::connect_to_host(std::string const &host,
                                        std::uint16_t port,
                                        std::string &error_out)
{
    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    addrinfo *resolved = nullptr;
    std::string const service = std::to_string(port);
    if (getaddrinfo(host.c_str(), service.c_str(), &hints, &resolved) != 0) {
        error_out = "hote introuvable : " + host;
        return false;
    }
    for (addrinfo *entry = resolved; entry != nullptr;
         entry = entry->ai_next) {
        auto const attempt = static_cast<std::intptr_t>(
            socket(entry->ai_family, entry->ai_socktype, entry->ai_protocol));
        if (attempt == INVALID_HANDLE) {
            continue;
        }
        if (connect(static_cast<
#if defined(_WIN32)
                        SOCKET
#else
                        int
#endif
                        >(attempt),
                    entry->ai_addr, static_cast<int>(entry->ai_addrlen))
            == 0) {
            handle_ = attempt;
            apply_receive_timeout(handle_);
            freeaddrinfo(resolved);
            return true;
        }
#if defined(_WIN32)
        closesocket(static_cast<SOCKET>(attempt));
#else
        ::close(static_cast<int>(attempt));
#endif
    }
    freeaddrinfo(resolved);
    error_out = "connexion refusee : " + host + ":" + service;
    return false;
}

bool tcp_client_socket::send_all(std::span<std::uint8_t const> data)
{
    std::size_t offset = 0;
    while (offset < data.size()) {
        auto const remaining = static_cast<int>(data.size() - offset);
#if defined(_WIN32)
        int const sent = send(
            static_cast<SOCKET>(handle_),
            reinterpret_cast<char const *>(data.data() + offset), remaining, 0);
#else
        ssize_t const sent = ::send(static_cast<int>(handle_),
                                    data.data() + offset,
                                    static_cast<std::size_t>(remaining),
                                    MSG_NOSIGNAL);
#endif
        if (sent > 0) {
            offset += static_cast<std::size_t>(sent);
            continue;
        }
        if (sent < 0 && is_timeout_error()) {
            continue;
        }
        return false;
    }
    return true;
}

bool tcp_client_socket::receive_available(
    std::vector<std::uint8_t> &destination, bool &received_any)
{
    received_any = false;
    std::array<std::uint8_t, READ_CHUNK_SIZE> chunk{};
#if defined(_WIN32)
    int const received =
        recv(static_cast<SOCKET>(handle_),
             reinterpret_cast<char *>(chunk.data()),
             static_cast<int>(chunk.size()), 0);
#else
    ssize_t const received = ::recv(static_cast<int>(handle_), chunk.data(),
                                    chunk.size(), 0);
#endif
    if (received > 0) {
        destination.insert(destination.end(), chunk.begin(),
                           chunk.begin() + received);
        received_any = true;
        return true;
    }
    // Zero octet signifie fermeture propre par le pair, pas simplement silence.
    return received < 0 && is_timeout_error();
}

} // namespace hypercom::client
