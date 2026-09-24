#pragma once

#include "common/crypto/key_types.hpp"

#include <cstdint>

namespace hypercom::tests {

// Minimal Noise responder on the loopback interface.
//
// It doesn't speak the application protocol: it establishes the channel,
// echoes back one encrypted frame, then closes. That's all it takes to
// verify that a session opens -- and, more importantly, that it REOPENS on
// the same client object, which no test used to cover.
//
// POSIX only: the project's server is developed and tested under WSL2, and
// duplicating this file for Winsock wouldn't buy us anything.

struct loopback_server_result {
    int sessions_served = 0;
    bool every_session_succeeded = true;
};

// Opens and listens on a socket on 127.0.0.1, with a port assigned by the OS.
[[nodiscard]] bool open_loopback_listener(int &descriptor_out,
                                          std::uint16_t &port_out);

// Serves session_count successive sessions, then closes the listening
// socket. Meant to run in a thread while the client connects.
void serve_noise_sessions(int listener_descriptor,
                          crypto::x25519_public_key const &static_public,
                          crypto::x25519_secret_key const &static_secret,
                          int session_count, loopback_server_result &result);

} // namespace hypercom::tests
