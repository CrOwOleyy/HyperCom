#include "client/keystore/master_seed_store.hpp"
#include "common/crypto/recovery_phrase.hpp"
#include "common/crypto/recovery_wordlist.hpp"
#include "common/crypto/sodium_runtime.hpp"
#include "common/util/hex_codec.hpp"
#include "tests/test_harness.hpp"

#include <algorithm>
#include <cstdio>
#include <sodium.h>
#include <string>
#include <vector>

namespace {

using namespace hypercom;

[[nodiscard]] crypto::recovery_entropy filled(std::uint8_t value)
{
    crypto::recovery_entropy entropy{};
    entropy.fill(value);
    return entropy;
}

// The list is part of the format: one wrong word changes every phrase.
void check_wordlist_integrity(tests::test_report &report)
{
    std::string joined;
    for (std::string_view const word : crypto::RECOVERY_WORDLIST) {
        joined.append(word);
        joined.push_back('\n');
    }
    std::array<std::uint8_t, crypto_hash_sha256_BYTES> digest{};
    crypto_hash_sha256(digest.data(),
                       reinterpret_cast<unsigned char const *>(joined.data()),
                       joined.size());
    std::string encoded;
    util::encode_hex(digest, encoded);
    // SHA-256 of the canonical BIP-39 english.txt.
    HYPERCOM_CHECK(report,
                   encoded == "2f5eed53a4727b4bf8880d8f3f199efc90e58503646d9ff8"
                              "eff3a2ed3b24dbda");
}

// Official BIP-39 test vectors (128-bit entropy).
void check_known_vectors(tests::test_report &report)
{
    struct vector_case {
        std::uint8_t filler;
        char const *phrase;
    };
    vector_case const cases[] = {
        {0x00, "abandon abandon abandon abandon abandon abandon abandon "
               "abandon abandon abandon abandon about"},
        {0x7f, "legal winner thank year wave sausage worth useful legal "
               "winner thank yellow"},
        {0x80, "letter advice cage absurd amount doctor acoustic avoid letter "
               "advice cage above"},
        {0xff, "zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo wrong"},
    };
    for (vector_case const &entry : cases) {
        std::string phrase;
        HYPERCOM_CHECK(report, crypto::entropy_to_phrase(filled(entry.filler),
                                                         phrase));
        HYPERCOM_CHECK(report, phrase == entry.phrase);
        crypto::recovery_entropy decoded{};
        HYPERCOM_CHECK(report,
                       crypto::phrase_to_entropy(entry.phrase, decoded));
        HYPERCOM_CHECK(report, decoded == filled(entry.filler));
    }
}

void check_round_trip_and_leniency(tests::test_report &report)
{
    for (int attempt = 0; attempt < 50; ++attempt) {
        crypto::recovery_entropy entropy{};
        crypto::generate_recovery_entropy(entropy);
        std::string phrase;
        crypto::recovery_entropy decoded{};
        HYPERCOM_CHECK(report, crypto::entropy_to_phrase(entropy, phrase));
        HYPERCOM_CHECK(report, crypto::phrase_to_entropy(phrase, decoded));
        HYPERCOM_CHECK(report, decoded == entropy);
    }
    crypto::recovery_entropy decoded{};
    // Extra spaces, tabs and capitals are what a human actually types.
    HYPERCOM_CHECK(
        report,
        crypto::phrase_to_entropy(
            "  ZOO zoo\tzoo zoo  zoo zoo zoo zoo zoo zoo zoo WRONG \n",
            decoded));
    HYPERCOM_CHECK(report, decoded == filled(0xff));
}

void check_rejections(tests::test_report &report)
{
    crypto::recovery_entropy decoded = filled(0x42);
    // Bad checksum: last word swapped.
    HYPERCOM_CHECK(report,
                   !crypto::phrase_to_entropy(
                       "zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo",
                       decoded));
    // Unknown word.
    HYPERCOM_CHECK(report,
                   !crypto::phrase_to_entropy(
                       "zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo hypercom",
                       decoded));
    // Wrong word counts.
    HYPERCOM_CHECK(report, !crypto::phrase_to_entropy("", decoded));
    HYPERCOM_CHECK(report,
                   !crypto::phrase_to_entropy(
                       "zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo wrong",
                       decoded));
    HYPERCOM_CHECK(report,
                   !crypto::phrase_to_entropy(
                       "zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo wrong",
                       decoded));
    // A failure must not write the output.
    HYPERCOM_CHECK(report, decoded == filled(0x42));
}

void check_seed_derivation(tests::test_report &report)
{
    crypto::ed25519_seed first{};
    crypto::ed25519_seed again{};
    crypto::ed25519_seed other{};
    HYPERCOM_CHECK(report, crypto::derive_master_seed(filled(0x01), first));
    HYPERCOM_CHECK(report, crypto::derive_master_seed(filled(0x01), again));
    HYPERCOM_CHECK(report, crypto::derive_master_seed(filled(0x02), other));
    HYPERCOM_CHECK(report, first == again);
    HYPERCOM_CHECK(report, first != other);
    // The seed is not the entropy itself.
    crypto::recovery_entropy const entropy = filled(0x01);
    HYPERCOM_CHECK(report,
                   !std::equal(entropy.begin(), entropy.end(), first.begin()));
}

// The point of the whole feature: a new machine, with only the phrase,
// ends up with the same seed -- and therefore the same identities.
void check_store_recovery(tests::test_report &report)
{
    std::string const original_path = "recovery_test_original.key";
    std::string const restored_path = "recovery_test_restored.key";
    std::remove(original_path.c_str());
    std::remove(restored_path.c_str());
    std::string failure;
    client::master_seed_store original{original_path};
    crypto::ed25519_seed created{};
    std::string phrase;
    HYPERCOM_CHECK(report,
                   original.create_seed("passe-a", created, phrase, failure));
    // Refuses to overwrite an existing seed.
    crypto::ed25519_seed ignored{};
    std::string ignored_phrase;
    HYPERCOM_CHECK(report,
                   !original.create_seed("passe-a", ignored, ignored_phrase,
                                         failure));
    client::master_seed_store restored{restored_path};
    crypto::ed25519_seed recovered{};
    // A mistyped phrase writes nothing.
    HYPERCOM_CHECK(report,
                   !restored.restore_seed(
                       "passe-b",
                       "zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo",
                       recovered, failure));
    HYPERCOM_CHECK(report, !restored.has_stored_seed());
    // A different passphrase on the new machine is fine: only the seed has
    // to match.
    HYPERCOM_CHECK(report,
                   restored.restore_seed("passe-b", phrase, recovered,
                                         failure));
    HYPERCOM_CHECK(report, recovered == created);
    crypto::ed25519_seed reopened{};
    HYPERCOM_CHECK(report, restored.unlock_seed("passe-b", reopened, failure));
    HYPERCOM_CHECK(report, reopened == created);
    // Restoring over an existing seed is refused.
    HYPERCOM_CHECK(report,
                   !restored.restore_seed("passe-b", phrase, recovered,
                                          failure));
    std::remove(original_path.c_str());
    std::remove(restored_path.c_str());
}

} // namespace

int main()
{
    hypercom::tests::test_report report;
    if (!hypercom::crypto::initialize_sodium()) {
        return 1;
    }
    check_wordlist_integrity(report);
    check_known_vectors(report);
    check_round_trip_and_leniency(report);
    check_rejections(report);
    check_seed_derivation(report);
    check_store_recovery(report);
    return report.summarize("recovery_phrase_test");
}
