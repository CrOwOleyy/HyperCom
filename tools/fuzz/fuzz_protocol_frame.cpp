#include <cstddef>
#include <cstdint>
#include <span>

#include "common/protocol/auth_message.hpp"
#include "common/protocol/comment_create_message.hpp"
#include "common/protocol/dm_ack_message.hpp"
#include "common/protocol/dm_fetch_message.hpp"
#include "common/protocol/dm_send_message.hpp"
#include "common/protocol/forum_create_message.hpp"
#include "common/protocol/forum_list_message.hpp"
#include "common/protocol/frame_codec.hpp"
#include "common/protocol/friend_message.hpp"
#include "common/protocol/hello_message.hpp"
#include "common/protocol/motd_message.hpp"
#include "common/protocol/ping_message.hpp"
#include "common/protocol/post_create_message.hpp"
#include "common/protocol/post_list_message.hpp"
#include "common/protocol/prekey_fetch_message.hpp"
#include "common/protocol/prekey_publish_message.hpp"
#include "common/protocol/profile_get_message.hpp"
#include "common/protocol/profile_set_message.hpp"
#include "common/protocol/register_message.hpp"
#include "common/protocol/status_message.hpp"
#include "common/protocol/thread_fetch_message.hpp"
#include "common/protocol/top8_message.hpp"

// Harnais libFuzzer sur le parseur de protocole.
//
// Probablement le test le plus rentable du projet : le parseur est la premiere
// chose qu'un inconnu atteint. Il est sans etat, sans I/O et sans acces base,
// donc ce fichier ne depend que de hypercom_protocol.
//
//   cmake -B build-fuzz -DHYPERCOM_BUILD_FUZZ=ON -DCMAKE_CXX_COMPILER=clang++
//   ./build-fuzz/bin/fuzz_protocol_frame tools/fuzz/corpus/ -max_len=4096
//
// Le harnais ne verifie aucune valeur : le succes est l'ABSENCE de plantage,
// de lecture hors bornes et d'allocation non bornee. Toute entree doit etre
// soit decodee, soit rejetee proprement.
//
// Chaque decodeur voit les memes octets, chacun sur son propre lecteur. Un
// decodeur absent de cette liste n'est pas fuzze : penser a l'ajouter en meme
// temps que le message.

namespace {

using namespace hypercom;

template <typename message_type>
void exercise_one(std::span<std::uint8_t const> payload)
{
    message_type message;
    proto::byte_reader reader{payload};
    static_cast<void>(message.read_from(reader));
}

void exercise_session_messages(std::span<std::uint8_t const> payload)
{
    exercise_one<proto::hello_request>(payload);
    exercise_one<proto::auth_challenge>(payload);
    exercise_one<proto::auth_response>(payload);
    exercise_one<proto::auth_accepted>(payload);
    exercise_one<proto::ping_request>(payload);
    exercise_one<proto::ping_response>(payload);
    exercise_one<proto::motd_push>(payload);
    exercise_one<proto::status_ok_response>(payload);
    exercise_one<proto::status_error_response>(payload);
    exercise_one<proto::register_request>(payload);
    exercise_one<proto::prekey_publish_request>(payload);
    exercise_one<proto::prekey_fetch_request>(payload);
    exercise_one<proto::prekey_bundle_response>(payload);
}

void exercise_content_messages(std::span<std::uint8_t const> payload)
{
    exercise_one<proto::forum_create_request>(payload);
    exercise_one<proto::forum_info_response>(payload);
    exercise_one<proto::forum_list_request>(payload);
    exercise_one<proto::forum_list_response>(payload);
    exercise_one<proto::post_create_request>(payload);
    exercise_one<proto::post_info_response>(payload);
    exercise_one<proto::post_list_request>(payload);
    exercise_one<proto::post_list_response>(payload);
    exercise_one<proto::thread_fetch_request>(payload);
    exercise_one<proto::thread_response>(payload);
    exercise_one<proto::comment_create_request>(payload);
    exercise_one<proto::comment_info_response>(payload);
}

void exercise_social_messages(std::span<std::uint8_t const> payload)
{
    exercise_one<proto::profile_get_request>(payload);
    exercise_one<proto::profile_response>(payload);
    exercise_one<proto::profile_set_request>(payload);
    exercise_one<proto::friend_add_request>(payload);
    exercise_one<proto::friend_list_response>(payload);
    exercise_one<proto::top8_set_request>(payload);
    exercise_one<proto::top8_response>(payload);
    exercise_one<proto::dm_send_request>(payload);
    exercise_one<proto::dm_fetch_request>(payload);
    exercise_one<proto::dm_list_response>(payload);
    exercise_one<proto::dm_ack_request>(payload);
}

void exercise_every_message(std::span<std::uint8_t const> payload)
{
    exercise_session_messages(payload);
    exercise_content_messages(payload);
    exercise_social_messages(payload);
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(std::uint8_t const *data,
                                      std::size_t size)
{
    std::span<std::uint8_t const> const input{data, size};
    proto::frame_header header{};
    // L'entete d'abord : c'est lui qui borne tout le reste.
    if (proto::decode_frame_header(input, header)
        && input.size() >= proto::FRAME_HEADER_SIZE) {
        exercise_every_message(input.subspan(proto::FRAME_HEADER_SIZE));
    }
    // Puis les messages sur les octets bruts, sans entete : un handler ne doit
    // pas non plus deborder si le routeur lui passe n'importe quoi.
    exercise_every_message(input);
    return 0;
}
