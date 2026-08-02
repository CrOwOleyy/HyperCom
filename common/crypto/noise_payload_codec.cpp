#include "common/crypto/noise_payload_codec.hpp"

namespace hypercom::crypto {

bool encrypt_and_hash(noise_symmetric_state &state,
                      std::span<std::uint8_t const> plaintext,
                      std::vector<std::uint8_t> &out)
{
    // La donnee associee est le hachage AVANT mixage du chiffre : l'ordre des
    // deux operations n'est pas interchangeable.
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
    // C'est bien le chiffre qui est mixe, jamais le clair -- sans quoi les deux
    // pairs n'obtiendraient pas le meme hachage.
    mix_hash(state, ciphertext);
    return true;
}

} // namespace hypercom::crypto
