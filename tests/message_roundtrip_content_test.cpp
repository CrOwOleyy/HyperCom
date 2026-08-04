#include <cstdint>
#include <span>
#include <string>
#include <vector>

#include "common/protocol/comment_create_message.hpp"
#include "common/protocol/forum_create_message.hpp"
#include "common/protocol/forum_list_message.hpp"
#include "common/protocol/post_create_message.hpp"
#include "common/protocol/post_list_message.hpp"
#include "common/protocol/thread_fetch_message.hpp"
#include "tests/test_harness.hpp"

// Aller-retour des messages de forum, de post et de commentaire.
//
// Ces messages transportent des listes, donc le point sensible n'est pas le
// champ isole mais le plafond de la liste : un pair qui annonce plus
// d'elements que le maximum doit etre refuse avant toute allocation.

namespace {

using namespace hypercom;

void fill_pattern(std::span<std::uint8_t> bytes, std::uint8_t seed)
{
    for (std::size_t index = 0; index < bytes.size(); ++index) {
        bytes[index] = static_cast<std::uint8_t>(seed + index);
    }
}

[[nodiscard]] proto::forum_record make_forum_record(std::uint64_t id)
{
    proto::forum_record record;
    record.id = id;
    record.name = "general";
    record.description = "Le forum par defaut.";
    fill_pattern(record.founder_pubkey, 0x11);
    record.founder_handle = "alice";
    record.theme_json = "{\"aero\":true}";
    record.created_at = 1754300000ULL;
    record.post_count = 3;
    return record;
}

[[nodiscard]] proto::post_record make_post_record(std::uint64_t id)
{
    proto::post_record record;
    record.id = id;
    record.forum_id = 1;
    fill_pattern(record.author_pubkey, 0x22);
    record.author_handle = "alice";
    record.title = "Premier post";
    record.body = "Corps du post avec un\tsaut de ligne.\n";
    record.created_at = 1754300001ULL;
    record.comment_count = 2;
    return record;
}

[[nodiscard]] proto::comment_record make_comment_record(std::uint64_t id)
{
    proto::comment_record record;
    record.id = id;
    record.post_id = 1;
    record.parent_comment_id = 0;
    fill_pattern(record.author_pubkey, 0x33);
    record.author_handle = "collaborateur";
    record.body = "Reponse au post.";
    record.created_at = 1754300002ULL;
    record.depth = 1;
    return record;
}

void check_forum_messages(tests::test_report &report)
{
    proto::forum_create_request create;
    create.name = "general";
    create.description = "Le forum par defaut.";
    create.theme_json = "{\"aero\":true}";
    std::vector<std::uint8_t> buffer;
    proto::byte_writer writer{buffer};
    create.write_to(writer);
    proto::byte_reader reader{buffer};
    proto::forum_create_request decoded_create;
    HYPERCOM_CHECK(report, decoded_create.read_from(reader));
    HYPERCOM_CHECK(report, reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report, decoded_create.name == create.name);
    HYPERCOM_CHECK(report,
                   decoded_create.description == create.description);
    HYPERCOM_CHECK(report, decoded_create.theme_json == create.theme_json);
    proto::forum_info_response info;
    info.forum = make_forum_record(1);
    std::vector<std::uint8_t> info_buffer;
    proto::byte_writer info_writer{info_buffer};
    info.write_to(info_writer);
    proto::byte_reader info_reader{info_buffer};
    proto::forum_info_response decoded_info;
    HYPERCOM_CHECK(report, decoded_info.read_from(info_reader));
    HYPERCOM_CHECK(report, info_reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report, decoded_info.forum.name == info.forum.name);
    HYPERCOM_CHECK(report,
                   decoded_info.forum.founder_pubkey
                       == info.forum.founder_pubkey);
    HYPERCOM_CHECK(report,
                   decoded_info.forum.post_count == info.forum.post_count);
}

void check_forum_list_messages(tests::test_report &report)
{
    proto::forum_list_request request;
    request.offset = 20;
    request.limit = 50;
    std::vector<std::uint8_t> buffer;
    proto::byte_writer writer{buffer};
    request.write_to(writer);
    proto::byte_reader reader{buffer};
    proto::forum_list_request decoded_request;
    HYPERCOM_CHECK(report, decoded_request.read_from(reader));
    HYPERCOM_CHECK(report, reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report, decoded_request.offset == request.offset);
    HYPERCOM_CHECK(report, decoded_request.limit == request.limit);
    proto::forum_list_response response;
    response.forums.push_back(make_forum_record(1));
    response.forums.push_back(make_forum_record(2));
    response.total_count = 2;
    std::vector<std::uint8_t> response_buffer;
    proto::byte_writer response_writer{response_buffer};
    response.write_to(response_writer);
    proto::byte_reader response_reader{response_buffer};
    proto::forum_list_response decoded_response;
    HYPERCOM_CHECK(report, decoded_response.read_from(response_reader));
    HYPERCOM_CHECK(report, response_reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report, decoded_response.forums.size() == 2);
    HYPERCOM_CHECK(report, decoded_response.forums[1].id == 2);
    HYPERCOM_CHECK(report,
                   decoded_response.total_count == response.total_count);
}

void check_post_messages(tests::test_report &report)
{
    proto::post_create_request create;
    create.forum_id = 1;
    create.title = "Premier post";
    create.body = "Corps du post.";
    std::vector<std::uint8_t> buffer;
    proto::byte_writer writer{buffer};
    create.write_to(writer);
    proto::byte_reader reader{buffer};
    proto::post_create_request decoded_create;
    HYPERCOM_CHECK(report, decoded_create.read_from(reader));
    HYPERCOM_CHECK(report, reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report, decoded_create.forum_id == create.forum_id);
    HYPERCOM_CHECK(report, decoded_create.title == create.title);
    HYPERCOM_CHECK(report, decoded_create.body == create.body);
    proto::post_list_response response;
    response.posts.push_back(make_post_record(1));
    response.total_count = 1;
    std::vector<std::uint8_t> response_buffer;
    proto::byte_writer response_writer{response_buffer};
    response.write_to(response_writer);
    proto::byte_reader response_reader{response_buffer};
    proto::post_list_response decoded_response;
    HYPERCOM_CHECK(report, decoded_response.read_from(response_reader));
    HYPERCOM_CHECK(report, response_reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report, decoded_response.posts.size() == 1);
    HYPERCOM_CHECK(report,
                   decoded_response.posts[0].body == response.posts[0].body);
    HYPERCOM_CHECK(report,
                   decoded_response.posts[0].author_pubkey
                       == response.posts[0].author_pubkey);
}

void check_comment_messages(tests::test_report &report)
{
    proto::comment_create_request create;
    create.post_id = 1;
    create.parent_comment_id = 0;
    create.body = "Reponse au post.";
    std::vector<std::uint8_t> buffer;
    proto::byte_writer writer{buffer};
    create.write_to(writer);
    proto::byte_reader reader{buffer};
    proto::comment_create_request decoded_create;
    HYPERCOM_CHECK(report, decoded_create.read_from(reader));
    HYPERCOM_CHECK(report, reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report, decoded_create.post_id == create.post_id);
    HYPERCOM_CHECK(report, decoded_create.body == create.body);
    proto::comment_info_response info;
    info.comment = make_comment_record(1);
    std::vector<std::uint8_t> info_buffer;
    proto::byte_writer info_writer{info_buffer};
    info.write_to(info_writer);
    proto::byte_reader info_reader{info_buffer};
    proto::comment_info_response decoded_info;
    HYPERCOM_CHECK(report, decoded_info.read_from(info_reader));
    HYPERCOM_CHECK(report, info_reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report,
                   decoded_info.comment.depth == info.comment.depth);
    HYPERCOM_CHECK(report, decoded_info.comment.body == info.comment.body);
    // Un corps au-dela du plafond ne doit pas franchir le decodeur.
    proto::comment_create_request oversized;
    oversized.body = std::string(proto::MAX_COMMENT_BODY_LENGTH + 1, 'a');
    std::vector<std::uint8_t> oversized_buffer;
    proto::byte_writer oversized_writer{oversized_buffer};
    oversized.write_to(oversized_writer);
    proto::byte_reader oversized_reader{oversized_buffer};
    proto::comment_create_request refused;
    HYPERCOM_CHECK(report, !refused.read_from(oversized_reader));
}

void check_thread_messages(tests::test_report &report)
{
    proto::thread_fetch_request request;
    request.post_id = 7;
    std::vector<std::uint8_t> buffer;
    proto::byte_writer writer{buffer};
    request.write_to(writer);
    proto::byte_reader reader{buffer};
    proto::thread_fetch_request decoded_request;
    HYPERCOM_CHECK(report, decoded_request.read_from(reader));
    HYPERCOM_CHECK(report, reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report, decoded_request.post_id == request.post_id);
    proto::thread_response response;
    response.post = make_post_record(7);
    response.comments.push_back(make_comment_record(1));
    response.comments.push_back(make_comment_record(2));
    response.truncated = 1;
    std::vector<std::uint8_t> response_buffer;
    proto::byte_writer response_writer{response_buffer};
    response.write_to(response_writer);
    proto::byte_reader response_reader{response_buffer};
    proto::thread_response decoded_response;
    HYPERCOM_CHECK(report, decoded_response.read_from(response_reader));
    HYPERCOM_CHECK(report, response_reader.count_remaining_bytes() == 0);
    HYPERCOM_CHECK(report, decoded_response.post.id == response.post.id);
    HYPERCOM_CHECK(report, decoded_response.comments.size() == 2);
    HYPERCOM_CHECK(report,
                   decoded_response.truncated == response.truncated);
}

// Le point le plus important du fichier : un nombre d'elements annonce
// au-dela du plafond doit etre refuse sur l'annonce seule, sans que le
// decodeur attende ni reserve les elements correspondants.
void check_list_caps(tests::test_report &report)
{
    std::vector<std::uint8_t> hostile;
    proto::byte_writer writer{hostile};
    writer.write_integer(static_cast<std::uint16_t>(proto::MAX_LIST_ITEMS + 1));
    proto::byte_reader reader{hostile};
    proto::forum_list_response refused;
    HYPERCOM_CHECK(report, !refused.read_from(reader));
    HYPERCOM_CHECK(report, refused.forums.empty());
    std::vector<std::uint8_t> hostile_comments;
    proto::byte_writer comment_writer{hostile_comments};
    make_post_record(1).write_to(comment_writer);
    comment_writer.write_integer(
        static_cast<std::uint16_t>(proto::MAX_THREAD_COMMENTS + 1));
    proto::byte_reader comment_reader{hostile_comments};
    proto::thread_response refused_thread;
    HYPERCOM_CHECK(report, !refused_thread.read_from(comment_reader));
    HYPERCOM_CHECK(report, refused_thread.comments.empty());
}

} // namespace

int main()
{
    hypercom::tests::test_report report;
    check_forum_messages(report);
    check_forum_list_messages(report);
    check_post_messages(report);
    check_comment_messages(report);
    check_thread_messages(report);
    check_list_caps(report);
    return report.summarize("aller-retour contenu");
}
