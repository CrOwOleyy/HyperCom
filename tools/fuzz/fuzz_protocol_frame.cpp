#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "common/protocol/comment_create_message.hpp"
#include "common/protocol/dm_ack_message.hpp"
#include "common/protocol/dm_fetch_message.hpp"
#include "common/protocol/dm_send_message.hpp"
#include "common/protocol/forum_create_message.hpp"
#include "common/protocol/forum_list_message.hpp"
#include "common/protocol/frame_codec.hpp"
#include "common/protocol/hello_message.hpp"
#include "common/protocol/post_create_message.hpp"
#include "common/protocol/post_list_message.hpp"
#include "common/protocol/profile_set_message.hpp"
#include "common/protocol/register_message.hpp"
#include "common/protocol/thread_fetch_message.hpp"
#include "common/protocol/top8_message.hpp"

// Harnais libFuzzer sur le parseur de protocole.
//
// Probablement le test le plus rentable du projet : le parseur est la premiere
// chose qu'un inconnu atteint. Il est sans etat, sans I/O et sans acces base,
// donc ce fichier ne depend que de hypercom_protocol.
//
//   cmake -B build-fuzz -DHYPERCOM_BUILD_FUZZ=ON \
//         -DCMAKE_CXX_COMPILER=clang++ \
//         -DHYPERCOM_SANITIZER=address,undefined
//   ./build-fuzz/bin/fuzz_protocol_frame corpus/ -max_len=4096
//
// Le harnais ne verifie aucune valeur : le succes est l'ABSENCE de plantage,
// de lecture hors bornes et d'allocation non bornee. Toute entree doit etre
// soit decodee, soit rejetee proprement.

namespace {

using namespace hypercom;

// Chaque message est essaye sur les memes octets. Le decodage peut echouer,
// c'est le cas normal -- ce qui est interdit, c'est de deborder.
void exercise_every_message(std::span<std::uint8_t const> payload)
{
    proto::hello_request hello;
    proto::byte_reader hello_reader{payload};
    static_cast<void>(hello.read_from(hello_reader));
    proto::register_request registration;
    proto::byte_reader register_reader{payload};
    static_cast<void>(registration.read_from(register_reader));
    proto::forum_create_request forum_create;
    proto::byte_reader forum_create_reader{payload};
    static_cast<void>(forum_create.read_from(forum_create_reader));
    proto::forum_list_response forum_list;
    proto::byte_reader forum_list_reader{payload};
    static_cast<void>(forum_list.read_from(forum_list_reader));
    proto::post_create_request post_create;
    proto::byte_reader post_create_reader{payload};
    static_cast<void>(post_create.read_from(post_create_reader));
    proto::post_list_response post_list;
    proto::byte_reader post_list_reader{payload};
    static_cast<void>(post_list.read_from(post_list_reader));
    proto::thread_response thread;
    proto::byte_reader thread_reader{payload};
    static_cast<void>(thread.read_from(thread_reader));
    proto::comment_create_request comment;
    proto::byte_reader comment_reader{payload};
    static_cast<void>(comment.read_from(comment_reader));
    proto::profile_set_request profile;
    proto::byte_reader profile_reader{payload};
    static_cast<void>(profile.read_from(profile_reader));
    proto::top8_set_request top8;
    proto::byte_reader top8_reader{payload};
    static_cast<void>(top8.read_from(top8_reader));
    proto::dm_send_request dm_send;
    proto::byte_reader dm_send_reader{payload};
    static_cast<void>(dm_send.read_from(dm_send_reader));
    proto::dm_list_response dm_list;
    proto::byte_reader dm_list_reader{payload};
    static_cast<void>(dm_list.read_from(dm_list_reader));
    proto::dm_ack_request dm_ack;
    proto::byte_reader dm_ack_reader{payload};
    static_cast<void>(dm_ack.read_from(dm_ack_reader));
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
