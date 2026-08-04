#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "client/ui/i18n.hpp"
#include "common/protocol/content_records.hpp"
#include "common/protocol/dm_envelope_record.hpp"
#include "common/protocol/forum_record.hpp"
#include "common/protocol/social_records.hpp"
#include "common/protocol/top8_message.hpp"

namespace hypercom::client {

// Deux onglets pour la colonne laterale : messages prives, ou social (amis,
// profils consultes, top 8). Evite d'empiler les trois dans le meme espace.
enum class side_panel_tab {
    direct_messages,
    social,
};

// Un message prive deja dechiffre, tel qu'il s'affiche.
//
// Il n'existe que dans cette structure, en memoire, pendant la duree de la
// session. Rien n'ecrit le clair sur le disque : fermer le client, c'est
// effacer la conversation.
struct decrypted_message {
    std::string sender_hex;
    std::string text;
    std::uint64_t received_at = 0;
    bool readable = false;
};

// Tout l'etat affichable du client.
//
// Structure sans methode, passee explicitement aux fonctions de dessin. ImGui
// est en mode immediat : chaque image relit cet etat et le redessine, il n'y a
// donc rien a synchroniser entre un modele et une vue.
struct ui_state {
    language current_lang = language::french;
    bool connected = false;
    bool registered = false;
    std::string handle;
    std::string identity_hex;
    std::string server_key_hex;
    std::string server_host;
    std::uint16_t server_port = 0;
    std::string status_message;
    bool status_is_error = false;
    // Leve une seule fois, par draw_auth_modal, juste apres la creation du
    // compte. gui_main le consomme pour lancer la musique et la sequence
    // d'accueil : une connexion ordinaire ne declenche donc rien.
    bool intro_requested = false;

    std::vector<proto::forum_record> forums;
    std::uint64_t selected_forum_id = 0;
    std::vector<proto::post_record> posts;
    std::uint64_t selected_post_id = 0;
    proto::post_record open_post;
    std::vector<proto::comment_record> comments;
    bool thread_truncated = false;

    side_panel_tab active_side_tab = side_panel_tab::direct_messages;

    std::vector<proto::friend_record> friends;
    proto::profile_record viewed_profile;
    std::array<proto::wire_public_key, proto::TOP8_SLOT_COUNT>
        viewed_top8_slots{};
    std::vector<proto::friend_record> viewed_top8_details;

    std::array<proto::wire_public_key, proto::TOP8_SLOT_COUNT>
        own_top8_slots{};
    std::vector<proto::friend_record> own_top8_details;

    std::vector<decrypted_message> inbox;
    std::string dm_recipient_hex;

    // Tampons de saisie. ImGui ecrit directement dedans, d'ou les tableaux de
    // taille fixe plutot que des std::string.
    char forum_name_input[64] = {};
    char forum_description_input[256] = {};
    char post_title_input[256] = {};
    char post_body_input[4096] = {};
    char comment_input[2048] = {};
    char display_name_input[64] = {};
    char bio_input[1024] = {};
    char dm_recipient_input[80] = {};
    char dm_text_input[2048] = {};
    char registration_handle_input[64] = {};
    char friend_add_input[80] = {};
};

} // namespace hypercom::client
