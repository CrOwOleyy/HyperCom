#include "common/crypto/recovery_phrase.hpp"

#include "common/crypto/hkdf_sha256.hpp"
#include "common/crypto/recovery_wordlist.hpp"
#include "common/crypto/secure_memory.hpp"
#include "common/crypto/sodium_runtime.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <sodium.h>
#include <vector>

namespace hypercom::crypto {
namespace {

constexpr std::size_t BITS_PER_WORD = 11;
constexpr std::size_t CHECKSUM_BITS = 4;
constexpr std::size_t ENTROPY_BITS = RECOVERY_ENTROPY_SIZE * 8;
static_assert(ENTROPY_BITS + CHECKSUM_BITS ==
                  RECOVERY_WORD_COUNT * BITS_PER_WORD,
              "128 bits of entropy + 4 bits of checksum = 12 words of 11 bits");

// Entropy followed by one byte whose top 4 bits are the checksum.
using packed_bits = std::array<std::uint8_t, RECOVERY_ENTROPY_SIZE + 1>;

[[nodiscard]] std::uint8_t checksum_nibble(recovery_entropy const &entropy)
{
    std::array<std::uint8_t, crypto_hash_sha256_BYTES> digest{};
    crypto_hash_sha256(digest.data(), entropy.data(), entropy.size());
    return static_cast<std::uint8_t>(digest[0] & 0xF0U);
}

[[nodiscard]] unsigned bit_at(packed_bits const &bits, std::size_t index)
{
    return (bits[index / 8] >> (7 - index % 8)) & 1U;
}

[[nodiscard]] std::string normalize_word(std::string_view word)
{
    std::string lowered;
    lowered.reserve(word.size());
    for (char const letter : word) {
        lowered.push_back(static_cast<char>(
            std::tolower(static_cast<unsigned char>(letter))));
    }
    return lowered;
}

// The list is sorted, which is part of the BIP-39 specification.
[[nodiscard]] bool find_word_index(std::string_view word, unsigned &out)
{
    auto const found = std::lower_bound(RECOVERY_WORDLIST.begin(),
                                        RECOVERY_WORDLIST.end(), word);
    if (found == RECOVERY_WORDLIST.end() || *found != word) {
        return false;
    }
    out = static_cast<unsigned>(found - RECOVERY_WORDLIST.begin());
    return true;
}

// Splits on any ASCII whitespace; false if there are more than 12 words.
[[nodiscard]] bool split_words(std::string_view phrase,
                               std::vector<std::string> &out)
{
    std::size_t position = 0;
    while (position < phrase.size()) {
        while (position < phrase.size() &&
               std::isspace(static_cast<unsigned char>(phrase[position]))) {
            ++position;
        }
        std::size_t const start = position;
        while (position < phrase.size() &&
               !std::isspace(static_cast<unsigned char>(phrase[position]))) {
            ++position;
        }
        if (position > start) {
            if (out.size() == RECOVERY_WORD_COUNT) {
                return false;
            }
            out.push_back(
                normalize_word(phrase.substr(start, position - start)));
        }
    }
    return out.size() == RECOVERY_WORD_COUNT;
}

} // namespace

void generate_recovery_entropy(recovery_entropy &out)
{
    fill_random_bytes(out);
}

bool entropy_to_phrase(recovery_entropy const &entropy, std::string &out)
{
    packed_bits bits{};
    std::copy(entropy.begin(), entropy.end(), bits.begin());
    bits[RECOVERY_ENTROPY_SIZE] = checksum_nibble(entropy);
    std::string phrase;
    for (std::size_t word = 0; word < RECOVERY_WORD_COUNT; ++word) {
        unsigned index = 0;
        for (std::size_t bit = 0; bit < BITS_PER_WORD; ++bit) {
            index = (index << 1U) | bit_at(bits, word * BITS_PER_WORD + bit);
        }
        if (word != 0) {
            phrase.push_back(' ');
        }
        phrase.append(RECOVERY_WORDLIST[index]);
    }
    wipe_bytes(bits);
    out = std::move(phrase);
    return true;
}

bool phrase_to_entropy(std::string_view phrase, recovery_entropy &out)
{
    std::vector<std::string> words;
    if (!split_words(phrase, words)) {
        return false;
    }
    packed_bits bits{};
    for (std::size_t word = 0; word < RECOVERY_WORD_COUNT; ++word) {
        unsigned index = 0;
        if (!find_word_index(words[word], index)) {
            wipe_bytes(bits);
            return false;
        }
        for (std::size_t bit = 0; bit < BITS_PER_WORD; ++bit) {
            unsigned const value = (index >> (BITS_PER_WORD - 1 - bit)) & 1U;
            std::size_t const position = word * BITS_PER_WORD + bit;
            bits[position / 8] = static_cast<std::uint8_t>(
                bits[position / 8] | (value << (7 - position % 8)));
        }
    }
    recovery_entropy candidate{};
    std::copy_n(bits.begin(), candidate.size(), candidate.begin());
    bool const valid = (bits[RECOVERY_ENTROPY_SIZE] & 0xF0U) ==
                       checksum_nibble(candidate);
    if (valid) {
        out = candidate;
    }
    wipe_bytes(candidate);
    wipe_bytes(bits);
    return valid;
}

bool derive_master_seed(recovery_entropy const &entropy, ed25519_seed &out)
{
    // Domain separation: the same 16 bytes never serve any other purpose.
    constexpr std::array<std::uint8_t, 20> SALT = {
        'h', 'y', 'p', 'e', 'r', 'c', 'o', 'm', '-', 'r',
        'e', 'c', 'o', 'v', 'e', 'r', 'y', '-', 'v', '1'};
    constexpr std::array<std::uint8_t, 20> INFO = {
        'h', 'y', 'p', 'e', 'r', 'c', 'o', 'm', '-', 'm',
        'a', 's', 't', 'e', 'r', '-', 's', 'e', 'e', 'd'};
    symmetric_key pseudo_random_key{};
    symmetric_key derived{};
    bool const succeeded =
        extract_pseudo_random_key(SALT, entropy, pseudo_random_key) &&
        expand_key_block(pseudo_random_key, INFO, 1, derived);
    if (succeeded) {
        std::copy(derived.begin(), derived.end(), out.begin());
    }
    wipe_bytes(pseudo_random_key);
    wipe_bytes(derived);
    return succeeded;
}

} // namespace hypercom::crypto
