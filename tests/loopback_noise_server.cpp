#include "tests/loopback_noise_server.hpp"

#include "common/crypto/noise_handshake_responder.hpp"
#include "common/crypto/noise_transport.hpp"
#include "common/protocol/length_prefixed_stream.hpp"
#include "common/protocol/protocol_limits.hpp"

#include <arpa/inet.h>
#include <array>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

namespace hypercom::tests {
namespace {

constexpr std::size_t READ_CHUNK_SIZE = 4096;
constexpr std::size_t MAX_TEST_MESSAGE_SIZE = proto::MAX_FRAME_SIZE;

// Without this timeout, a client that never shows up hangs accept() -- and
// therefore the test's join(). A failing test should fail, not freeze.
constexpr int ACCEPT_TIMEOUT_SECONDS = 10;

void apply_accept_timeout(int descriptor)
{
    timeval timeout{};
    timeout.tv_sec = ACCEPT_TIMEOUT_SECONDS;
    static_cast<void>(::setsockopt(descriptor, SOL_SOCKET, SO_RCVTIMEO,
                                   &timeout, sizeof(timeout)));
}

// Reads until a full message can be extracted, or fails if the peer closes.
[[nodiscard]] bool read_one_message(int descriptor,
                                    std::vector<std::uint8_t> &buffer,
                                    std::vector<std::uint8_t> &out)
{
    for (;;) {
        bool malformed = false;
        if (proto::extract_length_prefixed_message(
                buffer, MAX_TEST_MESSAGE_SIZE, out, malformed)) {
            return true;
        }
        if (malformed) {
            return false;
        }
        std::array<std::uint8_t, READ_CHUNK_SIZE> chunk{};
        ssize_t const received =
            ::recv(descriptor, chunk.data(), chunk.size(), 0);
        if (received <= 0) {
            return false;
        }
        buffer.insert(buffer.end(), chunk.begin(), chunk.begin() + received);
    }
}

[[nodiscard]] bool send_one_message(int descriptor,
                                    std::span<std::uint8_t const> payload)
{
    std::vector<std::uint8_t> wire;
    proto::append_length_prefixed_message(payload, wire);
    std::size_t offset = 0;
    while (offset < wire.size()) {
        ssize_t const sent = ::send(descriptor, wire.data() + offset,
                                    wire.size() - offset, MSG_NOSIGNAL);
        if (sent <= 0) {
            return false;
        }
        offset += static_cast<std::size_t>(sent);
    }
    return true;
}

// NK handshake on the responder side, then one encrypted echo round trip.
[[nodiscard]] bool
serve_one_session(int descriptor,
                  crypto::x25519_public_key const &static_public,
                  crypto::x25519_secret_key const &static_secret)
{
    crypto::noise_handshake_responder responder{static_public, static_secret};
    std::vector<std::uint8_t> buffer;
    std::vector<std::uint8_t> first;
    std::vector<std::uint8_t> payload;
    if (!read_one_message(descriptor, buffer, first) ||
        !responder.read_first_message(first, payload)) {
        return false;
    }
    std::vector<std::uint8_t> second;
    if (!responder.write_second_message({}, second) ||
        !send_one_message(descriptor, second)) {
        return false;
    }
    crypto::symmetric_key send_key{};
    crypto::symmetric_key receive_key{};
    if (!responder.export_transport_keys(send_key, receive_key)) {
        return false;
    }
    crypto::noise_transport channel{send_key, receive_key};
    std::vector<std::uint8_t> sealed;
    std::vector<std::uint8_t> opened;
    if (!read_one_message(descriptor, buffer, sealed) ||
        !channel.decrypt_message(sealed, opened)) {
        return false;
    }
    std::vector<std::uint8_t> echoed;
    return channel.encrypt_message(opened, echoed) &&
           send_one_message(descriptor, echoed);
}

} // namespace

bool open_loopback_listener(int &descriptor_out, std::uint16_t &port_out)
{
    int const descriptor = ::socket(AF_INET, SOCK_STREAM, 0);
    if (descriptor < 0) {
        return false;
    }
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = 0;
    if (::inet_pton(AF_INET, "127.0.0.1", &address.sin_addr) != 1 ||
        ::bind(descriptor, reinterpret_cast<sockaddr const *>(&address),
               sizeof(address)) != 0 ||
        ::listen(descriptor, 4) != 0) {
        ::close(descriptor);
        return false;
    }
    sockaddr_in assigned{};
    socklen_t length = sizeof(assigned);
    if (::getsockname(descriptor, reinterpret_cast<sockaddr *>(&assigned),
                      &length) != 0) {
        ::close(descriptor);
        return false;
    }
    apply_accept_timeout(descriptor);
    port_out = ::ntohs(assigned.sin_port);
    descriptor_out = descriptor;
    return true;
}

void serve_noise_sessions(int listener_descriptor,
                          crypto::x25519_public_key const &static_public,
                          crypto::x25519_secret_key const &static_secret,
                          int session_count, loopback_server_result &result)
{
    for (int index = 0; index < session_count; ++index) {
        int const accepted = ::accept(listener_descriptor, nullptr, nullptr);
        if (accepted < 0) {
            result.every_session_succeeded = false;
            break;
        }
        if (!serve_one_session(accepted, static_public, static_secret)) {
            result.every_session_succeeded = false;
        }
        // Closed via FIN, with no goodbye message: that's exactly what the
        // real server does, and what the client needs to be able to handle.
        ::close(accepted);
        ++result.sessions_served;
    }
    ::close(listener_descriptor);
}

} // namespace hypercom::tests
