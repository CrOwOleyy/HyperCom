#pragma once

#include <cstdint>

namespace hypercom::proto {

// Un octet, une famille par tranche de 16. Les valeurs sont figees : elles
// font partie du format de fil et ne se renumerotent pas.
enum class message_type : std::uint8_t {
    // Session -- 0x0*
    hello_request = 0x01,
    hello_response = 0x02,
    auth_challenge = 0x03,
    auth_response = 0x04,
    auth_accepted = 0x05,
    ping_request = 0x06,
    ping_response = 0x07,
    motd_push = 0x08,
    status_ok = 0x09,
    status_error = 0x0A,

    // Compte -- 0x1*
    register_request = 0x10,
    prekey_publish_request = 0x11,
    prekey_fetch_request = 0x12,
    prekey_bundle_response = 0x13,

    // Forums -- 0x2*
    forum_create_request = 0x20,
    forum_info_response = 0x21,
    forum_list_request = 0x22,
    forum_list_response = 0x23,

    // Contenu -- 0x3*
    post_create_request = 0x30,
    post_info_response = 0x31,
    post_list_request = 0x32,
    post_list_response = 0x33,
    thread_fetch_request = 0x34,
    thread_response = 0x35,
    comment_create_request = 0x36,
    comment_info_response = 0x37,

    // Social -- 0x4*
    profile_get_request = 0x40,
    profile_response = 0x41,
    profile_set_request = 0x42,
    friend_add_request = 0x43,
    friend_list_request = 0x44,
    friend_list_response = 0x45,
    top8_set_request = 0x46,
    top8_get_request = 0x47,
    top8_response = 0x48,

    // Prive -- 0x5*
    dm_send_request = 0x50,
    dm_fetch_request = 0x51,
    dm_list_response = 0x52,
    dm_ack_request = 0x53,

    // Blobs -- 0x6*, reserves pour la v2. Les valeurs sont posees maintenant
    // pour que brancher le P2P plus tard ne renumerote rien.
    blob_announce_request = 0x60,
    blob_locate_request = 0x61,
    blob_peers_response = 0x62,
};

// Un type inconnu est rejete avant d'atteindre un handler : le routeur ne
// travaille jamais sur un octet arbitraire.
[[nodiscard]] bool is_known_message_type(std::uint8_t raw_type);

} // namespace hypercom::proto
