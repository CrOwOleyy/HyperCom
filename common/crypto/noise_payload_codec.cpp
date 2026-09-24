#include "common/crypto/noise_payload_codec.hpp"

namespace hypercom::crypto {

bool encrypt_and_hash(noise_symmetric_state &state,
                      std::span<std::uint8_t const> plaintext,
                      std::vector<std::uint8_t> &out)
{
    // The associated data is the hash BEFORE the ciphertext is mixed in: the
    // order of the two operations is not interchangeable.
    if (!state.cipher.encrypt_with_ad(state.handshake_hash, plaintext, out)) {
        return false;
    }
    mix_hash(state, out);
    return true;
}

bool decrypt_and_hash(noise_symmetric_state &state,
                      std::span<std::uint8_t const> ciphertext,
                      std::vector<std::uint8_t> &out)
{
    if (!state.cipher.decrypt_with_ad(state.handshake_hash, ciphertext, out)) {
        return false;
    }
    // It really is the ciphertext that gets mixed in, never the plaintext --
    // otherwise the two peers would not end up with the same hash.
    mix_hash(state, ciphertext);
    return true;
}

} // namespace hypercom::crypto
