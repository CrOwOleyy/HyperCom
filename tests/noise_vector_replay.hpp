#pragma once

#include "common/crypto/key_types.hpp"
#include "common/crypto/noise_symmetric_state.hpp"
#include "common/util/hex_codec.hpp"
#include "tests/test_harness.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <string_view>
#include <vector>

namespace hypercom::tests {

// Manually replays a Noise_NK handshake with fixed keys and a fixed
// prologue, to compare it byte for byte against an official test vector.
//
// noise_handshake_initiator/responder don't work for this: they draw their
// ephemeral keys at random, while a vector imposes fixed keys. So these
// functions call the same primitives directly (mix_hash, mix_key,
// encrypt_and_hash, split_transport_keys) in the same order as those
// classes, with the imposed keys in place of random ones.

template <std::size_t N>
void load_fixed(test_report &report, std::string_view hex,
                std::array<std::uint8_t, N> &out)
{
    std::vector<std::uint8_t> decoded;
    bool const ok = util::decode_hex(hex, decoded) && decoded.size() == N;
    HYPERCOM_CHECK(report, ok);
    if (ok) {
        std::copy(decoded.begin(), decoded.end(), out.begin());
    }
}

[[nodiscard]] std::vector<std::uint8_t> load_bytes(test_report &report,
                                                   std::string_view hex);

// Decoded vector keys. resp_static_public is already checked against the
// reference: if compute_public_from_secret diverged, better to learn it
// here than to infer it from a handshake failure further down.
struct loaded_vector_keys {
    crypto::x25519_secret_key init_ephemeral_secret{};
    crypto::x25519_secret_key resp_static_secret{};
    crypto::x25519_secret_key resp_ephemeral_secret{};
    crypto::x25519_public_key resp_static_public{};
    crypto::symmetric_key expected_handshake_hash{};
    std::vector<std::uint8_t> prologue;
};

[[nodiscard]] loaded_vector_keys load_vector_keys(test_report &report);

// Both symmetric states once the handshake is done, ready for Split().
// Keeping both rather than just one makes it possible to check that they
// converge to the same hash.
struct handshake_result {
    crypto::noise_symmetric_state initiator;
    crypto::noise_symmetric_state responder;
};

[[nodiscard]] handshake_result run_handshake(test_report &report,
                                             loaded_vector_keys const &keys);

// Split() then the vector's four transport messages, alternating
// I -> R -> I -> R.
void check_transport(test_report &report, handshake_result const &handshake);

} // namespace hypercom::tests
