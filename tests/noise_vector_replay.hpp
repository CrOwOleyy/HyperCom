#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <string_view>
#include <vector>

#include "common/crypto/key_types.hpp"
#include "common/crypto/noise_symmetric_state.hpp"
#include "common/util/hex_codec.hpp"
#include "tests/test_harness.hpp"

namespace hypercom::tests {

// Rejoue a la main un handshake Noise_NK avec des cles et un prologue fixes,
// pour le comparer octet par octet a un vecteur de test officiel.
//
// noise_handshake_initiator/responder ne conviennent pas pour ca : ils tirent
// leurs cles ephemeres au hasard, et un vecteur impose des cles fixes. Ces
// fonctions appellent donc directement les memes primitives (mix_hash,
// mix_key, encrypt_and_hash, split_transport_keys) dans le meme ordre que ces
// classes, avec les cles imposees a la place de cles aleatoires.

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

// Cles du vecteur decodees. resp_static_public est deja verifiee contre la
// reference : si compute_public_from_secret divergeait, autant l'apprendre
// ici plutot que de le deduire d'un echec de handshake plus loin.
struct loaded_vector_keys {
    crypto::x25519_secret_key init_ephemeral_secret{};
    crypto::x25519_secret_key resp_static_secret{};
    crypto::x25519_secret_key resp_ephemeral_secret{};
    crypto::x25519_public_key resp_static_public{};
    crypto::symmetric_key expected_handshake_hash{};
    std::vector<std::uint8_t> prologue;
};

[[nodiscard]] loaded_vector_keys load_vector_keys(test_report &report);

// Les deux etats symetriques une fois le handshake termine, prets pour
// Split(). Garder les deux plutot qu'un seul permet de verifier qu'ils
// convergent vers le meme hachage.
struct handshake_result {
    crypto::noise_symmetric_state initiator;
    crypto::noise_symmetric_state responder;
};

[[nodiscard]] handshake_result run_handshake(test_report &report,
                                             loaded_vector_keys const &keys);

// Split() puis les quatre messages de transport du vecteur, en alternance
// I -> R -> I -> R.
void check_transport(test_report &report, handshake_result const &handshake);

} // namespace hypercom::tests
