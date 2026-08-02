#include "common/crypto/noise_symmetric_state.hpp"

#include <algorithm>

#include <sodium.h>

#include "common/crypto/hkdf_sha256.hpp"
#include "common/crypto/secure_memory.hpp"

namespace hypercom::crypto {

void initialize_symmetric_state(std::string_view protocol_name,
                                noise_symmetric_state &out)
{
    out.handshake_hash.fill(0);
    if (protocol_name.size() <= out.handshake_hash.size()) {
        // Nom assez court : il sert directement d'etat initial, complete de
        // zeros. C'est ce que prescrit la specification, et le nom de notre
        // suite fait exactement 32 octets.
        std::copy(protocol_name.begin(), protocol_name.end(),
                  out.handshake_hash.begin());
    } else {
        crypto_hash_sha256(
            out.handshake_hash.data(),
            reinterpret_cast<unsigned char const *>(protocol_name.data()),
            protocol_name.size());
    }
    out.chaining_key = out.handshake_hash;
    out.cipher = noise_cipher_state{};
}

void mix_hash(noise_symmetric_state &state, std::span<std::uint8_t const> data)
{
    crypto_hash_sha256_state hash_state;
    crypto_hash_sha256_init(&hash_state);
    crypto_hash_sha256_update(&hash_state, state.handshake_hash.data(),
                              state.handshake_hash.size());
    if (!data.empty()) {
        crypto_hash_sha256_update(&hash_state, data.data(), data.size());
    }
    crypto_hash_sha256_final(&hash_state, state.handshake_hash.data());
}

bool mix_key(noise_symmetric_state &state,
             std::span<std::uint8_t const> input_key_material)
{
    symmetric_key next_chaining_key{};
    symmetric_key temporary_key{};
    if (!derive_key_pair(state.chaining_key, input_key_material,
                         next_chaining_key, temporary_key)) {
        return false;
    }
    state.chaining_key = next_chaining_key;
    state.cipher.initialize_key(temporary_key);
    wipe_bytes(temporary_key);
    wipe_bytes(next_chaining_key);
    return true;
}

bool split_transport_keys(noise_symmetric_state const &state,
                          symmetric_key &first, symmetric_key &second)
{
    return derive_key_pair(state.chaining_key, {}, first, second);
}

} // namespace hypercom::crypto
