#include "client/cli/invite_link.hpp"
#include "client/keystore/server_identity.hpp"
#include "client/keystore/server_registry.hpp"
#include "common/crypto/keystore_envelope.hpp"
#include "common/crypto/sodium_runtime.hpp"
#include "tests/test_harness.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace {

using namespace hypercom;

[[nodiscard]] crypto::x25519_public_key make_server_key(std::uint8_t filler)
{
    crypto::x25519_public_key key{};
    key.fill(filler);
    return key;
}

// Generic sealing must hold for arbitrary sizes, from the seed (32 bytes)
// to the registry (variable).
void check_blob_round_trip(tests::test_report &report)
{
    for (std::size_t size :
         {std::size_t{0}, std::size_t{1}, std::size_t{32}, std::size_t{1000}}) {
        std::vector<std::uint8_t> const plaintext(size, 0xA5);
        std::vector<std::uint8_t> sealed;
        std::vector<std::uint8_t> recovered;
        HYPERCOM_CHECK(report, crypto::seal_blob("passe", plaintext, sealed));
        HYPERCOM_CHECK(report, crypto::open_blob("passe", sealed, recovered));
        HYPERCOM_CHECK(report, recovered == plaintext);
    }
    std::vector<std::uint8_t> const secret(32, 0x11);
    std::vector<std::uint8_t> sealed;
    std::vector<std::uint8_t> recovered;
    HYPERCOM_CHECK(report, crypto::seal_blob("bonne", secret, sealed));
    HYPERCOM_CHECK(report, !crypto::open_blob("mauvaise", sealed, recovered));
    // A single flipped byte must make authentication fail.
    sealed[sealed.size() - 1] ^= 0x01U;
    HYPERCOM_CHECK(report, !crypto::open_blob("bonne", sealed, recovered));
}

// The property that carries the whole multi-server design: the same seed
// and the same server yield the same identity, while two different servers
// yield two identities with no visible link.
void check_identity_derivation(tests::test_report &report)
{
    crypto::ed25519_seed seed{};
    seed.fill(0x42);
    crypto::x25519_public_key const first_server = make_server_key(0x01);
    crypto::x25519_public_key const second_server = make_server_key(0x02);
    crypto::identity_keypair first;
    crypto::identity_keypair first_again;
    crypto::identity_keypair second;
    HYPERCOM_CHECK(report,
                   client::derive_server_identity(seed, first_server, first));
    HYPERCOM_CHECK(report, client::derive_server_identity(seed, first_server,
                                                          first_again));
    HYPERCOM_CHECK(report,
                   client::derive_server_identity(seed, second_server, second));
    // Deterministic: that's what makes it possible to save only the seed.
    HYPERCOM_CHECK(report,
                   first.get_public_key() == first_again.get_public_key());
    // Uncorrelatable: two administrators comparing their databases see only
    // two arbitrary public keys.
    HYPERCOM_CHECK(report, first.get_public_key() != second.get_public_key());
    crypto::ed25519_seed other_seed{};
    other_seed.fill(0x43);
    crypto::identity_keypair from_other_seed;
    HYPERCOM_CHECK(report, client::derive_server_identity(
                               other_seed, first_server, from_other_seed));
    HYPERCOM_CHECK(report,
                   first.get_public_key() != from_other_seed.get_public_key());
}

void check_invite_link(tests::test_report &report)
{
    std::string const key(64, 'a');
    client::invite_link parsed;
    std::string failure;
    HYPERCOM_CHECK(report,
                   client::parse_invite_link("hypercom://192.0.2.7:7717#" + key,
                                             parsed, failure));
    HYPERCOM_CHECK(report, parsed.host == "192.0.2.7");
    HYPERCOM_CHECK(report, parsed.port == 7717);
    HYPERCOM_CHECK(report, parsed.server_key_hex == key);
    HYPERCOM_CHECK(report, client::format_invite_link("192.0.2.7", 7717, key) ==
                               "hypercom://192.0.2.7:7717#" + key);
    // Each missing or out-of-bounds part must be rejected.
    HYPERCOM_CHECK(report, !client::parse_invite_link("http://a:1#" + key,
                                                      parsed, failure));
    HYPERCOM_CHECK(report, !client::parse_invite_link("hypercom://a:7717",
                                                      parsed, failure));
    HYPERCOM_CHECK(report, !client::parse_invite_link("hypercom://a:7717#court",
                                                      parsed, failure));
    HYPERCOM_CHECK(report, !client::parse_invite_link(
                               "hypercom://a:99999#" + key, parsed, failure));
    HYPERCOM_CHECK(report, !client::parse_invite_link("hypercom://:7717#" + key,
                                                      parsed, failure));
}

void check_registry_round_trip(tests::test_report &report)
{
    std::string const path = "test_server_registry.dat";
    client::server_registry registry{path};
    std::vector<client::server_entry> written;
    client::server_entry home;
    home.label = "maison";
    home.endpoint = {"127.0.0.1", 7717, "", 0};
    home.server_key = make_server_key(0x07);
    home.source = client::identity_source::imported;
    home.imported_identity_path = "ancienne.key";
    home.trust_acknowledged = true;
    client::server_entry hidden;
    hidden.label = "cache";
    hidden.endpoint = {"exemple.onion", 7717, "127.0.0.1", 9050};
    hidden.server_key = make_server_key(0x09);
    written.push_back(home);
    written.push_back(hidden);
    std::string failure;
    HYPERCOM_CHECK(report, registry.save("passe", written, failure));
    HYPERCOM_CHECK(report, registry.has_stored_registry());
    std::vector<client::server_entry> reread;
    HYPERCOM_CHECK(report, registry.load("passe", reread, failure));
    HYPERCOM_CHECK(report, reread.size() == written.size());
    if (reread.size() == written.size()) {
        HYPERCOM_CHECK(report, reread[0].label == "maison");
        HYPERCOM_CHECK(report, reread[0].endpoint.port == 7717);
        HYPERCOM_CHECK(report, reread[0].server_key == home.server_key);
        HYPERCOM_CHECK(report,
                       reread[0].source == client::identity_source::imported);
        HYPERCOM_CHECK(report,
                       reread[0].imported_identity_path == "ancienne.key");
        HYPERCOM_CHECK(report, reread[0].trust_acknowledged);
        HYPERCOM_CHECK(report, reread[1].endpoint.host == "exemple.onion");
        HYPERCOM_CHECK(report, reread[1].endpoint.socks5_port == 9050);
        HYPERCOM_CHECK(report, !reread[1].trust_acknowledged);
    }
    // The server list reveals memberships: it must not open without the
    // right passphrase.
    std::vector<client::server_entry> refused;
    HYPERCOM_CHECK(report, !registry.load("mauvaise", refused, failure));
    std::remove(path.c_str());
}

} // namespace

int main()
{
    hypercom::tests::test_report report;
    if (!hypercom::crypto::initialize_sodium()) {
        return 1;
    }
    check_blob_round_trip(report);
    check_identity_derivation(report);
    check_invite_link(report);
    check_registry_round_trip(report);
    return report.summarize("multi_server_identity_test");
}
