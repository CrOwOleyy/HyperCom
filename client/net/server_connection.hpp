#pragma once

#include "client/net/tcp_client_socket.hpp"
#include "common/crypto/noise_handshake_initiator.hpp"
#include "common/crypto/noise_transport.hpp"
#include "common/protocol/frame_codec.hpp"

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace hypercom::client {

// Where to reach the server, and by what path.
//
// Empty socks5_host = direct connection. When filled in, the socket opens
// toward the proxy, which then relays to host:port -- it's the only way
// to reach a hidden .onion service. The Noise handshake proceeds
// identically in both cases: it has no idea what path its bytes take, so
// server key pinning protects exactly the same either way.
struct server_endpoint {
    std::string host;
    std::uint16_t port = 0;
    std::string socks5_host;
    std::uint16_t socks5_port = 0;
};

// Connection to the server: TCP, then a Noise_NK handshake, then
// encrypted frames.
//
// The server's static key is supplied at construction time: that's the
// pinning. It must come from a trusted channel -- displayed by the
// administrator, handed over in person -- and definitely not from the
// server itself, or pinning protects nothing.
//
// A substituted server will fail to decrypt the handshake's second
// message, without any secret ever being revealed.
class server_connection {
public:
    explicit server_connection(
        crypto::x25519_public_key const &server_static_public);

    // Also serves as reconnection: calling open_session again on a
    // dropped connection starts over with a full Noise handshake and a
    // fresh ephemeral key. No resumption token is kept from one session
    // to the next, so the server has nothing that would let it stitch two
    // connections together. The accepted tradeoff is having to redo the
    // challenge-response.
    [[nodiscard]] bool open_session(server_endpoint const &endpoint,
                                    std::string &error_out);

    [[nodiscard]] bool send_frame(proto::message_type type,
                                  std::span<std::uint8_t const> payload);

    [[nodiscard]] bool receive_frame(proto::frame_header &header,
                                     std::vector<std::uint8_t> &payload,
                                     std::string &error_out);

    [[nodiscard]] bool is_open() const;

private:
    [[nodiscard]] bool perform_handshake(std::string &error_out);

    [[nodiscard]] bool pull_next_message(std::vector<std::uint8_t> &out,
                                         std::string &error_out);

    tcp_client_socket socket_;
    crypto::x25519_public_key server_static_public_;
    crypto::noise_handshake_initiator handshake_;
    std::optional<crypto::noise_transport> transport_;
    std::vector<std::uint8_t> input_buffer_;
    bool open_;
};

} // namespace hypercom::client
