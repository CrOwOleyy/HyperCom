#pragma once

#include <cstddef>
#include <cstdint>

namespace hypercom::proto {

// Toutes les bornes du protocole, au meme endroit. Elles sont constexpr :
// pas de globale mutable (regle G4), donc pas de limite reglable a chaud.
// Ce qui varie selon le deploiement vit dans hypercom.conf et se passe en
// parametre.

constexpr std::uint16_t PROTOCOL_VERSION = 1;

// Cadrage : [u32 body_size][u8 type][payload]
// body_size compte l'octet de type ET le payload, mais pas le champ lui-meme.
constexpr std::size_t FRAME_LENGTH_FIELD_SIZE = 4;
constexpr std::size_t FRAME_TYPE_FIELD_SIZE = 1;
constexpr std::size_t FRAME_HEADER_SIZE =
    FRAME_LENGTH_FIELD_SIZE + FRAME_TYPE_FIELD_SIZE;

// Plafond dur, verifie AVANT toute allocation.
constexpr std::size_t MAX_FRAME_SIZE = 1024 * 1024;
constexpr std::size_t MAX_BODY_SIZE = MAX_FRAME_SIZE - FRAME_LENGTH_FIELD_SIZE;
constexpr std::size_t MAX_PAYLOAD_SIZE = MAX_FRAME_SIZE - FRAME_HEADER_SIZE;

// Tailles cryptographiques, dupliquees ici pour que le parseur reste
// independant de libsodium et donc fuzzable en isolation.
constexpr std::size_t PUBLIC_KEY_SIZE = 32;
constexpr std::size_t SIGNATURE_SIZE = 64;
constexpr std::size_t AUTH_NONCE_SIZE = 32;

// Bornes applicatives. 16 KiB pour un post : largement de quoi ecrire un long
// billet, et assez bas pour qu'un fil entier tienne dans une trame.
constexpr std::size_t MAX_HANDLE_LENGTH = 32;
constexpr std::size_t MAX_FORUM_NAME_LENGTH = 48;
constexpr std::size_t MAX_FORUM_DESCRIPTION_LENGTH = 512;
constexpr std::size_t MAX_POST_TITLE_LENGTH = 200;
constexpr std::size_t MAX_POST_BODY_LENGTH = 16 * 1024;
// Apercu servi dans une liste de posts. Sans troncature, une page de 200 posts
// tirerait 3 MiB et depasserait la trame a elle seule.
constexpr std::size_t MAX_POST_PREVIEW_LENGTH = 280;
constexpr std::size_t MAX_COMMENT_BODY_LENGTH = 8 * 1024;
constexpr std::size_t MAX_DISPLAY_NAME_LENGTH = 48;
constexpr std::size_t MAX_BIO_LENGTH = 2048;
constexpr std::size_t MAX_THEME_JSON_LENGTH = 1024;
constexpr std::size_t MAX_BLOB_REFERENCE_LENGTH = 96;
constexpr std::size_t MAX_ERROR_MESSAGE_LENGTH = 256;
constexpr std::size_t MAX_MOTD_LENGTH = 4096;
// Motif d'un signalement. Court par construction : c'est un signal pour
// l'admin, pas un rapport d'incident.
constexpr std::size_t MAX_REPORT_REASON_LENGTH = 500;

// Enveloppe DM : opaque pour le serveur, mais bornee comme tout le reste.
constexpr std::size_t MAX_DM_CIPHERTEXT_SIZE = 64 * 1024;

// Pagination : une reponse de liste ne depasse jamais ce nombre d'elements,
// quelle que soit la valeur demandee par le client.
constexpr std::uint16_t MAX_LIST_ITEMS = 200;
constexpr std::uint16_t DEFAULT_LIST_ITEMS = 50;
constexpr std::uint16_t MAX_THREAD_COMMENTS = 500;
constexpr std::uint16_t MAX_DM_BATCH_ITEMS = 100;
constexpr std::uint16_t MAX_FRIEND_ITEMS = 500;

constexpr std::uint8_t TOP8_SLOT_COUNT = 8;

constexpr std::uint16_t DEFAULT_CLEARNET_PORT = 7717;
constexpr std::uint16_t DEFAULT_ONION_PORT = 7718;

} // namespace hypercom::proto
