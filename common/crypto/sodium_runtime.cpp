#include "common/crypto/sodium_runtime.hpp"

#include <sodium.h>

#include "common/crypto/key_types.hpp"

namespace hypercom::crypto {

// Verrou de compilation : si une version de libsodium changeait une taille,
// mieux vaut une erreur de compilation qu'un tampon trop court a l'execution.
static_assert(ED25519_PUBLIC_KEY_SIZE == crypto_sign_PUBLICKEYBYTES);
static_assert(ED25519_SECRET_KEY_SIZE == crypto_sign_SECRETKEYBYTES);
static_assert(ED25519_SIGNATURE_SIZE == crypto_sign_BYTES);
static_assert(ED25519_SEED_SIZE == crypto_sign_SEEDBYTES);
static_assert(X25519_PUBLIC_KEY_SIZE == crypto_scalarmult_BYTES);
static_assert(X25519_SECRET_KEY_SIZE == crypto_scalarmult_SCALARBYTES);
static_assert(SYMMETRIC_KEY_SIZE == crypto_auth_hmacsha256_KEYBYTES);
static_assert(CHACHA_IETF_NONCE_SIZE
              == crypto_aead_chacha20poly1305_ietf_NPUBBYTES);
static_assert(XCHACHA_NONCE_SIZE
              == crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);
static_assert(AEAD_TAG_SIZE == crypto_aead_chacha20poly1305_ietf_ABYTES);
static_assert(ARGON2ID_SALT_SIZE == crypto_pwhash_SALTBYTES);

bool initialize_sodium()
{
    // sodium_init() rend 0 au premier appel, 1 s'il a deja eu lieu, -1 en cas
    // d'echec. Les deux premiers cas sont des succes.
    return sodium_init() >= 0;
}

void fill_random_bytes(std::span<std::uint8_t> destination)
{
    if (destination.empty()) {
        return;
    }
    randombytes_buf(destination.data(), destination.size());
}

} // namespace hypercom::crypto
