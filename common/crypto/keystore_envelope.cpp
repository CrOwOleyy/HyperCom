#include "common/crypto/keystore_envelope.hpp"

#include <algorithm>
#include <array>

#include <sodium.h>

#include "common/crypto/secure_memory.hpp"
#include "common/crypto/sodium_runtime.hpp"

namespace hypercom::crypto {
namespace {

constexpr std::array<std::uint8_t, KEYSTORE_MAGIC_SIZE> KEYSTORE_MAGIC{
    'H', 'Y', 'P', 'C', 'K', 'E', 'Y', '1'};
constexpr std::size_t KEYSTORE_HEADER_SIZE = KEYSTORE_MAGIC_SIZE + 1 + 4 + 4
                                             + ARGON2ID_SALT_SIZE
                                             + XCHACHA_NONCE_SIZE;

void append_u32(std::uint32_t value, std::vector<std::uint8_t> &out)
{
    for (std::size_t index = 0; index < 4; ++index) {
        out.push_back(static_cast<std::uint8_t>((value >> (index * 8U)) & 0xFFU));
    }
}

[[nodiscard]] std::uint32_t read_u32(std::span<std::uint8_t const> source,
                                     std::size_t offset)
{
    std::uint32_t value = 0;
    for (std::size_t index = 0; index < 4; ++index) {
        value |= static_cast<std::uint32_t>(source[offset + index])
                 << (index * 8U);
    }
    return value;
}

[[nodiscard]] bool derive_wrapping_key(std::string_view passphrase,
                                       std::span<std::uint8_t const> salt,
                                       std::uint32_t operations,
                                       std::uint32_t memory_kib,
                                       symmetric_key &out)
{
    return crypto_pwhash(out.data(), out.size(), passphrase.data(),
                         passphrase.size(), salt.data(),
                         static_cast<unsigned long long>(operations),
                         static_cast<std::size_t>(memory_kib) * 1024U,
                         crypto_pwhash_ALG_ARGON2ID13)
        == 0;
}

} // namespace

bool seal_blob(std::string_view passphrase,
               std::span<std::uint8_t const> plaintext,
               std::vector<std::uint8_t> &out)
{
    auto const operations =
        static_cast<std::uint32_t>(crypto_pwhash_OPSLIMIT_MODERATE);
    auto const memory_kib =
        static_cast<std::uint32_t>(crypto_pwhash_MEMLIMIT_MODERATE / 1024U);
    std::array<std::uint8_t, ARGON2ID_SALT_SIZE> salt{};
    std::array<std::uint8_t, XCHACHA_NONCE_SIZE> nonce{};
    fill_random_bytes(salt);
    fill_random_bytes(nonce);
    symmetric_key wrapping_key{};
    if (!derive_wrapping_key(passphrase, salt, operations, memory_kib,
                             wrapping_key)) {
        wipe_bytes(wrapping_key);
        return false;
    }
    std::vector<std::uint8_t> header;
    header.reserve(KEYSTORE_HEADER_SIZE);
    header.insert(header.end(), KEYSTORE_MAGIC.begin(), KEYSTORE_MAGIC.end());
    header.push_back(KEYSTORE_VERSION);
    append_u32(operations, header);
    append_u32(memory_kib, header);
    header.insert(header.end(), salt.begin(), salt.end());
    header.insert(header.end(), nonce.begin(), nonce.end());
    std::vector<std::uint8_t> sealed(plaintext.size() + AEAD_TAG_SIZE);
    unsigned long long written = 0;
    // L'en-tete entier sert de donnee associee : personne ne peut abaisser le
    // cout Argon2id d'un fichier existant pour le rendre attaquable.
    int const status = crypto_aead_xchacha20poly1305_ietf_encrypt(
        sealed.data(), &written, plaintext.data(), plaintext.size(),
        header.data(), header.size(), nullptr, nonce.data(),
        wrapping_key.data());
    wipe_bytes(wrapping_key);
    if (status != 0) {
        return false;
    }
    sealed.resize(static_cast<std::size_t>(written));
    out = std::move(header);
    out.insert(out.end(), sealed.begin(), sealed.end());
    return true;
}

bool open_blob(std::string_view passphrase,
               std::span<std::uint8_t const> sealed,
               std::vector<std::uint8_t> &out)
{
    if (sealed.size() < KEYSTORE_HEADER_SIZE + AEAD_TAG_SIZE) {
        return false;
    }
    if (!std::equal(KEYSTORE_MAGIC.begin(), KEYSTORE_MAGIC.end(),
                    sealed.begin())) {
        return false;
    }
    if (sealed[KEYSTORE_MAGIC_SIZE] != KEYSTORE_VERSION) {
        return false;
    }
    std::uint32_t const operations = read_u32(sealed, KEYSTORE_MAGIC_SIZE + 1);
    std::uint32_t const memory_kib = read_u32(sealed, KEYSTORE_MAGIC_SIZE + 5);
    auto const salt = sealed.subspan(KEYSTORE_MAGIC_SIZE + 9,
                                     ARGON2ID_SALT_SIZE);
    auto const nonce = sealed.subspan(KEYSTORE_MAGIC_SIZE + 9
                                          + ARGON2ID_SALT_SIZE,
                                      XCHACHA_NONCE_SIZE);
    symmetric_key wrapping_key{};
    if (!derive_wrapping_key(passphrase, salt, operations, memory_kib,
                             wrapping_key)) {
        wipe_bytes(wrapping_key);
        return false;
    }
    auto const header = sealed.first(KEYSTORE_HEADER_SIZE);
    auto const body = sealed.subspan(KEYSTORE_HEADER_SIZE);
    out.assign(body.size() - AEAD_TAG_SIZE, 0);
    unsigned long long written = 0;
    int const status = crypto_aead_xchacha20poly1305_ietf_decrypt(
        out.data(), &written, nullptr, body.data(), body.size(), header.data(),
        header.size(), nonce.data(), wrapping_key.data());
    wipe_bytes(wrapping_key);
    if (status != 0) {
        out.clear();
        return false;
    }
    out.resize(static_cast<std::size_t>(written));
    return true;
}

bool seal_identity_secret(std::string_view passphrase,
                          ed25519_secret_key const &secret,
                          std::vector<std::uint8_t> &out)
{
    return seal_blob(passphrase, secret, out);
}

bool open_identity_secret(std::string_view passphrase,
                          std::span<std::uint8_t const> sealed,
                          ed25519_secret_key &out)
{
    std::vector<std::uint8_t> plaintext;
    if (!open_blob(passphrase, sealed, plaintext)) {
        return false;
    }
    // Une taille inattendue signale un fichier d'un autre type scelle avec la
    // meme passphrase : on refuse plutot que de recopier ce qui tient.
    bool const usable = plaintext.size() == out.size();
    if (usable) {
        std::copy(plaintext.begin(), plaintext.end(), out.begin());
    }
    wipe_bytes(plaintext);
    return usable;
}

} // namespace hypercom::crypto
