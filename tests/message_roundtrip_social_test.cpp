#include "common/protocol/dm_ack_message.hpp"
#include "common/protocol/dm_fetch_message.hpp"
#include "common/protocol/dm_send_message.hpp"
#include "common/protocol/friend_message.hpp"
#include "common/protocol/prekey_fetch_message.hpp"
#include "common/protocol/prekey_publish_message.hpp"
#include "common/protocol/profile_get_message.hpp"
#include "common/protocol/profile_set_message.hpp"
#include "common/protocol/register_message.hpp"
#include "common/protocol/top8_message.hpp"
#include "tests/test_harness.hpp"

#include <cstdint>
#include <span>
#include <string>
#include <vector>

// Round trip for account, profile, and direct message messages.
//
// On the DM side, the decoder must understand nothing about the ciphertext:
// it treats it as an opaque blob and only bounds it. A test that only
// passed on a well-formed cipher would mask that property, so the bytes
// used here are deliberately arbitrary.

namespace {

using namespace hypercom;

void fill_pattern(std::span<std::uint8_t> bytes, std::uint8_t seed)
{
    for (std::size_t index = 0; index < bytes.size(); ++index) {
        bytes[index] = static_cast<std::uint8_t>(seed + index);
    }
}

[[nodiscard]] proto::friend_record make_friend_record(std::uint8_t seed)
{
    proto::friend_record record;
    fill_pattern(record.pubkey, seed);
    record.handle = "collaborateur";
    record.display_name = "Le collaborateur";
    record.status = proto::friendship_status::accepted;
    return record;
}

void check_account_messages(tests::test_report &report)
{
    proto::register_request registration;
    registration.handle = "alice";
    std::vector<std::uint8_t> buffer;
    proto::byte_writer writer{buffer};
    registration.write_to(writer);
    proto::byte_reader reader{buffer};
    proto::register_request decoded;
    HYPERCOM_CHECK(report, decoded.read_from(reader));
    HYPERCOM_CHECK(report, reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report, decoded.handle == registration.handle);
    // A handle longer than the cap must not make it past the decoder.
    proto::register_request oversized;
    oversized.handle = std::string(proto::MAX_HANDLE_LENGTH + 1, 'a');
    std::vector<std::uint8_t> oversized_buffer;
    proto::byte_writer oversized_writer{oversized_buffer};
    oversized.write_to(oversized_writer);
    proto::byte_reader oversized_reader{oversized_buffer};
    proto::register_request refused;
    HYPERCOM_CHECK(report, !refused.read_from(oversized_reader));
}

void check_prekey_messages(tests::test_report &report)
{
    proto::prekey_publish_request publish;
    fill_pattern(publish.prekey, 0x77);
    fill_pattern(publish.signature, 0x88);
    std::vector<std::uint8_t> buffer;
    proto::byte_writer writer{buffer};
    publish.write_to(writer);
    proto::byte_reader reader{buffer};
    proto::prekey_publish_request decoded_publish;
    HYPERCOM_CHECK(report, decoded_publish.read_from(reader));
    HYPERCOM_CHECK(report, reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report, decoded_publish.prekey == publish.prekey);
    HYPERCOM_CHECK(report, decoded_publish.signature == publish.signature);
    proto::prekey_fetch_request fetch;
    fill_pattern(fetch.target_pubkey, 0x99);
    std::vector<std::uint8_t> fetch_buffer;
    proto::byte_writer fetch_writer{fetch_buffer};
    fetch.write_to(fetch_writer);
    proto::byte_reader fetch_reader{fetch_buffer};
    proto::prekey_fetch_request decoded_fetch;
    HYPERCOM_CHECK(report, decoded_fetch.read_from(fetch_reader));
    HYPERCOM_CHECK(report, fetch_reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report, decoded_fetch.target_pubkey == fetch.target_pubkey);
    proto::prekey_bundle_response bundle;
    fill_pattern(bundle.owner_pubkey, 0xAA);
    fill_pattern(bundle.prekey, 0xBB);
    fill_pattern(bundle.signature, 0xCC);
    bundle.created_at = 1754300003ULL;
    std::vector<std::uint8_t> bundle_buffer;
    proto::byte_writer bundle_writer{bundle_buffer};
    bundle.write_to(bundle_writer);
    proto::byte_reader bundle_reader{bundle_buffer};
    proto::prekey_bundle_response decoded_bundle;
    HYPERCOM_CHECK(report, decoded_bundle.read_from(bundle_reader));
    HYPERCOM_CHECK(report, bundle_reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report, decoded_bundle.owner_pubkey == bundle.owner_pubkey);
    HYPERCOM_CHECK(report, decoded_bundle.created_at == bundle.created_at);
}

void check_profile_messages(tests::test_report &report)
{
    proto::profile_get_request request;
    fill_pattern(request.target_pubkey, 0x11);
    std::vector<std::uint8_t> buffer;
    proto::byte_writer writer{buffer};
    request.write_to(writer);
    proto::byte_reader reader{buffer};
    proto::profile_get_request decoded_request;
    HYPERCOM_CHECK(report, decoded_request.read_from(reader));
    HYPERCOM_CHECK(report, reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report,
                   decoded_request.target_pubkey == request.target_pubkey);
    proto::profile_set_request settings;
    settings.display_name = "Alice";
    settings.bio = "Administrateur.";
    settings.theme_json = "{\"aero\":true}";
    settings.banner_reference = "blob:0011223344";
    std::vector<std::uint8_t> settings_buffer;
    proto::byte_writer settings_writer{settings_buffer};
    settings.write_to(settings_writer);
    proto::byte_reader settings_reader{settings_buffer};
    proto::profile_set_request decoded_settings;
    HYPERCOM_CHECK(report, decoded_settings.read_from(settings_reader));
    HYPERCOM_CHECK(report, settings_reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report,
                   decoded_settings.display_name == settings.display_name);
    HYPERCOM_CHECK(report, decoded_settings.bio == settings.bio);
    HYPERCOM_CHECK(report, decoded_settings.theme_json == settings.theme_json);
    HYPERCOM_CHECK(report, decoded_settings.banner_reference ==
                               settings.banner_reference);
}

void check_friend_messages(tests::test_report &report)
{
    proto::friend_add_request request;
    fill_pattern(request.target_pubkey, 0x22);
    request.status = proto::friendship_status::blocked;
    std::vector<std::uint8_t> buffer;
    proto::byte_writer writer{buffer};
    request.write_to(writer);
    proto::byte_reader reader{buffer};
    proto::friend_add_request decoded_request;
    HYPERCOM_CHECK(report, decoded_request.read_from(reader));
    HYPERCOM_CHECK(report, reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report,
                   decoded_request.target_pubkey == request.target_pubkey);
    HYPERCOM_CHECK(report, decoded_request.status == request.status);
    proto::friend_list_response response;
    response.friends.push_back(make_friend_record(0x33));
    response.friends.push_back(make_friend_record(0x44));
    std::vector<std::uint8_t> response_buffer;
    proto::byte_writer response_writer{response_buffer};
    response.write_to(response_writer);
    proto::byte_reader response_reader{response_buffer};
    proto::friend_list_response decoded_response;
    HYPERCOM_CHECK(report, decoded_response.read_from(response_reader));
    HYPERCOM_CHECK(report, response_reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report, decoded_response.friends.size() == 2);
    HYPERCOM_CHECK(report, decoded_response.friends[0].handle ==
                               response.friends[0].handle);
    HYPERCOM_CHECK(report, decoded_response.friends[1].pubkey ==
                               response.friends[1].pubkey);
}

void check_top8_messages(tests::test_report &report)
{
    proto::top8_set_request request;
    fill_pattern(request.slots[0], 0x55);
    fill_pattern(request.slots[3], 0x66);
    std::vector<std::uint8_t> buffer;
    proto::byte_writer writer{buffer};
    request.write_to(writer);
    proto::byte_reader reader{buffer};
    proto::top8_set_request decoded_request;
    HYPERCOM_CHECK(report, decoded_request.read_from(reader));
    HYPERCOM_CHECK(report, reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report, decoded_request.slots == request.slots);
    // An empty slot remains a null key after the round trip.
    proto::wire_public_key const empty_slot{};
    HYPERCOM_CHECK(report, decoded_request.slots[1] == empty_slot);
    proto::top8_response response;
    fill_pattern(response.slots[0], 0x55);
    response.details.push_back(make_friend_record(0x55));
    std::vector<std::uint8_t> response_buffer;
    proto::byte_writer response_writer{response_buffer};
    response.write_to(response_writer);
    proto::byte_reader response_reader{response_buffer};
    proto::top8_response decoded_response;
    HYPERCOM_CHECK(report, decoded_response.read_from(response_reader));
    HYPERCOM_CHECK(report, response_reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report, decoded_response.slots == response.slots);
    HYPERCOM_CHECK(report, decoded_response.details.size() == 1);
}

void check_dm_messages(tests::test_report &report)
{
    proto::dm_send_request send;
    fill_pattern(send.recipient_pubkey, 0x77);
    send.ciphertext = {0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0xFF};
    std::vector<std::uint8_t> buffer;
    proto::byte_writer writer{buffer};
    send.write_to(writer);
    proto::byte_reader reader{buffer};
    proto::dm_send_request decoded_send;
    HYPERCOM_CHECK(report, decoded_send.read_from(reader));
    HYPERCOM_CHECK(report, reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report,
                   decoded_send.recipient_pubkey == send.recipient_pubkey);
    HYPERCOM_CHECK(report, decoded_send.ciphertext == send.ciphertext);
    proto::dm_fetch_request fetch;
    fetch.since_id = 99;
    fetch.limit = proto::MAX_DM_BATCH_ITEMS;
    std::vector<std::uint8_t> fetch_buffer;
    proto::byte_writer fetch_writer{fetch_buffer};
    fetch.write_to(fetch_writer);
    proto::byte_reader fetch_reader{fetch_buffer};
    proto::dm_fetch_request decoded_fetch;
    HYPERCOM_CHECK(report, decoded_fetch.read_from(fetch_reader));
    HYPERCOM_CHECK(report, fetch_reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report, decoded_fetch.since_id == fetch.since_id);
    HYPERCOM_CHECK(report, decoded_fetch.limit == fetch.limit);
    proto::dm_ack_request ack;
    ack.envelope_ids = {1, 2, 3, 99};
    std::vector<std::uint8_t> ack_buffer;
    proto::byte_writer ack_writer{ack_buffer};
    ack.write_to(ack_writer);
    proto::byte_reader ack_reader{ack_buffer};
    proto::dm_ack_request decoded_ack;
    HYPERCOM_CHECK(report, decoded_ack.read_from(ack_reader));
    HYPERCOM_CHECK(report, ack_reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report, decoded_ack.envelope_ids == ack.envelope_ids);
}

// The ciphertext is opaque but bounded. A peer announcing more than
// MAX_DM_CIPHERTEXT_SIZE must be rejected without any allocation.
void check_dm_caps(tests::test_report &report)
{
    proto::dm_send_request oversized;
    fill_pattern(oversized.recipient_pubkey, 0x88);
    oversized.ciphertext.assign(proto::MAX_DM_CIPHERTEXT_SIZE + 1, 0x41);
    std::vector<std::uint8_t> buffer;
    proto::byte_writer writer{buffer};
    oversized.write_to(writer);
    proto::byte_reader reader{buffer};
    proto::dm_send_request refused;
    HYPERCOM_CHECK(report, !refused.read_from(reader));
    HYPERCOM_CHECK(report, refused.ciphertext.empty());
    std::vector<std::uint8_t> hostile_list;
    proto::byte_writer list_writer{hostile_list};
    list_writer.write_integer(
        static_cast<std::uint16_t>(proto::MAX_DM_BATCH_ITEMS + 1));
    proto::byte_reader list_reader{hostile_list};
    proto::dm_list_response refused_list;
    HYPERCOM_CHECK(report, !refused_list.read_from(list_reader));
    HYPERCOM_CHECK(report, refused_list.envelopes.empty());
}

} // namespace

int main()
{
    hypercom::tests::test_report report;
    check_account_messages(report);
    check_prekey_messages(report);
    check_profile_messages(report);
    check_friend_messages(report);
    check_top8_messages(report);
    check_dm_messages(report);
    check_dm_caps(report);
    return report.summarize("aller-retour social");
}
