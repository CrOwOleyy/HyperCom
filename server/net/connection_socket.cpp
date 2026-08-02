#include "server/net/connection_socket.hpp"

#if defined(_WIN32)
#include <winsock2.h>
#else
#include <cerrno>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <array>

namespace hypercom::server {
namespace {

constexpr std::size_t READ_CHUNK_SIZE = 16 * 1024;
constexpr std::size_t MAX_PENDING_OUTPUT = 4 * 1024 * 1024;

} // namespace

connection_socket::connection_socket(int descriptor)
    : descriptor_{descriptor}, pending_output_{}
{
}

bool connection_socket::read_available(std::vector<std::uint8_t> &destination)
{
    std::array<std::uint8_t, READ_CHUNK_SIZE> chunk{};
    while (true) {
#if defined(_WIN32)
        int const received = ::recv(static_cast<SOCKET>(descriptor_.get_value()),
                                   reinterpret_cast<char *>(chunk.data()),
                                   static_cast<int>(chunk.size()), 0);
#else
        ssize_t const received =
            ::recv(descriptor_.get_value(), chunk.data(), chunk.size(), 0);
#endif
        if (received > 0) {
            destination.insert(destination.end(), chunk.begin(),
                               chunk.begin() + received);
            continue;
        }
        if (received == 0) {
            return false;
        }
#if defined(_WIN32)
        int const err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK) {
            return true;
        }
        if (err == WSAEINTR) {
            continue;
        }
#else
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return true;
        }
        if (errno == EINTR) {
            continue;
        }
#endif
        return false;
    }
}

bool connection_socket::flush_pending_writes(bool &has_remaining)
{
    while (!pending_output_.empty()) {
#if defined(_WIN32)
        int const sent = ::send(static_cast<SOCKET>(descriptor_.get_value()),
                                reinterpret_cast<char const *>(pending_output_.data()),
                                static_cast<int>(pending_output_.size()), 0);
#else
        ssize_t const sent = ::send(descriptor_.get_value(),
                                    pending_output_.data(),
                                    pending_output_.size(), MSG_NOSIGNAL);
#endif
        if (sent > 0) {
            pending_output_.erase(pending_output_.begin(),
                                  pending_output_.begin() + sent);
            continue;
        }
#if defined(_WIN32)
        int const err = WSAGetLastError();
        if (sent < 0 && err == WSAEWOULDBLOCK) {
            has_remaining = true;
            return true;
        }
        if (sent < 0 && err == WSAEINTR) {
            continue;
        }
#else
        if (sent < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            has_remaining = true;
            return true;
        }
        if (sent < 0 && errno == EINTR) {
            continue;
        }
#endif
        return false;
    }
    has_remaining = false;
    return true;
}

void connection_socket::queue_bytes(std::span<std::uint8_t const> data)
{
    if (pending_output_.size() + data.size() > MAX_PENDING_OUTPUT) {
        pending_output_.clear();
        return;
    }
    pending_output_.insert(pending_output_.end(), data.begin(), data.end());
}

int connection_socket::get_descriptor() const
{
    return descriptor_.get_value();
}

} // namespace hypercom::server
