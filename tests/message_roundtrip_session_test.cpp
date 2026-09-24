#include "common/protocol/auth_message.hpp"
#include "common/protocol/hello_message.hpp"
#include "common/protocol/motd_message.hpp"
#include "common/protocol/ping_message.hpp"
#include "common/protocol/prekey_publish_message.hpp"
#include "common/protocol/status_message.hpp"
#include "tests/test_harness.hpp"

#include <cstdint>
#include <span>
#include <string>
#include <vector>

// Round trip for session and account messages.
//
// An encoder and a decoder that diverge on a single field produce a silent
// misalignment: the following fields shift, yet the message still reads as
// "valid". So we also check that the reader ends with zero bytes remaining,
// the only way to catch this case.

namespace {

using namespace hypercom;

void fill_pattern(std::span<std::uint8_t> bytes, std::uint8_t seed)
{
    for (std::size_t index = 0; index < bytes.size(); ++index) {
        bytes[index] = static_cast<std::uint8_t>(seed + index);
    }
}

void check_hello_round_trip(tests::test_report &report)
{
    proto::hello_request original;
    original.protocol_version = proto::PROTOCOL_VERSION;
    fill_pattern(original.client_pubkey, 0x11);
    std::vector<std::uint8_t> buffer;
    proto::byte_writer writer{buffer};
    original.write_to(writer);
    proto::byte_reader reader{buffer};
    proto::hello_request decoded;
    HYPERCOM_CHECK(report, decoded.read_from(reader));
    HYPERCOM_CHECK(report, reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report,
                   decoded.protocol_version == original.protocol_version);
    HYPERCOM_CHECK(report, decoded.client_pubkey == original.client_pubkey);
    // A buffer truncated by one byte must fail, not yield a half-message.
    buffer.pop_back();
    proto::byte_reader truncated{buffer};
    proto::hello_request refused;
    HYPERCOM_CHECK(report, !refused.read_from(truncated));
}

void check_auth_challenge_round_trip(tests::test_report &report)
{
    proto::auth_challenge original;
    original.protocol_version = proto::PROTOCOL_VERSION;
    fill_pattern(original.nonce, 0x22);
    original.account_exists = 1;
    original.server_time = 1754300000ULL;
    std::vector<std::uint8_t> buffer;
    proto::byte_writer writer{buffer};
    original.write_to(writer);
    proto::byte_reader reader{buffer};
    proto::auth_challenge decoded;
    HYPERCOM_CHECK(report, decoded.read_from(reader));
    HYPERCOM_CHECK(report, reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report, decoded.nonce == original.nonce);
    HYPERCOM_CHECK(report, decoded.account_exists == original.account_exists);
    HYPERCOM_CHECK(report, decoded.server_time == original.server_time);
}

void check_auth_signature_messages(tests::test_report &report)
{
    proto::auth_response original;
    fill_pattern(original.signature, 0x33);
    std::vector<std::uint8_t> buffer;
    proto::byte_writer writer{buffer};
    original.write_to(writer);
    proto::byte_reader reader{buffer};
    proto::auth_response decoded;
    HYPERCOM_CHECK(report, decoded.read_from(reader));
    HYPERCOM_CHECK(report, reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report, decoded.signature == original.signature);
    proto::auth_accepted accepted;
    accepted.user_id = 42;
    accepted.handle = "alice";
    accepted.server_time = 1754300001ULL;
    std::vector<std::uint8_t> accepted_buffer;
    proto::byte_writer accepted_writer{accepted_buffer};
    accepted.write_to(accepted_writer);
    proto::byte_reader accepted_reader{accepted_buffer};
    proto::auth_accepted decoded_accepted;
    HYPERCOM_CHECK(report, decoded_accepted.read_from(accepted_reader));
    HYPERCOM_CHECK(report, accepted_reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report, decoded_accepted.user_id == accepted.user_id);
    HYPERCOM_CHECK(report, decoded_accepted.handle == accepted.handle);
    HYPERCOM_CHECK(report,
                   decoded_accepted.server_time == accepted.server_time);
}

// Both signing-input builders must be deterministic and bind the signature
// to its context: that's what prevents a signature produced here from being
// replayed elsewhere.
void check_signing_inputs(tests::test_report &report)
{
    proto::wire_nonce nonce{};
    proto::wire_public_key pubkey{};
    fill_pattern(nonce, 0x44);
    fill_pattern(pubkey, 0x55);
    std::vector<std::uint8_t> first;
    std::vector<std::uint8_t> second;
    proto::build_auth_signing_input(nonce, pubkey, first);
    proto::build_auth_signing_input(nonce, pubkey, second);
    HYPERCOM_CHECK(report, first == second);
    HYPERCOM_CHECK(report, first.size() == proto::AUTH_SIGNATURE_DOMAIN.size() +
                                               nonce.size() + pubkey.size());
    proto::wire_public_key other_pubkey{};
    fill_pattern(other_pubkey, 0x66);
    std::vector<std::uint8_t> third;
    proto::build_auth_signing_input(nonce, other_pubkey, third);
    HYPERCOM_CHECK(report, first != third);
    std::vector<std::uint8_t> prekey_input;
    std::vector<std::uint8_t> prekey_other;
    proto::build_prekey_signing_input(pubkey, other_pubkey, prekey_input);
    proto::build_prekey_signing_input(other_pubkey, pubkey, prekey_other);
    // Swapping identity and prekey must change the input, otherwise a
    // signed prekey could be reattached to a different identity.
    HYPERCOM_CHECK(report, prekey_input != prekey_other);
}

void check_ping_round_trip(tests::test_report &report)
{
    proto::ping_request request;
    request.token = 0x0123456789ABCDEFULL;
    std::vector<std::uint8_t> buffer;
    proto::byte_writer writer{buffer};
    request.write_to(writer);
    proto::byte_reader reader{buffer};
    proto::ping_request decoded;
    HYPERCOM_CHECK(report, decoded.read_from(reader));
    HYPERCOM_CHECK(report, reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report, decoded.token == request.token);
    proto::ping_response response;
    response.token = request.token;
    response.server_time = 1754300002ULL;
    std::vector<std::uint8_t> response_buffer;
    proto::byte_writer response_writer{response_buffer};
    response.write_to(response_writer);
    proto::byte_reader response_reader{response_buffer};
    proto::ping_response decoded_response;
    HYPERCOM_CHECK(report, decoded_response.read_from(response_reader));
    HYPERCOM_CHECK(report, response_reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report, decoded_response.token == response.token);
    HYPERCOM_CHECK(report,
                   decoded_response.server_time == response.server_time);
}

void check_motd_round_trip(tests::test_report &report)
{
    proto::motd_push original;
    original.revision = 7;
    original.body = "Bienvenue sur HyperCom.\nBon sejour.";
    std::vector<std::uint8_t> buffer;
    proto::byte_writer writer{buffer};
    original.write_to(writer);
    proto::byte_reader reader{buffer};
    proto::motd_push decoded;
    HYPERCOM_CHECK(report, decoded.read_from(reader));
    HYPERCOM_CHECK(report, reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report, decoded.revision == original.revision);
    HYPERCOM_CHECK(report, decoded.body == original.body);
    // A MOTD beyond the cap must be rejected on read.
    proto::motd_push oversized;
    oversized.body = std::string(proto::MAX_MOTD_LENGTH + 1, 'a');
    std::vector<std::uint8_t> oversized_buffer;
    proto::byte_writer oversized_writer{oversized_buffer};
    oversized.write_to(oversized_writer);
    proto::byte_reader oversized_reader{oversized_buffer};
    proto::motd_push refused;
    HYPERCOM_CHECK(report, !refused.read_from(oversized_reader));
}

void check_status_round_trip(tests::test_report &report)
{
    proto::status_ok_response ok;
    ok.reference_id = 1234;
    std::vector<std::uint8_t> buffer;
    proto::byte_writer writer{buffer};
    ok.write_to(writer);
    proto::byte_reader reader{buffer};
    proto::status_ok_response decoded_ok;
    HYPERCOM_CHECK(report, decoded_ok.read_from(reader));
    HYPERCOM_CHECK(report, reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report, decoded_ok.reference_id == ok.reference_id);
    proto::status_error_response error;
    error.code = proto::error_code::rate_limited;
    error.detail = "trop de requetes";
    std::vector<std::uint8_t> error_buffer;
    proto::byte_writer error_writer{error_buffer};
    error.write_to(error_writer);
    proto::byte_reader error_reader{error_buffer};
    proto::status_error_response decoded_error;
    HYPERCOM_CHECK(report, decoded_error.read_from(error_reader));
    HYPERCOM_CHECK(report, error_reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report, decoded_error.code == error.code);
    HYPERCOM_CHECK(report, decoded_error.detail == error.detail);
}

} // namespace

int main()
{
    hypercom::tests::test_report report;
    check_hello_round_trip(report);
    check_auth_challenge_round_trip(report);
    check_auth_signature_messages(report);
    check_signing_inputs(report);
    check_ping_round_trip(report);
    check_motd_round_trip(report);
    check_status_round_trip(report);
    return report.summarize("aller-retour session");
}
