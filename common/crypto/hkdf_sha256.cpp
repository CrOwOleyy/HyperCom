#include "common/crypto/hkdf_sha256.hpp"

#include <algorithm>
#include <array>

#include <sodium.h>

#include "common/crypto/secure_memory.hpp"

namespace hypercom::crypto {
namespace {

// HMAC-SHA256 dont la cle fait toujours 32 octets dans ce projet, ce qui
// correspond exactement a crypto_auth_hmacsha256_KEYBYTES.
[[nodiscard]] bool compute_hmac(symmetric_key const &key,
                                std::span<std::uint8_t const> first_part,
                                std::span<std::uint8_t const> second_part,
                                symmetric_key &out)
{
    crypto_auth_hmacsha256_state state;
    if (crypto_auth_hmacsha256_init(&state, key.data(), key.size()) != 0) {
        return false;
    }
    if (!first_part.empty()) {
        crypto_auth_hmacsha256_update(&state, first_part.data(),
                                      first_part.size());
    }
    if (!second_part.empty()) {
        crypto_auth_hmacsha256_update(&state, second_part.data(),
                                      second_part.size());
    }
    return crypto_auth_hmacsha256_final(&state, out.data()) == 0;
}

} // namespace

bool extract_pseudo_random_key(
    std::span<std::uint8_t const> salt,
    std::span<std::uint8_t const> input_key_material, symmetric_key &out)
{
    // RFC 5869 : le sel joue le role de cle HMAC a l'extraction. Un sel absent
    // vaut 32 octets nuls, ce que produit deja l'initialisation du tableau.
    symmetric_key salt_key{};
    if (salt.size() > salt_key.size()) {
        return false;
    }
    std::copy(salt.begin(), salt.end(), salt_key.begin());
    bool const succeeded = compute_hmac(salt_key, input_key_material, {}, out);
    wipe_bytes(salt_key);
    return succeeded;
}

bool expand_key_block(symmetric_key const &pseudo_random_key,
                      std::span<std::uint8_t const> info,
                      std::uint8_t block_index, symmetric_key &out)
{
    std::array<std::uint8_t, 1> const counter{block_index};
    return compute_hmac(pseudo_random_key, info, counter, out);
}

bool derive_key_pair(symmetric_key const &chaining_key,
                     std::span<std::uint8_t const> input_key_material,
                     symmetric_key &first, symmetric_key &second)
{
    symmetric_key temporary_key{};
    if (!compute_hmac(chaining_key, input_key_material, {}, temporary_key)) {
        return false;
    }
    bool succeeded = expand_key_block(temporary_key, {}, 0x01, first);
    if (succeeded) {
        // Le second bloc chaine sur le premier, conformement a HKDF-Expand.
        succeeded = expand_key_block(temporary_key, first, 0x02, second);
    }
    wipe_bytes(temporary_key);
    return succeeded;
}

} // namespace hypercom::crypto
