#include "client/ui/i18n.hpp"

#include <string_view>
#include <unordered_map>

namespace hypercom::client {

struct translation {
    const char* fr;
    const char* en;
};

static const std::unordered_map<std::string_view, translation> dictionary = {
    // gui_main.cpp
    {"status_encrypted", {"Canal chiffré (Noise_NK) et authentifié", "Encrypted channel (Noise_NK) & authenticated"}},
    {"status_offline", {"Hors ligne", "Offline"}},
    {"status_server", {"Serveur", "Server"}},
    {"status_identity", {"Identité", "Identity"}},
    {"status_registered", {"Enregistré sous", "Registered as"}},
    {"status_unregistered", {"Non enregistré", "Not registered"}},

    // ui_scale.cpp
    {"zoom_label", {"Zoom", "Zoom"}},
    {"zoom_reset", {"100 %", "100 %"}},

    // draw_auth_modal.cpp
    {"auth_welcome", {"BIENVENUE SUR HYPERCOM", "WELCOME TO HYPERCOM"}},
    {"auth_desc_1", {"Votre clé publique n'a pas encore de compte sur ce serveur.", "Your public key does not have an account on this server yet."}},
    {"auth_desc_2", {"Choisissez un pseudo pour vous enregistrer et accéder au réseau social.", "Choose a handle to register and access the social network."}},
    {"auth_handle_heading", {"PSEUDO DE COMPTE", "ACCOUNT HANDLE"}},
    {"auth_handle_hint", {"3 à 32 caractères (lettres, chiffres, tiret, souligné)", "3 to 32 characters (letters, numbers, dash, underscore)"}},
    {"auth_btn_register", {"S'inscrire et se connecter", "Register & Connect"}},
    {"auth_success", {"Compte créé avec succès !", "Account successfully created!"}},

    // draw_forum_panel.cpp
    {"forum_heading", {"FORUMS", "FORUMS"}},
    {"forum_btn_refresh", {"Actualiser", "Refresh"}},
    {"forum_create_heading", {"CRÉER UN FORUM", "CREATE FORUM"}},
    {"forum_create_name", {"Nom", "Name"}},
    {"forum_create_desc", {"Description", "Description"}},
    {"forum_create_btn", {"Créer ce forum", "Create this forum"}},
    {"thread_list_heading", {"FIL", "FEED"}},
    {"thread_list_empty", {"Choisir un forum", "Select a forum"}},
    {"thread_create_heading", {"NOUVEAU POST", "NEW POST"}},
    {"thread_create_title", {"Titre", "Title"}},
    {"thread_create_body", {"Texte", "Body"}},
    {"thread_create_btn", {"Publier le post", "Publish post"}},
    {"thread_create_hint", {"texte et liens uniquement", "text and links only"}},

    // draw_thread_panel.cpp
    {"post_read_empty", {"Choisir un post pour lire le fil", "Select a post to read the thread"}},
    {"post_reply_heading", {"RÉPONDRE", "REPLY"}},
    {"post_reply_btn", {"Publier la réponse", "Publish reply"}},
    {"reply_action", {"répondre", "reply"}},
    {"delete_action", {"retirer", "remove"}},
    {"content_removed", {"[retiré par son auteur]", "[removed by its author]"}},
    {"report_action", {"signaler", "report"}},
    {"report_account_btn", {"Signaler ce compte", "Report this account"}},
    {"replies_heading", {"RÉPONSES", "REPLIES"}},
    {"thread_truncated", {"(fil tronqué par le serveur)", "(thread truncated by server)"}},

    // draw_dm_panel.cpp
    {"profile_heading", {"PROFIL", "PROFILE"}},
    {"profile_name", {"Nom affiché", "Display name"}},
    {"profile_bio", {"Bio", "Bio"}},
    {"profile_btn_save", {"Enregistrer le profil", "Save profile"}},
    {"friends_heading", {"AMIS", "FRIENDS"}},
    {"friends_add", {"Ajouter par clé", "Add by key"}},
    {"social_tab_dm", {"Messages", "Messages"}},
    {"social_tab_social", {"Social", "Social"}},
    {"social_view_btn", {"voir", "view"}},
    {"social_favorite_btn", {"favori", "favorite"}},
    {"social_add_btn", {"+", "+"}},
    {"social_save_btn", {"Enregistrer le top 8", "Save top 8"}},
    {"top8_heading", {"TOP 8", "TOP 8"}},
    {"viewed_profile_heading", {"PROFIL CONSULTÉ", "VIEWED PROFILE"}},
    {"viewed_profile_empty", {"Choisir un ami pour voir son profil", "Select a friend to view their profile"}},
    {"dm_heading", {"MESSAGES PRIVÉS", "DIRECT MESSAGES"}},
    {"dm_status", {"Chiffré de bout en bout", "End-to-end encrypted"}},
    {"dm_desc", {"Seulement vous, et le destinataire, pouvez lire ces messages.", "Only you, and the recipient, can read these messages."}},
    {"dm_btn_prekey", {"Publier ma prekey", "Publish my prekey"}},
    {"dm_btn_inbox", {"Relever la boîte", "Check inbox"}},
    {"dm_recipient", {"Destinataire", "Recipient"}},
    {"dm_btn_send", {"Chiffrer et envoyer", "Encrypt and send"}},
    {"dm_unreadable", {"illisible", "unreadable"}},
    {"identity_heading", {"IDENTITÉ", "IDENTITY"}},
    {"identity_copy_key", {"Copier ma clé publique", "Copy my public key"}},
    {"identity_per_server", {"Cette clé ne vaut que sur ce serveur. Ailleurs vous en avez une autre, sans lien avec celle-ci : vos amis doivent vous rajouter sur chaque serveur.", "This key only applies to this server. Elsewhere you have a different, unlinked one: your friends must add you again on each server."}},

    // draw_server_bar.cpp
    {"servers_heading", {"SERVEURS", "SERVERS"}},
    {"servers_add_heading", {"REJOINDRE", "JOIN"}},
    {"servers_add_hint", {"Collez le lien reçu d'un ami (hypercom://…). Il n'existe pas d'annuaire : on ne rejoint un serveur que sur invitation.", "Paste the link a friend sent you (hypercom://…). There is no directory: you only join a server by invitation."}},
    {"servers_add_btn", {"Ajouter ce serveur", "Add this server"}},
    {"trust_heading", {"AVANT DE VOUS CONNECTER", "BEFORE YOU CONNECT"}},
    {"trust_server", {"Serveur :", "Server:"}},
    {"trust_body", {"L'opérateur de ce serveur verra vos publications, vos forums et le moment de vos connexions. Vos messages privés, eux, restent chiffrés : il ne peut pas les lire.", "This server's operator will see your posts, your forums and when you connect. Your direct messages stay encrypted: they cannot read them."}},
    {"trust_identity", {"Vous utiliserez ici une identité distincte de celle de vos autres serveurs. Aucun opérateur ne peut faire le lien entre elles.", "You will use an identity here that is separate from your other servers. No operator can link them together."}},
    {"trust_accept", {"J'ai compris, se connecter", "Understood, connect"}},
};

const char* tr(const char* key, language lang)
{
    auto it = dictionary.find(key);
    if (it != dictionary.end()) {
        return lang == language::french ? it->second.fr : it->second.en;
    }
    return key;
}

} // namespace hypercom::client
